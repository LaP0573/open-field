// Checks door.mjs, the install page's client for the door in base.pack, with no unit and no MIDI port:
//
//   node open-field/validate-door.mjs
//
// - the door's own examples of protocol 3, byte for byte: every message the client builds is the example's, and every
//   answer of the examples reads back as the example says;
// - the check every send goes through: what it lets through and what it refuses (a message to the unit maker's own
//   service, a control change, a commit with no payload or another one, a write to a door that cannot apply);
// - the client against a model of a unit with the door: hello, list, hash of every file, two files sent, one pack
//   marked to come off, commit 01, the owner's T1 on the model, the answer that says the unit restarts (the ports
//   given back and the page leaving at once), the start, the page back and every file held to its record;
// - "not ready" just after a start asked again for list and hash, a lost answer followed by status, commit never sent
//   twice, the pace (never a second message before the answer to the first or two seconds), no unit and no door;
// - what the client leaves out: no message for the speed test, no other form of commit, no key press over MIDI;
// - the page (index.html) holds the OP-1's ports only while it talks to the unit, drops its MIDI access when the unit
//   goes away while it is idle, and gives everything back before find my OP-1 and before the install steps open.
// Exit 0 when every check passes.
import { createHash } from "node:crypto";
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import * as D from "./door.mjs";
import * as P from "./packs-live.mjs";

const here = dirname(fileURLToPath(import.meta.url));
let passed = 0, failed = 0;
function check(what, ok, more = "") {
  if (ok) passed++;
  else { failed++; console.log("FAIL  " + what + (more ? "  " + more : "")); }
}
const hex = (t) => Uint8Array.from(t.trim().split(/\s+/).map((x) => parseInt(x, 16)));
const same = (a, b) => a.length === b.length && a.every((x, i) => x === b[i]);
const sha = (bytes) => createHash("sha256").update(bytes).digest();
const bytesOf = (text) => Uint8Array.from(text, (c) => c.charCodeAt(0));

// ---------- 1. the examples of protocol 3, byte for byte (base 23, OS 1.7.3) ----------

const EX = [
  ["hello", "F0 7D 4F 46 20 01 00 03 F7",
    "F0 7D 4F 46 60 01 00 00 03 01 17 00 30 2E 33 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 51 02 34 6B 1A 00 00 02 00 13 79 3F 01 00 00 00 00 00 00 00 40 00 00 08 00 00 00 00 00 40 00 00 00 20 00 40 00 00 00 00 00 40 20 02 04 00 20 00 5C 00 05 00 00 00 00 F7"],
  ["remove wink", "F0 7D 4F 46 26 02 00 00 77 69 6E 6B 2E 70 00 61 63 6B F7", "F0 7D 4F 46 66 02 00 00 01 02 00 00 F7"],
  ["take wink back", "F0 7D 4F 46 26 03 00 01 77 69 6E 6B 2E 70 00 61 63 6B F7", "F0 7D 4F 46 66 03 00 00 00 02 01 00 F7"],
  ["remove wink again", "F0 7D 4F 46 26 04 00 00 77 69 6E 6B 2E 70 00 61 63 6B F7", "F0 7D 4F 46 66 04 00 00 01 02 00 00 F7"],
  ["remove the base", "F0 7D 4F 46 26 05 00 00 62 61 73 65 2E 70 00 61 63 6B F7", "F0 7D 4F 46 66 05 02 00 01 F7"],
  ["remove a name that waits", "F0 7D 4F 46 26 06 00 00 79 6F 79 6F 2E 70 00 61 63 6B F7", "F0 7D 4F 46 66 06 05 00 02 F7"],
  ["remove a name not there", "F0 7D 4F 46 26 07 00 00 67 6F 6E 65 2E 70 00 61 63 6B F7", "F0 7D 4F 46 66 07 0B F7"],
  ["list of the removals", "F0 7D 4F 46 21 08 00 00 02 F7", "F0 7D 4F 46 61 08 00 00 01 00 00 01 01 52 08 00 00 00 09 77 69 6E 6B 00 2E 70 61 63 6B F7"],
  ["list of the waiting folder with flags", "F0 7D 4F 46 21 09 00 00 01 01 F7",
    "F0 7D 4F 46 61 09 00 40 02 00 00 02 49 7E 3A 00 05 00 08 6C 66 6F 2E 00 70 61 63 6B 09 1D 11 00 00 00 09 79 6F 79 6F 00 2E 70 61 63 6B F7"],
  ["commit 01", "F0 7D 4F 46 28 0A 00 01 F7", "F0 7D 4F 46 68 0A 00 30 01 01 01 00 1B 4B 05 00 00 F7"],
  ["status 03 while it asks", "F0 7D 4F 46 29 0B 00 03 F7",
    "F0 7D 4F 46 69 0B 07 00 02 00 3A 00 00 00 00 00 00 00 00 00 00 26 00 00 00 00 25 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 01 02 08 7E 3A 08 05 00 7E 3A 05 00 7E 01 3A 05 00 0C 3A 15 00 00 6C 66 6F 2E 70 61 63 00 6B 01 00 02 00 01 00 0C 00 00 1B 4B 05 00 00 00 00 00 00 01 00 00 00 F7"],
  ["status 03 while it applies", "F0 7D 4F 46 29 0C 00 03 F7",
    "F0 7D 4F 46 69 0C 0E 00 06 01 00 00 00 00 00 00 00 00 00 00 00 2A 00 00 00 00 29 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 01 02 08 7E 3A 08 05 00 7E 3A 05 00 7E 01 3A 05 00 0C 3A 15 00 00 6C 66 6F 2E 70 61 63 00 6B 02 00 02 00 01 00 0C 00 00 1B 4B 05 00 00 01 40 02 00 01 08 00 00 00 6C 66 6F 2E 70 61 63 00 6B F7"],
  ["abort while it applies", "F0 7D 4F 46 27 0D F7", "F0 7D 4F 46 67 0D 0E F7"],
  ["status 03 when the change is made", "F0 7D 4F 46 29 0E 00 03 F7",
    "F0 7D 4F 46 69 0E 0A 00 03 01 00 00 00 00 00 00 00 00 00 00 00 33 00 00 00 00 32 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 01 00 08 7E 3A 08 05 00 7E 3A 05 00 7E 01 3A 05 00 0C 3A 15 00 00 6C 66 6F 2E 70 61 63 00 6B 04 01 02 02 01 01 4C 02 00 1B 4B 05 00 1B 01 4B 05 00 00 00 00 00 F7"],
  ["commit 01 refused: another firmware", "F0 7D 4F 46 28 0F 00 01 F7", "F0 7D 4F 46 68 0F 0F 00 02 6C 66 6F 2E 70 61 00 63 6B F7"],
  ["commit 01 refused: 33 files", "F0 7D 4F 46 28 10 00 01 F7", "F0 7D 4F 46 68 10 03 00 06 21 F7"],
  ["commit 01 refused: the room", "F0 7D 4F 46 28 11 00 01 F7", "F0 7D 4F 46 68 11 03 00 07 20 0B 20 00 00 00 00 1F 00 F7"],
  ["commit 01 refused: nothing to apply", "F0 7D 4F 46 28 12 00 01 F7", "F0 7D 4F 46 68 12 05 00 03 F7"],
];
const built = {
  "hello": D.buildMessage(D.CMD.hello, 1, D.payloads.hello(3)),
  "remove wink": D.buildMessage(D.CMD.remove, 2, D.payloads.removeNote("wink.pack")),
  "take wink back": D.buildMessage(D.CMD.remove, 3, D.payloads.removeTakeBack("wink.pack")),
  "remove wink again": D.buildMessage(D.CMD.remove, 4, D.payloads.removeNote("wink.pack")),
  "remove the base": D.buildMessage(D.CMD.remove, 5, D.payloads.removeNote("base.pack")),
  "remove a name that waits": D.buildMessage(D.CMD.remove, 6, D.payloads.removeNote("yoyo.pack")),
  "remove a name not there": D.buildMessage(D.CMD.remove, 7, D.payloads.removeNote("gone.pack")),
  "list of the removals": D.buildMessage(D.CMD.list, 8, D.payloads.list(0, 2)),
  "list of the waiting folder with flags": D.buildMessage(D.CMD.list, 9, D.payloads.list(0, 1, true)),
  "commit 01": D.buildMessage(D.CMD.commit, 10, D.payloads.apply()),
  "status 03 while it asks": D.buildMessage(D.CMD.status, 11, D.payloads.status(3)),
  "status 03 while it applies": D.buildMessage(D.CMD.status, 12, D.payloads.status(3)),
  "abort while it applies": D.buildMessage(D.CMD.abort, 13, D.payloads.abort()),
  "status 03 when the change is made": D.buildMessage(D.CMD.status, 14, D.payloads.status(3)),
  "commit 01 refused: another firmware": D.buildMessage(D.CMD.commit, 15, D.payloads.apply()),
  "commit 01 refused: 33 files": D.buildMessage(D.CMD.commit, 16, D.payloads.apply()),
  "commit 01 refused: the room": D.buildMessage(D.CMD.commit, 17, D.payloads.apply()),
  "commit 01 refused: nothing to apply": D.buildMessage(D.CMD.commit, 18, D.payloads.apply()),
};
const answers = {};
const context3 = { maxMessage: 131072, maxOther: 128, protocol: 3 };
for (const [name, to, from] of EX) {
  check("example " + name + ": the client builds the message byte for byte", same(built[name], hex(to)), D.toHex(built[name]));
  check("example " + name + ": the check lets it through", D.checkMessage(built[name], context3) === null, D.checkMessage(built[name], context3));
  const a = D.parseAnswer(hex(from));
  check("example " + name + ": its answer reads", a && a.error === null && a.command === hex(to)[4] && a.number === hex(to)[5]);
  answers[name] = a;
}
{
  const h = D.parseHello(answers["hello"].payload);
  check("hello: protocol 3, base 23, release 0.3, OS 1.7.3", h.protocol === 3 && h.baseVersion === 23 && h.release === "0.3" && D.firmwareOf(h.osId) === "1.7.3");
  check("hello: the piece 16,384, build 4, 32 removals, the restart 1,500 ms after", h.piece === 16384 && h.build === 4 && h.maxRemovals === 32 && h.restartMs === 1500 && h.startFound === 0);
  check("hello: the largest messages", h.maxMessage === 131072 && h.maxOther === 128 && h.slice === 16384 && h.listEntries === 8);
  check("remove: noted, then taken back, then noted again", D.parseRemove(answers["remove wink"].payload).noted === 1 && D.parseRemove(answers["take wink back"].payload).flag === 1 && D.parseRemove(answers["remove wink again"].payload).noted === 1);
  check("remove refusals: the base, a name that waits, a name not there", answers["remove the base"].status === 2 && answers["remove a name that waits"].status === 5 && answers["remove a name not there"].status === 11);
  const R = D.parseList(answers["list of the removals"].payload);
  check("list of the removals: wink.pack, 2,130 bytes", R.total === 1 && R.entries[0].name === "wink.pack" && R.entries[0].size === 2130);
  const W = D.parseList(answers["list of the waiting folder with flags"].payload, { flags: true });
  check("list of the waiting folder: lfo.pack replaces, yoyo.pack new, both checked", W.total === 2 && W.entries[0].name === "lfo.pack" && W.entries[0].replaces && W.entries[0].checked && W.entries[1].name === "yoyo.pack" && !W.entries[1].replaces);
  const c = D.parseCommit(answers["commit 01"].payload);
  check("commit 01: 1 new, 1 updated, 1 removed, 379,803 bytes to check again", c.newFiles === 1 && c.replacing === 1 && c.removing === 1 && !c.baseComes && c.bytesToCheck === 379803);
  const s1 = D.parseStatus(answers["status 03 while it asks"].payload);
  check("status 03 while it asks: asking, 58 s left, the counts", answers["status 03 while it asks"].status === 7 && s1.state === 2 && s1.secondsLeft === 58 && s1.aState === 1 && s1.aToMove === 2 && s1.aRemovals === 1 && s1.aBytes === 379803);
  const s2 = D.parseStatus(answers["status 03 while it applies"].payload);
  check("status 03 while it applies: status 14, checking lfo.pack", answers["status 03 while it applies"].status === 14 && s2.state === 6 && s2.aState === 2 && s2.aName === "lfo.pack");
  check("status 03 while it applies: the words", D.applyProgressWords(s2) === "Checking 1 of 2, lfo.pack", D.applyProgressWords(s2));
  check("abort while it applies: too late (14)", answers["abort while it applies"].status === 14);
  const s3 = D.parseStatus(answers["status 03 when the change is made"].payload);
  check("status 03 when the change is made: status 10, done, two moved, one removed", answers["status 03 when the change is made"].status === 10 && s3.aState === 4 && s3.aOutcome === 1 && s3.aMoved === 2 && s3.aRemoved === 1);
  const f = D.commitRefusal(answers["commit 01 refused: another firmware"].status, answers["commit 01 refused: another firmware"].payload);
  check("commit refused: another firmware, named", f.reason === 2 && f.name === "lfo.pack" && /another OS version/.test(f.words), f.words);
  const n = D.commitRefusal(answers["commit 01 refused: 33 files"].status, answers["commit 01 refused: 33 files"].payload);
  check("commit refused: 33 pack files", n.count === 33 && /32 at most/.test(n.words), n.words);
  const n128 = D.commitRefusal(D.STATUS.noRoom, [6, 0x81], 128);
  check("commit refused: 129 pack files on a door that takes 128, its number named", n128.count === 129 && /takes 128 at most/.test(n128.words) && !/32/.test(n128.words), n128.words);
  // hello 04 and its tail (door build 7): [68] the generation, [69] 0, [70..71] the most files, [72..75] the room's bytes
  check("hello: the page asks 04 (F0 7D 4F 46 20 01 00 04 F7)", same(D.buildMessage(D.CMD.hello, 1, D.payloads.hello()), hex("F0 7D 4F 46 20 01 00 04 F7")) && D.checkMessage(D.buildMessage(D.CMD.hello, 1, D.payloads.hello()), {}) === null);
  check("hello 03's 68 bytes read as before, the loader N1's numbers", h.loaderSaid === false && h.loaderGeneration === 1 && h.packsMost === 32 && h.packRoom === 2031616 && h.maxRemovals === 32);
  const p68 = Array.from(answers["hello"].payload);
  const p76 = Uint8Array.from([...p68, 2, 0, 128, 0, ...le32(16777216)]);
  const h76 = D.parseHello(p76);
  check("hello's 76 bytes: protocol 3's fields and the tail read", h76.loaderSaid === true && h76.loaderGeneration === 2 && h76.packsMost === 128 && h76.packRoom === 16777216 && h76.protocol === 3 && h76.maxRemovals === 32 && h76.build === 4 && h76.baseVersion === 23);
  const h44 = D.parseHello(Uint8Array.from(p68.slice(0, 44))), h60 = D.parseHello(Uint8Array.from(p68.slice(0, 60)));
  check("hello's 44 and 60 bytes: the loader N1's numbers", [h44, h60].every((x) => x.loaderGeneration === 1 && x.packsMost === 32 && x.packRoom === 2031616));
  const refuses = (bytes) => { try { D.parseHello(Uint8Array.from(bytes)); return false; } catch (e) { return e.kind === "protocol"; } };
  check("hello: any other length refused (43, 67, 69, 72, 75, 77, 84)", [43, 67, 69, 72, 75, 77, 84].every((n) => refuses([...p68, ...new Array(16).fill(0)].slice(0, n))));
  check("hello: 76 bytes that say protocol 2 refused", refuses([2, ...p76.slice(1)]));
  check("commit refused: the room", /too big/.test(D.commitRefusal(answers["commit 01 refused: the room"].status, answers["commit 01 refused: the room"].payload).words));
  check("commit refused: nothing to apply", /nothing to change/.test(D.commitRefusal(answers["commit 01 refused: nothing to apply"].status, answers["commit 01 refused: nothing to apply"].payload).words));
}

