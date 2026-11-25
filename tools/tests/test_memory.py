"""The base's working memory (module/build_r5_memory.cpp) on the PC: tools/tests/host/memory_test.cpp built with
emscripten for a 32-bit target and run with node. Base 39 takes 32 memory asks of the packs in all (16 before).
Skipped where either tool is missing."""
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HOST = os.path.join(ROOT, "tools", "tests", "host")


@unittest.skipUnless(shutil.which("em++") and shutil.which("node"), "needs em++ and node")
class Memory(unittest.TestCase):
    def test_thirty_two_memory_asks(self):
        with tempfile.TemporaryDirectory() as out:
            js = os.path.join(out, "memory_test.js")
            subprocess.run(["em++", "-std=c++11", "-DOP1F_HOST", "-I", os.path.join(ROOT, "module"), "-I", HOST,
                            os.path.join(HOST, "memory_test.cpp"), "-o", js], check=True, capture_output=True)
            done = subprocess.run(["node", js], capture_output=True, text=True)
        self.assertEqual(done.returncode, 0, done.stderr)
        self.assertIn("memory host test: ok", done.stdout)


if __name__ == "__main__":
    unittest.main()
