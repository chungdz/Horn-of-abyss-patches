# Changelog

## 1.7.0 - 2026-08-14

- Restore the failed `UN32.def` to `IX32.def` redirects in both executables
  and `HD_HOTA.dll` to their original bytes.
- Restore the obsolete executable `IX44.def` display pointer and code-cave
  string to their original pointer and NOP bytes.
- Remove installed `NyxImageTrace.log` and `NyxRuntimeProbe.log`; retain only
  `NyxRuntimeFix.log`, which belongs to the working frame-table swap.
- Verify that final `HD_HOTA.dll` is byte-identical to stock HotA 1.8.0.
- Add a checksum-driven `restore-original` command. It creates a safety backup,
  restores original files from the backup history, removes Nyx-generated
  resources, and verifies every patch component as `original`.
- Test the reverse path by restoring the automatically created safety backup
  and verifying the cleaned Nyx state again.
- Add `RUNTIME-TRACING.md` with the static and live investigation workflow.

## 1.6.1 - 2026-08-14

- Record in-game confirmation that all three Nyx starting stacks use the
  `22-25 Pixies` range.
- Put Basic Wisdom in the first secondary-skill slot and Basic Fire Magic in
  the second slot.
- Correct Nyx's biography to use feminine pronouns.
- Guard upgrades from the complete 1.6.0 installation.

## 1.6.0 - 2026-08-14

- Correct the authoritative `HOTRAITS.TXT` row for Nyx to three identical
  `22-25 Pixies` stacks. The previous executable-only amount edit was
  overwritten when HotA loaded this text table.
- Restore the random-map Advanced Options specialty control to its native
  `UN44.def`, hero-ID frame path.
- Reuse the runtime-swapped, uniquely named `IX44.def` frame already
  confirmed in scenarios, so the random-map image faces left and uses the
  same brown-backed artwork instead of raw `CPRSMALL.def`.
- Add guarded upgrade recognition for the complete 1.5.0 installation and
  verify the final `HOTRAITS.TXT` fields and native popup constructor bytes.

## 1.5.0 - 2026-08-14

- Replace Basic Tactics with Basic Wisdom while retaining Basic Fire Magic
  and Fire Wall.
- Change all three starting creature IDs to Pixie and set every low/high
  amount pair to 22/25.
- Mirror the Pixie pixels in `IX32.def` and `IX44.def` so the standard
  specialty pictures face left.
- Mirror the separate `CPRSMALL.def` control in the random-map Advanced
  Options popup through its `HD_HOTA.dll` constructor argument.
- Record that transparent pixels in standard specialty views reveal the
  dialog's cyan placeholder because the original atlas frames are full-square
  artwork.
- Composite the mirrored Pixie over the native `DiBoxBck.pcx` brown dialog
  texture instead. Verify both generated frames contain zero cyan-key pixels.
- Guard upgrades from the previous Fire Magic/Tactics and mixed-creature
  record, back up all touched files, and update `patch.js` state inspection.

## 1.4.0 - 2026-08-14

- Trace the standard scenario and in-game specialty paths at runtime and
  confirm that neither uses the previously patched image-constructor path.
- Add a custom runtime fix that replaces only loaded frame 140 in `UN32.def`
  and `UN44.def` with the corresponding `IX32.def` and `IX44.def` frames.
- Identify Heroes III's per-frame cache collision: copied atlases initially
  reused the internal names `Un32Lust.PCX` and `Un44Lust.PCX`, so the engine
  returned the already-cached Bloodlust frame.
- Give the replacement frames unique internal names `NYX32PIX.PCX` and
  `NYX44PIX.PCX`. In-game testing confirmed Pixie pictures in both standard
  scenario hero selection and the entered scenario's hero screen.
- Install the reviewed runtime DLL through HD Mod's optional `setseed.dll`
  loader slot, back it up, verify its hash, and refuse to overwrite an
  unrelated DLL.
- Remove the Conflux creature-portrait background. Both specialty atlases now
  use the exact transparent 30x32 `CPRSMALL.def` frame 120 shown in the
  random-map popup, centered without scaling on 32x32 and 44x44 canvases.
- Preserve DEF special palette indices 0-7 and verify transparency, unique
  frame names, dimensions, and sprite bounds after installation.
- Update `patch.js` generation and status inspection to recognize the
  transparent `CPRSMALL.def` image and uniquely named `IX44.def` frame.

## 1.3.2 - 2026-08-14

- Record that the version 1.3.1 executable-only `IX32.def` redirection did not
  change the standard scenario selector.
