#pragma once
#include <fstream>
#include <filesystem>
#include <optional>
#include <nlohmann/json.hpp>
namespace lab {
// Audio follows the saved global gate AND the per-project gate. Failed reads
// preserve the last valid renderer state rather than accidentally unmuting it.
inline std::optional<bool> savedAudioEnabled(const std::filesystem::path& settings,const std::filesystem::path& profiles,const std::string& project){
 try{
  std::ifstream input(settings);nlohmann::json global;input>>global;if(!global.is_object())return {};
  if(global.value("muted",true))return false;
  if(!std::filesystem::exists(profiles))return true;
  std::ifstream local(profiles);nlohmann::json data;local>>data;if(!data.is_object())return {};
  return data.value(project,nlohmann::json::object()).value("playback",nlohmann::json::object()).value("audio",true);
 }catch(...){return {};}
}
}
