#pragma once
#include "catalog.hpp"
namespace lab::workshop {
// All Steam calls, HTTP transfers and disk writes run on one dedicated worker.
void search(json query);
void download(const std::string& id);
void retry();
json snapshot();
void shutdown();
bool takeInstalled();
}
