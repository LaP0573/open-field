# Building from the sources

Two words first. **The loader** is our small piece of code inside the firmware file: at every start it reads the pack files on the unit and runs them. **The base** is the shared pack every other pack relies on, the file `base.pack`.

The install needs no build: the install page has everything it needs, the loader's file and `base.pack` among it. This page is for those who want to see that the finished files here come from these sources, `builder/base.pack` among them, or who want to change them. A feature is built where it lives: `8 tracks`, `fx train`, `lfo` and `sidechain` in [open-field-packs](https://github.com/LaP0573/open-field-packs), whose `build.py` uses this repository's tools and headers.

## What is built, and from what

| Result | From | By |
| --- | --- | --- |
| The base, `base.pack` | its recipe `packs/base.json`, the sources in `module/`, the pictures' instructions in `pictures/` | `tools/pack_build.py` |
| The two loaders: `build/build_n2_loader.bin`, which is the builder's file `builder/loader-1.7.3-v2.bin` byte for byte, and `build/build_m4_loader.bin`, which is `builder/loader-1.7.3-v1.bin` | the sources `build.json` lists | `tools/module_build.py` |
| The example packs | each its own folder in `examples/` | `tools/pack_build.py` |
| The builder's manifest, `builder/loader.json` | the loader's file, its linked file, the profile, and your own `os.ldr` for one number | `builder/write-manifest.mjs` |
| Your firmware file | your own `os.ldr`, the loader's file and the manifest | the builder page, or `builder/build-image.mjs` |

A manifest is the small file that tells the builder what to do: which loader's file to add, where the two changes of the firmware file are, and the size and SHA-256 the finished firmware file must have.

Only the last two steps touch firmware, and they run on your computer with your own file. Nothing before them reads a firmware file.

## What you need

- A current Python 3. Every tool uses the standard library only; run them as `python -I tools/<name>.py`.
- The Blackfin toolchain and one helper file of the project **open-xy** (MIT licence). Put a checkout of it in a folder `open-xy/` at the top of this repository and let it fetch its toolchain:

```bash
git clone https://github.com/etcetc000/open-xy open-xy
```

```bash
python open-xy/sdk.py toolchain fetch
```

  The tools look for `open-xy/tooling/formats.py` and `open-xy/local/toolchain/bin`. The folder `open-xy/` is in `.gitignore`: it is not part of this repository. The builds kept here were made with open-xy at commit `f10f6bd`.
- For the installer's own checks: a current Node.

## One command

From the top of the repository:

```bash
python -I tools/build_all.py
```

It builds everything into `out/` and `build/`, and compares each result with the size and SHA-256 kept in `packs/built.json`, and the install page's own files with what it built: `builder/base.pack` with the `base.pack` it made, and the loader's file in `builder/` with the loader. It answers 0 when every file is the kept one, byte for byte. The compiler is pinned by the toolchain, so the same sources give the same bytes.

`base.pack`, which every unit needs, is then in `out/packs/`, and the example packs in `out/examples/`. The folder `out/` must be new or empty: name another with `--out` for a second build.

## The steps one by one

```bash
python -I tools/table_build.py --profile profiles/op1f-1.7.3.json --check
```

```bash
python -I tools/pack_build.py build3 --recipe packs/base.json --profile profiles/op1f-1.7.3.json --out-dir out/packs
```

```bash
python -I tools/module_build.py --name build_m4_loader --out build --linker linker/op1f-1.7.3-base.ld --include module module/build_j_boot.s module/build_j_base.s module/build_m4_loader.cpp module/build_k6_base_disk.s module/build_m1_loader_disk.cpp module/build_l1_base_table.s module/build_j5b_base.s module/build_l1_base_text_table.s
```

```bash
python -I tools/pack_build.py build3 --recipe examples/wink/recipe.json --profile profiles/op1f-1.7.3.json --out out/wink.pack
```

The loader's binary, `build/build_m4_loader.bin`, is the file the builder adds to a firmware file; `tools/build_all.py` copies it into `out/` under the name `build.json` gives it. The other examples build as `wink` does, with their own recipe; `trampolines` takes its own profile, `examples/trampolines/profile.json`. The tools refuse to overwrite a finished file: build into an empty `out/`.

## Your firmware file, with your own file

```bash
node builder/validate-builder.mjs --stock <your own os.ldr>
```

```bash
node builder/build-image.mjs <your own os.ldr> <the file to write>
```

The first checks the builder: your file, the manifest and the loader's file must give the image whose SHA-256 `builder/loader.json` names. The second writes that image. Neither sends anything anywhere. Keep both files out of the repository: `.gitignore` leaves out every `.ldr` file, and the check before a commit refuses one.

The manifest and the loader's file in `builder/` are made by a tool, from a build. After a build you can see that they are the ones the tool gives:

```bash
node builder/write-manifest.mjs --check --loader out/loader-1.7.3-v2.bin --elf build/build_n2_loader.elf --version v2 --profile profiles/op1f-1.7.3-n2.json --stock <your own os.ldr>
```

It writes nothing. It answers 0 when both files are, byte for byte, what the tool makes from this build and your own file.

## If you change something

- **A place where the loader or the base changes the firmware**: change `profiles/op1f-1.7.3.json`, then write the tables and the change list anew:

```bash
python -I tools/table_build.py --profile profiles/op1f-1.7.3.json --all
```

```bash
python -I tools/change_list.py --profile profiles/op1f-1.7.3-n2.json
```

  A place's `sum` is the CRC-32 of the firmware's own bytes there. You work it out from your own firmware file; the bytes themselves never go into the profile:

```bash
python -I tools/place_sum.py --stock <your own os.ldr> 0x8c0dd488
```

- **A pack's pictures**: a recipe names them. `python -I tools/pack_build.py pictures --recipe <recipe> --out <the pack's pictures source>` writes the directory the pack carries.
- **A pack's card or the base's release**: `python -I tools/pack_build.py card --recipe <recipe> --out <the pack's card source>` writes a pack's card from its recipe, and `python -I tools/pack_build.py release --recipe packs/base.json --out module/build_r6d_base_release.s` the release the start screen shows behind the words open-field.
- **A function of the firmware that a source binds**: add its words to `profiles/op1f-1.7.3-interfaces.json` and write the page anew with `python -I tools/interfaces.py`.
- **Anything in a pack, the base among them**: build again. `packs/built.json` then no longer matches, which is right: a changed pack is another file. Give it another version number in its recipe, and let the build write the new size and SHA-256:

```bash
python -I tools/build_all.py --out out-new --update
```

  `--update` renews the entries of the pack files and of the examples. For a new `base.pack` it also copies the file into `builder/` and writes `builder/base-pack.json` anew: once the change is merged, the install page puts that `base.pack` on every unit it installs, so the maintainer tries it on a unit first. A new `base.pack` also reaches a unit as a file copied into `user/pk`, with no install. `--update` does not renew the entries of the loader and of the builder's file by itself: see the next section.

## A change to the loader

The loader is not a file on the unit. It is the one file in `builder/` that the builder adds to an owner's firmware file. So a change to it is of another weight than a change to a pack, the base included:

1. **It reaches a unit only with a new install.** The owner builds a new firmware file and writes it to the unit, with the risk the guide describes. A pack is a file copied.
2. **It needs a new loader's file under a new version name.** A version that anyone may have installed keeps its bytes for good. So a changed loader takes the next free name: `v3` today, because `v1` and `v2` are there, and its file is `loader-1.7.3-v3.bin`.
3. **It needs a manifest for that file.** The builder writes a firmware file only when its SHA-256 is the one the manifest names, and a changed loader gives another firmware file.

Most changes need no new loader: what a pack can do is the base's, and the base is a pack. A new loader is for a change to what the loader itself does: how it reads, places and joins pack files, its start screen's words, its folder `packs`. Where a pack needs something only a newer loader gives, the pack names the loader's mark (`op1f_loader_1` today), and an older loader leaves it out with a line in the log.

There are two versions of the loader today. `v2` is the one the page offers by itself. It loads more pack files than `v1` and puts them in a room of their own. `v1` stands beside it, for someone who asks for it by name, as the way back. Both are built from the same sources. `build.json` lists `v1`'s under `loader` and `v2`'s under `loader_2`. For `v2`, two files of two lines set `OP1F_LOADER_GENERATION` to 2 and take in the source they stand for. What is for one version alone stands under `#if OP1F_LOADER_GENERATION` in that source. `v2` has a profile of its own, `profiles/op1f-1.7.3-n2.json`, and `write-manifest.mjs` takes it with `--profile`.

You can do all of it on your own computer. It takes the toolchain, Node, and your own OS 1.7.3 file. Open an issue first: such a change is worth agreeing on before the work is done.

### The steps

**1. Make the change.** The loader's sources are the ones `build.json` lists under `loader` and `loader_2`. Change one version at a time: the build gives the new name to the version whose file changed, and it stops when both did.

**2. Build under a new version name.**

```bash
python -I tools/build_all.py --out out-new --update --loader-version v3
```

This names the loader's file `loader-1.7.3-v3.bin` in `build.json`, builds it, and writes the new sizes and SHA-256s into `packs/built.json`. It answers 1 this time, because the builder's own two files are not written yet. Its last lines give the command that writes them.

**3. Write the loader's file and its manifest into `builder/`.**

```bash
node builder/write-manifest.mjs --loader out-new/loader-1.7.3-v3.bin --elf build/build_m4_loader.elf --version v3 --stock <your own os.ldr>
```

