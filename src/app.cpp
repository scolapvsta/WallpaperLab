#include "lab.h"
#include "i18n.hpp"
#include "workshop.hpp"
#include "catalog.hpp"
#include "preferences.hpp"
#include "project_properties.hpp"
#include "project_source.hpp"
#include "shortcuts.hpp"
#include "popup.hpp"
#include "displays.hpp"
#include "local_library.hpp"
#include "app_background.hpp"
#include "caelestia_integration.hpp"
#include "theme_catalog.h"
#include "theme_fonts.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstring>
#include <cctype>
#include <fcntl.h>
#include <fstream>
#include <future>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <spawn.h>
#include <sstream>
#include <set>
#include <sys/file.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
extern char **environ;
int engineMain(int,char**,const std::string&);
namespace {
using namespace lab;
std::vector<Item> items;
std::vector<std::string> monitors, monitorLabels;
std::vector<Display> displayData;
DisplayCanvas displayDraft;
bool displayDirty=false;
std::string displayMessage,displayInfo;
json carousel=json::array(), shortcut, popupSnapshot, placements=json::object();
std::vector<ResolutionFilter> resolutions;
bool switcherDemo=false;std::chrono::steady_clock::time_point demoStarted;
json switcherDemoProcess;
std::string activePath;
std::string fontPath, brandFontPath, buttonFontPath, fallbackFontPath, shortcutLabel;
int popupMaxWidth=800,popupMaxHeight=640;
json settings={{"mode","span"},{"screen",0},{"fps",60},{"muted",true},{"disable_mouse",true},{"disable_parallax",true},{"disable_particles",false}};
std::string errorMessage,statusMessage;
std::future<std::string> pending;
bool pendingPreview=false,shortcutCapture=false;
int deletingIndex=-1,deletedResult=-1;
int queuedSelection=-1;json queuedPlacements=json::object();
void flushPlacements(){for(auto& [path,pos]:queuedPlacements.items())saveWallpaperPosition(path,pos.at("x"),pos.at("y"));queuedPlacements=json::object();placements=loadPlacements();}
fs::path feedDir(){auto runtime=getenv("XDG_RUNTIME_DIR");return runtime?fs::path(runtime)/("wallpaper-lab-"+std::to_string(std::hash<std::string>{}(stateDir().string()))):stateDir()/"live-feed";}
json previewProcess;
bool stagedCatalogReady=false;std::future<std::vector<Item>> catalogScan;std::vector<Item> stagedItems;std::vector<ResolutionFilter> stagedResolutions;
std::vector<std::string> fontFamilies={"Default"};int fontRevision=0,appliedWeight=-1;std::string appliedFamily;
std::vector<fs::path> libraryRoots(){
 if(auto extra=getenv("WALLPAPER_LAB_LIBRARY"))return {fs::path(extra)};
 auto prefs=loadSettings();std::vector<fs::path> roots;auto custom=prefs.value("workshop_path",std::string());if(!custom.empty())roots.emplace_back(custom);else for(auto p:steamLibraries())roots.push_back(p/"steamapps/workshop/content/431960");auto local=localLibrary(prefs);fs::create_directories(local);roots.push_back(local);return roots;
}
std::string outputKey(std::string name){for(char& c:name)if(!std::isalnum((unsigned char)c)&&c!='-'&&c!='_')c='_';return name;}
fs::path feedFrame(const std::string& output){auto file=feedDir()/("frame-"+outputKey(output)+".bin");return fs::exists(file)?file:feedDir()/"frame.bin";}
std::future<json> fileJob;std::string fileNotice;
json selectedRunning(const json& selection);
bool owned(const json& process);
std::mutex integrationStatusMutex;std::future<void> integrationJob;bool integrationDirty=false;
extern "C" void startIntegrationJob();
void syncCaelestia(const Item& item,const json& prefs,bool alreadyLocked=false){
 if(!prefs.value("caelestia_enabled",false))return;
 fs::path capturedArtwork;
 try{
  // Prefer the actual rendered frame, including crop and image adjustments.
  // Fall back to the author's thumbnail when a paused renderer has no frame.
  auto process=selectedRunning(prefs);if(owned(process)&&process.value("path",std::string())==item.path){int screen=std::clamp(prefs.value("screen",0),0,std::max(0,(int)monitors.size()-1));writeJson(feedDir()/"request.json",{{"path",item.path},{"output",monitors.empty()?"":monitors[screen]}});auto requested=fs::last_write_time(feedDir()/"request.json");for(int tries=0;tries<10;tries++){std::this_thread::sleep_for(std::chrono::milliseconds(40));try{auto frame=feedFrame(monitors.empty()?"":monitors[screen]);if(fs::last_write_time(frame)<requested)continue;std::ifstream input(frame,std::ios::binary);uint32_t header[5]={};input.read((char*)header,20);if(header[0]!=0x574c4631||header[1]!=process.at("pid").get<uint32_t>()||header[2]<1||header[3]<1||uint64_t(header[2])*header[3]>40000000)continue;std::vector<char> pixels(size_t(header[2])*header[3]*3);input.read(pixels.data(),pixels.size());if(!input)continue;capturedArtwork=stateDir()/"integrations"/("artwork-"+std::to_string(getpid())+"-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".ppm");fs::create_directories(capturedArtwork.parent_path());std::ofstream ppm(capturedArtwork,std::ios::binary);ppm<<"P6\n"<<header[2]<<" "<<header[3]<<"\n255\n";ppm.write(pixels.data(),pixels.size());break;}catch(...){} }}
 }catch(...){}
 try{if(!fs::is_regular_file(stateDir()/"integrations/caelestia-runtime.json"))prepareCaelestia();std::unique_ptr<ControllerLock> publicationLock;bool published=updateCaelestia(capturedArtwork.empty()?item.preview:capturedArtwork.string(),true,[&]{if(!alreadyLocked)try{publicationLock=std::make_unique<ControllerLock>();}catch(...){return false;}auto latest=loadSettings();if(!latest.value("caelestia_enabled",false)||latest.value("caelestia_palette",0)!=prefs.value("caelestia_palette",0)||latest.value("caelestia_mode",0)!=prefs.value("caelestia_mode",0))return false;auto active=selectedRunning(latest);return owned(active)&&active.value("path",std::string())==item.path;},prefs.value("caelestia_palette",0),prefs.value("caelestia_mode",0));if(published){std::lock_guard statusLock(integrationStatusMutex);writeJson(stateDir()/"integration-status.json",{{"ok",true}});}}
 catch(const std::exception& e){try{std::lock_guard statusLock(integrationStatusMutex);writeJson(stateDir()/"integration-status.json",{{"ok",false},{"error",e.what()}});}catch(...){}}
 if(!capturedArtwork.empty()){std::error_code ec;fs::remove(capturedArtwork,ec);}
}


void resolveFonts(bool discover=false){
    registerThemeFonts();
    if(discover){
        auto list=capture({"fc-list",":lang=en","-f","%{family[0]}\n"});
        if(!list.empty()){std::set<std::string> families;std::istringstream lines(list[0]);std::string family;
            while(std::getline(lines,family))if(!family.empty()&&family.size()<100&&family.find("Symbols")==std::string::npos&&family.find("Emoji")==std::string::npos&&family.find("Math")==std::string::npos&&family.find("Rotated")==std::string::npos)families.insert(family);
            fontFamilies={"Default","Rajdhani","Oxanium","Orbitron"};for(auto& name:families){
                if(name=="Rajdhani"||name=="Oxanium"||name=="Orbitron")continue;
                if(name.find("Nerd Font")!=std::string::npos&&(name.ends_with(" Mono")||name.ends_with(" Propo")||name.find("MonoNL")!=std::string::npos))continue;
                if(name.find("CJK ")!=std::string::npos&&!name.ends_with(" SC"))continue;
                if(fontFamilies.size()>=96)break;fontFamilies.push_back(name);
            }
            auto selected=settings.value("font_family",std::string());if(!selected.empty()&&std::find(fontFamilies.begin(),fontFamilies.end(),selected)==fontFamilies.end()&&families.contains(selected)){if(fontFamilies.size()>=96)fontFamilies.pop_back();fontFamilies.push_back(selected);}
        }
    }
    std::string family=settings.value("font_family",std::string());if(family.empty())family=settings.value("ui_design",1)==2?"Oxanium":"Sans";
    auto match=[](const std::string& name){auto colon=name.find(':');auto bundled=bundledFont(name.substr(0,colon));if(!bundled.empty())return bundled;auto result=capture({"fc-match","-f","%{index}|%{file}",name});if(result.empty())return std::string();auto split=result[0].find('|');if(split==std::string::npos)return result[0];return "@"+result[0].substr(0,split)+":"+result[0].substr(split+1);};
    std::string weight=settings.value("font_weight",1)==0?"medium":settings.value("font_weight",1)==2?"bold":"demibold";
    fontPath=match((family.empty()?"Adwaita Sans":family)+":weight="+weight);
    buttonFontPath=match(family.empty()?"DejaVu Sans:style=Bold":family+":weight=demibold");
    brandFontPath=match((family.empty()?"Noto Serif":family)+":weight="+weight);
    fallbackFontPath=match("Noto Sans CJK SC");appliedFamily=settings.value("font_family",std::string());appliedWeight=settings.value("font_weight",1);fontRevision++;
}
std::string executable() {return fs::read_symlink("/proc/self/exe").string();}
std::string startTime(pid_t pid) {
    std::ifstream f("/proc/"+std::to_string(pid)+"/stat");std::string s;std::getline(f,s);
    auto pos=s.rfind(')');if(pos==std::string::npos)return {};
    std::istringstream stream(s.substr(pos+2));for(int i=0;i<=19;++i)stream>>s;return s;
}
bool owned(const json& process) {
    try {
        int pid=process.at("pid");
        return pid>1 && process.at("start")==startTime(pid) && fs::read_symlink("/proc/"+std::to_string(pid)+"/exe")==executable();
    }catch(...){return false;}
}
void stop(const json& process) {
    if(process.contains("outputs")){for(auto& [name,child]:process["outputs"].items())stop(child);return;}
    if(!owned(process))return;
    int pid=process.at("pid");
    kill(-pid,SIGTERM);
    for(int i=0;i<50 && owned(process);++i) {int status;waitpid(pid,&status,WNOHANG);std::this_thread::sleep_for(std::chrono::milliseconds(50));}
    if(owned(process))kill(-pid,SIGKILL);
    int status;waitpid(pid,&status,WNOHANG);
}
json running() {try{return readJson(stateDir()/"running.json");}catch(...){return json::object();}}
json selectedRunning(const json& selection=settings){auto process=running();if(process.contains("outputs")){int screen=std::clamp(selection.value("screen",0),0,std::max(0,(int)monitors.size()-1));if(monitors.empty())return json::object();return process["outputs"].value(monitors[screen],json::object());}return process;}
void refreshRunning(){auto process=selectedRunning();activePath=owned(process)?process.value("path",std::string()):std::string();}
std::vector<std::string> renderArgs(const Item& item,const json& prefs,bool preview) {
    if(!item.reason.empty())throw std::runtime_error(item.reason);
    std::string assets;if(item.origin=="local"&&item.type!="scene"){auto empty=stateDir()/"local-assets";fs::create_directories(empty);assets=empty.string();}else assets=assetsPath();if(assets.empty())throw std::runtime_error("Assets di Wallpaper Engine non trovati nelle librerie Steam");
    auto profilesForRender=loadPlacements();auto profile=profilesForRender.value(item.path,json::object()).value("playback",json::object());
    auto enabled=[&](const char* local,const char* global){return !prefs.value(global,false)&&profile.value(local,true);};
    std::vector<std::string> args={executable(),"--render","--assets-dir",assets,"--fps",std::to_string(std::clamp(prefs.value("fps",60),15,120)),"--volume",enabled("audio","muted")?"100":"0"};
    if(!enabled("reactive","disable_audio_processing"))args.push_back("--no-audio-processing");
    if(!enabled("mouse","disable_mouse"))args.push_back("--disable-mouse");
    if(!enabled("parallax","disable_parallax"))args.push_back("--disable-parallax");
    if(!enabled("particles","disable_particles"))args.push_back("--disable-particles");
    auto position=wallpaperPosition(loadPlacements(),item.path);
    if(position["x"].get<double>()!=0||position["y"].get<double>()!=0||position.value("zoom",1.)!=1)args.insert(args.end(),{"--lab-position",position.dump()});
    args.insert(args.end(),{"--lab-profile-file",(configDir()/"wallpaper_profiles.json").string(),"--lab-live-file",(stateDir()/"placement-live.json").string(),"--lab-project",item.path,"--lab-feed-dir",feedDir().string()});
    auto profiles=loadPlacements();
    if(profiles.contains(item.path)&&profiles[item.path].contains("variant")){
        auto choice=profiles[item.path]["variant"];
        if(choice.is_object())for(auto& variant:item.variants)if(choice.value("property",std::string())==variant.property&&choice.value("value",std::string())==variant.value){args.insert(args.end(),{"--set-property",variant.property+"="+variant.value});break;}
    }
    auto overrides=profiles.contains(item.path)?profiles[item.path].value("properties",json::object()):json::object();
    for(auto& [key,value]:overrides.items())if(item.properties.contains(key)){
        try{auto checked=validateProperty(item.properties.at(key),value);args.insert(args.end(),{"--set-property",key+"="+propertyArgument(checked)});}catch(...){}
    }
    auto renderPath=item.source.is_null()?item.path:materializeProject(item.path,item.source).string();
    if(preview) {args.insert(args.end(),{"--scaling","fit","--clamp","border","--window","0x0x1100x620","--no-fullscreen-pause",renderPath});return args;}
    int pause=prefs.value("fullscreen_policy",1);if(pause==0)args.push_back("--no-fullscreen-pause");else if(pause==2)args.push_back("--fullscreen-pause-only-active");
    if(monitors.empty())throw std::runtime_error("Nessun monitor Hyprland rilevato");
    auto mode=prefs.value("mode",std::string("span"));
    if(mode=="span" && monitors.size()>1) {
        std::string joined;for(auto& m:monitors){if(!joined.empty())joined+=",";joined+=m;}
        args.insert(args.end(),{"--screen-span",joined,"--scaling","fill","--bg",renderPath});
        if(prefs.contains("display_layout")){
            auto geometry=renderLayout(displayCanvas(displayData,prefs),displayData);
            args.insert(args.end(),{"--lab-layout",geometry.dump()});
        }
    } else if(mode=="mirror") {
        for(auto&m:monitors)args.insert(args.end(),{"--screen-root",m,"--scaling","fill","--bg",renderPath});
    } else {
        int index=std::clamp(prefs.value("screen",0),0,(int)monitors.size()-1);
        args.insert(args.end(),{"--screen-root",monitors[index],"--scaling","fill","--bg",renderPath});
    }
    return args;
}
std::string launch(Item item,json prefs,bool preview,int direction=0) {
    pid_t pid=-1;fs::path ready,transitionGate;
    try {
        fs::create_directories(stateDir());
        ControllerLock lock;
        prefs=loadSettings();
        if(direction) {
            auto active=selectedRunning(prefs);item=items.at(nextCarouselItem(loadCarousel(),items,owned(active)?active.value("path",std::string()):prefs.value("path",std::string()),direction));
            if(owned(active)&&active.value("path",std::string())==item.path)return {};
        }
        auto args=renderArgs(item,prefs,preview);
        if(!preview&&prefs.value("transition_type",0)>0&&!prefs.value("reduce_motion",false)){
         fs::create_directories(feedDir());
         auto prior=running();json config=json::object();auto cache=stateDir()/"transitions";fs::create_directories(cache);for(auto& e:fs::directory_iterator(cache)){std::error_code ec;if(fs::file_time_type::clock::now()-fs::last_write_time(e.path(),ec)>std::chrono::minutes(1))fs::remove_all(e.path(),ec);}
         auto token=stateDir()/"transitions"/std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());fs::create_directories(token);
         auto mouse=capture({"hyprctl","cursorpos","-j"});json cursor=json::object();try{if(!mouse.empty())cursor=json::parse(mouse[0]);}catch(...){}
         double mirrorX=.5,mirrorY=.5;for(const auto& display:displayData){double x=cursor.value("x",display.x+display.logicalWidth/2),y=cursor.value("y",display.y+display.logicalHeight/2);if(x>=display.x&&x<display.x+display.logicalWidth&&y>=display.y&&y<display.y+display.logicalHeight){mirrorX=(x-display.x)/display.logicalWidth;mirrorY=(y-display.y)/display.logicalHeight;break;}}
         for(int n=0;n<(int)monitors.size();n++){
          if(prefs.value("mode",std::string())=="single"&&n!=prefs.value("screen",0))continue;
          auto old=prior.contains("outputs")?prior["outputs"].value(monitors[n],json::object()):prior;if(!owned(old)||old.value("path",std::string())==item.path)continue;
          writeJson(feedDir()/"transition-request.json",{{"path",old["path"]},{"output",monitors[n]},{"transition",true},{"token",token.filename().string()}});
          for(int tries=0;tries<20;tries++){std::this_thread::sleep_for(std::chrono::milliseconds(40));try{auto frame=feedDir()/("snapshot-"+token.filename().string()+"-"+outputKey(monitors[n])+".bin");if(!fs::exists(frame))continue;std::ifstream file(frame,std::ios::binary);uint32_t header[5]={};file.read((char*)header,20);if(!file||header[0]!=0x574c4631||header[1]!=old["pid"].get<uint32_t>()||!header[2]||!header[3]||uint64_t(header[2])*header[3]>40000000||fs::file_size(frame)!=20+uint64_t(header[2])*header[3]*3)continue;auto dest=token/(std::to_string(n)+".bin");fs::copy_file(frame,dest,fs::copy_options::overwrite_existing);auto& d=displayData[n];config[monitors[n]]={{"file",dest.string()},{"effect",prefs["transition_type"]},{"duration",prefs.value("transition_duration",700)/1000.f},{"direction",prefs.value("transition_direction",0)},{"curve",prefs.value("transition_curve",0)},{"zoom",prefs.value("transition_zoom",30)/100.f},{"x",prefs.value("mode",std::string())=="mirror"?mirrorX:std::clamp((cursor.value("x",d.x+d.logicalWidth/2)-d.x)/d.logicalWidth,0.,1.)},{"y",prefs.value("mode",std::string())=="mirror"?mirrorY:std::clamp((cursor.value("y",d.y+d.logicalHeight/2)-d.y)/d.logicalHeight,0.,1.)}};break;}catch(...){} }
         }
         std::error_code requestError;fs::remove(feedDir()/"transition-request.json",requestError);for(auto& entry:fs::directory_iterator(feedDir()))if(entry.path().filename().string().starts_with("snapshot-"+token.filename().string()+"-"))fs::remove(entry.path(),requestError);if(!config.empty()){transitionGate=token/"start.json";for(auto& entry:config)entry["start_file"]=transitionGate.string();args.insert(args.end(),{"--lab-transition",config.dump()});}
        }
        ready=stateDir()/("ready-"+std::to_string(getpid())+"-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        args.insert(args.begin()+2,{"--ready-file",ready.string()});
        std::vector<char*> av;for(auto&s:args)av.push_back(s.data());av.push_back(nullptr);
        posix_spawn_file_actions_t fa;posix_spawn_file_actions_init(&fa);
        auto log=(stateDir()/(preview?"preview.log":"renderer.log")).string();
        posix_spawn_file_actions_addopen(&fa,STDOUT_FILENO,log.c_str(),O_CREAT|O_WRONLY|O_TRUNC,0600);
        posix_spawn_file_actions_adddup2(&fa,STDOUT_FILENO,STDERR_FILENO);
        posix_spawn_file_actions_addopen(&fa,STDIN_FILENO,"/dev/null",O_RDONLY,0);
        posix_spawnattr_t attr;posix_spawnattr_init(&attr);posix_spawnattr_setflags(&attr,POSIX_SPAWN_SETSID);
        int err=posix_spawn(&pid,av[0],&fa,&attr,av.data(),environ);
        posix_spawn_file_actions_destroy(&fa);posix_spawnattr_destroy(&attr);
        if(err)throw std::runtime_error(std::string("Avvio fallito: ")+strerror(err));
        bool success=false;
        for(int n=0;n<600;++n) {
            int status;if(waitpid(pid,&status,WNOHANG)==pid)throw std::runtime_error("Scena non avviata. Dettagli in "+log);
            if(fs::exists(ready)){success=true;break;}
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if(!success)throw std::runtime_error("Timeout del renderer; wallpaper precedente conservato. Log: "+log);
        json process={{"pid",pid},{"start",startTime(pid)},{"path",item.path},{"title",item.title}};
        fs::remove(ready);
        if(preview) {stop(previewProcess);previewProcess=process;}
        else {
            auto previous=running();
            // Record the new owned process before stopping the old one, so failed writes preserve the old wallpaper.
            auto oldPrefs=loadSettings();
            json victim=previous;
            if(prefs.value("mode",std::string())=="single"&&!monitors.empty()){
                auto name=monitors.at(std::clamp(prefs.value("screen",0),0,(int)monitors.size()-1));json outputs=previous.value("outputs",json::object());
                if(previous.contains("outputs"))victim=outputs.value(name,json::object());
                outputs[name]=process;process["outputs"]=outputs;process["mode"]="single";
                prefs["monitor_wallpapers"][name]=item.path;
            }
            prefs["path"]=item.path;writeJson(configDir()/"settings.json",prefs);
            try{writeJson(stateDir()/"running.json",process);}
            catch(...){writeJson(configDir()/"settings.json",oldPrefs);throw;}
            stop(victim);
            if(!transitionGate.empty()){try{writeJson(transitionGate,{{"time_ns",std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()}});}catch(const std::exception& e){std::cerr<<"Transition start signal: "<<e.what()<<"\n";}if(prefs.value("caelestia_enabled",false))std::this_thread::sleep_for(std::chrono::milliseconds(prefs.value("transition_duration",700)));}
            syncCaelestia(item,prefs,true);
        }
        return {};
    }catch(const std::exception& e){
        if(pid>1)stop(json{{"pid",pid},{"start",startTime(pid)}});
        if(!ready.empty()){std::error_code ec;fs::remove(ready,ec);}
        return e.what();
    }
}
void init(bool popupOnly=false,bool probeVideos=false) {
    settings=loadSettings();placements=loadPlacements();refreshRunning();
    carousel=loadCarousel();shortcut=shortcutSettings();
    setLanguage(languageIndex(settings.value("language",std::string("it"))));resolveFonts(!popupOnly);
    auto result=capture({"hyprctl","monitors","-j"});
    if(!result.empty())try {
        auto rows=json::parse(result[0]);displayData=parseDisplays(rows);
        for(auto& d:displayData){
            monitors.push_back(d.name);monitorLabels.push_back(d.name+"  /  "+std::to_string(d.width)+" x "+std::to_string(d.height));
        }
        for(auto& m:rows)if(m.value("focused",false)&&!m.value("disabled",false)){
            double scale=std::max(0.25,m.value("scale",1.0));int width=m.value("width",1000),height=m.value("height",800);
            if(m.value("transform",0)%2)std::swap(width,height);
            popupMaxWidth=std::clamp(int(width/scale*0.8),1,800);popupMaxHeight=std::clamp(int(height/scale*0.8),1,640);
        }
        displayDraft=displayCanvas(displayData,settings);
    }catch(...){}
    refreshRunning();
    if(popupOnly) {
        std::vector<fs::path> projects;for(auto& path:carousel)projects.emplace_back(path.get<std::string>());if(!activePath.empty()&&std::none_of(projects.begin(),projects.end(),[&](const auto& p){return p==activePath;}))projects.emplace_back(activePath);
        items=scanProjects(projects);
    } else {
        items=scan(libraryRoots(),probeVideos);
    }
    resolutions=resolutionFilters(items);
}
json describeDisplays(){
    json outputs=json::array();
    for(size_t i=0;i<displayData.size();i++){
        auto& d=displayData[i];auto& r=displayDraft.rects[i];
        outputs.push_back({{"name",d.name},{"width",d.width},{"height",d.height},{"mm_width",d.mmWidth},{"mm_height",d.mmHeight},{"measured",d.measured},{"rect",json::array({r.x,r.y,r.w,r.h})}});
    }
    auto error=canvasError(displayDraft);
    return {{"basis",displayDraft.basis},{"saved",settings.contains("display_layout")},{"outputs",outputs},{"error",error},{"renderer",error.empty()?renderLayout(displayDraft,displayData):json()}};
}
template<class F> int guarded(F operation) {
    try {operation();errorMessage.clear();return 1;}catch(const std::exception& e){errorMessage=e.what();return 0;}
}
int savePreference(const std::string& key,const json& value) {
    return guarded([&]{ControllerLock lock;auto latest=loadSettings();latest[key]=value;writeJson(configDir()/"settings.json",latest);settings=latest;});
}
int find(const std::string& id) {for(size_t i=0;i<items.size();++i)if(items[i].id==id || items[i].path==id)return i;throw std::runtime_error("Wallpaper non trovato: "+id);}
}
extern "C" {
int lab_language_count(){return languageCount();}
const char* lab_language_name(int i){return languageName(i);}
const char* lab_font_family(){static std::string family;family=settings.value("font_family",std::string());if(family.empty())family=settings.value("ui_design",1)==2?"Oxanium":"Sans";return family.c_str();}
int lab_font_count(){return fontFamilies.size();}
const char* lab_font_name(int i){return fontFamilies.at(i).c_str();}
const char* lab_font_preview_path(int i){static std::map<std::string,std::string> cache;auto family=i?fontFamilies.at(i):"Adwaita Sans";auto name=family+":weight="+(settings.value("font_weight",1)==0?"medium":settings.value("font_weight",1)==2?"bold":"demibold");if(!cache.contains(name)){auto v=capture({"fc-match","-f","%{index}|%{file}",name});if(v.empty())cache[name]="";else{auto split=v[0].find('|');cache[name]="@"+v[0].substr(0,split)+":"+v[0].substr(split+1);}}return cache[name].c_str();}
int lab_font_selected(){auto it=std::find(fontFamilies.begin(),fontFamilies.end(),settings.value("font_family",std::string("Default")));return it==fontFamilies.end()?0:it-fontFamilies.begin();}
int lab_font_revision(){return fontRevision;}
int lab_set_font(int i){return guarded([&]{auto family=fontFamilies.at(i);if(!savePreference("font_family",i==0?"":family))throw std::runtime_error(errorMessage);resolveFonts();});}
const char* lab_path(int i){return items.at(i).path.c_str();}
int lab_files_busy(){return fileJob.valid();}
const char* lab_files_notice(){return lab_tr(fileNotice.c_str());}
const char* lab_library_path(int local){static std::string value;if(local)value=localLibrary(settings).string();else{value=settings.value("workshop_path",std::string());if(value.empty())for(auto p:steamLibraries())if(fs::is_directory(p/"steamapps/workshop/content/431960")){value=(p/"steamapps/workshop/content/431960").string();break;}}return value.c_str();}
int lab_files_start(int action){if(fileJob.valid()||pending.valid()||catalogScan.valid())return 0;fileNotice.clear();std::string title=lab_tr(action==3?"Sfondo dell’app":action==0?"Importa immagini o video":action==1?"Workshop Steam":"Sfondi locali");auto prefs=loadSettings();fileJob=std::async(std::launch::async,[action,title,prefs]{auto files=chooseLocalFiles(action==1||action==2,title,action==3);if(files.empty())return json{{"cancelled",true}};if(action==3){auto image=importAppBackground(files.front());ControllerLock lock;auto latest=loadSettings();latest["app_background"]=image.string();writeJson(configDir()/"settings.json",latest);return json{{"background",true}};}if(action!=0){auto directory=fs::canonical(files.front());if(!fs::is_directory(directory))throw std::runtime_error("Seleziona una cartella valida");ControllerLock lock;auto latest=loadSettings();latest[action==1?"workshop_path":"local_path"]=directory.string();writeJson(configDir()/"settings.json",latest);return json{{"changed",true}};}int count=0;for(auto& file:files){importLocalMedia(file,localLibrary(prefs));count++;}return json{{"imported",count}};});return 1;}
const char* lab_app_background(){static std::string path;path=settings.value("app_background",std::string());return path.c_str();}
int lab_clear_app_background(){return lab_set_option("app_background_preset",0);}
int lab_import_files(const char* const* paths,int count){if(fileJob.valid()||pending.valid()||catalogScan.valid()||count<1||count>1000)return 0;std::vector<fs::path> files;for(int n=0;n<count;n++)files.emplace_back(paths[n]);auto root=localLibrary(settings);fileJob=std::async(std::launch::async,[files,root]{for(auto& file:files)importLocalMedia(file,root);return json{{"imported",files.size()}};});return 1;}
int lab_files_poll(){if(!fileJob.valid()||fileJob.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return 0;try{auto result=fileJob.get();settings=loadSettings();if(result.value("cancelled",false))return 0;fileNotice="Modifiche applicate e salvate.";return result.value("background",false)?2:1;}catch(const std::exception& e){errorMessage=e.what();return -1;}}
int lab_caelestia_available(){return fs::is_regular_file(caelestiaHelper());}
const char* lab_caelestia_status(){static std::string result;try{auto status=readJson(stateDir()/"integration-status.json");result=status.value("ok",false)?lab_tr("Modifiche applicate e salvate."):status.value("error",std::string());}catch(...){result=lab_tr(settings.value("caelestia_enabled",false)?"Attivo":"Disattivo");}return result.c_str();}
void lab_cancel_switcher_preview(){stop(switcherDemoProcess);switcherDemoProcess=json::object();}
int lab_rescan_start(){return guarded([&]{if(pending.valid()||fileJob.valid()||catalogScan.valid()||stagedCatalogReady)throw std::runtime_error("Attendi la fine del caricamento");auto roots=libraryRoots();catalogScan=std::async(std::launch::async,[roots]{return scan(roots,true);});});}
int lab_rescan_ready(){
    if(!catalogScan.valid()||catalogScan.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return 0;
    try{auto next=catalogScan.get();auto filters=resolutionFilters(next);stagedItems=std::move(next);stagedResolutions=std::move(filters);stagedCatalogReady=true;return 1;}catch(const std::exception& e){errorMessage=e.what();return -1;}
}
void lab_rescan_commit(){stagedCatalogReady=false;items.swap(stagedItems);stagedItems.clear();resolutions.swap(stagedResolutions);stagedResolutions.clear();refreshRunning();}
int lab_count(){return std::min((int)items.size(),2048);}
const char* lab_title(int i){return items.at(i).title.c_str();}
const char* lab_preview(int i){return items.at(i).preview.c_str();}
const char* lab_origin(int i){return items.at(i).origin.c_str();}
const char* lab_type(int i){return items.at(i).type.c_str();}
const char* lab_error(){return lab_tr(errorMessage.c_str());}
int lab_current(){auto p=activePath;for(int i=0;i<lab_count();++i)if(items[i].path==p)return i;return -1;}
const char* lab_status(){
    auto mode=settings.value("mode",std::string("span"));
    statusMessage=lab_tr(mode=="span"?"Panoramico":mode=="mirror"?"Duplicato":"Singolo");
    if(mode=="single"&&!monitors.empty())statusMessage+=" / "+monitors[std::clamp(settings.value("screen",0),0,(int)monitors.size()-1)];
    else statusMessage+=" / "+std::to_string(monitors.size())+" monitor";
    statusMessage+="    "+std::to_string(settings.value("fps",60))+" FPS    "+lab_tr(settings.value("muted",true)?"Audio disattivato":"Audio attivo");return statusMessage.c_str();
}
void lab_cycle_mode(){auto mode=settings.value("mode",std::string("span"));savePreference("mode",mode=="span"?"mirror":mode=="mirror"?"single":"span");}
void lab_cycle_screen(){if(!monitors.empty())savePreference("screen",(settings.value("screen",0)+1)%(int)monitors.size());}
void lab_cycle_fps(){int fps=settings.value("fps",60);savePreference("fps",fps==30?60:fps==60?120:30);}
void lab_toggle_mute(){savePreference("muted",!settings.value("muted",true));}
int lab_start(int i,int preview){
    if(pending.valid()||catalogScan.valid()||i<0||i>=lab_count())return 0;
    if(!items[i].reason.empty()){errorMessage=items[i].reason;return 0;}
    errorMessage.clear();pendingPreview=preview;
    pending=std::async(std::launch::async,launch,items[i],settings,(bool)preview,0);return 1;
}
int lab_select(int i){
    if(i<0||i>=lab_count())return pending.valid()?1:0;
    if(!items[i].reason.empty()){errorMessage=items[i].reason;return pending.valid()?1:0;}
    if(pending.valid()){if(i!=deletingIndex)queuedSelection=i;return 1;}
    try{settings=loadSettings();refreshRunning();}catch(const std::exception& e){errorMessage=e.what();return 0;}if(i==lab_current())return 0;
    return lab_start(i,0);
}
int lab_poll(){
    if(integrationJob.valid()&&integrationJob.wait_for(std::chrono::seconds(0))==std::future_status::ready)try{integrationJob.get();}catch(...){}
    if(integrationDirty&&!integrationJob.valid())startIntegrationJob();
    if(!pending.valid()||pending.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return 0;
    errorMessage=pending.get();
    if(deletingIndex>=0){if(errorMessage.empty()){items[deletingIndex].reason="Sfondo spostato nel cestino";carousel=loadCarousel();deletedResult=deletingIndex;}deletingIndex=-1;}
    try{flushPlacements();}catch(const std::exception& e){errorMessage=e.what();}
    try{refreshRunning();}catch(const std::exception& e){errorMessage=e.what();}
    if(queuedSelection>=0){int next=queuedSelection;queuedSelection=-1;if(next!=lab_current())return lab_start(next,0)?0:-1;}
    if(!errorMessage.empty())return -1;
    try{settings=loadSettings();refreshRunning();}catch(const std::exception& e){errorMessage=e.what();}
    return pendingPreview?2:1;
}
const char* lab_font(){return fontPath.c_str();}
const char* lab_button_font(){return buttonFontPath.c_str();}
const char* lab_fallback_font(){return fallbackFontPath.c_str();}
const char* lab_brand_font(){return brandFontPath.c_str();}
int lab_matches(int i,int type,int resolution,int audio){
    auto& item=items.at(i);if(!matches(item,type,0,audio))return 0;
    for(auto& f:resolutions)if(f.code==resolution)return resolutionMatches(item,f);return resolution==0;
}
int lab_matches_grouped(int i,int type,int audio,const int* codes,int count,int groups,int origin){if(i<0||i>=lab_count()||count<0||count>64)return 0;return matchesCriteria(items[i],resolutions,type,audio,std::vector<int>(codes,codes+count),settings.value("filter_any",false),groups,origin);}
int lab_matches_all(int i,int type,int audio,const int* codes,int count){
 if(i<0||i>=lab_count()||count<0||count>64)return 0;
 return matchesCriteria(items[i],resolutions,type,audio,std::vector<int>(codes,codes+count),settings.value("filter_any",false));
}
int lab_resolution_group(int i){return resolutions.at(i).group;}
int lab_supported_count(int i){return items.at(i).supported.size();}
const char* lab_supported_resolutions(int i){static std::string label;label.clear();for(auto [w,h]:items.at(i).supported){if(!label.empty())label+=" · ";label+=std::to_string(w)+" × "+std::to_string(h);}return label.c_str();}
int lab_playback(int i,const char* key){return placements.value(items.at(i).path,json::object()).value("playback",json::object()).value(key,true);}
int lab_playback_set(int i,const char* key,int enabled){return guarded([&]{const std::set<std::string> keys={"audio","reactive","mouse","parallax","particles"};if(!keys.contains(key))throw std::runtime_error("Opzione non valida");ControllerLock lock;auto profiles=loadPlacements();profiles[items.at(i).path]["playback"][key]=bool(enabled);writeJson(configDir()/"wallpaper_profiles.json",profiles);placements=profiles;});}
float lab_adjust(int i,const char* key){std::string k=key;return placements.value(items.at(i).path,json::object()).value("image",json::object()).value(k,k=="contrast"||k=="saturation"||k=="gamma"?1.f:0.f);}
int lab_adjust_set(int i,const char* key,float value,int commit){return guarded([&]{std::string k=key;const std::map<std::string,std::pair<float,float>> bounds={{"brightness",{-1,1}},{"contrast",{0,2}},{"saturation",{0,2}},{"hue",{-180,180}},{"temperature",{-1,1}},{"gamma",{.25,3}},{"vignette",{0,1}},{"rotation",{-180,180}},{"flip_x",{0,1}},{"flip_y",{0,1}}};auto range=bounds.find(k);if(range==bounds.end())throw std::runtime_error("Regolazione non valida");if(!std::isfinite(value))throw std::runtime_error("Valore non valido");value=std::clamp(value,range->second.first,range->second.second);auto path=items.at(i).path;placements[path]["image"][k]=value;
 writeJson(stateDir()/"placement-live.json",{{"path",path},{"image",placements[path]["image"]}});
 if(commit){ControllerLock lock;auto latest=loadPlacements();latest[path]["image"]=placements[path]["image"];writeJson(configDir()/"wallpaper_profiles.json",latest);placements=latest;if(settings.value("caelestia_enabled",false)&&i==lab_current())integrationDirty=true;}
});}
int lab_adjust_reset(int i,int additional_only){return guarded([&]{
 ControllerLock lock;auto latest=loadPlacements();auto path=items.at(i).path;
 auto& image=latest[path]["image"];if(!image.is_object())image=json::object();
 if(additional_only){
  for(auto key:{"temperature","hue","gamma","vignette","rotation","flip_x","flip_y"})image.erase(key);
 }else image=json::object();
 // Commit one profile update, preserving crop, playback and creator properties.
 writeJson(configDir()/"wallpaper_profiles.json",latest);placements=latest;
 writeJson(stateDir()/"placement-live.json",{{"path",path},{"image",image}});
 if(settings.value("caelestia_enabled",false)&&i==lab_current())integrationDirty=true;
});}
static const json& propertyOverrides(int i){static const json empty=json::object();auto p=placements.find(items.at(i).path);return p!=placements.end()&&p->contains("properties")?p->at("properties"):empty;}
static std::string propertyKey(int i,int p){return items.at(i).propertyOrder.at(p);}
static const json& propertySchema(int i,int p){return items.at(i).properties.at(propertyKey(i,p));}
static json propertyCurrent(int i,int p){return propertyValue(items.at(i).properties,propertyOverrides(i),propertyKey(i,p));}
int lab_property_count(int i){return items.at(i).propertyOrder.size();}
const char* lab_property_type(int i,int p){static std::string s;s=propertySchema(i,p).value("type",std::string());return s.c_str();}
int lab_property_scheme(int i,int p){return propertyKey(i,p)=="schemecolor";}
const char* lab_property_label(int i,int p){static std::string s;if(propertyKey(i,p)=="schemecolor")return lab_tr("Colore principale del wallpaper");auto text=propertySchema(i,p).value("text",propertyKey(i,p));s=propertyLabel(text,propertyKey(i,p));return text.starts_with("ui_")?lab_tr(s.c_str()):s.c_str();}
int lab_property_visible(int i,int p){return propertyVisible(items.at(i).properties,propertyOverrides(i),propertySchema(i,p).value("condition",std::string()));}
double lab_property_number(int i,int p){auto v=propertyCurrent(i,p);return v.is_boolean()?v.get<bool>():v.is_number()?v.get<double>():0;}
double lab_property_bound(int i,int p,int upper){return propertySchema(i,p).value(upper?"max":"min",upper?100.:0.);}
const char* lab_property_text(int i,int p){static std::string s;s=propertyArgument(propertyCurrent(i,p));return s.c_str();}
int lab_property_options(int i,int p){return propertySchema(i,p).value("options",json::array()).size();}
const char* lab_property_option(int i,int p,int o){static std::string s;auto label=propertySchema(i,p).at("options").at(o).value("label",std::string());s=propertyLabel(label,"Opzione");return label.starts_with("ui_")?lab_tr(s.c_str()):s.c_str();}
int lab_property_choice(int i,int p){auto options=propertySchema(i,p).value("options",json::array());for(size_t n=0;n<options.size();n++)if(options[n].at("value")==propertyCurrent(i,p))return n;return -1;}
static int saveProjectProperty(int i,int p,const json& value){return guarded([&]{
    auto key=propertyKey(i,p);auto checked=validateProperty(propertySchema(i,p),value);ControllerLock lock;auto profiles=loadPlacements();
    writeJson(stateDir()/"backups"/("property-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()))/"wallpaper_profiles.json",profiles);
    profiles[items.at(i).path]["properties"][key]=checked;writeJson(configDir()/"wallpaper_profiles.json",profiles);placements=profiles;
});}
int lab_property_number_set(int i,int p,double v){return saveProjectProperty(i,p,propertySchema(i,p).value("type",std::string())=="bool"?json(v!=0):json(v));}
int lab_property_choice_set(int i,int p,int o){return saveProjectProperty(i,p,propertySchema(i,p).at("options").at(o).at("value"));}
int lab_property_text_set(int i,int p,const char* value){return saveProjectProperty(i,p,std::string(value));}
int lab_properties_reset(int i){return guarded([&]{ControllerLock lock;auto profiles=loadPlacements();writeJson(stateDir()/"backups"/("properties-reset-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()))/"wallpaper_profiles.json",profiles);profiles[items.at(i).path].erase("properties");writeJson(configDir()/"wallpaper_profiles.json",profiles);placements=profiles;});}
int lab_variant_count(int i){return items.at(i).variants.size();}
const char* lab_variant_label(int i,int v){return items.at(i).variants.at(v).label.c_str();}
int lab_variant_selected(int i){
    auto& item=items.at(i);if(!placements.contains(item.path)||!placements[item.path].contains("variant"))return -1;
    auto choice=placements[item.path]["variant"];if(!choice.is_object())return -1;
    for(size_t n=0;n<item.variants.size();n++)if(choice.value("property",std::string())==item.variants[n].property&&choice.value("value",std::string())==item.variants[n].value)return n;return -1;
}
int lab_variant_set(int i,int v){return guarded([&]{
    if(v< -1)throw std::runtime_error("Variante non valida");auto& item=items.at(i);ControllerLock lock;auto profiles=loadPlacements();if(!profiles[item.path].is_object())profiles[item.path]=json::object();
    if(v<0)profiles[item.path].erase("variant");else{auto& variant=item.variants.at(v);profiles[item.path]["variant"]={{"property",variant.property},{"value",variant.value}};}
    if(!profiles[item.path].contains("position"))profiles[item.path]["position"]={{"x",0},{"y",0}};
    if(fs::exists(configDir()/"wallpaper_profiles.json"))writeJson(stateDir()/"backups"/("variant-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()))/"wallpaper_profiles.json",loadPlacements());
    writeJson(configDir()/"wallpaper_profiles.json",profiles);placements=profiles;
});}
int lab_live_frame(int i,unsigned char* pixels,int capacity,int* width,int* height){
    static auto sent=std::chrono::steady_clock::time_point{};static int lastItem=-1;static uint32_t lastPid=0,lastSequence=0;
    if(i<0||i>=lab_count())return 0;
    auto current=selectedRunning();if(!owned(current)||current.value("path",std::string())!=items[i].path)return 0;
    auto now=std::chrono::steady_clock::now();
    try{
        if(lastItem!=i||now-sent>std::chrono::milliseconds(500)){
            auto screen=std::clamp(settings.value("screen",0),0,std::max(0,int(monitors.size())-1));
            writeJson(feedDir()/"request.json",{{"path",items[i].path},{"output",monitors.empty()?"":monitors[screen]}});sent=now;lastItem=i;
        }
        std::ifstream input(feedFrame(monitors.empty()?"":monitors[std::clamp(settings.value("screen",0),0,(int)monitors.size()-1)]),std::ios::binary);uint32_t header[5]={};input.read(reinterpret_cast<char*>(header),sizeof(header));
        if(!input||header[0]!=0x574c4631||header[1]!=current.at("pid").get<uint32_t>()||(header[1]==lastPid&&header[4]==lastSequence)||header[2]<1||header[2]>640||header[3]<1||header[3]>360)return 0;
        int bytes=header[2]*header[3]*3;if(bytes>capacity)return 0;input.read(reinterpret_cast<char*>(pixels),bytes);if(!input)return 0;
        lastPid=header[1];lastSequence=header[4];*width=header[2];*height=header[3];return 1;
    }catch(...){return 0;}
}
float lab_zoom(int i){return wallpaperPosition(placements,items.at(i).path).value("zoom",1.);}
int lab_placement_live(int i,float x,float y,float zoom){return guarded([&]{if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(zoom)||std::abs(x)>1||std::abs(y)>1||zoom<1||zoom>3)throw std::runtime_error("Posizione non valida");writeJson(stateDir()/"placement-live.json",{{"path",items.at(i).path},{"position",{{"x",x},{"y",y},{"zoom",zoom}}}});});}
int lab_zoom_set(int i,float zoom){return guarded([&]{if(!std::isfinite(zoom)||zoom<1||zoom>3)throw std::runtime_error("Zoom non valido");ControllerLock lock;auto profiles=loadPlacements();auto position=wallpaperPosition(profiles,items.at(i).path);position["zoom"]=zoom;writeJson(stateDir()/"backups"/("zoom-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()))/"wallpaper_profiles.json",profiles);profiles[items.at(i).path]["position"]=position;writeJson(configDir()/"wallpaper_profiles.json",profiles);placements=profiles;std::error_code ec;fs::remove(stateDir()/"placement-live.json",ec);});}
int lab_position_live(int i,float x,float y){return guarded([&]{
    if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x)>1||std::abs(y)>1)throw std::runtime_error("Posizione non valida");
    writeJson(stateDir()/"placement-live.json",{{"path",items.at(i).path},{"position",{{"x",x},{"y",y},{"zoom",lab_zoom(i)}}}});
});}
int lab_resolution_count(){return resolutions.size();}
int lab_resolution_code(int i){return resolutions.at(i).code;}
int lab_resolution_size(int i){return resolutions.at(i).count;}
const char* lab_resolution_label(int i){return lab_tr(resolutions.at(i).label.c_str());}
float lab_position_x(int i){auto path=items.at(i).path;return queuedPlacements.contains(path)?queuedPlacements[path]["x"].get<float>():wallpaperPosition(placements,path)["x"].get<float>();}
float lab_position_y(int i){auto path=items.at(i).path;return queuedPlacements.contains(path)?queuedPlacements[path]["y"].get<float>():wallpaperPosition(placements,path)["y"].get<float>();}
int lab_set_position(int i,float x,float y){return guarded([&]{if(!std::isfinite(x)||!std::isfinite(y)||std::abs(x)>1||std::abs(y)>1)throw std::runtime_error("Posizione non valida");if(pending.valid()){queuedPlacements[items.at(i).path]={{"x",x},{"y",y}};lab_position_live(i,x,y);return;}saveWallpaperPosition(items.at(i).path,x,y);placements=loadPlacements();std::error_code ec;fs::remove(stateDir()/"placement-live.json",ec);});}
int lab_width(int i){return items.at(i).width;}
int lab_height(int i){return items.at(i).height;}
int lab_audio(int i){return items.at(i).audio;}
const char* lab_reason(int i){return lab_tr(items.at(i).reason.c_str());}
int lab_favorite(int i){return i>=0&&i<lab_count()&&std::find(carousel.begin(),carousel.end(),json(items[i].path))!=carousel.end();}
int lab_carousel_count(){return carouselItems(carousel,items).size();}
int lab_carousel_item(int index){auto order=carouselItems(carousel,items);return index>=0&&index<(int)order.size()?order[index]:-1;}
int lab_favorite_change(int i,int direction){return guarded([&]{if(i<0||i>=lab_count())return;if(!direction&&!lab_favorite(i)&&!items[i].reason.empty())throw std::runtime_error(items[i].reason);changeCarousel(items.at(i).path,direction);carousel=loadCarousel();});}
int lab_next(int direction){
    if(pending.valid())return 0;
    errorMessage.clear();pendingPreview=false;
    pending=std::async(std::launch::async,launch,Item{},settings,false,direction<0?-1:1);return 1;
}
void lab_refresh(){
    if(pending.valid())return;
    try{settings=loadSettings();setLanguage(languageIndex(settings.value("language",std::string("it"))));if(appliedFamily!=settings.value("font_family",std::string())||appliedWeight!=settings.value("font_weight",1))resolveFonts();placements=loadPlacements();carousel=loadCarousel();shortcut=shortcutSettings();refreshRunning();if(!displayDirty)displayDraft=displayCanvas(displayData,settings);}catch(const std::exception& e){errorMessage=e.what();}
}
static json workshopView;
void lab_ws_tick(){workshopView=workshop::snapshot();}
int lab_ws_installed(){return workshop::takeInstalled();}
int lab_ws_author_count(){return workshopView.contains("authors")?workshopView["authors"].size():0;}
const char* lab_ws_author(int i,int id){static std::string value;try{value=workshopView.at("authors").at(i).at(id?"id":"name");}catch(...){value="";}return value.c_str();}
int lab_ws_count(){return workshopView.contains("items")?workshopView["items"].size():0;}
const char* lab_ws_text(int i,const char* key){static std::string value;try{auto& node=i<0?workshopView:workshopView.at("items").at(i);auto v=node.value(key,json());if(v.is_string())value=v.get<std::string>();else if(v.is_array()){value.clear();for(auto& tag:v){if(!value.empty())value+=" · ";value+=tag.get<std::string>();}}else value="";}catch(...){value="";}return value.c_str();}
double lab_ws_number(int i,const char* key){try{auto& node=i<0?workshopView:workshopView.at("items").at(i);auto v=node.value(key,json(0));return v.is_boolean()?v.get<bool>():v.is_number()?v.get<double>():0;}catch(...){return 0;}}
const char* lab_ws_download_text(int i,const char* key){static std::string value;try{value=workshopView.at("downloads").at(workshopView["items"].at(i).at("id").get<std::string>()).value(key,std::string());}catch(...){value="";}return value.c_str();}
double lab_ws_download_progress(int i){try{auto d=workshopView.at("downloads").at(workshopView["items"].at(i).at("id").get<std::string>());return d.value("total",0.)>0?d.value("current",0.)/d.value("total",1.):0;}catch(...){return 0;}}
void lab_ws_search(const char* text,const char* author,const char* tags,int any,int sort,int page,int groupMask){json list=json::array();std::istringstream input(tags);std::string tag;while(std::getline(input,tag,'|'))if(!tag.empty())list.push_back(tag);json groupTags=json::array();if(!any)for(int g=0;g<5;g++)if(groupMask&(1<<g)){json grouped=json::array();for(auto it=list.begin();it!=list.end();){std::string t=*it;int kind=t.starts_with("Dual")?1:t.starts_with("Ultrawide")?2:t.starts_with("Triple")?3:t.starts_with("Portrait")?4:0;if(t.find(" x ")!=std::string::npos&&kind==g){grouped.push_back(*it);it=list.erase(it);}else ++it;}if(!grouped.empty())groupTags.push_back(grouped);}
workshop::search({{"tag_groups",groupTags},{"text",text},{"author",author},{"tags",list},{"any",bool(any)},{"sort",sort},{"page",page},{"page_size",settings.value("page_size",100)},{"mature",settings.value("workshop_mature",false)}});}
void lab_ws_open(int i,int author){guarded([&]{auto item=workshopView.at("items").at(i);std::string url=author?"https://steamcommunity.com/profiles/"+item.at("owner").get<std::string>():"https://steamcommunity.com/sharedfiles/filedetails/?id="+item.at("id").get<std::string>();capture({"xdg-open",url});});}
int lab_ws_download(int i){return guarded([&]{workshop::download(workshopView.at("items").at(i).at("id"));});}
int lab_deleted(){int result=deletedResult;deletedResult=-1;return result;}
int lab_delete(int i){
 if(i<0||i>=lab_count()||pending.valid()||catalogScan.valid())return 0;
 refreshRunning();auto target=items[i];Item replacement;bool active=target.path==activePath;std::vector<int> affected;auto all=running();if(all.contains("outputs"))for(int n=0;n<(int)monitors.size();n++){auto child=all["outputs"].value(monitors[n],json::object());if(owned(child)&&child.value("path",std::string())==target.path){active=true;affected.push_back(n);}}
 if(active){auto order=carouselItems(loadCarousel(),items);for(int n=0;n<lab_count();n++)if(std::find(order.begin(),order.end(),n)==order.end())order.push_back(n);
  for(int n:order)if(n!=i&&items[n].reason.empty()&&fs::is_regular_file(fs::path(items[n].path)/"project.json")){replacement=items[n];break;}}
 if(owned(previewProcess)&&previewProcess.value("path",std::string())==target.path){stop(previewProcess);previewProcess=json();}
 deletingIndex=i;queuedSelection=-1;pendingPreview=false;errorMessage.clear();
 pending=std::async(std::launch::async,[target,replacement,active,affected,prefs=settings](){
  bool stoppedLast=false;
  if(active&&!replacement.path.empty()){
   if(affected.empty()){auto error=launch(replacement,prefs,false);if(!error.empty())return error;}
   else {int selected=prefs.value("screen",0);for(int screen:affected){savePreference("screen",screen);auto error=launch(replacement,prefs,false);if(!error.empty()){savePreference("screen",selected);return error;}}savePreference("screen",selected);}
  }
  try{ControllerLock lock;auto current=running();if(current.contains("outputs")){auto outputs=current["outputs"];for(auto it=outputs.begin();it!=outputs.end();){if(it->value("path",std::string())==target.path){stop(*it);it=outputs.erase(it);}else ++it;}if(outputs.empty()){current=json::object();fs::remove(stateDir()/"running.json");}else{current=outputs.begin().value();current["outputs"]=outputs;writeJson(stateDir()/"running.json",current);}}if(owned(current)&&current.value("path",std::string())==target.path){
    if(!replacement.path.empty())throw std::runtime_error("Sfondo ancora attivo: eliminazione annullata");
    stop(current);stoppedLast=true;fs::remove(stateDir()/"running.json");auto latest=loadSettings();latest.erase("path");writeJson(configDir()/"settings.json",latest);
   }
   trashProject(target.path);auto list=loadCarousel();list.erase(std::remove(list.begin(),list.end(),json(target.path)),list.end());writeJson(configDir()/"carousel.json",list);return std::string();
  }catch(const std::exception& e){std::string error=e.what();if(stoppedLast&&fs::is_regular_file(fs::path(target.path)/"project.json")){auto restored=launch(target,prefs,false);if(!restored.empty())error+=" · Ripristino fallito: "+restored;}return error;}
 });return 1;
}
int lab_display_rect(int i,float* x,float* y,float* w,float* h){
    if(i<0||i>=(int)displayDraft.rects.size())return 0;
    auto& r=displayDraft.rects[i];*x=r.x;*y=r.y;*w=r.w;*h=r.h;return 1;
}
const char* lab_monitor_name(int i){return displayData.at(i).name.c_str();}
const char* lab_display_info(int i){
    auto& d=displayData.at(i);std::ostringstream out;
    out<<d.width<<" × "<<d.height<<" px  ·  "<<std::fixed<<std::setprecision(1)<<d.mmWidth/10<<" × "<<d.mmHeight/10<<" cm";
    if(!d.measured)out<<" "<<lab_tr("(stimate)");displayInfo=out.str();return displayInfo.c_str();
}
int lab_display_physical(){return displayDraft.basis=="physical";}
int lab_display_dirty(){return displayDirty;}
int lab_display_active(){return settings.contains("display_layout");}
const char* lab_display_error(){displayMessage=canvasError(displayDraft);return lab_tr(displayMessage.c_str());}
void lab_display_move(int i,float x,float y,float snap){moveDisplay(displayDraft,i,x,y,snap);displayDirty=true;}
void lab_display_align(int edge){alignDisplays(displayDraft,edge);displayDirty=true;}
void lab_display_basis(int physical){displayDraft=automaticCanvas(displayData,physical?"physical":"pixels");displayDirty=true;}
void lab_display_revert(){displayDraft=displayCanvas(displayData,settings);displayDirty=false;}
int lab_display_save(){
    return guarded([&]{
        ControllerLock lock;renderLayout(displayDraft,displayData);
        auto latest=loadSettings();auto backup=stateDir()/"backups"/("display-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
        writeJson(backup/"settings.json",latest);
        latest["display_layout"]=canvasSettings(displayDraft);writeJson(configDir()/"settings.json",latest);settings=latest;displayDirty=false;
    });
}
int lab_monitor_count(){return monitors.size();}
const char* lab_monitor(int i){return monitorLabels.at(i).c_str();}
int lab_mode(){auto m=settings.value("mode",std::string("span"));return m=="span"?0:m=="mirror"?1:2;}
int lab_screen(){return settings.value("screen",0);}
int lab_option(const char* key){if(std::string(key)=="app_background_preset")return settings.value(settings.value("ui_design",1)==2?"cyber_background":"strawberry_background",0);if(std::string(key)=="light_theme")return settings.value("ui_design",1)==1;if(std::string(key)=="language")return currentLanguage();if(std::string(key)=="ui_theme")return settings.value("ui_theme",0);auto it=settings.find(key);if(it==settings.end())return 0;return it->is_boolean()?it->get<bool>():it->is_number_integer()?it->get<int>():0;}
int lab_set_mode(int mode){return savePreference("mode",mode==0?"span":mode==1?"mirror":"single");}
int lab_set_screen(int screen){if(screen<0||screen>=(int)monitors.size())return 0;int ok=savePreference("screen",screen);refreshRunning();return ok;}
int lab_set_option(const char* key,int value){
    std::string k=key;
    if(k=="language"){if(value<0||value>=languageCount())return 0;int ok=savePreference(k,languageCode(value));if(ok)setLanguage(value);return ok;}
    if(k=="ui_theme"||k=="ui_design")return guarded([&]{
        ControllerLock lock;auto latest=loadSettings();int old=latest.value("ui_design",1);
        if(k=="ui_design"){
            int design=std::clamp(value,1,2);
            if(design!=old){
                latest[old==1?"strawberry_palette":"cyber_palette"]=latest["ui_theme"];
                latest[old==1?"strawberry_light":"cyber_light"]=latest["light_theme"];
                latest["ui_theme"]=LabPaletteForDesign(latest.value(design==1?"strawberry_palette":"cyber_palette",LabDefaultPalette(design)),design);
                latest["light_theme"]=design==1;
                latest[k]=design;
            }
        }else latest[k]=LabPaletteForDesign(value,old);
        writeJson(configDir()/"settings.json",latest);settings=latest;if(k=="ui_design")resolveFonts();
    });
    if(k=="app_background_preset")return guarded([&]{ControllerLock lock;auto latest=loadSettings();latest[latest.value("ui_design",1)==2?"cyber_background":"strawberry_background"]=std::clamp(value,0,3);latest["app_background"]="";writeJson(configDir()/"settings.json",latest);settings=latest;});
    if(k=="layout_density")return savePreference(k,std::clamp(value,0,2));
    if(k=="carousel_layout")return savePreference(k,std::clamp(value,0,3));
    if(k=="window_opacity"||k=="window_transparency")return guarded([&]{ControllerLock lock;auto latest=loadSettings();int t=std::clamp(k=="window_opacity"?100-value:value,0,30);latest["window_transparency"]=t;latest["window_opacity"]=100-t;writeJson(configDir()/"settings.json",latest);settings=latest;});
    if(k=="caelestia_enabled"){int ok=guarded([&]{ControllerLock lock;auto latest=loadSettings();latest[k]=bool(value);writeJson(configDir()/"settings.json",latest);settings=latest;if(!value)updateCaelestia("",false);});integrationDirty=ok&&value;if(integrationDirty&&!integrationJob.valid())startIntegrationJob();return ok;}
    const std::map<std::string,std::pair<int,int>> animationRanges={{"switcher_size",{70,140}},{"switcher_zoom",{105,150}},{"switcher_fade_in",{60,600}},{"switcher_fade_out",{60,600}},{"switcher_motion",{100,900}},{"switcher_reverse",{0,1}},{"wave_speed",{0,300}},{"wave_height",{0,40}},{"transition_duration",{100,2500}},{"transition_direction",{0,3}},{"transition_curve",{0,2}},{"transition_zoom",{5,60}},{"caelestia_palette",{0,3}},{"caelestia_mode",{0,2}}};
    if(auto range=animationRanges.find(k);range!=animationRanges.end()){int ok=savePreference(k,std::clamp(value,range->second.first,range->second.second));if(ok&&k.starts_with("caelestia_")&&settings.value("caelestia_enabled",false)){integrationDirty=true;if(!integrationJob.valid())startIntegrationJob();}return ok;}
    if(k=="page_size")return savePreference(k,value<=50?50:value<=100?100:200);
    if(k=="transition_type")return savePreference(k,std::clamp(value,0,8));
    if(k=="fullscreen_policy")return savePreference(k,std::clamp(value,0,2));
    if(k=="filter_width")return savePreference(k,std::clamp(value,220,600));
    if(k=="filter_any"||k=="reduce_motion"||k=="workshop_mature")return savePreference(k,(bool)value);
    if(k=="font_weight"){int ok=savePreference(k,std::clamp(value,0,2));if(ok)resolveFonts();return ok;}
    if(k=="inspector_width")return savePreference(k,std::clamp(value,280,1600));
    if(k=="fps")return savePreference(k,std::clamp(value,15,120));
    if(k=="muted"||k=="disable_audio_processing"||k=="disable_mouse"||k=="disable_parallax"||k=="disable_particles")return savePreference(k,(bool)value);
    return 0;
}
void startIntegrationJob(){integrationDirty=false;refreshRunning();int index=lab_current();auto item=index>=0?items[index]:Item{};auto prefs=settings;integrationJob=std::async(std::launch::async,[item,prefs]{try{prepareCaelestia();if(!item.path.empty())syncCaelestia(item,prefs);else{std::lock_guard lock(integrationStatusMutex);writeJson(stateDir()/"integration-status.json",{{"ok",true}});}}catch(const std::exception& e){std::lock_guard lock(integrationStatusMutex);writeJson(stateDir()/"integration-status.json",{{"ok",false},{"error",e.what()}});}});}
const char* lab_shortcut(){shortcutLabel=shortcut.value("combo",std::string("SUPER+Tab"));return shortcutLabel.c_str();}
int lab_shortcut_enabled(){return shortcut.value("enabled",false);}
int lab_record_shortcut(int enabled){
    return guarded([&]{
        if(!enabled){
            if(shortcutCapture){
                auto result=capture({"hyprctl","reload","config-only"});
                if(result.empty()||result[0].find("ok")==std::string::npos)throw std::runtime_error("Impossibile ripristinare lo shortcut. Ricarica la configurazione Hyprland.");
                shortcutCapture=false;
            }
            return;
        }
        if(shortcutCapture||!lab_shortcut_enabled())return;
        auto current=parseShortcut(lab_shortcut());auto result=capture({"hyprctl","binds","-j"});
        if(result.empty())throw std::runtime_error("Impossibile iniziare la registrazione dello shortcut");
        bool own=false;for(auto& bind:json::parse(result[0]))if(bind.value("description",std::string())=="Wallpaper Lab carousel"&&bind.value("modmask",0)==current.mask&&bind.value("key",std::string())==current.key)own=true;
        if(!own)return;
        // Suspend only our own primary binding in memory, so the current
        // shortcut can itself be recorded. Cancel/close restores the config.
        result=capture({"hyprctl","eval","hl.unbind("+json(current.combo).dump()+")"});
        if(result.empty()||result[0].find("ok")==std::string::npos)throw std::runtime_error("Registrazione dello shortcut non disponibile");
        shortcutCapture=true;
    });
}
int lab_set_shortcut(const char* combo,int enabled){return guarded([&]{configureShortcut(combo,enabled);shortcut=shortcutSettings();});}
int lab_switcher_preview_active(){
    if(switcherDemoProcess.contains("pid")){int status;if(waitpid(switcherDemoProcess["pid"].get<int>(),&status,WNOHANG)>0)switcherDemoProcess=json();}
    return owned(switcherDemoProcess);
}
int lab_preview_switcher(){return guarded([&]{
    if(lab_switcher_preview_active())return;
    if(switcherDemoProcess.contains("pid")){int status;waitpid(switcherDemoProcess["pid"].get<int>(),&status,WNOHANG);}
    auto exe=executable();char* args[]={exe.data(),const_cast<char*>("--switcher-preview"),nullptr};pid_t child;
    posix_spawnattr_t attr;posix_spawnattr_init(&attr);posix_spawnattr_setflags(&attr,POSIX_SPAWN_SETSID);
    int error=posix_spawn(&child,exe.c_str(),nullptr,&attr,args,environ);posix_spawnattr_destroy(&attr);
    if(error)throw std::runtime_error("Impossibile aprire l'anteprima dello switcher");
    switcherDemoProcess={{"pid",child},{"start",startTime(child)}};
});}
int lab_popup_poll(int* steps,int* released){
    try {
        if(switcherDemo){
            double t=std::chrono::duration<double>(std::chrono::steady_clock::now()-demoStarted).count();
            *released=0;*steps=1+int(std::max(0.0,t-0.6)/0.8);return t>4.6?-1:0;
        }
        auto state=popupState();
        if(state.empty()||state.value("done",false))return -1;
        bool reset=state.value("epoch",std::string())!=popupSnapshot.value("epoch",std::string())||state.value("session",0LL)!=popupSnapshot.value("session",0LL);
        popupSnapshot=state;*steps=state.value("steps",0);*released=state.value("commit",false);return reset?1:0;
    }catch(const std::exception& e){errorMessage=e.what();return -1;}
}
int lab_popup_selection(int count,int current,int steps){return popupSelection(count,current,steps);}
int lab_popup_max_width(){return popupMaxWidth;}
int lab_popup_max_height(){return popupMaxHeight;}
void lab_shutdown(){if(integrationJob.valid())try{integrationJob.get();}catch(...){}if(fileJob.valid())try{fileJob.get();}catch(...){}workshop::shutdown();lab_record_shortcut(0);if(catalogScan.valid())try{catalogScan.get();}catch(...){}while(pending.valid()){lab_poll();if(pending.valid())std::this_thread::sleep_for(std::chrono::milliseconds(20));}try{flushPlacements();}catch(...){}stop(previewProcess);stop(switcherDemoProcess);if(switcherDemoProcess.contains("pid")){int status;waitpid(switcherDemoProcess["pid"].get<int>(),&status,WNOHANG);}}
}
int main(int argc,char**argv) {
    try {
        if(argc>1&&std::string(argv[1])=="--version"){std::cout<<"Wallpaper Lab "<<WALLPAPER_LAB_VERSION<<'\n';return 0;}
        for(int i=1;i<argc;++i)if(std::string(argv[i]).starts_with("--type="))return engineMain(argc,argv,"");
        if(argc>1&&std::string(argv[1])=="--render") {
            std::string ready;std::vector<char*> av={argv[0]};
            for(int i=2;i<argc;++i){if(std::string(argv[i])=="--ready-file"&&i+1<argc)ready=argv[++i];else av.push_back(argv[i]);}
            int count=av.size();av.push_back(nullptr);return engineMain(count,av.data(),ready);
        }
        if(argc>1&&std::string(argv[1])=="--workshop-download"){
            if(argc!=3)throw std::runtime_error("Uso: --workshop-download ID");workshop::download(argv[2]);
            auto deadline=std::chrono::steady_clock::now()+std::chrono::minutes(5);json result;
            while(std::chrono::steady_clock::now()<deadline){result=workshop::snapshot();auto d=result["downloads"][argv[2]];if(d.value("status",std::string())=="installed"||d.value("status",std::string())=="error"||!result.value("error",std::string()).empty())break;std::this_thread::sleep_for(std::chrono::milliseconds(200));}
            std::cout<<result.dump(2)<<'\n';workshop::shutdown();return result["downloads"][argv[2]].value("status",std::string())=="installed"?0:1;
        }
        if(argc>1&&std::string(argv[1])=="--workshop-search"){
            json query=argc>2?json::parse(argv[2]):json::object();workshop::search(query);
            while(workshop::snapshot().value("busy",false))std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto result=workshop::snapshot();std::cout<<result.dump(2)<<'\n';workshop::shutdown();return result.value("error",std::string()).empty()?0:1;
        }
        if(argc>1&&std::string(argv[1])=="--help") {
            std::cout<<"Wallpaper Lab - prototype\nUsage: wallpaper-lab [--list | --status | --restore | --stop | --plan ID | --apply ID | --preview ID | --next | --previous | --carousel | --import FILE... | --library-path local|steam DIRECTORY | --favorite ID | --shortcut SUPER+Tab | --shortcut-off | --display-layout | --display-layout-save physical|pixels top|center|bottom|detected]\nLauncher: click a card to apply it and open the live inspector; drag its divider to resize it. Carousel layout is configured in its own section.\n";return 0;
        }
        if(argc>1&&std::string(argv[1])=="--switcher-preview"){
            init(true);switcherDemo=true;demoStarted=std::chrono::steady_clock::now();carousel_popup_main();return 0;
        }
        if(argc>1&&(std::string(argv[1])=="--carousel-event"||std::string(argv[1])=="--carousel-resume")) {
            PopupHost host;
            bool own=false;
            if(std::string(argv[1])=="--carousel-event") {
                if(argc!=6)throw std::runtime_error("Uso: --carousel-event EPOCH SESSION STEPS step|commit");
                // Action is the final field; token and counters never reach a shell.
                std::string action=argv[5];if(action!="step"&&action!="commit")throw std::runtime_error("Azione popup non valida");
                own=host.receive(argv[2],std::stoll(argv[3]),std::stoi(argv[4]),action=="commit");
            }else own=host.resume();
            if(!own)return 0;
            popupSnapshot=popupState();
            int choice=-1;
            try{init(true);choice=carousel_popup_main();}
            catch(...){host.finish(popupSnapshot);throw;}
            bool reopen=host.finish(popupSnapshot);
            if(reopen) {
                auto exe=executable();char* av[]={exe.data(),const_cast<char*>("--carousel-resume"),nullptr};pid_t child;
                int err=posix_spawn(&child,exe.c_str(),nullptr,nullptr,av,environ);
                if(err)throw std::runtime_error("Impossibile riaprire il popup");
            }
            // The window is already gone. Load only the confirmed scene.
            if(choice>=0&&choice<lab_count()&&items[choice].path!=activePath) {
                auto error=launch(items[choice],settings,false);
                if(!error.empty()){capture({"notify-send","-a","Wallpaper Lab","Carousel",error});throw std::runtime_error(error);}
            }
            return 0;
        }
        init(false,argc==1||(argc>1&&(std::string(argv[1])=="--ui-test"||std::string(argv[1])=="--display-settings"||std::string(argv[1])=="--appearance-settings"||std::string(argv[1])=="--language-settings"||std::string(argv[1])=="--resolutions")));
        if(argc>1) {
            std::string cmd=argv[1];
            if(cmd=="--import"){if(argc<3)throw std::runtime_error("Uso: --import FILE...");json imported=json::array();for(int n=2;n<argc;n++)imported.push_back(importLocalMedia(argv[n],localLibrary(settings)).string());std::cout<<imported.dump(2)<<'\n';return 0;}
            if(cmd=="--library-path"){if(argc!=4||(std::string(argv[2])!="local"&&std::string(argv[2])!="steam"))throw std::runtime_error("Uso: --library-path local|steam CARTELLA");auto directory=fs::canonical(argv[3]);if(!fs::is_directory(directory))throw std::runtime_error("Seleziona una cartella valida");if(!savePreference(std::string(argv[2])=="local"?"local_path":"workshop_path",directory.string()))throw std::runtime_error(errorMessage);return 0;}
            if(cmd=="--benchmark-properties"){
                if(argc!=3)throw std::runtime_error("Indica un wallpaper");int i=find(argv[2]);auto start=std::chrono::steady_clock::now();double checksum=0;int frames=120;
                for(int f=0;f<frames;f++)for(int p=0;p<lab_property_count(i);p++){
                    checksum+=lab_property_visible(i,p);checksum+=strlen(lab_property_type(i,p))+strlen(lab_property_label(i,p));
                    checksum+=strlen(lab_property_text(i,p));checksum+=lab_property_bound(i,p,0);checksum+=lab_property_choice(i,p)+1;
                }
                auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/frames;
                std::cout<<json({{"properties",lab_property_count(i)},{"milliseconds_per_frame",ms},{"frames",frames},{"checksum",checksum}}).dump()<<'\n';return 0;
            }
            if(cmd=="--properties"){
                if(argc!=3&&argc!=5)throw std::runtime_error("Uso: --properties ID [CHIAVE VALORE_JSON]");
                int item=find(argv[2]);if(argc==5){auto keys=propertyKeys(items.at(item).properties);auto key=std::find(keys.begin(),keys.end(),argv[3]);if(key==keys.end())throw std::runtime_error("Proprieta sconosciuta");if(!saveProjectProperty(item,key-keys.begin(),json::parse(argv[4])))throw std::runtime_error(errorMessage);}
                json out=json::array();for(auto& key:propertyKeys(items.at(item).properties)){auto p=items.at(item).properties.at(key);p["key"]=key;p["value"]=propertyValue(items.at(item).properties,propertyOverrides(item),key);out.push_back(p);}std::cout<<out.dump(2)<<'\n';return 0;
            }
            if(cmd=="--variant"){
                if(argc<3||argc>4)throw std::runtime_error("Uso: --variant ID [INDICE oppure -1 per originale]");
                int item=find(argv[2]);if(argc==4&&!lab_variant_set(item,std::stoi(argv[3])))throw std::runtime_error(errorMessage);
                std::cout<<json({{"selected",lab_variant_selected(item)},{"count",lab_variant_count(item)}}).dump()<<'\n';return 0;
            }
            if(cmd=="--ui-option"){
                if(argc!=4)throw std::runtime_error("Uso: --ui-option CHIAVE VALORE");
                int value=std::stoi(argv[3]);if(!(std::string(argv[2])=="font"?lab_set_font(value):lab_set_option(argv[2],value)))throw std::runtime_error("Opzione non valida");if(integrationJob.valid())integrationJob.get();std::cout<<settings.dump(2)<<'\n';return 0;
            }
            if(cmd=="--ui-preferences"){json out={{"settings",settings},{"fonts",fontFamilies},{"font_selected",lab_font_selected()}};std::cout<<out.dump(2)<<'\n';return 0;}
            if(cmd=="--resolutions"){json out=json::array();for(auto& f:resolutions)out.push_back({{"code",f.code},{"label",f.label},{"count",f.count},{"group",f.group}});std::cout<<out.dump(2)<<'\n';return 0;}
            if(cmd=="--position"){
                if(argc!=3&&argc!=5)throw std::runtime_error("Uso: --position ID [X Y] con valori tra -1 e 1");
                int item=find(argv[2]);if(argc==5&&!lab_set_position(item,std::stof(argv[3]),std::stof(argv[4])))throw std::runtime_error(errorMessage);
                std::cout<<wallpaperPosition(loadPlacements(),items[item].path).dump()<<'\n';return 0;
            }
            if(cmd=="--display-layout"){std::cout<<describeDisplays().dump(2)<<'\n';return 0;}
            if(cmd=="--display-layout-save"){
                if(argc!=4)throw std::runtime_error("Uso: --display-layout-save physical|pixels top|center|bottom|detected");
                std::string basis=argv[2],edge=argv[3];
                if(basis!="physical"&&basis!="pixels")throw std::runtime_error("Base della disposizione non valida");
                displayDraft=automaticCanvas(displayData,basis);
                if(edge=="top")alignDisplays(displayDraft,0);else if(edge=="center")alignDisplays(displayDraft,1);else if(edge=="bottom")alignDisplays(displayDraft,2);else if(edge!="detected")throw std::runtime_error("Allineamento non valido");
                if(!lab_display_save())throw std::runtime_error(errorMessage);
                std::cout<<describeDisplays().dump(2)<<'\n';return 0;
            }
            if(cmd=="--carousel"){std::cout<<loadCarousel().dump(2)<<'\n';return 0;}
            if(cmd=="--delete") {if(argc!=3||!lab_delete(find(argv[2])))throw std::runtime_error("Eliminazione non avviata");while(pending.valid()){lab_poll();std::this_thread::sleep_for(std::chrono::milliseconds(50));}if(!errorMessage.empty())throw std::runtime_error(errorMessage);std::cout<<"Sfondo spostato nel cestino\n";return 0;}
            if(cmd=="--favorite") {if(argc<3)throw std::runtime_error("Indica un wallpaper");int i=find(argv[2]);if(!lab_favorite_change(i,0))throw std::runtime_error(errorMessage);return 0;}
            if(cmd=="--shortcut"||cmd=="--shortcut-off") {configureShortcut(argc>2?argv[2]:shortcut.value("combo",std::string("SUPER+Tab")),cmd=="--shortcut");return 0;}
            if(cmd=="--next"||cmd=="--previous") {auto error=launch(Item{},settings,false,cmd=="--next"?1:-1);if(!error.empty())throw std::runtime_error(error);return 0;}
            if(cmd=="--list") {json out=json::array();for(auto&i:items)out.push_back({{"id",i.id},{"title",i.title},{"type",i.type},{"origin",i.origin},{"path",i.path},{"preview",i.preview},{"reason",i.reason},{"width",i.width},{"height",i.height},{"audio_reactive",i.audio},{"supported_resolutions",i.supported},{"resolution_variants",i.variants.size()}});std::cout<<out.dump(2)<<'\n';return 0;}
            if(cmd=="--status"){auto p=running();bool alive=owned(p);if(p.contains("outputs"))for(auto& [name,child]:p["outputs"].items())alive=alive||owned(child);p["alive"]=alive;std::cout<<p.dump(2)<<'\n';return 0;}
            if(cmd=="--stop"){
                fs::create_directories(stateDir());
                ControllerLock lock;
                stop(running());fs::remove(stateDir()/"running.json");return 0;
            }
            if(cmd=="--restore" || cmd=="--apply" || cmd=="--preview" || cmd=="--plan") {
                if(cmd=="--restore"&&settings.value("mode",std::string())=="single"&&settings.contains("monitor_wallpapers")){
                    int selected=settings.value("screen",0);auto saved=settings["monitor_wallpapers"];
                    for(int n=0;n<(int)monitors.size();n++)if(saved.contains(monitors[n])){lab_set_screen(n);auto error=launch(items.at(find(saved[monitors[n]])),settings,false);if(!error.empty())throw std::runtime_error(error);}
                    lab_set_screen(selected);if(settings.value("caelestia_enabled",false)&&lab_current()>=0)syncCaelestia(items[lab_current()],settings);return 0;
                }
                auto id=cmd=="--restore"?settings.value("path",std::string()):argc>2?argv[2]:"";
                if(id.empty())throw std::runtime_error("Nessun wallpaper selezionato");
                auto item=items.at(find(id));
                if(cmd=="--plan"){std::cout<<json(renderArgs(item,settings,false)).dump(2)<<'\n';return 0;}
                auto error=launch(item,settings,cmd=="--preview");if(!error.empty())throw std::runtime_error(error);
                if(cmd=="--preview")std::cout<<previewProcess.dump()<<'\n';return 0;
            }
            if(cmd!="--workshop-ui-test"&&cmd!="--workshop"&&cmd!="--ui-test"&&cmd!="--display-settings"&&cmd!="--appearance-settings"&&cmd!="--language-settings")throw std::runtime_error("Opzione sconosciuta: "+cmd);
        }
        return selector_main(argc,argv);
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
