#pragma once
#include "catalog.hpp"
#include <algorithm>
#include <cmath>
#include <regex>
#include <sstream>
namespace lab {
inline std::string propertyLabel(std::string text,const std::string& fallback) {
    static const std::regex html("<[^>]*>");
    text=std::regex_replace(text,html," ");
    auto first=text.find_first_not_of(" \r\n\t");if(first==std::string::npos)return {};
    text=text.substr(first,text.find_last_not_of(" \r\n\t")-first+1);
    static const std::map<std::string,std::string> labels={{"ui_editor_properties_scale","Formato dello schermo"},{"ui_editor_gizmo_rotate","Rotazione"},{"ui_editor_properties_proportional_size","Dimensione"},{"ui_editor_properties_alpha","Opacità"},{"ui_editor_properties_edges","Bordi"},{"ui_browse_properties_brightness","Luminosità"},{"ui_editor_properties_monochrome","Monocromatico"},{"ui_editor_effect_local_contrast_title","Contrasto locale"},{"ui_browse_context_menu_default","Predefinito"},{"ui_editor_properties_proportional_size","Dimensione"},{"ui_editor_welcome_modal_resize_toggle_label","Scala"},{"ui_browse_monitors_posititon","Posizione"},{"ui_browse_properties_scheme_color","Colore del tema"},{"ui_editor_effect_xray_title","Materiale X-Ray"},{"ui_editor_character_separation_view_background","Materiale dello sfondo"}};
    if(auto i=labels.find(text);i!=labels.end())return i->second;
    return text.starts_with("ui_")?fallback:text;
}
inline json propertyValue(const json& schema,const json& overrides,const std::string& key) {
    return overrides.contains(key)?overrides.at(key):schema.at(key).value("value",json());
}
inline bool propertyVisible(const json& schema,const json& overrides,const std::string& condition) {
    if(condition.empty())return true;
    // Deliberately interpret only comparisons, never execute creator JavaScript.
    static const std::regex comparison(R"(^\s*([a-zA-Z_][a-zA-Z_0-9]*)\.value\s*(==|!=|>=|<=|>|<)\s*([^\s]+)\s*$)");
    std::smatch m;if(!std::regex_match(condition,m,comparison)||!schema.contains(m[1].str()))return true;
    auto left=propertyValue(schema,overrides,m[1]);std::string raw=m[3];json right;
    try{right=json::parse(raw);}catch(...){return true;}
    auto numeric=[](const json& j)->double {if(j.is_boolean())return j.get<bool>()?1:0;if(j.is_number())return j.get<double>();if(j.is_string()){size_t n=0;auto s=j.get<std::string>();double d=std::stod(s,&n);if(n==s.size())return d;}throw std::runtime_error("Not numeric");};
    bool equal=left==right;double a=0,b=0;bool numbers=false;try{a=numeric(left);b=numeric(right);numbers=true;equal=a==b;}catch(...){}
    std::string op=m[2];if(op=="==")return equal;if(op=="!=")return !equal;
    if(!numbers)return true;return op==">"?a>b:op=="<"?a<b:op==">="?a>=b:a<=b;
}
inline json validateProperty(const json& property,json value) {
    auto type=property.value("type",std::string());
    if(type=="bool"&&value.is_boolean())return value;
    if(type=="slider"&&value.is_number()){
        double v=value.get<double>(),lo=property.value("min",0.),hi=property.value("max",100.);
        if(!std::isfinite(v)||v<lo||v>hi)throw std::runtime_error("Valore fuori intervallo");
        if(!property.value("fraction",true))v=std::round(v);
        else {double step=property.value("step",0.);if(std::isfinite(step)&&step>0)v=std::clamp(lo+std::round((v-lo)/step)*step,lo,hi);}
        return v;
    }
    if(type=="combo"&&property.contains("options"))for(auto& option:property.at("options"))if(option.at("value")==value)return value;
    if(type=="textinput"&&value.is_string()&&value.get_ref<const std::string&>().size()<=4096)return value;
    if(type=="color"&&value.is_string()){
        std::istringstream in(value.get<std::string>());double a,b,c;std::string extra;
        if(in>>a>>b>>c&&!(in>>extra)&&std::isfinite(a)&&std::isfinite(b)&&std::isfinite(c)&&a>=0&&a<=1&&b>=0&&b<=1&&c>=0&&c<=1)return value;
    }
    throw std::runtime_error("Proprieta o valore non supportato");
}
inline std::vector<std::string> propertyKeys(const json& schema){
    std::vector<std::string> keys;for(auto& [key,p]:schema.items())if(p.is_object()&&!propertyLabel(p.value("text",key),key).empty())keys.push_back(key);
    std::stable_sort(keys.begin(),keys.end(),[&](auto& a,auto& b){return schema.at(a).value("order",0)<schema.at(b).value("order",0);});return keys;
}
inline std::string propertyArgument(const json& value){return value.is_string()?value.get<std::string>():value.dump();}
}