It copies the loader's file into `builder/` and writes its manifest, `loader-1.7.3-v3.json`, beside it. `--elf` is the loader's linked file, which the build leaves in `build/`: the address at which the loader is entered is read from it. Your firmware file is read for one thing: the size and the SHA-256 of the firmware file the builder gives with the new loader. Nothing of it is written anywhere, and any file but the exact OS 1.7.3 file is refused.

The command above is for a change to `v1`. For a change to `v2`, the page's own, the command the build gives has `--elf build/build_n2_loader.elf` and `--profile profiles/op1f-1.7.3-n2.json`.

**4. Build again, and check the builder.**

```bash
python -I tools/build_all.py --out out-check
```

```bash
node builder/validate-builder.mjs --loader 1.7.3-v3 --stock <your own os.ldr>
```

The build now answers 0: every file is the kept one, the loader's file in `builder/` is the one that was built, and its manifest names it. The second command makes the firmware file from your own file and holds its SHA-256 to the manifest's.

**5. Run the tests, and send the change.** It holds your sources, `build.json`, `packs/built.json` and the two new files in `builder/`. Say whether you tried it on a unit, and what you saw.

### What an owner gets, and what stays with the maintainer

Your new version stands **beside** the one the builder's page offers. `builder/loader.json` is the manifest the page loads by itself, and you have not touched it. An owner who opens the page gets the same firmware file as before. Your version is offered only to someone who asks for it by name, with `?loader=1.7.3-v3` behind the page's address, and the page shows `not run on a unit yet` beside it.

Two things are still open then:

- **A try on a unit.** A build and a check on a computer say that the firmware file is put together right. They do not say that a unit starts with it. If you have a unit and accept the risk [the guide](guide.md) describes, try your version and say what you saw. The maintainer tries it too, before the next step.
- **Which version the page offers by itself.** That is the maintainer's to say. When the new version has run on a unit, one command makes it the page's own and writes the result beside it:

```bash
node builder/write-manifest.mjs --loader out-check/loader-1.7.3-v3.bin --elf build/build_m4_loader.elf --version v3 --current --status "ran on a unit (2026-11-20)" --stock <your own os.ldr>
```

  The manifest that was the page's own is kept under its own name. So the version before stays there for someone who asks for it by name.

### If you build the loader again

A new version is yours alone until someone else has it. If you change the loader once more before that, keep its name: run step 2 again with the same `--loader-version`, and step 3 with `--replace` at its end, which lets the tool write over your own unfinished file. A version that others may have installed is never written over. It gets a new name.

## The tools

Each tool's first lines are its manual.

| Tool | What it is for |
| --- | --- |
| `build_all.py` | Build everything and hold it to the kept sizes and hashes |
| `pack_build.py` | Make a pack file from its recipe; write a pack's card, pictures and release from its recipe; show what a file holds (`check`); link a set of packs as the unit places them, with a listing (`placed`) |
| `module_build.py` | Compile and link the loader |
| `table_build.py`, `change_list.py`, `interfaces.py` | Write the files that are made from a profile or from the sources: the tables, the change list, the page of the firmware's functions |
| `place_sum.py` | A place's checksum and what a trampoline copies there, from your own firmware file |
| `ldr_inspect.py`, `ldr_dump_records.py` | List a firmware file's records; write them as a folder for the tools below |
| `bfin_sweep.py`, `bfin_callers.py`, `ldr_xref.py`, `ldrmem.py` | Read your own firmware: its code one instruction a line, who refers to an address, where a text is used, what stands at an address ([methods](methods.md)) |
| `stub_calls.py` | Hold every call of a hand-written routine to the rule of the twelve free bytes |
| `os_port.py` | Find a profile's places and the sources' addresses in the firmware file of another version |
| `picture_make.py`, `picture_map.py`, `picture_delta.py`, `picture_take.py`, `picture_provenance.py` | The instructions for a screen's picture: make one, check one, and measure that a pack holds nothing of the device's pictures ([the base](base.md)) |
| `publish_check.py`, `stock_search.py`, `commit_check.py` | The check before a commit ([checking](checking.md)) |

The tools that read a firmware file print the firmware's own code and data. Keep what they print out of the repository.

## The tests

```bash
python -I -m unittest discover -s tools/tests
```

The tests of the tree's own consistency need nothing. Those that hold the profile's checksums to a firmware file need your own: set `OPEN_FIELD_STOCK` to its path, and they are skipped without it.

## What the names mean

The file names in `module/` carry the letters of the steps in which each part was first built (`build_b_…`, `build_k_…`, `build_m4_…`). They are kept because the recipes, the comments and the history of the work use them: so the loader's table is still named `build_l1_base_table.s`, from the days when the loader was called the base. Some comments point at working notes that are not in this repository.
