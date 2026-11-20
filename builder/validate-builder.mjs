// Checks the builder (patch.mjs) with your own firmware file. Nothing is uploaded and nothing here holds firmware.
//
//   node builder/validate-builder.mjs --stock <your own os.ldr> [--out <file to write the built image to>]
//   node builder/validate-builder.mjs --loader <os>-<version> --stock <your own os.ldr>
//   OPEN_FIELD_STOCK=<your own os.ldr> node builder/validate-builder.mjs
//
// Without --loader the manifest is loader.json: the version the page offers by itself. With it, a version by its
// name: --loader 1.7.3-v2 takes loader-1.7.3-v2.json, the manifest of a version kept beside that one, as
// index.html?loader=1.7.3-v2 does; or loader.json, when that is the version it names. The builder's file is the
// loader, our small piece of code inside the firmware file.
//
// The main check: your os.ldr, the manifest and the loader's file must give the image whose SHA-256 the
// manifest names, byte for byte. buildImage refuses to hand back anything else, so a fault in the builder is a
// refusal and never a bad image. Then the refusals: a wrong hash, a changed or cut file, a swapped loader's file,
// malformed fields. Without a firmware file only the checks that need none are made, and the answer says so.
import { readFileSync, existsSync, writeFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import { buildImage, sha256hex, ours } from "./patch.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const u8 = (p) => new Uint8Array(readFileSync(p));
let fails = 0, checks = 0, skipped = 0;
const ok = (m) => { console.log("  ok   " + m); checks++; };
const bad = (m) => { console.error("  FAIL " + m); fails++; };

const args = process.argv.slice(2);
const arg = (name) => { const at = args.indexOf(name); return at >= 0 ? args[at + 1] : undefined; };
const stockPath = arg("--stock") || process.env.OPEN_FIELD_STOCK;
const outPath = arg("--out");

// which manifest: loader.json, or the one of the version asked for, kept beside it or loader.json itself
const asked = arg("--loader");
if (asked !== undefined && !/^[0-9]+(\.[0-9]+)*-v[0-9]+$/.test(asked)) { console.error("no such loader: " + asked); process.exit(2); }
let manName = "loader.json";
if (asked !== undefined && existsSync(join(here, "loader-" + asked + ".json"))) manName = "loader-" + asked + ".json";
else if (asked !== undefined) {
  const own = JSON.parse(readFileSync(join(here, "loader.json"), "utf8"));
  if (`${own.os}-${ours(own).version}` !== asked) { console.error("no manifest for the loader " + asked + " in builder/"); process.exit(2); }
}
const manPath = join(here, manName);
const manText = readFileSync(manPath, "utf8");
const manifest = JSON.parse(manText);
const our = ours(manifest);
const loader = u8(join(here, our.file));

// 0. the install page's script parses: a slip in it stops the page before it loads anything, and no other check runs it
{
  const page = readFileSync(join(here, "index.html"), "utf8");
  const script = page.match(/<script type="module">([\s\S]*)<\/script>/);
  const { spawnSync } = await import("node:child_process");
  const { tmpdir } = await import("node:os");
  const file = join(tmpdir(), "open-field-page-" + process.pid + ".mjs");
  if (script) writeFileSync(file, script[1]);
  const parsed = script && spawnSync(process.execPath, ["--check", file], { encoding: "utf8" });
  if (parsed && parsed.status === 0) ok("the install page's script parses");
  else bad("the install page's script does not parse: " + (parsed ? parsed.stderr.split("\n").slice(0, 5).join(" ") : "no module script found"));
}

// 1. the loader's file is the one the manifest names
if (loader.length === our.bytes && (await sha256hex(loader)) === our.sha256) ok(`the loader's file is the manifest's (${loader.length} bytes)`);
else bad("the loader's file is not the one the manifest names");

// 2. the manifest holds no bytes of the firmware: no before/after field, no long run of hex but the three hashes
if (/"before"|"after"/.test(manText)) bad("the manifest holds a before/after field");
else {
  const longHex = (manText.match(/[0-9a-f]{16,}/g) || []).filter((h) => h.length !== 64);
  if (longHex.length) bad("the manifest holds a long run of hex that is no SHA-256: " + longHex[0]);
  else ok("the manifest holds no run of the firmware's bytes");
}

// 3. the loader's file is the loader alone, inside the loader's 64 KiB: the packs, base.pack among them, are files on
// the unit and are loaded behind it, never carried in the firmware file
if (loader.length > 0x10000) bad("the loader's file is larger than the loader's 64 KiB");
else ok(`the loader's file is the loader alone (${loader.length} bytes of its 64 KiB)`);

// 4. the manifest says whether its image has run on a unit: the page shows that line beside the version
if (typeof our.status !== "string" || !our.status) bad("the manifest does not say whether its image has run on a unit");
else ok(`status line: "${our.status}"`);

// 5. base.pack, which the page puts on every unit it installs, is the file base-pack.json names, as the page holds it
const baseText = readFileSync(join(here, "base-pack.json"), "utf8");
const baseNamed = JSON.parse(baseText);
const basePack = existsSync(join(here, baseNamed.file ?? "")) ? u8(join(here, baseNamed.file)) : null;
if (baseNamed.file !== "base.pack" || !basePack) bad("base-pack.json does not name base.pack beside it");
else if (basePack.length !== baseNamed.bytes || (await sha256hex(basePack)) !== baseNamed.sha256) bad("base.pack is not the file base-pack.json names");
else ok(`base.pack is the one base-pack.json names (${basePack.length} bytes, version ${baseNamed.version}, release ${baseNamed.release})`);

// 6. base-pack.json holds nothing but what names the file: no run of hex but its SHA-256
const baseHex = (baseText.match(/[0-9a-f]{16,}/g) || []).filter((h) => h.length !== 64);
if (baseHex.length || !Number.isInteger(baseNamed.version) || typeof baseNamed.release !== "string") bad("base-pack.json is not a name, a size, a SHA-256, a version and a release");
else ok("base-pack.json names base.pack and holds no run of the firmware's bytes");

if (!stockPath || !existsSync(stockPath)) {
  skipped = 9;
  console.log(`\n${fails === 0 ? "OK" : "FAILED"}: ${checks} checks, ${fails} failures; ${skipped} more need your own os.ldr (--stock FILE) and were not made`);
  process.exit(fails === 0 ? 0 : 1);
}

const stock = u8(stockPath);

// 7. the main check
try {
  const { image, report } = await buildImage(stock, manifest, loader);
  if (image.length !== manifest.output.bytes) bad(`the image is ${image.length} bytes, not the manifest's ${manifest.output.bytes}`);
  else if ((await sha256hex(image)) !== manifest.output.sha256) bad("the image's SHA-256 is not the manifest's");
  else {
    ok(`loader ${our.version}: the image is the one the manifest names (${image.length} bytes, SHA-256 ${report.output_sha256.slice(0, 12)}…)`);
    if (outPath) {
      if (existsSync(outPath)) bad(`${outPath} exists: nothing is overwritten`);
      else { writeFileSync(outPath, image, { flag: "wx" }); ok(`written to ${outPath}`); }
    }
  }
} catch (e) { bad("the build threw: " + e.message); }

// 8. a manifest with another expected output hash must be refused
try {
  const tampered = JSON.parse(JSON.stringify(manifest));
  tampered.output.sha256 = "0".repeat(64);
  await buildImage(stock, tampered, loader);
  bad("a wrong output hash was NOT refused");
} catch (e) { ok("refuses a build whose hash is not the manifest's"); }

// 9. a firmware file with one bit changed must be refused
try {
  const bent = stock.slice(); bent[1000] ^= 1;
  await buildImage(bent, manifest, loader);
  bad("a changed firmware file was NOT refused");
} catch { ok("refuses a firmware file that is not the exact 1.7.3 file"); }

// 10. a firmware file cut short must be refused
try {
  await buildImage(stock.slice(0, stock.length - 10), manifest, loader);
  bad("a firmware file cut short was NOT refused");
} catch { ok("refuses a firmware file cut short"); }

// 11. a swapped loader's file must be refused
try {
  const other = loader.slice(); other[0] ^= 0xff;
  await buildImage(stock, manifest, other);
  bad("a swapped loader's file was NOT refused");
} catch { ok("refuses a loader's file that is not the manifest's"); }

// 12. a Node Buffer handed in must not be changed (Buffer.slice() is a view, not a copy)
try {
  const buf = readFileSync(stockPath);
  const before = Buffer.from(buf);
  await buildImage(buf, manifest, loader);
  if (Buffer.compare(buf, before) !== 0) bad("buildImage changed the caller's Buffer");
  else ok("does not change a Buffer handed to it");
} catch (e) { bad("the build from a Buffer threw: " + e.message); }

// 13. a new value wider than 32 bits must be refused, not cut to 32 bits
try {
  const m = JSON.parse(JSON.stringify(manifest));
  m.edits.find((e) => e.kind === "imm32").new_imm = "0x101200000";
  await buildImage(stock, m, loader);
  bad("a 33-bit value was NOT refused");
} catch { ok("refuses a value wider than 32 bits"); }

// 14. a hook length the builder does not know must be refused
try {
  const m = JSON.parse(JSON.stringify(manifest));
  m.edits.find((e) => e.kind === "call").length = 16;
  await buildImage(stock, m, loader);
  bad("a 16-byte hook length was NOT refused");
} catch { ok("refuses a hook length it does not know"); }

// 15. a malformed hex field must be refused
try {
  const m = JSON.parse(JSON.stringify(manifest));
  m.edits[0].address = m.edits[0].address + "zz";
  await buildImage(stock, m, loader);
  bad("a malformed hex address was NOT refused");
} catch { ok("refuses a malformed hex field"); }

console.log(`\n${fails === 0 ? "OK" : "FAILED"}: ${checks} checks, ${fails} failures`);
process.exit(fails === 0 ? 0 : 1);
