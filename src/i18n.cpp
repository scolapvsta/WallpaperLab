#include "i18n.hpp"
#include <array>
#include <atomic>
#include <map>
#include <nlohmann/json.hpp>
#include "translations_data.h"
namespace lab {
static constexpr const char* codes[]={"it","en","de","es","fr","pt","nl","pl","ro","sv","zh-CN","zh-TW","ja","ko","hi","ru","ar","bn","id","tr","vi","ur"};
static constexpr const char* names[]={"Italiano","English","Deutsch","Español","Français","Português","Nederlands","Polski","Română","Svenska","简体中文","繁體中文","日本語","한국어","हिन्दी","Русский","العربية","বাংলা","Bahasa Indonesia","Türkçe","Tiếng Việt","اردو"};
static std::atomic<int> language{0};
int languageCount(){return 22;}
const char* languageCode(int i){return codes[i>=0&&i<22?i:0];}
const char* languageName(int i){return names[i>=0&&i<22?i:0];}
int languageIndex(const std::string& code){for(int i=0;i<22;i++)if(code==codes[i])return i;return 0;}
void setLanguage(int i){language=i>=0&&i<22?i:0;}
int currentLanguage(){return language;}
const char* translate(const char* key){
    int selected=language.load();if(!key||!*key||selected==0)return key?key:"";
    static const auto catalog=nlohmann::json::parse(translationsData).get<std::map<std::string,std::array<std::string,21>>>();
    auto found=catalog.find(key);return found==catalog.end()?key:found->second.at(selected-1).c_str();
}
}
extern "C" const char* lab_tr(const char* key){return lab::translate(key);}
