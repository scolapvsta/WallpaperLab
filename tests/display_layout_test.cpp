#include "displays.hpp"
#include "WallpaperEngine/Render/Helpers/SpanGeometry.h"
#include <cmath>
#include "WallpaperEngine/Render/Helpers/WallpaperPlacement.h"
#include <iostream>
using namespace lab;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
template<class F>void rejects(F f,const char* message){bool failed=false;try{f();}catch(...){failed=true;}require(failed,message);}
int main(){try{
    namespace placement=WallpaperEngine::Render::Helpers::WallpaperPlacement;
    auto pan=placement::parse("{\"x\":0.4,\"y\":-0.5}");
    require(std::abs(placement::shift(1,pan.x)+0.2)<0.00001,"positive horizontal pan must sample earlier pixels");
    require(placement::shift(-1,pan.y)==-0.25,"vertical flip did not preserve pan direction");
    rejects([]{placement::parse("{\"x\":2,\"y\":0}");},"out of range renderer position accepted");
    rejects([]{placement::parse("{\"x\":\"0\",\"y\":0}");},"nonnumeric renderer position accepted");
    auto source=json::array({
        {{"name","right"},{"model","QHD"},{"width",2560},{"height",1440},{"x",1920},{"y",0},{"scale",1},{"physicalWidth",600},{"physicalHeight",330}},
        {{"name","left"},{"width",1920},{"height",1080},{"x",0},{"y",0},{"physicalWidth",530},{"physicalHeight",300}}
    });
    auto data=parseDisplays(source);require(data.size()==2&&data[0].name=="left","connector ordering failed");
    auto physical=automaticCanvas(data,"physical");
    require(physical.rects[1].x==530&&physical.rects[0].y==physical.rects[1].y,"physical top alignment or adjacency failed");
    auto payload=renderLayout(physical,data);
    require(payload["left"][3]==1309&&payload["right"][3]==1440,"different pixel density was not compensated");
    require(payload["left"][2]==payload["right"][0],"rounding introduced a seam gap");
    // The same sampled panorama line must occupy the same physical height.
    double leftMM=360.0/payload["left"][3].get<int>()*300,rightMM=360.0/payload["right"][3].get<int>()*330;
    require(std::abs(leftMM-rightMM)<0.02,"physical seam continuity failed");
    auto pixels=automaticCanvas(data,"pixels");auto p=renderLayout(pixels,data);
    require(p["left"]==json::array({0,0,1920,1080})&&p["right"]==json::array({1920,0,2560,1440}),"logical pixel fallback changed");
    alignDisplays(physical,2);require(physical.rects[0].y==30&&physical.rects[1].y==0,"bottom alignment failed");
    alignDisplays(physical,1);require(physical.rects[0].y==15,"center alignment failed");
    alignDisplays(physical,0);moveDisplay(physical,1,534,3,8);require(physical.rects[1].x==530&&physical.rects[1].y==0,"edge snapping failed");
    moveDisplay(physical,1,600,-100,0);require(physical.rects[1].y==-100,"negative offset was not retained");
    auto settings=json{{"display_layout",canvasSettings(physical)}};
    auto restored=displayCanvas(data,settings);require(restored.rects[1].x==600&&restored.rects[1].y==-100,"named positions did not round trip");
    auto negative=renderLayout(restored,data);require(negative["right"][1]==0&&negative["left"][1].get<int>()>0,"normalization of negative positions failed");
    std::reverse(data.begin(),data.end());auto reordered=displayCanvas(data,settings);
    require(reordered.rects[0].name=="right"&&reordered.rects[0].x==600,"profile followed indices instead of connectors");
    moveDisplay(restored,1,100,0,0);require(!canvasError(restored).empty(),"overlapping displays accepted");
    rejects([&]{renderLayout(restored,data);},"overlap reached renderer");
    source[0]["scale"]=2;source[0]["transform"]=1;auto rotated=parseDisplays(source);
    require(rotated[1].width==1440&&rotated[1].height==2560&&rotated[1].mmHeight==600&&rotated[1].logicalWidth==720,"rotation or fractional scaling mishandled");
    source[0]["physicalWidth"]=0;auto estimated=parseDisplays(source);require(!estimated[1].measured&&estimated[1].mmWidth>0,"missing EDID dimensions not isolated");
    using namespace WallpaperEngine::Render::Helpers;
    auto decoded=SpanGeometry::parse(payload.dump());require(decoded.at("left").w==1309,"renderer snapshot parser mismatch");
    rejects([]{SpanGeometry::parse("{\"x\":[0,0,0,1080]}");},"zero rectangle accepted");
    rejects([]{SpanGeometry::parse("{\"x\":[0,0,1.2,1080]}");},"fractional render rectangle accepted");
    rejects([]{SpanGeometry::parse("{\"x\":[16000,0,1920,1080]}");},"oversized canvas accepted");
    rejects([]{SpanGeometry::parse("{}");},"empty explicit calibration accepted");
    std::cout<<"Physical display layout, snapping, alignments, persistence and renderer bridge passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
