"""The pack tool and an effect's wagon picture (module/op1f_wagon_art.h, docs/level-8.md "Your own wagon picture").

The example crush, built once as it is and once with a wagon named in its ask: the tool writes the ask's table 184
bytes long with the picture's address at +180, links it as an address of the pack's own, and `check` reads it back.
Needs the toolchain in open-xy/ (docs/build.md)."""
import json, os, shutil, subprocess, sys, tempfile, unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TOOLCHAIN = os.path.join(ROOT, "open-xy", "local", "toolchain", "bin")
TOOL = [sys.executable, "-I", os.path.join(ROOT, "tools", "pack_build.py")]
ART = """
#include "op1f_wagon_art.h"
static const unsigned short wagon_colours[2] = {0, 0xffff};
static const unsigned char wagon_pixels[4] = {0, 1, 1, 0};
extern "C" const Op1fWagonArt crush_wagon OP1F_NAMED("crush_wagon");
extern "C" const Op1fWagonArt crush_wagon = {OP1F_WAGON_ART_BYTES, 0, 100, 2, 2, 2, 0, wagon_colours, wagon_pixels};
"""


def built(folder, wagon):
    """The crush pack from a copy in `folder`, with or without a wagon; the words of `check`."""
    source = os.path.join(folder, "crush.cpp")
    shutil.copy(os.path.join(ROOT, "examples", "crush", "crush.cpp"), source)
    if wagon:
        with open(source, "a") as f:
            f.write(ART)
    with open(os.path.join(ROOT, "examples", "crush", "recipe.json")) as f:
        recipe = json.load(f)
    recipe["sources"] = [source, os.path.join(folder, "crush_card.s")]
    recipe["build"] = "test_wagon_crush"
    if wagon:
        recipe["card"]["asks"][0]["wagon"] = "crush_wagon"
    path = os.path.join(folder, "recipe.json")
    with open(path, "w") as f:
        json.dump(recipe, f)
    pack = os.path.join(folder, "crush.pack")
    for step in (["card", "--recipe", path, "--out", recipe["sources"][1]],
                 ["build3", "--recipe", path, "--profile", os.path.join(ROOT, "profiles", "op1f-1.7.3.json"), "--out", pack,
                  "--build-dir", os.path.join(folder, "build")],
                 ["check", "--pack", pack]):
        done = subprocess.run(TOOL + step, cwd=ROOT, capture_output=True, text=True)
        if done.returncode:
            raise AssertionError(done.stdout[-2000:] + done.stderr[-2000:])
    return done.stdout


@unittest.skipUnless(os.path.isdir(TOOLCHAIN), "the toolchain is needed: docs/build.md says how to get it")
class AWagonPicture(unittest.TestCase):
    def test_an_effect_that_names_one_is_built_and_read_back(self):
        with tempfile.TemporaryDirectory() as folder:
            self.assertIn("shown, a wagon's picture", built(folder, True))

    def test_an_effect_without_one_keeps_its_180_bytes(self):
        with tempfile.TemporaryDirectory() as folder:
            words = built(folder, False)
        self.assertIn("effect 'crush'", words)
        self.assertNotIn("wagon", words)


if __name__ == "__main__":
    unittest.main()
