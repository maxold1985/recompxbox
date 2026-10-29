# Diagnosing QEMU memory write errors

The native GDB RSP client reports the exact QEMU reply to each failed
memory write, e.g. `E14` (target memory access failure) versus a transport
or checksum failure. Previously both were reported as the same generic error.

For Fatal Frame, the observed failing guest address is
`0x00133B60`, within section 5. This address is below 256 MiB and
therefore not evidence of insufficient QEMU RAM on its own.

Restart a **fresh, disposable** QEMU process before retrying:

```powershell
qemu-system-i386 -machine pc -m 256M -accel tcg -display none -monitor none -serial none -S -gdb tcp:127.0.0.1:1234
recompxbox.exe --iso "..\\Fatal Frame.iso" --qemu-tcp 127.0.0.1:1234
```

Use separate terminals for these commands. The target is still a generic
PC machine, not an Xbox-compatible machine; the bridge only transfers
memory and does not start execution.
