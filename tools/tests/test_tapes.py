"""The base's files per tape (module/build_r7_tapes.cpp, base 43) on the PC: tools/tests/host/tapes_test.cpp built with
emscripten for a 32-bit target and run with node. Skipped where either is missing."""
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


@unittest.skipUnless(shutil.which("em++") and shutil.which("node"), "needs em++ and node")
class FilesPerTape(unittest.TestCase):
    def test_copies_and_deletes_follow_the_tapes(self):
        with tempfile.TemporaryDirectory() as out:
            js = os.path.join(out, "tapes_test.js")
            subprocess.run(["em++", "-std=c++11", "-DOP1F_HOST", "-I", os.path.join(ROOT, "module"), "-I", os.path.join(ROOT, "tools", "tests", "host"),
                            os.path.join(ROOT, "tools", "tests", "host", "tapes_test.cpp"), "-o", js], check=True, capture_output=True)
            done = subprocess.run(["node", js], capture_output=True, text=True)
        self.assertEqual(done.returncode, 0, done.stderr)
        self.assertIn("tapes host test: ok", done.stdout)


if __name__ == "__main__":
    unittest.main()
