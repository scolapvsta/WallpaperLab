#pragma once
#include "catalog.hpp"
#include <cstdlib>
#include <iostream>
namespace lab {
// The compositor must not slide/fade the native wallpaper under our own shader.
// This named runtime rule affects only our namespace and is reapplied after reloads.
inline const char* desktopLayerRule(){return R"lua(if _G.__wallpaper_lab_background_rule then
 _G.__wallpaper_lab_background_rule:set_enabled(true)
else
 _G.__wallpaper_lab_background_rule = hl.layer_rule({name="wallpaper-lab-background",match={namespace="^wallpaper-lab-background$"},no_anim=true})
end)lua";}
inline void prepareDesktopSurface(){
 const char* session=std::getenv("HYPRLAND_INSTANCE_SIGNATURE");if(!session||!*session)return;
 try{auto result=capture({"hyprctl","eval",desktopLayerRule()});if(result.empty()||result[0].find("ok")==std::string::npos)std::cerr<<"Wallpaper Lab: impossibile isolare le animazioni del compositor per lo sfondo\n";}catch(const std::exception& e){std::cerr<<"Wallpaper Lab: regola del compositor: "<<e.what()<<"\n";}
}
}
