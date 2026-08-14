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
const SPECIALTY_DISPLAY_POINTER_OFFSET = 0xe0d89;
const SPECIALTY_DISPLAY_STRING_OFFSET = 0x1ff5d2;
const HD_HOTA_SPECIALTY_FRAME_OFFSET = 0x234d86;
const HD_HOTA_SPECIALTY_POINTER_OFFSET = 0x234d8b;
const HD_HOTA_SPECIALTY_POSITION_OFFSET = 0x234d9a;
const HD_HOTA_SPECIALTY_LAYOUT_OFFSET = 0x234dc3;
const HD_HOTA_SPECIALTY_RESOURCE_OFFSET = 0x29ee3c;
const PIXIE_PORTRAIT_FRAME = 120;
const SPECIALTY_DISPLAY_RESOURCE = "IX44.def";
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
  hdHotA: "HD_HOTA.dll",
  language: path.join("Data", "HotA_lng.lod"),
  spritesBase: path.join("Data", "H3sprite.lod"),
  spritesExpansion: path.join("Data", "H3ab_spr.lod"),
  displayResource: path.join("Data", SPECIALTY_DISPLAY_RESOURCE),
  hdFilesIni: path.join(
    "_HD3_Data",
    "Compability",
    "#hota",
    "Files.ini",
  ),
};

const HD_OVERRIDE_STATE_KEY = "HD Mod specialty overrides";
const DISPLAY_OVERRIDE_STATE_KEY = "Unique specialty display override";
const HD_OVERRIDE_FILES = SPECIALTY_ICON_RESOURCES.map((resource) =>
  path.join("_HD3_Data", "Compability", "#hota", resource.name),
);
const HD_DISPLAY_RESOURCE = path.join(
  "_HD3_Data",
  "Compability",
  "#hota",
  SPECIALTY_DISPLAY_RESOURCE,
);

const ORIGINAL_HASHES = {
  [FILES.exe]: "b5f2f793af0986050fb41df7209c25d861ae0f837af52bb3bd6864ba4de84f41",
  [FILES.hdExe]: "5aaab925f06cccf23bb09814767590a95b84a557eb33d244800520be4f1f18de",
  [FILES.hdHotA]: "0ccb8e9eb0a43495c3a9dd09770f51ee41cc2ed79f9730298ac433c8432c4951",
  [FILES.language]: "f4ba08f4adfcfb3dcffdc8fa2063307ff2a6caa48212b11073ef43dc73d3047e",
  [FILES.spritesBase]: "57caf2c50573f33a0d91e4222e51d3a73c136d44decf59dde21cacad88fe5d66",
  [FILES.spritesExpansion]: "e0d5003742c8602827ef409966784483dece6eedde76aa2cfeee26cb12d25d67",
};

const PATCHED_FILES = [
  FILES.exe,
  FILES.hdExe,
  FILES.hdHotA,
  FILES.language,
  FILES.spritesBase,
  FILES.spritesExpansion,
];
const PATCHED_STATE_KEYS = [
  ...PATCHED_FILES,
  HD_OVERRIDE_STATE_KEY,
  DISPLAY_OVERRIDE_STATE_KEY,
];

const V101_HASHES = {
  [FILES.exe]: "c4d40880504228b26f5a341d579a553d3aeadc34a6b4c60fcbfec0fd39ca59f5",
  [FILES.hdExe]: "2cf1a00a6fe774b0fdb82262d69f724f30e7e0cde6a849f57c843d00055d9975",
  [FILES.hdHotA]: ORIGINAL_HASHES[FILES.hdHotA],
  [FILES.language]: "529b294498002f8d49a42c50db89361097ed58edcb76e36418108cdfdf792671",
  [FILES.spritesBase]: ORIGINAL_HASHES[FILES.spritesBase],
  [FILES.spritesExpansion]: "789739587a20725d3dc2b16685b4c248eaa6e299a34ea20b15d2db342f9c13d8",
};

const V102_HASHES = {
  ...V101_HASHES,
  [FILES.spritesBase]: "95e55bdb456faec843690e9bc7e21d7720006e20c3ce959db96462f39ddbe1e5",
};

