#!/usr/bin/env node

"use strict";

const crypto = require("crypto");
const fs = require("fs");
const path = require("path");
const zlib = require("zlib");

const gameDir = path.resolve(process.argv[2] || "..");
const files = {
  dll: "HD_HOTA.dll",
  language: path.join("Data", "HotA_lng.lod"),
  bitmap: path.join("Data", "H3bitmap.lod"),
  bitmapExpansion: path.join("Data", "H3ab_bmp.lod"),
  hdFilesIni: path.join("_HD3_Data", "Compability", "#hota", "Files.ini"),
  hdFilesIni15: path.join("_HD3_Data", "Compability", "#hota15", "Files.ini"),
};
const portraitNames = ["HPL004EL.pcx", "HPS004EL.pcx"];
const portraitAssets = portraitNames.map((name) =>
  fs.readFileSync(path.join(__dirname, "assets", name)),
);
const specialtyNames = ["UN32.def", "UN44.def", "IX44.def"];
const positionOffset = 0x234d9a;
const layoutOffset = 0x234dc3;
const finalPosition = Buffer.from("6a126a4e", "hex");
const currentLayout = Buffer.from(
  "8b45cc8b4830668b51306689501c668b51346689501e6683401a0a90909090909090",
  "hex",
);
const finalLayout = Buffer.from(
  "8b45cc8b4830668b51306689501c668b51346689501e66834018066683401a049090",
  "hex",
);

function hash(buffer) {
  return crypto.createHash("sha256").update(buffer).digest("hex");
}

function read(relativePath) {
  return fs.readFileSync(path.join(gameDir, relativePath));
}

function writeIfChanged(relativePath, buffer) {
  const destination = path.join(gameDir, relativePath);
  if (!fs.readFileSync(destination).equals(buffer)) {
    fs.writeFileSync(destination, buffer);
  }
}

function findLodEntry(archive, requestedName) {
  const count = archive.readUInt32LE(8);
  for (let index = 0; index < count; index += 1) {
    const directoryOffset = 92 + index * 32;
    const name = archive
      .subarray(directoryOffset, directoryOffset + 16)
      .toString("latin1")
      .replace(/\0.*$/, "");
    if (name.toLowerCase() === requestedName.toLowerCase()) {
      return {
        directoryOffset,
        offset: archive.readUInt32LE(directoryOffset + 16),
        size: archive.readUInt32LE(directoryOffset + 20),
        compressedSize: archive.readUInt32LE(directoryOffset + 28),
      };
    }
  }
  throw new Error(`LOD entry not found: ${requestedName}`);
}

function extractLodEntry(archive, name) {
  const entry = findLodEntry(archive, name);
  const storedSize = entry.compressedSize || entry.size;
  const stored = archive.subarray(entry.offset, entry.offset + storedSize);
  return entry.compressedSize ? zlib.inflateSync(stored) : Buffer.from(stored);
}

function replaceLodEntry(archive, name, replacement) {
  const entry = findLodEntry(archive, name);
  const updated = Buffer.from(archive);
  updated.writeUInt32LE(updated.length, entry.directoryOffset + 16);
  updated.writeUInt32LE(replacement.length, entry.directoryOffset + 20);
  updated.writeUInt32LE(0, entry.directoryOffset + 28);
  return Buffer.concat([updated, replacement]);
}

function patchPortraits(archive) {
  let updated = archive;
  for (let index = 0; index < portraitNames.length; index += 1) {
    const current = extractLodEntry(updated, portraitNames[index]);
    const expectedSize = index === 0 ? 4492 : 2316;
    if (current.length !== expectedSize) {
      throw new Error(`Unexpected ${portraitNames[index]} size.`);
    }
    if (!current.equals(portraitAssets[index])) {
      updated = replaceLodEntry(
        updated,
        portraitNames[index],
        portraitAssets[index],
      );
    }
  }
  return updated;
}

function patchLanguage(archive) {
  const hotraits = extractLodEntry(archive, "HOTRAITS.TXT").toString("latin1");
  const rows = hotraits.split(/\r?\n/);
  const index = rows.findIndex(
    (row) => row.startsWith("Inteus\t") || row.startsWith("Nyx\t"),
  );
  if (index < 0) {
    throw new Error("Inteus row was not found in HOTRAITS.TXT.");
  }
  rows[index] = rows[index].replace(/^Inteus\t/, "Nyx\t");

  const bios = extractLodEntry(archive, "HeroBios.txt").toString("latin1");
  const updatedBios = bios.replace(
    "Inteus's command of fire",
    "Nyx's command of fire",
  );
  const updatedTraits = Buffer.from(rows.join("\r\n"), "latin1");
  const updatedBiography = Buffer.from(updatedBios, "latin1");
  let updated = archive;
  if (!extractLodEntry(updated, "HOTRAITS.TXT").equals(updatedTraits)) {
    updated = replaceLodEntry(updated, "HOTRAITS.TXT", updatedTraits);
  }
  if (!extractLodEntry(updated, "HeroBios.txt").equals(updatedBiography)) {
    updated = replaceLodEntry(updated, "HeroBios.txt", updatedBiography);
  }
  return updated;
}

function patchDll(buffer) {
  if (!buffer.subarray(positionOffset, positionOffset + 4).equals(finalPosition)) {
    throw new Error("Unexpected specialty position bytes in HD_HOTA.dll.");
  }
  const layout = buffer.subarray(layoutOffset, layoutOffset + currentLayout.length);
  if (!layout.equals(currentLayout) && !layout.equals(finalLayout)) {
    throw new Error("Unexpected specialty layout code in HD_HOTA.dll.");
  }
  const updated = Buffer.from(buffer);
  if (layout.equals(currentLayout)) {
    finalLayout.copy(updated, layoutOffset);
  }
  return updated;
}

