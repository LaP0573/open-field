// open-field: where the write buffer of te-boot's dev menu entry 4 lands, worked out from a plain
// file listing of the unit's volume. No memory reading. Runs in the browser (plain ES module, no Node
// APIs in this file) and in Node.
//
// The model was worked out for te-boot 1000.1.1 and held against real writes.
//
// te-boot 1000.2.0 has a profile beside 1000.1.1's (BOOTLOADERS, below). Its numbers are a contributor's reading,
// not read again by us, and one reported write on one unit holds them. Every verdict is for the bootloader the
// caller names; none is assumed, and a version that is not in BOOTLOADERS gets no verdict.
//
// Why it matters: entry 4 reads /os.ldr into a buffer from te-boot's general allocator (8-byte aligned),
// and its eMMC driver asserts the source is 32-byte aligned (rsi.c:311). The buffer lands 8 bytes behind
// the highest chunk of the heap in use, so its address modulo 32 follows from what te-boot allocated
// before it -- almost all of it the yaffs2 mount of the volume, which takes one record per file and
// folder and index nodes by file size, 100 at a time. So the file listing decides whether entry 4 writes
// or stops, and a file of a chosen size can move the buffer onto a 32-byte boundary.

// ---- constants (tools/teboot_heap.py, with the te-boot addresses it cites) ----

// The numbers the model takes from each bootloader it knows. Every other constant in this file, and every
// address cited beside one, is 1000.1.1's and is used for both.
export const BOOTLOADERS = Object.freeze({
  "1000.1.1": Object.freeze({
    heapBase: 0x88a1ca70,                    // the heap table's one entry (0x889e9df8)
    heapLength: 0x01fffffc,
    grow: 0x200,                             // the word at 0x889eeb8c
  }),
  // 1000.2.0's numbers are the reading of the contributor of open-field's pull request 10, who found them at the
  // addresses below in that bootloader's own code. They were not read again by us. One reported write on one
  // unit holds them (2026-11-20): it landed, with the pad file of 14,400,000 bytes that this profile chooses.
  "1000.2.0": Object.freeze({
    heapBase: 0x88a22b3c,                    // the heap table's one entry (0x889efec8)
    heapLength: 0x01fffffc,
    grow: 0x200,                             // the word at 0x889f4c5c
  }),
});

// The bootloader's version as the model keys it: "1000.2.0", or "1000.2.0+0" as te-boot gives it. Anything
// else, or nothing, throws a sentence the page can show: there is no default bootloader.
export function bootloaderVersion(version) {
  const key = typeof version === "string" ? version.replace(/\+0$/, "") : "";
  if (!Object.hasOwn(BOOTLOADERS, key)) {
    const given = version === undefined || version === null || version === "" ? "none" : String(version);
    throw new Error("the placement model knows the bootloaders " + Object.keys(BOOTLOADERS).join(" and ") + " only, and was given " + given);
  }
  return key;
}

export const ALIGN_WANTED = 32;              // rsi.c:311

const CHUNK_DATA = 4080;                      // 4096 less 16 bytes of tags inside the chunk (0x889c766c)
const OBJECT_BYTES = 0x88;                    // one record per file or folder (0x889c1cc6)
const TNODE_BYTES = 40;                       // one index node (0x889c7884)
const PER_BATCH = 100;                        // 0x889c1b9c, 0x889c1d90
const FAKE_OBJECTS = 4;                       // root, lost+found, unlinked, deleted (0x889c747c)
const WRAP = 35;                              // yaffs's / the USB stack's allocators add this (0x889bf302)

// The static constructors' requests, made before main (table at 0x8803d02c): a global string's exit
// entry, then for each of the four serial handlers two buffers (size + 31) and an exit entry (16 bytes).
const STATIC_CONSTRUCTORS = [
  0x10,
  0x20000 + 31, 0x1000 + 31, 0x10,
  0x2000 + 31, 0x1000 + 31, 0x10,
  0x100 + 31, 0x200 + 31, 0x10,
  0x2000 + 31, 0x1000 + 31, 0x10,
];

// Above the mount, in a visit that never opened the drive (the study's 12.11): te-boot asks 32 KiB and
// gives it back, then the main menu's start makes the USB stack's blocks from the mount's end up. 6,920
// bytes without a computer configuring the device, 9,544 with one. The browser is a computer on the
// cable, so a one-click tool uses WITH_COMPUTER. Both are 8 modulo 32, so the buffer is 16 modulo 32
// behind the mount's end either way.
export const USB_BASE = 6920;
export const USB_WITH_COMPUTER = 9544;

