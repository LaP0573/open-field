"""On the PC, built with emscripten for a 32-bit target and run with node (skipped where either is missing): the base's
options (module/build_r8_options.cpp, base 31), tools/tests/host/options_test.cpp."""
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


@unittest.skipUnless(shutil.which("em++") and shutil.which("node"), "needs em++ and node")
class OnThePC(unittest.TestCase):
    def test_values_read_changed_told_and_written(self):
        with tempfile.TemporaryDirectory() as out:
            js = os.path.join(out, "options_test.js")
            subprocess.run(["em++", "-std=c++11", "-DOP1F_HOST", "-I", os.path.join(ROOT, "module"), "-I", os.path.join(ROOT, "tools", "tests", "host"),
                            os.path.join(ROOT, "tools", "tests", "host", "options_test.cpp"), "-o", js], check=True, capture_output=True)
            done = subprocess.run(["node", js], capture_output=True, text=True)
        self.assertEqual(done.returncode, 0, done.stderr)
        self.assertIn("options host test: ok", done.stdout)


if __name__ == "__main__":
    unittest.main()
