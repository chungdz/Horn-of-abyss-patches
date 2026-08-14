# Changelog

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
