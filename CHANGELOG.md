# Changelog

## 1.2.1 - 2026-08-14

- Documented the verified partial support in HotA 1.8.0 with HD Mod 5.6 R16.
- Removed unsafe loose hero-portrait overrides after reproducing an
  `HD_TC2.dll+0x6710` crash with both raw and standard PCX files.
- Restored the original empty `HotA_ext.lod` after confirming that extension
  archive overrides do not affect the scenario specialty dialog.
- Recorded the unresolved runtime-table work instead of claiming that hero
  portraits and scenario specialty pictures are complete.

## 1.2.0 - 2026-08-14

- Renamed hero 140 to Nyx and added experimental large/small portraits.
- Corrected the random-map HD popup specialty-icon positioning.

## 1.1.0 - 2026-08-14

- Fixed the HD Mod random-map hero popup by patching `HD_HOTA.dll` to use
  built-in Pixie portrait frame 120.
- Positioned the 32x32 Pixie portrait inside the original specialty layout.
- Added guarded backup, restore, upgrade, and status handling for the DLL.
- Documented the final working implementation and unsuccessful override
  experiments.

## 1.0.3 - 2026-08-14

- Added HotA-specific HD Mod loose overrides for `UN32.def` and `UN44.def`.
- Registered the overrides in the HotA HD compatibility pack.
- Added exact backup and restore handling for generated override files.

## 1.0.2 - 2026-08-14

- Patched Inteus's 32x32 and 44x44 specialty pictures in both sprite archives.
- Added guarded in-place upgrades from version 1.0.1.
- Preserved the exact pre-apply state in new backup manifests.

## 1.0.1 - 2026-08-14

- Added the 32x32 Pixie picture and targeted the expansion sprite archive.

## 1.0.0 - 2026-08-14

- Reworked Inteus, hero ID 140, into a Pixie/Sprite Elementalist.
- Added the Pixie/Sprite tier-one creature specialty.
- Set Basic Fire Magic and Basic Tactics.
- Set Fire Wall as the starting spell.
- Updated specialty text and biography.
- Replaced Inteus's Bloodlust specialty picture with a Pixie portrait.
- Added checksum guards, timestamped backups, status reporting, and exact
  restoration.
