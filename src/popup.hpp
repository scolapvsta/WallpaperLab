#pragma once
#include "catalog.hpp"
namespace lab {
// Absolute counters keep rapid/reordered compositor deliveries idempotent.
json mergePopupEvent(json current,const std::string& epoch,long long session,int steps,bool commit);
int popupSelection(int count,int current,int steps);
class PopupHost {
    int fd=-1;
public:
    ~PopupHost();
    bool receive(const std::string& epoch,long long session,int steps,bool commit);
    bool resume();
    // Seals only the session seen by the UI, releases ownership atomically,
    // and reports a newer session needing a fresh host.
    bool finish(const json& seen);
};
json popupState();
}
