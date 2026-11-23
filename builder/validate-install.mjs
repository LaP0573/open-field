// Checks install.mjs, the install page's plan, with no unit and no firmware file:
//
//   node builder/validate-install.mjs
//
// And plays the page's part on a folder the Windows helper works in, so that the helper can be run whole without a
// unit and without a browser (open-field-helper.ps1 -Source <a folder standing for the unit's disk> -Folder <folder>):
//
//   node builder/validate-install.mjs --play <folder> --stock <your own os.ldr> --bootloader <version>
//
// --play waits for the helper to list the disk, builds the new firmware file with the same code as the page
// (patch.mjs, the manifest loader.json and the loader's file beside this file), makes and writes the plan with the same
// code as the page (install.mjs's prepare, with the page's own base.pack beside this file, held to base-pack.json),
// waits for the helper to finish, and holds its last listing against the plan with the placement model. Exit 0: the
// helper said ready and the page's check agrees. With --way-back it plays the page's second choice: the owner's own
// original file written back, nothing built and no pack file copied.
// --bootloader is required (1000.1.1 or 1000.2.0): it stands for what the page reads from the unit. The other helper
// is played the same way: open-field-helper.py --source <that folder> --folder <folder>.
import { readFileSync, writeFileSync, readdirSync, renameSync, existsSync, mkdirSync } from "node:fs";
import { createHash } from "node:crypto";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import {
  NAMES, PACK_FOLDER, makePlan, checkAfter, compareListings, normalRows, padBytes, padName, packOrder, packNameProblem,
  prepare, readHelper, readPacks, checkFolder, sha256,
} from "./install.mjs";
import { analyzeListing, fileTnodes, placement, recommendPad, mountEndFor, plainBuffer, bootloaderVersion, SEEN_RESTS } from "./placement.mjs";
import { RAW, NOTICE, readListing, rawLink, isOurs, fileName, withNeeds, neededBy, download } from "./catalog.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const IMAGE_BYTES = 2104908;      // the size of the firmware file the page builds today (loader.json's output, image N1 for OS 1.7.3; image M4's for 1.7.0 was 2096140, image L5's 2217628 until 2026-11-18)
const STOCK_BYTES = 2079016;      // the size of the original OS 1.7.3 file (OS 1.7.0's was 2070248)

// A folder of this computer, as install.mjs wants one.
export function nodeFolder(root) {
  const at = (name) => join(root, ...name.split("/"));
  const missing = (e) => e && e.code === "ENOENT";
  return {
    async readBytes(name) { try { return new Uint8Array(readFileSync(at(name))); } catch (e) { if (missing(e)) return null; throw e; } },
    async readText(name) { try { return readFileSync(at(name), "utf8"); } catch (e) { if (missing(e)) return null; throw e; } },
    async list(sub) {
      try { return readdirSync(at(sub), { withFileTypes: true }).map((d) => ({ name: d.name, kind: d.isDirectory() ? "folder" : "file" })); }
      catch (e) { if (missing(e)) return []; throw e; }
    },
    // written beside and renamed, as the browser's folder access does: a reader never sees half a file
    async writeBytes(name, bytes) { mkdirSync(dirname(at(name)), { recursive: true }); writeFileSync(at(name) + ".part", bytes); renameSync(at(name) + ".part", at(name)); },
    async writeText(name, text) { mkdirSync(dirname(at(name)), { recursive: true }); writeFileSync(at(name) + ".part", text, "utf8"); renameSync(at(name) + ".part", at(name)); },
  };
}

// A folder held in memory, which also records the order of the writes.
function memoryFolder(files = {}) {
  const store = new Map(Object.entries(files).map(([k, v]) => [k, typeof v === "string" ? new TextEncoder().encode(v) : v]));
  const writes = [];
  return {
    store, writes,
    async readBytes(name) { return store.has(name) ? store.get(name) : null; },
    async readText(name) { return store.has(name) ? new TextDecoder().decode(store.get(name)) : null; },
    async list(sub) {
      const out = new Map();
      for (const key of store.keys()) {
        if (!key.startsWith(sub + "/")) continue;
        const rest = key.slice(sub.length + 1);
        const name = rest.split("/")[0];
        out.set(name, rest.includes("/") ? "folder" : "file");
      }
      return [...out].map(([name, kind]) => ({ name, kind }));
    },
    async writeBytes(name, bytes) { store.set(name, bytes); writes.push(name); },
    async writeText(name, text) { store.set(name, new TextEncoder().encode(text)); writes.push(name); },
  };
}

// The bootloader the checks made before 1000.2.0 was known are for.
const BL = Object.freeze({ bootloader: "1000.1.1" });

let checks = 0, fails = 0;
function check(what, ok, detail = "") {
  checks++;
  if (!ok) { fails++; console.error("  FAIL " + what + (detail ? ": " + detail : "")); }
}
function throws(what, fn, pattern) {
  try { fn(); check(what, false, "no error"); }
  catch (e) { check(what, pattern.test(e.message), e.message); }
}

