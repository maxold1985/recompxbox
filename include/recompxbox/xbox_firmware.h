#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace recompxbox {
enum class XboxFirmwareKind { McpxBootRom, FlashBios };
struct XboxFirmware {
    XboxFirmwareKind kind{};
    std::string path;
    std::vector<std::uint8_t> bytes;
    // BIOS is mirrored over the Xbox 16 MiB flash window at 0xFF000000.
    bool readFlashByte(std::uint32_t physicalAddress, std::uint8_t& out) const;
};
bool loadXboxFirmware(const std::string& path, XboxFirmwareKind kind,
                      XboxFirmware& out, std::string& error);
}