function patchFilesIni(buffer, namesToAdd, namesToRemove = []) {
  const text = buffer.toString("latin1");
  const newline = text.includes("\r\n") ? "\r\n" : "\n";
  const removed = new Set(namesToRemove.map((name) => name.toLowerCase()));
  const lines = text
    .replace(/\r?\n+$/, "")
    .split(/\r?\n/)
    .filter(
      (line) =>
        !removed.has(
          line.trim().replace(/^"|"$/g, "").toLowerCase(),
        ),
    );
  const listed = new Set(
    lines.map((line) =>
      line.trim().replace(/^"|"$/g, "").toLowerCase(),
    ),
  );
  for (const name of namesToAdd) {
    if (!listed.has(name.toLowerCase())) {
      lines.push(`"${name}"`);
    }
  }
  return Buffer.from(`${lines.join(newline)}${newline}`, "latin1");
}

const originals = Object.fromEntries(
  Object.values(files).map((relativePath) => [relativePath, read(relativePath)]),
);
const stamp = new Date().toISOString().replace(/[-:]/g, "").replace(/\..*$/, "").replace("T", "-");
const backupDir = path.join(gameDir, "ConfluxElementalistPatch", "backups", stamp);
fs.mkdirSync(path.join(backupDir, "Data"), { recursive: true });
const manifest = { patch: "Nyx Pixie Elementalist finalizer", createdAt: new Date().toISOString(), files: {} };
for (const [relativePath, buffer] of Object.entries(originals)) {
  const destination = path.join(backupDir, relativePath);
  fs.mkdirSync(path.dirname(destination), { recursive: true });
  fs.writeFileSync(destination, buffer);
  manifest.files[relativePath] = hash(buffer);
}
for (const pack of ["#hota", "#hota15"]) {
  for (const name of [...portraitNames, ...specialtyNames]) {
    const relativePath = path.join(
      "_HD3_Data",
      "Compability",
      pack,
      name,
    );
    const source = path.join(gameDir, relativePath);
    if (!fs.existsSync(source)) {
      manifest.files[relativePath] = null;
      continue;
    }
    const destination = path.join(backupDir, relativePath);
    fs.mkdirSync(path.dirname(destination), { recursive: true });
    const buffer = fs.readFileSync(source);
    fs.writeFileSync(destination, buffer);
    manifest.files[relativePath] = hash(buffer);
  }
}
fs.writeFileSync(path.join(backupDir, "manifest.json"), `${JSON.stringify(manifest, null, 2)}\n`);

writeIfChanged(files.dll, patchDll(originals[files.dll]));
writeIfChanged(files.language, patchLanguage(originals[files.language]));
writeIfChanged(files.bitmap, patchPortraits(originals[files.bitmap]));
writeIfChanged(
  files.bitmapExpansion,
  patchPortraits(originals[files.bitmapExpansion]),
);
writeIfChanged(
  files.hdFilesIni,
  patchFilesIni(
    originals[files.hdFilesIni],
    specialtyNames,
    portraitNames,
  ),
);
writeIfChanged(
  files.hdFilesIni15,
  patchFilesIni(
    originals[files.hdFilesIni15],
    specialtyNames,
    portraitNames,
  ),
);
for (const pack of ["#hota", "#hota15"]) {
  for (const name of portraitNames) {
    const portraitPath = path.join(
      gameDir,
      "_HD3_Data",
      "Compability",
      pack,
      name,
    );
    if (fs.existsSync(portraitPath)) {
      fs.unlinkSync(portraitPath);
    }
  }
  for (const name of specialtyNames) {
    fs.writeFileSync(
      path.join(gameDir, "_HD3_Data", "Compability", pack, name),
      read(path.join("_HD3_Data", "Compability", "#hota", name)),
    );
  }
}

for (const relativePath of [files.bitmap, files.bitmapExpansion]) {
  const archive = read(relativePath);
  for (let index = 0; index < portraitNames.length; index += 1) {
    if (!extractLodEntry(archive, portraitNames[index]).equals(portraitAssets[index])) {
      throw new Error(`Portrait verification failed: ${relativePath}/${portraitNames[index]}`);
    }
  }
}
for (const pack of ["#hota", "#hota15"]) {
  const filesIni = read(
    path.join("_HD3_Data", "Compability", pack, "Files.ini"),
  );
  if (
    !filesIni.equals(
      patchFilesIni(filesIni, specialtyNames, portraitNames),
    )
  ) {
    throw new Error(`HD resource registration verification failed: ${pack}`);
  }
  for (const name of portraitNames) {
    if (
      fs.existsSync(
        path.join(
          gameDir,
          "_HD3_Data",
          "Compability",
          pack,
          name,
        ),
      )
    ) {
      throw new Error(`Unsafe loose portrait still exists: ${pack}/${name}`);
    }
  }
  for (const name of specialtyNames) {
    const installed = read(
      path.join("_HD3_Data", "Compability", pack, name),
    );
    const expected = read(
      path.join("_HD3_Data", "Compability", "#hota", name),
    );
    if (!installed.equals(expected)) {
      throw new Error(`Specialty DEF verification failed: ${pack}/${name}`);
    }
  }
}

console.log("Finalized Nyx successfully.");
console.log(`Backup: ${backupDir}`);
