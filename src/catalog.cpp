#include "project_properties.hpp"
#include "project_source.hpp"
#include "catalog.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
namespace lab {
static fs::path home() { const char* p = getenv("HOME"); if (!p) throw std::runtime_error("HOME is not set"); return p; }
fs::path configDir() { auto p = getenv("XDG_CONFIG_HOME"); return (p ? fs::path(p) : home()/".config")/"wallpaper-lab"; }
fs::path stateDir() { auto p = getenv("XDG_STATE_HOME"); return (p ? fs::path(p) : home()/".local/state")/"wallpaper-lab"; }
json readJson(const fs::path& path) {
    if (fs::file_size(path) > 4*1024*1024) throw std::runtime_error("JSON exceeds 4 MiB: " + path.string());
    std::ifstream f(path); return json::parse(f);
}
void writeJson(const fs::path& path, const json& value) {
    fs::create_directories(path.parent_path());
    auto tmp = path.string()+".tmp."+std::to_string(getpid());
    { std::ofstream f(tmp); f << value.dump(2) << '\n'; if (!f) throw std::runtime_error("Cannot write " + tmp); }
    fs::rename(tmp,path);
}
std::vector<std::string> capture(const std::vector<std::string>& args) {
    int pipes[2]; if (pipe(pipes)) throw std::runtime_error("pipe failed");
    posix_spawn_file_actions_t fa; posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_adddup2(&fa,pipes[1],STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&fa,pipes[0]); posix_spawn_file_actions_addclose(&fa,pipes[1]);
    std::vector<char*> av; for (const auto& s:args) av.push_back(const_cast<char*>(s.c_str())); av.push_back(nullptr);
    pid_t pid; int err=posix_spawnp(&pid,av[0],&fa,nullptr,av.data(),environ);
    posix_spawn_file_actions_destroy(&fa); close(pipes[1]);
    if (err) { close(pipes[0]); return {}; }
    std::string out; char buf[4096]; ssize_t n;
    while ((n=read(pipes[0],buf,sizeof(buf)))>0) out.append(buf,n);
    close(pipes[0]); int status; while(waitpid(pid,&status,0)<0 && errno==EINTR) {}
    if (!WIFEXITED(status) || WEXITSTATUS(status)) return {};
    return {out};
}
std::vector<fs::path> steamLibraries() {
    std::vector<fs::path> seeds={home()/".local/share/Steam",home()/".steam/steam",home()/".var/app/com.valvesoftware.Steam/.local/share/Steam"};
    std::set<fs::path> paths;
    for (auto p:seeds) if(fs::is_directory(p)) paths.insert(fs::canonical(p));
    const std::regex re("\"path\"\\s*\"([^\"]+)\"");
    for (auto p:seeds) {
        std::ifstream f(p/"steamapps/libraryfolders.vdf");
        std::string text((std::istreambuf_iterator<char>(f)),{});
        for (auto i=std::sregex_iterator(text.begin(),text.end(),re); i!=std::sregex_iterator(); ++i)
            if(fs::is_directory((*i)[1].str())) paths.insert(fs::canonical((*i)[1].str()));
    }
    return {paths.begin(),paths.end()};
}
std::string assetsPath() {
    for (auto p:steamLibraries()) if(fs::is_directory(p/"steamapps/common/wallpaper_engine/assets")) return (p/"steamapps/common/wallpaper_engine/assets").string();
    return {};
}
static std::string lower(std::string s) { std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return std::tolower(c);});return s; }
static std::string stringValue(const json& j,const char* key,const std::string& fallback="") { auto i=j.find(key); return i!=j.end()&&i->is_string()?i->get<std::string>():fallback; }
bool matches(const Item& item,int type,int resolution,int audio) {
    const char* types[]={"","scene","video","web","preset"};
    if(type>0&&(type>4||item.type!=types[type]))return false;
    if(audio==1&&item.audio!=1)return false;
    if(audio==2&&item.audio!=0)return false;
    if(audio==3&&item.audio!=-1)return false;
    if(resolution){
        bool known=item.width>0&&item.height>0;
        if(resolution==6)return !known;
        if(!known)return false;
        if(resolution==1&&!(item.height==1080&&item.width==1920))return false;
        if(resolution==2&&!(item.height==1440&&item.width==2560))return false;
        if(resolution==3&&!(item.height==2160&&item.width==3840))return false;
        if(resolution==4&&item.width/(float)item.height<=2.f)return false;
        if(resolution==5&&item.width>=item.height)return false;
        if(resolution==7&&!(item.width==3840&&item.height==1080))return false;
        if(resolution==8&&!(item.width==5120&&item.height==1440))return false;
        if(resolution==9&&!(item.width==7680&&item.height==2160))return false;
    }
    return true;
}
bool resolutionMatches(const Item& item,const ResolutionFilter& f){
    if(f.code==0)return true;
    if(f.code==6)return item.supported.empty()&&item.width<=0;
    auto exact=[&](int w,int h){return w==f.width&&h==f.height;};
    if(f.width>0){
        if(exact(item.width,item.height))return true;
        for(auto [w,h]:item.supported)if(exact(w,h))return true;
        return false;
    }
    return matches(item,0,f.code,0);
}
bool matchesCriteria(const Item& item,const std::vector<ResolutionFilter>& filters,int type,int audio,const std::vector<int>& codes,bool any,int groups){
 bool result=!any,active=false;auto criterion=[&](bool test){active=true;result=any?result||test:result&&test;};
 if(type)criterion(matches(item,type,0,0));if(audio)criterion(matches(item,0,0,audio));
 for(int g=0;g<6;g++)if(groups&(1<<g)){bool active=false,matched=false;for(auto& f:filters)if(f.group==g&&std::find(codes.begin(),codes.end(),f.code)!=codes.end()){active=true;matched|=resolutionMatches(item,f);}if(active)criterion(matched);}
 for(int code:codes){bool matched=false,grouped=false;for(auto& f:filters)if(f.code==code){matched=resolutionMatches(item,f);grouped=groups&(1<<f.group);}if(!grouped)criterion(matched);}
 return !active||result;
}
bool resolutionAll(const Item& item,const std::vector<ResolutionFilter>& filters,const std::vector<int>& codes){
    for(int code:codes){auto f=std::find_if(filters.begin(),filters.end(),[&](auto& f){return f.code==code;});if(f==filters.end()||!resolutionMatches(item,*f))return false;}return true;
}
std::vector<ResolutionFilter> resolutionFilters(const std::vector<Item>& items){
    // Wallpaper Engine's official common resolutions, grouped for accordion filters.
    std::vector<ResolutionFilter> result={
        {0,0,0,0,"Tutte le risoluzioni",5},
        {10,1280,720,0,"1280 × 720",0},{11,1366,768,0,"1366 × 768",0},
        {1,1920,1080,0,"1920 × 1080",0},{2,2560,1440,0,"2560 × 1440",0},{3,3840,2160,0,"3840 × 2160",0},
        {7,3840,1080,0,"3840 × 1080",1},{8,5120,1440,0,"5120 × 1440",1},{9,7680,2160,0,"7680 × 2160",1},
        {12,2560,1080,0,"2560 × 1080",2},{13,3440,1440,0,"3440 × 1440",2},
        {14,4096,768,0,"4096 × 768",3},{15,5760,1080,0,"5760 × 1080",3},{16,7680,1440,0,"7680 × 1440",3},{17,11520,2160,0,"11520 × 2160",3},
        {18,720,1280,0,"720 × 1280",4},{19,1080,1920,0,"1080 × 1920",4},{20,1440,2560,0,"1440 × 2560",4},{21,2160,3840,0,"2160 × 3840",4},
        {6,0,0,0,"Da verificare",5}
    };
    for(auto& f:result)for(auto& item:items)if(resolutionMatches(item,f))f.count++;
    return result;
}
static std::vector<std::pair<int,int>> dimensions(const std::string& text){
    std::vector<std::pair<int,int>> result;
    std::string normalized=text;for(size_t at=0;(at=normalized.find("×",at))!=std::string::npos;)normalized.replace(at,2,"x");
    static const std::regex pattern(R"((\d{3,5})\s*[xX]\s*(\d{3,5}))");
    for(auto i=std::sregex_iterator(normalized.begin(),normalized.end(),pattern);i!=std::sregex_iterator();++i){int w=std::stoi((*i)[1]),h=std::stoi((*i)[2]);if(w<=16384&&h<=16384)result.emplace_back(w,h);}
    return result;
}
static void declaredResolutions(Item& item,const json& project){
    std::set<std::pair<int,int>> sizes;
    if(item.width>0&&item.height>0)sizes.emplace(item.width,item.height);
    if(project.contains("tags")&&project["tags"].is_array())for(auto& tag:project["tags"])if(tag.is_string())for(auto size:dimensions(tag.get<std::string>()))sizes.insert(size);
    auto general=project.value("general",json::object());
    if(general.is_object()&&general.contains("properties")&&general["properties"].is_object()){
        for(auto& [key,property]:general["properties"].items()){
            if(!property.is_object()||property.value("type",std::string())!="combo")continue;
            auto name=lower(key+" "+stringValue(property,"text"));
            if(name.find("resolution")==std::string::npos&&name.find("risoluz")==std::string::npos)continue;
            if(!property.contains("options")||!property["options"].is_array())continue;
            for(auto& option:property["options"]){
                if(!option.is_object()||!option.contains("value")||!(option["value"].is_string()||option["value"].is_number()))continue;
                auto value=option["value"].is_string()?option["value"].get<std::string>():option["value"].dump();
                auto label=stringValue(option,"label",value);auto dims=dimensions(label);if(dims.empty())dims=dimensions(value);
                if(dims.size()!=1)continue;auto [w,h]=dims[0];sizes.emplace(w,h);
                item.variants.push_back({w,h,key,value,label});
            }
        }
    }
    item.supported.assign(sizes.begin(),sizes.end());
}
// Read only the requested JSON entry; never unpack Workshop assets to disk.
static json sceneMetadata(const fs::path& project,const std::string& file) {
    fs::path relative=file;
    if(relative.is_absolute())return {};
    for(auto& part:relative)if(part=="..")return {};
    auto direct=fs::weakly_canonical(project/relative);
    auto rel=direct.lexically_relative(project);
    if(!rel.empty()&&*rel.begin()!=".."&&fs::is_regular_file(direct))return readJson(direct);
    std::ifstream stream(project/"scene.pkg",std::ios::binary);
    if(!stream)return {};
    auto number=[&](){unsigned char b[4];if(!stream.read((char*)b,4))throw std::runtime_error("Truncated package");return uint32_t(b[0])|(uint32_t(b[1])<<8)|(uint32_t(b[2])<<16)|(uint32_t(b[3])<<24);};
    auto string=[&](){auto n=number();if(n>4096)throw std::runtime_error("Invalid package string");std::string s(n,'\0');if(!stream.read(s.data(),n))throw std::runtime_error("Truncated package string");return s;};
    if(!string().starts_with("PKGV"))return {};
    auto count=number();if(count>100000)return {};
    uint32_t offset=0,length=0;
    for(uint32_t n=0;n<count;n++){
        auto name=string();auto start=number(),size=number();
        if(name==file){offset=start;length=size;}
        if(stream.tellg()>8*1024*1024)return {};
    }
    if(!length||length>4*1024*1024)return {};
    auto base=uint64_t(stream.tellg()),total=fs::file_size(project/"scene.pkg");
    if(base+offset+length>total)return {};
    stream.seekg(base+offset);std::string text(length,'\0');
    if(!stream.read(text.data(),length))return {};
    return json::parse(text);
}
static void metadata(Item& item,const json& project,const fs::path& path,bool probeVideos) {
    // Optional metadata errors must never make an otherwise playable project invalid.
    try {
        auto general=project.value("general",json::object());
        item.audio=general.value("supportsaudioprocessing",false)?1:0;
        if(item.type=="scene"){
            auto scene=sceneMetadata(path,stringValue(project,"file","scene.json"));
            if(scene.contains("general")){
                auto projection=scene["general"].value("orthogonalprojection",json::object());
                if(projection.is_null())item.reason="Scena 3D prospettica: rendering dei modelli 3D non ancora supportato";
                if(projection.contains("width")&&projection["width"].is_number())item.width=projection["width"].get<int>();
                if(projection.contains("height")&&projection["height"].is_number())item.height=projection["height"].get<int>();
            }
        } else if(item.type=="video"&&probeVideos){
            auto media=fs::weakly_canonical(path/stringValue(project,"file"));
            auto rel=media.lexically_relative(path);
            if(!rel.empty()&&*rel.begin()!=".."&&fs::is_regular_file(media)){
                auto output=capture({"ffprobe","-v","quiet","-protocol_whitelist","file","-select_streams","v:0","-show_entries","stream=width,height","-of","json",media.string()});
                if(!output.empty()){
                    auto data=json::parse(output[0]);
                    if(data.contains("streams")&&!data["streams"].empty()){
                        item.width=data["streams"][0].value("width",0);item.height=data["streams"][0].value("height",0);
                    }
                }
            }
        }
        if(item.width<1||item.height<1||item.width>65536||item.height>65536)item.width=item.height=0;
    }catch(...){}
}
std::vector<Item> scan(const std::vector<fs::path>& roots,bool probeVideos) {
    std::vector<fs::path> projects;
    for(auto root:roots) {
        if(!fs::is_directory(root)) continue;
        for(const auto& entry:fs::directory_iterator(root,fs::directory_options::skip_permission_denied)) {
            if(entry.is_directory())projects.push_back(entry.path());
        }
    }
    return scanProjects(projects,probeVideos);
}
std::vector<Item> scanProjects(const std::vector<fs::path>& projects,bool probeVideos) {
    std::vector<Item> out; std::set<fs::path> seen;
    for(const auto& project:projects) {
            if(!fs::is_directory(project) || !fs::is_regular_file(project/"project.json")) continue;
            auto path=fs::canonical(project); if(!seen.insert(path).second) continue;
            Item item; item.path=path.string(); item.id=path.filename().string(); item.title=item.id;
            try {
                auto original=readJson(path/"project.json");auto j=original;
                if(original.contains("dependency")){try{j=resolveProject(path,projects);item.source=j;}catch(const std::exception& e){item.reason=e.what();}}
                item.title=stringValue(j,"title",item.id); item.type=lower(stringValue(j,"type"));
                auto preview=stringValue(j,"preview");
                if(!preview.empty()) {
                    auto target=fs::weakly_canonical(path/preview);
                    auto rel=target.lexically_relative(path);
                    if(!rel.empty() && *rel.begin()!=".." && fs::is_regular_file(target)) item.preview=target.string();
                }
                if(item.preview.empty()) for(auto name:{"preview.jpg","preview.gif","preview.png"}) if(fs::is_regular_file(path/name)) { item.preview=(path/name).string();break; }
                if(!item.reason.empty()) { item.type="preset"; }
                else if(item.type.empty() && fs::exists(path/"scene.pkg")) item.type="scene";
                if(item.type!="scene" && item.type!="video" && item.type!="web" && item.reason.empty()) item.reason="Tipo di progetto non supportato";
                auto assetPath=item.source.is_null()?path:fs::path(j.value("_lab_base",path.string()));
                if(item.type=="scene" && !fs::exists(assetPath/"scene.pkg") && !fs::exists(assetPath/stringValue(j,"file","scene.json"))) item.reason="File della scena mancante";
                if(item.type=="video" || item.type=="web") {
                    auto f=stringValue(j,"file");
                    if(f.empty() || !fs::is_regular_file((item.source.is_null()?path:fs::path(j.value("_lab_base",path.string())))/f)) item.reason="File del progetto mancante";
                }
                if(item.type=="scene"||item.type=="video"||item.type=="web")metadata(item,j,assetPath,probeVideos);
                declaredResolutions(item,j);
                auto general=j.value("general",json::object());
                if(general.is_object()&&general.contains("properties")&&general["properties"].is_object())item.properties=general["properties"];
                item.propertyOrder=propertyKeys(item.properties);
            } catch(const std::exception& e) {item.type="invalid"; item.reason=e.what();}
            out.push_back(std::move(item));
    }
    std::sort(out.begin(),out.end(),[](const Item&a,const Item&b){return lower(a.title)<lower(b.title);});return out;
}
}
