#include "project_properties.hpp"
#include "WallpaperEngine/Render/Helpers/WallpaperPlacement.h"
#include <iostream>
using namespace lab;
static void check(bool b){if(!b)throw std::runtime_error("Project property regression");}
int main(){
 json schema={{"shape",{{"type","combo"},{"value","1"},{"order",1},{"text","Shape"},{"options",json::array({{{"label","Square"},{"value","1"}},{{"label","Circle"},{"value","2"}}})}}},{"amount",{{"type","slider"},{"value",.3},{"min",0},{"max",1},{"step",.1},{"order",2},{"text","Amount"}}}};
 check(propertyKeys(schema).at(0)=="shape");check(propertyVisible(schema,json::object(),"shape.value == 1"));
 check(!propertyVisible(schema,{{"shape","2"}},"shape.value == 1"));check(propertyVisible(schema,{},"arbitraryFunction()"));
 check(validateProperty(schema["amount"],.54)==.5);bool rejected=false;try{validateProperty(schema["amount"],2);}catch(...){rejected=true;}check(rejected);
 rejected=false;try{validateProperty(schema["shape"],"3");}catch(...){rejected=true;}check(rejected);
 check(validateProperty({{"type","bool"}},false)==false);check(propertyLabel("<img src='https://invalid/'>","image").empty());
 namespace place=WallpaperEngine::Render::Helpers::WallpaperPlacement;
 check(place::cropShift(1,.8)==0);check(std::abs(place::cropShift(.5,1)+.25)<1e-6);
 float a=0,b=1;place::zoomRange(a,b,2);check(a==.25&&b==.75);
 for(float offset:{-1.f,0.f,1.f}){auto shift=place::cropShift(b-a,offset);check(a+shift>=0&&b+shift<=1);}
 check(place::parse("{\"x\":0,\"y\":0,\"zoom\":2}").zoom==2);
 std::cout<<"Creator properties, conditions, validation and bounded crop/zoom passed\n";
}
