#pragma once
#include "catalog.hpp"
#include <array>
namespace lab::login {
inline constexpr std::array<const char*,3> widgets={"clock","date","login"};
json defaults();
json resetUi(json document,int widget=-1);
json applyPalette(json document,const unsigned char* rgb,int width,int height);
json colors(const json& document);
json restoreColors(json document,const json& colors);
json makePreset(const json& document,const std::string& name,bool full);
json applyPreset(json document,const json& preset,bool full,const std::vector<Item>& library={});
fs::path presetDirectory(bool full);
std::vector<json> listPresets(bool full);
void savePreset(const json& document,const std::string& name,bool full);
std::string sourceFor(const json& document,const std::string& output="");
json validate(json document);
json load();
void save(const json& document);
json copyLibrarySettings(json document,const Item& item,const json& libraryProfiles);
json projectProfile(const json& document,const std::string& source,const json& libraryProfiles);
json imageProfile(const json& document,const std::string& output="");
bool supported(const Item& item);
std::string unsupportedReason(const Item& item);
std::array<double,2> snapPosition(const json& document,int widget,double x,double y);
fs::path prepare(const json& document,const Item& item,const json& desktop,const std::vector<std::string>& outputs,const std::vector<Item>& library={});
std::vector<std::string> renderArguments(const json& prepared,bool windowed=false);
}
