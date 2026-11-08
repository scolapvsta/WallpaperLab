#include "render_gpu.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>
namespace fs=std::filesystem;
static void check(bool ok){if(!ok)throw std::runtime_error("Render GPU regression");}
int main(){
 auto root=fs::temp_directory_path()/("wallpaper-gpus-"+std::to_string(getpid()));
 try{
  fs::create_directories(root/"by-path");
  for(auto name:{"pci-0000:03:00.0-render","pci-0000:03:00.0-card","pci-0000:12:00.0-render","platform-simple-framebuffer.0-card"})std::ofstream(root/"by-path"/name);
  fs::create_directories(root/"pci/0000:03:00.0");std::ofstream(root/"pci/0000:03:00.0/vendor")<<"0x1002\n";std::ofstream(root/"pci/0000:03:00.0/device")<<"0x744c\n";
  fs::create_directories(root/"pci/0000:12:00.0");std::ofstream(root/"pci/0000:12:00.0/vendor")<<"0x1002\n";std::ofstream(root/"pci/0000:12:00.0/device")<<"0x164e\n";
  std::ofstream(root/"pci.ids")<<"# comment\n1002  Advanced Micro Devices, Inc. [AMD/ATI]\n\t744c  Navi 31 [Radeon RX 7900 XT/7900 XTX]\n\t\t1002 0e3b  subsystem\n";
  auto gpus=lab::renderGpus(root/"by-path",root/"pci",root/"pci.ids");
  check(gpus.size()==2);
  check(gpus[0].id=="pci-0000_03_00_0"&&gpus[0].label=="Radeon RX 7900 XT/7900 XTX (0000:03:00.0)");
  check(gpus[1].id=="pci-0000_12_00_0"&&gpus[1].label=="1002:164e (0000:12:00.0)");
  check(lab::driPrimeFor("pci-0000_12_00_0",gpus)=="pci-0000_12_00_0");
  check(lab::driPrimeFor("",gpus).empty()&&lab::driPrimeFor("pci-0000_99_00_0",gpus).empty());
  check(lab::renderGpus(root/"missing").empty());
  fs::remove_all(root);std::cout<<"Render node listing, PCI names and DRI_PRIME fallback passed\n";
 }catch(...){fs::remove_all(root);throw;}
}
