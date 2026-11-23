// open-field: the listing of packs, as the install page reads it, and the download of a ticked pack.
//
// The listing is catalog/packs.json of the repository; the host's pages put a copy beside the page as packs.json
// (.github/workflows/pages.yml). tools/catalog_check.py holds every entry to its file before it is merged; this
// module reads only what the page needs, and holds every download to its entry once more.
//
// A ticked pack is downloaded from its author's repository at the commit the listing names: a file of a repository at
// a commit is one a web page may read (raw.githubusercontent.com answers any page), and that address never changes
// when the author pushes again. The file is held to the entry's size and SHA-256 before it goes anywhere near the
// folder. This is the only place where the page loads anything from another site.
//
// The page's live list of the packs on a unit (packs-live.mjs) reads the listing through the same functions, and two
// more below: the entry for a file on the unit, and the versions of the files the page can recognise.
//
// A listed entry may follow a branch of its author's repository instead of naming a commit: it names no commit,
// SHA-256, size, places or level, as the file says those itself. When the page opens, each followed file of the unit's
// firmware is downloaded once (resolveFollowed), held to its entry (checkFollowed: its name, its firmware, a version at
// least the entry's, the packs, the level of the base and the folders it needs among those the listing allows), and
// kept: a ticked followed pack goes on the unit with exactly the bytes checked then. Such a file's new versions reach
// the page without a review, whoever its author; the page's row says where the file comes from.
//
// Runs in the browser and in Node (no Node APIs in this file), so that validate-install.mjs runs the same code.

import { sha256 } from "./install.mjs";
import { firmwareOf } from "./door.mjs";

export const LISTING_FILE = "packs.json";                     // beside the page, put there by the host's pages
export const RAW = "https://raw.githubusercontent.com/";       // the only site the page downloads from
export const NOTICE = "A pack is a program that can read and write the files on your unit: take packs only from people you trust.";

const REPOSITORY = /^https:\/\/github\.com\/([A-Za-z0-9](?:[A-Za-z0-9-]{0,37}[A-Za-z0-9])?)\/([A-Za-z0-9._-]{1,100})$/;
const COMMIT = /^[0-9a-f]{40}$/;
const SHA256 = /^[0-9a-f]{64}$/;
const ID = /^[\x20-\x7e]{1,15}$/;
const PATH_PART = /^[A-Za-z0-9 _.()+-]+$/;

function pathOk(path) {
  if (typeof path !== "string" || !/\.pack$/i.test(path)) return false;
  return path.split("/").every((part) => part && part !== "." && part !== ".." && PATH_PART.test(part));
}

