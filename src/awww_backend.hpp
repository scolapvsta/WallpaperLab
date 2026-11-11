#pragma once
#include "local_library.hpp"
#include <cmath>
#include <csignal>
#include <regex>
#include <thread>
#include <sys/wait.h>
namespace lab {
inline constexpr const char* awwwTransitions[]={"none","simple","fade","left","right","top","bottom","wipe","wave","grow","center","any","outer","random"};
inline constexpr const char* awwwPositions[]={"cursor","center","top","left","right","bottom","top-left","top-right","bottom-left","bottom-right"};
inline bool usesAwww(const Item& item,const json& prefs){return prefs.value("wallpaper_backend",0)==1&&item.origin=="local"&&item.type=="image";}
inline fs::path awwwImage(const Item& item){
 auto root=fs::canonical(item.path);auto project=readJson(root/"project.json");
 auto valid=[&](fs::path path){auto real=fs::canonical(path),relative=real.lexically_relative(root);return !relative.empty()&&*relative.begin()!=".."&&fs::is_regular_file(real)&&localMediaImage(mediaExtension(real));};
 if(auto file=project.value("lab_media_file",std::string());!file.empty()){
  if(valid(root/file))return fs::canonical(root/file);
  throw std::runtime_error("Local image file is missing or outside its project");
 }
 // Imports made before the backend selector stored only the media kind.
 for(auto& entry:fs::directory_iterator(root))if(entry.path().stem()=="media"&&valid(entry.path()))return fs::canonical(entry.path());
 throw std::runtime_error("Local image file is missing or outside its project");
}
inline std::vector<std::string> awwwArgs(const Item& item,const json& prefs,const std::vector<std::string>& monitors){
 if(monitors.empty())throw std::runtime_error("Nessun monitor Hyprland rilevato");
 if(prefs.value("mode",std::string("span"))=="span"&&monitors.size()>1)throw std::runtime_error("awww supports single or mirror mode; select one in Display settings");
 std::string outputs;
 if(prefs.value("mode",std::string())=="single")outputs=monitors.at(std::clamp(prefs.value("screen",0),0,int(monitors.size())-1));
 else for(auto& name:monitors){if(!outputs.empty())outputs+=",";outputs+=name;}
 std::string position=awwwPositions[std::clamp(prefs.value("awww_position",0),0,9)];
 if(position=="cursor"){
  auto result=capture({"hyprctl","cursorpos","-j"});
  try{auto cursor=json::parse(result.at(0));position=std::to_string(cursor.at("x").get<int>())+","+std::to_string(cursor.at("y").get<int>());}
  catch(...){throw std::runtime_error("Cannot read Hyprland cursor position for awww");}
 }
 std::vector<std::string> args={"awww","img",awwwImage(item).string(),"--outputs",outputs,"--transition-type",awwwTransitions[std::clamp(prefs.value("awww_transition",12),0,13)],"--transition-step",std::to_string(std::clamp(prefs.value("awww_step",2),1,100)),"--transition-duration",std::to_string(std::clamp(prefs.value("awww_duration",2000),100,10000)/1000.),"--transition-fps",std::to_string(std::clamp(prefs.value("awww_fps",144),1,240)),"--transition-pos",position,"--transition-angle",std::to_string(std::clamp(prefs.value("awww_angle",45),0,360))};
 if(prefs.value("awww_invert_y",true))args.push_back("--invert-y");
 if(prefs.value("reduce_motion",false)){auto effect=std::find(args.begin(),args.end(),"--transition-type");*(effect+1)="none";}
 return args;
}
inline json importAwwwHook(const std::string& hook){
 std::smatch match;
 if(!std::regex_match(hook,match,std::regex(R"(^\s*awww\s+img\s+["']?\$WALLPAPER_PATH["']?\s+([\s\S]*)$)")))throw std::runtime_error("Caelestia wallpaper hook must begin with awww img and WALLPAPER_PATH");
 auto command=std::regex_replace(match[1].str(),std::regex(R"(\$\(hyprctl\s+cursorpos\s*\|\s*tr\s+-d\s+' '\s*\))"),"cursor");
 auto end=command.find("&&");std::string after=end==std::string::npos?"":command.substr(end+2);command=command.substr(0,end);
 json prefs={{"wallpaper_backend",1},{"awww_transition",1},{"awww_step",2},{"awww_duration",3000},{"awww_fps",30},{"awww_position",1},{"awww_invert_y",false},{"awww_angle",45},{"wallpaper_post_hook",after},{"wallpaper_post_hook_enabled",!after.empty()}};
 std::istringstream input(command);std::string key,value;
 while(input>>key){
  if(key=="--invert-y"){prefs["awww_invert_y"]=true;continue;}
  if(!(input>>value))throw std::runtime_error("Unsupported awww option in Caelestia hook");
  if(key=="--transition-type"||key=="--transition-pos"){
   bool found=false;int count=key=="--transition-type"?14:10;
   for(int n=0;n<count;n++)if(value==(key=="--transition-type"?awwwTransitions[n]:awwwPositions[n])){prefs[key=="--transition-type"?"awww_transition":"awww_position"]=n;found=true;break;}
   if(!found)throw std::runtime_error("Unsupported awww option in Caelestia hook");
  }else{
   if(!std::regex_match(value,std::regex(R"([0-9]+(\.[0-9]+)?)")))throw std::runtime_error("Unsupported awww option in Caelestia hook");
   if(key=="--transition-duration"){double seconds=std::stod(value);if(seconds<.1||seconds>10)throw std::runtime_error("Unsupported awww option in Caelestia hook");prefs["awww_duration"]=int(std::round(seconds*1000));}
   else{int number=std::stoi(value);if(value.find('.')!=std::string::npos)throw std::runtime_error("Unsupported awww option in Caelestia hook");
    if(key=="--transition-step"&&number>=1&&number<=255)prefs["awww_step"]=std::min(number,100);
    else if(key=="--transition-fps"&&number>=1&&number<=240)prefs["awww_fps"]=number;
    else if(key=="--transition-angle"&&number>=0&&number<=360)prefs["awww_angle"]=number;
    else throw std::runtime_error("Unsupported awww option in Caelestia hook");
   }
  }
 }
 return prefs;
}
inline std::vector<std::string> awwwMatchingOutputs(const std::string& query,const std::string& image,const std::string& outputs){
 std::vector<std::string> matched;std::istringstream names(outputs);std::string name;
 while(std::getline(names,name,',')){std::istringstream lines(query);std::string line;while(std::getline(lines,line))if(line.find(": "+name+": ")!=std::string::npos&&line.ends_with("currently displaying: image: "+image)){matched.push_back(name);break;}}
 return matched;
}
inline volatile sig_atomic_t awwwStopRequested=0;
inline void awwwStopHandler(int){awwwStopRequested=1;}
inline int awwwMain(const std::vector<std::string>& args,const fs::path& ready){
 std::signal(SIGTERM,awwwStopHandler);std::signal(SIGINT,awwwStopHandler);
 if(args.size()<5||args[0]!="awww"||args[1]!="img")throw std::runtime_error("Invalid awww session arguments");
 if(capture(args).empty())throw std::runtime_error("awww failed; check that awww-daemon is running. Previous wallpaper retained.");
 {std::ofstream file(ready);file<<getpid()<<'\n';if(!file)throw std::runtime_error("Cannot write awww readiness file");}
 while(!awwwStopRequested){
  for(int n=0;n<10&&!awwwStopRequested;n++)std::this_thread::sleep_for(std::chrono::milliseconds(100));
  if(awwwStopRequested)break;auto query=capture({"awww","query"});
  if(query.empty()||awwwMatchingOutputs(query[0],args[2],args[4]).empty())break;
 }
 return 0;
}
// Explicit Stop clears only images still belonging to this application.
inline void clearAwww(const json& process){
 if(process.contains("outputs")){for(auto& child:process["outputs"])clearAwww(child);return;}
 if(process.value("backend",std::string())!="awww")return;
 auto query=capture({"awww","query"});if(query.empty())return;std::string outputs;
 for(auto& name:awwwMatchingOutputs(query[0],process.value("image",std::string()),process.value("awww_outputs",std::string()))){if(!outputs.empty())outputs+=",";outputs+=name;}
 if(!outputs.empty())capture({"awww","clear","--outputs",outputs});
}
inline void runWallpaperPostHook(const Item& item,const json& prefs){
 if(!usesAwww(item,prefs)||!prefs.value("wallpaper_post_hook_enabled",false))return;
 auto hook=prefs.value("wallpaper_post_hook",std::string());if(hook.empty())return;
 try{
  auto result=capture({"/bin/sh","-c","export WALLPAPER_PATH=\"$1\"\n(\n"+hook+"\n) >\"$2\" 2>&1\npost_hook_status=$?\nprintf '%s' \"$post_hook_status\"\n","wallpaper-lab-post-hook",awwwImage(item).string(),(stateDir()/"post-hook.log").string()});
  writeJson(stateDir()/"post-hook-status.json",{{"ok",!result.empty()&&result[0]=="0"}});
 }catch(const std::exception& e){std::ofstream log(stateDir()/"post-hook.log");log<<e.what();}
}
}
