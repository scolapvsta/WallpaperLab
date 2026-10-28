#pragma once
#include "catalog.hpp"
namespace lab {
// All controller/configuration mutations share this inter-process lock.
class ControllerLock {
    int fd = -1;
public:
    ControllerLock();
    ~ControllerLock();
    ControllerLock(const ControllerLock&) = delete;
    ControllerLock& operator=(const ControllerLock&) = delete;
};
void trashProject(const fs::path& path);
json loadPlacements();
json wallpaperPosition(const json& profiles,const std::string& path);
void saveWallpaperPosition(const std::string& path,double x,double y);
json loadSettings();
json loadCarousel();
void changeCarousel(const std::string& path, int operation);
std::vector<int> carouselItems(const json& list, const std::vector<Item>& items);
int nextCarouselItem(const json& list, const std::vector<Item>& items, const std::string& current, int direction);
}
