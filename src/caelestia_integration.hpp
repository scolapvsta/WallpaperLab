#pragma once
#include "catalog.hpp"
#include "local_library.hpp"
#include <regex>
#include <functional>
namespace lab {
inline fs::path caelestiaScheme(){return stateDir().parent_path()/"caelestia/scheme.json";}
inline fs::path caelestiaHelper(){if(auto path=getenv("WALLPAPER_LAB_CAELESTIA_HELPER"))return path;return fs::read_symlink("/proc/self/exe").parent_path()/"caelestia_palette.py";}
inline std::string caelestiaPython(){auto saved=stateDir()/"integrations/caelestia-runtime.json";if(fs::exists(saved)){auto python=readJson(saved).value("python",std::string());if(fs::is_regular_file(python))return python;}return "python3";}
inline void prepareCaelestia(){auto helper=caelestiaHelper();if(!fs::is_regular_file(helper))throw std::runtime_error("Componente Caelestia non trovato nel runtime");auto result=capture({"python3",helper.string(),"--prepare"});if(result.empty())throw std::runtime_error("Impossibile preparare l’integrazione Caelestia");auto data=json::parse(result[0]);if(!data.contains("python"))throw std::runtime_error("Componente Caelestia non disponibile");writeJson(stateDir()/"integrations/caelestia-runtime.json",data);}
inline bool updateCaelestia(const std::string& preview,bool enabled,const std::function<bool()>& mayPublish=[] {return true;},int variant=0,int mode=0){
 auto record=stateDir()/"integrations/caelestia.json",target=caelestiaScheme();
 if(!enabled){
  if(!fs::exists(record))return false;auto saved=readJson(record);
  // Respect manual changes to Caelestia made after our last published palette.
  if(fs::exists(target)&&readJson(target)==saved.value("published",json())){if(saved.value("existed",false))writeJson(target,saved.at("previous"));else fs::remove(target);}
  fs::remove(record);return true;
 }
 if(!fs::is_regular_file(preview))throw std::runtime_error("Anteprima non disponibile per i colori di Caelestia");
 const char* variants[]={"vibrant","tonalspot","expressive","fidelity"},*modes[]={"auto","dark","light"};
 std::vector<std::string> output;if(fs::is_regular_file(caelestiaHelper()))output=capture({caelestiaPython(),caelestiaHelper().string(),"--variant",variants[std::clamp(variant,0,3)],"--mode",modes[std::clamp(mode,0,2)],preview});else if(getenv("WALLPAPER_LAB_CAELESTIA_TEST_LEGACY"))output=capture({"caelestia","wallpaper","--print",preview});
 if(output.empty())throw std::runtime_error("Prepara prima l’integrazione Caelestia dalle impostazioni");auto palette=json::parse(output[0]);
 if(!palette.contains("colours")||!palette["colours"].is_object()||!palette["colours"].contains("primary")||!palette["colours"].contains("surface"))throw std::runtime_error("Palette Caelestia non valida");
 static const std::regex hex("[0-9a-fA-F]{6}");for(auto& [key,value]:palette["colours"].items())if(!value.is_string()||!std::regex_match(value.get<std::string>(),hex))throw std::runtime_error("Palette Caelestia non valida");
 if(!mayPublish())return false;
 json saved;if(fs::exists(record))saved=readJson(record);else saved={{"existed",fs::exists(target)},{"previous",fs::exists(target)?readJson(target):json()}};
 if(fs::exists(target)&&saved.contains("published")&&readJson(target)!=saved["published"]){saved["previous"]=readJson(target);saved["existed"]=true;}
 saved["published"]=palette;writeJson(record,saved);writeJson(target,palette);return true;
}
}
