#include "lab.h"
#include "catalog.hpp"
#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <future>
#include <iostream>
#include <spawn.h>
#include <sstream>
#include <sys/file.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
extern char **environ;
int engineMain(int,char**,const std::string&);
namespace {
using namespace lab;
std::vector<Item> items;
std::vector<std::string> monitors;
json settings={{"mode","span"},{"screen",0},{"fps",60},{"muted",true},{"disable_mouse",true},{"disable_parallax",true},{"disable_particles",false}};
std::string errorMessage,statusMessage;
std::future<std::string> pending;
bool pendingPreview=false;
json previewProcess;
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
    if(!owned(process))return;
    int pid=process.at("pid");
    kill(-pid,SIGTERM);
    for(int i=0;i<50 && owned(process);++i) {int status;waitpid(pid,&status,WNOHANG);std::this_thread::sleep_for(std::chrono::milliseconds(50));}
    if(owned(process))kill(-pid,SIGKILL);
    int status;waitpid(pid,&status,WNOHANG);
}
json running() {try{return readJson(stateDir()/"running.json");}catch(...){return json::object();}}
std::vector<std::string> renderArgs(const Item& item,const json& prefs,bool preview) {
    if(!item.reason.empty())throw std::runtime_error(item.reason);
    std::string assets=assetsPath();if(assets.empty())throw std::runtime_error("Assets di Wallpaper Engine non trovati nelle librerie Steam");
    std::vector<std::string> args={executable(),"--render","--assets-dir",assets,"--fps",std::to_string(std::clamp(prefs.value("fps",60),15,120)),"--volume",prefs.value("muted",true)?"0":"100"};
    if(prefs.value("muted",true))args.push_back("--no-audio-processing");
    if(prefs.value("disable_mouse",true))args.push_back("--disable-mouse");
    if(prefs.value("disable_parallax",true))args.push_back("--disable-parallax");
    if(prefs.value("disable_particles",false))args.push_back("--disable-particles");
    if(preview) {args.insert(args.end(),{"--scaling","fit","--clamp","border","--window","0x0x1100x620","--no-fullscreen-pause",item.path});return args;}
    if(monitors.empty())throw std::runtime_error("Nessun monitor Hyprland rilevato");
    auto mode=prefs.value("mode",std::string("span"));
    if(mode=="span" && monitors.size()>1) {
        std::string joined;for(auto& m:monitors){if(!joined.empty())joined+=",";joined+=m;}
        args.insert(args.end(),{"--scaling","fill","--screen-span",joined,"--bg",item.path});
    } else if(mode=="mirror") {
        for(auto&m:monitors)args.insert(args.end(),{"--scaling","fill","--screen-root",m,"--bg",item.path});
    } else {
        int index=std::clamp(prefs.value("screen",0),0,(int)monitors.size()-1);
        args.insert(args.end(),{"--scaling","fill","--screen-root",monitors[index],"--bg",item.path});
    }
    return args;
}
std::string launch(Item item,json prefs,bool preview) {
    int lock=-1;pid_t pid=-1;fs::path ready;
    try {
        fs::create_directories(stateDir());
        lock=open((stateDir()/"controller.lock").c_str(),O_CREAT|O_RDWR|O_CLOEXEC,0600);
        if(lock<0 || flock(lock,LOCK_EX))throw std::runtime_error("Impossibile bloccare il controller");
        auto args=renderArgs(item,prefs,preview);
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
            prefs["path"]=item.path;writeJson(configDir()/"settings.json",prefs);
            writeJson(stateDir()/"running.json",process);
            stop(previous);
        }
        close(lock);return {};
    }catch(const std::exception& e){
        if(pid>1)stop(json{{"pid",pid},{"start",startTime(pid)}});
        if(!ready.empty()){std::error_code ec;fs::remove(ready,ec);}
        if(lock>=0)close(lock);
        return e.what();
    }
}
void init() {
    try{auto saved=readJson(configDir()/"settings.json");if(saved.is_object())settings.update(saved);}catch(...){}
    auto result=capture({"hyprctl","monitors","-j"});
    if(!result.empty())try {
        auto list=json::parse(result[0]);std::sort(list.begin(),list.end(),[](auto&a,auto&b){return a.value("x",0)<b.value("x",0);});
        for(auto&m:list)if(!m.value("disabled",false))monitors.push_back(m.at("name"));
    }catch(...){}
    std::vector<fs::path> roots;for(auto p:steamLibraries())roots.push_back(p/"steamapps/workshop/content/431960");
    if(auto extra=getenv("WALLPAPER_LAB_LIBRARY"))roots={fs::path(extra)};
    items=scan(roots);
}
int find(const std::string& id) {for(size_t i=0;i<items.size();++i)if(items[i].id==id || items[i].path==id)return i;throw std::runtime_error("Wallpaper non trovato: "+id);}
}
extern "C" {
int lab_count(){return std::min((int)items.size(),2048);}
const char* lab_title(int i){return items.at(i).title.c_str();}
const char* lab_preview(int i){return items.at(i).preview.c_str();}
const char* lab_type(int i){return items.at(i).type.c_str();}
const char* lab_error(){return errorMessage.c_str();}
int lab_current(){auto p=settings.value("path",std::string());for(int i=0;i<lab_count();++i)if(items[i].path==p)return i;return -1;}
const char* lab_status(){
    auto mode=settings.value("mode",std::string("span"));
    statusMessage=mode=="span"?"Panoramico":mode=="mirror"?"Duplicato":"Singolo";
    if(mode=="single"&&!monitors.empty())statusMessage+=" / "+monitors[std::clamp(settings.value("screen",0),0,(int)monitors.size()-1)];
    else statusMessage+=" / "+std::to_string(monitors.size())+" monitor";
    statusMessage+="    "+std::to_string(settings.value("fps",60))+" FPS    "+(settings.value("muted",true)?"Audio off":"Audio on");return statusMessage.c_str();
}
void lab_cycle_mode(){auto mode=settings.value("mode",std::string("span"));settings["mode"]=mode=="span"?"mirror":mode=="mirror"?"single":"span";}
void lab_cycle_screen(){if(!monitors.empty())settings["screen"]=(settings.value("screen",0)+1)%(int)monitors.size();}
void lab_cycle_fps(){int fps=settings.value("fps",60);settings["fps"]=fps==30?60:fps==60?120:30;}
void lab_toggle_mute(){settings["muted"]=!settings.value("muted",true);}
int lab_start(int i,int preview){
    if(pending.valid()||i<0||i>=lab_count())return 0;
    if(!items[i].reason.empty()){errorMessage=items[i].reason;return 0;}
    errorMessage.clear();pendingPreview=preview;
    pending=std::async(std::launch::async,launch,items[i],settings,(bool)preview);return 1;
}
int lab_poll(){
    if(!pending.valid()||pending.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return 0;
    errorMessage=pending.get();if(!errorMessage.empty())return -1;return pendingPreview?2:1;
}
void lab_shutdown(){if(pending.valid())pending.get();stop(previewProcess);}
}
int main(int argc,char**argv) {
    try {
        for(int i=1;i<argc;++i)if(std::string(argv[i]).starts_with("--type="))return engineMain(argc,argv,"");
        if(argc>1&&std::string(argv[1])=="--render") {
            std::string ready;std::vector<char*> av={argv[0]};
            for(int i=2;i<argc;++i){if(std::string(argv[i])=="--ready-file"&&i+1<argc)ready=argv[++i];else av.push_back(argv[i]);}
            int count=av.size();av.push_back(nullptr);return engineMain(count,av.data(),ready);
        }
        if(argc>1&&std::string(argv[1])=="--help") {
            std::cout<<"Wallpaper Lab - prototype\nUsage: wallpaper-lab [--list | --status | --restore | --stop | --plan ID | --apply ID | --preview ID]\nSelector: 1-4 layouts, / search, Enter apply, RMB image preview, Space live scene, M mode, Tab monitor, F FPS, V audio.\n";return 0;
        }
        init();
        if(argc>1) {
            std::string cmd=argv[1];
            if(cmd=="--list") {json out=json::array();for(auto&i:items)out.push_back({{"id",i.id},{"title",i.title},{"type",i.type},{"path",i.path},{"preview",i.preview},{"reason",i.reason}});std::cout<<out.dump(2)<<'\n';return 0;}
            if(cmd=="--status"){auto p=running();p["alive"]=owned(p);std::cout<<p.dump(2)<<'\n';return 0;}
            if(cmd=="--stop"){
                fs::create_directories(stateDir());
                int lock=open((stateDir()/"controller.lock").c_str(),O_CREAT|O_RDWR|O_CLOEXEC,0600);
                if(lock<0 || flock(lock,LOCK_EX))throw std::runtime_error("Impossibile bloccare il controller");
                stop(running());fs::remove(stateDir()/"running.json");close(lock);return 0;
            }
            if(cmd=="--restore" || cmd=="--apply" || cmd=="--preview" || cmd=="--plan") {
                auto id=cmd=="--restore"?settings.value("path",std::string()):argc>2?argv[2]:"";
                if(id.empty())throw std::runtime_error("Nessun wallpaper selezionato");
                auto item=items.at(find(id));
                if(cmd=="--plan"){std::cout<<json(renderArgs(item,settings,false)).dump(2)<<'\n';return 0;}
                auto error=launch(item,settings,cmd=="--preview");if(!error.empty())throw std::runtime_error(error);
                if(cmd=="--preview")std::cout<<previewProcess.dump()<<'\n';return 0;
            }
            if(cmd!="--ui-test")throw std::runtime_error("Opzione sconosciuta: "+cmd);
        }
        return selector_main(argc,argv);
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
