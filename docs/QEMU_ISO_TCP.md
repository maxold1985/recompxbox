# QEMU TCG TCP connection from an Xbox ISO

The existing GDB RSP bridge can now load `default.xbe` directly from an
XDVDFS ISO, without first exporting the XBE to disk.

Start QEMU (a disposable instance, paused):

```powershell
qemu-system-i386 -machine pc -m 256M -accel tcg -display none -monitor none -serial none -S -gdb tcp:127.0.0.1:1234
```

In another terminal:

```powershell
python scripts/qemu_xbe_bridge.py "..\\Fatal Frame.iso" --iso --write --host 127.0.0.1 --port 1234
```

Without `--write`, the command validates the image and reports the XBE
layout but does not connect or write memory. The TCP client uses GDB RSP
`M` packets to copy XBE headers and sections into guest virtual memory.

This is a **memory-transfer bridge only**. QEMU's PC machine is not an Xbox
machine, and the bridge does not set protected mode, initialize Xbox kernel
services, or start game execution. The `--extract-xbe-decrypted` export is
not needed for this transfer: use the original ISO.
