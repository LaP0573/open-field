// open-field: the room for packs, as the install page counts it. What a pack file takes of the loader's room (its
// foot), where the loader would place a set of pack files, and what that leaves: the room's bytes, the most pack files,
// and the engines and effects the base registers. The page counts the way the door counts before it lets a change go
// (module/build_r4c_door.cpp, room_needed), and the door counts the way the loader places the packs at the start
// (module/build_m4_loader.cpp, run_new). No browser object and no Node API in this file: validate-room.mjs runs the
// same code as the page.
//
// The page counts only the files it holds: its own base.pack, the files of the listing and the files picked on the
// computer, each found by its SHA-256. A pack file on the unit that it does not hold is "not known": it is left out of
// the count and named, and every figure is then the least it can be.

import { N1_LOADER } from "./door.mjs";

export const HEADER_BYTES = 96;
// A pack stands at the next address that is a multiple of 32 bytes from where it was linked (PACK_ALIGN of the loader).
export const PACK_ALIGN = 32;

// The room's first address, by the loader's generation. The door's hello says the generation, the room's bytes and the
// most pack files, but not where the room begins, and where each pack stands within 32 bytes depends on it.
//   1, the loader N1: docs/packs.md; PACK_BASE of the door; pack_link.base in profiles/op1f-1.7.3-n2.json, the window
//      every pack is linked for, which is N1's room
//   2, the loader N2: reserve.pack_room.base in builder/loader.json; pack_room.base in profiles/op1f-1.7.3-n2.json
// validate-room.mjs holds both to those files.
export const ROOM_FIRST = Object.freeze({ 1: 0x8d2f4234, 2: 0x85000000 });

// How many engines and effects the base registers for all packs together: OP1F_ENGINES_MOST and OP1F_EFFECTS_MOST in
// module/build_r6e_card.h. The door does not say them, so they are written here; validate-room.mjs holds them to the
// header. One ask of kind OP1F_ASK_ENGINE (8) on a card is one engine, one of kind OP1F_ASK_EFFECT (7) one effect.
export const ENGINES_MOST = 8;
export const EFFECTS_MOST = 16;
const ASK_EFFECT = 7, ASK_ENGINE = 8;
const CARD_MARK = 0x44524143;                       // "CARD"
const CARD_ASKS_AT = 56, CARD_LEVEL_4_BYTES = 64, ASKS_MOST = 16, ASK_BYTES = 12;   // the card of level 4 and later

// What a pack file takes of the room, from its header (docs/packs.md, "What is in the file"):
// { name, version, osId, link, body, lists, zeroed, foot, engines, effects }, or null for a file the loader does not
// read as a pack of format 3 (the door counts no room for it). foot: the body and the larger of its lists and its zeroed
// data, the loader's and the door's figure. engines, effects: the asks of those kinds on its card, 0 for a pack with no
// card, null when the card does not stand where the header says.
export function footOf(bytes) {
  if (!(bytes instanceof Uint8Array) || bytes.length < HEADER_BYTES) return null;
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const word = (at) => view.getUint32(at, true);
  if (word(0) !== 0x4631504f || word(4) !== 0x4b434150 || word(8) !== 3 || word(12) !== HEADER_BYTES) return null;   // "OP1FPACK", format 3
  const osId = word(16), link = word(20), body = word(24), zeroed = word(32), lists = word(64), extra = word(68), version = word(72);
  if (HEADER_BYTES + body + lists + extra !== bytes.length) return null;      // the sizes the header gives are the file's
  let name = "";                                     // the pack's name as the loader takes it: printable bytes, up to the first other
  for (let at = 40; at < 56 && bytes[at] >= 0x20 && bytes[at] < 0x7f; at++) name += String.fromCharCode(bytes[at]);
  const asks = asksOf(word, link, body, word(92));
  return { name, version, osId, link, body, lists, zeroed, foot: body + Math.max(lists, zeroed), engines: asks.engines, effects: asks.effects };
}

// The engines and effects a card asks for: { engines, effects }, both null when the card or its list of asks does not
// lie in the body as it should. cardAt: where in the body the card stands (0 for none).
function asksOf(word, link, body, cardAt) {
  const none = { engines: 0, effects: 0 }, unread = { engines: null, effects: null };
  if (!cardAt) return none;
  if (cardAt % 4 || cardAt + 12 > body || word(HEADER_BYTES + cardAt) !== CARD_MARK) return unread;
  const size = word(HEADER_BYTES + cardAt + 4), level = word(HEADER_BYTES + cardAt + 8);
  if (size < CARD_LEVEL_4_BYTES || level < 4) return none;                    // asks came with level 4
  if (cardAt + CARD_LEVEL_4_BYTES > body) return unread;
  const list = word(HEADER_BYTES + cardAt + CARD_ASKS_AT), count = word(HEADER_BYTES + cardAt + CARD_ASKS_AT + 4);
  if (!count) return none;
  const at = list - link;                            // the list's address is as linked
  if (count > ASKS_MOST || at < 0 || at % 4 || at + ASK_BYTES * count > body) return unread;
  let engines = 0, effects = 0;
  for (let k = 0; k < count; k++) {
    const kind = word(HEADER_BYTES + at + ASK_BYTES * k);
    if (kind === ASK_ENGINE) engines++;
    else if (kind === ASK_EFFECT) effects++;
  }
  return { engines, effects };
}

