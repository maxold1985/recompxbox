#include "recompxbox/x86_probe.h"
#include <cstdint>

namespace recompxbox {
X86ProbeResult probeX86(const std::vector<std::uint8_t>& mem,
                        std::uint32_t start, std::size_t maxSteps) {
    X86ProbeResult r;
    r.eip = start;
    auto fetch32 = [&](std::uint32_t at, std::uint32_t& v) {
        if (at > mem.size() || mem.size() - at < 4) return false;
        v = static_cast<std::uint32_t>(mem[at]) |
            (static_cast<std::uint32_t>(mem[at+1]) << 8) |
            (static_cast<std::uint32_t>(mem[at+2]) << 16) |
            (static_cast<std::uint32_t>(mem[at+3]) << 24);
        return true;
    };
    while (r.steps < maxSteps) {
        if (r.eip >= mem.size()) { r.reason = "EIP outside image"; return r; }
        const auto op = mem[r.eip];
        ++r.steps;
        if (op == 0x90) { ++r.eip; continue; } // NOP
        if (op == 0xC3) { ++r.eip; r.halted = true; r.reason = "RET (diagnostic stop)"; return r; }
        if (op == 0xCC) { ++r.eip; r.halted = true; r.reason = "INT3 breakpoint"; return r; }
        if (op == 0xB8 || op == 0x05 || op == 0x2D || op == 0xE9) {
            std::uint32_t immediate = 0;
            if (!fetch32(r.eip + 1, immediate)) { r.reason = "Truncated immediate"; return r; }
            const std::uint32_t next = r.eip + 5;
            if (op == 0xB8) r.eax = immediate;        // MOV EAX, imm32
            if (op == 0x05) r.eax += immediate;       // ADD EAX, imm32
            if (op == 0x2D) r.eax -= immediate;       // SUB EAX, imm32
            r.eip = op == 0xE9 ? next + immediate : next; // JMP rel32
            continue;
        }
        if (op == 0xEB) { // JMP rel8
            if (static_cast<std::size_t>(r.eip) + 1 >= mem.size()) {
                r.reason = "Truncated rel8"; return r;
            }
            const auto delta = static_cast<std::int8_t>(mem[r.eip+1]);
            r.eip = static_cast<std::uint32_t>(
                static_cast<std::int64_t>(r.eip) + 2 + delta);
            continue;
        }
        r.reason = "Unsupported opcode 0x";
        constexpr char hex[] = "0123456789ABCDEF";
        r.reason += hex[op >> 4];
        r.reason += hex[op & 15];
        return r;
    }
    r.reason = "Instruction budget exhausted";
    return r;
}
}