- Identify the sole `HD_HOTA.dll` `un32.def` resource string and its four
  32x32 image-control constructor references.
- Redirect the HD DLL's shared resource string to `ix32.def`, preserving all
  other heroes because the isolated atlas differs only at Nyx's frame.
- Guard, back up, and verify the original and already-patched DLL states.
- Mark in-game confirmation of the HD DLL redirection as pending.

## 1.3.1 - 2026-08-14

- Identify the standard scenario selector's dynamic `UN32.def` construction
  at executable virtual address `0x0051DCCB`.
- Redirect the sole `un32.def` string in each executable to the isolated
  `ix32.def` name at file offset `0x2817DC`.
- Add `IX32.def` to both sprite archives, `Data`, and both HD compatibility
  packs, and register it in each `Files.ini`.
- Back up and byte-verify every new resource path and decompress all 4,586
  sprite-archive entries after patching.
- Restore finalizer backups exactly from their checksummed manifests,
  including removal of resources that did not exist before finalization.
- Mark in-game confirmation of the standard scenario selector as pending.

## 1.3.0 - 2026-08-14

- Confirm the custom Nyx portrait in the random-map starting-hero selector.
- Add a read-only runtime probe for the HotA portrait tables and document the
  verified `setseed.dll` diagnostic loader path.
- Confirm that hero 140 already resolves through `HPL004EL.PCX` and
  `HPS004EL.PCX`; no runtime-table rewrite is needed.
- Convert the verified LOD-PCX portrait assets to standard indexed BMP files,
  install them in both HotA compatibility packs, and register
  `HPL004EL.bmp` and `HPS004EL.bmp` in each `Files.ini`.
- Continue removing unsafe loose `.pcx` portrait files. HD Mod maps registered
  BMP replacements to HotA's corresponding PCX resource requests.

## 1.2.1 - 2026-08-14

- Mark hero portraits and standard scenario specialty pictures as unresolved.
- Remove loose `HPL004EL.pcx` and `HPS004EL.pcx` overrides from both HD packs.
  Raw LOD-PCX and standard ZSoft PCX variants both reproduced
  `HD_TC2.dll+0x6710` null dereferences.
- Record that patching both sprite LODs, both HD compatibility packs, and
  `HotA_ext.lod` did not change the standard scenario Bloodlust picture.
- Retain the confirmed random-map Advanced Options Pixie specialty control and
  its corrected position.

## 1.2.0 - 2026-08-14

- Rename hero 140 from Inteus to Nyx.
- Add experimental Nyx large and small portraits to both bitmap archives.
- Experiment with the HD popup portrait loader; this was later reverted after
  an access violation.
- Position the Pixie specialty item through direct `xPos` and `yPos`
  adjustments after construction.

## 1.1.0 - 2026-08-14

- Identified the random-map Advanced Options popup as
  `HD.UI.Ext.HeroDlg` in `HD_HOTA.dll`.
- Patch its specialty control to use built-in `CPRSMALL.def` frame 120, the
  Pixie portrait, instead of the unresolved custom-resource path.
- Center the 32x32 Pixie portrait in the original 44x44 specialty layout.
- Back up, restore, validate, and report the `HD_HOTA.dll` state.
- Register `IX44.def` in both sprite archives for non-HD specialty views.
- Document the successful binary offsets and the rejected resource-override
  approaches.

## 1.0.3 - 2026-08-14

- Install the patched `UN32.def` and `UN44.def` as loose resources in the HotA
  HD compatibility pack.
- Register both resources in `_HD3_Data/Compability/#hota/Files.ini`.
- Back up and restore `Files.ini` and any pre-existing loose resources
  exactly.
- Support guarded in-place upgrades from version 1.0.2.

## 1.0.2 - 2026-08-14

- Patch `UN32.def` and `UN44.def` in both `Data/H3sprite.lod` and
  `Data/H3ab_spr.lod`, avoiding dependence on archive-resolution order.
- Support guarded in-place upgrades from the exact version 1.0.1 state.
- Record pre-apply file states so restore returns to either a clean
  installation or version 1.0.1 exactly.

## 1.0.1 - 2026-08-14

- Patch `UN32.def` and `UN44.def` in `Data/H3ab_spr.lod`.
- Add the 32x32 Pixie picture used by random-map starting-hero selection.
- Leave `Data/H3sprite.lod` unchanged.

## 1.0.0 - 2026-08-14

- Initial Inteus Pixie Elementalist patch for HotA 1.8.0.