// ---------- 2. the check every send goes through ----------

{
  const ok = (bytes, ctx) => D.checkMessage(Uint8Array.from(bytes), ctx) === null;
  check("the identity request goes", ok(D.IDENTITY_REQUEST, {}));
  check("a message to the maker's own service is refused, whatever follows", !ok([0xF0, 0x00, 0x20, 0x76, 0x00, 0x01, 0xF7], context3) && !ok([0xF0, 0x00, 0x20, 0x76, 0x7F, 0x7F, 0x7F, 0x7F, 0xF7], context3));
  check("a control change is refused (no key press over MIDI)", !ok([0xB0, 0x0B, 0x7F], context3) && !ok([0xB0, 0x0B, 0x00], context3) && !ok([0x90, 0x3C, 0x7F], context3));
  check("commit with no payload is refused", !ok(D.buildMessage(D.CMD.commit, 1), context3));
  check("commit 02 (a test stop) is refused", !ok(D.buildMessage(D.CMD.commit, 1, [2, 1]), context3));
  check("the speed test's message (0x2A) is refused", !ok(D.buildMessage(0x2A, 1, [1, 2, 3]), context3));
  check("every command this client does not have is refused", [0x00, 0x10, 0x14, 0x1F, 0x2A, 0x2D, 0x2F, 0x30, 0x3F].every((c) => !ok(D.buildMessage(c, 1), context3)));
  check("writes go only to a door that can apply", [D.CMD.begin, D.CMD.data, D.CMD.end, D.CMD.remove, D.CMD.commit].every((c) => !ok(D.buildMessage(c, 1, c === D.CMD.commit ? [1] : [0]), { maxMessage: 131072, maxOther: 128, protocol: 2 })));
  check("hello, list, hash, status and abort go to any door", ok(D.buildMessage(D.CMD.hello, 1, [3]), {}) && ok(D.buildMessage(D.CMD.list, 1, [0]), {}) && ok(D.buildMessage(D.CMD.status, 1, []), {}) && ok(D.buildMessage(D.CMD.abort, 1, []), {}));
  check("before hello, 64 bytes at most", !ok(D.buildMessage(D.CMD.list, 1, new Array(60).fill(0)), {}) && ok(D.buildMessage(D.CMD.hash, 1, D.payloads.hashStart("a".repeat(34) + ".pack")), {}));
  check("a message longer than the unit's largest is refused", !ok(D.buildMessage(D.CMD.data, 1, new Uint8Array(200)), { maxMessage: 128, maxOther: 128, protocol: 3 }));
  check("a byte of 0x80 or more inside is refused", !ok([0xF0, 0x7D, 0x4F, 0x46, 0x29, 0x01, 0x80, 0xF7], context3));
  check("a number of 0x40 or more is refused", !ok([0xF0, 0x7D, 0x4F, 0x46, 0x29, 0x40, 0xF7], context3));
  check("a name the loader would not take is refused before anything is built", (() => { try { D.payloads.removeNote("../x.pack"); return false; } catch (e) { return e.kind === "name"; } })());
}

// ---------- 3. the source: what the client leaves out ----------

