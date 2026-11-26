// Writes a version of the builder's file, the loader, and the manifest that goes with it. For a change to the loader:
// the loader is our small piece inside the firmware file, the one thing the builder adds to an owner's firmware, so a
// change to it needs a new such file and a manifest that names it (docs/build.md, "A change to the loader"). A change
// to the base, the pack base.pack, needs neither: the base is a file on the unit, never inside the firmware file.
//
//   node builder/write-manifest.mjs --loader <the built loader file> --elf <the loader's linked file> --version <v4> --stock <your own os.ldr>
//   OPEN_FIELD_STOCK=<your own os.ldr> node builder/write-manifest.mjs --loader ... --elf ... --version ...
//
// What it is given:
//   --loader   the loader's binary, as tools/build_all.py leaves it in its --out folder (build.json names it under blob)
//   --elf      the loader's linked file, build/<name>.elf: the address of the loader's entry is read from it, and the
//              loader's file is held to it, so that a linked file of another build is refused
//   --version  the new version's name: v and a number. A version that anyone may have installed keeps its bytes for
//              good, so a changed loader gets a new name
//   --stock    your own OS 1.7.3 file, or the environment's OPEN_FIELD_STOCK. It is read for one thing: the size and
//              the SHA-256 of the firmware file the builder gives with this loader. Nothing of it is written anywhere.
//              A file that is not the one the profile is for is refused
//
// More of what it is given:
//   --profile  the profile of the loader's room and of the two changes of the firmware file. Without it, the one
//              build.json names (profiles/op1f-1.7.3.json, the loader v1's). The loader v2 has a profile of its own,
//              profiles/op1f-1.7.3-n2.json: its room holds the loader alone, and its field pack_room says where the
//              loader puts the pack files. For such a profile the tool refuses three things: a heap that does not end
//              at the loader's first byte (heap_fill_base and the new heap size), a room that is not inside the
//              file's record of zeros, and a loader that does not fit its room with its zeroed data (__module_end in
//              the linked file). The manifest then also carries the pack room and the loader's end, so the page makes
//              the same checks
//
// What it writes, into this folder (or the one --out names):
//   loader-<os>-<version>.bin    the loader's file, byte for byte the one given
//   loader-<os>-<version>.json   its manifest: the loader's size and SHA-256, the address of its entry, the two
//                                changes of the firmware file, and the size and SHA-256 of the firmware file the
//                                builder gives. The page offers this version to someone who asks for it by name:
//                                index.html?loader=<os>-<version>
//   loader.json                  in place of the line above, when the page's own manifest already names this
//                                version, or with --current: the version the page offers by itself. Which version that
//                                is, is the maintainer's to say. The manifest that was there is then kept under its
//                                own name
//
// More:
//   --status "<a line>"   what the page shows beside the version: whether the firmware file has run on a unit. A new
//                         version says "not run on a unit yet"; a manifest written again keeps the line it has
//   --replace             write over a loader's file of this version name that holds other bytes. Only for a version of
//                         your own that nobody else has. A version whose manifest says that it has run on a unit
//                         is never written over
//   --check               write nothing: say whether the files that are there are the ones this tool gives, and answer
//                         1 when one is not. Without a firmware file it holds everything but the two things that
//                         need one (where the two changes lie in the file, and the firmware file's SHA-256)
//
// The firmware file is put together by the builder's own code (assembleImage in patch.mjs), the code the page runs:
// the SHA-256 in the manifest is that of the file the builder gives, and no second description of the build exists.
// What that proves and what it does not: the builder will give this file, and the file has the right structure. It
// does not prove that a unit starts with it. That takes a try on a unit.
//
// Answers 0 when it wrote or found everything as it should be, 1 when --check found a difference, 2 when it refused.
import { readFileSync, writeFileSync, existsSync, unlinkSync, mkdirSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join, basename, resolve } from "node:path";
import { parseLdr, mappedOffset } from "./ldr.mjs";
import { assembleImage, buildImage, sha256hex } from "./patch.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const NOT_RUN = "not run on a unit yet";

class Refusal extends Error {}
const need = (condition, message) => { if (!condition) throw new Refusal(message); };

