#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
namespace lab {
namespace fs = std::filesystem;
using json = nlohmann::json;
struct Item { std::string id, title, type, preview, path, reason; };
std::vector<fs::path> steamLibraries();
std::vector<Item> scan(const std::vector<fs::path>& roots);
json readJson(const fs::path& path);
void writeJson(const fs::path& path, const json& value);
fs::path configDir();
fs::path stateDir();
std::string assetsPath();
std::vector<std::string> capture(const std::vector<std::string>& args);
}
