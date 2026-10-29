#include "recompxbox/xbe_image.h"
#include "recompxbox/xdvdfs.h"
#include "recompxbox/qemu_rsp.h"
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
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
    bool isoMode = false, decodeExport = false, debugKeys = false, qemuWrite = false, cpuTrace = false;
    std::string endpoint = "127.0.0.1:1234";
    std::string inputPath, outputPath;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--qemu-tcp" && i + 1 < argc) {
            qemuWrite = true;
            endpoint = argv[++i];
        } else if (arg == "--cpu-trace") {
            cpuTrace = true;
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
                     " [--extract-xbe-decrypted <output.xbe>] [--debug-keys] [--qemu-tcp host:port] [--cpu-trace]\n";
        return 2;
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
        if (!recompxbox::sendXbeToQemu(image, endpoint, error, cpuTrace)) {
            std::cerr << "QEMU TCP error: " << error << "\n";
            return 1;
        }
        std::cout << "XBE transferred to paused QEMU (CPU not started)\n";
    }
    std::cout << "x86 execution: NOT IMPLEMENTED (CPU trace is inspection only)\n";
    return 0;
}
