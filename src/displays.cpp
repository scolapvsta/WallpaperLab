#include "displays.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
namespace lab {
std::vector<Display> parseDisplays(const json& list){
    std::vector<Display> result;std::set<std::string> seen;
    if(!list.is_array())return result;
    for(auto& row:list){
        try{
            if(row.value("disabled",false))continue;
            Display d;d.name=row.at("name");d.model=row.value("model",d.name);
            d.width=row.at("width");d.height=row.at("height");if(d.width<1||d.height<1||d.name.empty()||!seen.insert(d.name).second)continue;
            d.mmWidth=row.value("physicalWidth",0.0);d.mmHeight=row.value("physicalHeight",0.0);
            if(row.value("transform",0)%2){std::swap(d.width,d.height);std::swap(d.mmWidth,d.mmHeight);}
            d.measured=d.mmWidth>=50&&d.mmHeight>=50&&d.mmWidth<=3000&&d.mmHeight<=3000;
            if(!d.measured){d.mmWidth=d.width*25.4/96;d.mmHeight=d.height*25.4/96;}
            double scale=std::max(0.25,row.value("scale",1.0));
            d.logicalWidth=d.width/scale;d.logicalHeight=d.height/scale;
            d.x=row.value("x",0.0);d.y=row.value("y",0.0);result.push_back(d);
        }catch(...){}
    }
    std::sort(result.begin(),result.end(),[](auto&a,auto&b){return a.x==b.x?a.y<b.y:a.x<b.x;});
    return result;
}
DisplayCanvas automaticCanvas(const std::vector<Display>& displays,const std::string& basis){
    DisplayCanvas c;c.basis=basis=="pixels"?"pixels":"physical";
    for(size_t i=0;i<displays.size();i++){
        auto& d=displays[i];DisplayRect r{d.name,0,0,c.basis=="physical"?d.mmWidth:d.logicalWidth,c.basis=="physical"?d.mmHeight:d.logicalHeight};
        if(c.basis=="pixels"){r.x=d.x;r.y=d.y;}
        else if(i){
            // Preserve compositor adjacency, while replacing pixel lengths by
            // panel lengths. Exact top/bottom/center alignments stay exact.
            size_t anchor=0;double best=std::numeric_limits<double>::max();
            for(size_t j=0;j<i;j++){
                auto& a=displays[j];double dx=std::max({a.x-d.x-d.logicalWidth,d.x-a.x-a.logicalWidth,0.0});
                double dy=std::max({a.y-d.y-d.logicalHeight,d.y-a.y-a.logicalHeight,0.0});
                double score=dx*dx+dy*dy+std::abs(d.x-a.x)*0.0001;
                if(score<best){best=score;anchor=j;}
            }
            auto& a=displays[anchor];auto& p=c.rects[anchor];
            if(d.x>=a.x+a.logicalWidth-2)r.x=p.x+p.w+(d.x-a.x-a.logicalWidth)*p.w/a.logicalWidth;
            else if(d.x+d.logicalWidth<=a.x+2)r.x=p.x-r.w+(d.x+d.logicalWidth-a.x)*p.w/a.logicalWidth;
            else if(std::abs(d.x-a.x)<2)r.x=p.x;
            else if(std::abs(d.x+d.logicalWidth-a.x-a.logicalWidth)<2)r.x=p.x+p.w-r.w;
            else r.x=p.x+(d.x-a.x)*p.w/a.logicalWidth;
            if(d.y>=a.y+a.logicalHeight-2)r.y=p.y+p.h+(d.y-a.y-a.logicalHeight)*p.h/a.logicalHeight;
            else if(d.y+d.logicalHeight<=a.y+2)r.y=p.y-r.h+(d.y+d.logicalHeight-a.y)*p.h/a.logicalHeight;
            else if(std::abs(d.y-a.y)<2)r.y=p.y;
            else if(std::abs(d.y+d.logicalHeight-a.y-a.logicalHeight)<2)r.y=p.y+p.h-r.h;
            else if(std::abs(d.y+d.logicalHeight/2-a.y-a.logicalHeight/2)<2)r.y=p.y+(p.h-r.h)/2;
            else r.y=p.y+(d.y-a.y)*p.h/a.logicalHeight;
        }
        c.rects.push_back(r);
    }
    if(!c.rects.empty()){
        double x=c.rects[0].x,y=c.rects[0].y;for(auto& r:c.rects){x=std::min(x,r.x);y=std::min(y,r.y);}
        for(auto& r:c.rects){r.x-=x;r.y-=y;}
    }
    return c;
}
DisplayCanvas displayCanvas(const std::vector<Display>& displays,const json& settings){
    auto profile=settings.value("display_layout",json::object());
    auto c=automaticCanvas(displays,profile.value("basis",std::string("physical")));
    if(profile.contains("positions")&&profile["positions"].is_object())for(auto& r:c.rects){
        if(!profile["positions"].contains(r.name))continue;
        auto pos=profile["positions"][r.name];
        if(pos.is_array()&&pos.size()==2&&pos[0].is_number()&&pos[1].is_number()){
            double x=pos[0],y=pos[1];if(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<=50000&&std::abs(y)<=50000){r.x=x;r.y=y;}
        }
    }
    return c;
}
json canvasSettings(const DisplayCanvas& c){
    json positions=json::object();for(auto& r:c.rects)positions[r.name]=json::array({r.x,r.y});
    return {{"version",1},{"basis",c.basis},{"positions",positions}};
}
std::string canvasError(const DisplayCanvas& c){
    if(c.rects.empty())return "Nessun monitor disponibile.";
    double minX=1e9,minY=1e9,maxX=-1e9,maxY=-1e9;
    for(size_t i=0;i<c.rects.size();i++){
        auto& r=c.rects[i];
        if(!std::isfinite(r.x)||!std::isfinite(r.y)||!std::isfinite(r.w)||!std::isfinite(r.h)||r.w<=0||r.h<=0||std::abs(r.x)>50000||std::abs(r.y)>50000)return "Posizione del monitor non valida.";
        minX=std::min(minX,r.x);minY=std::min(minY,r.y);maxX=std::max(maxX,r.x+r.w);maxY=std::max(maxY,r.y+r.h);
        for(size_t j=0;j<i;j++){
            auto& b=c.rects[j];
            if(std::min(r.x+r.w,b.x+b.w)-std::max(r.x,b.x)>0.01&&std::min(r.y+r.h,b.y+b.h)-std::max(r.y,b.y)>0.01)return "I monitor si sovrappongono. Trascinali uno accanto all'altro.";
        }
    }
    if(maxX-minX>(c.basis=="physical"?6000:16384)||maxY-minY>(c.basis=="physical"?6000:16384))return "I monitor sono troppo distanti. Avvicinali nella mappa.";
    return {};
}
void moveDisplay(DisplayCanvas& c,int index,double x,double y,double snap){
    if(index<0||index>=(int)c.rects.size()||!std::isfinite(x)||!std::isfinite(y))return;
    auto& r=c.rects[index];double sx=x,sy=y,dx=snap,dy=snap;
    if(snap>0)for(int j=0;j<(int)c.rects.size();j++)if(j!=index){
        auto& b=c.rects[j];
        for(double candidate:{b.x,b.x+b.w,b.x-r.w,b.x+b.w-r.w,b.x+(b.w-r.w)/2})if(std::abs(candidate-x)<dx){sx=candidate;dx=std::abs(candidate-x);}
        for(double candidate:{b.y,b.y+b.h,b.y-r.h,b.y+b.h-r.h,b.y+(b.h-r.h)/2})if(std::abs(candidate-y)<dy){sy=candidate;dy=std::abs(candidate-y);}
    }
    r.x=std::clamp(sx,-50000.0,50000.0);r.y=std::clamp(sy,-50000.0,50000.0);
}
void alignDisplays(DisplayCanvas& c,int edge){
    if(c.rects.empty())return;
    auto main=std::max_element(c.rects.begin(),c.rects.end(),[](auto&a,auto&b){return a.h<b.h;});
    double top=main->y,height=main->h;
    for(auto& r:c.rects)r.y=top+(edge==2?height-r.h:edge==1?(height-r.h)/2:0);
}
json renderLayout(const DisplayCanvas& c,const std::vector<Display>& displays){
    auto error=canvasError(c);if(!error.empty())throw std::runtime_error(error);
    double minX=1e9,minY=1e9,scale=1;
    for(auto& r:c.rects){minX=std::min(minX,r.x);minY=std::min(minY,r.y);}
    if(c.basis=="physical")for(auto& d:displays)scale=std::max({scale,d.width/d.mmWidth,d.height/d.mmHeight});
    json out=json::object();int fullW=0,fullH=0;
    for(auto& r:c.rects){
        int x=std::lround((r.x-minX)*scale),y=std::lround((r.y-minY)*scale);
        int right=std::lround((r.x+r.w-minX)*scale),bottom=std::lround((r.y+r.h-minY)*scale);
        if(right<=x||bottom<=y)throw std::runtime_error("Dimensioni del panorama non valide");
        out[r.name]=json::array({x,y,right-x,bottom-y});fullW=std::max(fullW,right);fullH=std::max(fullH,bottom);
    }
    if(fullW>16384||fullH>16384)throw std::runtime_error("Panorama troppo grande: avvicina i monitor nella mappa.");
    return out;
}
}
