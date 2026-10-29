#include "recompxbox/xbe_header.h"
#include <cassert>
#include <vector>

static void put32(std::vector<std::uint8_t>& b, unsigned off, std::uint32_t n) {
    for (unsigned i = 0; i < 4; ++i) b[off+i] = static_cast<std::uint8_t>(n >> (i*8));
}
int main() {
    using namespace recompxbox;
    std::vector<std::uint8_t> b(0x300, 0);
    XbeHeader h;
    std::vector<XbeSection> sections;
    std::string error;
    assert(!parseXbe(b,h,sections,error));
    b[0]='X'; b[1]='B'; b[2]='E'; b[3]='H';
    put32(b,0x104,0x10000); // Image base
    put32(b,0x108,0x200);   // Header size
    put32(b,0x10c,0x1000);  // Image size
    put32(b,0x11c,1);       // One section
    put32(b,0x120,0x10180);// Section table RVA translated to VA
    put32(b,0x184,0x10200);// Section VA
    put32(b,0x188,0x100);  // Virtual size
    put32(b,0x18c,0x200);  // Raw offset
    put32(b,0x190,0x100);  // Raw size
    assert(parseXbe(b,h,sections,error));
    assert(sections.size()==1 && sections[0].rawOffset==0x200);
    put32(b,0x190,0x101);  // Beyond file
    assert(!parseXbe(b,h,sections,error));
}