// The free rest at the heap's top when the mount begins: 96 after a start that asks nothing before it,
// 2768 when the charging-wait USB stack ran first. Above ~30 batches both give the same buffer modulo 32.
export const START_REST = 96;
export const SEEN_RESTS = [96, 2768];

// ---- integer helpers (JS bitwise is 32-bit signed; heap addresses exceed 2^31, so use arithmetic) ----

function alignDown(x, n) { return x - (x % n); }
function alignUp(x, n) { const r = x % n; return r === 0 ? x : x + (n - r); }

// need_of: te-boot's allocator rounds a request up (0x889eea40): (size + 15) & ~7, at least 16.
function needOf(size) { return Math.max(alignDown(size + 15, 8), 16); }
function wrappedNeed(size) { return needOf(size + WRAP); }

// ---- the heap, word for word as te-boot keeps it (the Heap class in teboot_heap.py) ----

class Memory {
  // Only 4-aligned 32-bit word reads/writes are ever used, so a Map keyed by address is exact; unwritten
  // words read as 0, as te-boot's zero-fill leaves them for the model's purposes.
  constructor() { this.words = new Map(); }
  r32(a) { return this.words.get(a) || 0; }
  w32(a, v) { this.words.set(a, v >>> 0); }
}

class Heap {
  constructor(bootloader) {
    const profile = BOOTLOADERS[bootloaderVersion(bootloader)];
    this.base = profile.heapBase;
    this.grow = profile.grow;
    this.mem = new Memory();
    this.start = alignDown(this.base + 0x23, 8);
    this.length = profile.heapLength - (this.start - this.base);
    const m = this.mem, b = this.base;
    m.w32(b, this.start); m.w32(b + 4, this.length); m.w32(b + 8, 0);
    m.w32(b + 0xc, this.start); m.w32(b + 0x10, 0); m.w32(b + 0x14, 0); m.w32(b + 0x18, 0);
    this.live = new Map();           // pointer -> the size its chunk was given
  }
  get brk() { return this.mem.r32(this.base + 0xc); }
  get rover() { return this.mem.r32(this.base + 0x14); }
  get head() { return this.mem.r32(this.base + 0x18); }

  _sbrk(count) {
    const room = this.start + this.length - this.brk;
    if (room < count) return null;
    const old = this.brk;
    this.mem.w32(this.base + 0xc, old + count);
    return old;
  }

  malloc(size) {
    const m = this.mem;
    let need = alignDown(size + 15, 8);
    if (need <= size) return 0;
    if (need < 16) need = 16;
    let link;
    for (;;) {
      link = this._find(need);
      if (link) break;
      let step = Math.max(this.grow, need);
      let block = null;
      while (block === null) {
        block = this._sbrk(step);
        if (block === null) { if (step === need) return 0; step = Math.max(step >> 1, need); }
      }
      m.w32(block, step); m.w32(block + 4, 0); this._free(block + 8);
    }
    const block = m.r32(link);
    const sizeThere = m.r32(block);
    let taken;
    if (sizeThere - 16 < need) {                 // 0x889eea96: too little left for a chunk of its own
      m.w32(link, m.r32(block + 8));
      taken = sizeThere;
    } else {                                     // 0x889eeaba: take the low end, the rest stays free
      const rest = block + need;
      m.w32(link, rest);
      m.w32(rest + 8, m.r32(block + 8));
      m.w32(rest, sizeThere - need);
      m.w32(rest + 4, m.r32(block + 4));
      m.w32(block, need);
      taken = need;
    }
    m.w32(this.base + 0x14, link);
    this.live.set(block + 8, taken);
    return block + 8;
  }

  _find(need) {
    const m = this.mem, headLink = this.base + 0x18;
    const rover = this.rover;
    if (rover === 0) {                           // from the head to the end
      let link = headLink;
      for (;;) {
        const block = m.r32(link);
        if (block === 0) return 0;
        if (need <= m.r32(block) && m.r32(block + 4) === 0) return link;
        link = block + 8;
      }
    }
    const first = m.r32(rover);                  // from the rover to the end
    let link = rover;
    for (;;) {
      const block = m.r32(link);
      if (block === 0) break;
      if (need <= m.r32(block) && m.r32(block + 4) === 0) return link;
      link = block + 8;
    }
    link = headLink;                             // from the head to where the search began
    for (;;) {
      const block = m.r32(link);
      if (block === first) return 0;
      if (need <= m.r32(block) && m.r32(block + 4) === 0) return link;
      link = block + 8;
    }
  }

