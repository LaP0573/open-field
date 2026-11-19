# Credits: parts from other projects, and work this project learnt from

## Parts that are reused

Two projects' work is reused here, each under the MIT licence. Their licence texts are kept beside the code.

| Project | What is reused | Where | Licence text |
| --- | --- | --- | --- |
| **Ferret**, by Bag of Waffles (https://github.com/bagofwaffles/ferret) | The code that talks to the bootloader over USB MIDI to switch its developer menu on and off, by way of open-xy's adaptation of it | `builder/ferret.mjs` | `builder/ferret.LICENSE` |
| **open-xy**, by the Open-XY contributors (https://github.com/etcetc000/open-xy) | The handling of the firmware file's format, ported to JavaScript; the compiler settings | `builder/ldr.mjs`, parts of `builder/patch.mjs`, the flags in `tools/module_build.py` | `builder/open-xy.LICENSE` |

The build tools also use, without copying it, one file of open-xy (`tooling/formats.py`) and the compiler toolchain that project provides. [docs/build.md](docs/build.md) says how to get them. They are not part of this repository.

Changes made here: `builder/ferret.mjs` uses the OP-1 Field's message header, confirms the unit by its answers, and computes one table that the original writes out.

## Work this project learnt from

No code was taken for these. The ideas were.

- **open-xy** showed the way this whole project works: a builder that adds to the firmware file an owner copies off their own unit, so that no firmware is ever passed on; a list of the places that are changed, kept apart from the code; and an install through the bootloader's own menu, with no key.
- **Ferret** showed that the bootloader's developer menu can be switched on over USB.
- **Juno60**, by pendragon-andyh (https://github.com/pendragon-andyh/Juno60), measured the Juno-60 chorus: the rates, sweeps and balance of its modes, which `examples/juneaux` uses.

If your work helped and is not named here, that is an oversight. Open an issue and it will be put right.
