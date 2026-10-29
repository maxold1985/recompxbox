#include "recompxbox/nv2a.h"
#include <sstream>
namespace recompxbox {
bool Nv2aPgraph::write(uint32_t sub, uint32_t method, uint32_t value) {
    if (sub >= 8 || method >= 0x2000 || (method & 3)) return false;
    registers_[sub][method / 4] = value;
    trace_.push_back({sub, method, value});
    return true;
}
bool Nv2aPgraph::read(uint32_t sub, uint32_t method, uint32_t& value) const {
    if (sub >= 8 || method >= 0x2000 || (method & 3)) return false;
    value = registers_[sub][method / 4];
    return true;
}
bool Nv2aFifo::submit(const uint32_t* words, size_t count,
                      Nv2aPgraph& gpu, std::string& error) {
    if (!words && count) { error = "Null pushbuffer"; return false; }
    if (count > 1048576) { error = "Pushbuffer exceeds safety limit"; return false; }
    // Decode and validate first, avoiding partial state changes on malformed data.
    std::vector<Nv2aWrite> pending;
    size_t pos = 0;
    while (pos < count) {
        const uint32_t header = words[pos++];
        if (header == 0) continue; // NOP
        // NV2A increasing and non-increasing method packets.
        const bool increasing = (header & 0xe0000003u) == 0;
        const bool nonIncreasing = (header & 0xe0000003u) == 0x40000000u;
        if (!increasing && !nonIncreasing) {
            error = "Unsupported NV2A control-flow/packet opcode";
            return false;
        }
        const uint32_t sub = (header >> 13) & 7;
        const uint32_t method = header & 0x1ffcu;
        const uint32_t n = (header >> 18) & 0x7ffu;
        if (n > count - pos) { error = "Truncated NV2A method packet"; return false; }
        if (n && (method >= 0x2000 || (increasing && (uint64_t(method) + 4ull * (n - 1) >= 0x2000)))) {
            error = "NV2A method outside register window";
            return false;
        }
        for (uint32_t i = 0; i < n; ++i)
            pending.push_back({sub, method + (increasing ? 4u * i : 0u), words[pos++]});
    }
    for (const auto& w : pending)
        gpu.write(w.subchannel, w.method, w.value);
    return true;
}
}
