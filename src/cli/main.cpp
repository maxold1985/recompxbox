#include "recompxbox/xbe_image.h"
#include "recompxbox/xdvdfs.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 2 && !(argc == 3 && std::string(argv[1]) == "--iso")) {
        std::cerr << "Usage: recompxbox <default.xbe> | --iso <game.iso>\n";
        return 2;
    }
    std::vector<std::uint8_t> bytes;
    if (argc == 3) {
        recompxbox::Xdvdfs iso;
        std::string error;
        if (!iso.open(argv[2], error) || !iso.readFile("D:\\default.xbe", bytes, error)) {
            std::cerr << "ISO error: " << error << "\n";
            return 1;
        }
        std::cout << "Loaded D:\\default.xbe from ISO (" << bytes.size() << " bytes)\n";
    } else {
        std::ifstream input(argv[1], std::ios::binary);
        if (!input) {
            std::cerr << "Cannot open XBE file: " << argv[1] << "\n";
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
    std::cout << "x86 execution: NOT IMPLEMENTED\n";
    return 0;
}
