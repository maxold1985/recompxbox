import importlib.util
import pathlib
import struct
import tempfile
import unittest

module_path = pathlib.Path(__file__).resolve().parents[1] / "scripts" / "xdvdfs_reader.py"
spec = importlib.util.spec_from_file_location("xdvdfs_reader", module_path)
reader = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reader)


def make_iso(path, corrupt=False):
    data = bytearray(40 * 2048)
    volume = 32 * 2048
    data[volume:volume + len(reader.MAGIC)] = reader.MAGIC
    struct.pack_into("<II", data, volume + 20, 34, 2048)
    entry = 34 * 2048
    name = b"default.xbe"
    struct.pack_into("<HHIIBB", data, entry, 0, 0, 35, 4, 0, len(name))
    data[entry + 14:entry + 14 + len(name)] = name
    data[35 * 2048:35 * 2048 + 4] = b"XBEH"
    if corrupt:
        struct.pack_into("<I", data, entry + 8, 1000000)
    path.write_bytes(data)


class XDVDFSTests(unittest.TestCase):
    def test_extract(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "game.iso"
            dest = pathlib.Path(directory) / "default.xbe"
            make_iso(path)
            image = reader.XDVDFS(path)
            try:
                self.assertIn("default.xbe", image.walk())
                self.assertEqual(image.extract("DEFAULT.XBE", dest), 4)
            finally:
                image.close()
            self.assertEqual(dest.read_bytes(), b"XBEH")

    def test_bad_magic(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "bad.iso"
            path.write_bytes(bytes(40 * 2048))
            with self.assertRaises(ValueError):
                reader.XDVDFS(path)

    def test_bad_extent(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "bad.iso"
            make_iso(path, corrupt=True)
            image = reader.XDVDFS(path)
            try:
                with self.assertRaises(ValueError):
                    image.walk()
            finally:
                image.close()


if __name__ == "__main__":
    unittest.main()
