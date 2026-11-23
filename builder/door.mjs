// open-field: the install page's client for the door, the small part of base.pack that answers this page over USB
// MIDI while the OP-1 field plays as usual. Through it the page lists the pack files on the unit, reads their SHA-256,
// sends pack files into a waiting folder on the unit, marks packs for removal, and asks the unit to apply the change.
// The unit asks its owner on its own screen and changes nothing until T1 is pressed there; then it restarts by itself.
//
// Runs in the browser and in Node (no browser object and no Node API in this file), so that validate-door.mjs and the
// tests run the same code as the page.
//
// What this client may ever send, all of it held by checkMessage(), which every send goes through:
// - the door's messages F0 7D 4F 46 <command> <number> <packed payload> F7, command and number below 0x40, every byte
//   between F0 and F7 below 0x80, never longer than the unit's largest message (64 bytes before the unit has said hello):
//   hello, list, hash, status and abort to any door; begin, data, end, remove and commit only to a door that said it
//   can apply a change (protocol 3), and commit only with its payload 01 (the apply);
// - the standard identity request F0 7E 7F 06 01 F7, which tells a unit with no door from no unit at all.
// It never sends a control change or a note, and never a message that begins F0 00 20 76: that prefix is the unit
// maker's own service, which this page leaves alone. The check refuses such a message before anything else.
// The pace: one message, its answer, then the next; never a second message before the answer to the first has come or
// two seconds have passed.

export const PAGE_PROTOCOL = 3;                     // the door's protocol this page speaks: 3, the one that can apply
export const HEADER = Object.freeze([0xF0, 0x7D, 0x4F, 0x46]);
export const IDENTITY_REQUEST = Object.freeze([0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7]);
const REFUSED_PREFIX = Object.freeze([0xF0, 0x00, 0x20, 0x76]); // the only place these bytes are written: a prefix that is refused, never built
export const MAX_BEFORE_HELLO = 64;                 // the largest message before the unit has said what it takes
export const PAGE_MAX_MESSAGE = 65536;              // this page's own cap on a message, whatever the unit says it takes
export const ANSWER_WAIT_MS = 2000;                 // no answer within this: the page asks where the unit stands
export const ASK_PACE_MS = 250;                     // while the unit asks or applies, one status every quarter second
export const SILENT_STATUS_LIMIT = 3;               // three status messages in a row with no answer: the unit does not answer
export const NOT_READY_TRIES = 10;                  // "not ready" to begin or abort 01: the same message again a second later, at most this many times
export const MAX_TRIES = 3;                         // sends of one message when status answers and the message does not
// Just after a start the door answers "not ready" to list, hash and remove: while the unit looks for a change that was
// cut short, and from base 31 until four seconds after the start's first screen is drawn (some six seconds after the
// door first answers, on a unit with many packs). Those three act the same twice, so they are asked again every 250 ms, at most this
// many times.
export const START_WAIT_TRIES = 40;
export const AFTER_PAGE = "after.html";             // beside index.html: the page that holds no MIDI while the unit restarts
export const RETURN_SLOW_MS = 20000;                // back on the page: no answer within this, and the page says to quit the browser and open it again
export const RETURN_GIVE_UP_MS = 120000;            // the page stops looking for the unit after this (the owner can look again)
export const RECORD_KEY = "open-field-change-";     // the record of a change in the browser's storage, followed by the firmware: one record a firmware

export const CMD = Object.freeze({ hello: 0x20, list: 0x21, hash: 0x22, begin: 0x23, data: 0x24, end: 0x25, remove: 0x26, abort: 0x27, commit: 0x28, status: 0x29 });
const SENDABLE = Object.freeze([CMD.hello, CMD.list, CMD.hash, CMD.begin, CMD.data, CMD.end, CMD.remove, CMD.abort, CMD.commit, CMD.status]);
const WRITES = Object.freeze([CMD.begin, CMD.data, CMD.end, CMD.remove, CMD.commit]); // sent only to a door of protocol 3
const ASKED_AGAIN_ON_13 = Object.freeze([CMD.list, CMD.hash, CMD.remove]);
export const COMMAND_NAMES = Object.freeze({ 0x20: "hello", 0x21: "list", 0x22: "hash", 0x23: "begin", 0x24: "data", 0x25: "end", 0x26: "remove", 0x27: "abort", 0x28: "commit", 0x29: "status" });

export const STATUS = Object.freeze({ done: 0, unreadable: 1, nameRefused: 2, noRoom: 3, noSuchCommand: 4, outOfOrder: 5, fingerprintDiffers: 6, asking: 7, cancelled: 8, more: 9, restarting: 10, fileUnreadable: 11, tapeMoves: 12, folderNotReady: 13, applying: 14, loaderRefuses: 15 });
const STATUS_NAMES = Object.freeze(["done", "arguments unreadable", "name refused", "no room", "no such command", "out of order", "fingerprint differs",
  "busy asking", "cancelled", "more to come", "restarting", "file unreadable", "the tape moves", "not ready", "changing its packs", "a file the unit would not use"]);
// The firmware a door is for, by the id its hello gives: the first four bytes of the original firmware file's SHA-256.
export const OS_IDS = Object.freeze({ 0x1AEB34D1: "1.7.3", 0xD67577F9: "1.7.0" });

export class DoorError extends Error {
  constructor(kind, message) { super(message); this.kind = kind; }
}

export function statusWords(s) { return (STATUS_NAMES[s] ?? "status") + " (" + s + ")"; }
export function firmwareOf(osId) { return OS_IDS[osId] ?? null; }
export function hex32(n) { return "0x" + (n >>> 0).toString(16).toUpperCase().padStart(8, "0"); }
export function toHex(bytes) { return Array.from(bytes, (b) => b.toString(16).toUpperCase().padStart(2, "0")).join(" "); }
export function digestHex(bytes) { return Array.from(bytes, (b) => b.toString(16).padStart(2, "0")).join(""); }
export function hexBytes(text) {
  const t = String(text).toLowerCase();
  if (!/^[0-9a-f]{64}$/.test(t)) throw new Error("a SHA-256 is 64 hex digits");
  return Uint8Array.from({ length: 32 }, (_, i) => parseInt(t.slice(2 * i, 2 * i + 2), 16));
}
// Only a MIDI port whose name holds "OP-1" is ever opened, listened to or sent to.
export function isOp1PortName(name) { return typeof name === "string" && name.toLowerCase().includes("op-1"); }
export function isIdentityReply(data) {
  return !!data && data.length >= 6 && data[0] === 0xF0 && data[1] === 0x7E && data[3] === 0x06 && data[4] === 0x02 && data[data.length - 1] === 0xF7;
}

function startsWith(bytes, prefix) {
  if (bytes.length < prefix.length) return false;
  for (let i = 0; i < prefix.length; i++) if (bytes[i] !== prefix[i]) return false;
  return true;
}
function sameBytes(a, b) { return a.length === b.length && startsWith(a, b); }

// ---------- the packing: seven bytes into eight, a head byte first holding their top bits (bit 0 the first byte's) ----------

export function packedLength(n) { return n + Math.ceil(n / 7); }
export function messageLength(payloadBytes) { return 7 + packedLength(payloadBytes); }

export function pack(payload) {
  const n = payload.length;
  const out = new Uint8Array(packedLength(n));
  let o = 0;
  for (let g = 0; g < n; g += 7) {
    const len = Math.min(7, n - g);
    let head = 0;
    for (let i = 0; i < len; i++) {
      const b = payload[g + i];
      if (!Number.isInteger(b) || b < 0 || b > 0xFF) throw new Error("not a byte at index " + (g + i));
      if (b & 0x80) head |= 1 << i;
    }
    out[o++] = head;
    for (let i = 0; i < len; i++) out[o++] = payload[g + i] & 0x7F;
  }
  return out;
}

// { bytes, error }: error is null, or why the packing is broken (bytes then holds what was unpacked before the fault).
export function unpack(packed) {
  const L = packed.length;
  const out = new Uint8Array(Math.max(0, L - Math.ceil(L / 8)));
  let o = 0, i = 0;
  while (i < L) {
    const head = packed[i];
    if (head & 0x80) return { bytes: out.slice(0, o), error: "a head byte with bit 7 set at " + i };
    const len = Math.min(7, L - i - 1);
    if (len === 0) return { bytes: out.slice(0, o), error: "the last group is a head alone" };
    if (head >> len) return { bytes: out.slice(0, o), error: "a head with a bit for a byte its group does not have, at " + i };
    for (let k = 0; k < len; k++) {
      const b = packed[i + 1 + k];
      if (b & 0x80) return { bytes: out.slice(0, o), error: "a byte with bit 7 set at " + (i + 1 + k) };
      out[o++] = b | (((head >> k) & 1) << 7);
    }
    i += 1 + len;
  }
  return { bytes: out.slice(0, o), error: null };
}

// ---------- numbers, little-endian ----------

function u32(v) { return [v & 0xFF, (v >>> 8) & 0xFF, (v >>> 16) & 0xFF, (v >>> 24) & 0xFF]; }
function rd16(b, o) { return b[o] | (b[o + 1] << 8); }
function rd32(b, o) { return (b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24)) >>> 0; }

// ---------- names: the ones the loader takes as a pack file's ----------

const NAME_FORBIDDEN = "/\\:*?\"<>|";
// null when the loader takes `name` as a pack file's name, else why not: 6 to 39 printable bytes ending in .pack (any
// capitals), not beginning with a dot or a space, none of / \ : * ? " < > |.
export function checkName(name) {
  if (typeof name !== "string") return "not a text";
  if (name.length < 6) return "shorter than 6 bytes";
  if (name.length > 39) return "longer than 39 bytes";
  for (let i = 0; i < name.length; i++) {
    const c = name.charCodeAt(i);
    if (c < 0x20 || c > 0x7E) return "a character outside 0x20 to 0x7E at " + i;
    if (NAME_FORBIDDEN.includes(name[i])) return "the character " + name[i] + " at " + i;
  }
  if (name.slice(-5).toLowerCase() !== ".pack") return "does not end in .pack";
  if (name[0] === "." || name[0] === " ") return "begins with a dot or a space";
  return null;
}
function nameBytes(name) {
  const why = checkName(name);
  if (why) throw new DoorError("name", "the name " + JSON.stringify(name) + " is not a pack file's: " + why);
  return Array.from(name, (c) => c.charCodeAt(0));
}

