#pragma once
#include <filesystem>
namespace lab::login {
inline std::filesystem::path astronautAssets(){
    auto installed=std::filesystem::read_symlink("/proc/self/exe").parent_path()/"login-assets/astronaut";
    return std::filesystem::is_directory(installed)?installed:std::filesystem::path("assets/login/astronaut");
}
}
