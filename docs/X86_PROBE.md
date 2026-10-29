# Diagnostic x86-32 probe

`probeX86` interprets only NOP, MOV EAX/imm32, ADD EAX/imm32,
SUB EAX/imm32, JMP rel8/rel32, INT3 and RET. It uses a bounded
instruction budget and an isolated byte-array guest image.

This is useful for synthetic instruction-level tests, **not a CPU
emulator capable of executing Xbox games**. No stack, other registers,
memory operands, privileged instructions, kernel calls or hardware
access are implemented. It does not call native x86 code.

To run tests: `ctest --test-dir build --output-on-failure`.