// ---------- frames ----------

export function buildMessage(command, number, payload = []) {
  if (!Number.isInteger(command) || command < 0 || command > 0x3F) throw new Error("command out of range");
  if (!Number.isInteger(number) || number < 0 || number > 0x3F) throw new Error("number out of range");
  const p = pack(payload);
  const m = new Uint8Array(7 + p.length);
  m.set(HEADER, 0);
  m[4] = command;
  m[5] = number;
  m.set(p, 6);
  m[m.length - 1] = 0xF7;
  return m;
}

// null for a message that is not the door's (another device, an identity answer, a clock); else { command, number, status, payload, error }.
export function parseAnswer(bytes) {
  if (!bytes || bytes.length < 4 || !startsWith(bytes, HEADER)) return null;
  const n = bytes.length;
  if (n < 8) return { error: "an answer shorter than 8 bytes" };
  if (bytes[n - 1] !== 0xF7) return { error: "an answer that does not end with F7" };
  for (let i = 1; i < n - 1; i++) if (bytes[i] >= 0x80) return { error: "an answer with a byte of 0x80 or more inside" };
  if (bytes[4] < 0x60) return { error: "a message of the door's that is not an answer (command byte " + bytes[4] + ")" };
  const u = unpack(bytes.subarray(7, n - 1));
  return { command: bytes[4] - 0x40, number: bytes[5], status: bytes[6], payload: u.bytes, wireLength: n, error: u.error ? "the answer's packing is broken: " + u.error : null };
}

// The messages the page sends, each built from its payload (the client sends them through exchange()).
export const payloads = Object.freeze({
  hello: () => [PAGE_PROTOCOL],
  // the third byte is bits (protocol section 14.1): 01 protocol 3's flags, 02 each entry's header (a door of build 5 or later)
  list: (index, folder = 0, flags = false, header = false) => {
    const bits = (flags ? 1 : 0) | (header ? 2 : 0);
    return bits ? [index, folder, bits] : folder ? [index, folder] : [index];
  },
  hashStart: (name) => [0x00, ...nameBytes(name)],
  hashNext: (done) => [0x01, ...u32(done)],
  begin: (size, sha256, name) => {
    const n = nameBytes(name);
    if (!Number.isInteger(size) || size < 1 || size > 0xFFFFFFFF) throw new Error("size out of range");
    if (!sha256 || sha256.length !== 32) throw new Error("a SHA-256 is 32 bytes");
    return [...u32(size), ...Array.from(sha256), ...n];
  },
  data: (offset, bytes) => { const p = new Uint8Array(4 + bytes.length); p.set(u32(offset), 0); p.set(bytes, 4); return p; },
  endClose: () => [0],
  endNext: (done) => [1, ...u32(done)],
  removeNote: (name) => [0, ...nameBytes(name)],
  removeTakeBack: (name) => [1, ...nameBytes(name)],
  removeAll: () => [2],
  abort: () => [],
  abortAll: () => [1],
  apply: () => [1],
  status: (protocol) => (protocol >= 3 ? [3] : protocol >= 2 ? [2] : []),
});

function text(bytes) {
  let end = bytes.length;
  while (end > 0 && bytes[end - 1] === 0) end--;
  return String.fromCharCode(...bytes.subarray(0, end));
}

// hello's answer: 44 bytes (protocol 1), 60 (protocol 2) or 68 (protocol 3).
export function parseHello(p) {
  if (p.length !== 44 && p.length !== 60 && p.length !== 68) throw new DoorError("protocol", "hello's answer holds " + p.length + " bytes, not 44, 60 or 68");
  if (p.length === 60 && p[0] < 2) throw new DoorError("protocol", "hello's answer holds 60 bytes but says protocol " + p[0]);
  if (p.length === 68 && p[0] < 3) throw new DoorError("protocol", "hello's answer holds 68 bytes but says protocol " + p[0]);
  const h = {
    protocol: p[0], mark: p[1], baseVersion: rd16(p, 2), release: text(p.subarray(4, 20)), osId: rd32(p, 20),
    maxMessage: rd32(p, 24), maxOther: rd32(p, 32), slice: rd32(p, 36), listEntries: p[40], state: p[41],
  };
  if (p.length >= 60) Object.assign(h, { piece: rd32(p, 44), maxFile: rd32(p, 48), margin: rd32(p, 52), maxWaiting: p[56], waiting: p[57], build: p[58] });
  if (p.length === 68) Object.assign(h, { maxRemovals: p[60], removalsNoted: p[61], restartMs: rd16(p, 62), startFound: p[64], applyOutcome: p[65] });
  return h;
}

// list's answer. flags: the list was asked with its third byte 01, so an entry of the waiting folder carries two more bits.
export function parseList(p, options = {}) {
  if (p.length < 4) throw new DoorError("protocol", "list's answer holds " + p.length + " bytes, fewer than 4");
  const r = { total: p[0], beyond: p[1], first: p[2], count: p[3], entries: [] };
  let o = 4;
  for (let k = 0; k < r.count; k++) {
    if (o + 6 > p.length) throw new DoorError("protocol", "list's answer ends inside entry " + k);
    const flags = p[o], size = rd32(p, o + 1), L = p[o + 5];
    if (L < 1 || L > 39 || o + 6 + L > p.length) throw new DoorError("protocol", "list's entry " + k + " has a name length of " + L);
    const name = String.fromCharCode(...p.subarray(o + 6, o + 6 + L));
    const e = { flags, size, name, isPack: !!(flags & 1), sizeUnknown: !!(flags & 2), nameCut: !!(flags & 4), checked: !!(flags & 8), underWay: !!(flags & 16) };
    if (options.flags) Object.assign(e, { unitRefuses: !!(flags & 32), replaces: !!(flags & 64) });
    o += 6 + L;
    if (options.header) {
      // section 14.2: the pack's version, the firmware id it was made for, and its name, as its file's header says
      if (o + 9 > p.length) throw new DoorError("protocol", "list's entry " + k + " ends inside its header's fields");
      const version = rd32(p, o), osId = rd32(p, o + 4), N = p[o + 8];
      if (N > 16 || o + 9 + N > p.length) throw new DoorError("protocol", "list's entry " + k + " has a pack name of " + N + " bytes");
      const read = version !== 0xFFFFFFFF;
      Object.assign(e, { headerRead: read, version: read ? version : null, osId: read ? osId : null, packName: read ? String.fromCharCode(...p.subarray(o + 9, o + 9 + N)) : null });
      o += 9 + N;
    }
    r.entries.push(e);
  }
  if (o !== p.length) throw new DoorError("protocol", "list's answer holds " + (p.length - o) + " bytes after its entries");
  return r;
}

export function parseHash(status, p) {
  if (status === STATUS.fileUnreadable) {
    if (p.length !== 8) throw new DoorError("protocol", "hash's status 11 holds " + p.length + " bytes, not 8");
    return { size: rd32(p, 0), done: rd32(p, 4) };
  }
  const want = status === STATUS.done ? 48 : 16;
  if (p.length !== want) throw new DoorError("protocol", "hash's answer with status " + status + " holds " + p.length + " bytes, not " + want);
  const r = { size: rd32(p, 0), done: rd32(p, 4) };
  if (status === STATUS.done) r.digest = p.slice(16, 48);
  return r;
}

// status's answer: 36 bytes; with 02, 56 and the transfer's name; with 03, those and 20 bytes and the apply's name.
export function parseStatus(p) {
  const x = p.length >= 56 ? 56 + p[39] : -1;
  const two = x > 0 && p.length === x;
  const three = x > 0 && p.length >= x + 20 && p.length === x + 20 + p[x + 17];
  if (p.length !== 36 && !two && !three) throw new DoorError("protocol", "status's answer holds " + p.length + " bytes, not 36, 56 and a name, or 76 and two names");
  const s = { state: p[0], outcome: p[1], secondsLeft: rd16(p, 2), hashSize: rd32(p, 4), hashDone: rd32(p, 8) };
  if (p.length >= 56) Object.assign(s, {
    xState: p[36], xOutcome: p[37], waiting: p[38], xName: String.fromCharCode(...p.subarray(56, 56 + p[39])),
    xSize: rd32(p, 40), xReceived: rd32(p, 44), xChecked: rd32(p, 48), freeKb: rd32(p, 52),
  });
  if (three) Object.assign(s, {
    aState: p[x], aOutcome: p[x + 1], aToMove: p[x + 2], aCheckedFiles: p[x + 3], aRemovals: p[x + 4], aRemoved: p[x + 5], aMoved: p[x + 6],
    startFound: p[x + 7], aBytes: rd32(p, x + 8), aBytesChecked: rd32(p, x + 12), removalsNoted: p[x + 16], aName: String.fromCharCode(...p.subarray(x + 20, x + 20 + p[x + 17])),
  });
  return s;
}

