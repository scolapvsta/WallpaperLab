#include "preferences.hpp"
#include "theme_catalog.h"
#include "shortcuts.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <limits>
using namespace lab;
void require(bool b,const char* message){if(!b)throw std::runtime_error(message);}
template<class F> void rejects(F f,const char* msg){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}require(rejected,msg);}
int main(){
    auto root=fs::current_path()/("wallpaper-lab-preferences-"+std::to_string(getpid()));
    setenv("XDG_CONFIG_HOME",(root/"config").c_str(),1);setenv("XDG_STATE_HOME",(root/"state").c_str(),1);
    try {
        for(int theme=0;theme<LAB_THEME_COUNT;theme++){
            writeJson(configDir()/"settings.json",{{"theme_catalog",true},{"ui_theme",theme}});
            require(loadSettings()["theme_collection"]==labThemes[theme].collection,"theme collection mismatch");
        }
        writeJson(configDir()/"settings.json",{{"ui_theme",5},{"light_theme",false}});require(loadSettings()["light_theme"]==true&&loadSettings()["ui_theme"]==5,"legacy pastel migration failed");
        writeJson(configDir()/"settings.json",{{"ui_theme",1}});require(loadSettings()["light_theme"]==true&&loadSettings()["ui_theme"]==1,"legacy light migration failed");
        writeJson(configDir()/"settings.json",{{"ui_theme",4},{"light_theme",true},{"theme_variants",true}});require(loadSettings()["light_theme"]==true&&loadSettings()["ui_theme"]==12,"legacy light pink did not migrate to independent Sakura Paper");
        writeJson(configDir()/"settings.json",{{"theme_catalog",true},{"theme_ui_v2",true},{"ui_design",1},{"ui_theme",29},{"light_theme",false}});
        require(loadSettings()["light_theme"]==false&&loadSettings()["ui_design"]==1&&loadSettings()["ui_theme"]==29,"independent dark strawberry mode lost on reload");
        writeJson(configDir()/"settings.json",{{"theme_catalog",true},{"ui_theme",4}});
        require(loadSettings()["ui_design"]==0&&loadSettings()["light_theme"]==false,"legacy appearance changed on migration");
        for(int i=0;i<LAB_THEME_COUNT;i++)for(int light=0;light<2;light++){
            auto colors=LabResolvedPalette(i,light);require(colors.light==light,"palette mode ignored");
            require(colors.bg!=colors.text&&colors.surface!=colors.accent,"palette lost contrast");
        }
        require(loadCarousel().empty(),"new carousel is not empty");
        std::vector<Item> items={{"1","One","scene","","/one",""},{"2","Two","scene","","/two",""},{"3","Broken","preset","","/three","unsupported"}};
        changeCarousel("/two",0);changeCarousel("/one",0);changeCarousel("/three",0);
        auto list=loadCarousel();
        require(nextCarouselItem(list,items,"",1)==1,"first follows saved order");
        require(nextCarouselItem(list,items,"/two",1)==0,"forward cycle failed");
        require(nextCarouselItem(list,items,"/one",1)==1,"wrap or unsupported skip failed");
        require(nextCarouselItem(list,items,"/two",-1)==0,"reverse wrap failed");
        list.insert(list.begin(),"/missing");require(nextCarouselItem(list,items,"/missing",1)==1,"missing entry not skipped");
        changeCarousel("/one",-1);require(loadCarousel()[0]=="/one","reorder not persisted");
        changeCarousel("/one",0);require(loadCarousel().size()==2,"remove not persisted");
        changeCarousel("/two",0);rejects([&]{nextCarouselItem(loadCarousel(),items,"",1);},"unsupported-only cycle accepted");
        {ControllerLock lock;rejects([]{changeCarousel("/other",0);},"concurrent write accepted");}
        require(loadCarousel().size()==1,"failed write corrupted list");
        writeJson(configDir()/"carousel.json",json::array({"/one","/one",17,"/two"}));require(loadCarousel().size()==2,"duplicate normalization failed");
        writeJson(configDir()/"carousel.json",json::object());rejects([]{loadCarousel();},"corrupt carousel silently overwritten");
        require(wallpaperPosition(loadPlacements(),"/one")["x"]==0,"new wallpaper must be centered");
        saveWallpaperPosition("/one",0.35,-0.2);saveWallpaperPosition("/two",-0.7,0.8);
        require(wallpaperPosition(loadPlacements(),"/one")["x"]==0.35,"other wallpaper overwrote position");
        writeJson(configDir()/"carousel.json",json::array());changeCarousel("/one",0);changeCarousel("/one",0);changeCarousel("/one",0);
        require(wallpaperPosition(loadPlacements(),"/one")["y"]==-0.2,"favorite removal lost wallpaper profile");
        auto saved=loadPlacements();
        rejects([]{saveWallpaperPosition("/one",1.01,0);},"out of range position accepted");
        rejects([]{saveWallpaperPosition("/one",std::numeric_limits<double>::quiet_NaN(),0);},"NaN position accepted");
        {ControllerLock lock;rejects([]{saveWallpaperPosition("/one",0,0);},"concurrent placement write accepted");}
        require(loadPlacements()==saved,"failed placement save changed profiles");
        saveWallpaperPosition("/one",0,0);require(wallpaperPosition(loadPlacements(),"/one")["x"]==0,"center reset failed");
        require(wallpaperPosition(loadPlacements(),"/two")["x"]==-0.7,"center reset changed another profile");
        require(fs::exists(stateDir()/"backups"),"placement backup missing");
        writeJson(configDir()/"wallpaper_profiles.json",json::array());
        rejects([]{saveWallpaperPosition("/one",0,0);},"corrupt profiles overwritten");
        require(readJson(configDir()/"wallpaper_profiles.json").is_array(),"corrupt profiles not preserved");
        auto project=root/"projects/123";fs::create_directories(project);
        rejects([&]{trashProject(project);},"non-project directory accepted for trash");
        writeJson(project/"project.json",{{"title","Disposable fixture"}});
        auto alias=root/"project-link";fs::create_directory_symlink(project,alias);
        rejects([&]{trashProject(alias);},"symlink accepted for trash");
        setenv("XDG_DATA_HOME",(root/"data").c_str(),1);
        trashProject(project);
        require(!fs::exists(project)&&fs::exists(root/"data/Trash/files/123/project.json"),"project was not moved to recoverable system trash");
        require(fs::exists(root/"data/Trash/info/123.trashinfo"),"trash restoration metadata missing");
        auto sc=parseShortcut("Shift + SUPER + Tab");require(sc.mask==65&&sc.combo=="SUPER+SHIFT+Tab","shortcut normalization failed");
        rejects([]{parseShortcut("SUPER+Tab;touch /tmp/no");},"unsafe key accepted");
        rejects([]{parseShortcut("Tab");},"unmodified global key accepted");
        rejects([]{parseShortcut("SUPER+SUPER+Tab");},"duplicate modifier accepted");
        auto lua=shortcutConfig(parseShortcut("SUPER+Tab"),"/tmp/a b'c/wallpaper-lab",true);
        require(lua.find("--carousel-event")!=std::string::npos&&lua.find("description")!=std::string::npos,"shortcut missing action or ownership label");
        require(lua.find(" size =")==std::string::npos&&lua.find("no_anim = true")!=std::string::npos&&lua.find("no_blur = true")!=std::string::npos,"popup must retain content sizing and immediate transparent presentation");
        fs::remove_all(root);std::cout<<"Carousel persistence, ordering, wrap, exclusions, locking and shortcut validation passed\n";
    }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}
}