// An id the page may name a file after on the unit: `<id>.pack`.
function idOk(id) {
  return typeof id === "string" && ID.test(id) && !/[\\/:*?"<>|]/.test(id) && !/^[ .]|[ .]$/.test(id);
}

// A branch an entry may follow: 1 to 100 of A-Z a-z 0-9 _ . -, not beginning with . or -, no .., not ending in . or
// .lock. No slash.
function branchOk(name) {
  return typeof name === "string" && /^[A-Za-z0-9_.-]{1,100}$/.test(name) && !/^[.-]/.test(name) && !name.includes("..") && !/(\.|\.lock)$/.test(name);
}
function branchOf(entry) {
  if (!branchOk(entry.follow)) throw new Error(entry.id + ": its branch is not a branch's name");
  return encodeURIComponent(entry.follow);
}

// A followed entry whose file was downloaded and held to it when the page opened (resolveFollowed): a listing read from
// a file cannot make one, as its bytes are kept beside it.
function taken(entry) {
  return entry.follow !== undefined && entry.followed === true && entry.kept instanceof Uint8Array;
}

// The link a listed entry's file is downloaded from: its repository, at its commit (or the head of the branch it
// follows), its path.
export function rawLink(entry) {
  const found = REPOSITORY.exec(entry.repository);
  if (!found) throw new Error(entry.id + ": its repository is not a GitHub repository's address");
  if (entry.follow !== undefined) return RAW + found[1] + "/" + found[2] + "/refs/heads/" + branchOf(entry) + "/" + entry.path.split("/").map(encodeURIComponent).join("/");
  return RAW + found[1] + "/" + found[2] + "/" + entry.commit + "/" + entry.path.split("/").map(encodeURIComponent).join("/");
}

// The name the pack's file takes in the folder's packs and on the unit.
export const fileName = (entry) => entry.id + ".pack";

// What the page offers from a listing, for the unit's firmware (the loader's manifest names it, "1.7.3"):
// {base, offered: Map id -> entry, categories: [{key, label, entries}], otherFirmware, problems}. Every author's
// entry is read alike: the page sets no pack apart by who made it.
// Only `listed` entries are offered, each once, under its first category; `bundled` base.pack is shown apart, as it
// goes on every unit. An entry the page cannot use (a field out of form) is left out and named in `problems`. A
// followed entry is offered once resolveFollowed has taken its file; one it has not is left out and named.
export function readListing(data, firmware) {
  if (!data || typeof data !== "object" || !Array.isArray(data.packs) || !data.categories || typeof data.categories !== "object") {
    throw new Error("the listing of packs is not in form");
  }
  const offered = new Map(), heldBack = new Set();
  const problems = [];
  let base = null, otherFirmware = 0;
  for (const entry of data.packs) {
    if (!entry || typeof entry !== "object") continue;
    if (entry.status === "bundled" && entry.id === "base") { base = entry; continue; }
    if (entry.status !== "listed") continue;
    const follows = entry.follow !== undefined, waiting = follows && !taken(entry);      // waiting: a followed file not taken
    const fine = idOk(entry.id) && REPOSITORY.test(entry.repository ?? "") && (follows ? branchOk(entry.follow) : COMMIT.test(entry.commit ?? "")) && pathOk(entry.path) &&
      (waiting || (SHA256.test(entry.sha256 ?? "") && Number.isInteger(entry.bytes) && entry.bytes > 0)) && Number.isInteger(entry.version) &&
      Array.isArray(entry.categories) && entry.categories.length > 0 && entry.categories.every((c) => c in data.categories) &&
      typeof entry.description === "string" && typeof entry.author === "string" && Array.isArray(entry.needs ?? []);
    if (!fine) { problems.push((typeof entry.id === "string" ? entry.id : "an entry") + ": not in form, left out"); continue; }
    if ([...offered.keys()].some((id) => id.toLowerCase() === entry.id.toLowerCase()) || heldBack.has(entry.id.toLowerCase())) { problems.push(entry.id + ": named twice, left out"); continue; }
    if (entry.firmware !== firmware) { otherFirmware++; continue; }
    // a followed file left out still holds its id: no later entry of the same name takes its place
    if (waiting) { heldBack.add(entry.id.toLowerCase()); problems.push(followedSaid(entry) + " was not read and checked when the page opened, left out"); continue; }
    offered.set(entry.id, entry);
  }
  const categories = Object.entries(data.categories).map(([key, label]) => ({ key, label: String(label), entries: [] }));
  for (const entry of offered.values()) categories.find((c) => c.key === entry.categories[0]).entries.push(entry);
  return { base, offered, categories: categories.filter((c) => c.entries.length), otherFirmware, problems };
}

// The packs a pack cannot run without, besides base.pack (which goes on every unit), as ids: {ids, missing}.
// `missing` names what a ticked pack needs and the listing does not offer.
export function withNeeds(listing, ticked) {
  const ids = new Set(), missing = new Set(), todo = [...ticked];
  while (todo.length) {
    const id = todo.pop();
    if (ids.has(id)) continue;
    const entry = listing.offered.get(id);
    if (!entry) { missing.add(id); continue; }
    ids.add(id);
    for (const need of entry.needs ?? []) if (need !== "base") todo.push(need);
  }
  return { ids, missing };
}

// The ticked packs that need `id`, besides itself.
export function neededBy(listing, ticked, id) {
  return [...ticked].filter((other) => other !== id && withNeeds(listing, [other]).ids.has(id));
}

// The listed entry whose file on the unit would be `name` (the unit's own file name, any capitals), or null.
export function entryForFile(listing, name) {
  const want = String(name).toLowerCase();
  for (const entry of listing.offered.values()) if (fileName(entry).toLowerCase() === want) return entry;
  return null;
}

// The versions of pack files the page can recognise on a unit by their SHA-256 alone: every entry of the listing for
// this firmware that names a file (whatever its status), and an entry's `earlier` files where the listing gives them
// ([{version, sha256}], older files of the same pack). Map sha256 -> {id, version}. The install page uses it to tell
// an older file of a pack ("an update is out") from a file it does not know ("a different file").
export function knownVersions(data, firmware) {
  const out = new Map();
  if (!data || !Array.isArray(data.packs)) return out;
  for (const entry of data.packs) {
    if (!entry || typeof entry !== "object" || entry.firmware !== firmware || typeof entry.id !== "string") continue;
    if (SHA256.test(entry.sha256 ?? "") && Number.isInteger(entry.version)) out.set(entry.sha256, { id: entry.id, version: entry.version });
    for (const old of Array.isArray(entry.earlier) ? entry.earlier : []) {
      if (old && SHA256.test(old.sha256 ?? "") && Number.isInteger(old.version) && !out.has(old.sha256)) out.set(old.sha256, { id: entry.id, version: old.version });
    }
  }
  return out;
}

// The one load from another site: a file of a repository at a commit (rawLink, pictureLink).
const fetchRaw = (url, options) => fetch(url, options);

// The bytes the host answers for `url`, or an error that says what went wrong (the caller names the pack). With
// `most`, a file the host says is larger is refused before it is read.
async function fileAt(url, get, most) {
  let answer;
  try { answer = await get(url, { credentials: "omit", redirect: "error" }); }
  catch (e) { throw new Error("could not be downloaded (" + (e.message || e) + ")"); }
  if (!answer.ok) throw new Error("the download answered " + answer.status + ", not the file");
  const told = most && answer.headers && typeof answer.headers.get === "function" ? Number(answer.headers.get("content-length")) : 0;
  if (told > most) throw new Error("it has " + told.toLocaleString("en-US") + " bytes, more than the " + most.toLocaleString("en-US") + " the page takes from a branch");
  return new Uint8Array(await answer.arrayBuffer());
}

// A listed entry's file, downloaded at its commit and held to the entry: {id, name, bytes, sha256, from}, or an error
// that names the pack and what was wrong. `get` is the browser's fetch; a test gives its own. A followed entry taken
// when the page opened gives back the bytes kept then, held to their SHA-256 again, and downloads nothing. A followed
// entry not taken yet is downloaded from its branch and held to the entry by checkFollowed, with the level of the base
// the listing offers (`baseLevel`; resolveFollowed gives it): the answer then has the file's `facts` too.
export async function download(entry, get = fetchRaw, baseLevel) {
  const url = rawLink(entry);
  if (entry.follow !== undefined && !taken(entry)) {
    if (!Number.isInteger(baseLevel)) throw new Error(followedSaid(entry) + " was not checked when the page opened: refused");
    let fresh;
    try { fresh = await fileAt(url, get, FOLLOWED_MOST); } catch (e) { throw new Error(followedSaid(entry) + " is refused: " + e.message); }
    const facts = checkFollowed(entry, fresh, baseLevel);
    return { id: entry.id, name: fileName(entry), bytes: fresh, sha256: await sha256(fresh), from: url, facts };
  }
  let bytes;
  if (taken(entry)) bytes = entry.kept.slice();           // the followed file checked when the page opened: no second download
  else try { bytes = await fileAt(url, get); } catch (e) { throw new Error(entry.id + ": " + e.message); }
  if (bytes.length !== entry.bytes) throw new Error(entry.id + ": the file downloaded has " + bytes.length + " bytes, the listing says " + entry.bytes + ": refused");
  const digest = await sha256(bytes);
  if (digest !== entry.sha256) throw new Error(entry.id + ": the file downloaded is not the one the listing names (its SHA-256 differs): refused");
  return { id: entry.id, name: fileName(entry), bytes, sha256: digest, from: url };
}

// ---- followed entries: a pack file's facts, the check of a followed file, and the entries resolved at load ----

export const FOLLOWED_MOST = 2 * 1024 * 1024;           // the largest file the page takes from a branch
const PACK_MAGIC = [0x4f, 0x50, 0x31, 0x46, 0x50, 0x41, 0x43, 0x4b];   // "OP1FPACK"
const FORMAT_LINKED = 3, HEADER_BYTES = 96, ENTRY_BYTES = 20, LIST_COUNTS = 6, NEEDED = 1;
const CARD_MARK = 0x44524143;                             // "CARD"
const CARD_2_BYTES = 48, CARD_3_BYTES = 56, FOLDER_BYTES = 36;
const LEVEL_MARK = /^op1f_level_([0-9]+)$/;               // the mark of a level of the base's interface
const NAME_MOST = 255;                                    // the longest name the page reads in a pack's lists (ours: 40 at most)

const CRC_TABLE = Uint32Array.from({ length: 256 }, (_, n) => {
  let c = n;
  for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
  return c >>> 0;
});
function crc32(bytes) {
  let c = 0xffffffff;
  for (const b of bytes) c = CRC_TABLE[(c ^ b) & 0xff] ^ (c >>> 8);
  return (c ^ 0xffffffff) >>> 0;
}
// Text of the bytes up to the first zero, one character a byte.
function textOf(bytes) {
  let out = "";
  for (const b of bytes) { if (!b) break; out += String.fromCharCode(b); }
  return out;
}

// What a pack file of format 3 says of itself, as tools/pack_build.py reads it (parse3, read_lists, card_in,
// folders_in): {name, version, osId, firmware, places, needs, level, offersLevel, card, folders}. `needs` are the packs
// it needs (works with, needed), `level` the highest level of the base it needs and `offersLevel` the highest it
// offers (null for none), `card` its card's level (null for none), `folders` [{name, place}] its card lists. A file
// that is no such pack throws an error that says what is wrong, in a plain sentence.
export function packFacts(bytes) {
  if (!(bytes instanceof Uint8Array) || bytes.length < HEADER_BYTES || PACK_MAGIC.some((b, n) => bytes[n] !== b)) throw new Error("it is not a pack file");
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const word = (at) => view.getUint32(at, true);
  if (word(8) !== FORMAT_LINKED || word(12) !== HEADER_BYTES) throw new Error("it is not a pack of format 3, the one the loader takes");
  const osId = word(16), link = word(20), bodyBytes = word(24), compare = word(28), crc = word(36);
  const table = word(56), tableEnd = word(60), listsBytes = word(64), extraBytes = word(68), version = word(72), cardPlace = word(92);
  if (HEADER_BYTES + bodyBytes + listsBytes + extraBytes !== bytes.length || bodyBytes % 4 || compare > bodyBytes) {
    throw new Error("its header's sizes do not add up to the file's " + bytes.length + " bytes");
  }
  if (crc32(bytes.subarray(HEADER_BYTES, HEADER_BYTES + bodyBytes + listsBytes)) !== crc) throw new Error("the CRC-32 of its body and lists is not its header's");
  if (!(link <= table && table <= tableEnd && tableEnd <= link + compare && table % 4 === 0 && (tableEnd - table) % ENTRY_BYTES === 0)) {
    throw new Error("its table of changes does not lie in its code and constant data");
  }
  const rawName = bytes.subarray(40, 56);
  let nameEnd = rawName.length;
  while (nameEnd > 0 && rawName[nameEnd - 1] === 0) nameEnd--;
  if (rawName.subarray(0, nameEnd).some((b) => b > 0x7f)) throw new Error("its name is not in ASCII");
  const name = Array.from(rawName.subarray(0, nameEnd), (b) => String.fromCharCode(b)).join("");
  const lists = readLists(bytes.subarray(HEADER_BYTES + bodyBytes, HEADER_BYTES + bodyBytes + listsBytes));
  // its card, where the header says: on a word, the card's mark, a size of whole words that ends in the body
  const bodyAt = HEADER_BYTES, inBody = (at) => word(bodyAt + at);
  let card = null, folders = [];
  if (cardPlace) {
    const stands = cardPlace % 4 === 0 && cardPlace + 8 <= bodyBytes && inBody(cardPlace) === CARD_MARK;
    const size = stands ? inBody(cardPlace + 4) : 0;
    if (!stands || size % 4 || size < 8 || size > bodyBytes - cardPlace) throw new Error("its card does not stand where its header says");
    const field = (k) => (k + 4 <= Math.min(size, CARD_2_BYTES) ? inBody(cardPlace + k) : 0);
    card = field(8);
    if (size >= CARD_3_BYTES && card >= 3) {
      const at = inBody(cardPlace + CARD_2_BYTES) - link, count = inBody(cardPlace + CARD_2_BYTES + 4);
      if (count && (at < 0 || at + FOLDER_BYTES * count > bodyBytes)) throw new Error("its card's list of folders does not lie in its body");
      for (let n = 0; n < count; n++) {
        const from = bodyAt + at + FOLDER_BYTES * n;
        folders.push({ name: textOf(bytes.subarray(from, from + 16)), place: textOf(bytes.subarray(from + 16, from + 32)) });
      }
    }
  }
  const levelOf = (names) => names.reduce((most, n) => { const m = LEVEL_MARK.exec(n); return m ? Math.max(most ?? 0, Number(m[1])) : most; }, null);
  return {
    name, version, osId, firmware: firmwareOf(osId), places: (tableEnd - table) / ENTRY_BYTES,
    needs: lists.works.filter((w) => w.needed).map((w) => w.name).sort(),
    level: levelOf(lists.imports.filter((i) => i.needed).map((i) => i.name)), offersLevel: levelOf(lists.exports.map((e) => e.name)),
    card, folders,
  };
}

// The lists behind a pack's body: the packs it works with, its offers and its needs, by name.
function readLists(lists) {
  const fault = (why) => new Error("its lists cannot be read: " + why);
  if (lists.length < 4 * LIST_COUNTS) throw fault("they are shorter than their counts");
  const view = new DataView(lists.buffer, lists.byteOffset, lists.byteLength);
  const word = (at) => view.getUint32(at, true);
  const [works, exports, imports, places, marks, stringBytes] = [0, 1, 2, 3, 4, 5].map((n) => word(4 * n));
  if (4 * LIST_COUNTS + 12 * works + 8 * exports + 8 * imports + 12 * places + 8 * marks + stringBytes !== lists.length) throw fault("their counts do not fit their size");
  const strings = lists.subarray(lists.length - stringBytes);
  const read = new Map();                                 // each name read once, however often the lists mention it
  const string = (offset) => {
    if (read.has(offset)) return read.get(offset);
    if (offset >= strings.length) throw fault("a name lies outside the strings");
    const room = strings.subarray(offset, offset + NAME_MOST + 1), end = room.indexOf(0);
    if (end < 0) throw fault(room.length > NAME_MOST ? "a name is longer than " + NAME_MOST + " characters" : "a name lies outside the strings");
    if (room.subarray(0, end).some((b) => b > 0x7f)) throw fault("a name is not in ASCII");
    const text = String.fromCharCode(...room.subarray(0, end));
    read.set(offset, text);
    return text;
  };
  let at = 4 * LIST_COUNTS;
  const out = { works: [], exports: [], imports: [] };
  for (let n = 0; n < works; n++, at += 12) out.works.push({ name: string(word(at)), needed: !!(word(at + 8) & NEEDED) });
  for (let n = 0; n < exports; n++, at += 8) out.exports.push({ name: string(word(at)) });
  for (let n = 0; n < imports; n++, at += 8) out.imports.push({ name: string(word(at)), needed: !!(word(at + 4) & NEEDED) });
  return out;
}

// How a refusal of a followed file begins: the pack, whose file and on which branch.
function followedSaid(entry) {
  return entry.id + ": its file on " + entry.author + "'s branch " + entry.follow;
}
const quoted = (text) => JSON.stringify(String(text));

// A followed entry's file held to the entry: its facts (packFacts) when it passes, or an error that names the pack, the
// branch and the one thing wrong. `baseLevel` is the level of the base the listing offers (its bundled base's
// offers_level).
export function checkFollowed(entry, bytes, baseLevel) {
  const refused = (why) => new Error(followedSaid(entry) + " is refused: " + why);
  if (bytes.length > FOLLOWED_MOST) throw refused("it has " + bytes.length.toLocaleString("en-US") + " bytes, more than the " + FOLLOWED_MOST.toLocaleString("en-US") + " the page takes from a branch");
  let facts;
  try { facts = packFacts(bytes); } catch (e) { throw refused(e.message); }
  if (facts.name !== entry.id) throw refused("its name is " + quoted(facts.name) + ", the listing's id is " + quoted(entry.id));
  if (facts.firmware !== entry.firmware) {
    throw refused(facts.firmware ? "it is built for OS " + facts.firmware + ", the listing says " + entry.firmware :
      "it is built for a firmware the page does not know (0x" + facts.osId.toString(16).toUpperCase().padStart(8, "0") + "), the listing says " + entry.firmware);
  }
  if (facts.version < entry.version) throw refused("its version is " + facts.version + ", the listing takes " + entry.version + " or higher");
  const allowed = new Set(entry.needs ?? []), extra = facts.needs.filter((n) => !allowed.has(n));
  if (extra.length) throw refused("it needs " + (extra.length === 1 ? "the pack " : "the packs ") + extra.map(quoted).join(", ") + ", which the listing's needs do not name");
  if ((facts.level ?? 0) > baseLevel) throw refused("it needs level " + facts.level + " of the base, and the listing's base offers " + baseLevel);
  const room = (entry.folders ?? []).map((f) => f && f.name + "\0" + f.place);
  const outside = facts.folders.find((f) => !room.includes(f.name + "\0" + f.place));
  if (outside) throw refused("its card asks for the folder " + quoted(outside.name) + " in " + quoted(outside.place) + ", which the listing's folders do not name");
  return facts;
}

// What a followed entry must hold before its file is downloaded: a reason it is out of form, or null.
function followedFault(entry) {
  for (const key of ["commit", "sha256", "bytes", "places", "level", "offers_level"]) {
    if (key in entry) return "it follows a branch and names its " + key + ", which only the file says";
  }
  if (!idOk(entry.id) || !REPOSITORY.test(entry.repository ?? "") || !pathOk(entry.path) || typeof entry.author !== "string") return "its id, repository, path or author is not in form";
  if (!branchOk(entry.follow)) return "its branch is not a branch's name";
  if (!Number.isInteger(entry.version) || entry.version < 0) return "its version is not a whole number";
  if (!Array.isArray(entry.needs ?? []) || !(entry.needs ?? []).every((n) => typeof n === "string")) return "its needs are not a list of names";
  if (!Array.isArray(entry.folders ?? []) || !(entry.folders ?? []).every((f) => f && typeof f.name === "string" && typeof f.place === "string")) return "its folders are not a list of names and places";
  return null;
}

export const FOLLOWED_WAIT = 20000;                     // how long the page waits for a followed file, in milliseconds

// The listing's data with each followed `listed` entry of this firmware resolved: its file downloaded once through
// `get` (download's own when none is given), held to the entry, and kept. A resolved entry is the listing's with the
// file's sha256, bytes, version, places, level and the packs it needs (among the entry's), `followed: true`, and the
// file's bytes as `kept`. An entry refused, or whose file did not come within `wait` milliseconds, is left as it is
// (readListing leaves it out), and its reason is one of `problems`. {data, problems}.
export async function resolveFollowed(data, firmware, get, wait = FOLLOWED_WAIT) {
  if (!data || typeof data !== "object" || !Array.isArray(data.packs)) return { data, problems: [] };
  const bases = data.packs.filter((e) => e && e.status === "bundled" && e.id === "base" && e.firmware === firmware && Number.isInteger(e.offers_level));
  const baseLevel = Math.max(0, ...bases.map((e) => e.offers_level));
  const problems = [];
  const packs = await Promise.all(data.packs.map(async (entry, n) => {
    if (!entry || typeof entry !== "object" || entry.status !== "listed" || entry.follow === undefined || entry.firmware !== firmware) return entry;
    const fault = followedFault(entry);
    if (fault) { problems[n] = (typeof entry.id === "string" ? entry.id : "an entry") + ": " + fault + ", left out"; return entry; }
    let timer;
    const late = new Promise((_, no) => { timer = setTimeout(() => no(new Error(followedSaid(entry) + " did not come within " + wait / 1000 + " s, left out")), wait); });
    try {
      const got = await Promise.race([download(entry, get, baseLevel), late]);
      return { ...entry, sha256: got.sha256, bytes: got.bytes.length, version: got.facts.version, places: got.facts.places, needs: got.facts.needs,
        ...(got.facts.level === null ? {} : { level: got.facts.level }), followed: true, kept: got.bytes };
    } catch (e) { problems[n] = e.message || String(e); return entry; }
    finally { clearTimeout(timer); }
  }));
  return { data: { ...data, packs }, problems: problems.filter((p) => p) };
}

// A listed entry's picture, if it names one in form: {path, sha256, bytes}, a PNG in the pack's repository (or null).
export const PICTURE_LARGEST = 100 * 1024;
export function pictureOf(entry) {
  const p = entry && entry.follow === undefined && entry.picture;   // a followed entry has no commit to take a picture at
  if (!p || typeof p !== "object" || typeof p.path !== "string" || !/\.png$/i.test(p.path) || !SHA256.test(p.sha256 ?? "") ||
      !Number.isInteger(p.bytes) || p.bytes <= 0 || p.bytes > PICTURE_LARGEST) return null;
  if (!p.path.split("/").every((part) => part && part !== "." && part !== ".." && PATH_PART.test(part))) return null;
  return p;
}

// The link of an entry's picture: the same repository, at the same commit, as its pack.
export function pictureLink(entry) {
  const found = REPOSITORY.exec(entry.repository);
  if (!found || !COMMIT.test(entry.commit ?? "") || !pictureOf(entry)) throw new Error(entry.id + ": no picture in form");
  return RAW + found[1] + "/" + found[2] + "/" + entry.commit + "/" + entry.picture.path.split("/").map(encodeURIComponent).join("/");
}

// An entry's picture, downloaded at its commit and held to its size, its SHA-256 and the PNG signature: the bytes,
// or an error. The page shows it only then, and the category's pictogram otherwise.
export async function downloadPicture(entry, get = fetchRaw) {
  const url = pictureLink(entry), picture = entry.picture;
  const answer = await get(url, { credentials: "omit", redirect: "error" });
  if (!answer.ok) throw new Error(entry.id + ": the picture's download answered " + answer.status);
  const bytes = new Uint8Array(await answer.arrayBuffer());
  if (bytes.length !== picture.bytes) throw new Error(entry.id + ": the picture has " + bytes.length + " bytes, the listing says " + picture.bytes);
  if (await sha256(bytes) !== picture.sha256) throw new Error(entry.id + ": the picture is not the one the listing names (its SHA-256 differs)");
  if (![0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a].every((b, i) => bytes[i] === b)) throw new Error(entry.id + ": the picture is not a PNG");
  return bytes;
}
