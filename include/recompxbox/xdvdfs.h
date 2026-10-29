#pragma once
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace recompxbox {
struct XdvdfsEntry {
    std::string path;
    std::uint32_t sector = 0;
    std::uint32_t size = 0;
    bool directory = false;
};
class Xdvdfs {
public:
    bool open(const std::string& isoPath, std::string& error);
    bool list(std::vector<XdvdfsEntry>& entries, std::string& error);
    bool readFile(const std::string& xboxPath, std::vector<std::uint8_t>& bytes,
                  std::string& error);
private:
    std::ifstream iso_;
    std::uint64_t fileSize_ = 0, base_ = 0;
    std::uint32_t stride_ = 2048, prefix_ = 0;
    std::uint32_t rootSector_ = 0, rootSize_ = 0;
    std::map<std::string, XdvdfsEntry> files_;
    bool readAt(std::uint64_t offset, std::uint8_t* out, std::size_t size);
    bool walk(std::uint32_t sector, std::uint32_t size, const std::string& prefix,
              unsigned depth, std::string& error);
};
}