export function parseAbort(p) {
  if (p.length !== 1 && p.length !== 2) throw new DoorError("protocol", "abort's answer holds " + p.length + " bytes, not 1 or 2");
  const f = p[0];
  return { hashEnded: !!(f & 1), pageClosed: !!(f & 2), transferEnded: !!(f & 4), filesRemoved: !!(f & 8), more: !!(f & 16), removed: p.length === 2 ? p[1] : 0 };
}
export function parseBegin(p) {
  if (p.length !== 8) throw new DoorError("protocol", "begin's answer holds " + p.length + " bytes, not 8");
  return { freeKb: rd32(p, 0), waiting: p[4], replaced: p[5] === 1 };
}
export function parseData(p) {
  if (p.length !== 12) throw new DoorError("protocol", "data's answer holds " + p.length + " bytes, not 12");
  return { received: rd32(p, 0) };
}
export function parseEnd(status, p) {
  if (status === STATUS.fingerprintDiffers) {
    const reason = p[0];
    if (reason === 1 && p.length === 41) return { reason, size: rd32(p, 1), done: rd32(p, 5), digest: p.slice(9, 41) };
    if ((reason === 2 || reason === 3) && p.length === 9) return { reason, size: rd32(p, 1), received: rd32(p, 5), done: rd32(p, 5) };
    throw new DoorError("protocol", "end's status 6 holds " + p.length + " bytes with reason " + reason);
  }
  const want = status === STATUS.done ? 48 : 16;
  if (p.length !== want) throw new DoorError("protocol", "end's answer with status " + status + " holds " + p.length + " bytes, not " + want);
  const r = { size: rd32(p, 0), done: rd32(p, 4) };
  if (status === STATUS.done) r.digest = p.slice(16, 48);
  return r;
}
export function parseRemove(p) {
  if (p.length !== 4) throw new DoorError("protocol", "remove's answer holds " + p.length + " bytes, not 4");
  return { noted: p[0], waiting: p[1], flag: p[2] };
}
export function parseCommit(p) {
  if (p.length !== 8) throw new DoorError("protocol", "commit 01's answer holds " + p.length + " bytes, not 8");
  return { newFiles: p[0], replacing: p[1], removing: p[2], baseComes: p[3] === 1, bytesToCheck: rd32(p, 4) };
}

// ---------- the words a musician reads ----------

const APPLY_OUTCOME_WORDS = Object.freeze([
  "Nothing happened yet.",
  "Done. Your OP-1 changed its packs and restarts by itself.",
  "You pressed T2 on your OP-1, so nothing changed.",
  "Nobody pressed T1 or T2 for a minute, so your OP-1 put the question away. Nothing changed.",
  "You cancelled from this page. Nothing changed.",
  "Your OP-1 couldn't show its question, so nothing changed.",
  "Your OP-1 checked the files once more and one of them didn't hold up, so it stopped before changing anything.",
  "Your OP-1 was being switched off, so it stopped before changing anything.",
  "The tape kept moving for ten seconds, so your OP-1 stopped before changing anything.",
  "Your OP-1 couldn't write its note, so it stopped before changing anything.",
  "Your OP-1 couldn't finish the change. Switch it off and on again and it finishes the change as it starts.",
  "Your OP-1 stopped part way and restarts. It finishes the change as it starts.",
]);
export function applyOutcomeWords(o, name) {
  const w = APPLY_OUTCOME_WORDS[o] ?? "Your OP-1 ended the change with a code this page doesn't know (" + o + ").";
  return name && (o === 6 || o === 10) ? w + " The file was " + name + "." : w;
}
// After a change that ended with nothing changed, the files stay in the waiting folder until the unit next starts.
export const STILL_WAITING_WORDS = "The files you sent stay on your OP-1, in a waiting folder of their own, until it next starts. Press the button again to have another go.";
const START_FOUND_WORDS = Object.freeze([
  "",
  "Your OP-1 found a change that had been cut short and finished it as it started.",
  "Your OP-1 found a change that had been cut short, but it was already whole.",
  "Your OP-1 found a note from another base and threw it away. Nothing changed.",
  "Your OP-1 found a note it couldn't read and set it aside. Nothing changed.",
  "Your OP-1 found a change that had been cut short and couldn't finish it.",
]);
export function startFoundWords(f) { return START_FOUND_WORDS[f] ?? ""; }
const UNIT_REFUSES_WORDS = Object.freeze({
  1: "it isn't a pack file your OP-1 can use",
  2: "it's made for another OS version than the one on your OP-1",
  3: "its size isn't the one written inside it, so the file is damaged or cut short",
  4: "its checksum doesn't hold, so the file is damaged",
  5: "it calls itself the base but isn't named base.pack",
  6: "it's named base.pack but isn't a base",
  7: "its card or its lists wouldn't load",
  8: "it clashes with the base, so the base would be left out at the start. Untick it, or bring the version that goes with the new base",
  9: "a folder of that name is in the way in your OP-1's packs folder",
});
export const SWITCH_OFF_AND_ON_WORDS = "Switch your OP-1 off and on, then try again. Nothing changed.";
// status 13 with 03 (the protocol's 13.8): since it started, the unit got a message of Teenage Engineering's own MIDI service,
// from another program or a Bluetooth MIDI device, and changes no pack until it restarts (the review of base 25, S-1)
export const OTHER_MIDI_WORDS = "Something else has talked to your OP-1 over MIDI since it started, another music program or a Bluetooth MIDI device, and after that it won't change its packs until it restarts. Close other music programs and disconnect Bluetooth MIDI, then switch your OP-1 off and on and try again. Nothing changed.";

// Why commit 01 was refused, in the owner's words: { status, words, reason?, name? }. Nothing changed in every case.
export function commitRefusal(status, payload) {
  const p = Uint8Array.from(payload);
  const r = { status, words: "" };
  const said = (w) => Object.assign(r, { words: w });
  switch (status) {
    case STATUS.loaderRefuses:
      Object.assign(r, { reason: p[0], name: String.fromCharCode(...p.subarray(1)) });
      return said("Your OP-1 won't use " + r.name + ", because " + (UNIT_REFUSES_WORDS[r.reason] ?? "of reason " + r.reason) + ". Nothing changed.");
    case STATUS.noRoom:
      if (p[0] === 6 && p.length === 2) { r.count = p[1]; return said("That would leave " + r.count + " pack files on your OP-1, and it takes 32 at most. Untick a few. Nothing changed."); }
      if (p[0] === 7 && p.length === 9) return said("Those packs together are too big for the room your OP-1 keeps for packs. Untick one or two. Nothing changed.");
      if (p[0] === 1 && p.length === 5) return said("Your OP-1's disk is too full for this (" + rd32(p, 1).toLocaleString("en-US") + " kB free). Free some space first. Nothing changed.");
      if (p[0] === 8) return said("Your OP-1's packs folder holds too many files. Take some off in disk mode first. Nothing changed.");
      return said("Your OP-1 says there's no room (reason " + p[0] + "). Nothing changed.");
    case STATUS.outOfOrder:
      if (p[0] === 3) return said("There's nothing to change. No file is waiting and no pack is marked to come off.");
      return said("Your OP-1 is in disk mode. Leave disk mode and try again. Nothing changed.");
    case STATUS.tapeMoves: return said("The tape is moving. Stop it and try again. Nothing changed.");
    case STATUS.folderNotReady:
      if (p[0] === 3) return said(OTHER_MIDI_WORDS);
      return said("Your OP-1 isn't ready yet. It has just started, or it's being switched off. Wait a moment and try again. Nothing changed.");
    case STATUS.asking: return said("Your OP-1 is already asking something on its screen. Answer it there first. Nothing changed.");
    case STATUS.applying: return said("Your OP-1 is already changing its packs.");
    case STATUS.restarting: return said("Your OP-1 is restarting.");
    case STATUS.unreadable: return said("Your OP-1 didn't understand the request. Its base is probably older than this page. Nothing changed.");
    default: return said("Your OP-1 answered " + statusWords(status) + ". Nothing changed.");
  }
}

function beginRefusalWords(status, p) {
  switch (status) {
    case STATUS.folderNotReady: return "your OP-1 isn't ready yet (it has just started, or it's being switched off)";
    case STATUS.outOfOrder: return "your OP-1 is in disk mode";
    case STATUS.tapeMoves: return "the tape is moving";
    case STATUS.nameRefused: return "your OP-1 refused the name";
    case STATUS.unreadable: return "the file is empty or bigger than your OP-1 takes";
    case STATUS.asking: return "your OP-1 is asking something on its screen";
    case STATUS.applying: return "your OP-1 is changing its packs";
    case STATUS.noRoom:
      if (p[0] === 1) return "your OP-1's disk is too full";
      if (p[0] === 2) return "too many files are waiting on your OP-1 already";
      if (p[0] === 3) return "your OP-1 couldn't open the file for writing";
      return "there's no room (reason " + p[0] + ")";
    default: return "begin answered " + statusWords(status);
  }
}

function removeRefusalWords(status, p) {
  switch (status) {
    case STATUS.nameRefused: return p[0] === 1 ? "the base never comes off" : "your OP-1 refused the name";
    case STATUS.outOfOrder: return p[0] === 2 ? "a file of that name is waiting to take its place" : "your OP-1 is in disk mode";
    case STATUS.fileUnreadable: return "it isn't on your OP-1 any more";
    case STATUS.noRoom: return "too many packs are marked to come off at once";
    case STATUS.folderNotReady: return "your OP-1 isn't ready yet";
    case STATUS.asking: return "your OP-1 is asking something on its screen";
    case STATUS.applying: return "your OP-1 is changing its packs";
    case STATUS.restarting: return "your OP-1 is restarting";
    default: return "remove answered " + statusWords(status);
  }
}

// What the unit is doing during a change, from status 03, in a line for the page.
export function applyProgressWords(s) {
  if (!s || s.aState === undefined) return "";
  switch (s.aState) {
    case 1: return "Your OP-1 is asking on its screen. T1 applies the change and restarts it. T2 cancels.";
    case 2:
      if (!s.aToMove) return "Your OP-1 gets ready to change its packs.";
      if (s.aCheckedFiles >= s.aToMove) return "Your OP-1 checked every file again.";
      return "Checking " + (s.aCheckedFiles + 1) + " of " + s.aToMove + (s.aName ? ", " + s.aName : "");
    case 3: {
      const all = s.aRemovals + s.aToMove, made = s.aRemoved + s.aMoved;
      return "Changing " + Math.min(all, made + 1) + " of " + all + (s.aName ? ", " + s.aName : "");
    }
    case 4: return "Done. Your OP-1 restarts.";
    case 5: return "Stopped before any change.";
    case 6: return "Not finished. Switch your OP-1 off and on.";
    default: return "";
  }
}

// ---------- the check every send goes through ----------