  _free(pointer) {
    const m = this.mem;
    if (pointer === 0 || (pointer & 3)) return;
    let block = pointer - 8;
    const size = m.r32(block);
    if (size < 16 || (size & 7)) return;
    const head = this.head;
    if (head === 0 || block < head) {            // the new lowest block
      m.w32(block + 8, head);
      m.w32(this.base + 0x18, block);
    } else {
      let before = head;
      for (;;) {
        const after = m.r32(before + 8);
        if (after === 0 || !(after < block)) break;
        before = after;
      }
      const end = before + m.r32(before);
      if (block < end) return;
      if (block === end && m.r32(before + 4) === m.r32(block + 4)) {
        m.w32(before, m.r32(before) + size);     // joined to the block below
        block = before;
      } else {
        const after = m.r32(before + 8);
        if (after !== 0 && after < block + size) return;
        m.w32(block + 8, after);
        m.w32(before + 8, block);
      }
    }
    const after = m.r32(block + 8);              // joined to the block above, the rover dropped
    if (after !== 0 && after === block + m.r32(block) && m.r32(block + 4) === m.r32(after + 4)) {
      m.w32(this.base + 0x14, 0);
      m.w32(block, m.r32(block) + m.r32(after));
      m.w32(block + 8, m.r32(after + 8));
    }
  }

  free(pointer) {
    if (pointer === 0) return;
    if (!this.live.has(pointer)) throw new Error("free of a pointer the model did not hand out");
    this.live.delete(pointer);
    this._free(pointer);
  }

  calloc(count, size) { return this.malloc(count * size); }

  wrappedMalloc(size) {
    const raw = this.malloc(size + WRAP);
    if (raw === 0) return 0;
    const aligned = alignDown(raw + WRAP, 32);
    this.mem.w32(aligned - 4, raw);
    return aligned;
  }

  wrappedFree(aligned) { if (aligned) this.free(this.mem.r32(aligned - 4)); }

  topOfUse() {
    if (this.live.size === 0) return this.start;
    let best = 0;
    for (const [pointer, taken] of this.live) {
      const top = pointer - 8 + taken;
      if (top > best) best = top;
    }
    return best;
  }

  freeChunks() {
    const out = [];
    let block = this.head;
    while (block) { out.push([block, this.mem.r32(block)]); block = this.mem.r32(block + 8); }
    return out;
  }
}

// te-boot's start as it leaves the heap for the mount: the static constructors, then one free block at
// the top of `rest` bytes (heap_before_mount in teboot_heap.py).
function heapBeforeMount(rest, bootloader) {
  const heap = new Heap(bootloader);
  for (const size of STATIC_CONSTRUCTORS) heap.malloc(size);
  const top = heap.topOfUse();
  const free = heap.freeChunks();
  if (free.length !== 1 || free[0][0] !== top || free[0][1] !== heap.brk - top)
    throw new Error("the model's start does not end in one free block at the top");
  if (rest !== heap.brk - top) {
    if (rest < 16 || (rest % 8)) throw new Error("a free rest of " + rest + " bytes is no block of this heap");
    heap.mem.w32(top, rest);
    heap.mem.w32(heap.base + 0xc, top + rest);
  }
  return heap;
}

// The fixed allocations of yaffs_guts_initialise on an empty volume with a checkpoint (Mount.__init__ in
// teboot_heap.py, the checkpoint path with no files or folders). This is all mount_end_for needs before
// the batch loop, because the mount's end depends only on the batch counts.
function mountFixed(heap) {
  heap.calloc(1, 0xe94);                                     // the device record
  for (let i = 0; i < 6; i++) heap.wrappedMalloc(4096);      // six temporary buffers
  heap.wrappedMalloc(10 * 0x1c);                             // the cache table
  for (let i = 0; i < 10; i++) heap.wrappedMalloc(4096);     // ten cache buffers
  heap.wrappedMalloc(128 * 4);                               // the gc list
  // _tables_and_first_records: the two per-block tables, the allocator's record, the four fake objects
  heap.wrappedMalloc(4608 * 8);
  heap.wrappedMalloc(4608 * 16);
  heap.wrappedMalloc(0x24);
  // the four fake objects are the first batch of records: one batch + its list entry
  heap.wrappedMalloc(PER_BATCH * OBJECT_BYTES);
  heap.wrappedMalloc(8);
  heap.wrappedMalloc(127 * 12);                              // two block summaries
  heap.wrappedMalloc(127 * 12);
  heap.wrappedMalloc(4096);                                  // the checkpoint buffer
  heap.wrappedMalloc(4 * 289);                               // the checkpoint block list
}

