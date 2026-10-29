#!/usr/bin/env python3
"""Transfer validated XBE sections to a paused QEMU i386 guest via GDB RSP.

This is a real CPU/memory transport integration, NOT an Xbox kernel runtime.
It does not configure protected mode, paging, entrypoint, or execute XBE code.
"""
import argparse
from pathlib import Path

from xdvdfs_reader import XDVDFS
import socket
import struct
import sys

MAX_IMAGE = 256 * 1024 * 1024
MAX_SECTIONS = 4096
CHUNK = 512


def u32(data, offset):
    return struct.unpack_from("<I", data, offset)[0]


def parse_xbe(data):
    if len(data) < 0x180 or data[:4] != b"XBEH":
        raise ValueError("Invalid XBEH header")
    base, header_size, image_size = (u32(data, x) for x in (0x104, 0x108, 0x10C))
    count, table_va = (u32(data, x) for x in (0x11C, 0x120))
    if not (0x180 <= header_size <= len(data)) or not (header_size <= image_size <= MAX_IMAGE):
        raise ValueError("Invalid image/header size")
    if count > MAX_SECTIONS or table_va < base:
        raise ValueError("Invalid section table")
    table = table_va - base
    if table > header_size or count * 0x38 > header_size - table:
        raise ValueError("Section table outside header")
    sections = []
    for i in range(count):
        off = table + i * 0x38
        va, vs, raw, size = (u32(data, off + x) for x in (4, 8, 12, 16))
        if (va < base or va - base > image_size or vs > image_size - (va - base)
                or raw > len(data) or size > len(data) - raw or size > vs):
            raise ValueError(f"Invalid section {i}")
        sections.append((va, vs, data[raw:raw + size]))
    return base, data[:header_size], sections


class RSP:
    def __init__(self, host, port, timeout):
        self.sock = socket.create_connection((host, port), timeout=timeout)
        self.sock.settimeout(timeout)

    def close(self):
        self.sock.close()

    def read_byte(self):
        value = self.sock.recv(1)
        if not value:
            raise ConnectionError("QEMU closed GDB connection")
        return value

    def command(self, payload):
        encoded = payload.encode("ascii")
        checksum = sum(encoded) & 0xff
        self.sock.sendall(b"$" + encoded + f"#{checksum:02x}".encode())
        if self.read_byte() != b"+":
            raise ConnectionError("GDB packet not acknowledged")
        while self.read_byte() != b"$":
            pass
        result = bytearray()
        while True:
            ch = self.read_byte()
            if ch == b"#":
                break
            result.extend(ch)
        check = int(self.read_byte() + self.read_byte(), 16)
        if sum(result) & 0xff != check:
            self.sock.sendall(b"-")
            raise ConnectionError("Invalid GDB response checksum")
        self.sock.sendall(b"+")
        answer = result.decode("ascii")
        if answer.startswith("E") and len(answer) == 3:
            raise RuntimeError(f"QEMU GDB error {answer}")
        return answer

    def write(self, address, data):
        for start in range(0, len(data), CHUNK):
            block = data[start:start + CHUNK]
            location = address + start
            response = self.command(f"M{location:x},{len(block):x}:{block.hex()}")
            if response != "OK":
                raise RuntimeError(f"QEMU rejected write at 0x{location:x}: {response}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("xbe", help="Path to a locally owned XBE or Xbox XDVDFS ISO")
    parser.add_argument("--iso", action="store_true", help="Read D:/default.xbe directly from an XDVDFS ISO")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=1234)
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument("--write", action="store_true",
                        help="Explicitly authorize writing XBE headers/sections into QEMU guest memory")
    args = parser.parse_args()
    if args.iso:
        image = XDVDFS(args.xbe)
        try:
            data_path = Path(args.xbe)
            image.walk()
            # Read the XBE directly from the ISO; no intermediate output file.
            record = image.entries.get("default.xbe")
            if record is None or record[3] & 0x10:
                raise ValueError("default.xbe not found in ISO root")
            if record[2] > MAX_IMAGE:
                raise ValueError("XBE file exceeds maximum supported size")
            data = image.read(record[1] * 2048, record[2])
            print(f"XDVDFS: {data_path.name}, default.xbe={len(data)} bytes")
        finally:
            image.close()
    else:
        with open(args.xbe, "rb") as handle:
            data = handle.read()
    base, header, sections = parse_xbe(data)
    print(f"XBE base=0x{base:08x}, sections={len(sections)}")
    if not args.write:
        print("Dry run. Add --write to transfer bytes to a paused QEMU guest.")
        return 0
    rsp = RSP(args.host, args.port, args.timeout)
    try:
        print("QEMU GDB target:", rsp.command("qSupported:multiprocess+"))
        rsp.write(base, header)
        print(f"Wrote {len(header)} header bytes at 0x{base:08x}")
        for i, (va, virtual_size, raw) in enumerate(sections):
            rsp.write(va, raw)
            if virtual_size > len(raw):
                # Initialize BSS/zero-filled tail of each section.
                zero = bytes(CHUNK)
                remaining = virtual_size - len(raw)
                at = va + len(raw)
                while remaining:
                    size = min(remaining, CHUNK)
                    rsp.write(at, zero[:size])
                    at += size
                    remaining -= size
            print(f"Section {i}: 0x{va:08x}, virtual={virtual_size}, raw={len(raw)}")
        print("XBE image transferred. CPU execution NOT started.")
    finally:
        rsp.close()
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, ConnectionError, RuntimeError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        sys.exit(1)