// null when the message may be sent, else why it is refused.
// context = { maxMessage (the unit's largest message; 64 before hello), maxOther (the largest of any command but data), protocol (the door's, from its hello) }.
export function checkMessage(bytes, context = {}) {
  if (!(bytes instanceof Uint8Array) && !Array.isArray(bytes)) return "not a byte array";
  const n = bytes.length;
  if (n === 0) return "empty";
  for (let i = 0; i < n; i++) {
    const b = bytes[i];
    if (!Number.isInteger(b) || b < 0 || b > 0xFF) return "not a byte at index " + i;
  }
  if (startsWith(bytes, REFUSED_PREFIX)) return "addressed to the maker's own service on the unit";
  if (sameBytes(bytes, IDENTITY_REQUEST)) return null;
  if (bytes[0] !== 0xF0) return "not SysEx: this page sends nothing else";
  if (bytes[n - 1] !== 0xF7) return "does not end with F7";
  for (let i = 1; i < n - 1; i++) if (bytes[i] >= 0x80) return "a byte of 0x80 or more inside, at index " + i;
  if (!startsWith(bytes, HEADER)) return "neither the identity request nor the door's mark F0 7D 4F 46";
  if (n < 7) return "shorter than the door's smallest message";
  if (bytes[4] >= 0x40) return "command of 0x40 or more";
  if (bytes[5] >= 0x40) return "number of 0x40 or more";
  if (!SENDABLE.includes(bytes[4])) return "a command this page never sends (" + toHex([bytes[4]]) + ")";
  if (WRITES.includes(bytes[4]) && !(context.protocol >= 3)) return "a command that changes something, sent only to a door that said it can apply a change";
  if (n > PAGE_MAX_MESSAGE) return "longer than the page's cap of " + PAGE_MAX_MESSAGE + " bytes";
  const maxMessage = Math.min(context.maxMessage ?? MAX_BEFORE_HELLO, PAGE_MAX_MESSAGE);
  if (n > maxMessage) return "longer than the unit's largest message, " + maxMessage + " bytes";
  const maxOther = Math.min(context.maxOther ?? MAX_BEFORE_HELLO, maxMessage);
  if (bytes[4] !== CMD.data && n > maxOther) return "longer than " + maxOther + " bytes, the largest of any command but data";
  const u = unpack(Uint8Array.from(bytes).subarray(6, n - 1));
  if (u.error) return "broken packing: " + u.error;
  const p = u.bytes;
  // commit only as the apply, 01: a commit with no payload restarts the unit and applies nothing, and this page never asks for that
  if (bytes[4] === CMD.commit && !(p.length === 1 && p[0] === 1)) return "commit with a payload other than 01: this page never sends it";
  if (bytes[4] === CMD.abort && !(p.length === 0 || (p.length === 1 && p[0] === 1))) return "abort with a payload other than none or 01";
  if (bytes[4] === CMD.status && !(p.length === 0 || (p.length === 1 && (p[0] === 2 || p[0] === 3)))) return "status with a payload other than none, 02 or 03";
  if (bytes[4] === CMD.hello && p.length !== 1) return "hello with a payload other than one byte";
  return null;
}

// ---------- incoming messages ----------

// A queue of incoming messages, each stamped with the clock when it arrived. clock = { now(), setTimer(fn, ms) -> handle, clearTimer(handle) }.
export class MessageQueue {
  constructor(clock) { this.clock = clock; this.items = []; this.waiter = null; }
  push(data) {
    const item = { data: Uint8Array.from(data), at: this.clock.now() };
    if (this.waiter) {
      const w = this.waiter;
      this.waiter = null;
      this.clock.clearTimer(w.timer);
      w.resolve(item);
    } else {
      this.items.push(item);
      if (this.items.length > 256) this.items.shift();
    }
  }
  next(timeoutMs) {
    if (this.items.length) return Promise.resolve(this.items.shift());
    if (this.waiter) return Promise.reject(new Error("one wait at a time"));
    return new Promise((resolve) => {
      const w = { resolve, timer: null };
      w.timer = this.clock.setTimer(() => { if (this.waiter === w) { this.waiter = null; resolve(null); } }, Math.max(0, timeoutMs));
      this.waiter = w;
    });
  }
  clear() { this.items.length = 0; }
  abort() {
    if (this.waiter) {
      const w = this.waiter;
      this.waiter = null;
      this.clock.clearTimer(w.timer);
      w.resolve(null);
    }
  }
}

// ---------- the client: one exchange at a time ----------
// io = { send(bytes), next(timeoutMs) -> Promise<{data, at}|null>, now(), sleep(ms) -> Promise, stopped() -> bool, release(), letGo(why) }
// release() closes the MIDI input and output; the client calls it at once on an answer that says the unit restarts.
// letGo(why) then drops the page's MIDI access and leaves for after.html, which holds no MIDI (letGoOfMidi below), so
// that the browser keeps nothing of the unit while it goes away and comes back. why is "restart", or "power" when the
// unit asks to be switched off and on.

const DEFAULT_LIMITS = Object.freeze({ maxMessage: MAX_BEFORE_HELLO, maxOther: MAX_BEFORE_HELLO, slice: 16384, listEntries: 8 });

export class DoorClient {
  constructor(io, options = {}) {
    this.io = io;
    this.waitMs = options.waitMs ?? ANSWER_WAIT_MS;
    this.paceMs = options.paceMs ?? ASK_PACE_MS;
    this.onFault = options.onFault ?? (() => {});
    this.tapeRetryMs = options.tapeRetryMs ?? 1000;
    this.notReadyRetryMs = options.notReadyRetryMs ?? 1000;
    this.notReadyTries = options.notReadyTries ?? NOT_READY_TRIES;
    this.number = 0;
    this.limits = Object.assign({}, DEFAULT_LIMITS);
    this.hello = null;
    this.protocol = 1;                 // the door's, from its hello: 3 only when it said so with its 68 bytes
    this.asking = false;
    this.restarting = false;
    this.lettingGo = false;
    this.lastSend = null;              // { at, answered } of the last message that waits for an answer
    this.faults = [];
    this.counts = { sent: 0, answered: 0, lostAnswers: 0, resent: 0, statusChecks: 0, lateIgnored: 0 };
    this.sentCommands = [];            // the command byte of every door message sent, in order (for the tests)
    this.kept = new Map();             // name -> SHA-256 (hex) of each file this client saw kept in the waiting folder since the unit's last start
  }

  statusForm() { return payloads.status(this.protocol); }
  context() { return { maxMessage: this.limits.maxMessage, maxOther: this.limits.maxOther, protocol: this.protocol }; }

  fault(text) {
    const f = { at: this.io.now(), text };
    this.faults.push(f);
    this.onFault(f);
  }

  takeNumber() { const n = this.number; this.number = (this.number + 1) & 0x3F; return n; }

  // After a restart the port is back: the client starts again from the limits before hello.
  resume() {
    this.restarting = false;
    this.asking = false;
    this.lastSend = null;
    this.hello = null;
    this.limits = Object.assign({}, DEFAULT_LIMITS);
    this.protocol = 1;
    this.kept.clear();                 // every start empties the waiting folder
    this.lettingGo = false;
  }

  beginRestart(why = "restart") {
    if (this.restarting) return;
    this.restarting = true;
    this.asking = false;
    try { this.io.release(); } catch (e) { this.fault("closing the ports: " + (e && e.message ? e.message : e)); }
    this.letGo(why);
  }

  letGo(why) {
    if (this.lettingGo || typeof this.io.letGo !== "function") return;
    this.lettingGo = true;
    try { this.io.letGo(why); } catch (e) { this.fault("letting go of MIDI: " + (e && e.message ? e.message : e)); }
  }

  // The only place that hands bytes to io.send: the pace first, then the check.
  async send(bytes, waitsForAnswer) {
    if (this.restarting) throw new DoorError("restarting", "the unit is restarting: nothing is sent until it is back");
    if (this.lastSend && !this.lastSend.answered) {
      const since = this.io.now() - this.lastSend.at;
      if (since < this.waitMs) await this.io.sleep(Math.ceil(this.waitMs - since) + 1);
    }
    if (this.restarting) throw new DoorError("restarting", "the unit is restarting: nothing is sent until it is back");
    const why = checkMessage(bytes, this.context());
    if (why) throw new DoorError("refused", "message refused: " + why);
    this.io.send(bytes);
    const at = this.io.now();
    if (waitsForAnswer) this.lastSend = { at, answered: false };
    this.counts.sent++;
    if (bytes[0] === 0xF0 && bytes[1] === 0x7D) this.sentCommands.push(bytes[4]);
    return at;
  }

  // One message and its answer, or null after two seconds. An answer whose command or number is not the awaited one is ignored.
  async exchange(command, payload) {
    const number = this.takeNumber();
    const bytes = buildMessage(command, number, payload);
    const sentAt = await this.send(bytes, true);
    const deadline = sentAt + this.waitMs;
    for (;;) {
      const left = deadline - this.io.now();
      if (left <= 0) break;
      const item = await this.io.next(left);
      if (!item) break;
      const a = parseAnswer(item.data);
      if (!a) continue;
      if (a.command !== command || a.number !== number) {
        this.counts.lateIgnored++;
        if (a.error) this.fault("a message of the door's that could not be read: " + a.error);
        continue;
      }
      this.lastSend.answered = true;
      this.counts.answered++;
      this.noteAnswer(command, a);
      return { answer: a, sentAt, at: item.at, number };
    }
    this.counts.lostAnswers++;
    this.fault("no answer to " + COMMAND_NAMES[command] + " (number " + number + ") within " + this.waitMs / 1000 + " s");
    return null;
  }

  noteAnswer(command, a) {
    if (a.status === STATUS.restarting) { this.beginRestart(); return; }
    if (command === CMD.status || command === CMD.hello) this.asking = a.status === STATUS.asking;
    else if (command === CMD.commit) this.asking = a.status === STATUS.done || a.status === STATUS.asking;
    else if (command === CMD.abort && a.status === STATUS.done) this.asking = false;
    else if (a.status === STATUS.asking) this.asking = true;
  }

