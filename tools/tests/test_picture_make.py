"""tools/picture_make.py: what the unit's reader of pictures stops at, held in `check` and `census`.

  python -I -m unittest discover -s tools/tests -p "test_picture_make.py"

Needs nothing but the tree. The "unit's own picture" here is made up for the test: no picture of the device is in
this repository.
"""

import contextlib
import io
import os
import shutil
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import picture_make  # noqa: E402

OWN = b'<svg>\n<rect id="ground_x5F_plate" fill="#010203" width="333.1234" height="111"/>\n</svg>\n'       # made up for the test
STOPS = "it stops the unit at its start"


def instructions(ours):
    """A set of instructions: the made-up picture with `ours` put in before its last tag."""
    at = OWN.rfind(b"</svg")
    return picture_make.instructions_for_added("dial.svg", OWN, OWN[:at] + ours + OWN[at:])


def stops(ours):
    """What `stops_in` says of a picture made from the made-up one and `ours`."""
    spans = []
    picture = picture_make.make(instructions(ours), lambda name: OWN if name == "dial.svg" else None, None, 4, spans)
    return picture_make.stops_in(picture, spans)


class APartsName(unittest.TestCase):
    def test_a_name_the_reader_stops_at(self):
        for name in ("knob_x", "knob_x1", "knob_xy", "_x", "a_x5F_x4"):
            self.assertEqual(picture_make.id_stop(name), "no `_` ends it", name)
        self.assertEqual(picture_make.id_stop("my_xml_page"), "'ml' is no code")
        self.assertEqual(picture_make.id_stop("row_x2G_a"), "'2G' is no code")
        self.assertEqual(picture_make.id_stop("a_x5F_xzz_"), "'zz' is no code")       # looked at again behind a code

    def test_a_name_it_reads(self):
        for name in ("knob", "knob_1", "x_axis", "dot_x5F_white", "a_x2D_b_x2d_c", "knob_X"):
            self.assertIsNone(picture_make.id_stop(name), name)

    def test_a_name_it_leaves_as_it_is(self):
        for name in ("knob_x_1", "max_xpos_a", "a_x5_b", "a_x005F_b", "a_x_b_x"):   # no code of two characters: it looks no further
            self.assertIsNone(picture_make.id_stop(name), name)


