import pathlib
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "scripts"))
from test_xdvdfs_reader import make_iso

SCRIPT = pathlib.Path(__file__).resolve().parents[1] / "scripts" / "qemu_xbe_bridge.py"


class BridgeIsoTests(unittest.TestCase):
    def test_iso_detects_xbe_and_rejects_truncated_header(self):
        with tempfile.TemporaryDirectory() as tmp:
            iso = pathlib.Path(tmp) / "game.iso"
            make_iso(iso)
            result = subprocess.run(
                [sys.executable, str(SCRIPT), str(iso), "--iso"],
                capture_output=True, text=True, timeout=10)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("XDVDFS:", result.stdout)
            self.assertIn("Invalid XBEH header", result.stderr)


if __name__ == "__main__":
    unittest.main()