  // Up to three status messages, one at a time; the first answer is returned. alreadySilent counts a lost status before.
  async statusCheck(alreadySilent = 0) {
    let silent = alreadySilent;
    while (silent < SILENT_STATUS_LIMIT) {
      if (this.io.stopped()) throw new DoorError("stopped", "stopped");
      this.counts.statusChecks++;
      const r = await this.exchange(CMD.status, this.statusForm());
      if (r) return r;
      silent++;
    }
    throw new DoorError("silent", "the unit does not answer: " + SILENT_STATUS_LIMIT + " status messages in a row got no answer");
  }

  // One message under the retry rule: its answer; or, when two seconds pass with none, a status, and the message again
  // (commit never: then the status's answer, with lost: true). resend(status fields) gives hash's next try.
  async request(command, payload, resend) {
    for (let tries = 1, startWaits = 0; ; tries++) {
      const r = await this.exchange(command, payload);
      if (r && r.answer.status === STATUS.folderNotReady && !r.answer.error && r.answer.payload.length === 0 && ASKED_AGAIN_ON_13.includes(command) && startWaits < START_WAIT_TRIES) {
        startWaits++; tries--;         // the unit has just started: the same message again shortly
        await this.io.sleep(this.paceMs);
        continue;
      }
      if (r) return r;
      if (command === CMD.status) return Object.assign({ lost: true }, await this.statusCheck(1));
      const st = await this.statusCheck(0);
      if (st.answer.status === STATUS.restarting || command === CMD.commit) return Object.assign({ lost: true }, st);
      if (tries >= MAX_TRIES) throw new DoorError("protocol", "the unit answers status but not " + COMMAND_NAMES[command] + ", " + tries + " times");
      if (command === CMD.hash) {
        if (st.answer.error) throw new DoorError("protocol", st.answer.error);
        payload = resend(parseStatus(st.answer.payload));
      }
      this.counts.resent++;
    }
  }

  // The answer for the command asked, or a DoorError for what stands in the way.
  expect(r, command, okStatuses) {
    const a = r.answer;
    if (a.status === STATUS.restarting) throw new DoorError("restarting", "the unit is restarting");
    if (r.lost || a.command !== command) throw new DoorError("protocol", "no answer to " + COMMAND_NAMES[command]);
    if (a.status === STATUS.asking && !okStatuses.includes(STATUS.asking)) throw new DoorError("asking", "the unit is asking its owner something: nothing else is done until it stops");
    if (a.status === STATUS.applying && !okStatuses.includes(STATUS.applying)) throw new DoorError("applying", "the unit is changing its packs: nothing else is done until it has restarted");
    if (a.error) throw new DoorError("protocol", a.error);
    if (!okStatuses.includes(a.status)) throw new DoorError("status", COMMAND_NAMES[command] + " answered " + statusWords(a.status));
    return a;
  }

  applyLimits(h) {
    const maxMessage = Math.max(MAX_BEFORE_HELLO, Math.min(h.maxMessage, PAGE_MAX_MESSAGE));
    this.limits = { maxMessage, maxOther: Math.min(h.maxOther, maxMessage), slice: h.slice, listEntries: h.listEntries || 8 };
    this.protocol = h.protocol >= 3 && h.maxRemovals !== undefined ? 3 : h.protocol >= 2 && h.piece !== undefined ? 2 : 1;
    if (h.piece !== undefined) {
      let piece = h.piece;                                  // the largest piece whose data message stays within the page's largest message
      while (piece > 1 && messageLength(4 + piece) > maxMessage) piece--;
      Object.assign(this.limits, { piece, maxFile: h.maxFile, margin: h.margin, maxWaiting: h.maxWaiting });
    }
    if (this.protocol === 3) Object.assign(this.limits, { maxRemovals: h.maxRemovals, restartMs: h.restartMs });
  }

  // hello: { status, hello } or { restarting: true }.
  async sayHello() {
    this.limits = Object.assign({}, DEFAULT_LIMITS);
    this.protocol = 1;
    const r = await this.request(CMD.hello, payloads.hello());
    if (r.answer.status === STATUS.restarting) return { restarting: true };
    const a = this.expect(r, CMD.hello, [STATUS.done, STATUS.asking, STATUS.cancelled, STATUS.applying]);
    const h = parseHello(a.payload);
    this.hello = h;
    this.applyLimits(h);
    this.asking = a.status === STATUS.asking || h.state === 2;
    return { status: a.status, hello: h };
  }

  // The identity request, after the pace: the answer, or null after two seconds.
  async identity() {
    const sentAt = await this.send(Uint8Array.from(IDENTITY_REQUEST), true);
    const deadline = sentAt + this.waitMs;
    for (;;) {
      const left = deadline - this.io.now();
      if (left <= 0) return null;
      const item = await this.io.next(left);
      if (!item) return null;
      if (isIdentityReply(item.data)) { this.lastSend.answered = true; return item; }
    }
  }

  // hello; when the door does not answer, the identity request tells "no door" from "no unit".
  async connect() {
    try {
      const r = await this.sayHello();
      return Object.assign({ kind: r.restarting ? "restarting" : "door" }, r);
    } catch (e) {
      if (!(e instanceof DoorError) || e.kind !== "silent") throw e;
    }
    const id = await this.identity();
    return id ? { kind: "no door" } : { kind: "no unit" };
  }

  // list, page after page; from 0 again (up to three times) when the total changes between pages.
  // folder 0 is user/pk, 1 the waiting folder user/pkin, 2 the removals marked; flags asks two more bits of protocol 3.
  // whether this door gives each entry's header (section 14): build 5 or later, which hello 03 says in its field 58
  canHeader() { return this.protocol >= 3 && !!this.hello && (this.hello.build ?? 0) >= 5; }

  async listPacks(folder = 0, flags = false, header = false) {
    if ((folder === 2 || flags) && this.protocol < 3) throw new DoorError("protocol", "the door speaks protocol " + this.protocol + ": it has no list of removals");
    if (header && !this.canHeader()) throw new DoorError("protocol", "the door on this unit gives no pack's header: it is older than build 5");
    for (let round = 1; ; round++) {
      let total = null, beyond = 0, index = 0, changed = false;
      const entries = [];
      for (;;) {
        if (this.io.stopped()) throw new DoorError("stopped", "stopped");
        const a = this.expect(await this.request(CMD.list, payloads.list(index, folder, flags, header)), CMD.list, [STATUS.done]);
        const L = parseList(a.payload, { flags, header });
        if (total === null) { total = L.total; beyond = L.beyond; }
        else if (L.total !== total) { changed = true; break; }
        entries.push(...L.entries);
        if (L.count === 0 || L.first + L.count >= L.total) break;
        index = L.first + L.count;
        if (index > 63) break;
      }
      if (!changed) return { total, beyond, entries };
      this.fault("the folder changed while it was listed; listing again from the start");
      if (round >= 3) throw new DoorError("protocol", "the folder kept changing while it was listed");
    }
  }

  // hash: the SHA-256 of one file of user/pk, slice by slice, computed on the unit.
  async hashPack(name, onProgress = () => {}) {
    const start = payloads.hashStart(name);
    const resend = (s) => (s.state === 1 ? payloads.hashNext(s.hashDone) : start);
    let payload = start, slices = 0, done = -1, starts = 0;
    for (;;) {
      if (this.io.stopped()) throw new DoorError("stopped", "stopped");
      const r = await this.request(CMD.hash, payload, resend);
      const a = this.expect(r, CMD.hash, [STATUS.done, STATUS.more, STATUS.nameRefused, STATUS.outOfOrder, STATUS.fileUnreadable]);
      if (a.status === STATUS.nameRefused) throw new DoorError("name", "your OP-1 refused the name " + name);
      if (a.status === STATUS.fileUnreadable) throw new DoorError("unreadable", "your OP-1 couldn't read " + name);
      if (a.status === STATUS.outOfOrder) {
        if (a.payload.length === 16) {
          done = parseHash(STATUS.more, a.payload).done;
          payload = payloads.hashNext(done);
        } else {
          if (++starts > 2) throw new DoorError("protocol", "hash of " + name + ": the unit lost the fingerprint in progress three times");
          payload = start; slices = 0; done = -1;
        }
        continue;
      }
      const h = parseHash(a.status, a.payload);
      if (h.done <= done) { slices = 0; done = -1; }        // started again by the retry rule
      if (h.done > h.size) throw new DoorError("protocol", "hash of " + name + ": done " + h.done + " of " + h.size);
      done = h.done;
      slices++;
      if (slices > Math.ceil(h.size / Math.max(1, this.limits.slice)) + 4) throw new DoorError("protocol", "hash of " + name + ": more slices than the file can hold");
      onProgress({ name, size: h.size, done: h.done });
      if (a.status === STATUS.done) {
        if (h.done !== h.size) throw new DoorError("protocol", "hash of " + name + ": finished at " + h.done + " of " + h.size);
        return { name, size: h.size, sha256: digestHex(h.digest) };
      }
      if (h.done >= h.size) throw new DoorError("protocol", "hash of " + name + ": more to come at " + h.done + " of " + h.size);
      payload = payloads.hashNext(done);
    }
  }

  // ---------- files into the waiting folder ----------

  needs3(what) { if (this.protocol < 3) throw new DoorError("protocol", "the door on this unit speaks protocol " + this.protocol + ": it cannot " + what); }

  // One message of a transfer: its answer, or after two seconds the answer of status 03 (up to three), with lost: true.
  async xfer(command, payload) {
    const r = await this.exchange(command, payload);
    if (r) {
      if (r.answer.status === STATUS.restarting) return { restarting: true };
      if (r.answer.error) throw new DoorError("protocol", r.answer.error);
      return r;
    }
    const st = await this.statusCheck(0);
    if (st.answer.status === STATUS.restarting) return { restarting: true };
    if (st.answer.error) throw new DoorError("protocol", st.answer.error);
    const s = parseStatus(st.answer.payload);
    if (s.xState === undefined) throw new DoorError("protocol", "status answered without the transfer's block");
    return { lost: true, s };
  }

