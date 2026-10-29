# XDVDFS reader / XBE extractor

Read-only Python implementation for Xbox classic XDVDFS images.

```powershell
python scripts/xdvdfs_reader.py "C:\\Games\\game.iso" --list
python scripts/xdvdfs_reader.py "C:\\Games\\game.iso" --output default.xbe
python scripts/xdvdfs_reader.py "C:\\Games\\game.iso" --extract "data/config.ini" --output config.ini
```

Supports standard 2048-byte XISO sectors and raw 2352-byte sector
layouts with 16/24-byte sector headers, and a common 387-sector
offset layout. Detects the XDVDFS descriptor, reads directory trees,
validates extents, and extracts files in bounded chunks.

Does not support all redump/drive-specific formats, compressed archives,
multi-part files, or every image layout. Does not execute games.
The XBE alone is generally insufficient to run a game; game data files
must also remain accessible to the eventual runtime.

Tests: `python3 -m unittest discover -s tests -p test_xdvdfs_reader.py`.
