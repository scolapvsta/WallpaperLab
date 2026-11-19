#include "login_config.hpp"
#include "login_canvas.hpp"
#include "login_astronaut.hpp"
#include "theme_fonts.hpp"
#include <cairo.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
using namespace lab;using namespace lab::login;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){
    auto temporary=fs::temp_directory_path()/("lab-astronaut-"+std::to_string(getpid()));setenv("XDG_CONFIG_HOME",(temporary/"config").c_str(),1);setenv("XDG_STATE_HOME",(temporary/"state").c_str(),1);registerThemeFonts();
    try{
        auto presets=listPresets(false);check(presets.size()==10,"Missing bundled Astronaut presets");
        auto base=validate(defaults());base["source"]="user-wallpaper";base["mode"]="single";base["wallpapers"]["DP-2"]="another-wallpaper";base["fps"]=90;
        for(auto& preset:presets){
            auto d=applyPreset(base,preset,false);check(astronaut(d)&&d["source"]==base["source"]&&d["fps"]==90&&d["wallpapers"]==base["wallpapers"],"UI preset changed wallpaper settings");
            check(d["widgets"].size()==3,"Preset resurrected removed widgets");
            auto roundTrip=applyPreset(base,makePreset(d,"Edited",false),false);check(d==roundTrip,"Native design lost settings on save/load");
            CanvasState state;state.preview=false;state.focus=1;state.caret=true;state.session="Hyprland";
            for(auto& c:astronautControls(d)){auto r=c.rect;check(hit(d,r[0]+r[2]/2,r[1]+r[3]/2,&state)==c.action,"Astronaut action/hit mismatch");}
            state.menuOpen=true;state.sessions={"First","Second","Third"};auto menu=sessionMenuRect(d,3);check(hit(d,menu[0]+menu[2]/2,menu[1]+menu[3]/2,&state)==101,"Astronaut session menu mismatch");state.menuOpen=false;
            state.keyboardOpen=true;for(auto& key:keyboardControls(d)){auto r=key.rect;check(hit(d,r[0]+r[2]/2,r[1]+r[3]/2,&state)==key.action,"Virtual keyboard hit mismatch");}state.keyboardOpen=false;
            state.powerConfirm=0;check(hit(d,.4,.56,&state)==300&&hit(d,.56,.56,&state)==301,"System action confirmation mismatch");state.powerConfirm=-1;
            for(auto dims:std::vector<std::pair<int,int>>{{640,360},{1920,1080},{2560,1440},{1280,1024}}){
                auto pixels=draw(d,dims.first,dims.second,state,1795088040);
                for(size_t n=0;n<pixels.rgba.size();n+=4)for(int k=0;k<3;k++)check(pixels.rgba[n+k]<=pixels.rgba[n+3],"Native design is not premultiplied");
                if(argc>1&&dims.first==1920){fs::create_directories(argv[1]);for(size_t n=0;n<pixels.rgba.size();n+=4)std::swap(pixels.rgba[n],pixels.rgba[n+2]);auto surface=cairo_image_surface_create_for_data(pixels.rgba.data(),CAIRO_FORMAT_ARGB32,pixels.width,pixels.height,pixels.width*4);auto path=fs::path(argv[1])/(fs::path(preset["file"].get<std::string>()).stem().string()+"-native.png");check(cairo_surface_write_to_png(surface,path.c_str())==CAIRO_STATUS_SUCCESS,"Cannot export design check");cairo_surface_destroy(surface);}
            }
            auto colorsBefore=colors(d);unsigned char rgb[]={40,30,60,180,190,230};auto proposed=applyPalette(d,rgb,2,1);check(restoreColors(proposed,colorsBefore)==d,"Palette altered native preset geometry/alpha");
        }
        auto original=presets.front(),edited=applyPreset(base,original,false);edited["widgets"]["clock"]["color"]="#123456";auto source=readJson(original["file"]);savePreset(edited,original["name"],false);auto list=listPresets(false);check(list.size()==10&&!list.front()["bundled"].get<bool>(),"Personal override duplicated bundled preset");check(readJson(original["file"])==source,"Saving a preset modified shipped designs");check(applyPreset(base,list.front(),false)["widgets"]["clock"]["color"]=="#123456","Personal override did not load");
        bool rejected=false;try{edited["widgets"]["login"]["astronaut"]["blur"]=1e9;validate(edited);}catch(...){rejected=true;}check(rejected,"Invalid backdrop accepted");
        fs::remove_all(temporary);std::cout<<"10 Astronaut presets: scope, persistence, overrides, geometry, actions, palette and four output sizes passed\n";
    }catch(...){fs::remove_all(temporary);throw;}
}