  async readStatus() {
    const r = await this.request(CMD.status, this.statusForm());
    if (r.answer.status === STATUS.restarting) return { restarting: true, status: STATUS.restarting };
    if (r.answer.error) throw new DoorError("protocol", r.answer.error);
    return Object.assign({ status: r.answer.status }, parseStatus(r.answer.payload));
  }

  // abort 01 until the unit says no file remains; "not ready" just after a start is asked again a second later.
  async emptyWaiting() {
    this.needs3("empty its waiting folder");
    let removed = 0, messages = 0, more = true;
    while (more) {
      if (messages >= 12) { this.fault("the waiting folder was still not empty after " + messages + " abort 01 messages"); break; }
      messages++;
      let a, waits = 0;
      for (;;) {
        a = this.expect(await this.request(CMD.abort, payloads.abortAll()), CMD.abort, [STATUS.done, STATUS.folderNotReady]);
        if (a.status !== STATUS.folderNotReady) break;
        if (waits >= this.notReadyTries) throw new DoorError("notready", "your OP-1 isn't ready yet. Try again in a moment.");
        waits++;
        await this.io.sleep(this.notReadyRetryMs);
      }
      const f = parseAbort(a.payload);
      removed += f.removed;
      more = f.more;
      if (more && f.removed === 0) { this.fault("abort 01 said more files remain but removed none"); break; }
    }
    const left = await this.listPacks(1);
    for (const name of Array.from(this.kept.keys())) if (!left.entries.some((e) => e.name === name)) this.kept.delete(name);
    return { removed, messages, left: left.entries };
  }

  async stopTransfer(res, why) {
    try { this.expect(await this.request(CMD.abort, payloads.abort()), CMD.abort, [STATUS.done]); }
    catch (e) {
      if (e instanceof DoorError && e.kind === "restarting") return this.fileEnd(res, "restarting", "your OP-1 is restarting");
      throw e;
    }
    return this.fileEnd(res, "stopped", why);
  }

  fileRefusal(name, size) {
    const nameWhy = checkName(name);
    if (nameWhy) return "its name won't do on your OP-1 (" + nameWhy + ")";
    if (size === 0) return "the file is empty";
    if (this.limits.maxFile && size > this.limits.maxFile) return "the file is bigger than your OP-1 takes";
    return null;
  }

  fileEnd(res, outcome, words) {
    res.outcome = outcome;
    res.words = words;
    res.kept = outcome === "kept";
    if (res.kept) this.kept.set(res.name, res.sha256);
    return res;
  }

  // One file: begin, the pieces, end, the unit's own check of what arrived. file = { name, data: Uint8Array, sha256: Uint8Array(32) };
  // ui = { onStep({phase, received, checked, size}), onTape(moving), wantsStop() }.
  async sendFile(file, ui = {}) {
    this.needs3("take files");
    const onStep = ui.onStep ?? (() => {});
    const wantsStop = ui.wantsStop ?? (() => false);
    const name = file.name, data = file.data, size = data.length;
    const res = { name, size, sha256: digestHex(file.sha256), outcome: null, kept: false, words: "", begins: 0, removedRetries: 0, notReadyWaits: 0 };
    const refusal = this.fileRefusal(name, size);
    if (refusal) return this.fileEnd(res, "refused", refusal);
    const begin = payloads.begin(size, file.sha256, name);
    this.kept.delete(name);
    let phase = "begin", received = 0, checked = 0, tapeShown = false;
    const tapeOff = () => { if (tapeShown && ui.onTape) ui.onTape(false); tapeShown = false; };
    const tapeWait = async () => { tapeShown = true; if (ui.onTape) ui.onTape(true); await this.io.sleep(this.tapeRetryMs); };
    const again = (why) => {
      res.removedRetries++;
      if (res.removedRetries > 2) return false;
      this.fault(name + ": " + why + "; sending it again from the start");
      phase = "begin"; received = 0; checked = 0;
      return true;
    };
    for (;;) {
      if (this.io.stopped() || wantsStop()) return this.stopTransfer(res, "stopped from the page");
      if (phase === "begin") {
        if (res.begins >= 6) return this.fileEnd(res, "failed", "your OP-1 never took the start of it");
        res.begins++;
        const r = await this.xfer(CMD.begin, begin);
        if (r.restarting) return this.fileEnd(res, "restarting", "your OP-1 is restarting");
        if (r.lost) {
          const s = r.s;
          if (s.xState === 4 && s.xName === name && s.xSize === size && s.xReceived === 0) { phase = "data"; received = 0; }
          continue;
        }
        const a = r.answer;
        if (a.status === STATUS.tapeMoves) { res.begins--; await tapeWait(); continue; }
        if (a.status === STATUS.folderNotReady) {
          if (res.notReadyWaits >= this.notReadyTries) return this.fileEnd(res, "refused", beginRefusalWords(a.status, a.payload));
          res.notReadyWaits++; res.begins--;
          await this.io.sleep(this.notReadyRetryMs);
          continue;
        }
        tapeOff();
        if (a.status !== STATUS.done) return this.fileEnd(res, "refused", beginRefusalWords(a.status, a.payload));
        parseBegin(a.payload);
        phase = "data"; received = 0;
        onStep({ phase, received, checked, size });
        continue;
      }
      if (phase === "data") {
        if (received >= size) { phase = "close"; continue; }
        const piece = data.subarray(received, Math.min(size, received + this.limits.piece));
        const r = await this.xfer(CMD.data, payloads.data(received, piece));
        if (r.restarting) return this.fileEnd(res, "restarting", "your OP-1 is restarting");
        if (r.lost) {
          const s = r.s;
          if (s.xState === 4 && s.xName === name) { received = s.xReceived; continue; }
          if (s.xState === 5 && s.xName === name) { phase = "check"; checked = s.xChecked; continue; }
          phase = "begin"; received = 0;
          continue;
        }
        const a = r.answer;
        if (a.status === STATUS.tapeMoves) { await tapeWait(); continue; }
        if (a.status === STATUS.folderNotReady) { tapeOff(); return this.fileEnd(res, "removed", "your OP-1 was being switched off"); }
        tapeOff();
        if (a.status === STATUS.done) {
          const d = parseData(a.payload);
          if (d.received <= received || d.received > size) throw new DoorError("protocol", name + ": data answered received " + d.received + " after " + received);
          received = d.received;
          onStep({ phase, received, checked, size });
          continue;
        }
        if (a.status === STATUS.outOfOrder && a.payload.length === 12) { received = parseData(a.payload).received; continue; }
        if (a.status === STATUS.outOfOrder) { phase = "begin"; received = 0; continue; }
        if (a.status === STATUS.noRoom) return this.fileEnd(res, "removed", "a write on your OP-1 failed");
        return this.fileEnd(res, "failed", "data answered " + statusWords(a.status));
      }
      if (phase === "close") {
        const r = await this.xfer(CMD.end, payloads.endClose());
        if (r.restarting) return this.fileEnd(res, "restarting", "your OP-1 is restarting");
        if (r.lost) {
          const s = r.s;
          if (s.xState === 5 && s.xName === name) { phase = "check"; checked = s.xChecked; continue; }
          if (s.xState === 4 && s.xName === name) { received = s.xReceived; phase = "data"; continue; }
          if (s.xState === 0 && s.xOutcome === 1 && s.xName === name) return this.fileEnd(res, "kept", "on your OP-1 and checked");
          phase = "begin"; received = 0;
          continue;
        }
        const a = r.answer;
        if (a.status === STATUS.more) { checked = parseEnd(a.status, a.payload).done; phase = "check"; onStep({ phase, received, checked, size }); continue; }
        if (a.status === STATUS.fingerprintDiffers) { parseEnd(a.status, a.payload); if (!again("short at the end")) return this.fileEnd(res, "removed", "it didn't arrive whole, three times over"); continue; }
        if (a.status === STATUS.outOfOrder) { phase = "begin"; received = 0; continue; }
        return this.fileEnd(res, "failed", "end answered " + statusWords(a.status));
      }
      if (phase === "check") {
        const r = await this.xfer(CMD.end, payloads.endNext(checked));
        if (r.restarting) return this.fileEnd(res, "restarting", "your OP-1 is restarting");
        if (r.lost) {
          const s = r.s;
          if (s.xState === 5 && s.xName === name) { checked = s.xChecked; continue; }
          if (s.xState === 0 && s.xName === name && s.xOutcome === 1) return this.fileEnd(res, "kept", "on your OP-1 and checked");
          if (s.xState === 0 && (s.xOutcome === 2 || s.xOutcome === 4)) { if (!again("removed at its check")) return this.fileEnd(res, "removed", "it didn't arrive whole, three times over"); continue; }
          phase = "begin"; received = 0;
          continue;
        }
        const a = r.answer;
        if (a.status === STATUS.more) { checked = parseEnd(a.status, a.payload).done; onStep({ phase, received, checked, size }); continue; }
        if (a.status === STATUS.done) {
          const e = parseEnd(a.status, a.payload);
          if (digestHex(e.digest) !== res.sha256) this.fault(name + ": the unit kept the file but gave the fingerprint " + digestHex(e.digest));
          return this.fileEnd(res, "kept", "on your OP-1 and checked");
        }
        if (a.status === STATUS.fingerprintDiffers) { parseEnd(a.status, a.payload); if (!again("removed at its check")) return this.fileEnd(res, "removed", "it didn't arrive whole, three times over"); continue; }
        if (a.status === STATUS.outOfOrder && a.payload.length === 16) { checked = parseEnd(STATUS.more, a.payload).done; continue; }
        if (a.status === STATUS.outOfOrder) { phase = "begin"; received = 0; continue; }
        return this.fileEnd(res, "failed", "end answered " + statusWords(a.status));
      }
    }
  }

  // ---------- removals and the apply ----------

