#include "recompxbox/nv2a.h"
#include <cassert>
#include <string>
int main() {
    recompxbox::Nv2aPgraph gpu;
    recompxbox::Nv2aFifo fifo;
    std::string error;
    const uint32_t increasing[] = {(2u << 18) | (3u << 13) | 0x100u, 0x1234u, 0x5678u};
    assert(fifo.submit(increasing, 3, gpu, error));
    uint32_t value = 0;
    assert(gpu.read(3, 0x100, value) && value == 0x1234);
    assert(gpu.read(3, 0x104, value) && value == 0x5678);
    const uint32_t noninc[] = {0x40000000u | (2u << 18) | 0x180u, 11, 22};
    assert(fifo.submit(noninc, 3, gpu, error));
    assert(gpu.read(0, 0x180, value) && value == 22);
    const auto prior = gpu.trace().size();
    const uint32_t truncated[] = {(2u << 18) | 0x100u, 33};
    assert(!fifo.submit(truncated, 2, gpu, error));
    assert(gpu.trace().size() == prior);
    const uint32_t unsupported[] = {0x20000001u};
    assert(!fifo.submit(unsupported, 1, gpu, error));
    assert(gpu.trace().size() == prior);
}
