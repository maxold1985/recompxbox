#pragma once
#include <array>
#include <cstdint>
#include <vector>
namespace recompxbox {
// Xbox NV2A BAR0 register layout, adapted from xemu/xqemu NV2A block map.
// This is a diagnostic MMIO register bank, NOT a complete GPU implementation.
enum class Nv2aBlock { PMC, PBUS, PFIFO, PVIDEO, PTIMER, PFB, PGRAPH, PCRTC, PRAMDAC, USER, Unknown };
struct Nv2aMmioEvent { uint32_t offset; uint32_t value; Nv2aBlock block; };
class Nv2aMmio {
public:
    Nv2aBlock blockAt(uint32_t offset) const;
    bool write32(uint32_t offset, uint32_t value);
    bool read32(uint32_t offset, uint32_t& value) const;
    const std::vector<Nv2aMmioEvent>& writes() const { return writes_; }
private:
    // Sparse register bank; only addresses explicitly written have stored values.
    std::vector<Nv2aMmioEvent> writes_;
};
}
