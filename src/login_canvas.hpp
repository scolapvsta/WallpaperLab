#pragma once
#include "login_config.hpp"
#include <ctime>
namespace lab::login {
struct CanvasState {
    std::string username="User",input,session="Desktop",prompt="Password",status,title;
    bool keyboardOpen=false,keyboardShift=false,reveal=false;
    int powerConfirm=-1;
    bool secret=true,busy=false,preview=true,paused=false;
    int focus=0,cursor=0;
    bool caret=false;float inputProgress=1;
    bool menuOpen=false;int menuIndex=0,menuScroll=0;std::vector<std::string> sessions;
};
struct Pixels { int width=0,height=0;std::vector<unsigned char> rgba; };
Pixels draw(const json& document,int width,int height,const CanvasState& state,std::time_t time);
// Normalized action regions shared by the native greeter and editor composition.
std::array<double,4> widgetRect(const json& document,int widget);
std::array<double,4> sessionMenuRect(const json& document,int count);
std::string greeting(const json& document,const CanvasState& state,const std::tm& now);
int hit(const json& document,double x,double y,const CanvasState* state=nullptr);
}