const V104_HASHES = {
  ...V102_HASHES,
  [FILES.exe]: "bc8698d3bf1d440bc9c25d0ac0b20e3cbeefe4033bd233211aaf2039b65b73c8",
  [FILES.hdExe]: "92e98247fbccf0d1cbafd87d61bfa7b4060f95eba20c015b76ff809eb51914bb",
};

const V105_HASHES = {
  ...V104_HASHES,
  [FILES.hdHotA]: "d9ecbfdd911a532394ac068ccf7acdbe8b08e17010776d4fce28ff7698b5bf49",
};

const V106_HASHES = {
  ...V105_HASHES,
  [FILES.spritesBase]: "e9927169fd68b5c18f6fa525ed88eba053bc068b19b9741fd98bf63088a5610b",
  [FILES.spritesExpansion]: "a2341a75c541836dd89ba43866ccec9af9bac2fb82641eb52334ee873399a2ca",
};

const V107_HASHES = {
  ...V106_HASHES,
  [FILES.hdHotA]: "6fb654c3e2659c8aa17c9735a5ad12be5df9fc422db5fb5eadb49735ef61b3e2",
};

const V108_HASHES = {
  ...V107_HASHES,
  [FILES.hdHotA]: "8c1fe0651e3826dabc475b1474a53f384982662543afc6065ca82cea0b552d25",
};

const V109_HASHES = {
  ...V108_HASHES,
  [FILES.hdHotA]: "a7c7b57374dc1375c7a71a723b8d1fbe459553becbc797b5a2626e78f759a26a",
};

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

const ORIGINAL_DISPLAY_POINTER = Buffer.from("68909d6700", "hex");
const PATCHED_DISPLAY_POINTER = Buffer.from("68d2f55f00", "hex");
const ORIGINAL_DISPLAY_STRING = Buffer.alloc(
  Buffer.byteLength(SPECIALTY_DISPLAY_RESOURCE) + 1,
  0x90,
);
const PATCHED_DISPLAY_STRING = Buffer.from(
  `${SPECIALTY_DISPLAY_RESOURCE.toLowerCase()}\0`,
  "latin1",
);
const ORIGINAL_HD_HOTA_SPECIALTY_RESOURCE = Buffer.from(
  "UN44.def\0",
  "latin1",
);
const LEGACY_HD_HOTA_SPECIALTY_RESOURCE = Buffer.from(
  `${SPECIALTY_DISPLAY_RESOURCE}\0`,
  "latin1",
);
const ORIGINAL_HD_HOTA_SPECIALTY_FRAME = Buffer.from("8b450850", "hex");
const PATCHED_HD_HOTA_SPECIALTY_FRAME = Buffer.from("6a789090", "hex");
const ORIGINAL_HD_HOTA_SPECIALTY_POINTER = Buffer.from("3c042a01", "hex");
const PATCHED_HD_HOTA_SPECIALTY_POINTER = Buffer.from("ac6d2901", "hex");
const ORIGINAL_HD_HOTA_SPECIALTY_POSITION = Buffer.from("6a126a48", "hex");
const HIGH_HD_HOTA_SPECIALTY_POSITION = Buffer.from("6a186a4e", "hex");
const RIGHT_HD_HOTA_SPECIALTY_POSITION = Buffer.from("6a1c6a4e", "hex");
const CORNER_HD_HOTA_SPECIALTY_POSITION = Buffer.from("6a186a52", "hex");
const PATCHED_HD_HOTA_SPECIALTY_POSITION = Buffer.from("6a126a4e", "hex");
const ORIGINAL_HD_HOTA_SPECIALTY_LAYOUT = Buffer.from(
  "8b4dcc8b51308b45cc668b4a306689481c8b55cc8b42308b4dcc668b50346689511e8b",
  "hex",
);
const LEGACY_HD_HOTA_SPECIALTY_LAYOUT = Buffer.from(
  "8b45cc8b4830668b51306689501c668b51346689501e6683401a0a90909090909090",
  "hex",
);
const PATCHED_HD_HOTA_SPECIALTY_LAYOUT = Buffer.from(
  "8b45cc8b4830668b51306689501c668b51346689501e66834018066683401a049090",
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
  const recordsOriginal =
    equalAt(buffer, SPECIALTY_OFFSET, ORIGINAL_SPECIALTY) &&
    equalAt(buffer, HERO_DATA_OFFSET, ORIGINAL_HERO);
  const recordsPatched =
    equalAt(buffer, SPECIALTY_OFFSET, PATCHED_SPECIALTY) &&
    equalAt(buffer, HERO_DATA_OFFSET, PATCHED_HERO);
  const displayOriginal =
    equalAt(
      buffer,
      SPECIALTY_DISPLAY_POINTER_OFFSET,
      ORIGINAL_DISPLAY_POINTER,
    ) &&
    equalAt(
      buffer,
      SPECIALTY_DISPLAY_STRING_OFFSET,
      ORIGINAL_DISPLAY_STRING,
    );
  const displayPatched =
    equalAt(
      buffer,
      SPECIALTY_DISPLAY_POINTER_OFFSET,
      PATCHED_DISPLAY_POINTER,
    ) &&
    equalAt(
      buffer,
      SPECIALTY_DISPLAY_STRING_OFFSET,
      PATCHED_DISPLAY_STRING,
    );

  if (recordsOriginal && displayOriginal) {
    return "original";
  }
  if (recordsPatched && displayOriginal) {
    return "legacy";
  }
  if (recordsPatched && displayPatched) {
    return "patched";
  }
  return "unknown";
}

