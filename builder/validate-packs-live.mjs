// Checks packs-live.mjs, the logic of the install page's live list of packs, with no unit:
//
//   node open-field/validate-packs-live.mjs
//
// The unit's files are given as the door gives them (name, size, SHA-256, whether the loader takes the name), with the
// sizes and SHA-256 of real pack files: the page's own base.pack beside this file, read from its header, and our
// packs as the listing names them (the set of OS 1.7.3's first release, and a later set of the same four). Held here:
// every state a row can have; the eight rules (a tick is "on the unit after the change"; an update only when its
// button is pressed; a new base brings every listed pack on the unit to the listing's file, ticked and locked; the base
// never comes off; a pack another needs is ticked with it; 32 packs at most and no name the loader would not take; the
// plan's order, removals first and base.pack last; the way back for one pack is to untick it). Exit 0 when every check passes.
import { readFileSync } from "node:fs";
import { createHash } from "node:crypto";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import { readListing, knownVersions } from "./catalog.mjs";
import * as P from "./packs-live.mjs";

const here = dirname(fileURLToPath(import.meta.url));
let passed = 0, failed = 0;
function check(what, ok, more = "") {
  if (ok) passed++;
  else { failed++; console.log("FAIL  " + what + (more ? "  " + more : "")); }
}

// ---------- the page's own base.pack, as the page holds it: base-pack.json and the file it names, its header read ----------

const named = JSON.parse(readFileSync(join(here, "base-pack.json"), "utf8"));
const baseBytes = new Uint8Array(readFileSync(join(here, named.file)));
const view = new DataView(baseBytes.buffer, baseBytes.byteOffset, baseBytes.byteLength);
const magic = String.fromCharCode(...baseBytes.subarray(0, 8));
const packName = String.fromCharCode(...baseBytes.subarray(40, 56)).replace(/\0+$/, "");
const headerVersion = view.getUint32(72, true);
check("the page's base.pack is the file base-pack.json names", baseBytes.length === named.bytes && createHash("sha256").update(baseBytes).digest("hex") === named.sha256);
check("its header: a pack file named base, of the version base-pack.json says", magic === "OP1FPACK" && packName === "base" && headerVersion === named.version, magic + " " + packName + " " + headerVersion);
const OS = "1.7.3";

// ---------- two listings: the first release's set, and a later one of the same four ----------

const REPO = "https://github.com/LaP0573/open-field-packs";
const FIRST = "c7953464dffe065211ee8960f5ab12541fd5e89f", LATER = "0123456789abcdef0123456789abcdef01234567";
const N1 = {
  base: { sha256: "5c9a5ead757e4bc7d21b592a4bb2fe59edc7758d7f9749d462e5b059f43b59d3", bytes: 107492, version: 17, release: "0.3" },
  "8 tracks": { sha256: "dffc324f1cb0a45fd42592418678904686f5cb240f4b135f4804d43f6cd8bf2f", bytes: 99484, version: 7 },
  "fx train": { sha256: "58d07f9f5c69ba25e50b59b4920961da5a861e931dbf9a2e621775e49b5f68f8", bytes: 426794, version: 10 },
  lfo: { sha256: "22f7eafa2f002100ddb5d733a47b2de58b664e5cb2565090c11b72e6871e3363", bytes: 375422, version: 8 },
  sidechain: { sha256: "505570137195cff863a7b794ec35c70b59ad239528ba93c46095cfa16cf689be", bytes: 104062, version: 7 },
  wink: { sha256: "35ff760bd94daf67f20f960d8c3304d3bf533b07c094d6530c942f9df05b5085", bytes: 2130, version: 1 },
  yoyo: { sha256: "e0b949f11b525a880582617c00c6491e3703dd3d9ce1444fafb47fa4aee7ef54", bytes: 4381, version: 1 },
  list: { sha256: "39e1aa2190c646a214d0f95e28de3214372051826104707d7f9555fc24b626d7", bytes: 7281, version: 1 },
};
const R2 = {
  base: { sha256: "06af62738e87d40c539297b24a56f2ff2c77275308698e9885d2f5a1a67dce83", bytes: 120278, version: 21, release: "0.6" },
  "8 tracks": { sha256: "9ba0a800b3cf3a67beb987e538e0ded123fc3259ebe352fe9c28854320f8c03e", bytes: 104352, version: 8 },
  "fx train": { sha256: "de8d669c6200fe97b16c7f6ef8b9092bdc0ad5486c6a6e8c7d780ce0eefa6d68", bytes: 482342, version: 11 },
  lfo: { sha256: "d2215f995b523baa89a771eae030ec50a583fa61361b1393dd20b32be050d664", bytes: 430062, version: 9 },
  sidechain: { sha256: "9c5d031d1da64a85e3adca74ea2c74159e964657186fbb69787a395eb195b02f", bytes: 106150, version: 8 },
};
const CATS = { tape: "tape", effects: "effects", modulation: "modulation", sequencing: "sequencing", mixer: "mixer", looks: "screens and looks", tools: "tools", examples: "examples", library: "library" };
const FEATURES = [["8 tracks", "tape"], ["fx train", "effects"], ["lfo", "modulation"], ["sidechain", "mixer"]];

