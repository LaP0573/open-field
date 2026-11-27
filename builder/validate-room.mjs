// Checks room.mjs, the install page's count of the room for packs, with no unit:
//
//   node builder/validate-room.mjs
//
// Pack files are made up here, each a header of 96 bytes and a body, with the sizes, versions, names and cards each
// check needs; the page's own base.pack beside this file is read too. Held here: what a file takes of the room (its
// body and the larger of its lists and its zeroed data) and what is no pack the loader reads; the engines and effects
// a card asks for; the order of the names, byte by byte; each pack at the next multiple of 32 bytes from where it was
// linked, the next behind its foot rounded to a word; of two files of one pack the higher version, the other counted
// only where the placed file stays on the unit; a file of another firmware, and a file the page does not hold; the
// files after a change; and the numbers this file keeps beside the files that give them (the loader's manifest, the
// profile of N2, the base's header). Exit 0 when every check passes.
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import { N1_LOADER, parseHello } from "./door.mjs";
import * as R from "./room.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const root = join(here, "..");
let passed = 0, failed = 0;
function check(what, ok, more = "") {
  if (ok) passed++;
  else { failed++; console.log("FAIL  " + what + (more ? "  " + more : "")); }
}

const OS_ID = 0x1aeb34d1;                       // OS 1.7.3, as a pack's header names it (door.mjs OS_IDS)
const LINK = 0x8d2f4234;                        // where the tools link every pack

// A made-up pack file: a header of 96 bytes, then the body, the lists and what follows them, as docs/packs.md says.
// card: { level, asks: [kinds] } puts a card of 64 bytes at the start of the body and its list of asks behind it.
function pack({ name = "p", version = 1, link = LINK, body = 64, zeroed = 0, lists = 24, extra = 0, osId = OS_ID, card = null, format = 3 } = {}) {
  const bytes = new Uint8Array(96 + body + lists + extra);
  const view = new DataView(bytes.buffer);
  const put = (at, v) => view.setUint32(at, v >>> 0, true);
  bytes.set([0x4f, 0x50, 0x31, 0x46, 0x50, 0x41, 0x43, 0x4b], 0);   // OP1FPACK
  put(8, format); put(12, 96); put(16, osId); put(20, link); put(24, body); put(32, zeroed); put(64, lists); put(68, extra); put(72, version);
  for (let i = 0; i < name.length; i++) bytes[40 + i] = name.charCodeAt(i);
  if (card) {
    put(92, 4);                                  // the card stands 4 bytes into the body
    const c = 96 + 4;
    put(c, 0x44524143); put(c + 4, 64); put(c + 8, card.level);
    put(c + 56, link + 4 + 64); put(c + 60, card.asks.length);
    card.asks.forEach((kind, k) => put(c + 64 + 12 * k, kind));
  }
  return bytes;
}

// ---------- what a file takes of the room ----------

const a = R.footOf(pack({ name: "alpha", version: 3, body: 1000, zeroed: 500, lists: 240 }));
check("a pack's foot is its body and the larger of its lists and zeroed data (zeroed larger)", a && a.foot === 1500 && a.name === "alpha" && a.version === 3 && a.link === LINK, JSON.stringify(a));
const b = R.footOf(pack({ body: 1000, zeroed: 100, lists: 240 }));
check("... and the lists larger", b && b.foot === 1240, JSON.stringify(b));
check("what follows the lists (the pictures) takes no room", R.footOf(pack({ body: 1000, zeroed: 0, lists: 240, extra: 5000 })).foot === 1240);
check("a file that is no pack: no facts", R.footOf(new Uint8Array(200)) === null && R.footOf(new Uint8Array(10)) === null && R.footOf(null) === null);
check("a pack of another format: no facts", R.footOf(pack({ format: 2 })) === null);
const cut = pack({ body: 64 }).subarray(0, 150);
check("a file whose sizes are not the header's: no facts", R.footOf(cut) === null);
const odd = pack({ name: "ab" });
odd[42] = 0x07; odd[43] = 0x63;
check("the name ends at the first byte that is not printable, as the loader takes it", R.footOf(odd).name === "ab");
check("the firmware it was made for is read", R.footOf(pack({ osId: 0x12345678 })).osId === 0x12345678);

// ---------- the engines and effects a card asks for ----------

