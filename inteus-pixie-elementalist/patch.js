#!/usr/bin/env node

"use strict";

const crypto = require("crypto");
const fs = require("fs");
const path = require("path");
const zlib = require("zlib");

const HERO_ID = 140;
const HERO_NAME = "Inteus";
const SPECIALTY_OFFSET = 0x279a00;
const HERO_DATA_OFFSET = 0x27d020;
const PIXIE_PORTRAIT_FRAME = 120;
const SPECIALTY_ICON_RESOURCES = [
  {
    name: "UN32.def",
    expectedFrameName: "un32lust.pcx",
    size: 32,
  },
  {
    name: "UN44.def",
    expectedFrameName: "un44lust.pcx",
    size: 44,
  },
];

const FILES = {
  setup: "HotA_Setup.ini",
  exe: "h3hota.exe",
  hdExe: "h3hota HD.exe",
  language: path.join("Data", "HotA_lng.lod"),
  sprites: path.join("Data", "H3ab_spr.lod"),
};

const ORIGINAL_HASHES = {
  [FILES.exe]: "b5f2f793af0986050fb41df7209c25d861ae0f837af52bb3bd6864ba4de84f41",
  [FILES.hdExe]: "5aaab925f06cccf23bb09814767590a95b84a557eb33d244800520be4f1f18de",
  [FILES.language]: "f4ba08f4adfcfb3dcffdc8fa2063307ff2a6caa48212b11073ef43dc73d3047e",
  [FILES.sprites]: "e0d5003742c8602827ef409966784483dece6eedde76aa2cfeee26cb12d25d67",
};

const PATCHED_FILES = [
  FILES.exe,
  FILES.hdExe,
  FILES.language,
  FILES.sprites,
];

const ORIGINAL_SPECIALTY = Buffer.from(
  "030000002b0000000000000000000000000000000000000000000000",
  "hex",
);
const PATCHED_SPECIALTY = Buffer.from(
  "01000000760000000000000000000000000000000000000000000000",
  "hex",
);

const ORIGINAL_HERO = Buffer.from(
  "00000000070000001100000007000000010000000e00000001000000010000002b000000760000007000000073000000",
  "hex",
);
const PATCHED_HERO = Buffer.from(
  "0000000007000000110000000e000000010000001300000001000000010000000d000000760000007000000073000000",
  "hex",
);

const SPECIALTY_FIELDS = [
  "Pixies",
  "Creature Bonus: Pixies",
  "{Pixies and Sprites}\n\nIncreases the Speed of allied Pixies and Sprites by 1 and their Attack and Defense skills by 10% for every level (rounded up).",
];

const BIOGRAPHY =
  "Inteus's command of fire drew the smallest spirits of the Conflux to his side. He shelters Pixies and Sprites behind walls of living flame, training them to strike with speed and surprising strength.";

function fail(message) {
  console.error(`Error: ${message}`);
  process.exitCode = 1;
}

function hash(buffer) {
  return crypto.createHash("sha256").update(buffer).digest("hex");
}

function readGameFile(gameDir, relativePath) {
  const filePath = path.join(gameDir, relativePath);
  if (!fs.existsSync(filePath)) {
    throw new Error(`Missing required file: ${filePath}`);
  }
  return fs.readFileSync(filePath);
}

function assertHotAVersion(gameDir) {
  const setup = readGameFile(gameDir, FILES.setup).toString("latin1");
  if (!/Main Version\s*=\s*1\.8\.0\b/.test(setup)) {
    throw new Error("This patch supports HotA 1.8.0 only.");
  }
}

function bufferAt(buffer, offset, length) {
  return buffer.subarray(offset, offset + length);
}

function equalAt(buffer, offset, expected) {
  return bufferAt(buffer, offset, expected.length).equals(expected);
}

function executableState(buffer) {
  if (
    equalAt(buffer, SPECIALTY_OFFSET, ORIGINAL_SPECIALTY) &&
    equalAt(buffer, HERO_DATA_OFFSET, ORIGINAL_HERO)
  ) {
    return "original";
  }
  if (
    equalAt(buffer, SPECIALTY_OFFSET, PATCHED_SPECIALTY) &&
    equalAt(buffer, HERO_DATA_OFFSET, PATCHED_HERO)
  ) {
    return "patched";
  }
  return "unknown";
}

