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
// plan's order, removals first and base.pack last; the way back for one pack is to untick it), and rule 8, the pack
// files picked on the computer with the + tile. Exit 0 when every check passes.
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
  // the door's own most files (hello's tail, door build 7): 32 for a door that says 32 or nothing, 128 for the loader N2
  const said32 = rowsFor(data, many.filter((f) => f.name !== "sidechain.pack"), { baseVersion: 17, release: "0.3", packsMost: 32 }, pageBase(N1));
  const c32 = P.initialChoices(said32.rows); c32.ticked.add("garden.pack");
  const r32 = P.resolve(said32.rows, c32, said32.listing, pageBase(N1));
  check("rule 6 with a door that says 32: 33 refused, 32 named", said32.rows.packsMost === 32 && r32.problems.length === 1 && /loads 32 at most/.test(r32.problems[0]), JSON.stringify(r32.problems));
  const said128 = rowsFor(data, many.filter((f) => f.name !== "sidechain.pack"), { baseVersion: 17, release: "0.3", packsMost: 128 }, pageBase(N1));
  const c128 = P.initialChoices(said128.rows); c128.ticked.add("garden.pack");
  check("rule 6 with a door that says 128: 33 allowed", said128.rows.packsMost === 128 && P.resolve(said128.rows, c128, said128.listing, pageBase(N1)).problems.length === 0);
  const lots = n1Unit().filter((f) => f.name !== "sidechain.pack");
  while (lots.length < 127) lots.push({ name: "more" + String(lots.length).padStart(3, "0") + ".pack", size: 10, sha256: (lots.length % 10).toString().repeat(64), isPack: true });
  const full = rowsFor(data, lots, { baseVersion: 17, release: "0.3", packsMost: 128 }, pageBase(N1));
  const cf = P.initialChoices(full.rows);
  check("rule 6 with 128: 127 pack files, nothing wrong", P.resolve(full.rows, cf, full.listing, pageBase(N1)).problems.length === 0);
  cf.ticked.add("garden.pack");                                                            // and sidechain with it: 129
  const rf = P.resolve(full.rows, cf, full.listing, pageBase(N1));
  check("rule 6 with 128: 129 refused, 128 named", rf.counts.packsAfter === 129 && rf.problems.length === 1 && /leave 129 packs .* loads 128 at most/.test(rf.problems[0]), JSON.stringify(rf.problems));
  cf.ticked.delete("more010.pack");
  check("rule 6 with 128: one unticked, 128 allowed", P.resolve(full.rows, cf, full.listing, pageBase(N1)).problems.length === 0);
  check("MOST_PACKS is still 32, the default", P.MOST_PACKS === 32);
}

// ---------- 4b. the loader's room: the line for a unit on N1 when the page offers N2 ----------

