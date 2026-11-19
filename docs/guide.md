# The owner's guide

How to put open-field on your own OP-1 Field, how to add and take away a feature, how to start the unit as the original firmware, and how to go back for good.

open-field has three parts. **The loader** is a small piece of our code that goes, once, into the firmware file of your unit: at every start it reads the pack files on the unit. **The base**, the file `base.pack`, is the shared pack every feature needs. **The features** are one file each, `8 tracks.pack` say. The install puts the loader in and `base.pack` on the unit; the features are files you copy.

Read the whole page before you begin. The install changes the firmware of your unit. A write that goes wrong can leave a unit unable to start, for good. Installing this will void the warranty of your device. open-field is unofficial: it is not made or supported by Teenage Engineering, and it is not affiliated with them. Whether to install it is your decision, and you do it at your own risk ([NOTICE.md](../NOTICE.md)).

## What you need

- An OP-1 Field with the original **OS 1.7.3** and bootloader **1000.1.1**. The install page checks both and refuses any other. The original firmware's start screen shows `1.7.3`. A build of 1.7.3 with another number behind it, such as `1.7.3+1`, is another firmware file: the page's check of the firmware file in step 5 refuses it.
- A **Windows** computer with **Chrome or Edge**, and a USB cable that carries data. A Mac or a Linux computer cannot be used yet.
- A **charged** unit. A write that loses power halfway can leave nothing to start from.
- Half an hour in which you will not be interrupted.

**If your unit is on a later OS version than 1.7.3, you cannot use this yet.** The builder takes only the OS 1.7.3 file, which such a unit does not hold. Whether such a unit can be brought back to OS 1.7.3 is not known: do not count on it.

**If your unit is on OS 1.7.3 and you want to keep open-field, do not update the unit.** An official update writes the maker's firmware over the one with our loader in it. The unit is then on the original firmware of the new version, open-field is gone from it, and you are in the case above.

### A unit on OS 1.7.0

open-field was made for OS 1.7.0 first. A unit still on OS 1.7.0 has two ways:

- **Update the unit to OS 1.7.3** with Teenage Engineering's own update, as they offer it, and then install open-field from this page as on any unit. The update writes the maker's firmware: if the unit ran open-field for 1.7.0, the loader is gone after it, and the unit is on the original OS 1.7.3. Check which version the update gives before you run it: this page works with OS 1.7.3 itself. A later build, such as `1.7.3+1`, is another firmware, and the page refuses it.
- **Stay on OS 1.7.0** and use the page kept for it: the install page links to it at the top ("Still on OS 1.7.0?"). It is the release of before, kept as it was, and it will not change. Use it only on a unit that was never updated to 1.7.3: on a unit that was, it stops at its own first step.

**Pack files left from OS 1.7.0.** A pack file is made for one firmware. The loader for 1.7.3 leaves out every pack file made for 1.7.0 and starts everything else; the start screen counts what it left out, `1.7.3 - open-field 0.3 (4 left out)` for four such files. Nothing breaks, but those features do not run. Two ways to set it right:

- In step 5 of the install, tick the packs again in the listing: the page puts their 1.7.3 files into the folder `packs`, and the helper copies them onto the unit in place of the old files of the same name. `base.pack` is always the page's own. Look at the folder `packs` inside `open-field` on your computer before step 5, too: a pack file from your 1.7.0 install that is still there would be copied onto the unit again, and left out again. Delete the ones you do not tick.
- Or delete the old files from the folder `packs` on the unit in the disk mode ([below](#taking-a-feature-away)).

## Two rules before anything else

1. **Keep the firmware file you copy off your unit.** The helper puts it into the folder `open-field` in your user folder as `os-1.7.3-yours.ldr`. Put a copy somewhere safe as well, away from the computer you work on. It is your way back.
2. **In the bootloader's menus press only the keys this guide names.** In its main menu, `3` opens the developer menu. In the developer menu you will only ever need `2` and `4`: its key `1` is a factory reset, which erases your recordings and sounds at once and asks nothing, and its key `3` resets a calibration.

## The install, once

One page leads you through it, `builder/index.html`, with a small helper that does the file work on the unit's disk (on Windows a web page cannot reach the unit's files). The browser wants the page from a web address, not from a file: [builder/README.md](../builder/README.md) has the one command that serves it from your own computer. Then open it in Chrome or Edge, and follow it. In short:

1. **Check your unit.** Switch the unit off. Hold the **COM** key and switch it on: the bootloader's menu shows. Plug in the USB cable. On the page, press **Find my OP-1 Field**. The page reads the bootloader's version and refuses any but the supported one; then it can switch the developer menu on, which is needed once for a unit.
2. **Open the unit's drive.** On the unit press `3` (the developer menu opens), then `2` (its drive mode). The unit shows on the computer as a drive named OP-1.
3. **Choose the folder.** The page and the helper share one folder on the computer: `open-field` in your user folder. On the page, press **Choose the folder**; in the dialog, open your user folder, make a folder named `open-field` there if there is none (the dialog's **New folder**), choose it, and let the browser view and edit its files. The page writes a small mark into it (`page.json`) and keeps the folder for the rest of the install. On a later visit it offers it again: press **Use open-field again**.
4. **Run the helper.** The page shows one line to paste into PowerShell (press the Windows key, type `PowerShell`, press Enter). The line carries the page's mark. The helper first checks that mark in its folder, so that the page and the helper never work in two different folders; if the mark is not there, it stops at once and does nothing. Then it finds the unit, copies your own firmware file off (it lies in `upgrade/firmware/blackfin/bin` on the unit), takes the unit's log files off into the same folder (the unit writes logs whenever it runs, and they would change the count of the next step), takes off the pad files an earlier install left, lists the unit's disk, and waits. Leave its window open.
5. **Build, and plan the write.** This starts on the page by itself once the helper waits. The page checks that your file is the exact OS 1.7.3 file, builds the new one, and checks its own result against a known SHA-256. Then it works out where the write lands: the bootloader can only write the file when its working memory happens to stand at the right place, and where it stands follows from what is on the unit's disk (how many files and folders, and how large the files are). When the disk as it stands would not do, the page adds one extra file, the pad file, of the size that makes it do. In the same step, tick the packs you want from the listing ([below](#pack-files-in-the-same-visit)). When you have put a copy of your own file somewhere safe, tick the box and press **Put the files on the unit**.
6. **The helper finishes by itself.** It puts the new file on the unit as `os.ldr`, the pad file if there is one, and the pack files with `base.pack` last, lists the disk again and holds the listing against the page's plan row for row, and reads every file it copied back to check its SHA-256. It says `READY TO WRITE`, and so does the page.

The page and the helper write nothing to the unit but `os.ldr`, the pad file, `base.pack` and the pack files of the folder `packs`, and take nothing off it but your firmware file (a copy), the log files and the pad files of earlier installs. Nothing is sent anywhere: the page works inside your browser, on your files.

If your unit's `upgrade` folder holds another file than the original OS 1.7.3 one (it can, on a unit that once took a custom package), the page says so and asks for the original file you kept from before.

If you prefer a command line for the build alone, `node builder/build-image.mjs <your os.ldr> <the new file>` does what the page does, with the same code.

### Pack files in the same visit

**`base.pack` comes with the install.** Every unit needs it: without it no feature runs. The page has its own copy beside it, checks it against a known SHA-256, and puts it into the folder `packs` inside `open-field`; the helper copies it onto the unit with the new firmware file. If `packs` already holds a `base.pack` (an earlier install leaves one there), the page's takes its place; one with other bytes is first kept in the folder `replaced`, so nothing of yours is lost.

**Packs from the listing.** Step 5 shows the packs of the listing ([docs/catalog.md](catalog.md)) by category, each with what it does, who made it, its version and what it needs, and a tick. Tick the ones you want. The page downloads each from its author's repository, at the version the listing names, and checks it against the listing's SHA-256: a file that is not that one is refused, and its tick taken off. A pack that another one needs is ticked with it. The page puts the ticked files into `packs` when you press **Put the files on the unit**; a file of the same name and other bytes already there is first kept in `replaced`. A pack whose file is already in `packs` with the same bytes shows ticked and goes on as it is: to leave it out, take it out of `packs` and press **Look at the packs folder again**.

Other pack files you have can go onto the unit in the same visit: put them into the folder `packs` before you press **Put the files on the unit**. The page counts them in the placement, and the helper copies them into `user/pk` on the unit, `base.pack` last. You can also copy them later, in the disk mode ([adding a feature](#adding-a-feature)).

The way back to the original firmware (step 5's second choice) puts no pack on.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch on starts the unit with no packs ([the safe start](#the-safe-start)).

### Write

1. On the unit press `2` to close the drive. Switch the unit off with its switch. **From here to the write, do not start the unit normally.** If you do, it writes to its disk and the plan no longer holds: go back to step 2 and run the helper again.
2. Hold **COM** and switch on. Press `3`: the developer menu opens. Wait five seconds. Press `4`, the entry `write os to boot partition`.
3. The screen reads `looking for /os.ldr` and then **stands still for up to half a minute. Do not touch the unit and do not switch it off while it is still.**
4. When the line `firmware written ok!` shows, switch the unit off with its switch. That line by itself proves nothing: the bootloader prints it whatever happened.
5. Wait five seconds. Switch the unit on with no key held.

**The proof is the start screen.** Under the version it says what runs:

| The start screen reads | What it means |
| --- | --- |
| `1.7.3 - open-field 0.3` | The loader is in, and `base.pack` and your feature files run. `0.3` is the release of the base |
| `1.7.3 stock (no pack)` | The loader is in, and there is no pack file on the unit: the unit behaves exactly as before |
| `1.7.3 stock (4 need base)` | The loader is in, and the feature files cannot run because `base.pack` is not on the unit. Copy it into `packs`: it is [`builder/base.pack`](../builder/base.pack) in this repository |
| `1.7.3` and nothing behind it | The loader does not run: on a first install, the write did not take, and the unit runs its firmware of before |

If, in place of step 3, small text shows that names a source file and a line number (`rsi.c` and a number): the write stopped before it touched the firmware your unit runs. Switch off, switch on with no key, and the unit starts as before. One thing did change: the unit keeps a spare copy of its firmware, and the stop has erased that spare. So from now on there is no spare, which is one more reason for rule 1. Go back to step 1 of the install and run it again.

The pad file may stay on the unit; it served only this write. The next install takes it off before it counts the disk, so there is never more than one. Do not delete or add a file between the helper's `READY TO WRITE` and the write: that moves the place where the write lands.

## Adding a feature

`base.pack`, which every feature needs, is on the unit since the install. A feature is one more file.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch on starts the unit with no packs ([the safe start](#the-safe-start)).

1. Get the feature file, `8 tracks.pack` say. The feature files are not kept in this repository: the packs of the listing are offered by the install page ([above](#pack-files-in-the-same-visit)), and this project's own four features lie in the repository [open-field-packs](https://github.com/LaP0573/open-field-packs), beside this one. A pack that is not listed comes from wherever its author shares it. You can also build this project's own from this repository with one command ([build.md](build.md)).
2. Put the unit into its disk mode, as you do to copy samples. It shows a folder `packs`.
3. Copy the file into `packs`.
4. Leave the disk mode and switch the unit off and on.

The start screen reads `1.7.3 - open-field 0.3`. It does not name the features: each shows itself where it is used ([what each feature adds](#what-each-feature-adds-in-short)).

You can also copy the files in the bootloader's drive mode (hold COM, switch on, `3`, `2`); the folder is then `user/pk`.

A file may have any name that ends in `.pack`. If a file cannot run, the start screen counts it, `1.7.3 - open-field 0.3 (1 left out)`, and everything else runs.

**A new `base.pack`** is copied over the old one in the same way, with no install; the next run of the install page also puts the page's own on. The number behind `open-field` on the start screen is the release of the base that runs.

## Taking a feature away

Delete its file from `packs` in the disk mode, and switch the unit off and on. Keep `base.pack` while any feature file is there. With no pack file left, the unit starts as the original firmware and the start screen reads `1.7.3 stock (no pack)`.

## What each feature adds, in short

- **8 tracks.** Hold the tape key and press track key 1 to 4 to select tracks 5 to 8. Hold the mixer key for the mixer's page of tracks 5 to 8. Everything else works on eight tracks as it did on four, undo included. In the disk mode each tape's folder shows eight track files.
- **sidechain.** Shift + the mixer key opens its page. A track can be turned down each time another track, or the beat, hits. The four knobs set the source, the amount, the hold and the release.
- **fx train.** On a synth or drum sound, T3 opens the train: four wagons, each can carry an effect. Turn a knob to choose a wagon's effect, press it briefly to open that effect's page, hold it to switch the wagon off or on, shift + turn to move it. Shift + T3: tap for the list of saved trains, hold to save one.
- **engines.** Hosts the synth engines that other packs bring: with an engine's pack on the unit, its name stands in the engine list beside the stock ones, with its own faceplate, and its presets appear from the second start. The example `drone` is one.
- **lfo.** On a synth sound, T4 opens four LFOs. The knobs work as on the train. Shift + T4: tap for the list of saved sets, hold to save one.

Saved trains and LFO sets are files. The disk mode shows them in the folders `fx` and `lfo`, so you can keep copies on the computer or pass your own on.

A pack may have folders of its own, which the disk mode shows beside `packs`, `fx` and `lfo`: the example pack `list` has one, `list`. On the unit's disk they lie in `user/pf`. A pack's folder stays there when the pack is deleted; delete it in the bootloader's drive mode if you no longer want it.

## Your tapes

A tape recorded with `8 tracks.pack` is still a tape the original firmware reads. Tracks 1 to 4 are stored exactly as the original firmware stores them. Tracks 5 to 8 are stored beside them in files of our own, which the original firmware leaves alone.

One thing to know: if you record on such a tape while the unit runs **without** `8 tracks.pack`, the original firmware may use tape that tracks 5 to 8 were using. When the pack is back, those stretches of tracks 5 to 8 play silence. Tracks 1 to 4 are never affected.

## The safe start

Hold the **help** key while you switch the unit on, and keep it held until the first screen shows. The unit then starts as the original firmware for this one start: no feature file is read. The start screen reads `1.7.3 stock (help key)`.

Use it when a feature file misbehaves. The disk mode still shows the folder `packs`, so you can delete or replace the file. The next start with no key held reads the feature files again.

## The way back to the original firmware

There are three ways, from the lightest to the most complete.

1. **For one start**: the safe start above.
2. **For as long as you like, with no install**: delete every file in `packs`. The unit then starts as the original firmware every time. Only the small loader is still in the firmware file, and it does nothing but look for pack files and show the folder `packs`.
3. **Completely**: write the firmware file you kept (rule 1) back onto the unit. Go through the install page as for the install; in its step 5, choose **my own original file: the way back to the original firmware**. Nothing is built and no pack file is copied; the helper puts your own file on the unit as `os.ldr`, and the write is the same keys. After it the unit is as it was before you began.

The third way has the same risk as the install, and it has not yet been tried on a unit that no longer starts. That is why rule 1 and a charged unit matter.

## If something is not as described

- **Find my OP-1 Field lists the unit but gets no reply.** Another tab or program is holding the unit's MIDI port, often a tab that used the OP-1 while it ran normally: a browser shares one connection between its tabs. Close that tab or program, or close the browser and open it again; keep the unit on the bootloader's menu, and press Find again.
- **The page refuses your unit or your file.** Then your OS or bootloader version is not the supported one, or the file is not the original OS 1.7.3 file. Do not try to get around the refusal.
- **The write stops with small text.** See "Write" above. It is the common case and harms nothing.
- **The unit does not start after a write.** Hold COM and switch on. The bootloader is a separate program and should still show its menu; from there, write the file you kept, by the way back above. This has not been rehearsed on a unit in that state.
- **The helper stops.** It says why in red, and it writes nothing to the unit after a stop. Run its line again: it starts over, from the disk as it finds it.
- **A feature misbehaves.** Use the safe start, delete its file, start again.
- **The start screen reads `1.7.3 stock (4 need base)`.** `base.pack` is not on the unit. Copy it into `packs` (it is `builder/base.pack` in this repository) and start again.
- **The start screen says `(1 left out)`.** A pack file cannot run: a file damaged while it was copied, or one made for another base. The others run. The unit's log says which file and why ([methods](methods.md), "The log").