function hdHotAState(buffer) {
  const codeOriginal =
    equalAt(
      buffer,
      HD_HOTA_SPECIALTY_FRAME_OFFSET,
      ORIGINAL_HD_HOTA_SPECIALTY_FRAME,
    ) &&
    equalAt(
      buffer,
      HD_HOTA_SPECIALTY_POINTER_OFFSET,
      ORIGINAL_HD_HOTA_SPECIALTY_POINTER,
    );
  const codePatched =
    equalAt(
      buffer,
      HD_HOTA_SPECIALTY_FRAME_OFFSET,
      PATCHED_HD_HOTA_SPECIALTY_FRAME,
    ) &&
    equalAt(
      buffer,
      HD_HOTA_SPECIALTY_POINTER_OFFSET,
      PATCHED_HD_HOTA_SPECIALTY_POINTER,
    );
  const resourceOriginal =
    equalAt(
      buffer,
      HD_HOTA_SPECIALTY_RESOURCE_OFFSET,
      ORIGINAL_HD_HOTA_SPECIALTY_RESOURCE,
    );
  const resourceLegacy = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_RESOURCE_OFFSET,
    LEGACY_HD_HOTA_SPECIALTY_RESOURCE,
  );
  const positionOriginal = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_POSITION_OFFSET,
    ORIGINAL_HD_HOTA_SPECIALTY_POSITION,
  );
  const positionHigh = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_POSITION_OFFSET,
    HIGH_HD_HOTA_SPECIALTY_POSITION,
  );
  const positionRight = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_POSITION_OFFSET,
    RIGHT_HD_HOTA_SPECIALTY_POSITION,
  );
  const positionCorner = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_POSITION_OFFSET,
    CORNER_HD_HOTA_SPECIALTY_POSITION,
  );
  const positionPatched = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_POSITION_OFFSET,
    PATCHED_HD_HOTA_SPECIALTY_POSITION,
  );
  const layoutOriginal = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_LAYOUT_OFFSET,
    ORIGINAL_HD_HOTA_SPECIALTY_LAYOUT,
  );
  const layoutLegacy = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_LAYOUT_OFFSET,
    LEGACY_HD_HOTA_SPECIALTY_LAYOUT,
  );
  const layoutPatched = equalAt(
    buffer,
    HD_HOTA_SPECIALTY_LAYOUT_OFFSET,
    PATCHED_HD_HOTA_SPECIALTY_LAYOUT,
  );

  if (codeOriginal && resourceOriginal && positionOriginal && layoutOriginal) {
    return "original";
  }
  if (codeOriginal && resourceLegacy && positionOriginal && layoutOriginal) {
    return "legacy";
  }
  if (codePatched && resourceOriginal && positionOriginal && layoutOriginal) {
    return "uncentered";
  }
  if (codePatched && resourceOriginal && positionHigh && layoutOriginal) {
    return "high";
  }
  if (codePatched && resourceOriginal && positionRight && layoutOriginal) {
    return "right";
  }
  if (codePatched && resourceOriginal && positionCorner && layoutOriginal) {
    return "corner";
  }
  if (codePatched && resourceOriginal && positionPatched && layoutLegacy) {
    return "legacy-layout";
  }
  if (codePatched && resourceOriginal && positionPatched && layoutPatched) {
    return "patched";
  }
  return "unknown";
}

