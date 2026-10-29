# Export an XBE with decoded header fields

Use a personally obtained Xbox classic XISO:

```powershell
recompxbox.exe --iso "C:\\Games\\Fatal Frame.iso" --extract-xbe-decrypted "FatalFrame-decoded.xbe"
```

For an existing XBE:

```powershell
recompxbox.exe "default.xbe" --extract-xbe-decrypted "default-decoded.xbe"
```

The export preserves all section bytes and replaces only the little-endian
header fields at offsets `0x128` (entry point) and `0x158` (kernel thunk)
with XOR-decoded addresses. Retail keys are used by default; `--debug-keys`
selects Xbox debug keys. The decoded addresses are checked against the XBE
image address range before writing.

**Important:** This is *header deobfuscation*, not decryption of the XBE
program or assets. The resulting file is intended for reverse-engineering
tools, **not** for booting on an Xbox or a standard XBE loader: its header
contains decoded rather than expected encoded fields, and signatures are
not repaired. The original ISO and XBE are never modified.