function options(words) {
  const out = { flags: new Set() };
  const valued = new Set(["--loader", "--elf", "--version", "--stock", "--status", "--out", "--profile"]);
  const flags = new Set(["--current", "--replace", "--check"]);
  for (let at = 0; at < words.length; at++) {
    const word = words[at];
    if (flags.has(word)) out.flags.add(word);
    else if (valued.has(word)) {
      need(at + 1 < words.length, word + " wants a value");
      need(!(word in out), word + " is given twice");
      out[word] = words[++at];
    } else throw new Refusal("not an option of this tool: " + word);
  }
  return out;
}

const u8 = (path) => new Uint8Array(readFileSync(path));
const hex8 = (number) => "0x" + number.toString(16).padStart(8, "0");
const number = (text) => { need(/^0x[0-9a-fA-F]{1,8}$/.test(text), "not an address: " + text); return parseInt(text, 16); };

// The parts of a linked file (a 32-bit ELF file, the low byte first) this tool needs: where it says the code is
// entered, the sections that are loaded with their bytes, and the addresses of its global names.
function linkedFile(bytes) {
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  need(bytes.length >= 52 && view.getUint32(0, false) === 0x7f454c46 && bytes[4] === 1 && bytes[5] === 1, "the linked file is not a 32-bit ELF file");
  const entry = view.getUint32(24, true);
  const tableAt = view.getUint32(32, true), each = view.getUint16(46, true), count = view.getUint16(48, true);
  need(each >= 40 && tableAt + each * count <= bytes.length, "the linked file's table of sections is cut short");
  const sections = [];
  for (let index = 0; index < count; index++) {
    const at = tableAt + index * each;
    const section = { type: view.getUint32(at + 4, true), flags: view.getUint32(at + 8, true), address: view.getUint32(at + 12, true),
                      offset: view.getUint32(at + 16, true), size: view.getUint32(at + 20, true), link: view.getUint32(at + 24, true),
                      each: view.getUint32(at + 36, true) };
    need(section.type === 8 || section.offset + section.size <= bytes.length, "a section of the linked file is cut short");     // 8: no bytes in the file
    sections.push(section);
  }
  const names = new Map();
  for (const section of sections) {
    if (section.type !== 2) continue;                                             // 2: the table of names
    need(section.link < sections.length && section.each >= 16, "the linked file's table of names is malformed");
    const texts = sections[section.link];
    for (let at = section.offset; at + 16 <= section.offset + section.size; at += section.each) {
      const global = (bytes[at + 12] >> 4) === 1, defined = view.getUint16(at + 14, true) !== 0;
      if (!global || !defined) continue;
      let start = texts.offset + view.getUint32(at, true), end = start;
      while (end < texts.offset + texts.size && bytes[end]) end++;
      names.set(String.fromCharCode(...bytes.subarray(start, end)), view.getUint32(at + 4, true));
    }
  }
  const loaded = sections.filter((section) => section.type === 1 && (section.flags & 2) && section.size > 0);       // 1: bytes of the program; 2: loaded
  return { entry, names, loaded };
}

// The loader has the first 64 KiB of the room for our code; the packs are loaded behind it (linker/op1f-1.7.3-base.ld).
// A profile with pack_room gives the loader the whole room, reserved_bytes, and the packs a room of their own.
const LOADER_ROOM = 0x10000;

// A profile's pack_room, held to its form: {base, bytes, files}.
function packRoom(profile) {
  const room = profile.pack_room;
  need(room && typeof room === "object" && !Array.isArray(room), "the profile's pack_room is not {base, bytes, files}");
  const base = number(room.base);
  for (const field of ["bytes", "files"])
    need(Number.isInteger(room[field]) && room[field] > 0 && room[field] <= 0xffffffff, `the profile's pack_room.${field} is not a count above 0`);
  return { base: hex8(base), bytes: room.bytes, files: room.files };
}

