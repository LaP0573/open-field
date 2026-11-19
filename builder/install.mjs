// open-field: the install's plan, made once, from the helper's listing of the unit's disk.
//
// The page and the helper (open-field-helper.ps1 on Windows, open-field-helper.py on Linux) share one folder on the computer. The helper takes the
// owner's own firmware file and the log files off the unit, lists the disk, and waits. This module, run by the page,
// builds the new firmware file, decides with placement.mjs whether the write lands or needs a pad file, and writes
// the plan: every file to put on the unit, and the whole listing the disk must show afterwards, row for row. The
// helper then does what the plan says and holds its new listing against the plan's: a comparison, with no model in
// it. The placement is decided here and nowhere else.
//
// Runs in the browser and in Node (no Node APIs in this file). The folder is given as an object with five
// functions, so the page (the browser's folder access) and a test (Node's fs) run the same code:
//   readBytes(name) -> Uint8Array or null when there is no such file; readText(name) -> string or null;
//   list(sub) -> [{name, kind: "file" | "folder"}] or [] when there is no such folder;
//   writeBytes(name, bytes); writeText(name, text).
// A name is a path inside the folder with "/" between its parts.

import { analyzeListing, recommendPad } from "./placement.mjs";

export const PLAN_VERSION = 1;
export const HELPER_VERSION = 1;

// The files of the shared folder. The owner's own firmware file and the new one have names of their own, so that
// they cannot be mixed up; only on the unit is the new one called os.ldr.
export const NAMES = Object.freeze({
  stock: "os-1.7.3-yours.ldr",      // the helper's name for the owner's own file (another if one with other bytes is there)
  image: "open-field-os.ldr",       // the new firmware file, written by the page
  helper: "helper.json",            // the helper's state, written by the helper
  before: "listing-before.json",    // the disk as the helper found it, after the log files were taken off
  after: "listing-after.json",      // the disk once the helper has put the files on
  plan: "plan.json",                // written by the page, last: the helper waits for it
  packs: "packs",                   // a sub folder for pack files; the helper copies them to user/pk. The page puts base.pack there
  replaced: "replaced",             // where the page keeps a pack file of other bytes that it found in packs, before it puts its own there
  page: "page.json",                // the page's mark, written when the folder is chosen; the helper's line names it, and the helper checks it
});

export const PACK_FOLDER = "user/pk";
export const PAD_STEM = "open-field-pad";
export const BASE_PACK = "base.pack";   // the shared pack every other relies on: it comes with the page (base-pack.json beside it)

// ---- listings ----

// One row as the helper writes it, the form of every listing of a unit's disk: {path, size, dir}. Paths with "/" and no slash at either
// end; a folder has dir true and no size.
export function normalRow(row) {
  const path = String(row?.path ?? "").replace(/\\/g, "/").replace(/^\/+|\/+$/g, "");
  if (!path) throw new Error("a listing row has no path");
  if (row.dir === true) return { path, dir: true };
  const size = Number(row.size);
  return { path, size: Number.isFinite(size) ? Math.trunc(size) : -1, dir: false };
}

export function normalRows(rows) {
  if (!Array.isArray(rows)) throw new Error("a listing is a JSON array of { path, size, dir } rows");
  const out = rows.map(normalRow);
  const seen = new Set();
  for (const row of out) {
    if (seen.has(row.path)) throw new Error("the listing names " + row.path + " twice");
    seen.add(row.path);
  }
  return out;
}

export function rowKey(row) { return row.dir ? "D " + row.path : "F " + row.path + " " + row.size; }

// The same disk? Compared as sets of rows: the order of a listing is the shell's and means nothing.
export function compareListings(want, have) {
  const a = new Set(normalRows(want).map(rowKey)), b = new Set(normalRows(have).map(rowKey));
  const missing = [...a].filter((k) => !b.has(k)).sort();
  const extra = [...b].filter((k) => !a.has(k)).sort();
  return { same: missing.length === 0 && extra.length === 0, missing, extra };
}

export function sortedRows(rows) {
  return normalRows(rows).sort((x, y) => (x.path < y.path ? -1 : x.path > y.path ? 1 : 0));
}

// ---- small helpers ----

export function hex(bytes) { return [...bytes].map((b) => b.toString(16).padStart(2, "0")).join(""); }

export async function sha256(bytes) {
  return hex(new Uint8Array(await globalThis.crypto.subtle.digest("SHA-256", bytes)));
}

// The pad file's content does not matter, only its size (the count of index nodes follows from the size alone).
// A pattern that is not all zeros, as tools/teboot_heap.py's padfile writes, so that nothing on the way stores it
// as a hole.
// Byte i is the low byte of i * 2654435761, which repeats every 256 bytes.
export function padBytes(count) {
  const buf = new Uint8Array(count);
  const period = new Uint8Array(256);
  for (let i = 0; i < 256; i++) period[i] = Math.imul(i, 2654435761) & 0xff;
  for (let at = 0; at < count; at += 256) buf.set(period.subarray(0, Math.min(256, count - at)), at);
  return buf;
}

