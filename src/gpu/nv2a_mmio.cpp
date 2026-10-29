#include "recompxbox/nv2a_mmio.h"
namespace recompxbox {
namespace {
struct Range { uint32_t base, size; Nv2aBlock block; };
constexpr Range blocks[] = {
    {0x000000,0x001000,Nv2aBlock::PMC},
    {0x001000,0x001000,Nv2aBlock::PBUS},
    {0x002000,0x002000,Nv2aBlock::PFIFO},
    {0x008000,0x001000,Nv2aBlock::PVIDEO},
    {0x009000,0x001000,Nv2aBlock::PTIMER},
    {0x100000,0x001000,Nv2aBlock::PFB},
    {0x400000,0x002000,Nv2aBlock::PGRAPH},
    {0x600000,0x001000,Nv2aBlock::PCRTC},
    {0x680000,0x001000,Nv2aBlock::PRAMDAC},
    {0x800000,0x800000,Nv2aBlock::USER}
};
}
Nv2aBlock Nv2aMmio::blockAt(uint32_t offset) const {
    for (const auto& b : blocks)
        if (offset >= b.base && offset - b.base < b.size) return b.block;
    return Nv2aBlock::Unknown;
}
bool Nv2aMmio::write32(uint32_t offset, uint32_t value) {
    if ((offset & 3) || blockAt(offset) == Nv2aBlock::Unknown) return false;
    // Retain the most recent value; no hardware side effects are emulated.
    for (auto& event : writes_)
        if (event.offset == offset) { event.value = value; return true; }
    writes_.push_back({offset, value, blockAt(offset)});
    return true;
}
bool Nv2aMmio::read32(uint32_t offset, uint32_t& value) const {
    if ((offset & 3) || blockAt(offset) == Nv2aBlock::Unknown) return false;
    for (const auto& event : writes_)
        if (event.offset == offset) { value = event.value; return true; }
    value = 0; // Diagnostic default, not NV2A reset behavior.
    return true;
}
}
