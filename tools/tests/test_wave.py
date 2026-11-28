"""tools/pack_build.py and base 45: an LFO layer's wave, as a recipe asks for it and as the card's source names it.

  python -I -m unittest discover -s tools/tests -p "test_wave.py"

Needs nothing but the tree.
"""

import json
import os
import re
import struct
import sys
import unittest
from unittest import mock

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import pack_build  # noqa: E402

SWAY = {"kind": "lfo", "name": "sway", "drum": True, "shown": "sway_shown", "wave": "sway_wave", "cycles": 3000}


def plain(**changed):
    """The ask of a layer with no wave, with `changed` put in."""
    ask = {"kind": "lfo", "name": "sway", "drum": True, "shown": None}
    ask.update(changed)
    return ask


def waved(**changed):
    """The ask of a layer with a wave, with `changed` put in (None takes a key out)."""
    ask = dict(SWAY, **changed)
    return {key: value for key, value in ask.items() if value is not None or key == "shown"}


def recipe(*asks):
    return {"name": "sway", "version": 1, "card": {"level": 10, "frame": "sway_frame", "asks": list(asks)}}


class AnAsk(unittest.TestCase):
    def test_a_wave_gives_36_bytes_and_two_words(self):
        ask = pack_build.lfo_of_card(1, SWAY, [])
        self.assertEqual(ask[0], 11)
        self.assertEqual(ask[3], {"shown": "sway_shown", "wave": "sway_wave"})
        self.assertEqual(ask[4], {"cycles": 3000})
        self.assertEqual(pack_build.table_bytes_of(ask), 36)
        self.assertEqual(pack_build.ask_table_words(ask), ["sway_wave", 3000])
        self.assertTrue(pack_build.has_wave([ask]))
        self.assertTrue(pack_build.ask_words(*ask).endswith(", a wave of 3000 cycles"))

    def test_no_wave_gives_28_bytes_and_no_word(self):
        ask = pack_build.lfo_of_card(1, plain(), [])
        self.assertEqual(ask[4], {})
        self.assertEqual(pack_build.table_bytes_of(ask), 28)
        self.assertEqual(pack_build.ask_table_words(ask), [])
        self.assertFalse(pack_build.has_wave([ask]))
        self.assertNotIn("wave", pack_build.ask_words(*ask))
        self.assertEqual(pack_build.lfo_of_card(1, plain(wave=None), [])[4], {})

    def test_refused(self):
        for ask, said in ((waved(cycles=0), "cost is not between 1 and 8000"),
                          (waved(cycles=8001), "cost is not between 1 and 8000"),
                          (waved(cycles="3000"), "cost is not between 1 and 8000"),
                          (waved(cycles=True), "cost is not between 1 and 8000"),
                          (waved(cycles=None), "cost is not between 1 and 8000"),
                          (plain(cycles=3000), "a cost and no wave"),
                          (waved(wave="not a symbol"), "its wave is the symbol"),
                          (waved(wave=7), "its wave is the symbol"),
                          (waved(speed=2), "'speed'")):
            with self.subTest(ask=ask):
                with self.assertRaisesRegex(ValueError, said):
                    pack_build.lfo_of_card(1, ask, [])

    def test_the_most_is_taken(self):
        self.assertEqual(pack_build.lfo_of_card(1, waved(cycles=1), [])[4], {"cycles": 1})
        self.assertEqual(pack_build.lfo_of_card(1, waved(cycles=8000), [])[4], {"cycles": 8000})


