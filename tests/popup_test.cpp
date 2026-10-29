#include "popup.hpp"
#include "popup_layout.h"
#include "library_layout.h"
#include "popup_animation.h"
#include <iostream>
#include <unistd.h>
using namespace lab;
void check(bool b,const char* text){if(!b)throw std::runtime_error(text);}
int main(){
    auto root=fs::temp_directory_path()/("wallpaper-popup-test-"+std::to_string(getpid()));setenv("XDG_STATE_HOME",root.c_str(),1);
    try{
        for(int frames:{30,60,120}) {
            float opening=0,closing=1;
            for(int n=0;n<frames;n++){
                opening=popup_visibility(opening,0,1.f/frames);
                closing=popup_visibility(closing,1,1.f/frames);
                check(fabsf(popup_ease(opening)+popup_ease(closing)-1)<0.0001f,"entry and exit fade must be symmetric");
            }
            check(opening==1&&closing==0,"fade failed to finish");
        }
        auto halfway=popup_visibility(0,0,0.07f);
        check(fabsf(halfway-0.5f)<0.0001f&&popup_visibility(halfway,1,0.07f)==0,"mid-entry release must reverse without flashing");
        PopupMotion zoom={0,0};for(int n=0;n<45;n++)popup_motion(&zoom,1,0.01f);
        check(zoom.value>0.98f&&zoom.value<=1,"selected zoom did not settle");
        float previous=zoom.value;popup_motion(&zoom,0,0.01f);
        check(zoom.value>0&&zoom.value<previous,"selection reversal must be continuous");
        for(int fps:{30,60,120}){
            PopupMotion motion={0,0};float last=0;
            for(int n=0;n<fps;n++){
                float target=n<fps/3?1:n<2*fps/3?2:5;
                popup_motion(&motion,target,1.f/fps);
                check(motion.value>=last&&motion.value<=target,"rapid forward taps reversed or overshot");last=motion.value;
            }
            check(motion.value>4.8f,"continuous motion did not catch up with rapid taps");
        }
        for(int count:{1,2,3,4,21})for(int step=0;step<3000;step++){
            float phase=step*0.037f,sum=0;
            for(int i=0;i<count;i++){
                float d=popup_distance(i,phase,count),focus=popup_focus(d);
                check(focus>=0&&focus<=1,"rapid focus escaped bounds");
                check(fabsf(focus-popup_focus(popup_distance(i,phase+count,count)))<0.0001f,"cyclic focus changed across wrap");
                sum+=focus;
            }
            check(fabsf(sum-1)<0.0002f,"rapid scrolling created multiple growing focus envelopes");
        }
        for(int width=250;width<2200;width++){
            auto grid=library_grid_layout(width,246);
            check(grid.columns>0&&grid.tile>=210&&grid.tile<=300,"resizing produced unreadable library tiles");
            check(grid.columns*grid.tile+(grid.columns-1)*22+40<=width+0.1f,"library columns overflow available width");
        }
        check(library_grid_layout(790,246).columns==3,"grid should shrink tiles before dropping a column");
        auto threeH=popup_layout(1,3,800,640),threeV=popup_layout(2,3,800,640);
        check(threeH.width==3*threeH.tile&&threeV.height==3*threeV.tile,"three-card river must fit complete settled thumbnails");
        auto threeGrid=popup_layout(0,3,800,640);check(threeGrid.columns*threeGrid.rows==4,"three favorites require one explicit empty cell");
        auto four=popup_layout(0,4,800,640),nine=popup_layout(0,9,800,640);
        check(four.columns==2&&four.rows==2&&four.width==320&&four.height==320,"four favorites must form a borderless 2x2");
        check(nine.columns==3&&nine.rows==3&&nine.width==480&&nine.height==480,"nine favorites must form a 3x3");
        check(popup_layout(1,4,800,640).height==160&&popup_layout(2,4,800,640).width==160,"river must remain a slim strip");
        for(int mode=0;mode<4;mode++)for(int count:{0,1,3,4,9,21,2048})for(int limit:{100,640}){
            auto p=popup_layout(mode,count,limit,limit);
            check(p.width>0&&p.height>0&&p.width<=limit&&p.height<=limit,"popup escaped monitor bounds");
            if(mode==0&&count>0)check(p.columns*p.rows>0&&p.width==p.columns*p.tile&&p.height==p.rows*p.tile,"grid must cover its exact tile area");
            auto frame=popup_frame(mode,count,limit,limit);
            check(frame.width<=limit&&frame.height<=limit,"overflow surface escaped monitor bounds");
            if(count>0)check(frame.padding>=frame.content.tile*0.19f+2.6f,"enlarged card border would be clipped at the panel edge");
        }
        auto state=mergePopupEvent(json::object(),"test",1,1,false);
        state=mergePopupEvent(state,"test",1,8,false);state=mergePopupEvent(state,"test",1,2,false);
        check(state["steps"]==8,"out-of-order step regressed selection");
        state=mergePopupEvent(state,"test",1,10,true);state=mergePopupEvent(state,"test",1,9,false);
        check(state["steps"]==10&&state["commit"],"late step reopened released session");
        state["done"]=true;check(mergePopupEvent(state,"test",1,11,false)==state,"completed session revived");
        auto newer=mergePopupEvent(state,"test",2,1,false);check(!newer["commit"].get<bool>()&&!newer["done"].get<bool>(),"new session not reset");
        check(mergePopupEvent(newer,"test",1,99,true)==newer,"old release affected new session");
        check(popupSelection(3,0,1)==0&&popupSelection(3,0,4)==0&&popupSelection(3,-1,1)==0,"wrap/initial selection");
        check(popupSelection(3,2,1)==2&&popupSelection(3,2,2)==0&&popupSelection(3,2,3)==1,"open on active and advance in saved order");
        check(popupSelection(0,-1,10)==-1&&popupSelection(1,0,100)==0,"empty/single list");
        PopupHost first,second;
        check(first.receive("test",1,1,false),"first host not created");
        check(!second.receive("test",1,2,false),"duplicate popup owner");
        auto seen=popupState();check(!first.finish(seen),"completed session requested restart");
        check(!second.receive("test",1,2,true),"late release recreated popup");
        check(second.receive("test",2,3,true),"fast-release session lost");
        check(popupState()["commit"].get<bool>(),"release-before-window lost");
        PopupHost third;check(!third.receive("test",3,1,false),"new generation duplicated running popup");
        check(second.finish(mergePopupEvent(json::object(),"test",2,3,true)),"newer session wakeup lost");
        check(third.resume(),"queued session not resumable");third.finish(popupState());
        fs::remove_all(root);std::cout<<"Popup ordering, rapid steps, early release, singleton and handoff passed\n";
    }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}
}
