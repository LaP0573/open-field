## What it changes, and why



## How to try it



## What was tried on a unit

Which unit (OS and bootloader versions), which pack files were on it, what you did and what the unit did. Say what you did not try. "Built only, not tried on a unit" is a fine answer, as long as it is said.



## For a new place in the firmware

How you know what stands there, in your own words. Leave this out when the change adds no place.



## Whose work it builds on

A project, a post or a person, if any. They are named in THIRD-PARTY.md.



## Before sending

- [ ] `python -I -m unittest discover -s tools/tests` passes.
- [ ] `python -I tools/build_all.py` says `the same` for every file, or the change gives each changed example or pack a new version and renews `packs/built.json` with `--update`. A change to the base carries its sources only, with no new version or release, and the build says `ahead` for `base.pack` (`docs/build.md`).
- [ ] `node builder/validate-devmode.mjs` passes.
- [ ] The check before a commit ran with my own firmware at hand, or, without one, `python -I tools/publish_check.py --forms-only` on the files I changed.
- [ ] Nothing of the firmware is in the change: no firmware file or part of one, no log of a unit, no listing of its code, no picture of the device (`CONTRIBUTING.md`).

The checks the host runs on this change (`.github/workflows/checks.yml`) are not the whole check: the check for anything of the firmware needs the maintainer's own copy of it, and runs before the change goes in (`docs/checking.md`).