class WhatAPictureHolds(unittest.TestCase):
    def test_nothing_in_a_plain_picture(self):
        self.assertEqual(stops(b'<g id="oud_type_o"><path d="M211.9,198H224.9" stroke-width="1.3"/></g>\n'), [])
        self.assertEqual(stops(b'<g id="a_x5F_b" transform="matrix(1 0 0 1 2.5 3.1234)"><path d="M1.25,2.5l3,4z"/></g>\n'), [])
        self.assertEqual(stops(b'<g transform="translate(10,20.123456)"><g transform="scale(2 , 3)"/></g>\n'), [])

    def test_a_parts_name(self):
        said = stops(b'<g id="oud_type_x"><path d="M1,2H3"/></g>\n')
        self.assertEqual(len(said), 1)
        self.assertIn("names a part 'oud_type_x'", said[0])
        self.assertIn("no `_` ends it", said[0])

    def test_a_number_with_more_digits_than_four(self):
        self.assertEqual(len(stops(b'<path d="M240.12345,31V21"/>\n')), 1)
        self.assertEqual(len(stops(b'<path d="M240,31V21.00001"/>\n')), 1)            # right behind a path's letter
        self.assertEqual(len(stops(b'<polygon points="1,2 3.123456,4 5,6"/>\n')), 1)
        self.assertEqual(len(stops(b'<path stroke-width="1.30000" d="M1,2H3"/>\n')), 1)
        self.assertEqual(len(stops(b'<g transform="matrix(1 0 0 1 2.123456 3)"/>\n')), 1)
        said = stops(b'<path d="M.123456,1L2.5,3.14159"/>\n')
        self.assertEqual(len(said), 2)
        self.assertIn("writes the number .123456, with 6 digits behind the point", said[0])
        self.assertIn("writes the number 3.14159, with 5 digits behind the point", said[1])

    def test_a_number_that_is_not_held_to_it(self):
        self.assertEqual(stops(b'<path d="M1.2345,2.0000L3,4"/>\n'), [])                # four is read
        self.assertEqual(stops(b'<path d="M1.234567e2,2L3,4"/>\n'), [])                 # an exponent: read another way
        self.assertEqual(stops(b'<path fill="#123456" d="M1,2H3"/>\n'), [])             # a colour is no number
        self.assertEqual(stops(b'<g id="v1.234567"><path d="M1,2H3"/></g>\n'), [])      # nor is a name

    def test_a_transform(self):
        self.assertIn("with 5 number(s)", stops(b'<g transform="matrix(1 0 0 1 2)"/>\n')[0])
        for transform in (b"translate(10)", b"scale(2)", b"translate(10-20)", b"translate()"):
            said = stops(b'<g transform="' + transform + b'"/>\n')
            self.assertEqual(len(said), 1, transform)
            self.assertIn("wants two numbers with a comma or a space between them", said[0])
        self.assertEqual(len(stops(b'<g transform="translate(1,2) scale(3)"/>\n')), 1)

    def test_only_the_bytes_of_ours_are_judged(self):
        own = b'<svg>\n<g id="theirs_x"><path d="M1.123456,2H3"/></g>\n</svg>\n'      # made up: not ours to judge
        at = own.rfind(b"</svg")
        data = picture_make.instructions_for_added("dial.svg", own, own[:at] + b'<g id="ours"/>\n' + own[at:])
        spans = []
        picture = picture_make.make(data, lambda name: own, None, 4, spans)
        self.assertEqual(picture_make.stops_in(picture, spans), [])
        self.assertEqual(len(picture_make.stops_in(picture, [(0, len(picture))])), 2)


class TheCommands(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.mkdtemp(prefix="picture-make-test-")
        self.display = os.path.join(self.folder, "display")
        os.makedirs(self.display)
        with open(os.path.join(self.display, "dial.svg"), "wb") as f:
            f.write(OWN)

    def tearDown(self):
        shutil.rmtree(self.folder, ignore_errors=True)

    def run_on(self, command, ours, *more):
        path = os.path.join(self.folder, "page.svg.make")
        with open(path, "wb") as f:
            f.write(instructions(ours))
        said = io.StringIO()
        with contextlib.redirect_stdout(said):
            answer = picture_make.main([command, "--instructions", path, "--display", self.display, *more])
        return answer, said.getvalue()

    def test_check_and_census_let_a_plain_picture_by(self):
        ours = b'<g id="ours_type_o"><path fill="#010203" d="M1,2H3"/></g>\n'
        self.assertEqual(self.run_on("check", ours)[0], 0)
        answer, said = self.run_on("census", ours)
        self.assertEqual((answer, STOPS in said), (0, False))

    def test_check_and_census_refuse_what_the_reader_stops_at(self):
        ours = b'<g id="ours_type_x"><path fill="#010203" d="M1,2H3"/></g>\n'
        for command in ("check", "census"):
            answer, said = self.run_on(command, ours)
            self.assertEqual(answer, 1, command)
            self.assertIn("page.svg.make: names a part 'ours_type_x'", said)
            self.assertIn(STOPS, said)

    def test_many_are_counted_and_the_first_named(self):
        ours = b'<path fill="#010203" d="M' + b"L".join(b"%d.123456,2" % k for k in range(40)) + b'"/>\n'
        answer, said = self.run_on("census", ours)
        self.assertEqual(answer, 1)
        self.assertEqual(said.count(STOPS), picture_make.STOPS_SAID)
        self.assertIn("40 thing(s) the unit's reader of pictures stops at in all", said)


if __name__ == "__main__":
    unittest.main()
