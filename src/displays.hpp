#pragma once
#include "catalog.hpp"
namespace lab {
struct Display {
    std::string name,model;
    int width=0,height=0;
    double x=0,y=0,logicalWidth=0,logicalHeight=0,mmWidth=0,mmHeight=0;
    bool measured=false;
};
struct DisplayRect { std::string name;double x=0,y=0,w=0,h=0; };
struct DisplayCanvas { std::string basis="physical";std::vector<DisplayRect> rects; };
std::vector<Display> parseDisplays(const json& list);
DisplayCanvas automaticCanvas(const std::vector<Display>& displays,const std::string& basis);
DisplayCanvas displayCanvas(const std::vector<Display>& displays,const json& settings);
json canvasSettings(const DisplayCanvas& canvas);
std::string canvasError(const DisplayCanvas& canvas);
void moveDisplay(DisplayCanvas& canvas,int index,double x,double y,double snap);
void alignDisplays(DisplayCanvas& canvas,int edge);
json renderLayout(const DisplayCanvas& canvas,const std::vector<Display>& displays);
}