const engine = R.footOf(pack({ body: 256, card: { level: 9, asks: [8, 7, 7, 4] } }));
check("a card's asks: one engine, two effects (a memory ask counts for neither)", engine.engines === 1 && engine.effects === 2, JSON.stringify(engine));
check("a pack without a card asks for none", R.footOf(pack()).engines === 0 && R.footOf(pack()).effects === 0);
check("a card of level 3 has no asks", R.footOf(pack({ body: 256, card: { level: 3, asks: [8] } })).engines === 0);
const lost = pack({ body: 256, card: { level: 9, asks: [8] } });
new DataView(lost.buffer).setUint32(96 + 4, 0, true);           // the card's mark gone
check("a card that does not stand where the header says: its asks are not known", R.footOf(lost).engines === null && R.footOf(lost).effects === null);
const far = pack({ body: 256, card: { level: 9, asks: [8] } });
new DataView(far.buffer).setUint32(96 + 4 + 56, LINK + 4096, true); // the list of asks outside the body
check("a list of asks outside the body: not known", R.footOf(far).engines === null);

// ---------- the room of the unit's loader ----------

const n2Hello = { loaderSaid: true, loaderGeneration: 2, packsMost: 128, packRoom: 16777216, osId: OS_ID };
const n2 = R.roomOf(n2Hello, null);
check("N2's room: from 0x85000000, 16 MiB, 128 files, from the door's hello", n2.generation === 2 && n2.first === 0x85000000 && n2.bytes === 16777216 && n2.files === 128 && n2.osId === OS_ID, JSON.stringify(n2));
const n1 = R.roomOf({ loaderSaid: false, ...N1_LOADER, osId: OS_ID }, null);
check("a door that says no loader: N1's room, from 0x8d2f4234", n1.generation === 1 && n1.first === 0x8d2f4234 && n1.bytes === 2031616 && n1.files === 32, JSON.stringify(n1));
check("no hello at all: N1's numbers", R.roomOf(null, null).first === 0x8d2f4234 && R.roomOf(null, null).bytes === N1_LOADER.packRoom);
// hello's own bytes, read by door.mjs as the page reads them: 68 bytes, then a tail of 8
const hello = new Uint8Array(76);
hello[0] = 3; hello[68] = 2; hello[70] = 128; hello[72] = 0; hello[73] = 0; hello[74] = 0; hello[75] = 1;
check("from the door's own answer (parseHello): generation 2, 128 files, 16 MiB", (() => { const r = R.roomOf(parseHello(hello), null); return r.generation === 2 && r.files === 128 && r.bytes === 16777216 && r.first === 0x85000000; })());
const manifest = JSON.parse(readFileSync(join(here, "loader.json"), "utf8"));
const n3 = { loaderSaid: true, loaderGeneration: 3, packsMost: manifest.reserve.pack_room.files, packRoom: manifest.reserve.pack_room.bytes };
check("a generation the page doesn't know: the page's own manifest's room, when its bytes and files are the door's", R.roomOf(n3, manifest).first === Number(manifest.reserve.pack_room.base));
check("... and no first address when they are not", R.roomOf({ ...n3, packRoom: 1 << 25 }, manifest).first === null && R.roomOf(n3, null).first === null);

// ---------- where the packs stand ----------

