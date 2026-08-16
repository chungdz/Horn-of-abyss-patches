# Changelog

## Unreleased - 2026-08-16

- Recorded the user's report that version 0.1.2 still does not change the
  visible Chinese hero, specialty, and post-battle text as intended.
- Added `UNRESOLVED.md` with completed work, limits of the automated checks,
  rollback locations, and the remaining runtime investigation.
- Changed status output so mechanical installation can no longer be reported
  as a completed, in-game-verified port.

## 0.1.2 - 2026-08-16

- Patched the active `H3中文-基础资源` loose hero files that override
  `HotA_lng.lod`, attempting to replace the visible `恩托斯` name and
  `嗜血奇术` specialty.
- Updated runtime 9 to scope the Spiritism result text in both HotA General
  Text objects, including the adventure/UI table used after battle.
- Kept the native `招魂术` result text for ordinary Necromancy heroes.
- Added a checksum-gated corrective upgrade, incremental backup, active-pack
  status checks, and installation-record notes.

## 0.1.1 - 2026-08-16

- Renamed the Chinese Spiritism localization from `通灵术` to `唤灵术`.
- Added a checksum-gated runtime-only upgrade with a small rollback backup.
- Clears the previous runtime log so the new DLL receives a fresh launch check.

## 0.1.0 - 2026-08-16

- Added a reversible installer for the Chinese HotA 1.8.0 package.
- Preserved localized archives by replacing only the required LOD entries.
- Added GBK Nyx, specialty, Spiritism description, and battle-result text.
- Mapped HD Mod 5.6 R10 hero-panel, exchange-dialog, and inspection-hook RVAs.
- Preserved and checksum-gated the Chinese `HotA.dll` and `HD_HOTA.dll`.
- Added installation records, full file backups, status checks, and restore.
