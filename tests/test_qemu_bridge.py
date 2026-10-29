import importlib.util
import pathlib
import struct
import unittest
from unittest.mock import patch

path = pathlib.Path(__file__).resolve().parents[1] / "scripts" / "qemu_xbe_bridge.py"
spec = importlib.util.spec_from_file_location("qemu_xbe_bridge", path)
bridge = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bridge)


def fixture():
    data = bytearray(0x300)
    data[:4] = b"XBEH"
    for off, value in [(0x104, 0x10000), (0x108, 0x200),
                       (0x10C, 0x1000), (0x11C, 1), (0x120, 0x10180),
                       (0x184, 0x10200), (0x188, 0x100),
                       (0x18C, 0x200), (0x190, 0x100)]:
        struct.pack_into("<I", data, off, value)
    data[0x200] = 0x90
    return data


class BridgeTests(unittest.TestCase):
    def test_valid_image(self):
        base, headers, sections = bridge.parse_xbe(fixture())
        self.assertEqual(base, 0x10000)
        self.assertEqual(len(headers), 0x200)
        self.assertEqual(sections[0][0], 0x10200)
        self.assertEqual(sections[0][2][0], 0x90)

    def test_reject_bad_section(self):
        data = fixture()
        struct.pack_into("<I", data, 0x190, 0x101)
        with self.assertRaises(ValueError):
            bridge.parse_xbe(data)

    def test_reject_invalid_magic(self):
        data = fixture()
        data[0] = 0
        with self.assertRaises(ValueError):
            bridge.parse_xbe(data)


if __name__ == "__main__":
    unittest.main()