  // remove, one form: { ok: true, noted, waiting, flag } or { ok: false, status, words }.
  async removeRequest(payload) {
    this.needs3("take packs off");
    const r = await this.request(CMD.remove, payload);
    const a = r.answer;
    if (a.status === STATUS.restarting) throw new DoorError("restarting", "the unit is restarting");
    if (r.lost || a.command !== CMD.remove) throw new DoorError("protocol", "no answer to remove");
    if (a.error) throw new DoorError("protocol", a.error);
    if (a.status !== STATUS.done) return { ok: false, status: a.status, words: removeRefusalWords(a.status, a.payload) };
    return Object.assign({ ok: true }, parseRemove(a.payload));
  }
  noteRemoval(name) { return this.removeRequest(payloads.removeNote(name)); }
  takeBackRemoval(name) { return this.removeRequest(payloads.removeTakeBack(name)); }

  newApplyResult() {
    return { asked: false, applying: false, ended: false, restarting: false, assumed: false, notFinished: false, notTaken: false, refused: null, counts: null, outcome: null, words: "", cancelSent: false, statuses: 0, last: null, commitLost: false };
  }

  applyRestarting(result, answer) {
    if (answer && !answer.error && answer.command === CMD.status) {
      try { result.last = parseStatus(answer.payload); } catch (e) { /* the restart matters, not the payload */ }
    }
    return Object.assign(result, { restarting: true, outcome: 1, words: applyOutcomeWords(1) });
  }

  // commit 01, then status 03 every quarter second while the unit asks or applies, until it restarts or the change ends.
  // hooks = { onAsked(counts), onState(fields), wantsCancel() }. commit 01 is never sent twice; once T1 is pressed only status is sent.
  async applyPacks(hooks = {}) {
    this.needs3("apply a change");
    const result = this.newApplyResult();
    const r = await this.request(CMD.commit, payloads.apply());
    if (r.lost) {
      result.commitLost = true;
      this.fault("commit 01's answer was lost; not sent again: the unit's status decides");
      if (r.answer.status === STATUS.restarting) return this.applyRestarting(result, r.answer);
      result.pending = r;
    } else {
      const a = r.answer;
      if (a.status === STATUS.restarting) return this.applyRestarting(result, a);
      if (a.error) throw new DoorError("protocol", a.error);
      if (a.status === STATUS.done) {
        result.asked = true;
        result.counts = parseCommit(a.payload);
        if (hooks.onAsked) hooks.onAsked(result.counts);
      } else if (a.status === STATUS.applying) {
        result.applying = true;
      } else {
        result.refused = commitRefusal(a.status, a.payload);
        return Object.assign(result, { ended: true, words: result.refused.words });
      }
    }
    return this.watchApply(result, hooks);
  }

  // A start that finishes the last change, or a change the page did not ask: followed to its restart.
  async followApply(hooks = {}) {
    const result = this.newApplyResult();
    result.applying = true;
    return this.watchApply(result, hooks);
  }

  async watchApply(result, hooks = {}) {
    const onState = hooks.onState ?? (() => {});
    const wantsCancel = hooks.wantsCancel ?? (() => false);
    let pending = result.pending ?? null;
    delete result.pending;
    try {
      for (;;) {
        let st;
        if (pending) { st = pending; pending = null; }
        else {
          const began = this.io.now();
          // the cancel only while the unit asks; once T1 is pressed the page does not stop the change and sends nothing but status
          if (this.asking && !result.applying && !result.cancelSent && (wantsCancel() || this.io.stopped())) {
            result.cancelSent = true;
            const ar = await this.request(CMD.abort, payloads.abort());
            if (ar.answer.status === STATUS.restarting) return this.applyRestarting(result, ar.answer);
            if (ar.answer.status === STATUS.applying) result.applying = true;
            await this.pace(began);
            continue;
          }
          st = await this.request(CMD.status, this.statusForm());
          st.began = began;
        }
        const a = st.answer;
        if (a.status === STATUS.restarting) return this.applyRestarting(result, a);
        if (a.error) throw new DoorError("protocol", a.error);
        const s = parseStatus(a.payload);
        result.statuses++;
        result.last = s;
        onState(Object.assign({ status: a.status }, s));
        if (a.status === STATUS.asking || a.status === STATUS.applying) {
          if (a.status === STATUS.asking) result.asked = true;
          else result.applying = true;
          if (a.status === STATUS.applying && (s.aState === 6 || s.state === 7)) {
            // not finished: the unit's page says to switch it off and on; the next start finishes. The page lets go now.
            const outcome = s.aOutcome ?? 10;
            this.beginRestart("power");
            return Object.assign(result, { ended: true, notFinished: true, outcome, words: applyOutcomeWords(outcome, s.aName) });
          }
          if (st.began !== undefined) await this.pace(st.began); else await this.io.sleep(this.paceMs);
          continue;
        }
        if (!result.asked && !result.applying) {
          return Object.assign(result, { ended: true, notTaken: true, outcome: 0, words: "Your OP-1 didn't take the request, and it wasn't sent twice. Nothing changed. Press the button again." });
        }
        const outcome = s.aOutcome !== undefined ? s.aOutcome : s.outcome;
        return Object.assign(result, { ended: true, outcome, words: applyOutcomeWords(outcome, s.aName) });
      }
    } catch (e) {
      // The unit went quiet after T1: the change goes on by itself and the unit restarts. The page lets go as for a restart.
      const away = (e instanceof DoorError && ["silent", "stopped"].includes(e.kind)) || (!(e instanceof DoorError) && this.io.stopped());
      if ((result.applying || result.asked) && away) {
        this.fault("the unit stopped answering while it changed its packs (" + (e && e.message ? e.message : e) + "): taken as its restart");
        this.beginRestart();
        return Object.assign(result, { restarting: true, assumed: true, outcome: null, words: "Your OP-1 went quiet while it changed its packs. It restarts by itself." });
      }
      throw e;
    }
  }

  async pace(began) {
    const left = this.paceMs - (this.io.now() - began);
    if (left > 0) await this.io.sleep(left);
  }

  // The whole change: what the unit holds read, files that waited from before taken away, the removals marked, the
  // files sent (base.pack last), both lists read back and held to the plan, the record kept, commit 01 and the watch.
  // plan = { files: [{ name, data: Uint8Array, sha256: Uint8Array(32), id?, version? }], remove: [names] }.
  // hooks = { onStep(stage, words), onFile(name, step), onTape(moving), wantsStop(), saveRecord(record) -> bool, onAsked(counts), onState(fields), wantsCancel(), firmware }.
  // { stage, words, files, record, recordKept, apply }: stage "applied" when the unit restarts (or asks to be switched off and on), else where it stopped.
  async changePacks(plan, hooks = {}) {
    this.needs3("change its packs");
    const say = hooks.onStep ?? (() => {});
    const out = { stage: null, words: "", files: [], record: null, recordKept: null, apply: null, warnings: [] };
    const stop = (stage, words) => Object.assign(out, { stage, words });
    const files = (plan.files ?? []).slice().sort((a, b) => (a.name.toLowerCase() === "base.pack") - (b.name.toLowerCase() === "base.pack"));
    const remove = plan.remove ?? [];
    // 1. the page's own refusals: nothing is sent
    const names = files.map((f) => f.name);
    for (const f of files) {
      if (names.indexOf(f.name) !== names.lastIndexOf(f.name)) return stop("refused", "Two files are named " + f.name + ".");
      const why = this.fileRefusal(f.name, f.data.length);
      if (why) return stop("refused", f.name + ": " + why + ". Nothing was sent.");
    }
    for (const name of remove) {
      if (checkName(name)) return stop("refused", name + " isn't a pack file's name, so it can't come off from here.");
      if (name.toLowerCase() === "base.pack") return stop("refused", "The base never comes off.");
      if (names.includes(name)) return stop("refused", name + " can't come off and go on in the same change.");
    }
    if (!files.length && !remove.length) return stop("refused", "Nothing to change.");
    if (remove.length > (this.limits.maxRemovals ?? 32)) return stop("refused", "At most " + (this.limits.maxRemovals ?? 32) + " packs can come off in one go.");
    // 2. what the unit holds now
    say("read", "Reading what's on your OP-1");
    const pk = await this.listPacks(0);
    for (const name of remove) if (!pk.entries.some((e) => e.name === name)) return stop("refused", name + " isn't on your OP-1 any more. Connect again to read it afresh.");
    // 3. the waiting folder holds only files sent from here with the same fingerprint; anything else goes first
    const chosen = new Map(files.map((f) => [f.name, digestHex(f.sha256)]));
    const waitsRight = (e) => e.checked && !e.underWay && chosen.has(e.name) && this.kept.get(e.name) === chosen.get(e.name);
    let waiting = await this.listPacks(1, true);
    if (waiting.entries.some((e) => !waitsRight(e))) {
      say("empty", "Clearing files that were waiting from before");
      const em = await this.emptyWaiting();
      if (em.left.some((e) => e.checked)) return stop("waiting", "Some files from before are still waiting on your OP-1 and the page couldn't clear them. Nothing changed.");
      waiting = await this.listPacks(1, true);
    }
    // 4. the removals: the unit's marks made to match the page's ticks
    say("remove", "Marking the packs that come off");
    const marked = await this.listPacks(2);
    for (const e of marked.entries) {
      if (remove.includes(e.name)) continue;
      const r = await this.takeBackRemoval(e.name);
      if (!r.ok) return stop("remove", e.name + ": " + r.words + ". Nothing changed.");
    }
    for (const name of remove) {
      if (marked.entries.some((e) => e.name === name)) continue;
      const r = await this.noteRemoval(name);
      if (!r.ok) return stop("remove", name + ": " + r.words + ". Nothing changed.");
    }
    // 5. the files, each kept and checked by the unit, base.pack last
    for (const f of files) {
      if (waiting.entries.some((e) => e.name === f.name && waitsRight(e))) {
        out.files.push({ name: f.name, size: f.data.length, sha256: chosen.get(f.name), outcome: "kept", kept: true, words: "already waiting on your OP-1", skipped: true });
        continue;
      }
      if (this.io.stopped() || (hooks.wantsStop && hooks.wantsStop())) return stop("stopped", "Stopped before your OP-1 was asked. Nothing changed.");
      say("send", "Sending " + f.name);
      const res = await this.sendFile(f, { onStep: (x) => hooks.onFile && hooks.onFile(f.name, x), onTape: hooks.onTape, wantsStop: hooks.wantsStop });
      out.files.push(res);
      if (!res.kept) return stop(res.outcome === "restarting" ? "restarting" : "send", f.name + " didn't go: " + res.words + ". Nothing changed.");
    }
    // 6. read back: what the unit would apply, held to the plan
    const nowMarked = await this.listPacks(2);
    const nowWaiting = await this.listPacks(1, true);
    const same = (a, b) => a.length === b.length && a.every((x, i) => x === b[i]);
    const waitNames = nowWaiting.entries.filter((e) => e.checked).map((e) => e.name).sort();
    if (!same(waitNames, names.slice().sort())) return stop("check", "The files waiting on your OP-1 aren't the ones chosen here. Nothing changed.");
    const markNames = nowMarked.entries.map((e) => e.name).sort();
    if (!same(markNames, remove.slice().sort())) return stop("check", "The packs marked to come off aren't the ones unticked here. Nothing changed.");
    for (const e of nowWaiting.entries) if (e.unitRefuses) out.warnings.push(e.name);
    // 7. the record, before commit 01, so that it outlives the page
    const record = makeRecord({
      firmware: hooks.firmware ?? firmwareOf(this.hello ? this.hello.osId : 0), hello: this.hello,
      sent: files.map((f) => ({ name: f.name, size: f.data.length, sha256: chosen.get(f.name), replaces: pk.entries.some((e) => e.name === f.name), id: f.id ?? null, version: f.version ?? null })),
      removed: remove.map((name) => ({ name, size: (pk.entries.find((e) => e.name === name) || {}).size ?? null })),
    });
    out.record = record;
    out.recordKept = hooks.saveRecord ? !!hooks.saveRecord(record) : false;
    // 8. the apply
    say("commit", "Asking your OP-1");
    const keep = (phase) => { if (record.phase !== phase) { record.phase = phase; if (hooks.saveRecord) hooks.saveRecord(record); } };
    const ap = await this.applyPacks({
      onAsked: (c) => { record.counts = c; keep("asking"); if (hooks.onAsked) hooks.onAsked(c); },
      onState: (s) => { if (s.status === STATUS.asking) keep("asking"); else if (s.status === STATUS.applying) keep("applying"); if (hooks.onState) hooks.onState(s); },
      wantsCancel: hooks.wantsCancel,
    });
    out.apply = ap;
    record.outcome = ap.outcome; record.words = ap.words;
    if (ap.restarting) keep("restarting");
    else if (ap.notFinished) keep("not finished");
    else if (ap.refused) keep("refused");
    else keep("ended");
    if (hooks.saveRecord) hooks.saveRecord(record);
    if (ap.restarting || ap.notFinished) return Object.assign(out, { stage: "applied", words: ap.words });
    return stop(ap.refused ? "refused by the unit" : "ended", ap.words + (ap.outcome >= 2 && ap.outcome <= 9 ? " " + STILL_WAITING_WORDS : ""));
  }

