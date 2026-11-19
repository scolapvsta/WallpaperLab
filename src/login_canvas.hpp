#pragma once
#include "login_config.hpp"
#include <ctime>
namespace lab::login {
struct CanvasState {
    std::string username="User",input,session="Desktop",prompt="Password",status,title;
    bool secret=true,busy=false,preview=true,paused=false;
    int focus=0;
};
struct Pixels { int width=0,height=0;std::vector<unsigned char> rgba; };
Pixels draw(const json& document,int width,int height,const CanvasState& state,std::time_t time);
// Normalized action regions shared by the native greeter and editor composition.
std::array<double,4> widgetRect(const json& document,int widget);
int hit(const json& document,double x,double y);
}
