#include "login_config.hpp"
#include "login_canvas.hpp"
#include "login_astronaut.hpp"
#include "login_motion.hpp"
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
        // Every preset must show a newly enabled welcome, without touching clock/date.
        for(auto& preset:presets){auto d=applyPreset(base,preset,false);CanvasState display;display.preview=false;display.focus=-1;
            auto& form=d["widgets"]["login"];form["welcome_enabled"]=false;auto hidden=draw(d,1280,720,display,1795088040);
            for(auto message:{std::string("Welcome home"),std::string("Ciao, città — ようこそ"),std::string(100,'W')}){form["welcome_text"]=message;form["welcome_enabled"]=true;auto shown=draw(d,1280,720,display,1795088040);int changed=0;for(size_t n=0;n<shown.rgba.size();n+=4)if(shown.rgba[n]!=hidden.rgba[n]||shown.rgba[n+1]!=hidden.rgba[n+1]||shown.rgba[n+2]!=hidden.rgba[n+2])changed++;check(changed>100,"Enabled welcome is invisible/clipped");
                auto clock=widgetRect(d,0);int y0=int(std::ceil(clock[1]*720))+2,y1=int((clock[1]+clock[3])*720)-2;for(int y=y0;y<y1;y++)for(int x=0;x<1280;x++)for(int c=0;c<4;c++)check(shown.rgba[(y*1280+x)*4+c]==hidden.rgba[(y*1280+x)*4+c],"Welcome overlaps the clock");
                if(argc>1&&message=="Welcome home"){for(size_t n=0;n<shown.rgba.size();n+=4)std::swap(shown.rgba[n],shown.rgba[n+2]);auto surface=cairo_image_surface_create_for_data(shown.rgba.data(),CAIRO_FORMAT_ARGB32,1280,720,1280*4);auto path=fs::path(argv[1])/(fs::path(preset["file"].get<std::string>()).stem().string()+"-welcome.png");cairo_surface_write_to_png(surface,path.c_str());cairo_surface_destroy(surface);}
            }
            form["dynamic_welcome"]=true;display.username="Alex";check(draw(d,1280,720,display,1795088040).rgba!=hidden.rgba,"Dynamic welcome disappeared");
            // Cropped motion rendering preserves the full-frame region, allowing 2/255
            // channel rounding from Cairo clipped groups and scaled icon masks.
            display.input="abc";display.cursor=3;display.dotProgress={1,.7,.2};display.focus=1;display.focusMix={.2,.8,0};display.sessions={"First","Second"};
            for(auto dimensions:std::vector<std::pair<int,int>>{{640,360},{1920,1080}})for(int mode=0;mode<3;mode++){
                display.menuOpen=mode==1;display.menuAlpha=mode==1?.4:0;display.keyboardOpen=mode==2;display.keyboardAlpha=mode==2?.6:0;
                auto [width,height]=dimensions;auto full=draw(d,width,height,display,1795088040);auto region=motionRect(d,width,height,display);auto partial=draw(d,width,height,display,1795088040,region);
                int maximum=0;
                for(int y=0;y<region[3];y++)for(int x=0;x<region[2];x++)for(int c=0;c<4;c++){
                    int original=full.rgba[((y+region[1])*width+x+region[0])*4+c],cropped=partial.rgba[(y*region[2]+x)*4+c];
                    int delta=std::abs(original-cropped);maximum=std::max(maximum,delta);
                }
                check(maximum<=2,"Cropped animation differs from the full canvas beyond rounding tolerance");
            }
        }
        Motion motion;CanvasState animated;animated.focus=1;animated.input="xx";motion.inserted(0,1);motion.inserted(1,1.1);motion.sample(animated,1.14,.28);check(animated.dotProgress[0]>animated.dotProgress[1]&&animated.dotProgress[1]>0,"Rapid typing restarted earlier dots");motion.erased(0);check(motion.births.size()==1&&motion.births.front()==1.1,"Masked deletion lost timing index");motion.clear();check(motion.births.empty(),"Password timings survived clear");check(dotGrowth(0)==0&&dotGrowth(.7)>1&&std::abs(dotGrowth(1)-1)<1e-6,"Dot pulse has wrong endpoints");
        auto original=presets.front(),edited=applyPreset(base,original,false);edited["widgets"]["clock"]["color"]="#123456";auto source=readJson(original["file"]);savePreset(edited,original["name"],false);auto list=listPresets(false);check(list.size()==10&&!list.front()["bundled"].get<bool>(),"Personal override duplicated bundled preset");check(readJson(original["file"])==source,"Saving a preset modified shipped designs");check(applyPreset(base,list.front(),false)["widgets"]["clock"]["color"]=="#123456","Personal override did not load");
        bool rejected=false;try{edited["widgets"]["login"]["astronaut"]["blur"]=1e9;validate(edited);}catch(...){rejected=true;}check(rejected,"Invalid backdrop accepted");
        fs::remove_all(temporary);std::cout<<"10 Astronaut presets: scope, persistence, overrides, welcome text, partial motion redraws, geometry, actions, palette and four output sizes passed\n";
    }catch(...){fs::remove_all(temporary);throw;}
}
