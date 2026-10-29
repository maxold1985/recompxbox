#include "recompxbox/xbe_image.h"
#include <cassert>
#include <vector>
static void write32(std::vector<std::uint8_t>& b, unsigned at, std::uint32_t v) {
    for (unsigned i=0; i<4; ++i) b[at+i]=static_cast<std::uint8_t>(v>>(i*8));
}
int main() {
    std::vector<std::uint8_t> b(0x300,0);
    b[0]='X';b[1]='B';b[2]='E';b[3]='H';
    write32(b,0x104,0x10000);
    write32(b,0x108,0x200);
    write32(b,0x10c,0x1000);
    write32(b,0x11c,1);
    write32(b,0x120,0x10180);
    write32(b,0x184,0x10200);
    write32(b,0x188,0x100);
    write32(b,0x18c,0x200);
    write32(b,0x190,0x100);
    b[0x200]=0x90; // x86 NOP, copied but never executed.
    recompxbox::XbeImage image;
    std::string error;
    assert(recompxbox::loadXbeImage(b,image,error));
    assert(image.memory.size()==0x1000);
    assert(image.memory[0x200]==0x90);
    assert(image.memory[0x201]==0);
    write32(b,0x188,0x10); // virtual section smaller than file section
    assert(!recompxbox::loadXbeImage(b,image,error));
}
