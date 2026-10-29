#include "recompxbox/xbe_image.h"
#include <algorithm>
#include <cstring>
#include <utility>

namespace recompxbox {
bool loadXbeImage(const std::vector<std::uint8_t>& file, XbeImage& image,
                  std::string& error) {
    image = {};
    XbeHeader header;
    std::vector<XbeSection> sections;
    if (!parseXbe(file, header, sections, error)) return false;
    // Guard against untrusted XBE files requesting multi-gigabyte allocations.
    constexpr std::uint32_t kMaxImage = 256u * 1024u * 1024u;
    if (header.imageSize > kMaxImage) {
        error = "XBE image exceeds 256 MiB safety limit";
        return false;
    }
    XbeImage candidate;
    candidate.header = header;
    candidate.sections = std::move(sections);
    candidate.memory.resize(header.imageSize, 0);
    std::memcpy(candidate.memory.data(), file.data(), header.headerSize);
    for (const auto& section : candidate.sections) {
        // Sections are copied into guest virtual-address-relative offsets.
        const auto dest = static_cast<std::size_t>(section.virtualAddress - header.baseAddress);
        if (section.rawSize > section.virtualSize) {
            error = "XBE section raw size exceeds virtual size";
            return false;
        }
        if (section.rawSize != 0)
            std::memcpy(candidate.memory.data() + dest,
                        file.data() + section.rawOffset, section.rawSize);
    }
    image = std::move(candidate);
    return true;
}
}
