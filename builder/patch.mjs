// open-field: build the patched os.ldr in the browser, from the owner's own stock os.ldr, a manifest and the
// loader's file. A faithful port of the parts of tools/ldr_build.py that the open-field loader needs: the
// two edits (the heap reservation and the load-point hook), the appended loader record and the rewritten
// FIRST header. See builder_design.md. Portions derived from open-xy (MIT).
//
// The words: the loader is our small piece inside the firmware file; the base is the shared pack every other
// pack relies on, the file base.pack, which the builder does not touch. A manifest written before step M4
// names the loader's file under the key "base", the word of its day; ours() reads either.
//
// The manifest holds no stock bytes: the builder reads the touched bytes from the owner's file, checks only
// their *shape*, and the whole-file SHA-256 pins the rest. The decisive safeguard is the output-hash gate:
// buildImage refuses to return any image whose SHA-256 is not the manifest's, so a port bug is a refusal,
// never a bad image.

import { req, header, parseLdr, mappedOffset, FIRST_FLAGS, FINAL_FLAGS, RECORD_FLAGS } from "./ldr.mjs";

// A strict 32-bit unsigned from a manifest field: a "0x…" string of 1–8 hex digits, or a plain integer.
// Rejects trailing junk, NaN, negatives and overflow (parseInt would accept "0x12zz" and overflow silently).
function hexInt(v) {
  let n;
  if (typeof v === "string") { req(/^0x[0-9a-fA-F]{1,8}$/.test(v), "not a 32-bit hex field: " + v); n = parseInt(v, 16); }
  else n = v;
  req(Number.isInteger(n) && n >= 0 && n <= 0xffffffff, "32-bit field out of range: " + v);
  return n;
}

// What a manifest says of our file in the firmware file: under "loader" since step M4, under "base" before.
export function ours(manifest) {
  const entry = manifest.loader ?? manifest.base;
  req(entry && typeof entry === "object", "the manifest names no loader");
  return entry;
}

export async function sha256hex(bytes) {
  const buf = await crypto.subtle.digest("SHA-256", bytes);
  return [...new Uint8Array(buf)].map((b) => b.toString(16).padStart(2, "0")).join("");
}

function pack4(h0, h1, h2, h3) {
  const out = new Uint8Array(8);
  new DataView(out.buffer).setUint16(0, h0 & 0xffff, true);
  new DataView(out.buffer).setUint16(2, h1 & 0xffff, true);
  new DataView(out.buffer).setUint16(4, h2 & 0xffff, true);
  new DataView(out.buffer).setUint16(6, h3 & 0xffff, true);
  return out;
}

// The stock `Rn = imm32` instruction (`DA0n hi lo 0000`) with its immediate replaced (ldr_build pointer_bytes).
// `site` is the 8 stock bytes read from the owner's file; its register is kept, its shape is checked.
function imm32Edit(site, newImm) {
  req(site[1] === 0xda && (site[0] & 0xf0) === 0 && site[6] === 0 && site[7] === 0, "not an Rn = imm32 site");
  return pack4(0xda00 | (site[0] & 0x0f), (newImm >>> 16) & 0xffff, newImm & 0xffff, 0);
}

// An absolute CALL.a / JUMP.a (8 or 12 bytes), zero-padded (ldr_build hook_bytes, absolute form). Only 8
// and 12 are valid, as in the Python tool; 4/6 would be a different (PC-relative) encoding, not ported.
function callAbs(target, length, call) {
  req(length === 8 || length === 12, "unsupported hook footprint " + length);
  req(target % 2 === 0, "odd hook target");
  const head = pack4(call ? 0xde00 : 0xdc00, (target >>> 16) & 0xffff, target & 0xffff, 0);
  if (length === 8) return head;
  const out = new Uint8Array(length);
  out.set(head, 0);
  return out;
}

