#!/usr/bin/env python3
"""Read Xbox XDVDFS game images; list files or extract default.xbe.

Supports raw 2048-byte-sector XISO and common raw 2352-byte-sector images.
Read-only: does not execute the XBE.
"""
import argparse
import pathlib
import struct
import sys

MAGIC = b"MICROSOFT*XBOX*MEDIA"
SECTOR = 2048
MAX_TREE_NODES = 200000


class XDVDFS:
    def __init__(self, path):
        self.file = open(path, "rb")
        self.size = pathlib.Path(path).stat().st_size
        self.layout = self.detect()
        self.root_sector, self.root_size = self.volume()
        self.entries = {}

    def close(self):
        self.file.close()

    def read(self, offset, length):
        if offset < 0 or length < 0:
            raise ValueError("Negative image offset")
        stride, prefix, base = self.layout
        out = bytearray()
        while length:
            sector, within = divmod(offset, SECTOR)
            amount = min(length, SECTOR - within)
            physical = base + sector * stride + prefix + within
            if physical + amount > self.size:
                raise ValueError("Image read exceeds file")
            self.file.seek(physical)
            data = self.file.read(amount)
            if len(data) != amount:
                raise ValueError("Truncated image")
            out.extend(data)
            offset += amount
            length -= amount
        return bytes(out)

    def detect(self):
        # Descriptor is at logical sector 32. Raw dumps can include 16 or
        # 24 bytes of framing before each 2048-byte data payload.
        for base in (0, 387 * 2048):
            for stride, prefix in ((2048, 0), (2352, 16), (2352, 24)):
                offset = base + 32 * stride + prefix
                if offset + len(MAGIC) > self.size:
                    continue
                self.file.seek(offset)
                if self.file.read(len(MAGIC)) == MAGIC:
                    return stride, prefix, base
        raise ValueError("XDVDFS volume descriptor not found")

    def volume(self):
        descriptor = self.read(32 * SECTOR, 32)
        if descriptor[:len(MAGIC)] != MAGIC:
            raise ValueError("Invalid XDVDFS magic")
        root_sector, root_size = struct.unpack_from("<II", descriptor, 20)
        if root_size == 0 or root_size > 64 * 1024 * 1024:
            raise ValueError("Invalid root directory size")
        self.read(root_sector * SECTOR, root_size)
        return root_sector, root_size

    def walk(self):
        self.entries = {}
        seen_dirs = set()

        def directory(sector, size, prefix, depth):
            if depth > 32 or len(self.entries) > MAX_TREE_NODES:
                raise ValueError("Directory nesting/entry limit exceeded")
            key = (sector, size)
            if key in seen_dirs:
                raise ValueError("Directory cycle detected")
            seen_dirs.add(key)
            data = self.read(sector * SECTOR, size)
            visited = set()

            def node(offset):
                if offset in visited:
                    raise ValueError("Directory tree cycle detected")
                visited.add(offset)
                if offset + 14 > len(data):
                    raise ValueError("Directory entry outside table")
                left, right, first_sector, length, attr, name_len = struct.unpack_from(
                    "<HHIIBB", data, offset)
                if not name_len or offset + 14 + name_len > len(data):
                    raise ValueError("Invalid filename length")
                name = data[offset + 14:offset + 14 + name_len].decode("latin-1")
                if name in (".", "..") or "/" in name or "\\" in name:
                    raise ValueError("Unsafe filename")
                if left:
                    node(left * 4)
                full = prefix + name
                if full.lower() in self.entries:
                    raise ValueError("Duplicate filename")
                self.entries[full.lower()] = (full, first_sector, length, attr)
                if attr & 0x10:
                    if length:
                        directory(first_sector, length, full + "/", depth + 1)
                elif length:
                    self.read(first_sector * SECTOR, min(length, 1))
                    # Check last byte without loading entire game assets.
                    self.read(first_sector * SECTOR + length - 1, 1)
                if right:
                    node(right * 4)

            if size:
                node(0)

        directory(self.root_sector, self.root_size, "", 0)
        return self.entries

    def extract(self, name, destination):
        if not self.entries:
            self.walk()
        record = self.entries.get(name.lower().replace("\\", "/"))
        if record is None:
            raise FileNotFoundError(f"File not found in XDVDFS: {name}")
        _, sector, size, attr = record
        if attr & 0x10:
            raise ValueError("Cannot extract a directory")
        output = pathlib.Path(destination)
        with output.open("wb") as handle:
            offset = sector * SECTOR
            remaining = size
            while remaining:
                amount = min(remaining, 1024 * 1024)
                handle.write(self.read(offset, amount))
                offset += amount
                remaining -= amount
        return size


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("iso", help="Xbox XDVDFS image")
    parser.add_argument("--list", action="store_true", help="List all files")
    parser.add_argument("--extract", default="default.xbe",
                        help="File to extract (default: default.xbe)")
    parser.add_argument("--output", default="default.xbe",
                        help="Output filename")
    args = parser.parse_args()
    image = XDVDFS(args.iso)
    try:
        entries = image.walk()
        if args.list:
            for name, sector, size, attr in entries.values():
                print(f"{'DIR ' if attr & 0x10 else 'FILE'} {size:>10} {name}")
            return 0
        count = image.extract(args.extract, args.output)
        print(f"Extracted {args.extract} ({count} bytes) -> {args.output}")
        return 0
    finally:
        image.close()


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError) as error:
        print(f"XDVDFS error: {error}", file=sys.stderr)
        sys.exit(1)