// For a profile with pack_room, the loader alone is in the room: the heap ends at its first byte, the room lies in the
// file's record of zeros, and the loader with its zeroed data (to __module_end) fits the room.
function heldToRoom(profile, loader, end) {
  const low = number(profile.module_base), fillLow = number(profile.heap_fill_base);
  const fillHigh = fillLow + profile.heap_fill_bytes, reserved = profile.reserved_bytes;
  const heaps = profile.image.filter((item) => item.kind === "imm32");
  need(heaps.length === 1, "a profile with pack_room has one change of the heap's size (kind imm32)");
  const heapEnd = fillLow + number(heaps[0].new_imm);
  need(heapEnd === low, `the heap does not end at the loader's first byte: heap_fill_base ${hex8(fillLow)} and the new heap size ` +
       `${hex8(number(heaps[0].new_imm))} end at ${hex8(heapEnd)}, and module_base is ${hex8(low)}`);
  need(Number.isInteger(reserved) && reserved > 0 && Number.isInteger(profile.heap_fill_bytes) && fillLow <= low && low + reserved <= fillHigh,
       `the room [${hex8(low)}, ${hex8(low + reserved)}) is not inside the file's record of zeros [${hex8(fillLow)}, ${hex8(fillHigh)})`);
  need(end !== undefined, "the linked file names no __module_end: the loader with its zeroed data cannot be held to its room");
  need(end >= low + loader.length, `the linked file's __module_end, ${hex8(end)}, lies before the end of the loader's file: ` +
       "not the linked file of this loader, or not linked at module_base");
  need(end - low <= reserved, `the loader with its zeroed data is ${end - low} bytes (__module_end ${hex8(end)}), and its room is ${reserved}`);
}

// The manifest as an object, its fields in the order the page's manifest has had them from its first version.
// `places` is [{address, offset}] for the two changes when a firmware file was read, or null. `end` is the loader's
// __module_end, given for a profile with pack_room alone: such a manifest also carries reserve.pack_room and loader.end.
function composed(profile, os, loader, loaderHash, stub, version, status, places, end) {
  const edits = profile.image.map((item, index) => {
    const offset = places ? "0x" + places[index].toString(16) : null;
    if (item.kind === "imm32") return { kind: "imm32", address: item.address, file_offset: offset, new_imm: hex8(number(item.new_imm)) };
    need(item.kind === "call" && (item.length === 8 || item.length === 12), "the profile names a change of the file this builder does not know: " + item.kind);
    return { kind: "call", address: item.address, file_offset: offset, length: item.length, target: "stub" };
  });
  const reserve = { module_base: profile.module_base, reserved_bytes: profile.reserved_bytes, heap_fill_base: profile.heap_fill_base,
                    heap_fill_bytes: profile.heap_fill_bytes };
  const ourFile = { file: `loader-${os}-${version}.bin`, bytes: loader.length, sha256: loaderHash, stub: hex8(stub) };
  if ("pack_room" in profile) {
    reserve.pack_room = packRoom(profile);
    ourFile.end = hex8(end);
  }
  return {
    os,
    stock: { sha256: profile.stock_sha256, bytes: profile.stock_bytes, records: profile.stock_records, entry: profile.entry },
    reserve,
    loader: { ...ourFile, version, status },
    edits,
    output: { bytes: profile.stock_bytes + 16 + loader.length, sha256: null },
  };
}

const written = (manifest) => JSON.stringify(manifest, null, 2) + "\n";
const same = (a, b) => a.length === b.length && a.every((value, index) => value === b[index]);