function listingOf(set, commit, extra = [], earlier = null) {
  const packs = [{ id: "base", author: "open-field", status: "bundled", categories: ["library"], description: "the base", path: "builder/base.pack", sha256: set.base.sha256, bytes: set.base.bytes, version: set.base.version, firmware: OS, places: 14, needs: [], folders: [] }];
  for (const [id, cat] of FEATURES) {
    const e = { id, author: "open-field", status: "listed", categories: [cat], description: id, repository: REPO, commit, path: id + ".pack", sha256: set[id].sha256, bytes: set[id].bytes, version: set[id].version, firmware: OS, places: 1, needs: ["base"], folders: [] };
    if (earlier && earlier[id]) e.earlier = earlier[id];
    packs.push(e);
  }
  for (const id of ["wink", "yoyo", "list"]) packs.push({ id, author: "open-field", status: "reserved", categories: ["examples"], description: id, path: id + ".pack", sha256: N1[id].sha256, bytes: N1[id].bytes, version: 1, firmware: OS, places: 0, needs: [], folders: [] });
  return { about: "", categories: CATS, packs: packs.concat(extra) };
}
const unitFile = (name, set, id) => ({ name, size: set[id].bytes, sha256: set[id].sha256, isPack: true, sizeUnknown: false, nameCut: false });
const n1Unit = () => [unitFile("8 tracks.pack", N1, "8 tracks"), unitFile("base.pack", N1, "base"), unitFile("fx train.pack", N1, "fx train"), unitFile("lfo.pack", N1, "lfo"),
  unitFile("list.pack", N1, "list"), unitFile("sidechain.pack", N1, "sidechain"), unitFile("wink.pack", N1, "wink"), unitFile("yoyo.pack", N1, "yoyo")];
const pageBase = (set) => ({ sha256: set.base.sha256, size: set.base.bytes, version: set.base.version, release: set.base.release });
function rowsFor(data, files, hello, base, extraKnown = []) {
  const listing = readListing(data, OS);
  const known = knownVersions(data, OS);
  for (const [sha, v] of extraKnown) known.set(sha, v);
  return { listing, rows: P.readRows({ listing, base, unit: { hello, files }, known }) };
}
const row = (rows, key) => [rows.base, ...rows.listed, ...rows.others].find((r) => r.key === key);

// ---------- 1. a unit with the first release's set, against its own listing: everything on, nothing to do ----------