// A name for the pad file that no file at the top level of the disk has.
export function padName(rows) {
  const top = new Set(normalRows(rows).filter((r) => !r.path.includes("/")).map((r) => r.path.toLowerCase()));
  for (let n = 1; ; n++) {
    const name = PAD_STEM + (n === 1 ? "" : "-" + n) + ".bin";
    if (!top.has(name)) return name;
  }
}

// A pack file's name as the loader reads the folder: a plain name ending in .pack.
export function packNameProblem(name) {
  if (!/\.pack$/i.test(name)) return "its name does not end in .pack";
  if (/[\\/:*?"<>|\u0000-\u001f]/.test(name)) return "its name has a character a file name on the unit cannot have";
  if (name.length > 120) return "its name is too long";
  return null;
}

// The order the helper copies pack files in: by name, base.pack last, so that a start made half-way through finds
// no base and runs as the original firmware rather than half a set.
export const isBase = (name) => name.toLowerCase() === BASE_PACK;

export function packOrder(names) {
  return [...names].sort((a, b) => (isBase(a) - isBase(b)) || (a < b ? -1 : a > b ? 1 : 0));
}

// ---- the plan ----

// listing: the helper's rows, the disk as it stands for the write. image: {bytes, sha256}. packs: [{name, bytes,
// sha256}]. pad: a pad file's {bytes} to force (a test's case), or undefined to let the model decide.
// Returns the plan the helper carries out, or throws with a sentence the page can show.
export function makePlan({ run, listing, image, packs = [], made = new Date().toISOString() }) {
  if (!run) throw new Error("the plan needs the helper's run");
  const before = normalRows(listing);
  const byPath = new Map(before.map((r) => [r.path, r]));
  const set = (path, size) => { byPath.set(path, { path, size, dir: false }); };

  // The new firmware file at the top level, as os.ldr (in the place of one that is there).
  const top = byPath.get("os.ldr");
  if (top && top.dir) throw new Error("the unit's disk has a folder named os.ldr at its top level");
  set("os.ldr", image.bytes);

  // The pack files into user/pk, the folder made when it is not there.
  let makeFolder = null;
  const packSteps = [];
  if (packs.length) {
    const user = byPath.get("user");
    if (!user || !user.dir) throw new Error("the unit's disk has no folder user; a pack file has nowhere to go");
    const pk = byPath.get(PACK_FOLDER);
    if (pk && !pk.dir) throw new Error("the unit's disk has a file named " + PACK_FOLDER + " where the pack folder belongs");
    if (!pk) { byPath.set(PACK_FOLDER, { path: PACK_FOLDER, dir: true }); makeFolder = PACK_FOLDER; }
    const names = new Set();
    for (const pack of packs) {
      const problem = packNameProblem(pack.name);
      if (problem) throw new Error("the pack file " + pack.name + " cannot be copied: " + problem);
      if (names.has(pack.name.toLowerCase())) throw new Error("two pack files are named " + pack.name);
      names.add(pack.name.toLowerCase());
    }
    for (const name of packOrder(packs.map((p) => p.name))) {
      const pack = packs.find((p) => p.name === name);
      const path = PACK_FOLDER + "/" + name;
      const there = byPath.get(path);
      if (there && there.dir) throw new Error("the unit's disk has a folder named " + path);
      packSteps.push({ file: NAMES.packs + "/" + name, name, path, bytes: pack.bytes, sha256: pack.sha256, replaces: !!there });
      set(path, pack.bytes);
    }
  }

  // The placement, by the model that is checked. With the image and the packs as they will be, does the write land?
  let expected = [...byPath.values()];
  let analysis = analyzeListing(expected);
  let pad = null;
  if (!analysis.current.pass || analysis.onEdge) {
    const found = recommendPad(analysis.counts);
    if (!found) throw new Error("no pad file in range lands the write on this disk (not expected on a real unit): stop here");
    pad = { file: padName(expected), name: null, bytes: found.bytes, sha256: null, marginNodes: found.marginNodes, marginRecords: found.marginRecords };
    pad.name = pad.file;
    set(pad.name, pad.bytes);
    expected = [...byPath.values()];
    analysis = analyzeListing(expected);
    if (!analysis.current.pass || analysis.onEdge) throw new Error("the pad file the model chose does not land the write: stop here");
  }

  const c = analysis.counts;
  return {
    plan: PLAN_VERSION,
    run,
    made,
    image: { file: NAMES.image, name: "os.ldr", path: "os.ldr", bytes: image.bytes, sha256: image.sha256, replaces: !!top },
    pad,
    makeFolder,
    packs: packSteps,
    expected: sortedRows(expected),
    counts: { files: c.files, folders: c.folders, records: c.records, nodes: c.nodes, recordBatches: c.recordBatches, nodeBatches: c.nodeBatches },
    buffers: analysis.current.perRest.map((r) => ({ rest: r.rest, buffer: "0x" + r.buffer.toString(16), mod32: r.mod })),
  };
}

// The disk after the helper's work, held against the plan: the same rows, and the model's verdict on the listing as
// it is (with the exact image on it, so no image size is given). Returns {ok, lines}.
export function checkAfter(plan, afterRows) {
  const lines = [];
  const diff = compareListings(plan.expected, afterRows);
  if (diff.same) lines.push("the disk's listing is the plan's, row for row (" + plan.expected.length + " rows)");
  else {
    for (const k of diff.missing.slice(0, 20)) lines.push("planned and not on the unit: " + k);
    for (const k of diff.extra.slice(0, 20)) lines.push("on the unit and not planned: " + k);
  }
  const analysis = analyzeListing(normalRows(afterRows));
  const pass = analysis.current.pass && !analysis.onEdge;
  lines.push(pass
    ? "the placement on the listing as it is: the write lands (" + analysis.counts.nodeBatches + " and " + analysis.counts.recordBatches + " batches)"
    : "the placement on the listing as it is: the write would NOT land");
  return { ok: diff.same && pass, lines, analysis };
}

// ---- the page's part, on the shared folder ----

async function readJson(folder, name) {
  const text = await folder.readText(name);
  if (text === null) return null;
  return JSON.parse(text.replace(/^﻿/, ""));
}

// What the helper says of itself, or null when it has written nothing yet.
export async function readHelper(folder) { return readJson(folder, NAMES.helper); }

// The pack files the owner put into the folder's packs sub folder, in the helper's order.
export async function readPacks(folder) {
  const entries = (await folder.list(NAMES.packs)).filter((e) => e.kind === "file" && /\.pack$/i.test(e.name));
  const packs = [];
  for (const name of packOrder(entries.map((e) => e.name))) {
    const bytes = await folder.readBytes(NAMES.packs + "/" + name);
    packs.push({ name, bytes: bytes.length, sha256: await sha256(bytes) });
  }
  return packs;
}

// From a helper that has listed the disk and waits: the owner's file, the build, the plan; the new file, the pad
// file and the plan written into the folder, the plan last. `build(stock)` returns {image, report} or throws (the
// page passes patch.mjs's buildImage with the manifest and the loader's file). `stock` may be given by the caller, for an
// owner whose unit holds another file than the original one and who chose their own kept file.
// `wayBack`: the owner's own original file is what is written (the way back to the original firmware), from the
// folder under its own name; no build, and no pack file (the original firmware does not read them). The caller has
// checked that `stock` (or the helper's file) is the original one.
// `base`: the page's own base.pack, {bytes, sha256}, which comes with the page and goes onto every unit it installs.
// It is written into the folder's packs as base.pack, so that the plan counts it and the helper copies it last. A
// base.pack of the same bytes there already is left as it is. One of other bytes (an earlier release, or one the
// owner put there) is kept in the folder's `replaced` sub folder under its SHA-256's first twelve digits, and the
// page's takes its place. The way back adds no pack, so `base` is passed over there.
// `chosen`: the packs the owner ticked in the listing, each downloaded and held to its entry by catalog.mjs,
// [{name, bytes, sha256, from}]. Each is written into the folder's packs under its name the same way: left as it is
// when the same bytes are there, the other bytes kept in `replaced` first. The way back passes them over too.
// What a pack file the page puts into the folder's packs finds there under its name, in small letters or capitals.
export async function packStep(folder, name, digestWanted) {
  const there = (await folder.list(NAMES.packs)).filter((e) => e.kind === "file" && e.name.toLowerCase() === name.toLowerCase());
  if (!there.length) return { state: "added", found: null, kept: null };
  const bytes = await folder.readBytes(NAMES.packs + "/" + there[0].name);
  const digest = await sha256(bytes);
  if (digest === digestWanted) return { state: "same", found: there[0].name, kept: null };
  return { state: "replaced", found: there[0].name, kept: NAMES.replaced + "/" + name.replace(/\.pack$/i, "") + "-" + digest.slice(0, 12) + ".pack", bytes };
}

export async function baseStep(folder, base) { return packStep(folder, BASE_PACK, base.sha256); }

export async function prepare(folder, { build, stock = null, made, wayBack = false, base = null, chosen = [] } = {}) {
  const helper = await readHelper(folder);
  if (!helper) throw new Error("the helper has written nothing into this folder yet");
  if (helper.state !== "listed") throw new Error("the helper is not waiting for a plan (it says: " + helper.state + ")");
  const listingText = await folder.readText(NAMES.before);
  if (listingText === null) throw new Error("the helper's listing is not in the folder");
  const listing = JSON.parse(listingText.replace(/^﻿/, ""));
  let stockBytes = stock;
  if (!stockBytes) {
    if (!helper.stock?.file) throw new Error("the helper found no firmware file on the unit");
    stockBytes = await folder.readBytes(helper.stock.file);
    if (!stockBytes) throw new Error("the folder has no " + helper.stock.file);
  }
  let image, report, imageFile = NAMES.image;
  if (wayBack) {
    image = stockBytes;
    imageFile = stock ? NAMES.stock : helper.stock.file;
    const there = await folder.readBytes(imageFile);
    if (!there || (await sha256(there)) !== (await sha256(image))) await folder.writeBytes(imageFile, image);
    report = { loader: { version: "the original file" } };
  } else ({ image, report } = await build(stockBytes));
  const imageSha = await sha256(image);
  let packs = wayBack ? [] : await readPacks(folder);
  const ticked = [];
  if (!wayBack) {
    const names = new Set();
    for (const pack of chosen) {
      if (isBase(pack.name)) throw new Error("base.pack comes with the page and is never ticked");
      const problem = packNameProblem(pack.name);
      if (problem) throw new Error("the pack file " + pack.name + " cannot be copied: " + problem);
      if (names.has(pack.name.toLowerCase())) throw new Error("two ticked packs are named " + pack.name);
      names.add(pack.name.toLowerCase());
      if ((await sha256(pack.bytes)) !== pack.sha256) throw new Error("the pack " + pack.name + " is not the file the listing names");
      ticked.push({ pack, step: await packStep(folder, pack.name, pack.sha256) });
    }
    packs = packs.filter((p) => !names.has(p.name.toLowerCase())).concat(chosen.map((p) => ({ name: p.name, bytes: p.bytes.length, sha256: p.sha256 })));
  }
  let step = null;
  if (!wayBack && base) {
    if ((await sha256(base.bytes)) !== base.sha256) throw new Error("the page's base.pack is not the file it names");
    step = await baseStep(folder, base);
    packs = packs.filter((p) => !isBase(p.name)).concat([{ name: BASE_PACK, bytes: base.bytes.length, sha256: base.sha256 }]);
  }
  const plan = makePlan({ run: helper.run, listing, image: { bytes: image.length, sha256: imageSha }, packs, made });
  plan.image.file = imageFile;
  plan.wayBack = wayBack;
  plan.before = { file: NAMES.before, sha256: await sha256(new TextEncoder().encode(listingText)) };
  plan.build = { loader: report?.loader?.version ?? null, output_sha256: report?.output_sha256 ?? imageSha };
  if (step) plan.base = { from: "the page", sha256: base.sha256, in_folder: step.state, kept: step.kept };
  if (ticked.length) plan.chosen = ticked.map(({ pack, step: s }) => ({ name: pack.name, sha256: pack.sha256, from: pack.from ?? null, in_folder: s.state, kept: s.kept }));
  if (!wayBack) await folder.writeBytes(NAMES.image, image);
  if (plan.pad) {
    const bytes = padBytes(plan.pad.bytes);
    plan.pad.sha256 = await sha256(bytes);
    await folder.writeBytes(plan.pad.file, bytes);
  }
  for (const { pack, step: s } of ticked) {           // the ticked packs, then base.pack, then the plan
    if (s.state === "same") continue;
    if (s.kept) await folder.writeBytes(s.kept, s.bytes);
    await folder.writeBytes(NAMES.packs + "/" + pack.name, pack.bytes);
  }
  if (step && step.state !== "same") {
    if (step.kept) await folder.writeBytes(step.kept, step.bytes);
    await folder.writeBytes(NAMES.packs + "/" + BASE_PACK, base.bytes);
  }
  await folder.writeText(NAMES.plan, JSON.stringify(plan, null, 1));
  return {
    plan, image, report, base: step && { state: step.state, found: step.found, kept: step.kept },
    chosen: ticked.map(({ pack, step: s }) => ({ name: pack.name, state: s.state, found: s.found, kept: s.kept })),
  };
}

// Once the helper says it is ready: its listing of the disk held against the plan, by the page as well.
export async function checkFolder(folder) {
  const helper = await readHelper(folder);
  const plan = await readJson(folder, NAMES.plan);
  const after = await readJson(folder, NAMES.after);
  if (!helper || !plan || !after) return { ok: false, lines: ["the folder does not hold the helper's last listing yet"] };
  if (plan.run !== helper.run) return { ok: false, lines: ["the plan in the folder is not for the helper's last run"] };
  return checkAfter(plan, after);
}