// ---- the volume, from a listing ----

export function fileChunks(size) { return Math.floor((size + CHUNK_DATA - 1) / CHUNK_DATA); }

// The index nodes yaffs2 holds for a file of this size once its tree is whole (file_tnodes, 0x889c7884).
// Arithmetic division, not JS `>>`: a bit shift is 32-bit signed, so a chunk count at or above 2^31 (a
// size over ~8.7 TB, unreachable on real hardware but possible from a garbage listing row) would turn
// negative and loop forever. This clamps a negative size and matches the Python model for every size.
export function fileTnodes(size) {
  const last = fileChunks(Math.max(0, size));
  let total = Math.floor(last / 16) + 1;
  let x = Math.floor(last / 16);
  let div = 16;                       // 2^4, then 2^7, 2^10, ... (Python's shift 4, 7, 10 ...)
  while (x) { div *= 8; total += Math.floor(last / div) + 1; x = Math.floor(x / 8); }
  return total;
}

export function batchesOf(count) { return Math.ceil(count / PER_BATCH); }

// Count a listing: an array of {path, size, dir}. Returns records, index nodes and their batch counts.
// yaffs's own folder lost+found is skipped (te-boot's MTP shows it as a one-chunk file; the mount makes
// it itself as one of the four fake objects).
//
// `imageBytes`: the size of the os.ldr that will be on the volume at the write visit. The placement
// depends on the volume's node count AT THAT MOMENT, when os.ldr is present. A listing may be taken
// before os.ldr is copied, or may hold a different, older os.ldr; either makes the count wrong by the
// image's own index nodes (yaffs holds ~1 node per 65 KB, so a 2 MB image is ~38 nodes — enough to cross
// a batch edge and move the buffer). So when `imageBytes` is given, a top-level `os.ldr` row is counted
// with that size exactly once, whatever the listing holds. Pass it whenever the image to write is known.
export function countListing(rows, { imageBytes = null } = {}) {
  let files = 0, folders = 0, nodes = 0, sawOsLdr = false;
  for (const row of rows) {
    const rel = String(row.path || "").replace(/\\/g, "/").replace(/^\/+|\/+$/g, "");
    if (rel === "lost+found") continue;
    if (row.dir === true) { folders += 1; continue; }   // strictly a folder; anything else is a file
    files += 1;
    if (rel === "os.ldr" && imageBytes !== null && !sawOsLdr) { sawOsLdr = true; nodes += fileTnodes(imageBytes); continue; }
    const sz = Number(row.size);                          // a non-numeric or negative size counts as 0
    nodes += fileTnodes(Number.isFinite(sz) && sz > 0 ? sz : 0);
  }
  if (imageBytes !== null && !sawOsLdr) { files += 1; nodes += fileTnodes(imageBytes); } // os.ldr not yet copied
  const records = FAKE_OBJECTS + files + folders;
  return {
    files, folders, records, nodes,
    recordBatches: batchesOf(records),
    nodeBatches: batchesOf(nodes),
    imageCounted: imageBytes !== null,
    // When the index-node total is an exact multiple of 100, the physical mount may take one node batch
    // more than batchesOf(nodes) (a transient node at a file's restore, if the last file has a single
    // node); the model can't tell from a listing, so the verdict here is uncertain on the edge. Nudge the
    // volume a little (any file) to move off it. recommendPad never lands a pad here.
    onNodeBatchEdge: nodes > 0 && nodes % PER_BATCH === 0,
  };
}

// ---- where the mount ends, and where the buffer lands ----

// The mount's end for that many batches (mount_end_for in teboot_heap.py). The first record batch is the
// four fake objects, already made by mountFixed, so the loop adds nodeBatches node-batches and
// (recordBatches - 1) record-batches; the order does not change the answer.
export function mountEndFor(nodeBatches, recordBatches, { bootloader, rest = START_REST } = {}) {
  const heap = heapBeforeMount(rest, bootloader);
  mountFixed(heap);
  for (let i = 0; i < nodeBatches; i++) { heap.wrappedMalloc(PER_BATCH * TNODE_BYTES); heap.wrappedMalloc(8); }
  for (let i = 0; i < recordBatches - 1; i++) { heap.wrappedMalloc(PER_BATCH * OBJECT_BYTES); heap.wrappedMalloc(8); }
  return heap.topOfUse();
}

