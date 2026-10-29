#include "recompxbox/nv2a_mmio.h"
#include <cassert>
int main() {
    recompxbox::Nv2aMmio gpu;
    using B = recompxbox::Nv2aBlock;
    assert(gpu.blockAt(0x2000) == B::PFIFO);
    assert(gpu.blockAt(0x400000) == B::PGRAPH);
    assert(gpu.blockAt(0x600000) == B::PCRTC);
    assert(gpu.blockAt(0x800000) == B::USER);
    assert(gpu.blockAt(0x500000) == B::Unknown);
    assert(gpu.write32(0x400100, 0x12345678));
    uint32_t v = 0;
    assert(gpu.read32(0x400100, v) && v == 0x12345678);
    assert(gpu.write32(0x400100, 99));
    assert(gpu.read32(0x400100, v) && v == 99);
    assert(gpu.writes().size() == 1);
    assert(!gpu.write32(0x400101, 5));
    assert(!gpu.write32(0x500000, 5));
}
