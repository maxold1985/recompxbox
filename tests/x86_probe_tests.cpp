#include "recompxbox/x86_probe.h"
#include <cassert>
#include <vector>
int main() {
    using recompxbox::probeX86;
    // MOV EAX, 5; ADD EAX, 3; SUB EAX, 1; RET
    std::vector<std::uint8_t> code = {
        0xB8,5,0,0,0, 0x05,3,0,0,0, 0x2D,1,0,0,0, 0xC3
    };
    auto result = probeX86(code, 0);
    assert(result.halted && result.eax == 7 && result.steps == 4);
    assert(!probeX86({0x0F},0).halted);
    assert(probeX86({0xEB,0xFE},0,10).steps == 10);
    assert(probeX86({0x90},10).reason == "EIP outside image");
    assert(probeX86({0xB8,0x01},0).reason == "Truncated immediate");
}
