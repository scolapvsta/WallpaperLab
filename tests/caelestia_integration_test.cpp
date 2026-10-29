#include "caelestia_integration.hpp"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
using namespace lab;
void check(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
int main(){auto root=fs::temp_directory_path()/("lab-caelestia-"+std::to_string(getpid()));setenv("XDG_STATE_HOME",root.c_str(),1);setenv("WALLPAPER_LAB_CAELESTIA_TEST_LEGACY","1",1);try{
 auto original=json{{"name","manual"},{"flavour","test"},{"mode","dark"},{"variant","tonalspot"},{"colours",{{"primary","abcdef"},{"surface","112233"}}}};writeJson(caelestiaScheme(),original);updateCaelestia("",false);check(readJson(caelestiaScheme())==original,"disabled integration changed scheme");
 fs::create_directories(root/"bin");auto fake=root/"bin/caelestia";std::ofstream(fake)<<"#!/bin/sh\nprintf '%s\\n' '{\"name\":\"dynamic\",\"mode\":\"dark\",\"flavour\":\"test\",\"variant\":\"tonalspot\",\"colours\":{\"primary\":\"123456\",\"surface\":\"010203\"}}'\n";chmod(fake.c_str(),0700);auto path=(root/"bin").string()+":"+getenv("PATH");setenv("PATH",path.c_str(),1);std::ofstream(root/"preview.png")<<"fixture";
 updateCaelestia((root/"preview.png").string(),true,[]{return false;});check(readJson(caelestiaScheme())==original,"cancelled generation published after disable");
 updateCaelestia((root/"preview.png").string(),true);check(readJson(caelestiaScheme())["colours"]["primary"]=="123456","palette not published");updateCaelestia("",false);check(readJson(caelestiaScheme())==original,"previous scheme not restored");
 updateCaelestia((root/"preview.png").string(),true);auto manual=original;manual["name"]="new-manual";writeJson(caelestiaScheme(),manual);updateCaelestia("",false);check(readJson(caelestiaScheme())==manual,"manual changes overwritten on disable");
 std::ofstream(fake)<<"#!/bin/sh\nprintf '%s\\n' '{\"colours\":{\"primary\":\"invalid\",\"surface\":\"000000\"}}'\n";bool failed=false;try{updateCaelestia((root/"preview.png").string(),true);}catch(...){failed=true;}check(failed&&readJson(caelestiaScheme())==manual,"invalid palette changed scheme");
 fs::remove_all(root);std::cout<<"Optional Caelestia publication, restore, manual override and validation passed\n";return 0;
 }catch(const std::exception& e){fs::remove_all(root);std::cerr<<e.what()<<'\n';return 1;}}
