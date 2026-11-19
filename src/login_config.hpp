#pragma once
#include "catalog.hpp"
#include <array>
namespace lab::login {
inline constexpr std::array<const char*,6> widgets={"clock","date","calendar","media","message","login"};
inline constexpr std::array<const char*,5> templates={"Orbit","Horizon","Neon","Paper","Canvas"};
json defaults();
json applyTemplate(json document,int preset);
json validate(json document);
json load();
void save(const json& document);
json imageProfile(const json& document);
bool supported(const Item& item);
fs::path prepare(const json& document,const Item& item,const json& desktop,const std::vector<std::string>& outputs);
std::vector<std::string> renderArguments(const json& prepared,bool windowed=false);
}