async function main() {
  const given = options(process.argv.slice(2));
  const check = given.flags.has("--check");
  for (const wanted of ["--loader", "--elf", "--version"]) need(given[wanted], `${wanted} is needed (the first lines of this file say what each option is)`);
  const version = given["--version"];
  need(/^v[0-9]+$/.test(version), "a version's name is v and a number, such as v4: " + version);
  const stockPath = given["--stock"] || process.env.OPEN_FIELD_STOCK || "";
  need(stockPath || check, "your own OS 1.7.3 file is needed: --stock <your own os.ldr>, or OPEN_FIELD_STOCK. Without it the SHA-256 of the firmware file the builder gives cannot be worked out");
  need(!stockPath || existsSync(stockPath), "no such file: " + stockPath);
  for (const path of [given["--loader"], given["--elf"]]) need(existsSync(path), "no such file: " + path);
  if ("--status" in given) need(/^[\x20-\x7e]{1,80}$/.test(given["--status"]) && given["--status"].trim() === given["--status"], "the status is one short line of plain letters");
  const folder = given["--out"] || here;

  let profilePath;
  if ("--profile" in given) {
    profilePath = resolve(given["--profile"]);
    need(existsSync(profilePath), "no such file: " + given["--profile"]);
  } else {
    const plan = JSON.parse(readFileSync(join(here, "..", "build.json"), "utf8"));            // the tree's own profile
    profilePath = join(here, "..", ...plan.profile.split("/"));
  }
  const profile = JSON.parse(readFileSync(profilePath, "utf8"));
  const os = (profile.id || "").split("-").filter((word) => /^[0-9]+(\.[0-9]+)*$/.test(word)).pop();
  need(os, "the profile's id names no OS version: " + profile.id);
  for (const field of ["entry", "heap_fill_base", "heap_fill_bytes", "module_base", "reserved_bytes", "stock_sha256", "stock_bytes", "stock_records", "image"])
    need(field in profile, "the profile has no " + field);
  const roomed = "pack_room" in profile;                      // the loader alone in its room, the packs in a room of their own
  if (roomed) packRoom(profile);

  // the loader's file, and the linked file it was made from
  const loader = u8(given["--loader"]);
  const low = number(profile.module_base);
  need(loader.length > 0 && loader.length % 4 === 0, `the loader's file is ${loader.length} bytes: the builder takes a length that is a multiple of 4`);
  if (roomed) need(loader.length <= profile.reserved_bytes, `the loader's file is ${loader.length} bytes and its room is ${profile.reserved_bytes}`);
  else need(loader.length <= LOADER_ROOM, `the loader's file is ${loader.length} bytes and the loader has ${LOADER_ROOM}: the packs are loaded behind it. ` +
            "Give the file tools/build_all.py writes into its --out folder (build.json names it under blob)");
  const linked = linkedFile(u8(given["--elf"]));
  const end = linked.names.get("__module_end");
  if (roomed) heldToRoom(profile, loader, end);
  const call = profile.image.find((item) => item.kind === "call");
  need(call && call.target, "the profile names no function for the builder's call");
  need(linked.names.has(call.target), `the linked file has no function ${call.target}: it is not the loader's linked file`);
  const stub = linked.names.get(call.target);
  need(linked.entry === stub, `the linked file is entered at ${hex8(linked.entry)} and ${call.target} stands at ${hex8(stub)}: the two must be the same`);
  need(stub % 2 === 0 && low <= stub && stub < low + loader.length, `the loader's entry, ${hex8(stub)}, is not inside the loader's file`);
  need(linked.loaded.length > 0, "the linked file holds no code");
  const elf = u8(given["--elf"]);
  for (const section of linked.loaded) {
    const at = section.address - low;
    need(at >= 0 && at + section.size <= loader.length && same(elf.subarray(section.offset, section.offset + section.size), loader.subarray(at, at + section.size)),
         `the loader's file is not the one this linked file was made with (its bytes at ${hex8(section.address)} differ): give the two of one build`);
  }
  const loaderHash = await sha256hex(loader);

  // the manifest that is there for this version, and the page's own
  const currentPath = join(folder, "loader.json"), besidePath = join(folder, `loader-${os}-${version}.json`);
  const read = (path) => (existsSync(path) ? JSON.parse(readFileSync(path, "utf8")) : null);
  const current = read(currentPath), beside = read(besidePath);
  const isCurrent = current !== null && current.os === os && current.loader && current.loader.version === version;
  const asCurrent = given.flags.has("--current") || isCurrent;
  const before = isCurrent ? current : beside;                // this version's manifest as it is now, if there is one
  const status = given["--status"] || (before && before.loader.sha256 === loaderHash && before.loader.status) || NOT_RUN;

  // the firmware file the builder gives with this loader
  let places = null, manifest;
  if (stockPath) {
    const stock = u8(stockPath);
    need(stock.length === profile.stock_bytes && (await sha256hex(stock)) === profile.stock_sha256,
         `${stockPath} is not the OS ${os} file the profile is for (its size or its SHA-256 differs). Only that one file can be used`);
    try {
      const blocks = parseLdr(stock);
      places = profile.image.map((item) => mappedOffset(blocks, number(item.address), item.kind === "call" ? item.length : 8));
      manifest = composed(profile, os, loader, loaderHash, stub, version, status, places, end);
      const image = await assembleImage(stock, manifest, loader);
      need(image.length === manifest.output.bytes, "the firmware file is not the size it should be");
      manifest.output.sha256 = await sha256hex(image);
      const gated = await buildImage(stock, manifest, loader);    // the builder as the page calls it, with the manifest as it will be written
      need(same(gated.image, image), "the builder gave another file the second time");
    } catch (problem) {
      if (problem instanceof Refusal) throw problem;
      throw new Refusal("the builder does not take this loader: " + problem.message);
    }
  } else {
    manifest = composed(profile, os, loader, loaderHash, stub, version, status, null, end);
  }

  const loaderName = manifest.loader.file, loaderPath = join(folder, loaderName);
  const manifestName = asCurrent ? "loader.json" : basename(besidePath), manifestPath = asCurrent ? currentPath : besidePath;
  const loaderThere = existsSync(loaderPath) ? u8(loaderPath) : null;

  if (check) {
    let wrong = 0;
    const say = (fine, what) => { console.log(`${fine ? "the same " : "DIFFERS  "} ${what}`); if (!fine) wrong++; };
    say(loaderThere !== null && same(loaderThere, loader), `${loaderName}: ${loaderThere === null ? "it is not there" : "the loader's file, byte for byte"}`);
    const there = existsSync(manifestPath) ? readFileSync(manifestPath, "utf8") : null;
    if (there === null) say(false, `${manifestName}: it is not there`);
    else if (stockPath) say(there === written(manifest), `${manifestName}: the manifest, byte for byte`);
    else {
      const held = JSON.parse(there);
      for (const edit of held.edits || []) edit.file_offset = null;
      if (held.output) held.output.sha256 = null;
      say(written(held) === written(manifest), `${manifestName}: the manifest, but for what needs a firmware file`);
      console.log("not held  where the two changes lie in the firmware file, and the firmware file's SHA-256: they need your own os.ldr (--stock)");
    }
    return wrong ? 1 : 0;
  }

  // nothing is written before everything has been looked at
  if (loaderThere !== null && !same(loaderThere, loader)) {
    const said = before && before.loader.status;
    need(!said || said === NOT_RUN,
         `${loaderName} is there with other bytes, and its manifest says: ${said}. A version that has run on a unit keeps its bytes for good: give the changed loader a new name`);
    need(given.flags.has("--replace"),
         `${loaderName} is there with other bytes. A version that anyone may have installed keeps its bytes for good: give the changed loader a new name. ` +
         "If this version is your own and nobody else has it, say --replace");
  }
  let keep = null;                                            // the page's own manifest, kept under its own name when another version takes its place
  if (asCurrent && current !== null && !isCurrent) {
    need(current.os && current.loader && /^v[0-9]+$/.test(current.loader.version || ""), "the manifest loader.json that is there names no version: it is left alone");
    const keptName = `loader-${current.os}-${current.loader.version}.json`, keptPath = join(folder, keptName);
    const text = readFileSync(currentPath);
    need(!existsSync(keptPath) || same(u8(keptPath), new Uint8Array(text)), `${keptName} is there and is not the manifest loader.json holds: one of the two has to go first`);
    keep = { name: keptName, path: keptPath, text };
  }

  mkdirSync(folder, { recursive: true });
  const put = (path, name, bytes) => {
    const there = existsSync(path) ? u8(path) : null;
    if (there !== null && same(there, bytes)) { console.log(`as it was  ${name}`); return; }
    writeFileSync(path, bytes);
    console.log(`${there === null ? "written   " : "changed   "} ${name} (${bytes.length} bytes)`);
  };
  put(loaderPath, loaderName, loader);
  if (keep) put(keep.path, keep.name, new Uint8Array(keep.text));
  put(manifestPath, manifestName, new TextEncoder().encode(written(manifest)));
  if (asCurrent && existsSync(besidePath)) { unlinkSync(besidePath); console.log(`taken out  ${basename(besidePath)}: loader.json is this version's manifest now`); }

  console.log(`  loader ${version} for OS ${os}: ${loader.length} bytes, SHA-256 ${loaderHash}, entered at ${manifest.loader.stub}`);
  console.log(`  the firmware file the builder gives: ${manifest.output.bytes} bytes, SHA-256 ${manifest.output.sha256}`);
  console.log(`  the page shows beside it: loader ${version}, OS ${os}, ${status}`);
  console.log(asCurrent ? "  This is the version the page offers by itself (loader.json)."
                        : `  The page offers this version to someone who asks for it by name: index.html?loader=${os}-${version}. What it offers by itself is the version loader.json names.`);
  return 0;
}

// The answer is set and the program is left to end by itself. A forced end (process.exit) right after the last hash
// was seen to stand still now and then on Windows, with all its lines already written.
try {
  process.exitCode = await main();
} catch (problem) {
  if (!(problem instanceof Refusal)) throw problem;
  console.error("refused: " + problem.message);
  process.exitCode = 2;
}