// Where entry 4's buffer stands in a visit that never opened the drive (plain_buffer in teboot_heap.py).
export function plainBuffer(mountEnd, withComputer = true) {
  return mountEnd + (withComputer ? USB_WITH_COMPUTER : USB_BASE) + 8;
}

// The buffer and whether it passes, for one listing's counts, over the power-on cases the unit shows.
// With a computer on the cable (the browser), and on any populated volume (above ~30 batches), the two
// rests agree, so `pass` is the honest verdict. `perRest` is there for the thin-volume case.
// `bootloader` is required: the unit's version, one of BOOTLOADERS.
export function placement(counts, { bootloader, withComputer = true, rests = SEEN_RESTS } = {}) {
  bootloader = bootloaderVersion(bootloader);
  const perRest = rests.map((rest) => {
    const end = mountEndFor(counts.nodeBatches, counts.recordBatches, { bootloader, rest });
    const buffer = plainBuffer(end, withComputer);
    return { rest, mountEnd: end, buffer, mod: buffer % ALIGN_WANTED };
  });
  const pass = perRest.every((r) => r.mod === 0);
  const anyPass = perRest.some((r) => r.mod === 0);
  return { perRest, pass, anyPass, agree: new Set(perRest.map((r) => r.mod)).size === 1 };
}

// ---- the pad file: a size that lands the buffer on a 32-byte boundary ----

// How many more items fit in a count's last batch of 100 before it tips into another batch (0..99).
function marginOf(count) { return PER_BATCH - (((count - 1) % PER_BATCH) + 1); }

// One extra file of `bytes` adds one record and fileTnodes(bytes) index nodes. This searches sizes in
// range for those that make the buffer pass for every power-on case, and returns the one that sits deepest
// inside BOTH its index-node batch and its record batch -- the most headroom before the unit's own log,
// which grows while it runs, tips either count into the next batch and moves the buffer again. The log
// adds index nodes fast (about 12 every 15 minutes on the unit it was measured on; another differs) and a record now
// and then, and either edge can move the write, so a safe pad keeps both away from an edge; `marginNodes`
// and `marginRecords` report the headroom. Returns null if nothing in range passes (not expected on a real
// volume). `step` and `max` bound the search; the defaults cover the sizes the unit has taken.
export function recommendPad(counts, { bootloader, withComputer = true, rests = SEEN_RESTS, minBytes = 1_000_000,
  maxBytes = 40_000_000, step = 100_000, minMarginRecords = 3 } = {}) {
  bootloader = bootloaderVersion(bootloader);
  let best = null, fallback = null;
  for (let bytes = minBytes; bytes <= maxBytes; bytes += step) {
    const nodes = counts.nodes + fileTnodes(bytes);
    const records = counts.records + 1;                 // the pad is one more record
    const next = { ...counts, nodes, records, nodeBatches: batchesOf(nodes), recordBatches: batchesOf(records) };
    if (!placement(next, { bootloader, withComputer, rests }).pass) continue;
    const marginNodes = marginOf(nodes), marginRecords = marginOf(records);
    if (marginNodes === 0) continue;                     // never land on the ambiguous node-batch edge
    const candidate = { bytes, counts: next, marginNodes, marginRecords };
    // prefer the most node headroom among pads whose record count is also clear of its edge; fall back to
    // the most node headroom overall if every passing pad sits near a record edge.
    if (marginRecords >= minMarginRecords) { if (best === null || marginNodes > best.marginNodes) best = candidate; }
    else if (fallback === null || marginNodes > fallback.marginNodes) fallback = candidate;
  }
  return best || fallback;
}

// ---- top-level: a listing in, the whole verdict out ----
//
// `imageBytes` (the size of the os.ldr to be written) should be passed whenever it is known: it makes the
// count reflect the write-visit volume with that exact image on it, whatever the listing holds (see
// countListing). Without it the verdict is only right if the listing was taken with the intended os.ldr
// already on the volume. `bootloader` (the unit's version, one of BOOTLOADERS) is required.
export function analyzeListing(rows, options = {}) {
  bootloaderVersion(options.bootloader);
  const counts = countListing(rows, options);
  const current = placement(counts, options);
  // onNodeBatchEdge: the verdict is uncertain because the mount may take an extra node batch (see
  // countListing). onEdge lets the caller warn and nudge the volume off the edge before trusting a pass.
  const result = { counts, current, needsPad: !current.pass, onEdge: counts.onNodeBatchEdge };
  if (!current.pass) result.pad = recommendPad(counts, options);
  return result;
}
