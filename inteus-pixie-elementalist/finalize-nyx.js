#!/usr/bin/env node

"use strict";

const crypto = require("crypto");
const fs = require("fs");
const path = require("path");
const zlib = require("zlib");

const gameDir = path.resolve(process.argv[2] || "..");
const files = {
  exe: "h3hota.exe",
  hdExe: "h3hota HD.exe",
  dll: "HD_HOTA.dll",
  language: path.join("Data", "HotA_lng.lod"),
  spritesBase: path.join("Data", "H3sprite.lod"),
  spritesExpansion: path.join("Data", "H3ab_spr.lod"),
  bitmap: path.join("Data", "H3bitmap.lod"),
  bitmapExpansion: path.join("Data", "H3ab_bmp.lod"),
  hdFilesIni: path.join("_HD3_Data", "Compability", "#hota", "Files.ini"),
  hdFilesIni15: path.join("_HD3_Data", "Compability", "#hota15", "Files.ini"),
};
const runtimeHookPath = path.join("_HD3_Data", "Common", "setseed.dll");
const runtimeHookAsset = fs.readFileSync(
  path.join(__dirname, "assets", "NyxRuntimeFix.dll"),
);
const dialogBackgroundName = "DiBoxBck.pcx";
const portraitNames = ["HPL004EL.pcx", "HPS004EL.pcx"];
const portraitAssets = portraitNames.map((name) =>
  fs.readFileSync(path.join(__dirname, "assets", name)),
);
const portraitBmpNames = portraitNames.map((name) =>
  name.replace(/\.pcx$/i, ".bmp"),
);
const scenarioName = "IX32.def";
const scenarioLoosePath = path.join("Data", scenarioName);
const displayName = "IX44.def";
const displayLoosePath = path.join("Data", displayName);
const specialtyVariants = [
  {
    sourceName: "UN32.def",
    targetName: scenarioName,
    frameName: "NYX32PIX.PCX",
    size: 32,
  },
  {
    sourceName: "UN44.def",
    targetName: displayName,
    frameName: "NYX44PIX.PCX",
    size: 44,
  },
];
const specialtyNames = ["UN32.def", "UN44.def", scenarioName, displayName];
const scenarioStringOffset = 0x2817dc;
const scenarioDllStringOffset = 0x295ff0;
const originalScenarioName = Buffer.from("un32.def\0", "latin1");
const patchedScenarioName = Buffer.from("ix32.def\0", "latin1");
const heroDataOffset = 0x27d020;
const armyAmountsOffset = heroDataOffset + 0x44;
const originalHeroRecord = Buffer.from(
  "00000000070000001100000007000000010000000e00000001000000010000002b000000760000007000000073000000",
  "hex",
);
const legacyHeroRecord = Buffer.from(
  "0000000007000000110000000e000000010000001300000001000000010000000d000000760000007000000073000000",
  "hex",
);
const finalHeroRecord = Buffer.from(
  "0000000007000000110000000e000000010000000700000001000000010000000d000000760000007600000076000000",
  "hex",
);
const emptyArmyAmounts = Buffer.alloc(24);
const finalArmyAmounts = Buffer.from(
  "160000001900000016000000190000001600000019000000",
  "hex",
);
const popupFrameOffset = 0x234d86;
const popupPointerOffset = 0x234d8b;
const positionOffset = 0x234d9a;
const mirrorOffset = 0x234d83;
const layoutOffset = 0x234dc3;
const originalPopupFrame = Buffer.from("8b450850", "hex");
const directPixiePopupFrame = Buffer.from("6a789090", "hex");
const originalPopupPointer = Buffer.from("3c042a01", "hex");
const directPixiePopupPointer = Buffer.from("ac6d2901", "hex");
const originalPosition = Buffer.from("6a126a48", "hex");
const finalPosition = Buffer.from("6a126a4e", "hex");
const originalMirror = Buffer.from([0x00]);
const finalMirror = Buffer.from([0x01]);
const originalLayout = Buffer.from(
  "8b4dcc8b51308b45cc668b4a306689481c8b55cc8b42308b4dcc668b50346689511e8b",
  "hex",
);
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

