# Opening XBE files from the terminal

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\recompxbox.exe "C:\Games\Xbox\default.xbe"
```

On single-config generators the binary may instead be `build/recompxbox`
(or `build/recompxbox.exe`).

The command reads the XBE, validates its header and section table,
allocates a bounded host-side byte array, copies headers and sections
to their guest virtual-address-relative offsets, and prints metadata.

This **does not** reserve guest virtual memory at Xbox addresses, map
hardware, decode entrypoint keys, execute x86, implement kernel calls
or run Xbox games. Do not interpret a successful load as game compatibility.
