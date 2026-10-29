#pragma once
#include "recompxbox/xbe_header.h"
#include <cstdint>
#include <string>
#include <vector>

namespace recompxbox {
struct XbeImage {
    XbeHeader header;
    std::vector<XbeSection> sections;
    // Host-side copy of the Xbox virtual image. Not executable guest memory.
    std::vector<std::uint8_t> memory;
};
bool loadXbeImage(const std::vector<std::uint8_t>& file, XbeImage& image,
                  std::string& error);
}