{
  const { listing, rows } = rowsFor(listingOf(N1, FIRST), n1Unit(), { baseVersion: 17, release: "0.3" }, pageBase(N1));
  check("the base on the unit is the page's own: on", rows.base.state === P.STATE.on);
  check("the four listed packs: on your OP-1, at their listed versions", rows.listed.length === 4 && rows.listed.every((r) => r.state === P.STATE.on && r.unitVersion === r.offerVersion));
  check("the examples are not offered: shown as on the unit and not in the list", rows.others.map((r) => r.key).sort().join(",") === "list.pack,wink.pack,yoyo.pack" && rows.others.every((r) => r.state === P.STATE.notListed && r.knownId));
  check("the examples are never offered to add", !rows.listed.some((r) => ["wink", "yoyo", "list"].includes(r.entry.id)) && !listing.offered.has("wink"));
  const choices = P.initialChoices(rows);
  check("rule 1: what is on the unit comes ticked", [...choices.ticked].sort().join(",") === "8 tracks.pack,fx train.pack,lfo.pack,list.pack,sidechain.pack,wink.pack,yoyo.pack");
  const r = P.resolve(rows, choices, listing, pageBase(N1));
  check("nothing to change at the start", r.plan.send.length === 0 && r.plan.remove.length === 0 && r.problems.length === 0 && r.counts.packsAfter === 8);
  check("no update offered where none is out", !P.updateOffered(rows.base) && rows.listed.every((x) => !P.updateOffered(x)));
  // rule 1 and the way back for one pack: untick it
  choices.ticked.delete("wink.pack");
  choices.ticked.delete("lfo.pack");
  const r2 = P.resolve(rows, choices, listing, pageBase(N1));
  check("rule 1: unticking marks for removal (wink, lfo)", r2.plan.remove.join(",") === "lfo.pack,wink.pack" && r2.plan.send.length === 0);
  check("a row coming off: its words and its dot", P.stateWords(row(rows, "wink.pack"), r2.view.get("wink.pack")) === "coming off" && P.stateColour(row(rows, "wink.pack"), r2.view.get("wink.pack")) === "red");
  check("the bar's lines for two removals", JSON.stringify(P.changeLines(rows, r2)) === JSON.stringify([{ sign: "−", text: "lfo" }, { sign: "−", text: "wink" }]));
  // rule 4: the base can never be unticked, whatever the ticks say
  choices.ticked.delete("base.pack");
  const r3 = P.resolve(rows, choices, listing, pageBase(N1));
  check("rule 4: the base never comes off", !r3.plan.remove.some((n) => n.toLowerCase() === "base.pack") && r3.view.get("base.pack").ticked && r3.view.get("base.pack").locked);
}

// ---------- 2. every state of a row ----------

