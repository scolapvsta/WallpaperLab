#include "i18n.hpp"
#include "translations_data.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace lab;
void require(bool b,const char* msg){if(!b)throw std::runtime_error(msg);}
int main(){try{
    auto data=nlohmann::json::parse(translationsData);std::set<std::string> codes;
    require(languageCount()==22,"expected 22 languages");
    for(int i=0;i<languageCount();i++){
        codes.insert(languageCode(i));require(languageIndex(languageCode(i))==i,"locale roundtrip failed");
        require(*languageName(i),"missing native locale name");setLanguage(i);
        for(auto& [key,values]:data.items())require(std::string(translate(key.c_str()))==(i?values[i-1].get<std::string>():key),"missing or mismatched translated message");
        require(std::string(translate("Workshop title — do not translate"))=="Workshop title — do not translate","user data was translated");
    }
    require(codes.size()==22,"duplicate locale codes");setLanguage(1);auto stable=translate("Libreria");require(std::string(stable)=="Library","English failed");setLanguage(2);require(std::string(translate("Libreria"))=="Bibliothek"&&std::string(stable)=="Library","locale updates invalidated text pointers");
    setLanguage(100);require(currentLanguage()==0,"invalid locale not normalized");
    std::cout<<data.size()<<" messages verified across 22 locales; fallback and live language switching passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