const feet = new Map();
function file(name, sha, bytes) { feet.set(sha, R.footOf(bytes)); return { file: name, sha256: sha }; }
const one = R.placeSet(R.withFacts([file("base.pack", "s1", pack({ name: "base", body: 1000, zeroed: 0, lists: 24 }))], feet), n2);
check("under N2 the first pack stands 20 bytes into the room (0x85000014, docs/packs.md)", one.placed[0].start === 0x14 && one.bytes === 0x14 + 1024, JSON.stringify(one));
const oneN1 = R.placeSet(R.withFacts([{ file: "base.pack", sha256: "s1" }], feet), n1);
check("under N1 at the room's first byte", oneN1.placed[0].start === 0 && oneN1.bytes === 1024);
const two = R.placeSet(R.withFacts([file("b.pack", "s2", pack({ name: "b", body: 1000, zeroed: 1, lists: 4 })), file("a.pack", "s3", pack({ name: "a", body: 60, zeroed: 0, lists: 4 }))], feet), n1);
check("in the order of the files' names", two.placed.map((p) => p.file).join() === "a.pack,b.pack");
// a takes 0..64 (a foot of 64), b (a foot of 1004) the next multiple of 32 from where it was linked: 64
check("the next pack at the next multiple of 32 from where it was linked", two.placed[1].start === 64 && two.bytes === 64 + 1004);
const three = R.placeSet(R.withFacts([{ file: "b.pack", sha256: "s2" }, file("c.pack", "s4", pack({ name: "c", body: 60, lists: 4 })), { file: "a.pack", sha256: "s3" }], feet), n1);
// b ends at 1068, a word already; the next multiple of 32 at or after it is 1088
check("behind a foot rounded up to a word, at the next multiple of 32", three.placed[2].file === "c.pack" && three.placed[2].start === 1088 && three.bytes === 1088 + 64, JSON.stringify(three.placed));
const odd2 = R.placeSet(R.withFacts([file("a.pack", "s5", pack({ name: "a", body: 64, zeroed: 33, lists: 4 })), { file: "c.pack", sha256: "s4" }], feet), n1);
check("a foot of 97 is rounded to 100, and the next pack stands at 128", odd2.placed[1].start === 128 && odd2.bytes === 192, JSON.stringify(odd2.placed));
const caps = R.placeSet(R.withFacts([{ file: "a.pack", sha256: "s3" }, { file: "B.pack", sha256: "s2" }], feet), n1);
check("byte by byte: a capital before a small letter", caps.placed[0].file === "B.pack");
const prefix = R.placeSet(R.withFacts([file("ab.pack", "s6", pack({ name: "ab" })), { file: "a.pack", sha256: "s3" }, file("a b.pack", "s7", pack({ name: "a b" }))], feet), n1);
check("byte by byte: a space, then the dot, then a letter", prefix.placed.map((p) => p.file).join() === "a b.pack,a.pack,ab.pack");

// ---------- two files of one pack ----------

const v1 = pack({ name: "twin", version: 1, body: 2000, lists: 4 }), v2 = pack({ name: "twin", version: 2, body: 400, lists: 4 });
const kept = R.placeSet(R.withFacts([file("twin old.pack", "t1", v1), file("twin.pack", "t2", v2)], feet), n1);
check("of two files of one pack the higher version is placed, the other stands by", kept.placed.length === 2 && kept.placed[0].file === "twin.pack" && !kept.placed[0].standsBy && kept.placed[1].standsBy, JSON.stringify(kept.placed));
check("... both counted where the placed file stays on the unit: the larger end", kept.bytes === 2004, String(kept.bytes));
const sending = R.placeSet(R.withFacts(R.afterChange([{ file: "twin old.pack", sha256: "t1" }], { send: [{ name: "twin.pack", sha256: "t2" }], remove: [] }), feet), n1);
check("... and only the placed file where it is one the change sends", sending.bytes === 404 && sending.placed.length === 2, String(sending.bytes));
const equal = R.placeSet(R.withFacts([file("twin a.pack", "t3", pack({ name: "twin", version: 2, body: 100, lists: 4 })), { file: "twin.pack", sha256: "t2" }], feet), n1);
check("of equal versions the first by name", equal.placed[0].file === "twin a.pack" && equal.placed[1].standsBy);
const asks = R.placeSet(R.withFacts([file("e1.pack", "e1", pack({ name: "e", version: 1, body: 256, card: { level: 9, asks: [8, 8] } })), file("e2.pack", "e2", pack({ name: "e", version: 2, body: 256, card: { level: 9, asks: [8] } })), file("f.pack", "f1", pack({ name: "f", body: 256, card: { level: 8, asks: [7] } }))], feet), n1);
check("engines and effects: of the placed files only", asks.engines === 1 && asks.effects === 1 && asks.asksKnown, JSON.stringify(asks));

// ---------- what the page cannot count ----------

const unknown = R.placeSet(R.withFacts([{ file: "a.pack", sha256: "s3" }, { file: "mine.pack", sha256: "nothere" }, { file: "z.pack", sha256: null }], feet), n1);
check("a file the page does not hold is left out and named", unknown.bytes === 64 && unknown.unknown.join() === "mine.pack,z.pack" && !unknown.asksKnown, JSON.stringify(unknown));
const other = R.placeSet(R.withFacts([{ file: "a.pack", sha256: "s3" }, file("old.pack", "o1", pack({ name: "old", body: 5000, osId: 0xd67577f9 }))], feet), n1);
check("a file made for another firmware takes no room", other.bytes === 64 && other.placed.length === 1);
feet.set("np", null);
const notPack = R.placeSet(R.withFacts([{ file: "a.pack", sha256: "s3" }, { file: "junk.pack", sha256: "np" }], feet), n1);
check("a file the page holds that is no pack takes no room and is not unknown", notPack.bytes === 64 && notPack.unknown.length === 0 && notPack.asksKnown);
const unread = R.placeSet(R.withFacts([{ file: "far.pack", sha256: "far" }].map((e) => (feet.set("far", R.footOf(far)), e)), feet), n1);
check("a placed card whose asks can't be read: the counts are least ones", !unread.asksKnown && unread.engines === 0);