{
  const data = listingOf(R2, LATER, [], { lfo: [{ version: 8, sha256: N1.lfo.sha256 }] });
  const files = [unitFile("base.pack", N1, "base"), unitFile("lfo.pack", N1, "lfo"),
    { name: "fx train.pack", size: 2130, sha256: N1.wink.sha256, isPack: true },            // a pack of that name, other bytes, no known version
    { name: "8 tracks.pack", size: 104352, sha256: null, isPack: true, hashError: "unreadable" },
    { name: "dub.pack", size: 9000, sha256: "aa".repeat(32), isPack: true },
    { name: "notes.txt", size: 12, sha256: null, isPack: false },
    unitFile("wink.pack", N1, "wink")];
  const { listing, rows } = rowsFor(data, files, { baseVersion: 17, release: "0.3" }, pageBase(R2));
  check("the base: an update is out (base 17 on the unit, 21 beside the page)", rows.base.state === P.STATE.update && P.updateOffered(rows.base));
  check("an older file the listing names as earlier: an update is out", row(rows, "lfo.pack").state === P.STATE.update && row(rows, "lfo.pack").unitVersion === 8 && row(rows, "lfo.pack").offerVersion === 9);
  check("its words", P.stateWords(row(rows, "lfo.pack")) === "v8 on your OP-1 · v9 is out" && P.stateColour(row(rows, "lfo.pack")) === "ochre", P.stateWords(row(rows, "lfo.pack")));
  check("same name, other bytes, no version known: a different file", row(rows, "fx train.pack").state === P.STATE.different && P.stateColour(row(rows, "fx train.pack")) === "blue");
  check("a file whose fingerprint could not be read: a different file", row(rows, "8 tracks.pack").state === P.STATE.different);
  check("listed and not on the unit: not on it, unticked", row(rows, "sidechain.pack").state === P.STATE.absent && !P.initialChoices(rows).ticked.has("sidechain.pack"));
  check("on the unit and not in the listing: kept", row(rows, "dub.pack").state === P.STATE.notListed && P.initialChoices(rows).ticked.has("dub.pack"));
  check("a name the loader does not take: left out at the start, never removed from here", row(rows, "notes.txt").state === P.STATE.leftOut);
  const choices = P.initialChoices(rows);
  choices.ticked.delete("notes.txt");
  const r = P.resolve(rows, choices, listing, pageBase(R2));
  check("left out: locked, kept", r.view.get("notes.txt").locked && r.view.get("notes.txt").action === P.ACTION.keep && !r.plan.remove.includes("notes.txt"));
  check("rule 2: no update made until its button is pressed", r.plan.send.length === 0);
  choices.updates.add("lfo.pack");
  choices.ticked.add("sidechain.pack");
  const r2 = P.resolve(rows, choices, listing, pageBase(R2));
  check("rule 2: the update pressed sends the listing's file under the unit's name", r2.plan.send.some((s) => s.name === "lfo.pack" && s.kind === "update" && s.sha256 === R2.lfo.sha256 && s.version === 9));
  check("rule 1: ticking a listed pack adds it", r2.plan.send.some((s) => s.name === "sidechain.pack" && s.kind === "add"));
  check("the bar's lines", JSON.stringify(P.changeLines(rows, r2)) === JSON.stringify([{ sign: "↑", text: "lfo 8 → 9" }, { sign: "+", text: "sidechain" }]), JSON.stringify(P.changeLines(rows, r2)));
  check("the counts: one new, one updated, the bytes", r2.counts.added === 1 && r2.counts.updated === 1 && r2.counts.bytes === R2.lfo.bytes + R2.sidechain.bytes);
  // the same listing without the earlier versions: the older file is only "a different file", and its button still offers the listing's
  const plain = rowsFor(listingOf(R2, LATER), [unitFile("base.pack", N1, "base"), unitFile("lfo.pack", N1, "lfo")], { baseVersion: 17, release: "0.3" }, pageBase(R2));
  check("without the listing's earlier versions: a different file, its button still offered", row(plain.rows, "lfo.pack").state === P.STATE.different && P.updateOffered(row(plain.rows, "lfo.pack")));
  // the page's own record names the version it sent
  const remembered = rowsFor(listingOf(R2, LATER), [unitFile("base.pack", N1, "base"), unitFile("lfo.pack", N1, "lfo")], { baseVersion: 17, release: "0.3" }, pageBase(R2), [[N1.lfo.sha256, { id: "lfo", version: 8 }]]);
  check("a version the page's own record knows: an update is out", row(remembered.rows, "lfo.pack").state === P.STATE.update);
  // a base on the unit newer than the page's: a different file, no update offered (its way back is the install's)
  const newer = rowsFor(listingOf(N1, FIRST), n1Unit().map((f) => (f.name === "base.pack" ? Object.assign({}, f, { sha256: "bb".repeat(32), size: 181866 }) : f)), { baseVersion: 25, release: "0.7" }, pageBase(N1));
  check("a newer base on the unit: a different file, no update offered", newer.rows.base.state === P.STATE.different && !P.updateOffered(newer.rows.base));
  check("the header's words for each base", P.baseWords(rows).includes("0.6 is out") && P.baseWords(newer.rows).includes("didn't bring"), P.baseWords(rows));
  // the owner's ask (2026-11-20): when a change is offered, the version on the unit and the list's are both said
  check("a different file, its version unknown: the list's version said", P.stateWords(row(plain.rows, "lfo.pack")) === "a different file on your OP-1 · the list has v9", P.stateWords(row(plain.rows, "lfo.pack")));
  const ahead = rowsFor(listingOf(N1, FIRST), [unitFile("base.pack", N1, "base"), unitFile("lfo.pack", R2, "lfo")], { baseVersion: 17, release: "0.3" }, pageBase(N1), [[R2.lfo.sha256, { id: "lfo", version: 9 }]]);
  check("a newer file than the list's: both versions said", row(ahead.rows, "lfo.pack").state === P.STATE.different && P.stateWords(row(ahead.rows, "lfo.pack")) === "v9 on your OP-1 · the list has an older v8", P.stateWords(row(ahead.rows, "lfo.pack")));
  check("a newer base on the unit: the page's own release said too", P.baseWords(newer.rows).includes("(this page has 0.3)"), P.baseWords(newer.rows));
  // a door of build 5 gives each file's own version (the protocol's section 14): used when the fingerprint is unknown
  const withHeader = (f, version, name) => Object.assign(f, { headerVersion: version, headerName: name, headerOsId: 0x1AEB34D1 });
  const told = rowsFor(listingOf(N1, FIRST), [unitFile("base.pack", N1, "base"), withHeader(unitFile("lfo.pack", R2, "lfo"), 9, "lfo"), withHeader(unitFile("8 tracks.pack", R2, "8 tracks"), 6, "8 tracks"), withHeader(unitFile("sidechain.pack", R2, "sidechain"), 8, "wink")], { baseVersion: 17, release: "0.3" }, pageBase(N1));
  check("the header's version, newer than the list's: both said, no update", row(told.rows, "lfo.pack").unitVersion === 9 && P.stateWords(row(told.rows, "lfo.pack")) === "v9 on your OP-1 · the list has an older v8", P.stateWords(row(told.rows, "lfo.pack")));
  check("the header's version, older than the list's: an update is out", row(told.rows, "8 tracks.pack").state === P.STATE.update && P.stateWords(row(told.rows, "8 tracks.pack")) === "v6 on your OP-1 · v7 is out", P.stateWords(row(told.rows, "8 tracks.pack")));
  const other = rowsFor(listingOf(N1, FIRST), [unitFile("base.pack", N1, "base"), Object.assign(unitFile("lfo.pack", R2, "lfo"), { headerVersion: 9, headerName: "lfo", headerOsId: 0xD67577F9 }), Object.assign(unitFile("zap.pack", R2, "lfo"), { headerVersion: 2, headerName: "zap", headerOsId: 0xD67577F9 })], { baseVersion: 17, release: "0.3", osId: 0x1AEB34D1 }, pageBase(N1));
  check("a listed pack made for another OS version: said, grey, its header's version not taken", row(other.rows, "lfo.pack").otherFirmware && row(other.rows, "lfo.pack").unitVersion === null && P.stateWords(row(other.rows, "lfo.pack")).startsWith("made for another OS version") && P.stateColour(row(other.rows, "lfo.pack")) === "grey");
  check("a pack not in the list made for another OS version: said too", row(other.rows, "zap.pack").otherFirmware && P.stateWords(row(other.rows, "zap.pack")).startsWith("made for another OS version"));
  check("the unit's own OS version: nothing said", !row(told.rows, "lfo.pack").otherFirmware);
  check("a header naming another pack: its version not taken", row(told.rows, "sidechain.pack").unitVersion === null && row(told.rows, "sidechain.pack").state === P.STATE.different);
}