class TheCard(unittest.TestCase):
    def test_a_wave_names_both_marks_in_order(self):
        text = pack_build.render_card(recipe(SWAY), "sway.json")
        layer, wave = text.find("    .long op1f_layer_1\n"), text.find("    .long op1f_wave_1\n")
        self.assertGreater(layer, 0)
        self.assertGreater(wave, layer)
        self.assertIn("op1f_card_sway_wave:", text)
        self.assertIn("    .long 11, 36, op1f_card_sway_ask_0\n", text)
        self.assertIn("    .long sway_wave\n    .long 3000\n", text)

    def test_no_wave_no_mark(self):
        text = pack_build.render_card(recipe(plain()), "sway.json")
        self.assertIn("    .long op1f_layer_1\n", text)
        self.assertNotIn("op1f_wave_1", text)
        self.assertNotIn("_wave", text)
        self.assertIn("    .long 11, 28, op1f_card_sway_ask_0\n", text)


class TheBase(unittest.TestCase):
    def test_the_header_and_the_tool_agree(self):
        with open(os.path.join(ROOT, "module", "build_r6e_card.h")) as f:
            header = f.read()
        found = re.search(r"OP1F_LFO_WAVE_BYTES = (\d+), OP1F_WAVE_BYTES = \d+, OP1F_WAVE_CYCLES_MOST = (\d+)", header)
        self.assertIsNotNone(found)
        self.assertEqual(int(found.group(1)), pack_build.LFO_WAVE_BYTES)
        self.assertEqual(int(found.group(2)), pack_build.WAVE_CYCLES_MOST)
        self.assertIn('op1f_wave_1 OP1F_NAMED("%s")' % pack_build.WAVE_MARK, header)

    def test_the_base_offers_the_mark(self):
        with open(os.path.join(ROOT, "packs", "base.json")) as f:
            base = json.load(f)
        self.assertIn(pack_build.WAVE_MARK, base["exports"])
        with open(os.path.join(ROOT, "module", "build_r11_lfos.cpp")) as f:
            self.assertRegex(f.read(), r"\nconst unsigned op1f_wave_1 = 1;")


LOAD, TABLE_AT, WAVE_AT, ASKS_AT = 0x1000, 0x100, 0x40, 0x200


def table(bytes_=36, wave=LOAD + WAVE_AT, cycles=3000):
    """An LFO table as a pack holds it: the head of `bytes_`, then the wave and its cost where the head says 36."""
    head = pack_build.KIND_HEAD.pack(bytes_, b"sway", pack_build.LFO_DRUM, 0)
    raw = head + struct.pack("<2I", wave, cycles) if bytes_ >= 36 else head
    return raw.ljust(bytes_, b"\0")


def fields_of(raw, marks):
    """A pack's fields around one LFO table at TABLE_AT, linked at LOAD, needing `marks`."""
    body = bytearray(0x300)
    body[TABLE_AT:TABLE_AT + len(raw)] = raw
    imports = [(pack_build.level_name(10), True), (pack_build.LAYER_MARK, True)] + [(m, True) for m in marks]
    places = [(pack_build.CARD_3_BYTES, pack_build.KIND_WORD, None, LOAD + ASKS_AT)]
    return {"name": "sway", "version": 1, "load_address": LOAD, "body_bytes": len(body), "bss_bytes": 0,
            "lists": {"imports": imports, "exports": [], "places": places}}, body


def faults_of(raw, marks):
    """What card_faults says of a card of level 10 whose one ask is the LFO table `raw`."""
    fields, body = fields_of(raw, marks)
    card = {"bytes": 64, "name": "sway", "version": 1, "place": 0, "level": 10, "pictures": 0,
            "folder_count": 0, "folders": 0, "ask_count": 1, "asks": LOAD + ASKS_AT}
    with mock.patch.object(pack_build, "parse3", return_value=(fields, body, None)), \
            mock.patch.object(pack_build, "card_in", return_value=card), \
            mock.patch.object(pack_build, "asks_in", return_value=[(pack_build.LFO, len(raw), LOAD + TABLE_AT)]):
        return pack_build.card_faults(b"")