// ---------- the files after a change ----------

const now = [{ file: "base.pack", sha256: "b1" }, { file: "Echo.pack", sha256: "x1" }, { file: "dub.pack", sha256: "d1" }];
const after = R.afterChange(now, { send: [{ name: "echo.pack", sha256: "x2" }, { name: "new.pack", sha256: "n1" }], remove: ["DUB.pack"] });
check("after a change: taken off and replaced files gone (any capitals), sent files there and marked",
  after.map((e) => e.file + ":" + e.sha256 + ":" + e.sent).join() === "base.pack:b1:false,echo.pack:x2:true,new.pack:n1:true", JSON.stringify(after));
check("no change: the same files", R.afterChange(now, { send: [], remove: [] }).length === 3);

// ---------- the page's own base.pack ----------

const named = JSON.parse(readFileSync(join(here, "base-pack.json"), "utf8"));
const baseBytes = new Uint8Array(readFileSync(join(here, named.file)));
const base = R.footOf(baseBytes);
const bv = new DataView(baseBytes.buffer, baseBytes.byteOffset, baseBytes.byteLength);
check("the page's base.pack: its name and version, linked where every pack is", base && base.name === "base" && base.version === named.version && base.link === LINK && base.osId === OS_ID, JSON.stringify(base));
check("its foot from its header's own fields", base.foot === bv.getUint32(24, true) + Math.max(bv.getUint32(64, true), bv.getUint32(32, true)));
check("it asks for no engine and no effect of its own", base.engines === 0 && base.effects === 0, JSON.stringify(base));
const baseN1 = R.placeSet(R.withFacts([{ file: "base.pack", sha256: named.sha256 }], new Map([[named.sha256, base]])), n1);
check("it fits N1's room alone", baseN1.bytes === base.foot && baseN1.bytes < n1.bytes);

// ---------- the numbers this file keeps, beside the files that give them ----------

const profile = JSON.parse(readFileSync(join(root, "profiles", "op1f-1.7.3-n2.json"), "utf8"));
check("N2's first address is the manifest's reserve.pack_room.base", R.ROOM_FIRST[2] === Number(manifest.reserve.pack_room.base), manifest.reserve.pack_room.base);
check("... and the N2 profile's pack_room.base", R.ROOM_FIRST[2] === Number(profile.pack_room.base));
check("the manifest's room is what an N2 door says", manifest.reserve.pack_room.bytes === 16777216 && manifest.reserve.pack_room.files === 128);
check("N1's first address is the window packs are linked for (the N2 profile's pack_link.base)", R.ROOM_FIRST[1] === Number(profile.pack_link.base) && R.ROOM_FIRST[1] === LINK);
check("... and its bytes are N1's room, as door.mjs takes it for a door that says none", profile.pack_link.bytes === N1_LOADER.packRoom);
const card = readFileSync(join(root, "module", "build_r6e_card.h"), "utf8");
check("ENGINES_MOST is the base's OP1F_ENGINES_MOST", new RegExp("\\bOP1F_ENGINES_MOST = " + R.ENGINES_MOST + "\\b").test(card));
check("EFFECTS_MOST is the base's OP1F_EFFECTS_MOST", new RegExp("\\bOP1F_EFFECTS_MOST = " + R.EFFECTS_MOST + "\\b").test(card));
check("an engine is an ask of kind 8, an effect of kind 7", /\bOP1F_ASK_ENGINE = 8\b/.test(card) && /\bOP1F_ASK_EFFECT = 7\b/.test(card));
check("a pack stands at a multiple of 32 from where it was linked: the loader's PACK_ALIGN", R.PACK_ALIGN === 32 && /\bPACK_ALIGN = 32\b/.test(readFileSync(join(root, "module", "build_m4_loader.cpp"), "utf8")));

console.log((failed ? "FAILED: " : "OK: ") + passed + " passed, " + failed + " failed");
process.exitCode = failed ? 1 : 0;
