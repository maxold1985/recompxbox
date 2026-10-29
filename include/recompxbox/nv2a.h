#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace recompxbox {
// First NV2A front-end layer: register writes and method stream decoding.
// Rendering and GPU device attachment are intentionally separate.
struct Nv2aWrite {
    uint32_t subchannel;
    uint32_t method;
    uint32_t value;
};
class Nv2aPgraph {
public:
    static constexpr uint32_t RegisterCount = 0x2000 / 4;
    bool write(uint32_t subchannel, uint32_t method, uint32_t value);
    bool read(uint32_t subchannel, uint32_t method, uint32_t& value) const;
    const std::vector<Nv2aWrite>& trace() const { return trace_; }
    void clearTrace() { trace_.clear(); }
private:
    std::array<std::array<uint32_t, RegisterCount>, 8> registers_{};
    std::vector<Nv2aWrite> trace_;
};
class Nv2aFifo {
public:
    // Parse a bounded, linear NV pushbuffer. Jump/call opcodes are rejected
    // until guest memory and the actual DMA context are connected.
    bool submit(const uint32_t* words, size_t count, Nv2aPgraph& gpu,
                std::string& error);
};
}