function findLodEntry(archive, requestedName) {
  if (archive.subarray(0, 4).toString("latin1") !== "LOD\0") {
    throw new Error("Invalid LOD archive header.");
  }

  const entryCount = archive.readUInt32LE(8);
  for (let index = 0; index < entryCount; index += 1) {
    const entryOffset = 92 + index * 32;
    const name = archive
      .subarray(entryOffset, entryOffset + 16)
      .toString("latin1")
      .replace(/\0.*$/, "");
    if (name.toLowerCase() === requestedName.toLowerCase()) {
      return {
        directoryOffset: entryOffset,
        name,
        offset: archive.readUInt32LE(entryOffset + 16),
        size: archive.readUInt32LE(entryOffset + 20),
        type: archive.readUInt32LE(entryOffset + 24),
        compressedSize: archive.readUInt32LE(entryOffset + 28),
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
  const newOffset = updated.length;
  updated.writeUInt32LE(newOffset, entry.directoryOffset + 16);
  updated.writeUInt32LE(replacement.length, entry.directoryOffset + 20);
  updated.writeUInt32LE(0, entry.directoryOffset + 28);
  return Buffer.concat([updated, replacement]);
}

function findDefFrame(definition, requestedFrame) {
  if (
    definition.length < 784 ||
    definition.readUInt32LE(0) !== 0x47
  ) {
    throw new Error("Invalid DEF resource.");
  }

  const groupCount = definition.readUInt32LE(12);
  let directoryOffset = 784;
  let firstFrame = 0;

  for (let group = 0; group < groupCount; group += 1) {
    if (directoryOffset + 16 > definition.length) {
      throw new Error("Invalid DEF group directory.");
    }
    const frameCount = definition.readUInt32LE(directoryOffset + 4);
    const namesOffset = directoryOffset + 16;
    const offsetsOffset = namesOffset + frameCount * 13;
    const nextDirectoryOffset = offsetsOffset + frameCount * 4;
    if (nextDirectoryOffset > definition.length) {
      throw new Error("Invalid DEF frame directory.");
    }

    if (
      requestedFrame >= firstFrame &&
      requestedFrame < firstFrame + frameCount
    ) {
      const localFrame = requestedFrame - firstFrame;
      const nameOffset = namesOffset + localFrame * 13;
      const offsetTableEntry = offsetsOffset + localFrame * 4;
      const frameOffset = definition.readUInt32LE(offsetTableEntry);
      if (frameOffset + 32 > definition.length) {
        throw new Error(`Invalid DEF frame offset: ${requestedFrame}`);
      }
      return {
        frameOffset,
        name: definition
          .subarray(nameOffset, nameOffset + 13)
          .toString("latin1")
          .replace(/\0.*$/, ""),
        offsetTableEntry,
      };
    }

    firstFrame += frameCount;
    directoryOffset = nextDirectoryOffset;
  }

  throw new Error(`DEF frame not found: ${requestedFrame}`);
}

function decodeDefFrame(definition, requestedFrame) {
  const location = findDefFrame(definition, requestedFrame);
  const offset = location.frameOffset;
  const compression = definition.readUInt32LE(offset + 4);
  const width = definition.readUInt32LE(offset + 16);
  const height = definition.readUInt32LE(offset + 20);
  const dataOffset = offset + 32;
  const pixels = Buffer.alloc(width * height);

  if (compression === 0) {
    const end = dataOffset + pixels.length;
    if (end > definition.length) {
      throw new Error(`Truncated uncompressed DEF frame: ${requestedFrame}`);
    }
    definition.copy(pixels, 0, dataOffset, end);
  } else if (compression === 1) {
    for (let row = 0; row < height; row += 1) {
      const rowTableEntry = dataOffset + row * 4;
      if (rowTableEntry + 4 > definition.length) {
        throw new Error(`Truncated DEF row table: ${requestedFrame}`);
      }
      let cursor = dataOffset + definition.readUInt32LE(rowTableEntry);
      let column = 0;

      while (column < width) {
        if (cursor + 2 > definition.length) {
          throw new Error(`Truncated DEF row data: ${requestedFrame}`);
        }
        const code = definition[cursor];
        const runLength = definition[cursor + 1] + 1;
        cursor += 2;
        if (column + runLength > width) {
          throw new Error(`Invalid DEF run length: ${requestedFrame}`);
        }

        if (code === 0xff) {
          if (cursor + runLength > definition.length) {
            throw new Error(`Truncated DEF literal run: ${requestedFrame}`);
          }
          definition.copy(
            pixels,
            row * width + column,
            cursor,
            cursor + runLength,
          );
          cursor += runLength;
        } else {
          pixels.fill(
            code,
            row * width + column,
            row * width + column + runLength,
          );
        }
        column += runLength;
      }
    }
  } else {
    throw new Error(
      `Unsupported DEF compression ${compression} in frame ${requestedFrame}.`,
    );
  }

  return {
    ...location,
    height,
    pixels,
    width,
  };
}

function paletteColor(definition, paletteIndex) {
  const offset = 16 + paletteIndex * 3;
  return [
    definition[offset],
    definition[offset + 1],
    definition[offset + 2],
  ];
}

function pixieSpecialtyPixels(creaturePortraits, specialtyIcons, iconSize) {
  const source = decodeDefFrame(creaturePortraits, PIXIE_PORTRAIT_FRAME);
  if (source.width !== 58 || source.height !== 64) {
    throw new Error("Unexpected Pixie portrait dimensions.");
  }

  const cropSize = source.width;
  const cropLeft = 0;
  const cropTop = Math.floor((source.height - cropSize) / 2);
  const sourcePalette = Array.from({ length: 256 }, (_, index) =>
    paletteColor(creaturePortraits, index),
  );
  const targetPalette = Array.from({ length: 256 }, (_, index) =>
    paletteColor(specialtyIcons, index),
  );
  const pixels = Buffer.alloc(iconSize * iconSize);

  for (let y = 0; y < iconSize; y += 1) {
    const sourceY =
      cropTop +
      ((y + 0.5) * cropSize) / iconSize -
      0.5;
    const y0 = Math.max(cropTop, Math.floor(sourceY));
    const y1 = Math.min(cropTop + cropSize - 1, y0 + 1);
    const yWeight = sourceY - Math.floor(sourceY);

    for (let x = 0; x < iconSize; x += 1) {
      const sourceX =
        cropLeft +
        ((x + 0.5) * cropSize) / iconSize -
        0.5;
      const x0 = Math.max(cropLeft, Math.floor(sourceX));
      const x1 = Math.min(cropLeft + cropSize - 1, x0 + 1);
      const xWeight = sourceX - Math.floor(sourceX);
      const colors = [
        sourcePalette[source.pixels[y0 * source.width + x0]],
        sourcePalette[source.pixels[y0 * source.width + x1]],
        sourcePalette[source.pixels[y1 * source.width + x0]],
        sourcePalette[source.pixels[y1 * source.width + x1]],
      ];
      const rgb = [0, 1, 2].map((channel) => {
        const top =
          colors[0][channel] * (1 - xWeight) +
          colors[1][channel] * xWeight;
        const bottom =
          colors[2][channel] * (1 - xWeight) +
          colors[3][channel] * xWeight;
        return top * (1 - yWeight) + bottom * yWeight;
      });

      let bestIndex = 8;
      let bestDistance = Number.POSITIVE_INFINITY;
      for (let index = 8; index < targetPalette.length; index += 1) {
        const color = targetPalette[index];
        const distance =
          (rgb[0] - color[0]) ** 2 +
          (rgb[1] - color[1]) ** 2 +
          (rgb[2] - color[2]) ** 2;
        if (distance < bestDistance) {
          bestDistance = distance;
          bestIndex = index;
        }
      }
      pixels[y * iconSize + x] = bestIndex;
    }
  }

  return pixels;
}

function patchedSpecialtyIcons(originalIcons, creaturePortraits, resource) {
  const icon = findDefFrame(originalIcons, HERO_ID);
  if (
    icon.name.toLowerCase() !== resource.expectedFrameName ||
    originalIcons.readUInt32LE(4) !== resource.size ||
    originalIcons.readUInt32LE(8) !== resource.size
  ) {
    throw new Error(
      `Unexpected Inteus specialty icon in ${resource.name}.`,
    );
  }

  const pixels = pixieSpecialtyPixels(
    creaturePortraits,
    originalIcons,
    resource.size,
  );
  const frame = Buffer.alloc(32 + pixels.length);
  frame.writeUInt32LE(pixels.length, 0);
  frame.writeUInt32LE(0, 4);
  frame.writeUInt32LE(resource.size, 8);
  frame.writeUInt32LE(resource.size, 12);
  frame.writeUInt32LE(resource.size, 16);
  frame.writeUInt32LE(resource.size, 20);
  pixels.copy(frame, 32);

  const updated = Buffer.from(originalIcons);
  updated.writeUInt32LE(updated.length, icon.offsetTableEntry);
  return Buffer.concat([updated, frame]);
}

function patchedSpriteArchive(originalArchive) {
  const creaturePortraits = extractLodEntry(originalArchive, "TwCrPort.def");
  let archive = originalArchive;
  for (const resource of SPECIALTY_ICON_RESOURCES) {
    const specialtyIcons = extractLodEntry(originalArchive, resource.name);
    archive = replaceLodEntry(
      archive,
      resource.name,
      patchedSpecialtyIcons(
        specialtyIcons,
        creaturePortraits,
        resource,
      ),
    );
  }
  return archive;
}

function spriteState(archive) {
  if (hash(archive) === ORIGINAL_HASHES[FILES.sprites]) {
    return "original";
  }
  try {
    const creaturePortraits = extractLodEntry(archive, "TwCrPort.def");
    for (const resource of SPECIALTY_ICON_RESOURCES) {
      const specialtyIcons = extractLodEntry(archive, resource.name);
      const expected = pixieSpecialtyPixels(
        creaturePortraits,
        specialtyIcons,
        resource.size,
      );
      const actual = decodeDefFrame(specialtyIcons, HERO_ID);
      if (
        actual.width !== resource.size ||
        actual.height !== resource.size ||
        !actual.pixels.equals(expected)
      ) {
        return "unknown";
      }
    }
    return "patched";
  } catch {
    return "unknown";
  }
  return "unknown";
}

function parseTsv(text) {
  const rows = [];
  let row = [];
  let field = "";
  let quoted = false;

  for (let index = 0; index < text.length; index += 1) {
    const character = text[index];
    if (quoted) {
      if (character === '"') {
        if (text[index + 1] === '"') {
          field += '"';
          index += 1;
        } else {
          quoted = false;
        }
      } else {
        field += character;
      }
    } else if (character === '"') {
      quoted = true;
    } else if (character === "\t") {
      row.push(field);
      field = "";
    } else if (character === "\n") {
      if (field.endsWith("\r")) {
        field = field.slice(0, -1);
      }
      row.push(field);
      rows.push(row);
      row = [];
      field = "";
    } else {
      field += character;
    }
  }

  if (field.length || row.length) {
    row.push(field);
    rows.push(row);
  }
  return rows;
}

function serializeTsv(rows) {
  const quote = (field) => {
    const value = String(field);
    if (!/[\t\r\n"]/.test(value)) {
      return value;
    }
    return `"${value.replace(/"/g, '""')}"`;
  };
  return `${rows.map((row) => row.map(quote).join("\t")).join("\r\n")}\r\n`;
}

function patchedLanguageArchive(originalArchive) {
  const heroSpec = extractLodEntry(originalArchive, "HeroSpec.txt").toString("latin1");
  const specialtyRows = parseTsv(heroSpec);
  const specialtyRow = HERO_ID + 2;
  if (
    specialtyRows.length <= specialtyRow ||
    specialtyRows[specialtyRow][0] !== "Bloodlust"
  ) {
    throw new Error("Unexpected Inteus specialty text in HeroSpec.txt.");
  }
  specialtyRows[specialtyRow] = SPECIALTY_FIELDS;

  const heroBios = extractLodEntry(originalArchive, "HeroBios.txt")
    .toString("latin1")
    .split(/\r?\n/);
  if (
    heroBios.length <= HERO_ID ||
    !heroBios[HERO_ID].startsWith("At an early age, Inteus' mastery")
  ) {
    throw new Error("Unexpected Inteus biography in HeroBios.txt.");
  }
  heroBios[HERO_ID] = BIOGRAPHY;

  let archive = replaceLodEntry(
    originalArchive,
    "HeroSpec.txt",
    Buffer.from(serializeTsv(specialtyRows), "latin1"),
  );
  archive = replaceLodEntry(
    archive,
    "HeroBios.txt",
    Buffer.from(`${heroBios.join("\r\n")}`, "latin1"),
  );
  return archive;
}

function languageState(archive) {
  try {
    const specialtyRows = parseTsv(
      extractLodEntry(archive, "HeroSpec.txt").toString("latin1"),
    );
    const heroBios = extractLodEntry(archive, "HeroBios.txt")
      .toString("latin1")
      .split(/\r?\n/);
    if (
      specialtyRows[HERO_ID + 2]?.[0] === SPECIALTY_FIELDS[0] &&
      specialtyRows[HERO_ID + 2]?.[2] === SPECIALTY_FIELDS[2] &&
      heroBios[HERO_ID] === BIOGRAPHY
    ) {
      return "patched";
    }
    if (
      specialtyRows[HERO_ID + 2]?.[0] === "Bloodlust" &&
      heroBios[HERO_ID]?.startsWith("At an early age, Inteus' mastery")
    ) {
      return "original";
    }
  } catch {
    return "unknown";
  }
  return "unknown";
}

function inspect(gameDir) {
  assertHotAVersion(gameDir);
  const exe = readGameFile(gameDir, FILES.exe);
  const hdExe = readGameFile(gameDir, FILES.hdExe);
  const language = readGameFile(gameDir, FILES.language);
  const sprites = readGameFile(gameDir, FILES.sprites);
  return {
    exe,
    hdExe,
    language,
    sprites,
    states: {
      [FILES.exe]: executableState(exe),
      [FILES.hdExe]: executableState(hdExe),
      [FILES.language]: languageState(language),
      [FILES.sprites]: spriteState(sprites),
    },
  };
}

function verifyOriginalHashes(inspected) {
  const buffers = {
    [FILES.exe]: inspected.exe,
    [FILES.hdExe]: inspected.hdExe,
    [FILES.language]: inspected.language,
    [FILES.sprites]: inspected.sprites,
  };
  for (const relativePath of PATCHED_FILES) {
    const actual = hash(buffers[relativePath]);
    if (actual !== ORIGINAL_HASHES[relativePath]) {
      throw new Error(
        `${relativePath} does not match the supported HotA 1.8.0 build.\n` +
          `Expected ${ORIGINAL_HASHES[relativePath]}\nActual   ${actual}`,
      );
    }
  }
}

function timestamp() {
  return new Date()
    .toISOString()
    .replace(/[-:]/g, "")
    .replace(/\..*$/, "")
    .replace("T", "-");
}

function createBackup(gameDir) {
  const backupDir = path.join(
    gameDir,
    "ConfluxElementalistPatch",
    "backups",
    timestamp(),
  );
  fs.mkdirSync(path.join(backupDir, "Data"), { recursive: true });

  const manifest = {
    patch: "Inteus Pixie Elementalist",
    hotaVersion: "1.8.0",
    createdAt: new Date().toISOString(),
    files: {},
  };

  for (const relativePath of PATCHED_FILES) {
    const source = path.join(gameDir, relativePath);
    const destination = path.join(backupDir, relativePath);
    fs.mkdirSync(path.dirname(destination), { recursive: true });
    fs.copyFileSync(source, destination);
    manifest.files[relativePath] = hash(fs.readFileSync(destination));
  }
  fs.writeFileSync(
    path.join(backupDir, "manifest.json"),
    `${JSON.stringify(manifest, null, 2)}\n`,
  );
  return backupDir;
}

function apply(gameDir) {
  const inspected = inspect(gameDir);
  const states = Object.values(inspected.states);
  if (states.every((state) => state === "patched")) {
    console.log(`${HERO_NAME} is already patched as a Pixie Elementalist.`);
    return;
  }
  if (!states.every((state) => state === "original")) {
    throw new Error(
      `The installation is in a mixed or unknown state:\n${JSON.stringify(inspected.states, null, 2)}`,
    );
  }
  verifyOriginalHashes(inspected);

  const backupDir = createBackup(gameDir);
  const patchExecutable = (buffer) => {
    const updated = Buffer.from(buffer);
    PATCHED_SPECIALTY.copy(updated, SPECIALTY_OFFSET);
    PATCHED_HERO.copy(updated, HERO_DATA_OFFSET);
    return updated;
  };

  fs.writeFileSync(path.join(gameDir, FILES.exe), patchExecutable(inspected.exe));
  fs.writeFileSync(path.join(gameDir, FILES.hdExe), patchExecutable(inspected.hdExe));
  fs.writeFileSync(
    path.join(gameDir, FILES.language),
    patchedLanguageArchive(inspected.language),
  );
  fs.writeFileSync(
    path.join(gameDir, FILES.sprites),
    patchedSpriteArchive(inspected.sprites),
  );

  const verified = inspect(gameDir);
  if (!Object.values(verified.states).every((state) => state === "patched")) {
    throw new Error(
      `Patch verification failed. Restore from ${backupDir} before launching the game.`,
    );
  }

  console.log(`Patched ${HERO_NAME} successfully.`);
  console.log(`Backup: ${backupDir}`);
}

function latestBackup(gameDir) {
  const backupRoot = path.join(gameDir, "ConfluxElementalistPatch", "backups");
  if (!fs.existsSync(backupRoot)) {
    throw new Error("No patch backups were found.");
  }
  const backups = fs
    .readdirSync(backupRoot, { withFileTypes: true })
    .filter((entry) => entry.isDirectory())
    .map((entry) => entry.name)
    .sort();
  if (!backups.length) {
    throw new Error("No patch backups were found.");
  }
  return path.join(backupRoot, backups[backups.length - 1]);
}

function restore(gameDir, requestedBackup) {
  const backupDir = requestedBackup
    ? path.resolve(requestedBackup)
    : latestBackup(gameDir);
  const manifestPath = path.join(backupDir, "manifest.json");
  if (!fs.existsSync(manifestPath)) {
    throw new Error(`Backup manifest not found: ${manifestPath}`);
  }
  const manifest = JSON.parse(fs.readFileSync(manifestPath, "utf8"));

  for (const relativePath of PATCHED_FILES) {
    if (!manifest.files[relativePath]) {
      throw new Error(
        `Backup predates the specialty-picture patch: ${backupDir}`,
      );
    }
    const source = path.join(backupDir, relativePath);
    const data = fs.readFileSync(source);
    if (hash(data) !== manifest.files[relativePath]) {
      throw new Error(`Backup checksum mismatch: ${source}`);
    }
  }
  for (const relativePath of PATCHED_FILES) {
    fs.copyFileSync(path.join(backupDir, relativePath), path.join(gameDir, relativePath));
  }

  const verified = inspect(gameDir);
  if (!Object.values(verified.states).every((state) => state === "original")) {
    throw new Error("Restore verification failed.");
  }
  console.log(`Restored original files from ${backupDir}`);
}

function showStatus(gameDir) {
  const inspected = inspect(gameDir);
  console.log(`HotA 1.8.0 ${HERO_NAME} patch status:`);
  for (const [file, state] of Object.entries(inspected.states)) {
    console.log(`  ${file}: ${state}`);
  }
}

function defaultGameDir() {
  const candidates = [
    path.resolve(__dirname, ".."),
    path.resolve(__dirname, "..", ".."),
    process.cwd(),
  ];
  return (
    candidates.find((candidate) =>
      fs.existsSync(path.join(candidate, FILES.setup)),
    ) || candidates[0]
  );
}

function parseArguments(argv) {
  const options = {
    command: "status",
    gameDir: process.env.HOTA_GAME_DIR
      ? path.resolve(process.env.HOTA_GAME_DIR)
      : defaultGameDir(),
    backup: null,
  };
  const args = [...argv];
  if (args[0] && !args[0].startsWith("--")) {
    options.command = args.shift();
  }
  while (args.length) {
    const option = args.shift();
    if (option === "--game-dir") {
      options.gameDir = path.resolve(args.shift());
    } else if (option === "--backup") {
      options.backup = args.shift();
    } else {
      throw new Error(`Unknown argument: ${option}`);
    }
  }
  return options;
}

try {
  const options = parseArguments(process.argv.slice(2));
  if (options.command === "apply") {
    apply(options.gameDir);
  } else if (options.command === "restore") {
    restore(options.gameDir, options.backup);
  } else if (options.command === "status") {
    showStatus(options.gameDir);
  } else {
    fail("Usage: node patch.js [status|apply|restore] [--game-dir PATH] [--backup PATH]");
  }
} catch (error) {
  fail(error.message);
}
