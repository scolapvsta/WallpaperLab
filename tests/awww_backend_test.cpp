#include "awww_backend.hpp"
#include <iostream>
using namespace lab;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F> void rejects(F action){bool rejected=false;try{action();}catch(...){rejected=true;}require(rejected,"invalid hook accepted");}
int main(){
 auto root=fs::current_path()/("awww-test-"+std::to_string(getpid()));
 try{
  auto prefs=importAwwwHook(R"(awww img "$WALLPAPER_PATH" --transition-step 2 --transition-type outer --transition-duration 2 --transition-fps 144 --transition-pos $(hyprctl cursorpos | tr -d ' ') --invert-y && printf '%s' "$WALLPAPER_PATH")");
  require(prefs.at("awww_transition")==12&&prefs.at("awww_step")==2&&prefs.at("awww_duration")==2000&&prefs.at("awww_fps")==144&&prefs.at("awww_position")==0&&prefs.at("awww_invert_y")==true,"CLI import did not preserve transition");
  require(prefs.at("wallpaper_post_hook")==R"( printf '%s' "$WALLPAPER_PATH")","post-hook changed");
  rejects([]{importAwwwHook("echo unsafe");});
  rejects([]{importAwwwHook("awww img $WALLPAPER_PATH --transition-type nonexistent");});
  rejects([]{importAwwwHook("awww img $WALLPAPER_PATH --transition-fps 0");});
  rejects([]{importAwwwHook("awww img $WALLPAPER_PATH --transition-duration nan");});
  rejects([]{importAwwwHook("awww img $WALLPAPER_PATH --transition-step 2;echo");});
  fs::create_directories(root/"project");std::ofstream(root/"project/media $(touch escaped).png")<<"image";
  writeJson(root/"project/project.json",{{"lab_media_file","media $(touch escaped).png"}});
  Item image;image.path=fs::absolute(root/"project");image.type="image";image.origin="local";
  require(usesAwww(image,prefs),"local image bypassed backend");
  prefs["mode"]="single";prefs["screen"]=1;prefs["awww_position"]=1;
  auto args=awwwArgs(image,prefs,{"DP-1","HDMI-A-1"});
  require(args.at(2)==fs::canonical(root/"project/media $(touch escaped).png").string()&&args.at(4)=="HDMI-A-1","media/output argument changed");
  require(!fs::exists("escaped"),"image filename executed");
  prefs["reduce_motion"]=true;args=awwwArgs(image,prefs,{"DP-1"});require(args.at(6)=="none","reduced motion ignored");
  prefs["awww_step"]=255;prefs["awww_duration"]=99999;args=awwwArgs(image,prefs,{"DP-1"});
  require(args.at(8)=="100"&&std::stod(args.at(10))==10,"backend timing caps ignored");
  prefs["mode"]="span";rejects([&]{awwwArgs(image,prefs,{"DP-1","DP-2"});});
  prefs["mode"]="mirror";args=awwwArgs(image,prefs,{"DP-1","DP-2"});require(args.at(4)=="DP-1,DP-2","mirror outputs missing");
  image.type="video";require(!usesAwww(image,prefs),"video sent to awww");image.type="image";image.origin="steam";require(!usesAwww(image,prefs),"Workshop image sent to awww");
  fs::remove(root/"project/project.json");writeJson(root/"project/project.json",json::object());
  fs::rename(root/"project/media $(touch escaped).png",root/"project/media.gif");require(awwwImage(image).filename()=="media.gif","legacy GIF import failed");
  std::ofstream(root/"outside.png")<<"image";writeJson(root/"project/project.json",{{"lab_media_file","../outside.png"}});rejects([&]{awwwImage(image);});
  auto matched=awwwMatchingOutputs(": DP-1: 1920x1080, scale: 1, currently displaying: image: /tmp/a b.png\n: DP-2: 1920x1080, scale: 1, currently displaying: image: /tmp/other.png\n","/tmp/a b.png","DP-1,DP-2");
  require(matched==std::vector<std::string>{"DP-1"},"unrelated output matched");
  fs::remove_all(root);std::cout<<"awww import, arguments, media containment and output ownership passed\n";
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}
}