// The room of the unit's loader, from the door's hello (door.mjs parseHello: its tail, or N1's numbers for a door that
// says none): { generation, first, bytes, files, osId }. first is the room's first address: null for a generation
// this page does not know, unless the page's own loader manifest has a room of the same bytes and files
// (reserve.pack_room). osId: the firmware the unit runs, as a pack's header names it.
export function roomOf(hello, manifest) {
  const h = hello || {};
  const generation = Number.isInteger(h.loaderGeneration) ? h.loaderGeneration : N1_LOADER.loaderGeneration;
  const bytes = Number.isInteger(h.packRoom) && h.packRoom > 0 ? h.packRoom : N1_LOADER.packRoom;
  const files = Number.isInteger(h.packsMost) && h.packsMost > 0 ? h.packsMost : N1_LOADER.packsMost;
  let first = ROOM_FIRST[generation] ?? null;
  const own = manifest && manifest.reserve ? manifest.reserve.pack_room : null;
  if (first === null && own && own.bytes === bytes && own.files === files && /^0x[0-9a-f]{1,8}$/i.test(String(own.base))) first = Number(own.base);
  return { generation, first, bytes, files, osId: Number.isInteger(h.osId) ? h.osId : null };
}

// The pack files after a change: the unit's, less those the plan takes off and those a file it sends takes the place
// of (the same name, whatever its capitals), and the files it sends. now: [{ file, sha256 }]; plan: packs-live.mjs's
// resolve().plan, { send: [{ name, sha256 }], remove: [names] }. Answers [{ file, sha256, sent }].
export function afterChange(now, plan) {
  const low = (s) => String(s).toLowerCase();
  const gone = new Set([...(plan.remove || []), ...(plan.send || []).map((s) => s.name)].map(low));
  return [...now.filter((e) => !gone.has(low(e.file))).map((e) => ({ file: e.file, sha256: e.sha256, sent: false })),
    ...(plan.send || []).map((s) => ({ file: s.name, sha256: s.sha256, sent: true }))];
}

// A set of files with what the page knows of each: feet is a Map sha256 -> footOf's answer, holding only the files the
// page has read. Answers [{ file, sha256, sent, known, facts }].
export function withFacts(set, feet) {
  return set.map((e) => {
    const known = !!e.sha256 && feet.has(e.sha256);
    return { file: e.file, sha256: e.sha256 ?? null, sent: !!e.sent, known, facts: known ? feet.get(e.sha256) : null };
  });
}

// The door's order of names: byte by byte, a name before every longer name it begins.
function before(a, b) {
  const n = Math.min(a.length, b.length);
  for (let i = 0; i < n; i++) if (a.charCodeAt(i) !== b.charCodeAt(i)) return a.charCodeAt(i) - b.charCodeAt(i);
  return a.length - b.length;
}
const word4 = (n) => n + ((4 - (n % 4)) % 4);
const at32 = (next, link) => next + ((((link - next) % PACK_ALIGN) + PACK_ALIGN) % PACK_ALIGN);

// Where the loader would place a set of pack files, counted as the door counts it before it lets a change go. In the
// order of the files' names, byte by byte. Of the files that hold one pack (by the name in their headers), the highest
// version, the first of equal ones; the others stand by. Each placed pack at the next address that is a multiple of 32
// bytes from where it was linked, the next one behind its foot rounded up to a word. Where the placed file is one that
// stays on the unit, the files of the same pack that stand by are counted too, the largest end kept: the loader takes
// one of them when the placed file's checksum fails. A file made for another firmware (room.osId) takes no room.
// set: withFacts's answer. room: roomOf's answer (its first address not null).
// Answers { bytes, placed, unknown, engines, effects, asksKnown }:
//   bytes     from the room's first byte to the end of the last pack: what the door holds to the room's bytes
//   placed    [{ file, pack, version, start, foot, sent, standsBy, engines, effects }] in the loader's order, start
//             counted from the room's first byte; a file that stands by follows the file placed for its pack
//   unknown   the names of the files the page does not hold, left out of every figure
//   engines, effects  the asks of the placed packs; asksKnown false when a file is not known or a card not read
export function placeSet(set, room) {
  const taken = set.filter((e) => e.known && e.facts && (room.osId === null || room.osId === undefined || e.facts.osId === room.osId));
  const order = taken.slice().sort((a, b) => before(a.file, b.file));
  const done = new Set(), placed = [];
  let next = room.first, end = room.first, engines = 0, effects = 0, asksKnown = set.every((e) => e.known);
  for (let oi = 0; oi < order.length; oi++) {
    const e = order[oi];
    if (done.has(e)) continue;
    const same = order.filter((k) => !done.has(k) && k.facts.name === e.facts.name);
    let best = e;
    for (const k of same) if (k.facts.version > best.facts.version) best = k;
    const place = at32(next, best.facts.link), foot = best.facts.foot;
    let last = place + foot, after = place + word4(foot);
    placed.push(entryOf(best, place - room.first, false));
    if (best.facts.engines === null) asksKnown = false;
    else { engines += best.facts.engines; effects += best.facts.effects; }
    for (const k of same) {
      if (k === best) continue;
      const at = at32(next, k.facts.link);
      placed.push(entryOf(k, at - room.first, true));
      if (best.sent) continue;                      // the door counts the others only where the placed file stays
      last = Math.max(last, at + k.facts.foot);
      after = Math.max(after, at + word4(k.facts.foot));
    }
    end = Math.max(end, last);
    next = after;
    for (const k of same) done.add(k);
  }
  return { bytes: end - room.first, placed, unknown: set.filter((e) => !e.known).map((e) => e.file), engines, effects, asksKnown };
}

function entryOf(e, start, standsBy) {
  return { file: e.file, pack: e.facts.name, version: e.facts.version, start, foot: e.facts.foot, sent: e.sent, standsBy, engines: e.facts.engines, effects: e.facts.effects };
}
