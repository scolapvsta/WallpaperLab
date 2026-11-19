#pragma once
#include "login_canvas.hpp"
#include <algorithm>
#include <cmath>
namespace lab::login {
inline double ease(double t){t=std::clamp(t,0.,1.);return t*t*(3-2*t);}
inline double dotGrowth(double t){return t<.7?1.18*ease(t/.7):1.18-.18*ease((t-.7)/.3);}
struct Tween {
    double from=0,to=0,since=0;
    double value(double now,double duration=.18)const{return from+(to-from)*ease((now-since)/duration);}
    void target(double next,double now){if(next==to)return;from=value(now);to=next;since=now;}
    bool active(double now)const{return from!=to&&now-since<.18;}
};
// Holds timing and masked character indices only, never a second password copy.
struct Motion {
    std::vector<double> births;
    std::array<Tween,3> focus;
    Tween button,menu,keyboard;
    double caretSince=0;
    void inserted(int index,double now){births.insert(births.begin()+std::clamp(index,0,int(births.size())),now);}
    void erased(int index){if(index>=0&&index<int(births.size()))births.erase(births.begin()+index);}
    void clear(){births.clear();}
    bool sample(CanvasState& state,double now,double duration){
        bool active=false;state.dotProgress.clear();
        for(double birth:births){double t=std::clamp((now-birth)/duration,0.,1.);state.dotProgress.push_back(t);active|=t<1;}
        for(int n=0;n<3;n++){focus[n].target(state.focus==n?1:0,now);state.focusMix[n]=focus[n].value(now);active|=focus[n].active(now);}
        button.target(!state.busy&&!state.username.empty()&&!state.input.empty()?1:0,now);state.buttonMix=button.value(now);active|=button.active(now);
        menu.target(state.menuOpen?1:0,now);keyboard.target(state.keyboardOpen?1:0,now);state.menuAlpha=menu.value(now);state.keyboardAlpha=keyboard.value(now);active|=menu.active(now)||keyboard.active(now);
        double age=now-caretSince,phase=std::fmod(std::max(0.,age),1.2);
        state.caret=true;state.caretAlpha=age<.65?1:phase<.5?1:phase<.65?1-ease((phase-.5)/.15):phase<1.05?0:ease((phase-1.05)/.15);
        active|=state.focus<2&&!state.busy&&state.caretAlpha>0&&state.caretAlpha<1;
        return active;
    }
};
}
