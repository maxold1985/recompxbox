#include "recompxbox/xbox_firmware.h"
#include <fstream>
#include <iterator>
#include <utility>
namespace recompxbox {
bool loadXboxFirmware(const std::string& path, XboxFirmwareKind kind,
                      XboxFirmware& out, std::string& error) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) { error = "Cannot open firmware: " + path; return false; }
    const auto length = file.tellg();
    const bool valid = kind == XboxFirmwareKind::McpxBootRom
        ? length == std::streamoff(512)
        : length == std::streamoff(256 * 1024) ||
          length == std::streamoff(512 * 1024) ||
          length == std::streamoff(1024 * 1024);
    if (!valid) {
        error = "Invalid firmware size for " + path +
                (kind == XboxFirmwareKind::McpxBootRom
                    ? " (expected 512 bytes)" : " (expected 256 KiB, 512 KiB or 1 MiB)");
        return false;
    }
    file.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> bytes(static_cast<size_t>(length));
    if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) {
        error = "Firmware read failed: " + path;
        return false;
    }
    XboxFirmware result;
    result.kind = kind;
    result.path = path;
    result.bytes = std::move(bytes);
    out = std::move(result);
    return true;
}
bool readXboxBootByte(const XboxFirmware* flash, const XboxFirmware* mcpx,
                      std::uint32_t address, std::uint8_t& value) {
    if (address < 0xff000000u) return false;
    // MCPX 1.0 boot ROM is mapped over the last 512 bytes of flash.
    if (address >= 0xfffffe00u && mcpx &&
        mcpx->kind == XboxFirmwareKind::McpxBootRom && mcpx->bytes.size() == 512) {
        value = mcpx->bytes[address - 0xfffffe00u];
        return true;
    }
    return flash && flash->readFlashByte(address, value);
}
bool XboxFirmware::readFlashByte(std::uint32_t address, std::uint8_t& out) const {
    if (kind != XboxFirmwareKind::FlashBios || bytes.empty() || address < 0xff000000u)
        return false;
    out = bytes[(address - 0xff000000u) % bytes.size()];
    return true;
}
}