// A manifest of a loader that has the room to itself (since loader v2, the loader N2) names where the loader puts
// the packs, reserve.pack_room {base, bytes, files}, and the end of the loader with its zeroed data, loader.end (its
// __module_end). The three refusals of tools/ldr_build.py for such a loader: the heap ends at the loader's first
// byte (heap_fill_base + the imm32 edit's new heap size = module_base), the loader's reservation lies inside the
// zero-filled heap record (the room for packs, pack_room, is not in the heap and is only read for its numbers), and the loader with its zeroed data fits the room. A manifest without pack_room is not looked at here.
function checkOwnRoom(manifest, our, loader) {
  const reserve = manifest.reserve;
  if (!reserve || typeof reserve !== "object" || !("pack_room" in reserve)) return;
  const room = reserve.pack_room;
  req(room && typeof room === "object" && !Array.isArray(room), "the manifest's pack_room is not {base, bytes, files}");
  hexInt(room.base);
  for (const field of ["bytes", "files"])
    req(Number.isInteger(room[field]) && room[field] > 0 && room[field] <= 0xffffffff, "the manifest's pack_room." + field + " is not a count above 0");
  req(our.end !== undefined, "the manifest has a pack_room and names no loader.end");
  const end = hexInt(our.end);
  const low = hexInt(reserve.module_base), fillLow = hexInt(reserve.heap_fill_base);
  const reserved = reserve.reserved_bytes, fillBytes = reserve.heap_fill_bytes;
  req(Number.isInteger(reserved) && reserved > 0 && Number.isInteger(fillBytes) && fillBytes > 0, "the manifest's reservation sizes are not counts");
  const heaps = (manifest.edits || []).filter((edit) => edit.kind === "imm32");
  req(heaps.length === 1, "a loader with its own room needs exactly one heap (imm32) edit");
  const heapEnd = fillLow + hexInt(heaps[0].new_imm);
  req(heapEnd === low, "the heap does not end at the loader's first byte: it ends at 0x" + heapEnd.toString(16) +
      ", module_base is 0x" + low.toString(16));
  req(fillLow <= low && low + reserved <= fillLow + fillBytes, "the reservation is not inside the heap fill");
  req(end >= low + loader.length, "the loader's end (loader.end) lies before the end of its file");
  req(end - low <= reserved, "the loader with its zeroed data (to loader.end) does not fit its reservation");
}