// A volume like a unit's: the top-level files and folders, the factory's content (some 430 files and 115 MB, as a
// unit that was just reset shows: below about 30 batches the two starts the model knows disagree, and a disk that
// thin is not a unit's), then `files` more small files and a tape.
function volume({ files = 40, tapeBytes = 30_000_000, osLdr = null, pk = false, extra = [] } = {}) {
  const rows = [
    { path: "content", dir: true }, { path: "firmware.version", size: 8, dir: false }, { path: "logs", dir: true },
    { path: "lost+found", size: 4080, dir: false }, { path: "tape01", dir: true }, { path: "upgrade", dir: true },
    { path: "upgrade/firmware", dir: true }, { path: "upgrade/firmware/blackfin", dir: true }, { path: "upgrade/firmware/blackfin/bin", dir: true },
    { path: "upgrade/firmware/blackfin/bin/os.ldr", size: STOCK_BYTES, dir: false }, { path: "user", dir: true },
  ];
  for (let i = 0; i < 430; i++) rows.push({ path: "content/factory" + i + ".aif", size: 200_000 + i * 263, dir: false });
  for (let i = 0; i < files; i++) rows.push({ path: "content/f" + i + ".aif", size: 100_000 + i * 7919, dir: false });
  for (let t = 1; t <= 4; t++) rows.push({ path: "tape01/track_" + t + ".raw", size: tapeBytes + t, dir: false });
  if (osLdr !== null) rows.push({ path: "os.ldr", size: osLdr, dir: false });
  if (pk) rows.push({ path: "user/pk", dir: true });
  return rows.concat(extra);
}

// A tiny repeatable random source, so a failing case can be found again.
function random(seed) { let s = seed >>> 0; return () => { s = (Math.imul(s, 1664525) + 1013904223) >>> 0; return s / 4294967296; }; }