class TheReader(unittest.TestCase):
    def read(self, raw, waved):
        fields, body = fields_of(raw, [])
        return pack_build.lfo_table(fields, body, len(raw), LOAD + TABLE_AT, [], waved)

    def test_a_36_byte_table_is_read_with_its_wave(self):
        kind, size, _what, ask, wrong = self.read(table(), True)
        self.assertEqual((kind, size, wrong), (11, 36, None))
        self.assertEqual(ask[3], {"shown": 0, "wave": LOAD + WAVE_AT})
        self.assertEqual(ask[4], {"cycles": 3000})
        self.assertTrue(pack_build.has_wave([ask]))

    def test_without_the_mark_it_is_read_as_before(self):
        ask = self.read(table(), False)[3]
        self.assertEqual(ask[4], {})
        self.assertFalse(pack_build.has_wave([ask]))

    def test_the_wave_s_faults(self):
        for raw, said in ((table(wave=LOAD + 0x300), "its wave does not lie in its pack on an even address"),
                          (table(wave=LOAD + WAVE_AT + 1), "its wave does not lie in its pack on an even address"),
                          (table(cycles=0), "its wave's cost is not between 1 and 8000 cycles a call"),
                          (table(cycles=8001), "its wave's cost is not between 1 and 8000 cycles a call"),
                          (table(40, wave=LOAD + WAVE_AT + 1), "its wave does not lie in its pack on an even address")):
            with self.subTest(said=said, bytes=len(raw)):
                self.assertEqual(self.read(raw, True)[4], said)

    def test_40_bytes_with_a_wave_is_a_wave(self):
        kind, size, _what, ask, wrong = self.read(table(40), True)
        self.assertEqual((kind, size, wrong), (11, 40, None))
        self.assertEqual(ask[3], {"shown": 0, "wave": LOAD + WAVE_AT})
        self.assertEqual(ask[4], {"cycles": 3000})
        self.assertTrue(pack_build.has_wave([ask]))

    def test_a_wave_word_of_0_is_a_plain_layer(self):
        for raw in (table(36, wave=0, cycles=0), table(40, wave=0, cycles=99999)):
            with self.subTest(bytes=len(raw)):
                _kind, _size, _what, ask, wrong = self.read(raw, True)
                self.assertIsNone(wrong)
                self.assertEqual((ask[3], ask[4]), ({"shown": 0}, {}))
                self.assertFalse(pack_build.has_wave([ask]))

    def test_32_bytes_is_a_plain_layer(self):
        for waved in (True, False):
            with self.subTest(waved=waved):
                _kind, _size, _what, ask, wrong = self.read(table(32), waved)
                self.assertIsNone(wrong)
                self.assertFalse(pack_build.has_wave([ask]))


class TheCheck(unittest.TestCase):
    def test_a_wave_needs_the_mark(self):
        said = "it asks for a wave and the pack does not need op1f_wave_1"
        self.assertTrue(any(said in f for f in faults_of(table(), [])))
        self.assertFalse(any(said in f for f in faults_of(table(), [pack_build.WAVE_MARK])))
        self.assertFalse(any("wave" in f or "base 45" in f for f in faults_of(table(28), [])))

    def test_what_base_45_would_leave_aside(self):
        for raw, said in ((table(wave=LOAD + WAVE_AT + 1), "its wave does not lie"),
                          (table(40, wave=LOAD + 0x300), "its wave does not lie"),
                          (table(cycles=0), "its wave's cost is not between")):
            with self.subTest(said=said, bytes=len(raw)):
                faults = faults_of(raw, [])
                self.assertTrue(any(f.startswith("its ask 1 is not in order for base 45") and said in f for f in faults), faults)

    def test_other_sizes_are_not_faults(self):
        for raw in (table(32), table(36, wave=0, cycles=0), table(40, wave=0)):
            with self.subTest(bytes=len(raw)):
                self.assertFalse(any("base 45" in f or "not in order" in f for f in faults_of(raw, [])), faults_of(raw, []))


if __name__ == "__main__":
    unittest.main()
