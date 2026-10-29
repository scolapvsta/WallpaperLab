#pragma once
#include "catalog.hpp"
#include "local_library.hpp"
#include <regex>
#include <functional>
namespace lab {
inline fs::path caelestiaScheme(){return stateDir().parent_path()/"caelestia/scheme.json";}
inline bool updateCaelestia(const std::string& preview,bool enabled,const std::function<bool()>& mayPublish=[] {return true;}){
 auto record=stateDir()/"integrations/caelestia.json",target=caelestiaScheme();
 if(!enabled){
  if(!fs::exists(record))return false;auto saved=readJson(record);
  // Respect manual changes to Caelestia made after our last published palette.
  if(fs::exists(target)&&readJson(target)==saved.value("published",json())){if(saved.value("existed",false))writeJson(target,saved.at("previous"));else fs::remove(target);}
  fs::remove(record);return true;
 }
 if(!executableAvailable("caelestia"))throw std::runtime_error("Caelestia non rilevato");if(!fs::is_regular_file(preview))throw std::runtime_error("Anteprima non disponibile per i colori di Caelestia");
 // --print only computes a palette. Never call wallpaper --file or apply_colours:
 // those paths would also replace the wallpaper or recolour unrelated applications.
 auto output=capture({"caelestia","wallpaper","--print",preview});if(output.empty())throw std::runtime_error("Impossibile generare la palette Caelestia");auto palette=json::parse(output[0]);
 if(!palette.contains("colours")||!palette["colours"].is_object()||!palette["colours"].contains("primary")||!palette["colours"].contains("surface"))throw std::runtime_error("Palette Caelestia non valida");
 static const std::regex hex("[0-9a-fA-F]{6}");for(auto& [key,value]:palette["colours"].items())if(!value.is_string()||!std::regex_match(value.get<std::string>(),hex))throw std::runtime_error("Palette Caelestia non valida");
 if(!mayPublish())return false;
 json saved;if(fs::exists(record))saved=readJson(record);else saved={{"existed",fs::exists(target)},{"previous",fs::exists(target)?readJson(target):json()}};
 if(fs::exists(target)&&saved.contains("published")&&readJson(target)!=saved["published"]){saved["previous"]=readJson(target);saved["existed"]=true;}
 saved["published"]=palette;writeJson(record,saved);writeJson(target,palette);return true;
}
}
