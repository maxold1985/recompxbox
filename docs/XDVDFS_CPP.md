# Native XDVDFS ISO reader

The C++ `recompxbox` executable now accepts Xbox classic XDVDFS images:

```powershell
recompxbox.exe --iso "C:\\Games\\Xbox\\game.iso"
```

It detects supported 2048/2352-byte sector layouts, walks the
XDVDFS directory tree, finds `D:\\default.xbe`, reads its bytes,
and passes them to the existing XBE parser/guest image loader.
The reader is read-only and has bounds checks on directory extents
and extraction size. A future kernel file dispatcher can call
`Xdvdfs::readFile("D:\\\\media\\\\texture.xpr", ...)` for resources.

This is **not yet an Xbox NtCreateFile/NtReadFile implementation**,
nor a CPU execution path. It does not run games.
