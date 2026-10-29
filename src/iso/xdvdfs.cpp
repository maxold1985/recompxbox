#include "recompxbox/xdvdfs.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <functional>
#include <limits>
#include <set>

namespace recompxbox {
namespace {
constexpr std::uint64_t sectorSize = 2048;
constexpr char magic[] = "MICROSOFT*XBOX*MEDIA";
std::uint32_t u32(const std::uint8_t* p) {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
           (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
}
std::uint16_t u16(const std::uint8_t* p) {
    return std::uint16_t(p[0]) | (std::uint16_t(p[1]) << 8);
}
std::string normalized(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    if (s.size() >= 3 && std::tolower(static_cast<unsigned char>(s[0])) == 'd' &&
        s[1] == ':' && s[2] == '/') s.erase(0, 3);
    while (!s.empty() && s.front() == '/') s.erase(s.begin());
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}
}
bool Xdvdfs::readAt(std::uint64_t offset, std::uint8_t* out, std::size_t size) {
    while (size) {
        const auto sector = offset / sectorSize;
        const auto within = offset % sectorSize;
        const auto amount = std::min<std::size_t>(size, sectorSize - within);
        const auto physical = base_ + sector * stride_ + prefix_ + within;
        if (physical > fileSize_ || amount > fileSize_ - physical) return false;
        iso_.clear();
        iso_.seekg(static_cast<std::streamoff>(physical));
        if (!iso_.read(reinterpret_cast<char*>(out), static_cast<std::streamsize>(amount)))
            return false;
        out += amount;
        offset += amount;
        size -= amount;
    }
    return true;
}
bool Xdvdfs::open(const std::string& path, std::string& error) {
    iso_.close();
    files_.clear();
    iso_.open(path, std::ios::binary | std::ios::ate);
    if (!iso_) { error = "Cannot open ISO"; return false; }
    fileSize_ = static_cast<std::uint64_t>(iso_.tellg());
    bool found = false;
    for (auto base : {std::uint64_t(0), std::uint64_t(387 * 2048)}) {
        for (const auto layout : {std::pair<unsigned,unsigned>{2048,0}, {2352,16}, {2352,24}}) {
            const auto offset = base + 32ull * layout.first + layout.second;
            if (offset + sizeof(magic) - 1 > fileSize_) continue;
            std::array<char, sizeof(magic) - 1> bytes{};
            iso_.clear();
            iso_.seekg(static_cast<std::streamoff>(offset));
            if (iso_.read(bytes.data(), bytes.size()) &&
                std::equal(bytes.begin(), bytes.end(), magic)) {
                base_ = base;
                stride_ = layout.first;
                prefix_ = layout.second;
                found = true;
                break;
            }
        }
        if (found) break;
    }
    if (!found) { error = "XDVDFS descriptor not found"; return false; }
    std::array<std::uint8_t, 28> descriptor{};
    if (!readAt(32 * sectorSize, descriptor.data(), descriptor.size())) {
        error = "Cannot read volume descriptor"; return false;
    }
    rootSector_ = u32(descriptor.data() + 20);
    rootSize_ = u32(descriptor.data() + 24);
    if (!rootSize_ || rootSize_ > 64 * 1024 * 1024) {
        error = "Invalid root directory size"; return false;
    }
    return true;
}
bool Xdvdfs::walk(std::uint32_t sector, std::uint32_t size,
                  const std::string& prefix, unsigned depth, std::string& error) {
    if (depth > 32 || files_.size() > 200000 || size > 64 * 1024 * 1024) {
        error = "Directory limit exceeded"; return false;
    }
    std::vector<std::uint8_t> dir(size);
    if (!readAt(std::uint64_t(sector) * sectorSize, dir.data(), dir.size())) {
        error = "Invalid directory extent"; return false;
    }
    std::set<std::size_t> visited;
    std::function<bool(std::size_t)> visit = [&](std::size_t offset) -> bool {
        if (!visited.insert(offset).second || offset > dir.size() ||
            dir.size() - offset < 14) {
            error = "Invalid directory tree"; return false;
        }
        const auto* p = dir.data() + offset;
        const auto left = u16(p), right = u16(p + 2);
        const auto start = u32(p + 4), length = u32(p + 8);
        const auto attr = p[12], nameLen = p[13];
        if (!nameLen || nameLen > dir.size() - offset - 14) {
            error = "Invalid directory filename"; return false;
        }
        std::string name(reinterpret_cast<const char*>(p + 14), nameLen);
        if (name == "." || name == ".." || name.find_first_of("/\\") != std::string::npos) {
            error = "Unsafe directory filename"; return false;
        }
        if (left && !visit(std::size_t(left) * 4)) return false;
        const auto full = prefix + name;
        const auto key = normalized(full);
        if (files_.count(key)) { error = "Duplicate directory entry"; return false; }
        files_[key] = {full, start, length, (attr & 0x10) != 0};
        if (files_.size() > 200000) { error = "Too many entries"; return false; }
        if (attr & 0x10) {
            if (length && !walk(start, length, full + "/", depth + 1, error)) return false;
        } else if (length) {
            std::uint8_t check = 0;
            if (!readAt(std::uint64_t(start) * sectorSize + length - 1, &check, 1)) {
                error = "File extent outside image"; return false;
            }
        }
        if (right && !visit(std::size_t(right) * 4)) return false;
        return true;
    };
    return size == 0 || visit(0);
}
bool Xdvdfs::list(std::vector<XdvdfsEntry>& entries, std::string& error) {
    files_.clear();
    if (!iso_.is_open()) { error = "ISO is not open"; return false; }
    if (!walk(rootSector_, rootSize_, "", 0, error)) return false;
    entries.clear();
    for (const auto& item : files_) entries.push_back(item.second);
    return true;
}
bool Xdvdfs::readFile(const std::string& xboxPath,
                      std::vector<std::uint8_t>& bytes, std::string& error) {
    if (files_.empty()) {
        std::vector<XdvdfsEntry> unused;
        if (!list(unused, error)) return false;
    }
    const auto it = files_.find(normalized(xboxPath));
    if (it == files_.end() || it->second.directory) {
        error = "File not found in ISO"; return false;
    }
    if (it->second.size > 256u * 1024u * 1024u) {
        error = "File exceeds 256 MiB extraction limit"; return false;
    }
    bytes.resize(it->second.size);
    if (!readAt(std::uint64_t(it->second.sector) * sectorSize, bytes.data(), bytes.size())) {
        bytes.clear();
        error = "Failed reading ISO file"; return false;
    }
    return true;
}
}