function lodPcxToBmp(pcx, expectedWidth, expectedHeight) {
  if (pcx.length < 12 + 768) {
    throw new Error("LOD-PCX asset is truncated.");
  }
  const pixelSize = pcx.readUInt32LE(0);
  const width = pcx.readUInt32LE(4);
  const height = pcx.readUInt32LE(8);
  if (
    width !== expectedWidth ||
    height !== expectedHeight ||
    pixelSize !== width * height ||
    pcx.length !== 12 + pixelSize + 768
  ) {
    throw new Error(`Unexpected LOD-PCX layout: ${width}x${height}.`);
  }

  const pixelOffset = 14 + 40 + 256 * 4;
  const rowStride = (width + 3) & ~3;
  const imageSize = rowStride * height;
  const bmp = Buffer.alloc(pixelOffset + imageSize);

  bmp.write("BM", 0, "ascii");
  bmp.writeUInt32LE(bmp.length, 2);
  bmp.writeUInt32LE(pixelOffset, 10);
  bmp.writeUInt32LE(40, 14);
  bmp.writeInt32LE(width, 18);
  bmp.writeInt32LE(height, 22);
  bmp.writeUInt16LE(1, 26);
  bmp.writeUInt16LE(8, 28);
  bmp.writeUInt32LE(imageSize, 34);
  bmp.writeUInt32LE(256, 46);
  bmp.writeUInt32LE(256, 50);

  const sourcePixels = pcx.subarray(12, 12 + pixelSize);
  const sourcePalette = pcx.subarray(12 + pixelSize);
  for (let index = 0; index < 256; index += 1) {
    const source = index * 3;
    const destination = 54 + index * 4;
    bmp[destination] = sourcePalette[source + 2];
    bmp[destination + 1] = sourcePalette[source + 1];
    bmp[destination + 2] = sourcePalette[source];
  }
  for (let y = 0; y < height; y += 1) {
    sourcePixels.copy(
      bmp,
      pixelOffset + (height - 1 - y) * rowStride,
      y * width,
      (y + 1) * width,
    );
  }
  return bmp;
}

function decodeLodPcx(pcx) {
  if (pcx.length < 12 + 768) {
    throw new Error("LOD-PCX resource is truncated.");
  }
  const pixelSize = pcx.readUInt32LE(0);
  const width = pcx.readUInt32LE(4);
  const height = pcx.readUInt32LE(8);
  if (
    pixelSize !== width * height ||
    pcx.length !== 12 + pixelSize + 768
  ) {
    throw new Error(`Unexpected LOD-PCX layout: ${width}x${height}.`);
  }
  return {
    height,
    palette: pcx.subarray(12 + pixelSize),
    pixels: pcx.subarray(12, 12 + pixelSize),
    width,
  };
}

const portraitBmpAssets = [
  lodPcxToBmp(portraitAssets[0], 58, 64),
  lodPcxToBmp(portraitAssets[1], 48, 32),
];

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

function addOrReplaceLodEntry(archive, name, replacement, type = 0) {
  try {
    if (extractLodEntry(archive, name).equals(replacement)) {
      return archive;
    }
    return replaceLodEntry(archive, name, replacement);
  } catch (error) {
    if (error.message !== `LOD entry not found: ${name}`) {
      throw error;
    }
  }

  if (Buffer.byteLength(name, "latin1") > 15) {
    throw new Error(`LOD entry name is too long: ${name}`);
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
      return {
        frameOffset: definition.readUInt32LE(
          offsetsOffset + localFrame * 4,
        ),
        nameOffset: namesOffset + localFrame * 13,
        offsetTableEntry: offsetsOffset + localFrame * 4,
      };
    }
    firstFrame += frameCount;
    directoryOffset = nextDirectoryOffset;
  }
  throw new Error(`DEF frame not found: ${requestedFrame}`);
}

