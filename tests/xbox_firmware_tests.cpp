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
    std::remove(path.c_str());
}
