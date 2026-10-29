#include "recompxbox/xbe_image.h"
#include "recompxbox/xdvdfs.h"
#include "recompxbox/qemu_rsp.h"
#include "recompxbox/xbox_firmware.h"
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <utility>
#include <string>
#include <vector>

namespace {
void write32(std::vector<std::uint8_t>& data, std::size_t offset, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i)
        data[offset + i] = static_cast<std::uint8_t>(value >> (8 * i));
}
bool save(const std::string& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
    return out.good();
}
}

int main(int argc, char** argv) {
    // --extract-xbe-decrypted exports a research-only XBE with decoded
    // entry point and kernel thunk fields. It is NOT a bootable retail XBE.
    bool isoMode = false, decodeExport = false, debugKeys = false, qemuWrite = false, cpuTrace = false, cpuStep = false;
    std::string endpoint = "127.0.0.1:1234";
    std::string inputPath, outputPath, biosPath, mcpxPath;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--qemu-tcp" && i + 1 < argc) {
            qemuWrite = true;
            endpoint = argv[++i];
        } else if (arg == "--cpu-step") {
            cpuStep = true;
            cpuTrace = true;
        } else if (arg == "--cpu-trace") {
            cpuTrace = true;
        } else if (arg == "--bios" && i + 1 < argc) {
            biosPath = argv[++i];
        } else if (arg == "--mcpx" && i + 1 < argc) {
            mcpxPath = argv[++i];
        } else if (arg == "--iso") {
            isoMode = true;
        } else if (arg == "--debug-keys") {
            debugKeys = true;
        } else if (arg == "--extract-xbe-decrypted" && i + 1 < argc) {
            decodeExport = true;
            outputPath = argv[++i];
        } else if (!arg.empty() && arg[0] != '-' && inputPath.empty()) {
            inputPath = arg;
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            return 2;
        }
    }
    if (inputPath.empty() || (debugKeys && !decodeExport) || (cpuTrace && !qemuWrite)) {
        std::cerr << "Usage: recompxbox [--iso] <game.iso|default.xbe>"
                     " [--extract-xbe-decrypted <output.xbe>] [--debug-keys] [--qemu-tcp host:port] [--cpu-trace] [--cpu-step] [--bios xbox.bin] [--mcpx mcpx.bin]\n";
        return 2;
    }

    // Optional firmware is validated and inspected locally. It is NOT yet
    // attached to the generic PC QEMU machine or executed as Xbox firmware.
    recompxbox::XboxFirmware flashFirmware, mcpxFirmware;
    bool haveFlash = false, haveMcpx = false;
    for (const auto& option : {std::pair<std::string, recompxbox::XboxFirmwareKind>{
                                   biosPath, recompxbox::XboxFirmwareKind::FlashBios},
                               {mcpxPath, recompxbox::XboxFirmwareKind::McpxBootRom}}) {
        if (option.first.empty()) continue;
        recompxbox::XboxFirmware firmware;
        std::string firmwareError;
        if (!recompxbox::loadXboxFirmware(option.first, option.second, firmware, firmwareError)) {
            std::cerr << "Firmware error: " << firmwareError << "\n";
            return 1;
        }
        if (option.second == recompxbox::XboxFirmwareKind::FlashBios) {
            flashFirmware = firmware;
            haveFlash = true;
        } else {
            mcpxFirmware = firmware;
            haveMcpx = true;
        }
        std::cout << (option.second == recompxbox::XboxFirmwareKind::FlashBios
                         ? "Xbox Flash BIOS" : "MCPX Boot ROM")
                  << ": loaded " << firmware.bytes.size() << " bytes from "
                  << firmware.path << " (inspection only; not mapped into QEMU)\n";
    }
    if (haveFlash || haveMcpx) {
        // The reset vector is at physical 0xFFFFFFF0 (CS base + EIP).
        // This models Xbox ROM visibility, not QEMU machine memory mapping.
        std::cout << "Xbox firmware reset vector at 0xFFFFFFF0:";
        for (std::uint32_t i = 0; i < 16; ++i) {
            std::uint8_t value = 0;
            if (!recompxbox::readXboxBootByte(
                    haveFlash ? &flashFirmware : nullptr,
                    haveMcpx ? &mcpxFirmware : nullptr,
                    0xfffffff0u + i, value)) {
                std::cout << " unavailable";
                break;
            }
            std::cout << ' ' << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<unsigned>(value) << std::dec;
        }
        std::cout << "\nXbox ROM address map: host-side inspection only; "
                     "QEMU -machine pc still uses PC firmware.\n";
    }
    std::vector<std::uint8_t> bytes;
    if (isoMode) {
        recompxbox::Xdvdfs iso;
        std::string error;
        if (!iso.open(inputPath, error) ||
            !iso.readFile("D:\\default.xbe", bytes, error)) {
            std::cerr << "ISO error: " << error << "\n";
            return 1;
        }
        std::cout << "Loaded D:\\default.xbe from ISO (" << bytes.size() << " bytes)\n";
    } else {
        std::ifstream input(inputPath, std::ios::binary);
        if (!input) {
            std::cerr << "Cannot open XBE file: " << inputPath << "\n";
            return 1;
        }
        bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    }

    recompxbox::XbeImage image;
    std::string error;
    if (!recompxbox::loadXbeImage(bytes, image, error)) {
        std::cerr << "Invalid XBE: " << error << "\n";
        return 1;
    }
    std::cout << "XBE loaded into host-side guest image (not executed)\n";
    std::cout << "Image base: 0x" << std::hex << image.header.baseAddress
              << " size: 0x" << image.header.imageSize << std::dec << "\n";
    std::cout << "Sections: " << image.sections.size() << "\n";
    for (std::size_t i = 0; i < image.sections.size(); ++i) {
        const auto& s = image.sections[i];
        std::cout << "  [" << i << "] VA=0x" << std::hex << s.virtualAddress
                  << " virtual=0x" << s.virtualSize
                  << " raw=0x" << s.rawOffset << "+" << s.rawSize
                  << std::dec << "\n";
    }
    std::cout << "Encoded entry point: 0x" << std::hex
              << image.header.entryPointEncoded << std::dec << "\n";

    if (decodeExport) {
        // Xbox XBE retail/debug XOR obfuscation, not section encryption.
        constexpr std::uint32_t retailEntryKey = 0xA8FC57AB;
        constexpr std::uint32_t debugEntryKey = 0x94859D4B;
        constexpr std::uint32_t retailThunkKey = 0x5B6D40B6;
        constexpr std::uint32_t debugThunkKey = 0xEFB1F152;
        const auto entry = image.header.entryPointEncoded ^
                           (debugKeys ? debugEntryKey : retailEntryKey);
        const auto thunk = image.header.kernelThunkEncoded ^
                           (debugKeys ? debugThunkKey : retailThunkKey);
        const auto base = std::uint64_t(image.header.baseAddress);
        const auto end = base + image.header.imageSize;
        if (entry < base || entry >= end || thunk < base || thunk >= end) {
            std::cerr << "Decoded addresses are outside the XBE image; "
                         "check retail/debug key selection. No output written.\n";
            return 1;
        }
        write32(bytes, 0x128, entry);
        write32(bytes, 0x158, thunk);
        if (!save(outputPath, bytes)) {
            std::cerr << "Cannot write decoded XBE: " << outputPath << "\n";
            return 1;
        }
        std::cout << "Decoded entry point: 0x" << entry
                  << " kernel thunk: 0x" << thunk << std::dec << "\n";
        std::cout << "Wrote research-only decoded-header XBE: " << outputPath << "\n";
        std::cout << "WARNING: Not a bootable XBE; signatures/header fields "
                     "are not valid for normal Xbox loading.\n";
    }
    if (qemuWrite) {
        if (!recompxbox::sendXbeToQemu(image, endpoint, error, cpuTrace, cpuStep)) {
            std::cerr << "QEMU TCP error: " << error << "\n";
            return 1;
        }
        std::cout << "XBE transferred to QEMU (CPU execution only if --cpu-step was specified)\n";
    }
    std::cout << (cpuStep ? "x86 execution: one QEMU PC instruction stepped (NOT Xbox XBE boot)\n" : "x86 execution: NOT IMPLEMENTED (CPU trace is inspection only)\n");
    return 0;
}
