#pragma once
#include "recompxbox/xbe_image.h"
#include <string>
namespace recompxbox {
bool sendXbeToQemu(const XbeImage& image, const std::string& endpoint, std::string& error, bool cpuTrace = false, bool cpuStep = false);
}
