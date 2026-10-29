# XBE memory transfer to QEMU TCG (experimental)

This bridge connects the `recompxbox` XBE section loader to the **real
QEMU guest memory** using QEMU's built-in GDB Remote Serial Protocol (RSP)
server. It is an external process bridge, **not an in-process TCG library**.

## Build and start QEMU

Build the optional QEMU TCG target (see `QEMU_TCG.md`). On Linux/WSL2,
start the built QEMU i386 emulator with a paused CPU and local-only debugger:

```bash
./build/qemu-tcg/qemu-system-i386 -machine pc -m 256M -accel tcg \
  -display none -monitor none -serial none -S \
  -gdb tcp:127.0.0.1:1234
```

In another terminal, first validate the XBE:

```bash
python3 scripts/qemu_xbe_bridge.py /path/to/default.xbe
```

Then explicitly transfer XBE headers and sections to guest memory:

```bash
python3 scripts/qemu_xbe_bridge.py /path/to/default.xbe --write
```

The bridge uses GDB `M` packets to write guest addresses, including
zero-filled virtual section tails. QEMU's memory translation and machine
configuration still govern which addresses are writable. An ordinary PC
machine has reserved memory regions, so writes can fail or overwrite BIOS
or device state. Use only a disposable QEMU instance; never a production VM.

**Important:** the x86 CPU is still in its firmware startup context, not
an Xbox-compatible 32-bit protected-mode environment. This bridge does not
configure CPU registers, segment descriptors, page tables, Xbox kernel,
interrupts or GPU. It does not start executing XBE code and cannot run
games yet. The XBE entrypoint is encoded and not used.

For test coverage:
```bash
python3 -m unittest discover -s tests -p 'test_qemu_bridge.py'
```