{
  const v1 = { os: "1.7.3", reserve: { module_base: "0x8d2e4234", reserved_bytes: 2097152 } };
  const v2 = { os: "1.7.3", reserve: { module_base: "0x8d2e4234", reserved_bytes: 2097152, pack_room: { base: "0x85000000", bytes: 16777216, files: 128 } } };
  const n1Said = { loaderSaid: true, loaderGeneration: 1, packsMost: 32, packRoom: 2031616 };
  const n1Inferred = { loaderSaid: false, loaderGeneration: 1, packsMost: 32, packRoom: 2031616 };
  const n2 = { loaderSaid: true, loaderGeneration: 2, packsMost: 128, packRoom: 16777216 };
  const LINE = "This unit's loader has a room of 2 MB for 32 pack files. Install again for 16 MB and 128.";
  check("room: N1 said by the door, the page offers N2: the line", P.roomWords(n1Said, v2) === LINE, P.roomWords(n1Said, v2));
  check("room: an older door (N1's numbers taken, the loader not said), the page offers N2: no line, the unit may be on N2", P.roomWords(n1Inferred, v2) === null);
  check("room: a hello with no loader fields at all: no line", P.roomWords({ protocol: 3 }, v2) === null);
  check("room: the page offers N1 (no pack_room): no line", P.roomWords(n1Said, v1) === null && P.roomWords(n1Inferred, v1) === null);
  check("room: the unit on N2 already: no line", P.roomWords(n2, v2) === null);
  check("room: no manifest or no hello: no line", P.roomWords(n1Said, null) === null && P.roomWords(null, v2) === null);
  const v2b = { reserve: { pack_room: { base: "0x85000000", bytes: 33554432, files: 200 } } };
  check("room: the numbers come from the door and the manifest", P.roomWords({ loaderSaid: true, loaderGeneration: 1, packsMost: 40, packRoom: 3 * 1048576 }, v2b) === "This unit's loader has a room of 3 MB for 40 pack files. Install again for 32 MB and 200.");
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

// ---------- 6. rule 8: pack files picked on the computer ----------

{
  // as checkPickedFile answers, with the bytes the page read as `data`
  const pick = (name, more = {}) => Object.assign({ name, file: name + ".pack", version: 3, needs: [], level: null, size: 5000, sha256: (name.length % 10).toString().repeat(64), data: new Uint8Array(5000), from: name + "-build.pack" }, more);
  const orphan = { id: "orphan", author: "someone", status: "listed", categories: ["sequencing"], description: "needs what the list lacks", repository: "https://github.com/someone/orphan", commit: LATER,
    path: "orphan.pack", sha256: "dd".repeat(32), bytes: 50000, version: 2, firmware: OS, places: 0, needs: ["base", "nowhere"], folders: [] };
  const data = listingOf(N1, FIRST, [orphan]);
  const files = n1Unit().filter((f) => f.name !== "sidechain.pack").concat([
    { name: "dub.pack", size: 9000, sha256: "aa".repeat(32), isPack: true },
    Object.assign({ name: "Cool v2.pack", size: 700, sha256: "ab".repeat(32), isPack: true }, { headerVersion: 2, headerName: "cool", headerOsId: 0x1AEB34D1 })]);
  const { listing, rows } = rowsFor(data, files, { baseVersion: 17, release: "0.3" }, pageBase(R2));
  const choices = P.initialChoices(rows);
  const zap = pick("zap");
  const r = P.resolve(rows, choices, listing, pageBase(R2), [zap]);
  const zapSend = r.plan.send.find((s) => s.kind === P.ACTION.picked);
  check("a picked file the unit and the list don't know: sent under its pack's name, with its own bytes and SHA-256", r.plan.send.length === 1 && zapSend.name === "zap.pack" && zapSend.id === "zap" &&
    zapSend.version === 3 && zapSend.sha256 === zap.sha256 && zapSend.bytes === 5000 && zapSend.data === zap.data && zapSend.replaces === null && r.plan.remove.length === 0, JSON.stringify(zapSend));
  check("it counts as new, and the bar says it is yours", r.counts.added === 1 && r.counts.updated === 0 && JSON.stringify(P.changeLines(rows, r)) === JSON.stringify([{ sign: "+", text: "zap (your file)" }]), JSON.stringify(P.changeLines(rows, r)));
  check("its place: new, not the same as a file on the unit", JSON.stringify(r.placed.get(zap)) === JSON.stringify({ replaces: null, same: false }));
  check("no picked file: the plan as before", P.resolve(rows, choices, listing, pageBase(R2)).plan.send.length === 0 && P.resolve(rows, choices, listing, pageBase(R2), []).placed.size === 0);
  // in place of a file on the unit: by its name, or by the pack's name in a file's own header
  const dub = pick("dub"), cool = pick("cool");
  choices.ticked.delete("dub.pack");                                                        // unticked before: the picked file wins
  const r2 = P.resolve(rows, choices, listing, pageBase(R2), [dub, cool]);
  check("a picked file of the name of a file on the unit not in the list takes its place, never with a removal", r2.plan.send.some((s) => s.name === "dub.pack" && s.replaces === "dub.pack" && s.kind === P.ACTION.picked) &&
    !r2.plan.remove.includes("dub.pack") && r2.view.get("dub.pack").action === P.ACTION.picked && r2.view.get("dub.pack").locked, JSON.stringify(r2.plan));
  check("a picked file whose pack another file's header names takes that file's place, under the unit's name", r2.plan.send.some((s) => s.name === "Cool v2.pack" && s.id === "cool" && s.replaces === "Cool v2.pack" && s.from === 2) &&
    !r2.plan.send.some((s) => s.name === "cool.pack"), JSON.stringify(r2.plan.send.map((s) => s.name)));
  check("each counts as updated, the bar's sign an arrow", r2.counts.updated === 2 && r2.counts.added === 0 && P.changeLines(rows, r2).every((l) => l.sign === "↑" && l.text.endsWith(" (your file)")), JSON.stringify(P.changeLines(rows, r2)));
  check("its tile's words and dot, where a row shows it", P.stateWords(row(rows, "dub.pack"), r2.view.get("dub.pack")) === "your file goes in its place" && P.stateColour(row(rows, "dub.pack"), r2.view.get("dub.pack")) === "ochre");
  choices.ticked.add("dub.pack");
  // the very same file as the unit's: nothing sent
  const again = pick("dub", { sha256: "aa".repeat(32) });
  const r3 = P.resolve(rows, choices, listing, pageBase(R2), [again]);
  check("the same file as the unit's: nothing sent, said as the same", r3.plan.send.length === 0 && r3.placed.get(again).same && r3.placed.get(again).replaces === "dub.pack" && r3.view.get("dub.pack").action === P.ACTION.keep);
  check("a row a picked file takes is marked so, the same file or not, and no other row is", r2.view.get("dub.pack").picked && r3.view.get("dub.pack").picked && !r2.view.get("wink.pack").picked && !r2.view.get("lfo.pack").picked);
  // in place of a listed pack: on the unit, even when a new base comes; not on the unit
  const lfo = pick("lfo"), side = pick("sidechain");
  choices.updates.add("base.pack");
  const r4 = P.resolve(rows, choices, listing, pageBase(R2), [lfo, side]);
  check("a picked file of a listed pack's name on the unit takes its place, even when a new base brings the list's files", r4.plan.send.filter((s) => s.key === "lfo.pack").length === 1 &&
    r4.plan.send.find((s) => s.key === "lfo.pack").kind === P.ACTION.picked && r4.view.get("lfo.pack").action === P.ACTION.picked, JSON.stringify(r4.plan.send.map((s) => [s.name, s.kind])));
  check("a picked file of a listed pack's name not on the unit goes on instead of the list's file", r4.plan.send.filter((s) => s.key === "sidechain.pack").length === 1 && r4.plan.send.find((s) => s.key === "sidechain.pack").kind === P.ACTION.picked &&
    r4.placed.get(side).replaces === null);
  check("the other listed packs stay held to the new base, and base.pack is still last", ["8 tracks.pack", "fx train.pack"].every((k) => r4.view.get(k).locked && r4.view.get(k).why === "already the one that goes with the new base") &&
    r4.plan.send.map((s) => s.name).join(",") === "lfo.pack,sidechain.pack,base.pack", JSON.stringify(r4.plan.send.map((s) => s.name)));
  check("a listed row a picked file takes: its words", P.stateWords(row(rows, "lfo.pack"), r4.view.get("lfo.pack")) === "your file goes in its place" && /takes its place/.test(r4.view.get("lfo.pack").why));
  choices.updates.delete("base.pack");
  // what a picked file needs: the list's comes with it; a pack nothing brings is a problem
  const needy = pick("needy", { needs: ["sidechain", "base"] });
  const r5 = P.resolve(rows, choices, listing, pageBase(R2), [needy]);
  check("rule 5 for a picked file: the listed pack it needs is ticked with it and locked", r5.plan.send.some((s) => s.name === "sidechain.pack" && s.kind === "add") && r5.view.get("sidechain.pack").locked && r5.view.get("sidechain.pack").why === "needy needs it" && r5.problems.length === 0, JSON.stringify(r5.problems));
  check("a picked file needing another picked file: no problem", P.resolve(rows, choices, listing, pageBase(R2), [pick("needy", { needs: ["zap"] }), zap]).problems.length === 0);
  check("a picked file needing a file on the unit that stays: no problem", P.resolve(rows, choices, listing, pageBase(R2), [pick("needy", { needs: ["dub"] })]).problems.length === 0);
  choices.ticked.delete("dub.pack");
  const r6 = P.resolve(rows, choices, listing, pageBase(R2), [pick("needy", { needs: ["dub", "nowhere"] })]);
  check("a picked file needing a pack nothing brings, or one coming off: refused with words", r6.problems.length === 2 && r6.problems.every((p) => /^needy needs the pack (dub|nowhere), which isn't on your OP-1/.test(p)), JSON.stringify(r6.problems));
  choices.ticked.add("dub.pack");
  // never the base; 32 at most with the picked files counted
  const r7 = P.resolve(rows, choices, listing, pageBase(R2), [pick("base")]);
  check("a picked base.pack is never sent", r7.plan.send.length === 0 && r7.problems.length === 1 && /only comes with this page/.test(r7.problems[0]));
  const many = Array.from({ length: 22 }, (_, i) => pick("mine" + String(i).padStart(2, "0")));
  check("31 pack files after the change with the picked ones: allowed", P.resolve(rows, choices, listing, pageBase(R2), many).counts.packsAfter === 31 && P.resolve(rows, choices, listing, pageBase(R2), many).problems.length === 0);
  const r8 = P.resolve(rows, choices, listing, pageBase(R2), many.concat([pick("mine22"), pick("mine23")]));
  check("33 with the picked ones: refused with words", r8.counts.packsAfter === 33 && r8.problems.some((p) => /32 at most/.test(p)), JSON.stringify(r8.problems));
  // the review of 2026-11-24: the order of the matches, and needs that nothing brings
  const sideTest = Object.assign({ name: "side test.pack", size: 900, sha256: "cd".repeat(32), isPack: true }, { headerVersion: 9, headerName: "sidechain", headerOsId: 0x1AEB34D1 });
  const alt = rowsFor(data, files.concat([sideTest]), { baseVersion: 17, release: "0.3" }, pageBase(R2));
  const altChoices = P.initialChoices(alt.rows);
  altChoices.ticked.add("sidechain.pack");                                                    // the listed pack ticked too
  const side2 = pick("sidechain");
  const r9 = P.resolve(alt.rows, altChoices, alt.listing, pageBase(R2), [side2]);
  check("a file on the unit whose header names the picked pack is matched before a listed pack of that name not on the unit", r9.placed.get(side2).replaces === "side test.pack" &&
    r9.plan.send.map((s) => s.name).join(",") === "side test.pack", JSON.stringify(r9.plan.send.map((s) => s.name)));
  check("that listed pack is then never added beside it, even ticked", r9.view.get("sidechain.pack").action === P.ACTION.none && r9.view.get("sidechain.pack").locked && !r9.view.get("sidechain.pack").picked);
  const r10 = P.resolve(alt.rows, altChoices, alt.listing, pageBase(R2), [pick("sidechain", { sha256: "cd".repeat(32) })]);
  check("and when the picked file is that very file: nothing sent at all", r10.plan.send.length === 0 && r10.problems.length === 0, JSON.stringify(r10.plan.send.map((s) => s.name)));
  check("a header's pack name is matched whatever its capitals", P.resolve(rows, choices, listing, pageBase(R2), [pick("Cool")]).plan.send.map((s) => s.name).join(",") === "Cool v2.pack");
  const r11 = P.resolve(rows, choices, listing, pageBase(R2), [pick("needy", { needs: ["orphan"] })]);
  check("a picked file needing a listed pack that can't go on: refused with words", r11.problems.length === 1 && /^needy needs the pack orphan/.test(r11.problems[0]) && r11.view.get("orphan.pack").action === P.ACTION.none, JSON.stringify(r11.problems));
  const r12 = P.resolve(rows, choices, listing, pageBase(R2), [pick("needy", { needs: ["cool"] }), pick("cool v2")]);
  check("a needed pack whose file another picked file replaces doesn't count as there", r12.problems.length === 1 && /^needy needs the pack cool,/.test(r12.problems[0]), JSON.stringify(r12.problems));
  check("a needed pack whose file stays counts by its header's name", P.resolve(rows, choices, listing, pageBase(R2), [pick("needy", { needs: ["cool"] })]).problems.length === 0);
}

console.log((failed ? "FAILED: " : "OK: ") + passed + " passed, " + failed + " failed");
process.exitCode = failed ? 1 : 0;