// ---------- 3. rule 3: a new base brings the listed packs it needs (the first release's set, a listing of the later set) ----------

{
  const data = listingOf(R2, LATER);
  const { listing, rows } = rowsFor(data, n1Unit(), { baseVersion: 17, release: "0.3" }, pageBase(R2));
  check("every listed pack of the first set: a different file against the later listing", rows.listed.every((r) => r.state === P.STATE.different));
  const choices = P.initialChoices(rows);
  choices.ticked.delete("lfo.pack");                                                       // the owner had unticked one
  choices.updates.add("base.pack");
  const r = P.resolve(rows, choices, listing, pageBase(R2));
  check("rule 3: the new base comes", r.baseComes && r.plan.send.some((s) => s.kind === "base" && s.sha256 === R2.base.sha256));
  check("rule 3: each listed pack on the unit moves to the listing's file, ticked and locked", ["8 tracks.pack", "fx train.pack", "lfo.pack", "sidechain.pack"].every((k) => {
    const v = r.view.get(k);
    return v.ticked && v.locked && v.why === "comes with the new base" && v.action === P.ACTION.update && r.plan.send.some((s) => s.key === k && s.sha256 === R2[s.id].sha256);
  }));
  check("rule 3: the examples, unknown to the listing as files to offer, stay as they are", ["wink.pack", "yoyo.pack", "list.pack"].every((k) => r.view.get(k).action === P.ACTION.keep) && r.plan.remove.length === 0);
  check("rule 7: base.pack last among the files sent", r.plan.send[r.plan.send.length - 1].name === "base.pack" && r.plan.send.length === 5);
  check("the bar's line for the base", P.changeLines(rows, r).some((l) => l.sign === "↑" && l.text === "base 17 → 21"));
  // a listed pack not on the unit is not forced in
  const fewer = rowsFor(data, n1Unit().filter((f) => f.name !== "sidechain.pack"), { baseVersion: 17, release: "0.3" }, pageBase(R2));
  const c2 = P.initialChoices(fewer.rows);
  c2.updates.add("base.pack");
  const r2 = P.resolve(fewer.rows, c2, fewer.listing, pageBase(R2));
  check("rule 3: a listed pack that is not on the unit stays off", !r2.plan.send.some((s) => s.key === "sidechain.pack") && r2.view.get("sidechain.pack").action === P.ACTION.none);
  // the base's update not pressed: nothing forced
  const c3 = P.initialChoices(rows);
  const r3 = P.resolve(rows, c3, listing, pageBase(R2));
  check("rule 3 only with the base's update: nothing forced otherwise", !r3.baseComes && r3.plan.send.length === 0 && [...r3.view.values()].every((v) => v.why !== "comes with the new base"));
}

