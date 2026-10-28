#include "project_source.hpp"
#include <fstream>
#include <iostream>
using namespace lab;
static void check(bool ok){if(!ok)throw std::runtime_error("Preset dependency regression");}
int main(){
 auto root=fs::temp_directory_path()/("wallpaper-presets-"+std::to_string(getpid()));fs::create_directories(root);setenv("XDG_STATE_HOME",root.c_str(),1);
 try{
  auto base=root/"100",preset=root/"200";json properties={{"level",{{"type","slider"},{"value",3},{"min",0},{"max",10}}},{"image",{{"type","file"},{"value",""}}}};
  writeJson(base/"project.json",{{"title","Base"},{"type","web"},{"file","index.html"},{"general",{{"properties",properties}}}});std::ofstream(base/"index.html")<<"base html";fs::create_directories(preset/"files");std::ofstream(preset/"files/image.png")<<"preset image";
  writeJson(preset/"project.json",{{"title","Preset"},{"dependency","100"},{"preset",{{"level",7},{"image","files/image.png"},{"audioprocessing",true}}}});
  auto merged=resolveProject(preset,{});check(merged["title"]=="Preset"&&merged["type"]=="web"&&merged["general"]["properties"]["level"]["value"]==7&&merged["general"]["supportsaudioprocessing"]==true);
  auto cached=materializeProject(preset.string(),merged);check(fs::is_regular_file(cached/"index.html")&&!fs::is_symlink(cached/"index.html"));check(fs::canonical(cached/"index.html").parent_path()==cached);
  check(readJson(cached/"project.json")["general"]["properties"]["image"]["value"]==(cached/"files/image.png").string());check(readJson(base/"project.json")["general"]["properties"]["level"]["value"]==3);check(materializeProject(preset.string(),merged)==cached);
  writeJson(root/"300/project.json",{{"dependency","200"},{"title","Nested"},{"preset",{{"level",9}}}});check(resolveProject(root/"300",{})["general"]["properties"]["level"]["value"]==9);
  bool rejected=false;writeJson(root/"400/project.json",{{"dependency","999"}});try{resolveProject(root/"400",{});}catch(...){rejected=true;}check(rejected);
  rejected=false;writeJson(root/"400/project.json",{{"dependency","400"}});try{resolveProject(root/"400",{});}catch(...){rejected=true;}check(rejected);
  rejected=false;writeJson(root/"400/project.json",{{"dependency","../100"}});try{resolveProject(root/"400",{});}catch(...){rejected=true;}check(rejected);
  fs::remove_all(root);std::cout<<"Installed dependencies, typed defaults, local assets, caching and cycles passed\n";
 }catch(...){fs::remove_all(root);throw;}
}
