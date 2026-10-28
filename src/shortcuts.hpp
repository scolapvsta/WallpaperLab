#pragma once
#include "catalog.hpp"
namespace lab {
struct Shortcut {std::string key, mods, combo;int mask=0;};
Shortcut parseShortcut(const std::string& combo);
std::string shortcutConfig(const Shortcut& shortcut,const std::string& launcher,bool lua);
json shortcutSettings();
void configureShortcut(const std::string& combo,bool enabled);
}