function renameDefFrame(definition, frameIndex, frameName) {
  if (Buffer.byteLength(frameName, "latin1") > 12) {
    throw new Error(`DEF frame name is too long: ${frameName}`);
  }
  const frame = findDefFrame(definition, frameIndex);
  const updated = Buffer.from(definition);
  updated.fill(0, frame.nameOffset, frame.nameOffset + 13);
  updated.write(frameName, frame.nameOffset, "latin1");
  return updated;
}

function decodeDefFrame(definition, requestedFrame) {
  const location = findDefFrame(definition, requestedFrame);
  const frameOffset = location.frameOffset;
  if (frameOffset + 32 > definition.length) {
    throw new Error(`Truncated DEF frame header: ${requestedFrame}`);
  }
  const compression = definition.readUInt32LE(frameOffset + 4);
  const width = definition.readUInt32LE(frameOffset + 16);
  const height = definition.readUInt32LE(frameOffset + 20);
  const dataOffset = frameOffset + 32;
  const pixels = Buffer.alloc(width * height);

  if (compression === 0) {
    const end = dataOffset + pixels.length;
    if (end > definition.length) {
      throw new Error(`Truncated DEF frame data: ${requestedFrame}`);
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

  return { ...location, height, pixels, width };
}

function paletteColor(definition, paletteIndex) {
  const offset = 16 + paletteIndex * 3;
  return [
    definition[offset],
    definition[offset + 1],
    definition[offset + 2],
  ];
}

function pixieSpecialtyPixels(
  pixieIcons,
  specialtyIcons,
  dialogBackground,
  iconSize,
) {
  const source = decodeDefFrame(pixieIcons, 120);
  if (source.width !== 30 || source.height !== 32) {
    throw new Error(
      `Unexpected CPRSMALL Pixie dimensions: ${source.width}x${source.height}.`,
    );
  }
  if (iconSize < source.width || iconSize < source.height) {
    throw new Error(`Specialty canvas is too small: ${iconSize}.`);
  }

  const sourcePalette = Array.from({ length: 256 }, (_, index) =>
    paletteColor(pixieIcons, index),
  );
  const targetPalette = Array.from({ length: 256 }, (_, index) =>
    paletteColor(specialtyIcons, index),
  );
  const background = decodeLodPcx(dialogBackground);
  if (background.width !== 256 || background.height !== 256) {
    throw new Error(
      `Unexpected dialog background dimensions: ${background.width}x${background.height}.`,
    );
  }
  const backgroundPalette = Array.from({ length: 256 }, (_, index) => {
    const offset = index * 3;
    return [
      background.palette[offset],
      background.palette[offset + 1],
      background.palette[offset + 2],
    ];
  });
  const nearestTargetIndex = (color) => {
    let bestIndex = 8;
    let bestDistance = Number.POSITIVE_INFINITY;
    for (let targetIndex = 8; targetIndex < 256; targetIndex += 1) {
      const targetColor = targetPalette[targetIndex];
      const distance =
        (color[0] - targetColor[0]) ** 2 +
        (color[1] - targetColor[1]) ** 2 +
        (color[2] - targetColor[2]) ** 2;
      if (distance < bestDistance) {
        bestDistance = distance;
        bestIndex = targetIndex;
      }
    }
    return bestIndex;
  };
  const paletteMap = Array.from({ length: 256 }, (_, sourceIndex) => {
    if (sourceIndex < 8) {
      return sourceIndex;
    }
    return nearestTargetIndex(sourcePalette[sourceIndex]);
  });
  const backgroundMap = backgroundPalette.map(nearestTargetIndex);

  const pixels = Buffer.alloc(iconSize * iconSize);
  const textureLeft = 84;
  const textureTop = 0;
  for (let y = 0; y < iconSize; y += 1) {
    for (let x = 0; x < iconSize; x += 1) {
      const backgroundIndex =
        background.pixels[
          ((textureTop + y) % background.height) * background.width +
          ((textureLeft + x) % background.width)
        ];
      pixels[y * iconSize + x] = backgroundMap[backgroundIndex];
    }
  }
  const left = Math.floor((iconSize - source.width) / 2);
  const top = Math.floor((iconSize - source.height) / 2);
  for (let y = 0; y < source.height; y += 1) {
    for (let x = 0; x < source.width; x += 1) {
      const sourceIndex =
        source.pixels[
          y * source.width + (source.width - 1 - x)
        ];
      if (sourceIndex === 0) {
        continue;
      }
      pixels[(top + y) * iconSize + left + x] = paletteMap[sourceIndex];
    }
  }
  return pixels;
}

function replaceDefFramePixels(definition, frameIndex, size, pixels) {
  if (pixels.length !== size * size) {
    throw new Error(`Unexpected ${size}x${size} specialty pixel count.`);
  }
  const location = findDefFrame(definition, frameIndex);
  const current = decodeDefFrame(definition, frameIndex);
  if (
    current.width === size &&
    current.height === size &&
    current.pixels.equals(pixels)
  ) {
    return definition;
  }
  const frame = Buffer.alloc(32 + pixels.length);
  frame.writeUInt32LE(pixels.length, 0);
  frame.writeUInt32LE(0, 4);
  frame.writeUInt32LE(size, 8);
  frame.writeUInt32LE(size, 12);
  frame.writeUInt32LE(size, 16);
  frame.writeUInt32LE(size, 20);
  pixels.copy(frame, 32);

  const updated = Buffer.from(definition);
  updated.writeUInt32LE(updated.length, location.offsetTableEntry);
  return Buffer.concat([updated, frame]);
}

function buildSpecialtyDef(
  archive,
  dialogBackground,
  variant,
  uniqueName,
) {
  const pixieIcons = extractLodEntry(archive, "CPRSMALL.def");
  const specialtyIcons = extractLodEntry(archive, variant.sourceName);
  const pixels = pixieSpecialtyPixels(
    pixieIcons,
    specialtyIcons,
    dialogBackground,
    variant.size,
  );
  const patched = replaceDefFramePixels(
    specialtyIcons,
    140,
    variant.size,
    pixels,
  );
  return uniqueName
    ? renameDefFrame(patched, 140, variant.frameName)
    : patched;
}

function patchScenarioLookup(executable) {
  const current = executable.subarray(
    scenarioStringOffset,
    scenarioStringOffset + originalScenarioName.length,
  );
  if (
    !current.equals(originalScenarioName) &&
    !current.equals(patchedScenarioName)
  ) {
    throw new Error("Unexpected scenario specialty resource name.");
  }
  const updated = Buffer.from(executable);
  patchedScenarioName.copy(updated, scenarioStringOffset);
  return updated;
}

function patchExecutable(executable) {
  const updated = patchScenarioLookup(executable);
  const heroRecord = updated.subarray(
    heroDataOffset,
    heroDataOffset + finalHeroRecord.length,
  );
  if (
    !heroRecord.equals(originalHeroRecord) &&
    !heroRecord.equals(legacyHeroRecord) &&
    !heroRecord.equals(finalHeroRecord)
  ) {
    throw new Error("Unexpected Nyx hero record.");
  }
  const armyAmounts = updated.subarray(
    armyAmountsOffset,
    armyAmountsOffset + finalArmyAmounts.length,
  );
  if (
    !armyAmounts.equals(emptyArmyAmounts) &&
    !armyAmounts.equals(finalArmyAmounts)
  ) {
    throw new Error("Unexpected Nyx starting-army amount record.");
  }
  finalHeroRecord.copy(updated, heroDataOffset);
  finalArmyAmounts.copy(updated, armyAmountsOffset);
  return updated;
}

function patchSpecialtyArchive(archive, dialogBackground) {
  let updated = archive;
  for (const variant of specialtyVariants) {
    const source = findLodEntry(updated, variant.sourceName);
    const originalSource = extractLodEntry(updated, variant.sourceName);
    const patchedSource = buildSpecialtyDef(
      updated,
      dialogBackground,
      variant,
      false,
    );
    if (!originalSource.equals(patchedSource)) {
      updated = replaceLodEntry(updated, variant.sourceName, patchedSource);
    }
    const replacement = renameDefFrame(
      patchedSource,
      140,
      variant.frameName,
    );
    updated = addOrReplaceLodEntry(
      updated,
      variant.targetName,
      replacement,
      updated.readUInt32LE(source.directoryOffset + 24),
    );
  }
  return updated;
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
  const fields = rows[index].split("\t");
  const legacyFields = [
    "15",
    "25",
    "Pixies",
    "3",
    "5",
    "Air Elementals",
    "3",
    "5",
    "Water Elementals",
  ];
  const finalFields = [
    "22",
    "25",
    "Pixies",
    "22",
    "25",
    "Pixies",
    "22",
    "25",
    "Pixies",
  ];
  if (
    fields.length !== 10 ||
    !["Inteus", "Nyx"].includes(fields[0]) ||
    (
      !fields.slice(1).every((field, fieldIndex) =>
        field === legacyFields[fieldIndex]
      ) &&
      !fields.slice(1).every((field, fieldIndex) =>
        field === finalFields[fieldIndex]
      )
    )
  ) {
    throw new Error("Unexpected Inteus/Nyx row in HOTRAITS.TXT.");
  }
  rows[index] = ["Nyx", ...finalFields].join("\t");

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
  const popupFrame = buffer.subarray(
    popupFrameOffset,
    popupFrameOffset + originalPopupFrame.length,
  );
  if (
    !popupFrame.equals(originalPopupFrame) &&
    !popupFrame.equals(directPixiePopupFrame)
  ) {
    throw new Error("Unexpected specialty frame code in HD_HOTA.dll.");
  }
  const popupPointer = buffer.subarray(
    popupPointerOffset,
    popupPointerOffset + originalPopupPointer.length,
  );
  if (
    !popupPointer.equals(originalPopupPointer) &&
    !popupPointer.equals(directPixiePopupPointer)
  ) {
    throw new Error("Unexpected specialty resource pointer in HD_HOTA.dll.");
  }
  const position = buffer.subarray(positionOffset, positionOffset + 4);
  if (
    !position.equals(originalPosition) &&
    !position.equals(finalPosition)
  ) {
    throw new Error("Unexpected specialty position bytes in HD_HOTA.dll.");
  }
  const scenarioResource = buffer.subarray(
    scenarioDllStringOffset,
    scenarioDllStringOffset + originalScenarioName.length,
  );
  if (
    !scenarioResource.equals(originalScenarioName) &&
    !scenarioResource.equals(patchedScenarioName)
  ) {
    throw new Error("Unexpected scenario resource name in HD_HOTA.dll.");
  }
  const mirror = buffer.subarray(mirrorOffset, mirrorOffset + 1);
  if (!mirror.equals(originalMirror) && !mirror.equals(finalMirror)) {
    throw new Error("Unexpected Pixie mirror argument in HD_HOTA.dll.");
  }
  const hasLayout = (expected) =>
    buffer
      .subarray(layoutOffset, layoutOffset + expected.length)
      .equals(expected);
  if (
    !hasLayout(originalLayout) &&
    !hasLayout(currentLayout) &&
    !hasLayout(finalLayout)
  ) {
    throw new Error("Unexpected specialty layout code in HD_HOTA.dll.");
  }
  const updated = Buffer.from(buffer);
  patchedScenarioName.copy(updated, scenarioDllStringOffset);
  originalPopupFrame.copy(updated, popupFrameOffset);
  originalPopupPointer.copy(updated, popupPointerOffset);
  originalMirror.copy(updated, mirrorOffset);
  originalPosition.copy(updated, positionOffset);
  originalLayout.copy(updated, layoutOffset);
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
const dialogBackground = extractLodEntry(
  originals[files.bitmap],
  dialogBackgroundName,
);
if (
  fs.existsSync(path.join(gameDir, runtimeHookPath)) &&
  !read(runtimeHookPath).equals(runtimeHookAsset)
) {
  throw new Error(
    `${runtimeHookPath} is occupied by another DLL; refusing to overwrite it.`,
  );
}
const scenarioSourceDef = buildSpecialtyDef(
  originals[files.spritesExpansion],
  dialogBackground,
  specialtyVariants[0],
  false,
);
const scenarioDef = renameDefFrame(
  scenarioSourceDef,
  140,
  specialtyVariants[0].frameName,
);
const displaySourceDef = buildSpecialtyDef(
  originals[files.spritesExpansion],
  dialogBackground,
  specialtyVariants[1],
  false,
);
const displayDef = renameDefFrame(
  displaySourceDef,
  140,
  specialtyVariants[1].frameName,
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
for (const relativePath of [scenarioLoosePath, displayLoosePath]) {
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
{
  const source = path.join(gameDir, runtimeHookPath);
  if (!fs.existsSync(source)) {
    manifest.files[runtimeHookPath] = null;
  } else {
    const destination = path.join(backupDir, runtimeHookPath);
    fs.mkdirSync(path.dirname(destination), { recursive: true });
    const buffer = fs.readFileSync(source);
    fs.writeFileSync(destination, buffer);
    manifest.files[runtimeHookPath] = hash(buffer);
  }
}
for (const pack of ["#hota", "#hota15"]) {
  for (const name of [
    ...portraitNames,
    ...portraitBmpNames,
    ...specialtyNames,
  ]) {
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

writeIfChanged(files.exe, patchExecutable(originals[files.exe]));
writeIfChanged(files.hdExe, patchExecutable(originals[files.hdExe]));
writeIfChanged(files.dll, patchDll(originals[files.dll]));
writeIfChanged(files.language, patchLanguage(originals[files.language]));
writeIfChanged(
  files.spritesBase,
  patchSpecialtyArchive(originals[files.spritesBase], dialogBackground),
);
writeIfChanged(
  files.spritesExpansion,
  patchSpecialtyArchive(
    originals[files.spritesExpansion],
    dialogBackground,
  ),
);
writeIfChanged(files.bitmap, patchPortraits(originals[files.bitmap]));
writeIfChanged(
  files.bitmapExpansion,
  patchPortraits(originals[files.bitmapExpansion]),
);
writeIfChanged(
  files.hdFilesIni,
  patchFilesIni(
    originals[files.hdFilesIni],
    [...specialtyNames, ...portraitBmpNames],
    portraitNames,
  ),
);
writeIfChanged(
  files.hdFilesIni15,
  patchFilesIni(
    originals[files.hdFilesIni15],
    [...specialtyNames, ...portraitBmpNames],
    portraitNames,
  ),
);
fs.writeFileSync(path.join(gameDir, scenarioLoosePath), scenarioDef);
fs.writeFileSync(path.join(gameDir, displayLoosePath), displayDef);
fs.writeFileSync(path.join(gameDir, runtimeHookPath), runtimeHookAsset);
fs.writeFileSync(
  path.join(
    gameDir,
    "_HD3_Data",
    "Compability",
    "#hota",
    specialtyVariants[0].sourceName,
  ),
  scenarioSourceDef,
);
fs.writeFileSync(
  path.join(
    gameDir,
    "_HD3_Data",
    "Compability",
    "#hota",
    specialtyVariants[1].sourceName,
  ),
  displaySourceDef,
);
fs.writeFileSync(
  path.join(
    gameDir,
    "_HD3_Data",
    "Compability",
    "#hota",
    scenarioName,
  ),
  scenarioDef,
);
fs.writeFileSync(
  path.join(
    gameDir,
    "_HD3_Data",
    "Compability",
    "#hota",
    displayName,
  ),
  displayDef,
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
  for (let index = 0; index < portraitBmpNames.length; index += 1) {
    fs.writeFileSync(
      path.join(
        gameDir,
        "_HD3_Data",
        "Compability",
        pack,
        portraitBmpNames[index],
      ),
      portraitBmpAssets[index],
    );
  }
  for (const name of specialtyNames) {
    fs.writeFileSync(
      path.join(gameDir, "_HD3_Data", "Compability", pack, name),
      read(path.join("_HD3_Data", "Compability", "#hota", name)),
    );
  }
}

for (const relativePath of [files.exe, files.hdExe]) {
  const executable = read(relativePath);
  const installed = executable.subarray(
    scenarioStringOffset,
    scenarioStringOffset + patchedScenarioName.length,
  );
  if (!installed.equals(patchedScenarioName)) {
    throw new Error(`Scenario resource redirection failed: ${relativePath}`);
  }
  if (
    !executable
      .subarray(
        heroDataOffset,
        heroDataOffset + finalHeroRecord.length,
      )
      .equals(finalHeroRecord)
  ) {
    throw new Error(`Hero record verification failed: ${relativePath}`);
  }
  if (
    !executable
      .subarray(
        armyAmountsOffset,
        armyAmountsOffset + finalArmyAmounts.length,
      )
      .equals(finalArmyAmounts)
  ) {
    throw new Error(
      `Starting-army amount verification failed: ${relativePath}`,
    );
  }
}
const installedDllScenario = read(files.dll).subarray(
  scenarioDllStringOffset,
  scenarioDllStringOffset + patchedScenarioName.length,
);
if (!installedDllScenario.equals(patchedScenarioName)) {
  throw new Error("Scenario resource redirection failed: HD_HOTA.dll");
}
if (
  !read(files.dll)
    .subarray(popupFrameOffset, popupFrameOffset + originalPopupFrame.length)
    .equals(originalPopupFrame) ||
  !read(files.dll)
    .subarray(
      popupPointerOffset,
      popupPointerOffset + originalPopupPointer.length,
    )
    .equals(originalPopupPointer) ||
  !read(files.dll)
    .subarray(mirrorOffset, mirrorOffset + originalMirror.length)
    .equals(originalMirror) ||
  !read(files.dll)
    .subarray(positionOffset, positionOffset + originalPosition.length)
    .equals(originalPosition) ||
  !read(files.dll)
    .subarray(layoutOffset, layoutOffset + originalLayout.length)
    .equals(originalLayout)
) {
  throw new Error("Native specialty-popup restoration failed: HD_HOTA.dll");
}
for (const relativePath of [files.spritesBase, files.spritesExpansion]) {
  const archive = read(relativePath);
  for (const variant of specialtyVariants) {
    const expected = renameDefFrame(
      extractLodEntry(archive, variant.sourceName),
      140,
      variant.frameName,
    );
    if (!extractLodEntry(archive, variant.targetName).equals(expected)) {
      throw new Error(
        `Specialty DEF verification failed: ${relativePath}/${variant.targetName}`,
      );
    }
    const actual = decodeDefFrame(
      extractLodEntry(archive, variant.targetName),
      140,
    );
    const transparentPixels = actual.pixels.reduce(
      (count, pixel) => count + (pixel < 8 ? 1 : 0),
      0,
    );
    const cyanKeyPixels = actual.pixels.reduce(
      (count, pixel) => count + (pixel === 0 ? 1 : 0),
      0,
    );
    if (transparentPixels === 0 || cyanKeyPixels !== 0) {
      throw new Error(
        `Specialty background verification failed: ${relativePath}/${variant.targetName}`,
      );
    }
  }
}
if (!read(scenarioLoosePath).equals(scenarioDef)) {
  throw new Error(`Scenario DEF verification failed: ${scenarioLoosePath}`);
}
if (!read(displayLoosePath).equals(displayDef)) {
  throw new Error(`Specialty DEF verification failed: ${displayLoosePath}`);
}
if (!read(runtimeHookPath).equals(runtimeHookAsset)) {
  throw new Error(`Runtime hook verification failed: ${runtimeHookPath}`);
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
      patchFilesIni(
        filesIni,
        [...specialtyNames, ...portraitBmpNames],
        portraitNames,
      ),
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
  for (let index = 0; index < portraitBmpNames.length; index += 1) {
    const installed = read(
      path.join(
        "_HD3_Data",
        "Compability",
        pack,
        portraitBmpNames[index],
      ),
    );
    if (!installed.equals(portraitBmpAssets[index])) {
      throw new Error(
        `HD portrait BMP verification failed: ${pack}/${portraitBmpNames[index]}`,
      );
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
