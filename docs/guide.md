# The owner's guide

How to put open-field on your own OP-1 Field, how to add and take away a feature, how to start the unit as the original firmware, and how to go back for good.

open-field has three parts. **The loader** is a small piece of our code that goes, once, into the firmware file of your unit: at every start it reads the pack files on the unit. **The base**, the file `base.pack`, is the shared pack every feature needs. **The packs** are the features, one file each. The install puts the loader in and `base.pack` on the unit, and nothing else; packs come after, from the install page's packs section or copied by hand.

Read the whole page before you begin. The install changes the firmware of your unit. A write that goes wrong can leave a unit unable to start, for good. Installing this will void the warranty of your device. open-field is unofficial: it is not made or supported by Teenage Engineering, and it is not affiliated with them. Whether to install it is your decision, and you do it at your own risk ([NOTICE.md](../NOTICE.md)).

## What you need

- An OP-1 Field with the original **OS 1.7.3** and bootloader **1000.1.1**, or **1000.2.0**, with one install reported so far ([below](#a-unit-that-shows-1731)). The install page checks both and refuses any other. The start screen shows `1.7.3`, or `1.7.3+1` on a unit that is on that version. That label is the unit's version record, not the firmware file's, so it does not decide: step 5 checks your firmware file by its SHA-256 and refuses any but the original OS 1.7.3 file.
- A USB cable that carries data, and one of these computers:
  - **Windows** with **Chrome or Edge**. This is the way most installs so far were made.
  - A **Mac** with **Chrome**, `python3` (it comes with Apple's command line tools) and libusb (`brew install libusb`, with Homebrew).
  - **Linux** with `python3`, `curl`, `libusb-1.0` and a Chromium browser that has folder access. The install reported from Linux was made with Brave, with `brave://flags/#file-system-access-api` switched on.

  One install from a Mac and one from Linux were reported, both on open-field 0.3 with earlier forms of the helper. The base and packs of 0.25 and the helper as it is now have not been tried there, and neither has the way back.
- A **charged** unit. A write that loses power halfway can leave nothing to start from.
- Half an hour in which you will not be interrupted.

**If your unit is on a later OS version than 1.7.3, you cannot use this yet** (`1.7.3+1` does not count as one: [below](#a-unit-that-shows-1731)). The builder takes only the OS 1.7.3 file, which such a unit does not hold. Whether such a unit can be brought back to OS 1.7.3 is not known: do not count on it.

**If your unit is on OS 1.7.3 and you want to keep open-field, do not update the unit.** An official update writes the maker's firmware over the one with our loader in it. The unit is then on the original firmware of the new version, open-field is gone from it, and you are in the case above.

### A unit on OS 1.7.0

open-field is for OS 1.7.3 only. Update a unit still on OS 1.7.0 first, with Teenage Engineering's own update, to OS 1.7.3 or `1.7.3+1`, and then install open-field from this page as on any unit. The update writes the maker's firmware: if the unit ran open-field for 1.7.0, the loader is gone after it, and the unit is on the original firmware of the new version.

**`1.7.3+1` comes with a new bootloader, 1000.2.0.** It is more locked down than 1000.1.1 and doesn't let the unit go back to an older OS version. This page takes it, but only one install on it has been reported ([below](#a-unit-that-shows-1731)). Updating to it is at your own risk. Check which version the update gives before you run it.

**Pack files left from OS 1.7.0.** A pack file is made for one firmware. The loader for 1.7.3 leaves out every pack file made for 1.7.0 and starts everything else; the start screen counts what it left out at the end of its line, `(4 left out)` for four such files. Nothing breaks, but those features do not run. Two ways to set it right:

- After the install, connect on the install page and take the packs again from its packs section: it puts their 1.7.3 files on in place of the old files of the same name. `base.pack` is always the page's own.
- Or delete the old files from the folder `packs` on the unit in the disk mode ([below](#taking-a-feature-away)).

### A unit that shows 1.7.3+1

If your unit is already on `1.7.3+1`, you can use this page. A copy of that version's `os.ldr` given to the maintainer is byte for byte the original 1.7.3 file. The contributor of this part reported the same of their own copy. Yours is checked all the same: the SHA-256 check in step 5 decides, not the label on the screen.

Expect the label to stay through the install and the way back. Wherever this guide quotes `1.7.3` on the start screen, such a unit shows `1.7.3+1` in its place: after a good install `1.7.3+1 - open-field 0.25`, after the way back `1.7.3+1`.

`1.7.3+1` comes with bootloader **1000.2.0**. The page reads the version in step 1 and plans the write for it. Know how little that rests on:

- Three of the numbers the page plans with are the contributor's reading of that bootloader. The rest of the calculation is 1000.1.1's, assumed for 1000.2.0. The contributor reported that one write with them, on one unit, landed (2026-11-20). The maintainer has not read that bootloader or tried a unit with it.
- If those numbers, or that assumption, are wrong for your unit, the write can land in the wrong place. With 1000.1.1 such a write stops, and the stop erases the unit's spare copy of its firmware, as [Write](#write) says under the install. What 1000.2.0 does with one has not been seen.
- The page's button that switches the developer menu on has no report on 1000.2.0.
- The install was reported from a Mac, on open-field 0.3 with an earlier form of the helper. The base and packs of 0.25 and the helper as it is now have not been tried on 1000.2.0.
- The way back to the original firmware has not been tried there. Windows with 1000.2.0 has not been tried at all.

If that is too little for you, wait. And on a unit already on OS 1.7.3, do not update, change or write anything to get `1.7.3+1` or 1000.2.0: open-field needs neither.

## Two rules before anything else

1. **Keep the firmware file you copy off your unit.** The helper puts it into the working folder you pick in step 3 of the install, as `os-1.7.3-yours.ldr`. Put a copy somewhere safe as well, away from the computer you work on. It is your way back.
2. **In the bootloader's menus press only the keys this guide names.** In its main menu, `3` opens the developer menu. In the developer menu you will only ever need `2` and `4`: its key `1` is a factory reset, which erases your recordings and sounds at once and asks nothing, and its key `3` resets a calibration.

## The install, once

One page leads you through it, `builder/index.html`, with a small helper that does the file work on the unit's disk (the page cannot reach the unit's files by itself). Open it at [lap0573.github.io/open-field](https://lap0573.github.io/open-field/), in your computer's browser from [What you need](#what-you-need). (To serve it from your own computer instead, [builder/README.md](../builder/README.md) has the one command: the browser wants the page from a web address, not from a file.) Switch the unit on as usual, plug it in and press **connect**. On a unit with no open-field the page says so and shows one button, **install open-field**, which opens the install steps at the top of the page. In short:

1. **Check your unit.** Switch the unit off. Hold the **COM** key and switch it on: the bootloader's menu shows. Plug in the USB cable if it is not plugged in already. On the page, press **find my OP-1**. The page reads the bootloader's version, refuses any but 1000.1.1 and 1000.2.0, and plans the write for the one it read. If you change to another OP-1 at any point, start again from step 1 with that one. If the menu does not show `3. dev menu` yet, the page's button switches the developer menu on; then switch the unit off and on again with COM held, which is needed once for a unit.
2. **Open the unit's drive.** On the unit press `3` (the developer menu opens), then `2` (its drive mode). On Windows the unit shows as a drive named OP-1. On a Mac or Linux it need not show: the helper finds it over USB.
3. **Pick a working folder.** The page and the helper share one folder on the computer. Make an empty folder anywhere you like, then press **choose the folder** on the page, choose it, and let the browser view and edit its files. The page writes a small mark into it (`page.json`) and keeps the folder for the rest of the install. On a later visit it offers it again.
4. **Run the helper.** The page shows one line, for the computer it is open on. On Windows, paste it into PowerShell (press the Windows key, type `PowerShell`, press Enter). On a Mac, paste it into Terminal (press Cmd + Space, type `Terminal`, press Return). If macOS asks, allow Terminal to reach the folder; to reach the unit, the helper may end macOS's camera service, `ptpcamerad`. On Linux, paste it into a terminal. The line carries the page's mark and the folder's name: a browser never tells a page where a folder is, so the helper looks for a folder of that name with the mark in it, in its own current folder, your user or home folder, the Desktop, Documents and Downloads, and one level below each. If it finds none, or more than one, it asks you for the folder: on Windows you pick it in a window, on a Mac or Linux you give its path in the terminal. It checks the mark in the folder before anything else, so that the page and the helper never work in two different folders; if the mark is not there, it stops at once and does nothing. Then it finds the unit, copies your own firmware file off (it lies in `upgrade/firmware/blackfin/bin` on the unit), takes the unit's log files off into the same folder (the unit writes logs whenever it runs, and they would change the count of the next step), takes off the pad files an earlier install left and the files waiting in `user/pkin`, lists the unit's disk, and waits. Leave its window open.
5. **Pick what to install.** Once the helper waits, the page checks that your file is the exact OS 1.7.3 file, builds the new one, and checks its own result against a known SHA-256. Then it works out where the write lands: the bootloader can only write the file when its working memory happens to stand at the right place, and where it stands follows from what is on the unit's disk (how many files and folders, and how large the files are). When the disk as it stands would not do, the page adds one extra file, the pad file, of the size that makes it do. The page's log, at the bottom of the page, has the numbers. Choose **the latest version of the open-field custom firmware**, or **my backup of the original firmware** for the way back ([below](#the-way-back-to-the-original-firmware)). Put a copy of `os-1.7.3-yours.ldr` from your folder somewhere safe (rule 1), and press **put the files on your OP-1**.
6. **The helper finishes by itself.** It puts the new file on the unit as `os.ldr`, the pad file if there is one, and `base.pack`, lists the disk again and holds the listing against the page's plan row for row, and reads every file it copied back to check its SHA-256. It says `READY TO WRITE`, and the page says **Ready to write**.

The page and the helper write nothing to the unit but `os.ldr`, the pad file and `base.pack`, and take nothing off it but your firmware file (a copy), the log files, the pad files of earlier installs and the files waiting in `user/pkin`. Nothing is sent anywhere: the page works inside your browser, on your files.

If your unit's `upgrade` folder holds another file than the original OS 1.7.3 one (it can, on a unit that once took a custom package), the page says so and asks for the original file you kept from before.

If you prefer a command line for the build alone, `node builder/build-image.mjs <your os.ldr> <the new file>` does what the page does, with the same code.

### `base.pack`, and no other pack

**`base.pack` comes with the install.** Every unit needs it: without it no pack runs. The page has its own copy beside it, checks it against a known SHA-256, and puts it into the folder `packs` inside your working folder; the helper copies it onto the unit with the new firmware file. If `packs` already holds a `base.pack` (an earlier install leaves one there), the page's takes its place; one with other bytes is first kept in the folder `replaced`, so nothing of yours is lost.

**Every other pack goes on after the install**, from the page's packs section or by hand ([adding a feature](#adding-a-feature)). The install puts no other pack on: a pack file you left in the folder `packs` stays there, and the page's log names it.

The way back to the original firmware (step 5's second choice) puts no pack on.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch on starts the unit with no packs ([the safe start](#the-safe-start)).

### Write

1. Restart the unit in bootloader mode: switch it off with its switch, then switch it on while holding **COM**. **From here to the write, do not start the unit normally.** If you do, it writes to its disk and the plan no longer holds: go back to step 1 of the install and run the helper again.
2. Press `3`: the developer menu opens. Press `4`, the entry `write os to boot partition`. Never `1` there: it is the factory reset.
3. The screen reads `looking for /os.ldr` and then **stands still for up to half a minute. Do not touch the unit and do not switch it off while it is still.**
4. When the line `firmware written ok!` shows, switch the unit off with its switch. That line by itself proves nothing: the bootloader prints it whatever happened.
5. Switch the unit on with no key held.

**The proof is the start screen.** Under the version it says what runs. On a unit on `1.7.3+1` each line below begins with `1.7.3+1` in place of `1.7.3`, and that is a good write all the same: `1.7.3+1 - open-field 0.25`.

| The start screen reads | What it means |
| --- | --- |
| `1.7.3 - open-field 0.25` | The loader is in, and `base.pack` and your feature files run. `0.25` is the release of the base |
| `1.7.3 stock (no pack)` | The loader is in, and there is no pack file on the unit: the unit behaves exactly as before |
| `1.7.3 stock (4 need base)` | The loader is in, and the feature files cannot run because `base.pack` is not on the unit. Copy it into `packs`: it is [`builder/base.pack`](../builder/base.pack) in this repository. On a Mac, run the install again instead: it puts it on |
| `1.7.3` or `1.7.3+1`, and nothing behind it | The loader does not run. After the way back to the original firmware that is what you want. On a first install, the write did not take, and the unit runs its firmware of before |

On bootloader 1000.1.1, small text in place of step 3 that names a source file and a line number (`rsi.c` and a number) means the write stopped before it touched the firmware your unit runs. On 1000.2.0 this has not been seen, so none of this paragraph is promised there. Switch off, switch on with no key, and the unit starts as before. One thing did change: the unit keeps a spare copy of its firmware, and the stop has erased that spare. So from now on there is no spare, which is one more reason for rule 1. Go back to step 1 of the install and run it again.

The pad file may stay on the unit; it served only this write. The next install takes it off before it counts the disk, so there is never more than one. Do not delete or add a file between the helper's `READY TO WRITE` and the write: that moves the place where the write lands.

## Adding a feature

`base.pack`, which every feature needs, is on the unit since the install. A feature is one more file.

**A pack is a program that can read and write the files on your unit: take packs only from people you trust.** Holding the help key while you switch on starts the unit with no packs ([the safe start](#the-safe-start)).

The easiest way is the install page's packs section: switch the unit on as usual, plug it in, press **connect**, press **add** on the packs you want and send the change. The unit asks on its own screen: T1 confirms, T2 cancels. A pack that isn't in the list goes on the same way: press the **+** at the end of the list, pick its file on your computer, and send. The page checks that the file is a pack made for your OS version, and puts it on under the pack's own name, in place of an older file of the same pack. On a Mac use that section: no drive of the unit shows in Finder to copy to. By hand, on a computer that shows the unit's drive:

1. Get the pack file. Packs are not kept in this repository: the packs of the listing are offered by the install page's packs section, and each one's author publishes it. A pack that is not listed comes from wherever its author shares it.
2. Put the unit into its disk mode, as you do to copy samples. It shows a folder `packs`.
3. Copy the file into `packs`.
4. Leave the disk mode and switch the unit off and on.

The start screen reads `1.7.3 - open-field 0.25` (`1.7.3+1 - open-field 0.25` on a unit on `1.7.3+1`). It does not name the packs: each shows itself where it is used, as its author's manual says.

You can also copy the files in the bootloader's drive mode (hold COM, switch on, `3`, `2`); the folder is then `user/pk`.

A file may have any name that ends in `.pack`. If a file cannot run, the start screen counts it at the end of its line, `(1 left out)`, and everything else runs.

**A new `base.pack`** is copied over the old one in the same way, with no install; the next run of the install page also puts the page's own on. The number behind `open-field` on the start screen is the release of the base that runs.

## Taking a feature away

Delete its file from `packs` in the disk mode, and switch the unit off and on. On a Mac, take it off in the install page's packs section. Keep `base.pack` while any feature file is there. With no pack file left, the unit starts as the original firmware and the start screen reads `1.7.3 stock (no pack)`, or `1.7.3+1 stock (no pack)`.

## A pack's folders

A pack may have folders of its own, which the disk mode shows beside `packs`: the example pack `list` has one, `list`. On the unit's disk they lie in `user/pf`. A pack's folder stays there when the pack is deleted; delete it in the bootloader's drive mode if you no longer want it.

## A pack's settings

From open-field 0.14 a pack can add settings of its own to the unit's settings (hold shift and press COM, then T1). They look and turn like the unit's own.

- In **KEYBOARD**, **SYSTEM** and **MIDI**, a pack's settings come after the unit's own lines. Pick a line with the **ochre** knob and change it with the **red** knob, as you do for the unit's own. There's room for a few lines only; what doesn't fit goes under PACKS.
- **PACKS** is a row of its own, after the unit's six. On the right it lists the packs that have settings there or a page of their own. Pick one with the ochre knob and press the ochre knob: you get that pack's settings in the same place (ochre picks, red changes), or the pack's own page. **T1** takes you back to the list of packs. On the list, T1 leaves the settings as always.

The base keeps these settings in a file of its own on the unit, `user/pf/.options`, never in the unit's own settings, and they come back at the next start. **A change made in the last second or two before you switch off is lost**, so give it a moment. A pack you delete keeps its line in that file, and its settings are still there when you put it back.

## The safe start

Hold the **help** key while you switch the unit on, and keep it held until the first screen shows. The unit then starts as the original firmware for this one start: no feature file is read. The start screen reads `1.7.3 stock (help key)`, or `1.7.3+1 stock (help key)`.

Use it when a feature file misbehaves. The disk mode still shows the folder `packs`, so you can delete or replace the file. The next start with no key held reads the feature files again. On a Mac, Finder shows no disk: with a safe start the page's packs section cannot reach the unit either, so deleting takes an MTP file program (OpenMTP, for instance; not tried with open-field), or a Windows or Linux computer.

## The way back to the original firmware

There are three ways, from the lightest to the most complete.

1. **For one start**: the safe start above.
2. **For as long as you like, with no install**: delete every file in `packs`. The unit then starts as the original firmware every time. Only the small loader is still in the firmware file, and it does nothing but look for pack files and show the folder `packs`. A Mac needs an MTP file program or another computer for this: the page never takes `base.pack` off.
3. **Completely**: write the firmware file you kept (rule 1) back onto the unit. On the install page, press **connect**, then **reinstall or go back to the original firmware** under the table, and go through the steps as for the install; in step 5, choose **my backup of the original firmware**. Nothing is built and no pack file is copied; the helper puts your own file on the unit as `os.ldr`, and the write is the same keys. After it the unit is as it was before you began: the start screen reads `1.7.3` with nothing behind it, or `1.7.3+1` on a unit that is on that version.

The third way has the same risk as the install, and it has not yet been tried on a unit that no longer starts, with bootloader 1000.2.0, or from a Mac or Linux computer. That is why rule 1 and a charged unit matter.

## If something is not as described

- **find my OP-1 lists the unit but gets no reply.** Another tab or program is holding the unit's MIDI port, often a tab that used the OP-1 while it ran normally: a browser shares one connection between its tabs. (The install page itself gives the unit's ports back before it looks.) Close that tab or program, or close the browser and open it again; keep the unit on the bootloader's menu, and press find my OP-1 again.
- **The page refuses your unit or your file.** Then your OS or bootloader version is not a supported one, or the file is not the original OS 1.7.3 file. Do not try to get around the refusal.
- **The write stops with small text.** See "Write" above. On bootloader 1000.1.1 it is the common case: the firmware your unit runs is untouched, and the spare copy is erased. On 1000.2.0 it has not been seen.
- **The unit does not start after a write.** Hold COM and switch on. The bootloader is a separate program and should still show its menu; from there, write the file you kept, by the way back above. This has not been rehearsed on a unit in that state.
- **On a Mac or Linux the helper cannot open the unit.** Usually another program holds it. On a Mac, quit Image Capture, Photos, OpenMTP and Android File Transfer. On Linux, unmount OP-1 in the file manager and open nothing on it; if the computer itself does not let you open the unit, the helper names the udev rule to add. Then run its line again.
- **The helper stops.** It says why in red, and it writes nothing to the unit after a stop. Run the line the page shows in step 4 (each new step 1 makes a new line): it starts over, from the disk as it finds it.
- **A feature misbehaves.** Use the safe start, delete its file, start again.
- **The start screen says `stock (4 need base)`.** `base.pack` is not on the unit. Copy it into `packs` (it is `builder/base.pack` in this repository) and start again. On a Mac, run the install again instead.
- **The start screen says `(1 left out)`.** A pack file cannot run: a file damaged while it was copied, or one made for another base. The others run. The unit's log says which file and why ([methods](methods.md), "The log"). From release 0.16 the disk mode shows the logs in a folder `logs`: copy the newest. Easier still, with the install page connected: press **save logs** at the top of section 1 (it shows up when your unit can do it), and you get the logs of its last three starts in one .zip, on a Mac too. To report a problem, quote the lines that matter; never attach the log files.
