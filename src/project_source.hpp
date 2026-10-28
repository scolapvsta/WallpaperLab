#pragma once
#include "catalog.hpp"
#include <set>
#include <unistd.h>
#include <iomanip>
#include <sstream>
namespace lab {
// Resolve installed Workshop dependencies without changing downloaded projects.
inline json resolveProject(const fs::path& path,const std::vector<fs::path>& projects,std::set<fs::path> chain={}) {
    auto real=fs::canonical(path);
    if(!chain.insert(real).second)throw std::runtime_error("Dipendenza Workshop circolare");
    auto preset=readJson(real/"project.json");
    if(!preset.contains("dependency")){preset["_lab_base"]=real.string();return preset;}
    auto dep=preset["dependency"];std::string id=dep.is_string()?dep.get<std::string>():dep.dump();
    if(id.empty()||id.find_first_not_of("0123456789")!=std::string::npos)throw std::runtime_error("Dipendenza Workshop non valida");
    fs::path base=real.parent_path()/id;
    if(!fs::is_regular_file(base/"project.json"))for(auto& candidate:projects)if(candidate.filename()==id&&fs::is_regular_file(candidate/"project.json")){base=candidate;break;}
    if(!fs::is_regular_file(base/"project.json"))throw std::runtime_error("Dipendenza Workshop non installata: "+id);
    auto merged=resolveProject(base,projects,chain);
    auto layers=merged.value("_lab_layers",json::array());layers.push_back(real.string());merged["_lab_layers"]=layers;
    auto& props=merged["general"]["properties"];
    if(preset.contains("preset")&&preset["preset"].is_object())for(auto& [key,value]:preset["preset"].items()){
        if(key=="audioprocessing"&&value.is_boolean())merged["general"]["supportsaudioprocessing"]=value;
        if(!value.is_null()&&props.contains(key)&&props[key].is_object())props[key]["value"]=value;
    }
    for(auto key:{"title","preview","description","tags","contentrating"})if(preset.contains(key))merged[key]=preset[key];
    return merged;
}
inline void overlayProject(const fs::path& source,const fs::path& target){
    fs::create_directories(target);
    for(auto& entry:fs::directory_iterator(source)){
        if(entry.path().filename()=="project.json")continue;
        auto dest=target/entry.path().filename();
        if(entry.is_directory()){if(fs::is_symlink(dest)||fs::is_regular_file(dest))fs::remove(dest);overlayProject(entry.path(),dest);}
        else if(entry.is_regular_file()){if(fs::exists(dest)||fs::is_symlink(dest))fs::remove_all(dest);std::error_code ec;fs::create_hard_link(entry.path(),dest,ec);if(ec)fs::copy_file(entry.path(),dest);}
    }
}
inline fs::path materializeProject(const std::string& identity,const json& source){
    // Content and layer mtimes distinguish versions; an active cache is immutable.
    std::string fingerprint="hardlink-v2"+identity+source.dump();std::vector<fs::path> layers={source.at("_lab_base").get<std::string>()};
    for(auto& layer:source.value("_lab_layers",json::array()))layers.push_back(layer.get<std::string>());
    for(auto& layer:layers)for(auto& entry:fs::recursive_directory_iterator(layer))fingerprint+=entry.path().string()+std::to_string(entry.last_write_time().time_since_epoch().count());
    uint64_t hash=14695981039346656037ull;for(unsigned char c:fingerprint){hash^=c;hash*=1099511628211ull;}std::ostringstream name;name<<std::hex<<hash;
    auto root=stateDir()/"resolved-projects";fs::create_directories(root);auto result=root/name.str();if(fs::is_regular_file(result/"project.json"))return result;
    auto temp=root/(name.str()+".tmp-"+std::to_string(getpid()));fs::remove_all(temp);
    try{for(auto& layer:layers)overlayProject(layer,temp);auto data=source;data.erase("_lab_base");data.erase("_lab_layers");auto& props=data["general"]["properties"];
        for(auto& [key,p]:props.items())if(p.value("type",std::string())=="file"&&p.contains("value")&&p["value"].is_string()){
            auto file=fs::path(p["value"].get<std::string>());if(!file.empty()&&file.is_relative()&&fs::is_regular_file(temp/file))p["value"]=(result/file).string();
        }
        writeJson(temp/"project.json",data);if(!fs::exists(result))fs::rename(temp,result);else fs::remove_all(temp);}catch(...){fs::remove_all(temp);throw;}
    return result;
}
}
