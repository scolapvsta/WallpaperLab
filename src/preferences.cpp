#include "preferences.hpp"
#include "theme_catalog.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fcntl.h>
#include <set>
#include <sys/file.h>
#include <unistd.h>
namespace lab {
ControllerLock::ControllerLock() {
    fs::create_directories(stateDir());
    fd=open((stateDir()/"controller.lock").c_str(),O_CREAT|O_RDWR|O_CLOEXEC,0600);
    if(fd<0) throw std::runtime_error("Impossibile aprire il controller");
    if(flock(fd,LOCK_EX|LOCK_NB)) { close(fd);fd=-1;throw std::runtime_error("Cambio wallpaper in corso. Riprova tra poco."); }
}
ControllerLock::~ControllerLock(){if(fd>=0)close(fd);}
void trashProject(const fs::path& path) {
    // Only real project directories may be passed by the catalog controller.
    if(!path.is_absolute()||fs::is_symlink(path)||!fs::is_directory(path)||!fs::is_regular_file(path/"project.json"))
        throw std::runtime_error("Cartella wallpaper non valida: nessun file eliminato.");
    if(capture({"gio","trash","--",path.string()}).empty())
        throw std::runtime_error("Impossibile spostare lo sfondo nel cestino. Nessuna eliminazione permanente eseguita.");
}
json loadPlacements(){
    auto file=configDir()/"wallpaper_profiles.json";
    if(!fs::exists(file))return json::object();
    auto result=readJson(file);if(!result.is_object())throw std::runtime_error("Profili wallpaper non validi: file conservato.");return result;
}
json wallpaperPosition(const json& profiles,const std::string& path){
    json result={{"x",0.0},{"y",0.0},{"zoom",1.0}};
    try{
        const auto& p=profiles.at(path).at("position");
        double x=p.at("x"),y=p.at("y");
        if(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<=1&&std::abs(y)<=1)return {{"x",x},{"y",y},{"zoom",std::clamp(p.value("zoom",1.0),1.0,3.0)}};
    }catch(...){}return result;
}
void saveWallpaperPosition(const std::string& path,double x,double y){
    if(path.empty()||!std::isfinite(x)||!std::isfinite(y)||std::abs(x)>1||std::abs(y)>1)throw std::runtime_error("Posizione dello sfondo non valida");
    ControllerLock lock;auto latest=loadPlacements();
    if(fs::exists(configDir()/"wallpaper_profiles.json")){
        auto backup=stateDir()/"backups"/("placement-"+std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
        writeJson(backup/"wallpaper_profiles.json",latest);
    }
    auto position=wallpaperPosition(latest,path);position["x"]=x;position["y"]=y;latest[path]["position"]=position;
    writeJson(configDir()/"wallpaper_profiles.json",latest);
}
json loadSettings() {
    json prefs={{"window_opacity",100},{"page_size",100},{"fullscreen_policy",1},{"font_weight",1},{"carousel_layout",1},{"mode","span"},{"screen",0},{"fps",60},{"muted",true},{"disable_audio_processing",false},{"disable_mouse",true},{"disable_parallax",true},{"disable_particles",false}};
    auto file=configDir()/"settings.json";
    if(fs::exists(file)) { auto saved=readJson(file);if(!saved.is_object())throw std::runtime_error("Impostazioni non valide");prefs.update(saved); }
    if(!prefs.value("theme_variants",false)){int theme=prefs.value("ui_theme",prefs.value("light_theme",false)?1:0);prefs["light_theme"]=theme==1||theme==5;prefs["ui_theme"]=theme==1?0:theme;prefs["theme_variants"]=true;}
    if(!prefs.value("theme_catalog",false)){int id=std::clamp(prefs.value("ui_theme",0),0,10);int dark[]={0,1,2,3,4,14,6,7,8,9,10},light[]={1,1,13,13,12,5,11,13,5,13,11};prefs["ui_theme"]=prefs.value("light_theme",false)?light[id]:dark[id];prefs["theme_catalog"]=true;}
    int theme=std::clamp(prefs.value("ui_theme",0),0,LAB_THEME_COUNT-1);prefs["ui_theme"]=theme;prefs["light_theme"]=bool(labThemes[theme].light);
    prefs["window_transparency"]=std::clamp(prefs.value("window_transparency",100-prefs.value("window_opacity",100)),0,30);prefs["window_opacity"]=100-prefs["window_transparency"].get<int>();
    prefs["window_opacity"]=std::clamp(prefs.value("window_opacity",100),70,100);
    int pageSize=prefs.value("page_size",100);prefs["page_size"]=pageSize<=50?50:pageSize<=100?100:200;
    return prefs;
}
json loadCarousel() {
    auto file=configDir()/"carousel.json";
    if(!fs::exists(file))return json::array();
    auto list=readJson(file);
    if(!list.is_array())throw std::runtime_error("Lista carousel non valida");
    json clean=json::array();std::set<std::string> seen;
    for(auto& entry:list)if(entry.is_string()&&seen.insert(entry.get<std::string>()).second)clean.push_back(entry);
    return clean;
}
void changeCarousel(const std::string& path,int operation) {
    ControllerLock lock;
    auto list=loadCarousel();auto it=std::find(list.begin(),list.end(),json(path));
    if(operation==0) {if(it==list.end())list.push_back(path);else list.erase(it);}
    else if(it!=list.end()) {
        auto index=it-list.begin(),other=index+(operation<0?-1:1);
        if(other>=0&&other<(long)list.size())std::swap(list[index],list[other]);
    }
    writeJson(configDir()/"carousel.json",list);
}
std::vector<int> carouselItems(const json& list,const std::vector<Item>& items) {
    std::vector<int> indices;
    for(auto& path:list)for(size_t i=0;i<items.size();++i)if(path==items[i].path){indices.push_back(i);break;}
    return indices;
}
int nextCarouselItem(const json& list,const std::vector<Item>& items,const std::string& current,int direction) {
    // Keep the saved order; missing or unsupported entries never enter the cycle.
    auto indices=carouselItems(list,items);
    indices.erase(std::remove_if(indices.begin(),indices.end(),[&](int i){return !items[i].reason.empty();}),indices.end());
    if(indices.empty())throw std::runtime_error("Carousel vuoto: aggiungi almeno uno sfondo compatibile dalla libreria.");
    auto it=std::find_if(indices.begin(),indices.end(),[&](int i){return items[i].path==current;});
    if(it==indices.end())return direction>0?indices.front():indices.back();
    int pos=it-indices.begin(),size=indices.size();
    return indices[(pos+(direction>0?1:-1)+size)%size];
}
}