{
  const src = readFileSync(join(here, "door.mjs"), "utf8");
  const code = src.replace(/^\s*\/\/.*$/gm, "");
  check("no key press over MIDI in the client", !/PRESS_T1|0xB0/.test(src));
  check("no speed test in the client", !/\bsink\b|0x2A/i.test(code));
  check("no test stop and no visit mode in the client", !/testStop|commit 02|visit/i.test(code));
  const prefix = src.split("\n").filter((l) => /0x00,\s*0x20,\s*0x76/i.test(l));
  check("the maker's prefix is written once, as the prefix the check refuses", prefix.length === 1 && /const REFUSED_PREFIX/.test(prefix[0]) && (src.match(/REFUSED_PREFIX/g) || []).length === 2);
  check("io.send is called in one place, after the pace and the check", (src.match(/io\.send\(/g) || []).length === 1 &&
    src.indexOf("const since = this.io.now() - this.lastSend.at;") < src.indexOf("const why = checkMessage(bytes, this.context());") &&
    src.indexOf("const why = checkMessage(bytes, this.context());") < src.indexOf("this.io.send(bytes);"));
  check("only ports named OP-1", /export function isOp1PortName\(name\) \{ return typeof name === "string" && name\.toLowerCase\(\)\.includes\("op-1"\); \}/.test(src));
  check("the client loads nothing (no fetch, no import but none)", !/\bfetch\s*\(|\bimport\s*\(|^import /m.test(src));
}

// ---------- 4. a model of a unit with the door (protocol 3) ----------

const OS_173 = 0x1AEB34D1;
function le32(v) { return [v & 255, (v >>> 8) & 255, (v >>> 16) & 255, (v >>> 24) & 255]; }
function answer(command, number, status, payload = []) {
  const p = D.pack(Uint8Array.from(payload));
  return Uint8Array.from([0xF0, 0x7D, 0x4F, 0x46, (command & 0x3F) | 0x40, number, status, ...p, 0xF7]);
}
const isPackName = (n) => D.checkName(n) === null;
const byBytes = (a, b) => (a < b ? -1 : a > b ? 1 : 0);

class UnitModel {
  constructor(files, options = {}) {
    this.pk = new Map(Object.entries(files));
    this.pkin = new Map();                 // name -> { data, checked }
    this.removals = new Set();
    this.osId = options.osId ?? OS_173;
    this.base = options.base ?? 25;
    this.notReady = options.notReady ?? 0; // list and hash answer 13 this many times after a start
    this.drop = new Set();                 // the numbers of messages whose answer is lost
    this.received = [];                    // every message, as it came
    this.state = 0; this.outcome = 0; this.aState = 0; this.aOutcome = 0; this.counts = null; this.since = 0; this.t1 = false;
    this.xfer = null; this.lastX = { state: 0, outcome: 0, name: "", size: 0, received: 0, checked: 0 };
    this.hashing = null; this.restartAsked = false; this.messages = 0; this.answers = 0;
    this.noDoor = !!options.noDoor; this.noUnit = !!options.noUnit;
    this.build = options.build ?? 5;       // the door's build, hello's field 58
    this.tail = options.tail ?? null;      // door build 7: { generation, most, room }, hello's tail of 8 to hello 04 (none: a door before build 7)
    this.logs = options.logs ?? [];        // the log files, newest first: { name, data }
    this.logsNotReady = 0;                 // logs and logread answer 13 this many times
    this.logReads = [];                    // every logread: { name, offset, want }
  }
  start() {                                // a start: the waiting folder emptied, the marks forgotten
    this.pkin.clear(); this.removals.clear(); this.state = 0; this.aState = 0; this.aOutcome = 0; this.counts = null; this.t1 = false;
    this.xfer = null; this.hashing = null; this.restartAsked = false; this.notReady = this.notReadyAfterStart ?? 0;
  }
  tick(ms) {
    this.since += ms;
    if (this.state === 2 && this.autoT1 && this.since >= 600) { this.state = 6; this.aState = 2; this.since = 0; }
    else if (this.state === 6 && this.since >= 250) { this.apply(); }
  }
  apply() {                                // the change, as the unit makes it: removals first, then each file, base.pack last
    for (const n of this.removals) this.pk.delete(n);
    const names = [...this.pkin.keys()].filter((n) => this.pkin.get(n).checked).sort((a, b) => (a === "base.pack") - (b === "base.pack"));
    this.order = names.slice();
    for (const n of names) this.pk.set(n, this.pkin.get(n).data);
    this.moved = names.length; this.removed = this.removals.size;
    this.pkin.clear(); this.removals.clear();
    this.state = 3; this.aState = 4; this.aOutcome = 1;
  }
  listOf(folder) {
    if (folder === 0) return [...this.pk.keys()].sort(byBytes).map((n) => ({ name: n, size: this.pk.get(n).length, flags: isPackName(n) ? 1 : 0 }));
    if (folder === 1) return [...this.pkin.keys()].sort(byBytes).map((n) => ({ name: n, size: this.pkin.get(n).data.length, flags: (isPackName(n) ? 1 : 0) | (this.pkin.get(n).checked ? 8 : 0) | (this.pk.has(n) ? 64 : 0) }));
    return [...this.removals].sort(byBytes).map((n) => ({ name: n, size: this.pk.get(n).length, flags: 1 }));
  }
  statusPayload(form) {
    const p = [this.state, this.outcome, 0, 0, ...le32(this.hashing ? this.hashing.size : 0), ...le32(this.hashing ? this.hashing.done : 0), ...le32(this.messages), ...le32(this.answers), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    if (form < 2) return p;
    const x = this.xfer || this.lastX, xname = bytesOf(x.name || "");
    p.push(this.xfer ? this.xfer.state : 0, this.lastX.outcome, [...this.pkin.values()].filter((f) => f.checked).length, xname.length, ...le32(x.size || 0), ...le32(x.received || 0), ...le32(x.checked || 0), ...le32(1391116), ...xname);
    if (form < 3) return p;
    const c = this.counts || { moves: 0, removals: 0, bytes: 0 };
    p.push(this.aState, this.aOutcome, c.moves, this.aState >= 3 ? c.moves : 0, c.removals, this.aState >= 4 ? c.removals : 0, this.aState >= 4 ? c.moves : 0, 0, ...le32(c.bytes), ...le32(this.aState >= 3 ? c.bytes : 0), this.removals.size, 0, 0, 0);
    return p;
  }
  receive(m) {
    this.received.push(Uint8Array.from(m));
    if (this.noUnit) return [];
    if (same(m, D.IDENTITY_REQUEST)) return [Uint8Array.from([0xF0, 0x7E, 0x00, 0x06, 0x02, 0x7D, 0x00, 0x00, 0x00, 0xF7])];   // an identity answer (a made-up maker)
    if (this.noDoor || m[0] !== 0xF0 || m[1] !== 0x7D) return [];
    const cmd = m[4], num = m[5];
    const u = D.unpack(m.subarray(6, m.length - 1));
    const p = u.bytes;
    this.messages++;
    const out = this.answerFor(cmd, num, p);
    if (!out) return [];
    if (this.drop.has(num) && this.dropOnce !== num) { this.dropOnce = num; return []; }
    this.answers++;
    return [out];
  }
  answerFor(cmd, num, p) {
    const A = (status, payload) => answer(cmd, num, status, payload);
    if (this.state === 3) { this.restartAsked = true; return A(10); }
    if (cmd === D.CMD.hello) {
      const rel = [...bytesOf("0.7")]; while (rel.length < 16) rel.push(0);
      const payload = [3, 1, this.base & 255, this.base >> 8, ...rel, ...le32(this.osId), ...le32(131072), ...le32(114681), ...le32(128), ...le32(16384), 8, this.state, 0, 0,
        ...le32(16384), ...le32(4194304), ...le32(8388608), 32, [...this.pkin.values()].filter((f) => f.checked).length, this.build, 0, 32, this.removals.size, 0xDC, 0x05, 0, 0, 0, 0];
      if (this.tail && p.length === 1 && p[0] >= 4) payload.push(this.tail.generation, 0, this.tail.most & 255, this.tail.most >> 8, ...le32(this.tail.room));
      return A(this.state === 2 ? 7 : this.state === 6 ? 14 : 0, payload);
    }
    if (cmd === D.CMD.status) {
      return A(this.state === 2 ? 7 : this.state === 6 ? 14 : 0, this.statusPayload(p.length ? p[0] : 1));
    }
    if (this.state === 6) return A(14);
    if (this.state === 2 && cmd !== D.CMD.abort) return A(7);
    if ((cmd === D.CMD.list || cmd === D.CMD.hash) && this.notReady > 0) { this.notReady--; return A(13); }
    if (cmd === D.CMD.list) {
      const whole = this.listOf(p.length >= 2 ? p[1] : 0), cap = (this.build ?? 0) >= 8 ? 192 : 64, all = whole.slice(0, cap), first = p[0];
      (this.listFirsts ??= []).push(first);
      if (first > cap - 1) return A(2);
      const page = all.slice(first, first + 8);
      const payload = [all.length, whole.length - all.length, first, page.length];
      for (const e of page) { const nb = bytesOf(e.name); payload.push(e.flags, ...le32(e.size), nb.length, ...nb); }
      return A(0, payload);
    }
    if (cmd === D.CMD.hash) {
      if (p[0] === 0) {
        const name = String.fromCharCode(...p.subarray(1));
        if (!isPackName(name)) return A(2);
        const data = this.pk.get(name);
        if (!data) return A(11, [...le32(0xFFFFFFFF), ...le32(0)]);
        this.hashing = { name, size: data.length, done: 0 };
      } else if (!this.hashing) return A(5);
      const h = this.hashing, data = this.pk.get(h.name);
      h.done = Math.min(h.size, h.done + 16384);
      if (h.done < h.size) return A(9, [...le32(h.size), ...le32(h.done), ...le32(0), ...le32(0)]);
      this.hashing = null;
      return A(0, [...le32(h.size), ...le32(h.done), ...le32(0), ...le32(0), ...sha(data)]);
    }
    if (cmd === D.CMD.begin) {
      const size = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24), want = p.slice(4, 36), name = String.fromCharCode(...p.subarray(36));
      if (!isPackName(name)) return A(2);
      this.removals.delete(name);
      this.pkin.delete(name);
      this.xfer = { state: 4, name, size, want, data: new Uint8Array(size), received: 0, checked: 0 };
      this.lastX = this.xfer;
      return A(0, [...le32(1391116), [...this.pkin.values()].filter((f) => f.checked).length, 0, 0, 0]);
    }
    if (cmd === D.CMD.data) {
      const x = this.xfer, off = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
      if (!x || x.state !== 4) return A(5);
      if (off !== x.received) return A(5, [...le32(x.received), 0, 0, 0, 0, 0, 0, 0, 0]);
      x.data.set(p.subarray(4), off); x.received += p.length - 4;
      return A(0, [...le32(x.received), 0, 0, 0, 0, 0, 0, 0, 0]);
    }
    if (cmd === D.CMD.end) {
      const x = this.xfer;
      if (!x) return A(5);
      if (p[0] === 0) { x.state = 5; return A(9, [...le32(x.size), ...le32(0), 0, 0, 0, 0, 0, 0, 0, 0]); }
      x.checked = Math.min(x.size, x.checked + 16384);
      if (x.checked < x.size) return A(9, [...le32(x.size), ...le32(x.checked), 0, 0, 0, 0, 0, 0, 0, 0]);
      const got = sha(x.data);
      this.xfer = null;
      this.lastX = { state: 0, outcome: same(got, x.want) ? 1 : 2, name: x.name, size: x.size, received: x.received, checked: x.checked };
      if (!same(got, x.want)) return A(6, [1, ...le32(x.size), ...le32(x.checked), ...got]);
      this.pkin.set(x.name, { data: x.data, checked: true });
      return A(0, [...le32(x.size), ...le32(x.checked), 0, 0, 0, 0, 0, 0, 0, 0, ...got]);
    }
    if (cmd === D.CMD.remove) {
      const name = String.fromCharCode(...p.subarray(1));
      if (p[0] === 0) {
        if (name.toLowerCase() === "base.pack") return A(2, [1]);
        if (this.pkin.has(name)) return A(5, [2]);
        if (!this.pk.has(name)) return A(11);
        const had = this.removals.has(name); this.removals.add(name);
        return A(0, [this.removals.size, this.pkin.size, had ? 1 : 0, 0]);
      }
      const had = this.removals.delete(name);
      return A(0, [this.removals.size, this.pkin.size, had ? 1 : 0, 0]);
    }
    if (cmd === D.CMD.abort) {
      if (this.state === 2) { this.state = 0; this.aState = 5; this.aOutcome = 4; return A(0, [2]); }
      if (p[0] === 1) { const n = this.pkin.size; this.pkin.clear(); this.xfer = null; return A(0, [n ? 8 : 0, n]); }
      this.xfer = null;
      return A(0, [0]);
    }
    if (cmd === D.CMD.commit) {
      const waiting = [...this.pkin.values()].filter((f) => f.checked);
      if (!waiting.length && !this.removals.size) return A(5, [3]);
      const replacing = [...this.pkin.keys()].filter((n) => this.pk.has(n)).length;
      const bytes = waiting.reduce((s, f) => s + f.data.length, 0);
      this.counts = { moves: waiting.length, removals: this.removals.size, bytes };
      this.state = 2; this.aState = 1; this.since = 0;
      return A(0, [waiting.length - replacing, replacing, this.removals.size, this.pkin.has("base.pack") ? 1 : 0, ...le32(bytes)]);
    }
    if (cmd === D.CMD.logs || cmd === D.CMD.logread) {
      if (this.build < 6) return A(4);
      if (this.logsNotReady > 0) { this.logsNotReady--; return A(13); }
    }
    if (cmd === D.CMD.logs) {
      const first = p[0], page = this.logs.slice(first, first + 8);
      const payload = [this.logs.length & 255, this.logs.length >> 8, first, page.length];
      for (const f of page) { const nb = bytesOf(f.name); payload.push(...le32(f.data.length), nb.length, ...nb); }
      return A(0, payload);
    }
    if (cmd === D.CMD.logread) {
      const offset = (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)) >>> 0, want = p[4] | (p[5] << 8), name = String.fromCharCode(...p.subarray(6));
      if (!/^log_.*\.txt$/.test(name)) return A(2);                    // a name outside the log files' rule
      const f = this.logs.find((x) => x.name === name);
      if (!f) return A(11, [...le32(0xFFFFFFFF), ...le32(offset)]);    // a log not there: its size unreadable
      this.logReads.push({ name, offset, want });
      const from = Math.min(offset, f.data.length);
      return A(0, [...le32(f.data.length), ...le32(offset), ...f.data.subarray(from, Math.min(f.data.length, offset + want))]);
    }
    return A(4);
  }
}

// The page's side of the model: a clock that only moves when the client waits, and the breaches of the pace.
function bench(unit) {
  const b = { t: 0, queue: [], sent: [], breaches: [], released: 0, letGo: [], pendingSince: null, stop: false };
  b.io = {
    send(bytes) {
      if (b.pendingSince !== null && b.t - b.pendingSince < 2000) b.breaches.push("a message " + (b.t - b.pendingSince) + " ms after one with no answer");
      b.sent.push(Uint8Array.from(bytes));
      b.pendingSince = b.t;
      for (const a of unit.receive(Uint8Array.from(bytes))) b.queue.push({ data: a, at: b.t });
    },
    async next(ms) {
      if (b.queue.length) { b.pendingSince = null; return b.queue.shift(); }
      b.t += ms; unit.tick(ms);
      return null;
    },
    now: () => b.t,
    async sleep(ms) { b.t += ms; unit.tick(ms); },
    stopped: () => b.stop,
    release() { b.released++; },
    letGo(why) { b.letGo.push(why); },
  };
  return b;
}

function pack3(name, size, seed) {
  const d = new Uint8Array(size);
  for (let i = 0; i < size; i++) d[i] = (i * 7 + seed) & 255;
  return d;
}

// ---------- 4b. the loader's room from hello: a door of build 7 says it, an older one answers 04 as 03 ----------

{
  const files = { "base.pack": pack3("base", 1000, 1) };
  const seven = new UnitModel(files, { build: 7, tail: { generation: 2, most: 128, room: 16777216 } });
  const c7 = new D.DoorClient(bench(seven).io);
  const r7 = await c7.connect();
  const sent7 = seven.received.find((m) => m[4] === D.CMD.hello);
  check("a door of build 7: hello 04 sent, its tail taken into the limits", r7.kind === "door" && sent7 && sent7[7] === 4 && c7.hello.loaderGeneration === 2 && c7.limits.packsMost === 128 && c7.limits.packRoom === 16777216 && c7.protocol === 3,
    JSON.stringify(c7.limits));
  const six = new UnitModel(files, { build: 6 });
  const c6 = new D.DoorClient(bench(six).io);
  const r6 = await c6.connect();
  check("a door before build 7: hello 04 answered with 68 bytes, the loader N1's numbers", r6.kind === "door" && c6.protocol === 3 && c6.hello.loaderSaid === false && c6.limits.packsMost === 32 && c6.limits.packRoom === 2031616);
}

// ---------- 4c. a folder of more than 64 files: door build 8 lists 192, an older door 64 and says the rest ----------

{
  const files = {};
  for (let i = 0; i < 100; i++) files["p" + String(i).padStart(3, "0") + ".pack"] = pack3("p" + i, 64, i + 1);
  const eight = new UnitModel(files, { build: 8, tail: { generation: 2, most: 128, room: 16777216 } });
  const c8 = new D.DoorClient(bench(eight).io);
  await c8.connect();
  const L8 = await c8.listPacks(0);
  check("door build 8: all 100 files listed, past index 63", L8.total === 100 && L8.beyond === 0 && L8.entries.length === 100, L8.total + " " + L8.entries.length);
  const seven = new UnitModel(files, { build: 7, tail: { generation: 2, most: 128, room: 16777216 } });
  const c7 = new D.DoorClient(bench(seven).io);
  await c7.connect();
  const L7 = await c7.listPacks(0);
  check("door build 7: 64 files listed, the other 36 said as beyond, no index past 63 asked", L7.total === 64 && L7.beyond === 36 && L7.entries.length === 64 &&
    Math.max(...seven.listFirsts) <= 63 && Math.max(...eight.listFirsts) > 63, L7.total + " " + L7.beyond + " " + L7.entries.length);
}

// ---------- 5. the whole change against the model ----------

{
  const files = { "base.pack": pack3("base", 181866, 1), "fx train.pack": pack3("fx", 426794, 2), "lfo.pack": pack3("lfo-old", 375000, 3), "wink.pack": pack3("wink", 2130, 4) };
  const unit = new UnitModel(files, { notReady: 3 });
  unit.autoT1 = true;
  const b = bench(unit);
  const client = new D.DoorClient(b.io);
  const c = await client.connect();
  check("connect: the door answers protocol 3 on OS 1.7.3", c.kind === "door" && client.protocol === 3 && D.firmwareOf(c.hello.osId) === "1.7.3" && client.limits.piece === 16384);
  const L = await client.listPacks(0);
  check("list after a start: 'not ready' asked again, then the four files", L.total === 4 && unit.notReady === 0, JSON.stringify(L.entries.map((e) => e.name)));
  const hashes = [];
  for (const e of L.entries) hashes.push(await client.hashPack(e.name));
  check("hash of every file, slice by slice, each the file's SHA-256", hashes.every((h) => h.sha256 === sha(files[h.name]).toString("hex")));
  const newLfo = pack3("lfo-new", 381000, 9), sidechain = pack3("sidechain", 104062, 5);
  const many = { files: Array.from({ length: 33 }, (_, i) => { const d = pack3("m" + i, 64, 100 + i); return { name: "m" + String(i).padStart(2, "0") + ".pack", data: d, sha256: sha(d) }; }), remove: [] };
  const before = unit.received.length;
  const tooMany = await client.changePacks(many, { firmware: "1.7.3", saveRecord: () => true });
  check("33 packs to go on: refused before anything is sent, with the door's 32", tooMany.stage === "refused" && tooMany.words === "At most 32 packs can go on in one go. Untick a few, send, and then send the rest." && unit.received.length === before, tooMany.words);
  const plan = { files: [{ name: "lfo.pack", data: newLfo, sha256: sha(newLfo) }, { name: "sidechain.pack", data: sidechain, sha256: sha(sidechain) }], remove: ["wink.pack"] };
  let kept = null, asked = null;
  const out = await client.changePacks(plan, { firmware: "1.7.3", saveRecord: (r) => { kept = JSON.parse(JSON.stringify(r)); return true; }, onAsked: (x) => { asked = x; } });
  check("the change: applied, the unit restarts", out.stage === "applied" && out.apply.restarting && out.apply.outcome === 1, out.words);
  check("the unit asked with the counts: 1 new, 1 updated, 1 removed", asked && asked.newFiles === 1 && asked.replacing === 1 && asked.removing === 1);
  check("on the answer that says it restarts: the ports given back once, the page leaves once", b.released === 1 && b.letGo.length === 1 && b.letGo[0] === "restart");
  check("the record kept before commit, its phase restarting, for OS 1.7.3", kept && kept.phase === "restarting" && kept.firmware === "1.7.3" && kept.sent.length === 2 && kept.removed[0].name === "wink.pack");
  check("the unit took the removal first and moved the files", !unit.pk.has("wink.pack") && same(unit.pk.get("lfo.pack"), newLfo) && same(unit.pk.get("sidechain.pack"), sidechain));
  const commits = b.sent.filter((m) => m[0] === 0xF0 && m[1] === 0x7D && m[4] === D.CMD.commit);
  check("commit sent once, with 01", commits.length === 1 && same(D.unpack(commits[0].subarray(6, commits[0].length - 1)).bytes, [1]));
  const order = b.sent.filter((m) => m[1] === 0x7D && (m[4] === D.CMD.remove || m[4] === D.CMD.begin)).map((m) => m[4]);
  check("the removal marked before the first file is sent", order[0] === D.CMD.remove && order.slice(1).every((x) => x === D.CMD.begin));
  // the start, and the page back
  unit.notReadyAfterStart = 2;
  unit.start();
  const r = await D.reachDoorAgain(client, { takePorts: () => true, setTimer: () => 0, clearTimer: () => {}, onSlow: () => {} });
  check("back: the door found again", r.kind === "door");
  const after = await client.checkAfterRestart(kept);
  check("back: every file installed and the removal made", after.verdict && after.verdict.allGood, JSON.stringify(after.verdict));
  check("the pace held all through", b.breaches.length === 0, b.breaches.join("; "));
  check("nothing sent but the door's messages and the identity request", b.sent.every((m) => (m[0] === 0xF0 && m[1] === 0x7D) || same(m, D.IDENTITY_REQUEST)));
  check("never a message to the maker's own service", b.sent.every((m) => !(m[0] === 0xF0 && m[1] === 0x00 && m[2] === 0x20 && m[3] === 0x76)));
  const commands = new Set(b.sent.filter((m) => m[1] === 0x7D).map((m) => m[4]));
  check("only the commands the client has", [...commands].every((x) => [0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29].includes(x)), [...commands].join(","));
}

// ---------- 6. a lost answer, commit lost, T2, a refusal, no door, no unit ----------

{
  const unit = new UnitModel({ "base.pack": pack3("b", 5000, 1), "lfo.pack": pack3("l", 40000, 2) });
  const b = bench(unit);
  const client = new D.DoorClient(b.io);
  await client.connect();
  unit.drop.add(client.number);                              // the next message's answer is lost once
  const L = await client.listPacks(0);
  const statusAfter = b.sent.slice(-3).map((m) => m[4]);
  check("a lost answer: status after two seconds, then the message again", L.total === 2 && statusAfter[0] === D.CMD.list && statusAfter[1] === D.CMD.status && statusAfter[2] === D.CMD.list);
  check("a lost answer: the pace held", b.breaches.length === 0, b.breaches.join("; "));
  unit.notReady = 2;
  const h = await client.hashPack("lfo.pack");
  check("'not ready' to hash asked again", h.sha256 === sha(unit.pk.get("lfo.pack")).toString("hex") && unit.notReady === 0);
  // commit's answer lost: never sent again, the status decides
  const data = pack3("x", 3000, 7);
  const before = b.sent.length;
  unit.drop.clear();
  const plan = { files: [{ name: "extra.pack", data, sha256: sha(data) }], remove: [] };
  let asking = 0;
  const p = client.changePacks(plan, { firmware: "1.7.3", saveRecord: () => true, onState: (s) => { if (s.status === 7) asking++; }, wantsCancel: () => asking >= 2 });
  // the commit is the first message after the file's check; its number is not known beforehand: drop by command instead
  const realReceive = unit.receive.bind(unit);
  let droppedCommit = false;
  unit.receive = (m) => { const a = realReceive(m); if (!droppedCommit && m[4] === D.CMD.commit) { droppedCommit = true; return []; } return a; };
  const out = await p;
  unit.receive = realReceive;
  const commits = b.sent.slice(before).filter((m) => m[1] === 0x7D && m[4] === D.CMD.commit);
  check("commit's answer lost: commit never sent again", droppedCommit && commits.length === 1 && out.apply.commitLost);
  check("cancelled from the page while it asked: nothing changed, the files still wait", out.stage === "ended" && out.apply.outcome === 4 && /stay on your OP-1/.test(out.words) && unit.pkin.has("extra.pack"), out.words);
  // a refusal: nothing to apply
  unit.pkin.clear();
  const ap = await client.applyPacks({});
  check("a refusal said in words, nothing changed", ap.refused && /nothing to change/.test(ap.words));
  // no door: the unit answers the identity request only
  const quiet = new UnitModel({}, { noDoor: true });
  const c2 = await new D.DoorClient(bench(quiet).io).connect();
  check("no door: told from no unit by the identity request", c2.kind === "no door");
  const none = new UnitModel({}, { noUnit: true });
  const c3 = await new D.DoorClient(bench(none).io).connect();
  check("no unit: nothing answers", c3.kind === "no unit");
  // a door of another firmware: the client says which
  const other = new UnitModel({}, { osId: 0xD67577F9 });
  const c4 = await new D.DoorClient(bench(other).io).connect();
  check("a door for OS 1.7.0: the client says so", c4.kind === "door" && D.firmwareOf(c4.hello.osId) === "1.7.0");
}

// ---------- 7. the record in the browser's storage, one a firmware ----------

{
  const store = new Map();
  const storage = { getItem: (k) => (store.has(k) ? store.get(k) : null), setItem: (k, v) => store.set(k, v), removeItem: (k) => store.delete(k) };
  const rec = D.makeRecord({ firmware: "1.7.3", sent: [{ name: "lfo.pack", size: 3, sha256: "AB".repeat(32) }], removed: [{ name: "wink.pack", size: 1 }] });
  check("the record saved under a key that names its firmware", D.saveRecord(storage, "1.7.3", rec) && store.has("open-field-change-1.7.3"));
  check("the record read back for its firmware only", D.loadRecord(storage, "1.7.3") !== null && D.loadRecord(storage, "1.7.0") === null);
  check("a pending record before commit and while it restarts", D.recordPending(rec) && D.recordPending(Object.assign({}, rec, { phase: "restarting" })) && !D.recordPending(Object.assign({}, rec, { phase: "checked" })));
  const broken = { getItem() { throw new Error("no storage"); }, setItem() { throw new Error("no storage"); }, removeItem() { throw new Error("no storage"); } };
  check("a browser that keeps nothing: no throw", D.saveRecord(broken, "1.7.3", rec) === false && D.loadRecord(broken, "1.7.3") === null && D.forgetRecord(broken, "1.7.3") === false);
  const v = D.holdToRecord(rec, { entries: [{ name: "lfo.pack", size: 3, isPack: true }, { name: "wink.pack", size: 1, isPack: true }] }, [{ name: "lfo.pack", sha256: "ab".repeat(32) }]);
  check("held to the record: lfo installed, wink still there", v.files[0].verdict === "installed" && v.removed[0].verdict === "still there" && !v.allGood);
  const steps = [];
  const done = D.letGoOfMidi({ keepRecord: () => steps.push("record"), releasePorts: () => steps.push("ports"), dropAccess: () => steps.push("access"), navigate: (u) => steps.push(u) }, "power");
  check("letting go: the record, the ports, the access, then after.html", steps.join(",") === "record,ports,access,after.html?for=power" && done.length === 4);
}

// ---------- 8. list with each file's header (a door of build 5, base 25: the protocol's section 14) ----------

{
  const N1_HEADERS = ["F0 7D 4F 46 21 01 00 00 00 02 F7", "F0 7D 4F 46 61 01 00 60 08 00 00 08 01 1C 04 00 01 00 0D 38 20 74 72 00 61 63 6B 73 2E 70 61 40 63 6B 07 00 00 00 51 02 34 6B 1A 08 38 20 74 40 72 61 63 6B 73 01 64 01 23 01 00 09 62 61 73 00 65 2E 70 61 63 6B 11 28 00 00 00 51 34 6B 1A 00 04 62 61 73 65 01 2A 01 03 06 00 0D 66 78 20 00 74 72 61 69 6E 2E 70 00 61 63 6B 0A 00 00 00 05 51 34 6B 1A 08 66 78 00 20 74 72 61 69 6E 01 02 7E 3A 05 00 08 6C 66 00 6F 2E 70 61 63 6B 08 28 00 00 00 51 34 6B 1A 00 03 6C 66 6F 01 71 1C 00 00 00 09 6C 69 73 74 00 2E 70 61 63 6B 01 00 14 00 00 51 34 6B 1A 04 40 6C 69 73 74 01 7E 16 00 01 00 0E 73 69 64 65 00 63 68 61 69 6E 2E 70 00 61 63 6B 07 00 00 00 05 51 34 6B 1A 09 73 69 00 64 65 63 68 61 69 6E 00 01 52 08 00 00 09 77 00 69 6E 6B 2E 70 61 63 20 6B 01 00 00 00 51 34 01 6B 1A 04 77 69 6E 6B 00 01 1D 11 00 00 09 79 00 6F 79 6F 2E 70 61 63 20 6B 01 00 00 00 51 34 01 6B 1A 04 79 6F 79 6F F7"];
  const KINDS = ["F0 7D 4F 46 21 03 00 00 00 02 F7", "F0 7D 4F 46 61 03 00 00 05 00 00 05 01 64 00 00 00 00 08 62 61 64 2E 70 70 61 63 6B 7F 7F 7F 01 7F 00 00 00 00 00 01 03 64 23 01 00 09 62 61 00 73 65 2E 70 61 63 6B 50 11 00 00 00 51 34 6B 00 1A 04 62 61 73 65 00 00 0C 00 00 00 09 6E 6F 00 74 65 73 2E 74 78 74 0F 7F 7F 7F 7F 00 00 00 00 00 00 01 28 00 00 00 00 0A 73 68 6F 72 74 2E 70 70 61 63 6B 7F 7F 7F 01 7F 00 00 00 00 00 01 00 52 08 00 00 27 77 69 00 6E 6B 2D 77 69 74 68 00 2D 61 2D 6E 61 6D 65 00 2D 6F 66 2D 74 68 69 00 72 74 79 2D 6E 69 6E 00 65 2D 62 79 2E 70 61 40 63 6B 01 00 00 00 51 02 34 6B 1A 04 77 69 6E 00 6B F7"];
  check("list with the header: the client builds the message byte for byte", same(D.buildMessage(D.CMD.list, 1, D.payloads.list(0, 0, false, true)), hex(N1_HEADERS[0])));
  check("list with the header: the message for the kinds of entry", same(D.buildMessage(D.CMD.list, 3, D.payloads.list(0, 0, false, true)), hex(KINDS[0])));
  check("list without the header: the bytes as before", same(D.payloads.list(0, 1, true), [0, 1, 1]) && same(D.payloads.list(0), [0]) && same(D.payloads.list(0, 2), [0, 2]));
  const a = D.parseAnswer(hex(N1_HEADERS[1]));
  const L = D.parseList(a.payload, { header: true });
  check("N1's eight with their headers: names, versions, the firmware, the pack names",
    L.total === 8 && L.count === 8 && JSON.stringify(L.entries.map((e) => e.version)) === JSON.stringify([7, 17, 10, 8, 1, 7, 1, 1]) &&
    L.entries.every((e) => e.headerRead && e.osId === 0x1AEB34D1) && L.entries[0].packName === "8 tracks" && L.entries[2].packName === "fx train" && L.entries[1].packName === "base",
    JSON.stringify(L.entries.map((e) => [e.name, e.version, e.packName])));
  const K = D.parseList(D.parseAnswer(hex(KINDS[1])).payload, { header: true });
  const unread = K.entries.filter((e) => !e.headerRead);
  check("every kind of entry: the unreadable ones carry no version, no firmware, no name", unread.length >= 1 && unread.every((e) => e.version === null && e.osId === null && e.packName === null), JSON.stringify(K.entries.map((e) => [e.name, e.version, e.packName])));
  check("every kind of entry: a readable one still reads", K.entries.some((e) => e.headerRead && e.packName === "base"));
  // the client asks for the header only from a door of build 5 or later
  const c = new D.DoorClient({ send() {}, sleep: async () => {}, now: () => 0, stopped: () => false });
  c.protocol = 3; c.hello = { build: 4 };
  check("a door of build 4: no header asked", c.canHeader() === false);
  c.hello = { build: 5 };
  check("a door of build 5: the header asked", c.canHeader() === true);
  c.protocol = 2;
  check("a door of protocol 2: no header asked", c.canHeader() === false);
  // the page's rows take the header's version for a file it does not know by fingerprint
  const files = P.unitFiles({ entries: L.entries.map((e) => Object.assign({}, e)) }, []);
  check("the rows' files carry the header's version and name", files[3].headerVersion === 8 && files[3].headerName === "lfo" && files[3].headerOsId === 0x1AEB34D1);
}

// ---------- 9. the review of base 25: the words of the refusal after another program's MIDI (status 13 with 03) ----------

{
  const r = D.commitRefusal(D.STATUS.folderNotReady, [3]);
  check("status 13 03: the words name the other program and Bluetooth MIDI", r.words === D.OTHER_MIDI_WORDS && r.words.includes("Bluetooth MIDI") && r.words.endsWith("Nothing changed."), r.words);
  check("status 13 otherwise: not ready yet", D.commitRefusal(D.STATUS.folderNotReady, [1]).words.startsWith("Your OP-1 isn't ready yet"));
}

// ---------- 10. the page lets go of the OP-1's MIDI (the owner's fault of 2026-11-20, item 27) ----------
// After connect at the top, a restart into the bootloader and find my OP-1 got no answer, and a reload froze the
// browser: the page held the OP-1's ports while the unit went away. The page's own glue is read here: the ports are
// held only while the page talks, the access is dropped when the unit goes away while the page is idle, and
// everything is given back before find my OP-1 (step 1) and before the install steps open. And the page's port
// handling, lifted out of it, run against a model of the browser's MIDI: a unit that goes away while the page is
// idle leaves no port open and no access.

{
  const page = readFileSync(join(here, "index.html"), "utf8");
  const scripts = [...page.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/gi)].map((m) => m[1]);
  const live = scripts.find((s) => s.includes('import * as D from "./door.mjs";')) || "";
  const install = scripts.find((s) => s.includes('import { Session } from "./ferret.mjs";')) || "";
  const part = (src, head, end = "\n  }\n") => { const at = src.indexOf(head); if (at < 0) return ""; const rest = src.slice(at); const stop = rest.indexOf(end); return stop < 0 ? rest : rest.slice(0, stop); };
  const guarded = part(live, "  async function guarded(fn) {");
  check("every talk with the unit ends with its ports closed", /finally \{ running = false; if \(!leaving\) releasePorts\(\); refreshButtons\(\); \}/.test(guarded));
  const onState = part(live, "  function onStateChange(e) {");
  check("a port that goes away is closed at once", /if \(ours\) releasePorts\(\);/.test(onState));
  check("idle, the page drops its MIDI access when the OP-1 goes away", onState.indexOf("if (running || leaving") >= 0 && onState.indexOf("if (running || leaving") < onState.indexOf("dropAccess();"));
  const letGo = part(live, "  window.openFieldLetGoOfMidi = () => {", "\n  };\n");
  check("the packs section gives back its ports and its access when asked", ["releasePorts();", "dropAccess();", "forgetUnit();"].every((s) => letGo.includes(s)));
  const find = part(install, '  $("find").addEventListener("click", async () => {', "\n  });\n");
  check("find my OP-1 gives every port of the OP-1 back before it talks to the bootloader",
    find.indexOf("letGoOfEveryPort();") >= 0 && find.indexOf("letGoOfEveryPort();") < find.indexOf("session = bootSession();") && find.includes("finally { letGoOfBootloader(); talking = false; }"));
  const every = part(install, "  function letGoOfEveryPort() {");
  check("letting go of every port asks the packs section first", every.includes("if (window.openFieldLetGoOfMidi) window.openFieldLetGoOfMidi();"));
  check("opening the install steps lets go too", part(install, "  function openInstall(scroll = true) {").includes("window.openFieldLetGoOfMidi();"));
  const send = part(live, '  $("send").addEventListener("click", () => guarded(async () => {', "\n  }));\n");
  check("a change takes the ports again before it sends", send.indexOf("if (!takePorts()) throw") >= 0 && send.indexOf("if (!takePorts()) throw") < send.indexOf("await client.changePacks("));

  // The page's port functions, as the page has them, run against a model of the browser's MIDI.
  const fns = ["function closeQuietly(port) {", "function releasePorts() {", "function takePorts() {", "function dropAccess() {", "function forgetUnit() {", "function onStateChange(e) {"]
    .map((head) => { const at = live.indexOf("  " + head); if (at < 0) return ""; const rest = live.slice(at); const one = rest.indexOf("\n"); return rest.slice(0, one).trimEnd().endsWith("}") ? rest.slice(0, one) : rest.slice(0, rest.indexOf("\n  }\n") + 4); });
  check("the page's port functions are found", fns.every((f) => f.length > 0));
  const port = (id, name) => ({ id, name, state: "connected", connection: "closed", onmidimessage: null, close() { this.connection = "closed"; this.closes = (this.closes || 0) + 1; return Promise.resolve(); } });
  const ins = [port("in1", "OP-1 field")], outs = [port("out1", "OP-1 field")];
  const model = { inputs: new Map(ins.map((p) => [p.id, p])), outputs: new Map(outs.map((p) => [p.id, p])), onstatechange: null };
  const said = [];
  const env = {
    D, queue: { abort() {}, clear() {}, push() {} }, client: { hello: {} }, render() {}, setMoment(m) { said.push("moment " + m); },
    $: () => ({ set textContent(t) { said.push(t); } }), document: { body: { dataset: { moment: "ours" } } },
  };
  // eslint-disable-next-line no-new-func
  const make = new Function("env", "model", "with (env) { let access = model, input = null, output = null, running = false, leaving = false, unit = {}, rowsNow = {}, choices = {}, resolved = {};\n" +
    fns.join("\n") + "\n return { takePorts, releasePorts, onStateChange, held: () => ({ input, output, access }) }; }");
  try {
    const glue = make(env, model);
    model.onstatechange = glue.onStateChange;
    const took = glue.takePorts();
    check("connect takes the OP-1's two ports", took && glue.held().input === ins[0] && glue.held().output === outs[0] && typeof ins[0].onmidimessage === "function");
    glue.releasePorts();                                   // the end of the talk (guarded's finally)
    check("after the talk the page holds no port", glue.held().input === null && glue.held().output === null && ins[0].closes === 1 && outs[0].closes === 1 && ins[0].onmidimessage === null);
    ins[0].state = "disconnected";
    glue.onStateChange({ port: ins[0] });                 // the owner restarts the unit into its bootloader
    check("idle, the unit going away drops the page's MIDI access", glue.held().access === null && model.onstatechange === null);
    check("and the page says so and shows connect again", said.includes("moment connect") && said.some((t) => /went away/.test(String(t))));
  } catch (e) { check("the page's port functions run against the model", false, e.message); }
}

// ---------- 11. save logs: the unit's log files of its last three starts, in one .zip (a door of build 6) ----------
// logs (0x2B) names the files newest first, eight a page; logread (0x2C) gives a file's bytes from an offset. The link
// reads the first 128 bytes of each file from the newest back (a file whose first line is the log's own "log file logs/"
// line carries on the start before it), stops at three starts or twelve files, reads those whole in 2,048-byte pieces
// and makes one stored .zip of them, read back here.

{
  const logName = (i) => "log_2026-11-" + String(24 - Math.floor(i / 4)).padStart(2, "0") + "_" + String(12 - (i % 4)).padStart(2, "0") + ".32.13_0.txt";
  const bodyOf = (size, seed) => { const d = new Uint8Array(size); for (let i = 0; i < size; i++) d[i] = (i * 31 + seed * 7) & 255; return d; };
  // a file that carries on a start: its first line names it, as the unit's own line does; any other file begins a start
  const carriesOn = (name, size, seed) => { const d = bodyOf(size, seed); d.set(bytesOf("[ 0.000001] I disk_log[89]: log file logs/" + name + "\n")); return d; };
  const begins = (size, seed) => { const d = bodyOf(size, seed); d.set(bytesOf("[ 0.000001] I start\n")); return d; };
  const logUnit = (logs, build = 6) => new UnitModel({ "base.pack": pack3("b", 3000, 1) }, { build, logs });
  const readZip = (z) => {
    const v = new DataView(z.buffer, z.byteOffset, z.byteLength);
    const e = z.length - 22;
    if (v.getUint32(e, true) !== 0x06054B50) return { error: "no end record" };
    const n = v.getUint16(e + 10, true), cdSize = v.getUint32(e + 12, true), cdAt = v.getUint32(e + 16, true);
    if (v.getUint16(e + 8, true) !== n || cdAt + cdSize !== e) return { error: "the end record's counts" };
    const files = [];
    for (let k = 0, o = cdAt; k < n; k++) {
      if (v.getUint32(o, true) !== 0x02014B50) return { error: "central entry " + k };
      const c = { method: v.getUint16(o + 10, true), time: v.getUint16(o + 12, true), date: v.getUint16(o + 14, true), crc: v.getUint32(o + 16, true), packed: v.getUint32(o + 20, true), size: v.getUint32(o + 24, true), nameLen: v.getUint16(o + 28, true), at: v.getUint32(o + 42, true) };
      c.name = String.fromCharCode(...z.subarray(o + 46, o + 46 + c.nameLen));
      o += 46 + c.nameLen + v.getUint16(o + 30, true) + v.getUint16(o + 32, true);
      const L = c.at;
      if (v.getUint32(L, true) !== 0x04034B50) return { error: "local header " + k };
      const ln = v.getUint16(L + 26, true), lx = v.getUint16(L + 28, true);
      const local = { method: v.getUint16(L + 8, true), time: v.getUint16(L + 10, true), date: v.getUint16(L + 12, true), crc: v.getUint32(L + 14, true), packed: v.getUint32(L + 18, true), size: v.getUint32(L + 22, true), name: String.fromCharCode(...z.subarray(L + 30, L + 30 + ln)) };
      c.local = local;
      c.data = z.subarray(L + 30 + ln + lx, L + 30 + ln + lx + local.packed);
      files.push(c);
    }
    return { files, error: null };
  };
  // the CRC-32 by another hand: bit by bit, no table
  const crcBits = (bytes) => { let c = ~0; for (const b of bytes) { c ^= b; for (let k = 0; k < 8; k++) c = (c >>> 1) ^ (0xEDB88320 & -(c & 1)); } return (~c) >>> 0; };

  // the messages, byte for byte
  check("logs: the message to the unit, byte for byte", same(D.buildMessage(D.CMD.logs, 1, D.payloads.logs(8)), hex("F0 7D 4F 46 2B 01 00 08 F7")));
  const lr = D.buildMessage(D.CMD.logread, 2, D.payloads.logread(4096, 904, "log_2026-11-24_12.32.13_0.txt"));
  check("logread: the offset, the bytes wanted, then the name", lr[4] === 0x2C && same(D.unpack(lr.subarray(6, lr.length - 1)).bytes, [0x00, 0x10, 0, 0, 0x88, 0x03, ...bytesOf("log_2026-11-24_12.32.13_0.txt")]));
  const ok = (bytes) => D.checkMessage(bytes, context3) === null;
  check("the check lets logs and logread through", ok(D.buildMessage(D.CMD.logs, 1, [0])) && ok(lr) && ok(D.buildMessage(D.CMD.logread, 1, D.payloads.logread(0, 2048, "a"))));
  check("the check refuses logs with another payload, and logread with no name or more than 2,048 bytes",
    !ok(D.buildMessage(D.CMD.logs, 1, [])) && !ok(D.buildMessage(D.CMD.logs, 1, [0, 1])) && !ok(D.buildMessage(D.CMD.logread, 1, [0, 0, 0, 0, 0, 8])) &&
    !ok(D.buildMessage(D.CMD.logread, 1, [0, 0, 0, 0, 0x01, 0x08, 0x61])) && !ok(D.buildMessage(D.CMD.logread, 1, [0, 0, 0, 0, 0, 0, 0x61])));
  check("a log name with a folder or a dot first is refused before anything is built",
    ["../x.txt", "logs/x.txt", ".x", "a\\b", "", "x".repeat(65)].every((n) => { try { D.payloads.logread(0, 1, n); return false; } catch (e) { return e.kind === "name"; } }));
  check("the start rule reads the first line only", D.logContinues(bytesOf("[ 1.0] I disk_log[89]: log file logs/log_x.txt\nmore")) && !D.logContinues(bytesOf("[ 1.0] I start\n[ 1.1] I disk_log[89]: log file logs/log_x.txt\n")) && !D.logContinues(new Uint8Array(0)));
  check("the CRC-32 of 123456789 is CBF43926", D.crc32(bytesOf("123456789")) === 0xCBF43926 && crcBits(bytesOf("123456789")) === 0xCBF43926);

  // eleven files over two pages, newest first: the newest start is two files (0 carries on, 1 begins it), 2 begins the
  // next, and 3, empty, begins the third (no first line names it): three starts, and files 4 to 10 never read
  const logs = [
    { name: logName(0), data: carriesOn(logName(0), 5000, 0) },
    { name: logName(1), data: begins(300, 1) },
    { name: logName(2), data: begins(2048, 2) },
    { name: logName(3), data: new Uint8Array(0) },
    { name: logName(4), data: carriesOn(logName(4), 700, 4) },
    { name: logName(5), data: begins(4100, 5) },
    ...Array.from({ length: 5 }, (_, k) => ({ name: logName(6 + k), data: begins(100, 6 + k) })),
  ];
  const unit = logUnit(logs);
  unit.logsNotReady = 2;                                                // just after a start: "not ready" twice, then the answer
  const b = bench(unit);
  const client = new D.DoorClient(b.io);
  await client.connect();
  check("a door of build 6: the link's action is offered", client.canLogs() === true && client.hello.build === 6);
  const L = await client.listLogs();
  const logsSent = b.sent.filter((m) => m[1] === 0x7D && m[4] === D.CMD.logs).map((m) => D.unpack(m.subarray(6, m.length - 1)).bytes[0]);
  check("logs over two pages of eight, newest first, after 'not ready' twice", L.count === 11 && L.entries.length === 11 && L.entries.every((e, i) => e.name === logName(i) && e.size === logs[i].data.length) && unit.logsNotReady === 0 && same(logsSent, [0, 0, 0, 8]), JSON.stringify(logsSent));
  unit.logReads.length = 0;
  const got = await client.collectLogs();
  // from the newest back: 0 carries on, 1 begins (start 1 = 0, 1); 2 begins (start 2); 3 is empty and begins (start 3)
  check("the start rule: a start of two files, then two of one, and no file beyond the third start read",
    got.starts === 3 && got.files.length === 4 && got.files.map((f) => f.name).join() === [0, 1, 2, 3].map(logName).join() && !unit.logReads.some((r) => r.name === logName(4)),
    JSON.stringify(got.files.map((f) => f.name)));
  check("each file byte for byte as the unit has it", got.files.every((f, i) => same(f.data, logs[i].data) && !f.cut));
  const heads = unit.logReads.filter((r) => r.want === 128).map((r) => r.name);
  check("the heads read first, 128 bytes each, from the newest back", heads.join() === [0, 1, 2, 3].map(logName).join() && unit.logReads.slice(0, 4).every((r) => r.want === 128 && r.offset === 0));
  const first = unit.logReads.filter((r) => r.name === logName(0) && r.want !== 128).map((r) => r.offset + "/" + r.want);
  check("5,000 bytes in pieces of 2,048 with a short last one", first.join() === "0/2048,2048/2048,4096/904", first.join());
  const exact = unit.logReads.filter((r) => r.name === logName(2) && r.want !== 128).map((r) => r.offset + "/" + r.want);
  check("2,048 bytes in one piece, an empty file in one", exact.join() === "0/2048" && unit.logReads.filter((r) => r.name === logName(3) && r.want !== 128).length === 1, exact.join());
  check("the pace held while the logs were read", b.breaches.length === 0, b.breaches.join("; "));
  check("nothing sent but the door's reads", b.sent.every((m) => (m[0] === 0xF0 && m[1] === 0x7D && [0x20, 0x29, 0x2B, 0x2C].includes(m[4])) || same(m, D.IDENTITY_REQUEST)));

  // the .zip, read back
  const zip = D.logsZip(got.files);
  check("the .zip is named by the newest log's date and time", zip.name === "op1-field-logs-2026-11-24_12.32.13.zip", zip.name);
  const z = readZip(zip.bytes);
  check("the .zip reads back: an end record, a central directory and a local header for each file", z.error === null && z.files.length === 4, z.error);
  if (!z.error) {
    const wanted = [...got.files].reverse();
    check("the .zip holds the files oldest first, named as on the unit", z.files.map((f) => f.name).join() === wanted.map((f) => f.name).join() && z.files.every((f) => f.local.name === f.name));
    check("every file stored (no compression), its sizes and data as read",
      z.files.every((f, i) => f.method === 0 && f.local.method === 0 && f.size === wanted[i].data.length && f.packed === f.size && f.local.size === f.size && same(f.data, wanted[i].data)));
    check("every file's CRC-32 in both headers is its data's", z.files.every((f) => f.crc === crcBits(f.data) && f.local.crc === f.crc));
    const t = z.files[0];
    check("every file dated with the newest log's time, 2026-11-24 12:32:12 (two-second steps)",
      z.files.every((f) => f.date === t.date && f.time === t.time && f.local.date === t.date && f.local.time === t.time) &&
      (t.date >> 9) + 1980 === 2026 && ((t.date >> 5) & 15) === 11 && (t.date & 31) === 24 && (t.time >> 11) === 12 && ((t.time >> 5) & 63) === 32 && (t.time & 31) * 2 === 12);
  }

  // the 12-file cap: every file carries on, so no start is whole
  const many = Array.from({ length: 14 }, (_, i) => ({ name: logName(i), data: carriesOn(logName(i), 200 + i, i) }));
  const u2 = logUnit(many), b2 = bench(u2), c2 = new D.DoorClient(b2.io);
  await c2.connect();
  const g2 = await c2.collectLogs();
  check("at twelve files it stops: twelve listed, twelve read, the thirteenth never asked for",
    g2.files.length === 12 && g2.starts === 0 && !u2.logReads.some((r) => r.name === logName(12) || r.name === logName(13)) && g2.files.every((f, i) => same(f.data, many[i].data)));

  // a lost answer to logread: status, then the same piece again
  const u3 = logUnit([{ name: logName(0), data: begins(3000, 1) }]), b3 = bench(u3), c3 = new D.DoorClient(b3.io);
  await c3.connect();
  u3.drop.add(c3.number + 2);                                           // logs, the head, then the first whole piece: lost once
  const g3 = await c3.collectLogs();
  const tail = b3.sent.filter((m) => m[1] === 0x7D).map((m) => m[4]);
  check("a lost logread answer: status after two seconds, then the piece again, the file whole", g3.files.length === 1 && same(g3.files[0].data, u3.logs[0].data) && tail.includes(D.CMD.status) && b3.breaches.length === 0);

  // a door of build 5: no logs, nothing asked
  const u4 = logUnit(logs, 5), b4 = bench(u4), c4 = new D.DoorClient(b4.io);
  await c4.connect();
  let refused = null;
  try { await c4.collectLogs(); } catch (e) { refused = e; }
  check("a door of build 5: the link's action is not offered, and nothing is asked", c4.canLogs() === false && refused instanceof D.DoorError && refused.kind === "protocol" && !b4.sent.some((m) => m[1] === 0x7D && (m[4] === D.CMD.logs || m[4] === D.CMD.logread)));

  // 14 while the door applies, 2 for a refused name, 13 to the end
  const u5 = logUnit(logs), b5 = bench(u5), c5 = new D.DoorClient(b5.io);
  await c5.connect();
  u5.state = 6;
  let e5 = null;
  try { await c5.collectLogs(); } catch (e) { e5 = e; }
  check("while the door applies (14): the logs stop with 'applying'", e5 && e5.kind === "applying");
  u5.state = 0;
  let e6 = null, e8 = null;
  try { await c5.readLog("notes.txt"); } catch (e) { e6 = e; }
  check("a name the unit refuses (2): stopped with 'name'", e6 && e6.kind === "name");
  try { await c5.readLog("log_gone.txt"); } catch (e) { e8 = e; }
  check("a log no longer there (11 with its 8 bytes): stopped with 'unreadable'", e8 && e8.kind === "unreadable");
  u5.logsNotReady = 1000;
  let e7 = null;
  try { await c5.listLogs(); } catch (e) { e7 = e; }
  check("'not ready' to the end: asked again 40 times, then stopped with 'not ready'", e7 && e7.kind === "not ready" && u5.logsNotReady === 1000 - 41, e7 && e7.message);
  check("no logs: an empty list and no file", (await (async () => { const u = logUnit([]), c = new D.DoorClient(bench(u).io); await c.connect(); const g = await c.collectLogs(); return g.files.length === 0 && g.count === 0; })()));

  // the page: the link after disconnect in section 1's header, hidden until a door of build 6 says hello, one talk at a time
  const page = readFileSync(join(here, "index.html"), "utf8");
  check("the link sits right after disconnect, in its style, hidden at first",
    /<button type="button" class="linkish" id="disconnect">disconnect<\/button><button type="button" class="linkish hidden" id="saveLogs"[^>]*>save logs<\/button><\/span>/.test(page));
  const scripts = [...page.matchAll(/<script\b[^>]*>([\s\S]*?)<\/script>/gi)].map((m) => m[1]);
  const live = scripts.find((s) => s.includes('import * as D from "./door.mjs";')) || "";
  const part = (head, end = "\n  }\n") => { const at = live.indexOf(head); if (at < 0) return ""; const rest = live.slice(at); const stop = rest.indexOf(end); return stop < 0 ? rest : rest.slice(0, stop + end.length); };
  const handler = part('  $("saveLogs").addEventListener("click", () => guarded(async () => {', "\n  }));\n");
  check("the link's press is one guarded talk, which takes the ports before it reads", handler.length > 0 && handler.indexOf("if (!takePorts()) throw") < handler.indexOf("await client.collectLogs(") && handler.includes("D.logsZip(got.files)"));
  const fns = [part("  function logsLink() {"), part("  function logsSay(words, ms) {")];
  const btn = { hidden: true, disabled: false, textContent: "", classList: { toggle(c, on) { if (c === "hidden") btn.hidden = on; } } };
  const timers = [];
  const env = { $: () => btn, client: { canLogs: () => false }, running: false, setTimeout: (fn) => { timers.push(fn); return timers.length; }, clearTimeout: () => {} };
  try {
    // eslint-disable-next-line no-new-func
    const glue = new Function("env", "with (env) { let logsWords = \"\", logsTimer = null;\n" + fns.join("\n") + "\n return { logsLink, logsSay }; }")(env);
    glue.logsLink();
    check("a door of build 5 or none: the link stays hidden", btn.hidden === true && btn.textContent === "save logs");
    env.client = { canLogs: () => true };
    glue.logsLink();
    check("a door of build 6: the link shows, 'save logs'", btn.hidden === false && btn.disabled === false && btn.textContent === "save logs");
    env.running = true;
    glue.logsSay("saving logs 3 of 5");
    check("while it works: 'saving logs 3 of 5', and it can't be pressed", btn.textContent === "saving logs 3 of 5" && btn.disabled === true);
    env.running = false;
    glue.logsSay("saved", 4000);
    check("at the end 'saved', then 'save logs' again a few seconds later", btn.textContent === "saved" && btn.disabled === false && timers.length === 1 && (timers[0](), btn.textContent === "save logs"));
  } catch (e) { check("the page's link functions run against a model", false, e.message); }
  const refresh = part("  function refreshButtons() {");
  check("every start and end of a talk sets the link (refreshButtons)", refresh.includes("logsLink();"));
}

console.log((failed ? "FAILED: " : "OK: ") + passed + " passed, " + failed + " failed");
process.exitCode = failed ? 1 : 0;
