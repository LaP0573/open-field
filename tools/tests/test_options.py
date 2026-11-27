"""The base's options (module/build_r8_options.cpp, base 32, and base 44's options of words) on the PC:
tools/tests/host/options_test.cpp built with emscripten for a 32-bit target and run with node. Its old-base half runs
the same cards on base 38's unit (from the history, commit c3e6ccf, release 0.20; the unit is base 32's), which must not see the options of words. Skipped
where either tool, or for the old half that commit, is missing."""
import os
import shutil
import subprocess
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HOST = os.path.join(ROOT, "tools", "tests", "host")


def build_and_run(out, name, extra):
    js = os.path.join(out, name + ".js")
    subprocess.run(["em++", "-std=c++11", "-DOP1F_HOST", "-I", os.path.join(ROOT, "module"), "-I", HOST] + extra
                   + [os.path.join(HOST, "options_test.cpp"), "-o", js], check=True, capture_output=True)
    return subprocess.run(["node", js], capture_output=True, text=True)


@unittest.skipUnless(shutil.which("em++") and shutil.which("node"), "needs em++ and node")
class Options(unittest.TestCase):
    def test_options_and_options_of_words(self):
        with tempfile.TemporaryDirectory() as out:
            done = build_and_run(out, "options_test", [])
        self.assertEqual(done.returncode, 0, done.stderr)
        self.assertIn("options host test: ok", done.stdout)

    def test_an_older_base_does_not_see_options_of_words(self):
        old = subprocess.run(["git", "-C", ROOT, "show", "c3e6ccf:module/build_r8_options.cpp"], capture_output=True, text=True)
        if old.returncode:
            self.skipTest("base 38's commit is not in this clone")
        with tempfile.TemporaryDirectory() as out:
            source = os.path.join(out, "old_r8.cpp")
            with open(source, "w") as f:
                f.write(old.stdout)
            done = build_and_run(out, "options_old_test", ["-DOLD_UNIT", "-include", source])
        self.assertEqual(done.returncode, 0, done.stderr)
        self.assertIn("options old-base test: ok", done.stdout)


if __name__ == "__main__":
    unittest.main()