async function selfChecks() {
  // listings
  const a = volume(), b = [...volume()].reverse();
  check("a listing's order means nothing", compareListings(a, b).same);
  const c = volume(); c[1] = { ...c[1], size: 9 };
  const d = compareListings(a, c);
  check("a size that differs is named on both sides", !d.same && d.missing.length === 1 && d.extra.length === 1, JSON.stringify(d));
  throws("a listing that names a path twice is refused", () => normalRows([{ path: "a", size: 1 }, { path: "a", size: 2 }]), /twice/);
  check("back slashes and end slashes are made plain", normalRows([{ path: "\\user\\pk\\", dir: true }])[0].path === "user/pk");

  // the pad file
  const pad = padBytes(1000);
  check("the pad file has the size asked for", pad.length === 1000);
  check("the pad file is not all zeros", pad.some((x) => x !== 0));
  check("the pad file repeats every 256 bytes", pad[3] === pad[259] && pad[300] === (Math.imul(300, 2654435761) & 0xff));
  check("the pad file's name is free", padName(volume()) === "open-field-pad.bin");
  check("the pad file's name steps past one that is there",
    padName(volume({ extra: [{ path: "open-field-pad.bin", size: 5, dir: false }, { path: "OPEN-FIELD-PAD-2.BIN", size: 5, dir: false }] })) === "open-field-pad-3.bin");

  // pack files
  check("base.pack goes last, the others by name", JSON.stringify(packOrder(["sidechain.pack", "base.pack", "8 tracks.pack", "lfo.pack"])) ===
    JSON.stringify(["8 tracks.pack", "lfo.pack", "sidechain.pack", "base.pack"]));
  check("a name that does not end in .pack is refused", packNameProblem("base.pak") !== null);
  check("a plain pack name passes", packNameProblem("fx train.pack") === null);

  // plans on small volumes, with and without an os.ldr already at the top
  const image = { bytes: IMAGE_BYTES, sha256: "0".repeat(64) };
  for (const osLdr of [null, 2096456, IMAGE_BYTES]) {
    for (let files = 0; files < 60; files += 25) {
      const rows = volume({ files, osLdr });
      const plan = makePlan({ ...BL, run: "r", listing: rows, image });
      const top = plan.expected.filter((r) => r.path === "os.ldr");
      check("the plan has one os.ldr at the top, of the image's size", top.length === 1 && top[0].size === IMAGE_BYTES);
      const v = analyzeListing(plan.expected, BL);
      check("every plan's listing lands the write and is off a batch edge", v.current.pass && !v.onEdge, files + " files, os.ldr " + osLdr);
      const without = analyzeListing(rows, { ...BL, imageBytes: IMAGE_BYTES });
      check("a pad file is planned exactly when the disk with the image would not land", (plan.pad !== null) === (!without.current.pass || without.onEdge));
      check("the plan says whether os.ldr is replaced", plan.image.replaces === (osLdr !== null));
      if (plan.pad) check("the pad file's row is in the plan", plan.expected.some((r) => r.path === plan.pad.name && r.size === plan.pad.bytes));
      check("the plan's listing is the disk's with what is added", plan.expected.length === rows.length + (osLdr === null ? 1 : 0) + (plan.pad ? 1 : 0));
    }
  }

  // a sweep of larger, random volumes: the plan always lands and never sits on a batch edge
  const rnd = random(20261118);
  let padded = 0;
  for (let n = 0; n < 40; n++) {
    const rows = volume({ files: Math.floor(rnd() * 1500), tapeBytes: Math.floor(rnd() * 400_000_000), osLdr: rnd() < 0.5 ? null : 2096456 });
    const plan = makePlan({ ...BL, run: "r", listing: rows, image });
    const v = analyzeListing(plan.expected, BL);
    if (plan.pad) padded++;
    check("a random volume's plan lands", v.current.pass && !v.onEdge, "case " + n);
    const c2 = checkAfter(plan, plan.expected);
    check("a listing that is the plan's passes the check after", c2.ok, c2.lines.join("; "));
  }
  check("some random volumes need a pad file and some do not", padded > 3 && padded < 37, padded + " of 40");

  // pack files in the plan
  const packs = [{ name: "base.pack", bytes: 93596, sha256: "b".repeat(64) }, { name: "8 tracks.pack", bytes: 99476, sha256: "8".repeat(64) }];
  const p1 = makePlan({ ...BL, run: "r", listing: volume(), image, packs });
  check("the pack folder is made when it is not there", p1.makeFolder === PACK_FOLDER && p1.expected.some((r) => r.path === PACK_FOLDER && r.dir));
  check("the packs are copied base.pack last", p1.packs.map((p) => p.name).join("|") === "8 tracks.pack|base.pack");
  check("each pack's row is in the plan", p1.packs.every((p) => p.expected === undefined && p1.expected.some((r) => r.path === "user/pk/" + p.name && r.size === p.bytes)));
  const p2 = makePlan({ ...BL, run: "r", listing: volume({ pk: true, extra: [{ path: "user/pk/base.pack", size: 87452, dir: false }] }), image, packs });
  check("the pack folder is not made again", p2.makeFolder === null);
  check("a pack that is there is replaced, its row the new size", p2.packs.find((p) => p.name === "base.pack").replaces &&
    p2.expected.filter((r) => r.path === "user/pk/base.pack").length === 1 && p2.expected.find((r) => r.path === "user/pk/base.pack").size === 93596);
  check("the packs are counted in the placement", analyzeListing(p1.expected, BL).counts.files === volume().filter((r) => !r.dir && r.path !== "lost+found").length + 1 + (p1.pad ? 1 : 0) + 2);

  // refusals
  throws("a folder named os.ldr is refused", () => makePlan({ ...BL, run: "r", listing: volume({ extra: [{ path: "os.ldr", dir: true }] }), image }), /folder named os\.ldr/);
  throws("packs with no user folder are refused", () => makePlan({ ...BL, run: "r", listing: volume().filter((r) => r.path !== "user"), image, packs }), /no folder user/);
  throws("a pack whose name does not end in .pack is refused", () => makePlan({ ...BL, run: "r", listing: volume(), image, packs: [{ name: "x.bin", bytes: 1, sha256: "" }] }), /\.pack/);
  throws("a plan needs the helper's run", () => makePlan({ ...BL, listing: volume(), image }), /run/);

  // the check after: a missing, an added, a resized row
  const plan = makePlan({ ...BL, run: "r", listing: volume(), image });
  const bad = plan.expected.filter((r) => r.path !== "os.ldr");
  const c3 = checkAfter(plan, bad);
  check("a missing os.ldr fails the check after, and is named", !c3.ok && c3.lines.some((l) => l.includes("planned and not on the unit: F os.ldr")), c3.lines.join("; "));
  const more = [...plan.expected, { path: "stray.bin", size: 5, dir: false }];
  check("a file the plan does not name fails the check after", !checkAfter(plan, more).ok);

  // the page's part on a folder held in memory, with a stand-in for the build (no firmware here)
  const listing = volume({ files: 13 });
  const listingText = JSON.stringify(listing);
  const stock = new Uint8Array([1, 2, 3, 4]);
  const folder = memoryFolder({
    [NAMES.helper]: JSON.stringify({ helper: 1, run: "2026-11-18_20-00-00", state: "listed", stock: { file: NAMES.stock } }),
    [NAMES.before]: listingText,
    [NAMES.stock]: stock,
    [NAMES.packs + "/base.pack"]: new Uint8Array(93596),
    [NAMES.packs + "/notes.txt"]: "not a pack",
  });
  const fakeImage = new Uint8Array(IMAGE_BYTES).fill(7);
  let given = null;
  const { plan: made } = await prepare(folder, { ...BL, build: async (s) => { given = s; return { image: fakeImage, report: { loader: { version: "test" } } }; } });
  check("the build is given the owner's file from the folder", given && given.length === 4);
  check("the plan is for the helper's run", made.run === "2026-11-18_20-00-00");
  check("the plan names the listing it was made from by its SHA-256", made.before.sha256 === await sha256(new TextEncoder().encode(listingText)));
  check("the new file is written under its own name", folder.store.get(NAMES.image)?.length === IMAGE_BYTES);
  check("the plan is the last file written", folder.writes[folder.writes.length - 1] === NAMES.plan, folder.writes.join(", "));
  check("a pad file is written exactly when the plan has one", (made.pad === null) === !folder.writes.some((w) => w.startsWith("open-field-pad")));
  if (made.pad) check("the pad file's SHA-256 is in the plan", made.pad.sha256 === await sha256(folder.store.get(made.pad.file)));
  check("only .pack files of the packs folder go in", made.packs.length === 1 && made.packs[0].name === "base.pack" && made.packs[0].file === "packs/base.pack");
  // the way back: the owner's original file is what is written, from the folder, with no build and no pack file
  const back = memoryFolder({
    [NAMES.helper]: JSON.stringify({ helper: 1, run: "back-run", state: "listed", stock: { file: NAMES.stock } }),
    [NAMES.before]: listingText,
    [NAMES.stock]: new Uint8Array(STOCK_BYTES).fill(5),
    [NAMES.packs + "/base.pack"]: new Uint8Array(93596),
  });
  let built = false;
  const { plan: wayBack } = await prepare(back, { ...BL, build: async () => { built = true; return { image: fakeImage }; }, wayBack: true });
  check("the way back builds nothing", !built);
  check("the way back writes the owner's own file as os.ldr", wayBack.wayBack === true && wayBack.image.file === NAMES.stock && wayBack.image.bytes === STOCK_BYTES &&
    wayBack.expected.some((r) => r.path === "os.ldr" && r.size === STOCK_BYTES));
  check("the way back writes no new firmware file and copies no pack", !back.writes.includes(NAMES.image) && wayBack.packs.length === 0);
  check("the way back's plan lands", analyzeListing(wayBack.expected, BL).current.pass);
  // base.pack, which comes with the page (2026-11-18, the owner's word: "base.pack is part of open-field"): put into
  // the folder's packs, counted in the placement, copied last; one of other bytes kept in the folder's replaced first;
  // the way back adds none
  const pageBase = new Uint8Array(107492).fill(9);
  const base = { bytes: pageBase, sha256: await sha256(pageBase) };
  const listed = (run, more = {}) => memoryFolder({
    [NAMES.helper]: JSON.stringify({ helper: 1, run, state: "listed", stock: { file: NAMES.stock } }), [NAMES.before]: listingText, [NAMES.stock]: stock, ...more });
  const stand = async () => ({ image: fakeImage, report: { loader: { version: "test" } } });
  const fresh = listed("base-1", { [NAMES.packs + "/8 tracks.pack"]: new Uint8Array(99476).fill(8) });
  const r1 = await prepare(fresh, { ...BL, build: stand, base });
  check("the page's base.pack goes into the folder's packs", r1.base.state === "added" && (await sha256(fresh.store.get(NAMES.packs + "/base.pack"))) === base.sha256);
  check("the page's base.pack is in the plan, last, with its size and SHA-256", r1.plan.packs.map((p) => p.name).join("|") === "8 tracks.pack|base.pack" &&
    r1.plan.packs[1].bytes === 107492 && r1.plan.packs[1].sha256 === base.sha256 && r1.plan.packs[1].file === NAMES.packs + "/base.pack");
  check("the page's base.pack is counted in the placement", r1.plan.expected.some((r) => r.path === "user/pk/base.pack" && r.size === 107492) &&
    analyzeListing(r1.plan.expected, BL).current.pass);
  check("base.pack is written before the plan, and the plan last", fresh.writes.includes(NAMES.packs + "/base.pack") &&
    fresh.writes.indexOf(NAMES.packs + "/base.pack") < fresh.writes.indexOf(NAMES.plan) && fresh.writes[fresh.writes.length - 1] === NAMES.plan, fresh.writes.join(", "));
  check("the plan says where its base.pack came from", r1.plan.base?.from === "the page" && r1.plan.base.in_folder === "added" && r1.plan.base.kept === null);
  const same = listed("base-2", { [NAMES.packs + "/base.pack"]: pageBase });
  const r2 = await prepare(same, { ...BL, build: stand, base });
  check("a base.pack of the same bytes is left as it is", r2.base.state === "same" && !same.writes.includes(NAMES.packs + "/base.pack") &&
    !same.writes.some((w) => w.startsWith(NAMES.replaced + "/")) && r2.plan.packs.length === 1 && r2.plan.packs[0].sha256 === base.sha256);
  const older = new Uint8Array(93596).fill(3);
  const olderSha = await sha256(older);
  const other = listed("base-3", { [NAMES.packs + "/base.pack"]: older, [NAMES.packs + "/lfo.pack"]: new Uint8Array(375422).fill(4) });
  const r3 = await prepare(other, { ...BL, build: stand, base });
  check("a base.pack of other bytes is kept in the folder's replaced first", r3.base.state === "replaced" && r3.base.kept === NAMES.replaced + "/base-" + olderSha.slice(0, 12) + ".pack" &&
    (await sha256(other.store.get(r3.base.kept))) === olderSha && other.writes.indexOf(r3.base.kept) < other.writes.indexOf(NAMES.packs + "/base.pack"), JSON.stringify(r3.base));
  check("and the page's takes its place, once and last", (await sha256(other.store.get(NAMES.packs + "/base.pack"))) === base.sha256 &&
    r3.plan.packs.map((p) => p.name).join("|") === "lfo.pack|base.pack" && r3.plan.packs[1].sha256 === base.sha256 && r3.plan.base.kept === r3.base.kept);
  const back2 = listed("base-4", { [NAMES.packs + "/base.pack"]: older });
  const r4 = await prepare(back2, { ...BL, build: stand, wayBack: true, base });
  check("the way back adds no pack, not even the page's base.pack", r4.plan.packs.length === 0 && r4.base === null && !r4.plan.base &&
    !back2.writes.some((w) => w.startsWith(NAMES.packs + "/") || w.startsWith(NAMES.replaced + "/")), back2.writes.join(", "));
  await prepare(listed("base-5"), { ...BL, build: stand, base: { bytes: pageBase, sha256: "0".repeat(64) } }).then(() => check("a base.pack that is not the file it names is refused", false),
    (e) => check("a base.pack that is not the file it names is refused", /not the file it names/.test(e.message), e.message));
  // the page's own base.pack, beside this file: the one base-pack.json names
  const own = pageOwnBase();
  check("the base.pack beside this file is the one base-pack.json names", own.problem === null, own.problem);
  if (own.problem === null) check("base-pack.json names a version and a release", Number.isInteger(own.version) && own.version >= 1 && /^[0-9A-Za-z.]{1,15}$/.test(own.release));

  // packs ticked in the listing (PLAN.md piece 3, steps 2 and 3): put into the folder's packs by the page, counted in
  // the placement, copied before base.pack; one of the same bytes left as it is, one of other bytes kept in replaced
  const lfoBytes = new Uint8Array(375422).fill(11), winkBytes = new Uint8Array(2130).fill(12);
  const lfo = { name: "lfo.pack", bytes: lfoBytes, sha256: await sha256(lfoBytes), from: RAW + "x/y/" + "0".repeat(40) + "/lfo.pack" };
  const wink = { name: "wink.pack", bytes: winkBytes, sha256: await sha256(winkBytes), from: null };
  const oldLfo = new Uint8Array(375418).fill(13), oldLfoSha = await sha256(oldLfo);
  const withTicks = listed("tick-1", { [NAMES.packs + "/lfo.pack"]: oldLfo, [NAMES.packs + "/wink.pack"]: winkBytes, [NAMES.packs + "/8 tracks.pack"]: new Uint8Array(99476).fill(8) });
  const t1 = await prepare(withTicks, { ...BL, build: stand, base, chosen: [lfo, wink] });
  check("ticked packs are in the plan with the folder's own, base.pack last", t1.plan.packs.map((p) => p.name).join("|") === "8 tracks.pack|lfo.pack|wink.pack|base.pack",
    t1.plan.packs.map((p) => p.name).join("|"));
  check("a ticked pack is counted in the placement with its own size", t1.plan.expected.some((r) => r.path === "user/pk/lfo.pack" && r.size === 375422) &&
    analyzeListing(t1.plan.expected, BL).current.pass);
  check("a ticked pack of other bytes than the folder's is put in its place, the old one kept in replaced first",
    (await sha256(withTicks.store.get(NAMES.packs + "/lfo.pack"))) === lfo.sha256 &&
    (await sha256(withTicks.store.get(NAMES.replaced + "/lfo-" + oldLfoSha.slice(0, 12) + ".pack"))) === oldLfoSha &&
    withTicks.writes.indexOf(NAMES.replaced + "/lfo-" + oldLfoSha.slice(0, 12) + ".pack") < withTicks.writes.indexOf(NAMES.packs + "/lfo.pack"), withTicks.writes.join(", "));
  check("a ticked pack of the same bytes as the folder's is left as it is", !withTicks.writes.includes(NAMES.packs + "/wink.pack") &&
    t1.chosen.find((c) => c.name === "wink.pack")?.state === "same");
  check("the ticked packs are written before base.pack, and the plan last", withTicks.writes.indexOf(NAMES.packs + "/lfo.pack") < withTicks.writes.indexOf(NAMES.packs + "/base.pack") &&
    withTicks.writes[withTicks.writes.length - 1] === NAMES.plan, withTicks.writes.join(", "));
  check("the plan says where each ticked pack came from", t1.plan.chosen?.length === 2 && t1.plan.chosen[0].from === lfo.from && t1.plan.chosen[0].in_folder === "replaced");
  const tickBack = listed("tick-2");
  const t2 = await prepare(tickBack, { ...BL, build: stand, base, chosen: [lfo], wayBack: true });
  check("the way back puts no ticked pack on", t2.plan.packs.length === 0 && !t2.plan.chosen && !tickBack.writes.some((w) => w.startsWith(NAMES.packs + "/")));
  for (const [what, chosen, words] of [
    ["a ticked pack that is not its file is refused", [{ ...lfo, sha256: "0".repeat(64) }], /not the file the listing names/],
    ["base.pack is never ticked", [{ ...lfo, name: "base.pack" }], /never ticked/],
    ["two ticked packs of one name are refused", [lfo, { ...lfo }], /two ticked packs/],
    ["a ticked pack with a name the unit cannot hold is refused", [{ ...lfo, name: "a/b.pack" }], /cannot be copied/],
  ]) await prepare(listed("tick-x"), { ...BL, build: stand, base, chosen }).then(() => check(what, false), (e) => check(what, words.test(e.message), e.message));

  // the listing as the page reads it (catalog.mjs), and the download of a ticked pack held to its entry
  const COMMIT = "0123456789abcdef0123456789abcdef01234567";
  const entry = (id, more = {}) => ({ id, author: "someone", status: "listed", categories: ["tools"], description: "A pack.", repository: "https://github.com/someone/packs",
    commit: COMMIT, path: "build/" + id + ".pack", sha256: "a".repeat(64), bytes: 100, version: 1, firmware: "1.7.3", places: 0, needs: ["base"], folders: [], ...more });
  const data = {
    about: "test", categories: { tools: "tools", examples: "examples" }, ours: ["https://github.com/open-field-test/packs"],
    packs: [
      { id: "base", author: "open-field", status: "bundled", categories: ["tools"], description: "The base.", path: "builder/base.pack" },
      entry("one"), entry("two", { needs: ["base", "one"], categories: ["examples", "tools"] }), entry("old", { firmware: "1.7.0" }),
      entry("held", { status: "reserved" }), entry("ours", { author: "open-field", repository: "https://github.com/open-field-test/packs" }),
      entry("bad/id"), entry("One"), entry("needy", { needs: ["nowhere"] }),
    ],
  };
  const seen = readListing(data, "1.7.3");
  check("the listing offers only listed packs for the unit's firmware, each once", [...seen.offered.keys()].join("|") === "one|two|ours|needy", [...seen.offered.keys()].join("|"));
  check("the bundled base is shown apart", seen.base?.id === "base");
  check("a pack for another firmware is counted, not shown", seen.otherFirmware === 1);
  check("an entry out of form and a name held twice are left out and named", seen.problems.length === 2 && seen.problems.some((p) => /bad\/id/.test(p)) && seen.problems.some((p) => /One: named twice/.test(p)), seen.problems.join("; "));
  check("each pack is shown once, under its first category", seen.categories.map((c) => c.key + ":" + c.entries.map((e) => e.id).join(",")).join(" ") === "tools:one,ours,needy examples:two");
  check("open-field's own packs are those by open-field in the listing's own repositories", isOurs(seen, seen.offered.get("ours")) && !isOurs(seen, seen.offered.get("one")) &&
    !isOurs({ ours: new Set() }, seen.offered.get("ours")) && !isOurs(seen, { ...seen.offered.get("one"), author: "open-field" }));
  check("the notice is the owner's line", NOTICE === "A pack is a program that can read and write the files on your unit: take packs only from people you trust.");
  check("a ticked pack brings what it needs, base.pack aside", [...withNeeds(seen, ["two"]).ids].sort().join("|") === "one|two" && withNeeds(seen, ["needy"]).missing.has("nowhere"));
  check("a pack another ticked one needs is named", neededBy(seen, new Set(["two", "one"]), "one").join() === "two" && neededBy(seen, new Set(["one"]), "one").length === 0);
  check("the download link is the repository's file at the entry's commit", rawLink(entry("a b", { path: "packs/a b.pack" })) === RAW + "someone/packs/" + COMMIT + "/packs/a%20b.pack" &&
    RAW === "https://raw.githubusercontent.com/");
  check("a pack's file takes its id as its name", fileName(entry("fx train")) === "fx train.pack");
  throws("a malformed listing is refused", () => readListing({ packs: {} }, "1.7.3"), /not in form/);
  const fileBytes = new Uint8Array(100).fill(21), good = entry("one", { sha256: await sha256(fileBytes) });
  const answer = (bytes, status = 200) => async (url, options) => { answer.asked = { url, options }; return { ok: status === 200, status, arrayBuffer: async () => bytes.buffer.slice(0) }; };
  const got = await download(good, answer(fileBytes));
  check("a download that is the entry's file is taken, under the pack's name", got.name === "one.pack" && got.sha256 === good.sha256 && got.from === rawLink(good));
  check("the download sends no cookie and follows no redirect", answer.asked?.options?.credentials === "omit" && answer.asked?.options?.redirect === "error");
  for (const [what, get, words] of [
    ["a download of other bytes is refused", answer(new Uint8Array(100).fill(22)), /SHA-256 differs/],
    ["a download of another size is refused", answer(new Uint8Array(99).fill(21)), /99 bytes, the listing says 100/],
    ["a download that is not answered is refused", answer(fileBytes, 404), /answered 404/],
    ["a download that fails is refused", async () => { throw new Error("offline"); }, /could not be downloaded \(offline\)/],
  ]) await download(good, get).then(() => check(what, false), (e) => check(what, words.test(e.message) && e.message.startsWith("one: "), e.message));

  // the bootloader: every verdict names one the model knows, and none is assumed
  check("a version with +0 is the model's", bootloaderVersion("1000.2.0+0") === "1000.2.0" && bootloaderVersion("1000.1.1+0") === "1000.1.1" &&
    bootloaderVersion("1000.2.0") === "1000.2.0" && bootloaderVersion("1000.1.1") === "1000.1.1");
  const few = analyzeListing(volume(), BL).counts;
  for (const [what, given] of [["no bootloader", undefined], ["a null bootloader", null], ["an unknown bootloader", "1000.3.0"], ["a bootloader with another build number", "1000.2.0+1"],
    ["an empty bootloader", ""], ["a name every object has", "constructor"]]) {
    const options = given === undefined ? {} : { bootloader: given };
    throws("the placement refuses " + what, () => placement(few, options), /knows the bootloaders/);
    throws("the pad search refuses " + what, () => recommendPad(few, options), /knows the bootloaders/);
    throws("the mount's end refuses " + what, () => mountEndFor(10, 3, options), /knows the bootloaders/);
    throws("the listing's verdict refuses " + what, () => analyzeListing(volume(), options), /knows the bootloaders/);
    throws("a plan refuses " + what, () => makePlan({ run: "r", listing: volume(), image, bootloader: given }), /knows the bootloaders/);
    const untouched = listed("bl-x");
    await prepare(untouched, { build: stand, base, bootloader: given }).then(() => check("prepare refuses " + what, false),
      (e) => check("prepare refuses " + what + ", and writes nothing", /knows the bootloaders/.test(e.message) && untouched.writes.length === 0, e.message));
  }
  check("a plan for 1000.1.1 records it", r1.plan.bootloader === "1000.1.1");
  const on2 = listed("bl-2");
  const { plan: plan2 } = await prepare(on2, { build: stand, base, bootloader: "1000.2.0+0" });
  check("the plan records the bootloader, in the model's form", plan2.bootloader === "1000.2.0" && JSON.parse(new TextDecoder().decode(on2.store.get(NAMES.plan))).bootloader === "1000.2.0");
  check("a plan for 1000.2.0 lands by its profile", analyzeListing(plan2.expected, { bootloader: "1000.2.0" }).current.pass && checkAfter(plan2, plan2.expected).ok);
  // the check after uses the plan's bootloader: judged for the other one, a listing that lands stands 8 modulo 32 off
  const as1 = checkAfter({ ...plan2, bootloader: "1000.1.1" }, plan2.expected);
  check("the check after uses the plan's bootloader", !as1.ok && as1.analysis.current.perRest.every((r) => r.mod === 24), JSON.stringify(as1.analysis?.current.perRest));
  const as2 = checkAfter({ ...r1.plan, bootloader: "1000.2.0" }, r1.plan.expected);
  check("and the other way round", !as2.ok && as2.analysis.current.perRest.every((r) => r.mod === 8), JSON.stringify(as2.analysis?.current.perRest));
  const { bootloader: _gone, ...noBl } = plan2;
  const c4 = checkAfter(noBl, plan2.expected);
  check("a plan that names no bootloader is not checked, with a line that says so", !c4.ok && c4.lines.length === 1 && /cannot be checked.*none/.test(c4.lines[0]), c4.lines.join("; "));
  check("a plan that names an unknown bootloader is not checked", !checkAfter({ ...plan2, bootloader: "1000.3.0" }, plan2.expected).ok);
  // the folder's check, with the unit's bootloader and without it
  await on2.writeText(NAMES.after, JSON.stringify(plan2.expected));
  check("the folder's check with no bootloader given uses the plan's", (await checkFolder(on2)).ok);
  check("the folder's check passes for the plan's bootloader, with or without +0",
    (await checkFolder(on2, { bootloader: "1000.2.0" })).ok && (await checkFolder(on2, { bootloader: "1000.2.0+0" })).ok);
  const c5 = await checkFolder(on2, { bootloader: "1000.1.1+0" });
  check("the folder's check refuses another bootloader, with a line that says so", !c5.ok && c5.lines.length === 1 && /1000\.2\.0.*1000\.1\.1/.test(c5.lines[0]), c5.lines.join("; "));
  const c6 = await checkFolder(on2, { bootloader: "1000.3.0" });
  check("the folder's check refuses a bootloader the model does not know", !c6.ok && /knows the bootloaders/.test(c6.lines[0]), c6.lines.join("; "));
  await on2.writeText(NAMES.plan, JSON.stringify(noBl));
  check("the folder's check refuses a plan with no bootloader", !(await checkFolder(on2)).ok && !(await checkFolder(on2, { bootloader: "1000.2.0" })).ok);
  // The one write reported on 1000.2.0 (2026-11-20, it landed): its disk, with os.ldr and base.pack as the plan put
  // them, counted 541 records and 2547 index nodes before the pad. Only these counts are kept here.
  const landed = { records: 541, nodes: 2547, recordBatches: 6, nodeBatches: 26 };
  const landedPad = recommendPad(landed, { bootloader: "1000.2.0" });
  check("the counts of the write reported on 1000.2.0 get its pad, 14,400,000 bytes", landedPad?.bytes === 14_400_000, String(landedPad?.bytes));
  check("on 1000.1.1 that disk lands with no pad", placement(landed, BL).pass && !placement(landed, { bootloader: "1000.2.0" }).pass);
  // the two profiles differ in the heap's base alone, so a buffer of 1000.2.0's stands 0x60c8 above 1000.1.1's
  let cases = 0, off = 0;
  for (let nodeBatches = 0; nodeBatches <= 400; nodeBatches += 7) {
    for (let recordBatches = 1; recordBatches <= 60; recordBatches += 3) {
      for (const rest of SEEN_RESTS) {
        const end1 = mountEndFor(nodeBatches, recordBatches, { ...BL, rest });
        const end2 = mountEndFor(nodeBatches, recordBatches, { bootloader: "1000.2.0", rest });
        for (const withComputer of [true, false]) {
          cases++;
          if (plainBuffer(end2, withComputer) - plainBuffer(end1, withComputer) !== 0x60c8) off++;
        }
      }
    }
  }
  check("on 1000.2.0 every buffer stands 0x60c8 above 1000.1.1's", cases === 58 * 20 * 2 * 2 && off === 0, off + " of " + cases + " cases differ");

  // what a Mac's Finder leaves in the packs folder is no pack
  const mac = listed("mac-1", { [NAMES.packs + "/._lfo.pack"]: new Uint8Array(4096), [NAMES.packs + "/._base.pack"]: new Uint8Array(4096),
    [NAMES.packs + "/.hidden.PACK"]: new Uint8Array(10), [NAMES.packs + "/.DS_Store"]: new Uint8Array(6148), [NAMES.packs + "/lfo.pack"]: oldLfo });
  check("a file whose name begins with ._ is no pack, and any other dot's is, as on Windows", (await readPacks(mac)).map((p) => p.name).join("|") === ".hidden.PACK|lfo.pack");
  const m1 = await prepare(mac, { ...BL, build: stand, base, chosen: [lfo] });
  check("no ._ file gets into the plan or takes a pack's place", m1.plan.packs.map((p) => p.name).join("|") === ".hidden.PACK|lfo.pack|base.pack" && m1.base.state === "added" &&
    m1.chosen[0].found === "lfo.pack" && !m1.plan.expected.some((r) => r.path.startsWith("user/pk/._")) && !mac.writes.some((w) => /(^|\/)\._/.test(w)), mac.writes.join(", "));

  const helperGone = memoryFolder({});
  await prepare(helperGone, { ...BL, build: async () => ({ image: fakeImage }) }).then(() => check("no helper, no plan", false), (e) => check("no helper, no plan", /written nothing/.test(e.message), e.message));
  const notWaiting = memoryFolder({ [NAMES.helper]: JSON.stringify({ run: "x", state: "ready" }) });
  await prepare(notWaiting, { ...BL, build: async () => ({ image: fakeImage }) }).then(() => check("a helper that does not wait gets no plan", false),
    (e) => check("a helper that does not wait gets no plan", /not waiting/.test(e.message), e.message));
}