// ---------- 4. rule 5: a pack another needs; rule 6: 32 at most, no name the loader would not take ----------

{
  const garden = { id: "garden", author: "someone", status: "listed", categories: ["sequencing"], description: "a sequencer", repository: "https://github.com/someone/garden", commit: LATER,
    path: "garden.pack", sha256: "cc".repeat(32), bytes: 50000, version: 2, firmware: OS, places: 0, needs: ["base", "sidechain"], folders: [], level: 5 };
  const orphan = Object.assign({}, garden, { id: "orphan", path: "orphan.pack", sha256: "dd".repeat(32), needs: ["base", "nowhere"] });
  const bad = Object.assign({}, garden, { id: "a/b", path: "ab.pack", sha256: "ee".repeat(32), needs: ["base"] });
  const data = listingOf(N1, FIRST, [garden, orphan, bad]);
  const files = n1Unit().filter((f) => f.name !== "sidechain.pack");
  const { listing, rows } = rowsFor(data, files, { baseVersion: 17, release: "0.3" }, pageBase(N1));
  check("rule 6: a name the loader would not take is never offered (left out of the listing as read)", !listing.offered.has("a/b") && listing.problems.some((p) => p.startsWith("a/b")));
  const choices = P.initialChoices(rows);
  choices.ticked.add("garden.pack");
  const r = P.resolve(rows, choices, listing, pageBase(N1));
  check("rule 5: ticking garden ticks sidechain, which it needs", r.view.get("sidechain.pack").ticked && r.view.get("sidechain.pack").locked && r.view.get("sidechain.pack").why === "garden needs it");
  check("rule 5: both are sent", r.plan.send.map((s) => s.name).sort().join(",") === "garden.pack,sidechain.pack");
  choices.ticked.delete("sidechain.pack");
  const r2 = P.resolve(rows, choices, listing, pageBase(N1));
  check("rule 5: sidechain stays while garden is ticked", r2.view.get("sidechain.pack").ticked && r2.plan.send.some((s) => s.name === "sidechain.pack"));
  check("a pack that needs one the listing does not offer cannot be ticked", r.view.get("orphan.pack").locked && !r.view.get("orphan.pack").ticked && /doesn't offer/.test(r.view.get("orphan.pack").why));
  check("its words in a tile: not on it", P.stateWords(row(rows, "garden.pack")) === "not on it" && P.stateColour(row(rows, "garden.pack")) === "grey");
  // 32 at most: a unit with 31 pack files, two more ticked
  const many = n1Unit();
  for (let i = 0; i < 23; i++) many.push({ name: "extra" + String(i).padStart(2, "0") + ".pack", size: 100, sha256: (i % 10).toString().repeat(64), isPack: true });
  const big = rowsFor(data, many.filter((f) => f.name !== "sidechain.pack"), { baseVersion: 17, release: "0.3" }, pageBase(N1));
  const c4 = P.initialChoices(big.rows);
  check("30 pack files: nothing wrong", P.resolve(big.rows, c4, big.listing, pageBase(N1)).problems.length === 0);
  c4.ticked.add("garden.pack");                                                            // and sidechain with it: 32
  check("32 pack files after the change: allowed", P.resolve(big.rows, c4, big.listing, pageBase(N1)).problems.length === 0);
  many.push({ name: "extra99.pack", size: 1, sha256: "f".repeat(64), isPack: true });
  const bigger = rowsFor(data, many.filter((f) => f.name !== "sidechain.pack"), { baseVersion: 17, release: "0.3" }, pageBase(N1));
  const c5 = P.initialChoices(bigger.rows);
  c5.ticked.add("garden.pack");
  const r5 = P.resolve(bigger.rows, c5, bigger.listing, pageBase(N1));
  check("33 pack files after the change: refused with words", r5.counts.packsAfter === 33 && r5.problems.length === 1 && /32 at most/.test(r5.problems[0]), JSON.stringify(r5.problems));
  c5.ticked.delete("extra00.pack");
  check("one unticked: 32 again", P.resolve(bigger.rows, c5, bigger.listing, pageBase(N1)).problems.length === 0);
}

// ---------- 5. the plan's order, and the unit's files read from the door ----------

{
  const list = { entries: [{ name: "base.pack", size: 5, isPack: true }, { name: "x.pack", size: 3, isPack: true }, { name: "a very long name of more than thirty-nine", size: 1, isPack: false, nameCut: true }] };
  const files = P.unitFiles(list, [{ name: "base.pack", sha256: "AB".repeat(32) }, { name: "x.pack", error: "unreadable" }]);
  check("the door's files: SHA-256 in small letters, a cut name not a pack's", files[0].sha256 === "ab".repeat(32) && files[1].sha256 === null && files[1].hashError && !files[2].isPack);
  const data = listingOf(N1, FIRST);
  const { listing, rows } = rowsFor(data, n1Unit().filter((f) => f.name !== "lfo.pack"), { baseVersion: 17, release: "0.3" }, pageBase(R2));
  const choices = P.initialChoices(rows);
  choices.ticked.add("lfo.pack");
  choices.ticked.delete("wink.pack");
  choices.ticked.delete("yoyo.pack");
  choices.updates.add("base.pack");
  const r = P.resolve(rows, choices, listing, pageBase(R2));
  check("the plan: the removals, and the files with base.pack last", r.plan.remove.join(",") === "wink.pack,yoyo.pack" && r.plan.send.map((s) => s.name).join(",") === "lfo.pack,base.pack", JSON.stringify(r.plan.send.map((s) => s.name)));
  check("about how long it takes, in words the bar can use", P.secondsFor(r.counts.bytes) >= 3 && P.secondsFor(4000000) > P.secondsFor(400000));
}

console.log((failed ? "FAILED: " : "OK: ") + passed + " passed, " + failed + " failed");
process.exitCode = failed ? 1 : 0;