// Put the image together and check its structure, without a look at manifest.output. Returns the image.
// Not for a page: nothing has held this image to a known hash yet. It is here for write-manifest.mjs, the
// tool that writes a manifest, which needs the image this code gives before its size and hash stand in
// one; and for buildImage below, which adds the gate.
export async function assembleImage(stock, manifest, loader) {
  const our = ours(manifest);
  checkOwnRoom(manifest, our, loader);
  req((await sha256hex(stock)) === manifest.stock.sha256 && stock.length === manifest.stock.bytes,
      "this is not the exact stock " + manifest.os + " os.ldr (hash or size differs)");
  req((await sha256hex(loader)) === our.sha256 && loader.length === our.bytes,
      "the loader's file does not match the manifest");

  const blocks = parseLdr(stock);
  req(blocks.length === manifest.stock.records, "unexpected stock record count");
  const first = blocks[0], final = blocks[blocks.length - 1];
  const entry = hexInt(manifest.stock.entry);
  req((first.code & 0xffff) === FIRST_FLAGS && first.count === 0 &&
      first.argument === stock.length - 16 && first.target === entry, "unsupported FIRST record");
  req((final.code & 0xffff) === FINAL_FLAGS && final.count === 0, "unsupported FINAL record");

  const low = hexInt(manifest.reserve.module_base);
  const stub = hexInt(our.stub);
  const patched = new Uint8Array(stock);        // always a copy: Buffer.slice() is a view, which would
                                                // mutate the caller's stock and make the check below vacuous
  const editRanges = [];
  let sawHeapEdit = false;
  for (const edit of manifest.edits) {
    const address = hexInt(edit.address);
    const length = edit.kind === "call" ? hexInt(edit.length) : 8;
    req(length === 8 || length === 12, "unsupported edit length " + length);
    const offset = mappedOffset(blocks, address, length);   // map the whole edit, not a fixed 8 bytes
    req(offset === hexInt(edit.file_offset), "manifest file offset differs from the mapped offset");
    for (const [a, b] of editRanges) req(offset >= b || offset + length <= a, "overlapping edits");
    let after;
    if (edit.kind === "imm32") {
      req(length === 8, "imm32 edit must be 8 bytes");
      after = imm32Edit(stock.subarray(offset, offset + 8), hexInt(edit.new_imm));   // read the owner's stock
      sawHeapEdit = true;
    } else if (edit.kind === "call") {
      const target = edit.target === "stub" ? stub : hexInt(edit.target);
      req(low <= target && target < low + loader.length, "hook does not target the loader");
      after = callAbs(target, length, true);
    } else {
      throw new Error("unknown edit kind " + edit.kind);
    }
    patched.set(after, offset);
    editRanges.push([offset, offset + after.length]);
  }
  req(sawHeapEdit, "appending a loader needs the heap reservation (imm32) edit");

  // the loader goes in a stored record just before FINAL (so it loads over the zero-fill it overlaps)
  req(loader.length > 0 && loader.length <= manifest.reserve.reserved_bytes, "invalid loader size");
  req(loader.length % 4 === 0, "loader length must be a multiple of 4");
  const fillBase = hexInt(manifest.reserve.heap_fill_base);
  const fills = blocks.filter((b) => b.target < low + manifest.reserve.reserved_bytes && low < b.target + b.count);
  req(fills.length === 1 && fills[0].fill && fills[0].argument === 0 && fills[0].target === fillBase &&
      fills[0].count === manifest.reserve.heap_fill_bytes && fills[0].target <= low &&
      low + manifest.reserve.reserved_bytes <= fills[0].target + fills[0].count,
      "the reservation is not inside the zero-filled heap record");

  const at = final.offset;
  const rec = header(RECORD_FLAGS, low, loader.length, 0);
  let image = new Uint8Array(patched.length + rec.length + loader.length);
  image.set(patched.subarray(0, at), 0);
  image.set(rec, at);
  image.set(loader, at + rec.length);
  image.set(patched.subarray(at), at + rec.length + loader.length);

  // rewrite FIRST with the new length
  image.set(header(FIRST_FLAGS, first.target, 0, image.length - 16), 0);

  // post-build checks
  const reparsed = parseLdr(image);
  req(reparsed.length === blocks.length + 1, "record count changed");
  for (let i = 0; i < 16; i++) req(image[image.length - 16 + i] === stock[stock.length - 16 + i], "FINAL record changed");

  // reconstruction: strip the loader record, then the result must equal stock outside [0,16) and the edits
  const restored = new Uint8Array(image.length - rec.length - loader.length);
  restored.set(image.subarray(0, at), 0);
  restored.set(image.subarray(at + rec.length + loader.length), at);
  req(restored.length === stock.length, "reconstruction length mismatch");
  const allowed = (i) => i < 16 || editRanges.some(([a, b]) => i >= a && i < b);
  for (let i = 0; i < stock.length; i++) req(restored[i] === stock[i] || allowed(i), "unauthorized difference at " + i);

  return image;
}

// Build the patched image. `stock`, `loader`: Uint8Array. `manifest`: the parsed loader.json. Returns
// { image, report }. Throws on any mismatch; in particular refuses to return an image whose hash is not
// the manifest's output.sha256.
export async function buildImage(stock, manifest, loader) {
  const image = await assembleImage(stock, manifest, loader);
  const our = ours(manifest);
  req(image.length === manifest.output.bytes, "output length is not the manifest's");
  const outHash = await sha256hex(image);
  req(outHash === manifest.output.sha256,
      "the built image's hash is not the expected one (refusing to output): got " + outHash);

  return {
    image,
    report: {
      os: manifest.os, loader: { version: our.version, sha256: our.sha256 },
      stock_sha256: manifest.stock.sha256, output_sha256: outHash, output_bytes: image.length,
      edits: manifest.edits.map((e) => ({ kind: e.kind, file_offset: e.file_offset })),
    },
  };
}

// Undo (recovering the stock os.ldr from a patched image) is a noted follow-up, not in this core. It cannot
// be done from the clean manifest alone: stripping the loader record and rewriting FIRST leaves the edited
// bytes at the two sites, and the original heap immediate is not in the manifest (we dropped it as redundant
// for the forward build). A correct undo needs either the original edit bytes recorded as numbers (the heap
// immediate 0x01400000 and the hook's displaced load, which the loader's stub already replays) or a different
// manifest. Until then the owner keeps their own stock os.ldr (the builder gates on saving it), or re-picks
// it from upgrade/firmware/blackfin/bin/os.ldr. (Detecting an already-patched file as input, to give a
// clearer message than the stock-SHA mismatch, is a small UI follow-up; buildImage does not do it today.)
