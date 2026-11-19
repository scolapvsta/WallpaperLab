#pragma once
#include "login_canvas.hpp"
namespace lab::login {
// Native geometry adapted from Keyitdev's GPL-3.0-or-later Astronaut QML.
// All action hit regions use the same normalized rectangles as their drawing.
using Rect=std::array<double,4>;
struct ActionRegion { int action; Rect rect; };
inline bool astronaut(const json& d){return d["widgets"]["login"].value("layout",std::string("base"))=="astronaut";}
inline std::vector<ActionRegion> astronautControls(const json& d){
    auto r=widgetRect(d,2);auto& w=d["widgets"]["login"];auto& a=w["astronaut"];
    double width=r[2]*a["content_width"].get<double>(),x=r[0]+(r[2]-width)/2,h=r[3]*w["field_height"].get<double>(),y=r[1]+r[3]*a["field_y"].get<double>();
    std::vector<ActionRegion> result={{0,{x,y,width,h}},{1,{x,y+h+r[3]*a["row_gap"].get<double>(),width,h}},{2,{x,r[1]+r[3]*a["session_y"].get<double>(),width,r[3]*.025}}};
    if(a["button_visible"])result.push_back({3,{x,r[1]+r[3]*a["button_y"].get<double>(),width,r[3]*w["button_height"].get<double>()}});
    if(a["system_buttons"])for(int n=0;n<4;n++)result.push_back({10+n,{r[0]+r[2]*(.5+a["action_centers"][n].get<double>()-.052),r[1]+r[3]*a["actions_y"].get<double>(),r[2]*.104,r[3]*.07}});
    if(a["virtual_keyboard"])result.push_back({14,{x,r[1]+r[3]*a["keyboard_y"].get<double>(),width,r[3]*.03}});
    return result;
}
inline Rect keyboardRect(const json& d){auto r=widgetRect(d,2);return {r[0],r[1]+r[3]*.72,r[2],r[3]*.28};}
inline const std::array<std::string,5> keyRows={"1234567890-=", "qwertyuiop[]", "asdfghjkl;'", "zxcvbnm,./", ""};
inline std::vector<ActionRegion> keyboardControls(const json& d){
    auto r=keyboardRect(d);std::vector<ActionRegion> result;int id=200;
    for(int row=0;row<4;row++){double width=r[2]/12,y=r[1]+row*r[3]/5;for(char key:keyRows[row]){int col=id-200;for(int n=0;n<row;n++)col-=keyRows[n].size();result.push_back({id++,{r[0]+(12-keyRows[row].size())*width/2+col*width,y,width,r[3]/5}});}}
    for(int n=0;n<5;n++)result.push_back({250+n,{r[0]+n*r[2]/5,r[1]+4*r[3]/5,r[2]/5,r[3]/5}});return result;
}
}
