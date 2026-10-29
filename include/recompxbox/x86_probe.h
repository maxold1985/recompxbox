#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace recompxbox {
// Diagnostic x86-32 interpreter for a tiny instruction subset.
// No host code execution, kernel calls or real game support.
struct X86ProbeResult {
    std::uint32_t eax = 0;
    std::uint32_t eip = 0;
    std::size_t steps = 0;
    bool halted = false;
    std::string reason;
};
// startOffset is relative to the beginning of the loaded XBE image.
// maxSteps prevents infinite loops in untrusted guest code.
X86ProbeResult probeX86(const std::vector<std::uint8_t>& memory,
                        std::uint32_t startOffset, std::size_t maxSteps = 1000);
}
