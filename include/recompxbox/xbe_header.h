#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace recompxbox {
// Minimal XBE image metadata, decoded from on-disk little-endian bytes.
struct XbeHeader {
    std::uint32_t baseAddress = 0;
    std::uint32_t imageSize = 0;
    std::uint32_t headerSize = 0;
    std::uint32_t sectionCount = 0;
    std::uint32_t sectionTableAddress = 0;
    std::uint32_t entryPointEncoded = 0;
    std::uint32_t kernelThunkEncoded = 0;
};
struct XbeSection {
    std::uint32_t virtualAddress = 0;
    std::uint32_t virtualSize = 0;
    std::uint32_t rawOffset = 0;
    std::uint32_t rawSize = 0;
};
// Parses header and section table only. Does not decrypt entrypoint or execute XBE.
bool parseXbe(const std::vector<std::uint8_t>& bytes, XbeHeader& header,
              std::vector<XbeSection>& sections, std::string& error);
}
