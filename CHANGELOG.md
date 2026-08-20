# Changelog

## Pixie Transformer 0.1.5 - 2026-08-20

- Kept the town building name `Garden of Life`.
- Changed only the two Garden descriptions to explain Pixie conversion.
- Moved the description override into the active `BldgSpec.txt` entry inside
  `Data/HotA_lng.lod` after confirming HotA ignored the loose file for this
  town text.
- Preserved runtime 5 and all validated 0.1.4 gameplay behavior.
- Added guarded 0.1.4 upgrade and byte-exact language-archive restoration.

## Pixie Transformer 0.1.4 - 2026-08-20

- Used runtime 4's per-click failures to isolate the remaining problem to the
  HotA expanded transformer mapping.
- Corrected its pointer from unrelated global `HotA.dll+0x6354CC` to the
  initializer's verified final store at `HotA.dll+0x6354E4`.
- Added runtime 5 and a guarded upgrade from version 0.1.3.

## Pixie Transformer 0.1.3 - 2026-08-20

- Used the runtime 3 log and executable jump tables to confirm that Garden
  IDs `18/19` dispatch through `0x005D38E1`, not the Magic Lantern dwelling
  branch.
- Moved the direct transformer hook to the real Garden handler.
- Left actual Magic Lantern and other dwelling recruitment unchanged.
- Added runtime 4 and a guarded upgrade from version 0.1.2.

## Pixie Transformer 0.1.2 - 2026-08-20

- Corrected runtime 2's invalid town-manager type check by reading the actual
  `H3Town` type byte from `[manager+0x38]+0x04`.
- Changed a built Garden to consume the overlapping Magic Lantern left-click,
  disabling creature recruitment and routing directly to the transformer.
- Preserved recruitment only in towns without the Garden.
- Added runtime 3 diagnostics and guarded upgrades from versions 0.1.0 and
  0.1.1.

## Pixie Transformer 0.1.1 - 2026-08-20

- Recorded that version 0.1.0 failed in game because the Garden artwork uses
  the Magic Lantern dwelling control and the HotA transformer table was not
  available during loader initialization.
- Moved the hook to the Magic Lantern dwelling handler and redirected it only
  when the current Conflux town has Garden building bit `18` or `19`.
- Preserved native Magic Lantern recruitment when the Garden is absent or a
  transformer prerequisite is unavailable.
- Deferred HotA transformer-table resolution until the eligible Garden click.
- Added guarded 0.1.0 upgrade and restore support.

## Pixie Transformer 0.1.0 - 2026-08-20

- Added a separate patch that replaces Conflux Garden of Life clicks with the
  native transformer interface.
- Converts selected stacks 1:1 into Pixies while preserving the Necropolis
  Skeleton Transformer.
- Removes the Garden's old `+10` weekly growth bonus.
- Preserves Conflux Spiritism 0.2.9 through a companion-runtime loader.
- Added guarded install, generated building text, status, backup, restore,
  source, and technical records.

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