  // After the restart, hello having answered: status 03 (what the start found), the list of user/pk, and the
  // fingerprint of each file the record names, each held to the record. { applying: true } when a start finishes a change.
  async checkAfterRestart(record, hooks = {}) {
    const out = { restarting: false, applying: false, status: null, list: null, hashes: [], verdict: null };
    const st = await this.readStatus();
    if (st.restarting) return Object.assign(out, { restarting: true });
    out.status = st;
    if (st.status === STATUS.applying) return Object.assign(out, { applying: true });
    out.list = await this.listPacks(0);
    for (const f of (record && record.sent) || []) {
      const e = out.list.entries.find((x) => x.name === f.name);
      if (!e || e.sizeUnknown || e.size !== f.size) continue;
      if (hooks.onHash) hooks.onHash(f.name);
      try { out.hashes.push(await this.hashPack(f.name)); }
      catch (err) {
        if (err instanceof DoorError && ["unreadable", "name", "status"].includes(err.kind)) out.hashes.push({ name: f.name, error: err.message });
        else throw err;
      }
    }
    out.verdict = holdToRecord(record, out.list, out.hashes);
    return out;
  }
}

// ---------- the record of a change, kept in the browser's storage, one a firmware ----------
// storage is localStorage or anything with getItem, setItem and removeItem; every use is wrapped, so that a browser that
// keeps nothing still runs the page (it then cannot check the unit after its restart, and says so).

export const PENDING_PHASES = Object.freeze(["before commit", "asking", "applying", "restarting", "not finished"]);
export function recordKey(firmware) { return RECORD_KEY + String(firmware); }

export function makeRecord({ firmware = null, hello = null, sent = [], removed = [], at = null } = {}) {
  return {
    version: 1, firmware, madeAt: at ?? new Date().toISOString(), phase: "before commit",
    unit: hello ? { osId: hello.osId, baseVersion: hello.baseVersion, release: hello.release, build: hello.build ?? null } : null,
    sent: sent.map((f) => ({ name: f.name, size: f.size, sha256: String(f.sha256).toLowerCase(), replaces: !!f.replaces, id: f.id ?? null, version: f.version ?? null })),
    removed: removed.map((r) => ({ name: r.name, size: r.size ?? null })),
    counts: null, outcome: null, words: "", checked: null,
  };
}
export function saveRecord(storage, firmware, record) {
  try { storage.setItem(recordKey(firmware), JSON.stringify(record)); return true; } catch (e) { return false; }
}
export function loadRecord(storage, firmware) {
  try {
    const t = storage.getItem(recordKey(firmware));
    if (!t) return null;
    const r = JSON.parse(t);
    return r && r.version === 1 && r.firmware === firmware && Array.isArray(r.sent) && Array.isArray(r.removed) ? r : null;
  } catch (e) { return null; }
}
export function forgetRecord(storage, firmware) {
  try { storage.removeItem(recordKey(firmware)); return true; } catch (e) { return false; }
}
// A record whose change may have reached the unit and was not checked yet: the page checks the unit when it opens again.
export function recordPending(record) { return !!record && PENDING_PHASES.includes(record.phase); }

// The unit as it is after the restart, held to the record: "installed" when the name, the size and the SHA-256 agree.
export function holdToRecord(record, list, hashes = []) {
  const on = new Map(((list && list.entries) || []).map((e) => [e.name, e]));
  const files = ((record && record.sent) || []).map((f) => {
    const e = on.get(f.name), h = hashes.find((x) => x.name === f.name);
    const v = (verdict, words) => Object.assign({}, f, { verdict, words });
    if (!e) return v("not installed", "not on your OP-1");
    if (e.sizeUnknown) return v("not installed", "there, but its size couldn't be read");
    if (e.size !== f.size) return v("not installed", "there, with another size");
    if (!h || !h.sha256) return v("not installed", "there, but its fingerprint couldn't be read");
    if (h.sha256.toLowerCase() !== f.sha256.toLowerCase()) return v("not installed", "there, with other bytes");
    if (!e.isPack) return v("not installed", "there, but the loader doesn't take its name");
    return v("installed", "on your OP-1, byte for byte");
  });
  const removed = ((record && record.removed) || []).map((r) => Object.assign({}, r, on.has(r.name) ? { verdict: "still there", words: "still on your OP-1" } : { verdict: "removed", words: "off" }));
  return { files, removed, allGood: files.every((f) => f.verdict === "installed") && removed.every((r) => r.verdict === "removed") };
}

// ---------- letting go of MIDI before the restart, and coming back ----------

export function afterPageUrl(why) { return AFTER_PAGE + (why === "power" ? "?for=power" : ""); }

// On the answer that says the unit restarts (the client has closed the ports already): the record kept, both ports
// closed, the MIDI access dropped, and the page replaced by after.html in the same tab, all at once.
// host = { keepRecord(why), releasePorts(), dropAccess(), navigate(url) }. Never a word asking the owner to close the tab.
export function letGoOfMidi(host, why = "restart") {
  const done = [];
  for (const [step, fn] of [["record", () => host.keepRecord(why)], ["ports", () => host.releasePorts()], ["access", () => host.dropAccess()]]) {
    try { fn(); done.push(step); } catch (e) { done.push(step + " failed: " + (e && e.message ? e.message : e)); }
  }
  host.navigate(afterPageUrl(why));
  done.push("navigate");
  return done;
}

export const QUIT_BROWSER_WORDS = "Your OP-1 hasn't answered yet. If its start screen is long gone, quit the browser completely (every window), open it again and come back to this page. It remembers what to check.";

// Back on the page: the OP-1 ports taken (env.takePorts), then hello until the door answers.
// env = { takePorts() -> bool, setTimer(fn, ms), clearTimer(h), onSlow() }. After RETURN_SLOW_MS with no door, onSlow() once, and it goes on looking.
export async function reachDoorAgain(client, env, options = {}) {
  const io = client.io, t0 = io.now();
  const slowMs = options.slowMs ?? RETURN_SLOW_MS, giveUpMs = options.giveUpMs ?? RETURN_GIVE_UP_MS;
  let slow = false, tries = 0, noDoor = 0;
  const timer = env.setTimer(() => { slow = true; if (env.onSlow) env.onSlow(); }, slowMs);
  const done = (r) => Object.assign(r, { tries, slow, ms: io.now() - t0 });
  try {
    for (;;) {
      if (io.stopped()) return done({ kind: "stopped" });
      if (env.takePorts()) {
        tries++;
        client.resume();
        let r;
        try { r = await client.connect(); }
        catch (e) { if (!(e instanceof DoorError) || e.kind === "stopped") throw e; r = { kind: "error", error: e.message }; }
        if (r.kind === "door" || r.kind === "restarting") return done(r);
        if (r.kind === "no door" && ++noDoor >= 2) return done(r);
      }
      if (io.now() - t0 >= giveUpMs) return done({ kind: "no unit" });
      await io.sleep(1000);
    }
  } finally { env.clearTimer(timer); }
}