// The page's own base.pack, as the page loads it: base-pack.json and the file it names, beside this file.
// {bytes, sha256, version, release, problem}: problem is null when the file is the one named.
function pageOwnBase() {
  const nodeHash = (bytes) => createHash("sha256").update(bytes).digest("hex");
  try {
    const named = JSON.parse(readFileSync(join(here, "base-pack.json"), "utf8"));
    if (named.file !== "base.pack") return { problem: "base-pack.json names " + named.file };
    const bytes = new Uint8Array(readFileSync(join(here, named.file)));
    if (bytes.length !== named.bytes || nodeHash(bytes) !== named.sha256) return { problem: "base.pack is not the file base-pack.json names" };
    return { bytes, sha256: named.sha256, version: named.version, release: named.release, problem: null };
  } catch (e) { return { problem: e.message }; }
}

// ---- --play: the page's part on a real folder ----

const pause = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

async function play(root, { stockPath, manifestName = "loader.json", timeout = 900, wayBack = false, bootloader }) {
  try { bootloaderVersion(bootloader); }                 // asked first: nothing is waited for without it
  catch (e) { console.log("play: no plan: " + e.message); return 1; }
  const { buildImage, ours } = await import("./patch.mjs");
  const manifest = JSON.parse(readFileSync(join(here, manifestName), "utf8"));
  const loader = new Uint8Array(readFileSync(join(here, ours(manifest).file)));
  const folder = nodeFolder(root);
  const until = Date.now() + timeout * 1000;
  let helper = null;
  while (!existsSync(root)) {
    if (Date.now() > until) { console.log("play: the folder " + root + " never appeared"); return 1; }
    await pause(500);
  }
  for (;;) {
    helper = await readHelper(folder).catch(() => null);
    if (helper && (helper.state === "listed" || helper.state === "stopped")) break;
    if (Date.now() > until) { console.log("play: the helper did not list the disk in time"); return 1; }
    await pause(500);
  }
  if (helper.state === "stopped") { console.log("play: the helper stopped before the plan: " + helper.message); return 1; }
  // As the page does: the unit's own file when it is the original one, else the file the owner kept (--stock).
  const original = helper.stock && helper.stock.sha256 === manifest.stock.sha256;
  if (!original) console.log("play: the unit's firmware file is " + (helper.stock ? "not the original OS 1.7.3 file" : "not there") + (stockPath ? "; the kept file is used" : ""));
  const stock = !original && stockPath ? new Uint8Array(readFileSync(stockPath)) : null;
  const base = pageOwnBase();        // as the page does: its own base.pack, beside it, onto every unit it installs
  if (base.problem !== null) { console.log("play: no plan: " + base.problem); return 1; }
  let plan, step;
  try {
    ({ plan, base: step } = await prepare(folder, { build: (s) => buildImage(s, manifest, loader), stock, wayBack, base, bootloader }));
  } catch (e) { console.log("play: no plan: " + e.message); return 1; }
  console.log("play: plan written for run " + plan.run + ": os.ldr " + plan.image.bytes + " bytes" + (plan.pad ? ", pad " + plan.pad.name + " " + plan.pad.bytes + " bytes" : ", no pad")
    + ", " + plan.packs.length + " pack(s); " + plan.counts.records + " records, " + plan.counts.nodes + " index nodes, " + plan.counts.recordBatches + " and " + plan.counts.nodeBatches + " batches");
  if (step) console.log("play: the page's base.pack " + base.version + " (release " + base.release + "): " + step.state + (step.kept ? ", the one there kept as " + step.kept : ""));
  for (;;) {
    helper = await readHelper(folder).catch(() => null);
    if (helper && helper.run === plan.run && (helper.state === "ready" || helper.state === "stopped")) break;
    if (Date.now() > until) { console.log("play: the helper did not finish in time"); return 1; }
    await pause(500);
  }
  if (helper.state !== "ready") { console.log("play: the helper stopped: " + helper.message); return 1; }
  const result = await checkFolder(folder, { bootloader });
  for (const line of result.lines) console.log("play: " + line);
  console.log(result.ok ? "play: READY, the page agrees" : "play: the page does NOT agree");
  return result.ok ? 0 : 1;
}

const words = process.argv.slice(2);
if (words[0] === "--play") {
  const root = words[1];
  const opt = (name) => { const i = words.indexOf(name); return i >= 0 ? words[i + 1] : undefined; };
  if (!root || root.startsWith("--")) { console.error("usage: node validate-install.mjs --play <folder> --bootloader <version> [--stock <your own os.ldr>] [--timeout <seconds>]"); process.exitCode = 2; }
  else process.exitCode = await play(root, { stockPath: opt("--stock"), timeout: Number(opt("--timeout") || 900), wayBack: words.includes("--way-back"), bootloader: opt("--bootloader") });
} else {
  await selfChecks();
  console.log((fails ? "FAILED " : "ok ") + (checks - fails) + " of " + checks + " checks of install.mjs");
  process.exitCode = fails ? 1 : 0;
}
