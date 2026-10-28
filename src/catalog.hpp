#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
namespace lab {
namespace fs = std::filesystem;
using json = nlohmann::json;
struct ResolutionVariant {int width=0,height=0;std::string property,value,label;};
struct Item { std::string id, title, type, preview, path, reason; int width=0,height=0,audio=-1; std::vector<std::pair<int,int>> supported; std::vector<ResolutionVariant> variants; json properties=json::object(); std::vector<std::string> propertyOrder; json source; };
struct ResolutionFilter {int code=0,width=0,height=0,count=0;std::string label;int group=0;};
std::vector<ResolutionFilter> resolutionFilters(const std::vector<Item>& items);
bool resolutionMatches(const Item& item,const ResolutionFilter& filter);
bool matchesCriteria(const Item& item,const std::vector<ResolutionFilter>& filters,int type,int audio,const std::vector<int>& codes,bool any);
bool resolutionAll(const Item& item,const std::vector<ResolutionFilter>& filters,const std::vector<int>& codes);
bool matches(const Item& item,int type,int resolution,int audio);
std::vector<fs::path> steamLibraries();
std::vector<Item> scan(const std::vector<fs::path>& roots,bool probeVideos=false);
std::vector<Item> scanProjects(const std::vector<fs::path>& projects,bool probeVideos=false);
json readJson(const fs::path& path);
void writeJson(const fs::path& path, const json& value);
fs::path configDir();
fs::path stateDir();
std::string assetsPath();
std::vector<std::string> capture(const std::vector<std::string>& args);
}
