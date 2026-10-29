#include "recompxbox/xbox_firmware.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
int main() {
    const std::string path = "recompxbox_firmware_test.bin";
    { std::ofstream out(path, std::ios::binary); for (int i=0;i<512;++i) out.put(char(i & 255)); }
    recompxbox::XboxFirmware fw;
    std::string error;
    assert(recompxbox::loadXboxFirmware(path, recompxbox::XboxFirmwareKind::McpxBootRom, fw, error));
    assert(fw.bytes.size() == 512);
    assert(!recompxbox::loadXboxFirmware(path, recompxbox::XboxFirmwareKind::FlashBios, fw, error));
    recompxbox::XboxFirmware flash;
    flash.kind = recompxbox::XboxFirmwareKind::FlashBios;
    flash.bytes.resize(256 * 1024);
    flash.bytes[flash.bytes.size() - 16] = 0xEA;
    std::uint8_t value = 0;
    assert(recompxbox::readXboxBootByte(&flash, nullptr, 0xfffffff0u, value) && value == 0xEA);
    recompxbox::XboxFirmware mcpx;
    mcpx.kind = recompxbox::XboxFirmwareKind::McpxBootRom;
    mcpx.bytes.resize(512);
    mcpx.bytes[496] = 0x33;
    assert(recompxbox::readXboxBootByte(&flash, &mcpx, 0xfffffff0u, value) && value == 0x33);
    assert(recompxbox::readXboxBootByte(&flash, &mcpx, 0xfffc0000u, value) && value == 0);
    assert(!recompxbox::readXboxBootByte(&flash, &mcpx, 0x000ffff0u, value));
    std::remove(path.c_str());
}
