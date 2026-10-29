#include "recompxbox/xbe_header.h"
#include <cstddef>
#include <limits>

namespace recompxbox {
namespace {
bool read32(const std::vector<std::uint8_t>& b, std::size_t at, std::uint32_t& v) {
    if (at > b.size() || b.size() - at < 4) return false;
    v = static_cast<std::uint32_t>(b[at]) |
        (static_cast<std::uint32_t>(b[at+1]) << 8) |
        (static_cast<std::uint32_t>(b[at+2]) << 16) |
        (static_cast<std::uint32_t>(b[at+3]) << 24);
    return true;
}
}
bool parseXbe(const std::vector<std::uint8_t>& b, XbeHeader& header,
              std::vector<XbeSection>& sections, std::string& error) {
    error.clear();
    header = {};
    sections.clear();
    if (b.size() < 0x180 || b[0] != 'X' || b[1] != 'B' ||
        b[2] != 'E' || b[3] != 'H') {
        error = "Missing or truncated XBEH header";
        return false;
    }
    XbeHeader h;
    if (!read32(b, 0x104, h.baseAddress) ||
        !read32(b, 0x108, h.headerSize) ||
        !read32(b, 0x10C, h.imageSize) ||
        !read32(b, 0x11C, h.sectionCount) ||
        !read32(b, 0x120, h.sectionTableAddress) ||
        !read32(b, 0x128, h.entryPointEncoded) ||
        !read32(b, 0x158, h.kernelThunkEncoded)) {
        error = "Truncated XBE metadata";
        return false;
    }
    if (h.headerSize < 0x180 || h.headerSize > b.size() ||
        h.imageSize < h.headerSize || h.sectionCount > 4096 ||
        h.sectionTableAddress < h.baseAddress) {
        error = "Invalid XBE metadata bounds";
        return false;
    }
    const std::uint64_t tableOffset =
        static_cast<std::uint64_t>(h.sectionTableAddress) - h.baseAddress;
    const std::uint64_t tableBytes = static_cast<std::uint64_t>(h.sectionCount) * 0x38;
    if (tableOffset > h.headerSize || tableBytes > h.headerSize - tableOffset) {
        error = "Section table outside XBE headers";
        return false;
    }
    std::vector<XbeSection> parsed;
    for (std::uint32_t i = 0; i < h.sectionCount; ++i) {
        const auto off = static_cast<std::size_t>(tableOffset + i * 0x38u);
        XbeSection s;
        // XBE section: flags +0, virtual address +4, virtual size +8,
        // raw offset +12, raw size +16.
        if (!read32(b, off + 4, s.virtualAddress) ||
            !read32(b, off + 8, s.virtualSize) ||
            !read32(b, off + 12, s.rawOffset) ||
            !read32(b, off + 16, s.rawSize)) {
            error = "Truncated XBE section";
            return false;
        }
        if (s.rawOffset > b.size() || s.rawSize > b.size() - s.rawOffset ||
            s.virtualAddress < h.baseAddress ||
            static_cast<std::uint64_t>(s.virtualAddress) - h.baseAddress +
                s.virtualSize > h.imageSize) {
            error = "XBE section outside file or image";
            return false;
        }
        parsed.push_back(s);
    }
    header = h;
    sections = std::move(parsed);
    return true;
}
}