function findLodEntry(archive, requestedName, required = true) {
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
  if (!required) {
    return null;
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

function addOrReplaceLodEntry(archive, name, replacement, type = 0) {
  if (Buffer.byteLength(name, "latin1") > 15) {
    throw new Error(`LOD entry name is too long: ${name}`);
  }
  if (findLodEntry(archive, name, false)) {
    return replaceLodEntry(archive, name, replacement);
  }

  const entryCount = archive.readUInt32LE(8);
  const directoryOffset = 92 + entryCount * 32;
  let firstDataOffset = archive.length;
  for (let index = 0; index < entryCount; index += 1) {
    const dataOffset = archive.readUInt32LE(92 + index * 32 + 16);
    if (dataOffset > 0 && dataOffset < firstDataOffset) {
      firstDataOffset = dataOffset;
    }
  }
  if (directoryOffset + 32 > firstDataOffset) {
    throw new Error(`No free LOD directory slot for ${name}.`);
  }

  const updated = Buffer.from(archive);
  updated.fill(0, directoryOffset, directoryOffset + 32);
  updated.write(name, directoryOffset, "latin1");
  updated.writeUInt32LE(archive.length, directoryOffset + 16);
  updated.writeUInt32LE(replacement.length, directoryOffset + 20);
  updated.writeUInt32LE(type, directoryOffset + 24);
  updated.writeUInt32LE(0, directoryOffset + 28);
  updated.writeUInt32LE(entryCount + 1, 8);
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
  return registeredDisplayResource(archive);
}

function registeredDisplayResource(archive) {
  const sourceEntry = findLodEntry(archive, "UN44.def");
  return addOrReplaceLodEntry(
    archive,
    SPECIALTY_DISPLAY_RESOURCE,
    extractLodEntry(archive, "UN44.def"),
    sourceEntry.type,
  );
}

function spriteState(archive, originalHash) {
  if (hash(archive) === originalHash) {
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
    const displayEntry = findLodEntry(
      archive,
      SPECIALTY_DISPLAY_RESOURCE,
      false,
    );
    if (!displayEntry) {
      return "legacy";
    }
    if (
      !extractLodEntry(archive, SPECIALTY_DISPLAY_RESOURCE).equals(
        extractLodEntry(archive, "UN44.def"),
      )
    ) {
      return "unknown";
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
    const biographyPatched =
      heroBios[HERO_ID] === BIOGRAPHY ||
      heroBios[HERO_ID] === BIOGRAPHY.replace(/^Inteus's/, "Nyx's");
    if (
      specialtyRows[HERO_ID + 2]?.[0] === SPECIALTY_FIELDS[0] &&
      specialtyRows[HERO_ID + 2]?.[2] === SPECIALTY_FIELDS[2] &&
      biographyPatched
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

function hdOverrideState(gameDir, spritesExpansion) {
  const filesIni = readGameFile(gameDir, FILES.hdFilesIni).toString("latin1");
  const listedNames = new Set(
    filesIni
      .split(/\r?\n/)
      .map((line) => line.trim().replace(/^"|"$/g, "").toLowerCase())
      .filter(Boolean),
  );
  const listed = SPECIALTY_ICON_RESOURCES.map((resource) =>
    listedNames.has(resource.name.toLowerCase()),
  );
  const existing = HD_OVERRIDE_FILES.map((relativePath) =>
    fs.existsSync(path.join(gameDir, relativePath)),
  );

  if (listed.every((value) => !value) && existing.every((value) => !value)) {
    return "original";
  }
  if (!listed.every(Boolean) || !existing.every(Boolean)) {
    return "unknown";
  }

  try {
    for (let index = 0; index < SPECIALTY_ICON_RESOURCES.length; index += 1) {
      const expected = extractLodEntry(
        spritesExpansion,
        SPECIALTY_ICON_RESOURCES[index].name,
      );
      const actual = readGameFile(gameDir, HD_OVERRIDE_FILES[index]);
      if (!actual.equals(expected)) {
        return "unknown";
      }
    }
    return "patched";
  } catch {
    return "unknown";
  }
}

function displayOverrideState(gameDir, spritesExpansion) {
  const filesIni = readGameFile(gameDir, FILES.hdFilesIni).toString("latin1");
  const listed = filesIni
    .split(/\r?\n/)
    .map((line) => line.trim().replace(/^"|"$/g, "").toLowerCase())
    .includes(SPECIALTY_DISPLAY_RESOURCE.toLowerCase());
  const paths = [FILES.displayResource, HD_DISPLAY_RESOURCE];
  const existing = paths.map((relativePath) =>
    fs.existsSync(path.join(gameDir, relativePath)),
  );

  if (!listed && existing.every((value) => !value)) {
    return "original";
  }
  if (!listed || !existing.every(Boolean)) {
    return "unknown";
  }

  try {
    const expected = extractLodEntry(spritesExpansion, "UN44.def");
    return paths.every((relativePath) =>
      readGameFile(gameDir, relativePath).equals(expected),
    )
      ? "patched"
      : "unknown";
  } catch {
    return "unknown";
  }
}

function patchedHdFilesIni(original) {
  const text = original.toString("latin1");
  const newline = text.includes("\r\n") ? "\r\n" : "\n";
  const lines = text.replace(/\r?\n+$/, "").split(/\r?\n/);
  const listedNames = new Set(
    lines.map((line) =>
      line.trim().replace(/^"|"$/g, "").toLowerCase(),
    ),
  );
  for (const resource of SPECIALTY_ICON_RESOURCES) {
    if (!listedNames.has(resource.name.toLowerCase())) {
      lines.push(`"${resource.name}"`);
    }
  }
  if (!listedNames.has(SPECIALTY_DISPLAY_RESOURCE.toLowerCase())) {
    lines.push(`"${SPECIALTY_DISPLAY_RESOURCE}"`);
  }
  return Buffer.from(`${lines.join(newline)}${newline}`, "latin1");
}

function inspect(gameDir) {
  assertHotAVersion(gameDir);
  const exe = readGameFile(gameDir, FILES.exe);
  const hdExe = readGameFile(gameDir, FILES.hdExe);
  const hdHotA = readGameFile(gameDir, FILES.hdHotA);
  const language = readGameFile(gameDir, FILES.language);
  const spritesBase = readGameFile(gameDir, FILES.spritesBase);
  const spritesExpansion = readGameFile(gameDir, FILES.spritesExpansion);
  return {
    exe,
    hdExe,
    hdHotA,
    language,
    spritesBase,
    spritesExpansion,
    states: {
      [FILES.exe]: executableState(exe),
      [FILES.hdExe]: executableState(hdExe),
      [FILES.hdHotA]: hdHotAState(hdHotA),
      [FILES.language]: languageState(language),
      [FILES.spritesBase]: spriteState(
        spritesBase,
        ORIGINAL_HASHES[FILES.spritesBase],
      ),
      [FILES.spritesExpansion]: spriteState(
        spritesExpansion,
        ORIGINAL_HASHES[FILES.spritesExpansion],
      ),
      [HD_OVERRIDE_STATE_KEY]: hdOverrideState(gameDir, spritesExpansion),
      [DISPLAY_OVERRIDE_STATE_KEY]: displayOverrideState(
        gameDir,
        spritesExpansion,
      ),
    },
  };
}

function inspectedBuffers(inspected) {
  return {
    [FILES.exe]: inspected.exe,
    [FILES.hdExe]: inspected.hdExe,
    [FILES.hdHotA]: inspected.hdHotA,
    [FILES.language]: inspected.language,
    [FILES.spritesBase]: inspected.spritesBase,
    [FILES.spritesExpansion]: inspected.spritesExpansion,
  };
}

function verifyHashes(inspected, expectedHashes) {
  const buffers = inspectedBuffers(inspected);
  for (const relativePath of PATCHED_FILES) {
    const actual = hash(buffers[relativePath]);
    if (actual !== expectedHashes[relativePath]) {
      throw new Error(
        `${relativePath} does not match the supported HotA 1.8.0 build.\n` +
          `Expected ${expectedHashes[relativePath]}\nActual   ${actual}`,
      );
    }
  }
}

function hasStates(actual, expected) {
  return PATCHED_STATE_KEYS.every(
    (key) => actual[key] === expected[key],
  );
}

function timestamp() {
  return new Date()
    .toISOString()
    .replace(/[-:]/g, "")
    .replace(/\..*$/, "")
    .replace("T", "-");
}

function createBackup(gameDir, states) {
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
    states,
  };

  for (const relativePath of PATCHED_FILES) {
    const source = path.join(gameDir, relativePath);
    const destination = path.join(backupDir, relativePath);
    fs.mkdirSync(path.dirname(destination), { recursive: true });
    fs.copyFileSync(source, destination);
    manifest.files[relativePath] = hash(fs.readFileSync(destination));
  }
  for (const relativePath of [
    FILES.hdFilesIni,
    ...HD_OVERRIDE_FILES,
    FILES.displayResource,
    HD_DISPLAY_RESOURCE,
  ]) {
    const source = path.join(gameDir, relativePath);
    if (!fs.existsSync(source)) {
      manifest.files[relativePath] = null;
      continue;
    }
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
  const originalStates = Object.fromEntries(
    PATCHED_STATE_KEYS.map((key) => [key, "original"]),
  );
  const v101States = {
    [FILES.exe]: "legacy",
    [FILES.hdExe]: "legacy",
    [FILES.hdHotA]: "original",
    [FILES.language]: "patched",
    [FILES.spritesBase]: "original",
    [FILES.spritesExpansion]: "legacy",
    [HD_OVERRIDE_STATE_KEY]: "original",
    [DISPLAY_OVERRIDE_STATE_KEY]: "original",
  };
  const v102States = {
    ...v101States,
    [FILES.spritesBase]: "legacy",
  };
  const v103States = {
    ...v102States,
    [HD_OVERRIDE_STATE_KEY]: "patched",
  };
  const v104States = {
    ...v103States,
    [FILES.exe]: "patched",
    [FILES.hdExe]: "patched",
    [DISPLAY_OVERRIDE_STATE_KEY]: "patched",
  };
  const v105States = {
    ...v104States,
    [FILES.hdHotA]: "legacy",
  };
  const v106States = {
    ...v105States,
    [FILES.spritesBase]: "patched",
    [FILES.spritesExpansion]: "patched",
  };
  const v107States = {
    ...v106States,
    [FILES.hdHotA]: "uncentered",
  };
  const v108States = {
    ...v107States,
    [FILES.hdHotA]: "high",
  };
  const v109States = {
    ...v108States,
    [FILES.hdHotA]: "right",
  };
  const isOriginal = hasStates(inspected.states, originalStates);
  const isV101 = hasStates(inspected.states, v101States);
  const isV102 = hasStates(inspected.states, v102States);
  const isV103 = hasStates(inspected.states, v103States);
  const isV104 = hasStates(inspected.states, v104States);
  const isV105 = hasStates(inspected.states, v105States);
  const isV106 = hasStates(inspected.states, v106States);
  const isV107 = hasStates(inspected.states, v107States);
  const isV108 = hasStates(inspected.states, v108States);
  const isV109 = hasStates(inspected.states, v109States);
  if (
    !isOriginal &&
    !isV101 &&
    !isV102 &&
    !isV103 &&
    !isV104 &&
    !isV105 &&
    !isV106 &&
    !isV107 &&
    !isV108 &&
    !isV109
  ) {
    throw new Error(
      `The installation is in a mixed or unknown state:\n${JSON.stringify(inspected.states, null, 2)}`,
    );
  }
  verifyHashes(
    inspected,
    isOriginal
      ? ORIGINAL_HASHES
      : isV101
        ? V101_HASHES
      : isV104
        ? V104_HASHES
      : isV105
        ? V105_HASHES
      : isV106
        ? V106_HASHES
      : isV107
        ? V107_HASHES
      : isV108
        ? V108_HASHES
      : isV109
        ? V109_HASHES
        : V102_HASHES,
  );

  const backupDir = createBackup(gameDir, inspected.states);
  const patchExecutable = (buffer) => {
    const updated = Buffer.from(buffer);
    PATCHED_SPECIALTY.copy(updated, SPECIALTY_OFFSET);
    PATCHED_HERO.copy(updated, HERO_DATA_OFFSET);
    PATCHED_DISPLAY_POINTER.copy(
      updated,
      SPECIALTY_DISPLAY_POINTER_OFFSET,
    );
    PATCHED_DISPLAY_STRING.copy(
      updated,
      SPECIALTY_DISPLAY_STRING_OFFSET,
    );
    return updated;
  };

  fs.writeFileSync(path.join(gameDir, FILES.exe), patchExecutable(inspected.exe));
  fs.writeFileSync(path.join(gameDir, FILES.hdExe), patchExecutable(inspected.hdExe));
  const patchedHdHotA = Buffer.from(inspected.hdHotA);
  PATCHED_HD_HOTA_SPECIALTY_FRAME.copy(
    patchedHdHotA,
    HD_HOTA_SPECIALTY_FRAME_OFFSET,
  );
  PATCHED_HD_HOTA_SPECIALTY_POINTER.copy(
    patchedHdHotA,
    HD_HOTA_SPECIALTY_POINTER_OFFSET,
  );
  PATCHED_HD_HOTA_SPECIALTY_POSITION.copy(
    patchedHdHotA,
    HD_HOTA_SPECIALTY_POSITION_OFFSET,
  );
  PATCHED_HD_HOTA_SPECIALTY_LAYOUT.copy(
    patchedHdHotA,
    HD_HOTA_SPECIALTY_LAYOUT_OFFSET,
  );
  ORIGINAL_HD_HOTA_SPECIALTY_RESOURCE.copy(
    patchedHdHotA,
    HD_HOTA_SPECIALTY_RESOURCE_OFFSET,
  );
  fs.writeFileSync(path.join(gameDir, FILES.hdHotA), patchedHdHotA);
  if (inspected.states[FILES.language] === "original") {
    fs.writeFileSync(
      path.join(gameDir, FILES.language),
      patchedLanguageArchive(inspected.language),
    );
  }
  for (const [relativePath, archive] of [
    [FILES.spritesBase, inspected.spritesBase],
    [FILES.spritesExpansion, inspected.spritesExpansion],
  ]) {
    if (inspected.states[relativePath] === "original") {
      fs.writeFileSync(
        path.join(gameDir, relativePath),
        patchedSpriteArchive(archive),
      );
    } else if (inspected.states[relativePath] === "legacy") {
      fs.writeFileSync(
        path.join(gameDir, relativePath),
        registeredDisplayResource(archive),
      );
    }
  }
  const patchedExpansion = readGameFile(gameDir, FILES.spritesExpansion);
  for (let index = 0; index < SPECIALTY_ICON_RESOURCES.length; index += 1) {
    fs.writeFileSync(
      path.join(gameDir, HD_OVERRIDE_FILES[index]),
      extractLodEntry(
        patchedExpansion,
        SPECIALTY_ICON_RESOURCES[index].name,
      ),
    );
  }
  const displayResource = extractLodEntry(patchedExpansion, "UN44.def");
  for (const relativePath of [
    FILES.displayResource,
    HD_DISPLAY_RESOURCE,
  ]) {
    fs.writeFileSync(
      path.join(gameDir, relativePath),
      displayResource,
    );
  }
  fs.writeFileSync(
    path.join(gameDir, FILES.hdFilesIni),
    patchedHdFilesIni(readGameFile(gameDir, FILES.hdFilesIni)),
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

function safeManifestPath(root, relativePath) {
  const normalized = path.normalize(relativePath);
  if (
    path.isAbsolute(relativePath) ||
    normalized === "." ||
    normalized === ".." ||
    normalized.startsWith(`..${path.sep}`)
  ) {
    throw new Error(`Unsafe backup manifest path: ${relativePath}`);
  }
  return path.join(root, normalized);
}

function restoreFinalizerBackup(gameDir, backupDir, manifest) {
  const entries = Object.entries(manifest.files);
  if (!entries.length) {
    throw new Error(`Backup manifest contains no files: ${backupDir}`);
  }

  for (const [relativePath, expectedHash] of entries) {
    safeManifestPath(gameDir, relativePath);
    const source = safeManifestPath(backupDir, relativePath);
    if (expectedHash === null) {
      continue;
    }
    if (
      !fs.existsSync(source) ||
      hash(fs.readFileSync(source)) !== expectedHash
    ) {
      throw new Error(`Backup checksum mismatch: ${source}`);
    }
  }

  for (const [relativePath, expectedHash] of entries) {
    const source = safeManifestPath(backupDir, relativePath);
    const destination = safeManifestPath(gameDir, relativePath);
    if (expectedHash === null) {
      if (fs.existsSync(destination)) {
        fs.unlinkSync(destination);
      }
      continue;
    }
    fs.mkdirSync(path.dirname(destination), { recursive: true });
    fs.copyFileSync(source, destination);
  }

  for (const [relativePath, expectedHash] of entries) {
    const destination = safeManifestPath(gameDir, relativePath);
    if (expectedHash === null) {
      if (fs.existsSync(destination)) {
        throw new Error(`Restore verification failed: ${destination}`);
      }
    } else if (hash(fs.readFileSync(destination)) !== expectedHash) {
      throw new Error(`Restore verification failed: ${destination}`);
    }
  }
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

  if (manifest.patch === "Nyx Pixie Elementalist finalizer") {
    restoreFinalizerBackup(gameDir, backupDir, manifest);
    console.log(`Restored files from ${backupDir}`);
    return;
  }

  for (const relativePath of PATCHED_FILES) {
    if (!manifest.files[relativePath]) {
      throw new Error(
        `Backup predates the current specialty-picture patch: ${backupDir}`,
      );
    }
    const source = path.join(backupDir, relativePath);
    const data = fs.readFileSync(source);
    if (hash(data) !== manifest.files[relativePath]) {
      throw new Error(`Backup checksum mismatch: ${source}`);
    }
  }
  if (!Object.hasOwn(manifest.files, FILES.hdFilesIni)) {
    throw new Error(`Backup predates the HD Mod override patch: ${backupDir}`);
  }
  for (const relativePath of [
    FILES.hdFilesIni,
    ...HD_OVERRIDE_FILES,
    FILES.displayResource,
    HD_DISPLAY_RESOURCE,
  ]) {
    const expectedHash = manifest.files[relativePath];
    if (expectedHash === null) {
      continue;
    }
    const source = path.join(backupDir, relativePath);
    if (
      !expectedHash ||
      !fs.existsSync(source) ||
      hash(fs.readFileSync(source)) !== expectedHash
    ) {
      throw new Error(`Backup checksum mismatch: ${source}`);
    }
  }
  for (const relativePath of PATCHED_FILES) {
    fs.copyFileSync(path.join(backupDir, relativePath), path.join(gameDir, relativePath));
  }
  fs.copyFileSync(
    path.join(backupDir, FILES.hdFilesIni),
    path.join(gameDir, FILES.hdFilesIni),
  );
  for (const relativePath of [
    ...HD_OVERRIDE_FILES,
    FILES.displayResource,
    HD_DISPLAY_RESOURCE,
  ]) {
    const destination = path.join(gameDir, relativePath);
    if (manifest.files[relativePath] === null) {
      if (fs.existsSync(destination)) {
        fs.unlinkSync(destination);
      }
    } else {
      fs.copyFileSync(path.join(backupDir, relativePath), destination);
    }
  }

  const verified = inspect(gameDir);
  if (
    !manifest.states ||
    !hasStates(verified.states, manifest.states)
  ) {
    throw new Error("Restore verification failed.");
  }
  console.log(`Restored files from ${backupDir}`);
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
