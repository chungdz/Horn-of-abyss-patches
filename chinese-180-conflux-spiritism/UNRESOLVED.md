# Unresolved Chinese Port Status

The Chinese HotA 1.8.0 port is not confirmed working in game. Version 0.1.2
passes file checks and installs its runtime hooks, but the user reported that
the visible hero, specialty, and post-battle localization still did not
change as intended.

Do not describe or distribute this port as complete until an actual Chinese
gameplay test confirms all three visible results.

## Work Completed

- Confirmed the target executables match the supported HotA 1.8.0 baseline.
- Preserved the Chinese `HotA.dll` and HD Mod 5.6 R10 `HD_HOTA.dll`.
- Added an R10 runtime profile with mapped hero-panel, exchange-dialog, and
  hero-inspection addresses.
- Patched the Nyx records in `Data/HotA_lng.lod`.
- Patched the apparent active loose overrides in
  `_HD3_Data/Packs/H3中文-基础资源`:
  - `HOTRAITS.TXT` row 142 to `倪克斯` and three Pixie stacks.
  - `HeroSpec.txt` row 142 to the Pixie specialty.
  - `HeroBios.txt` row 140 to the Nyx biography.
- Localized Spiritism as `唤灵术` in the R10 runtime.
- Updated runtime 9 to alias General Text entries 171 and 172 through both
  `0x006A5DC4` and the adventure/UI object at `0x00696A68`.
- Created checksum-gated installers, installation records, full and
  incremental rollback backups, and a smoke-launch hook log.

## What The Checks Proved

The automated checks prove that the expected files were written, the DLL was
loaded, both General Text objects existed during startup, and all nine hooks
were installed. They do not prove which localization source the Chinese UI
ultimately renders or that the post-battle formatter reads the aliased pointer
at the moment the result dialog is built.

The previous `complete: yes` status was therefore misleading. The installer
now reports mechanical application separately and always marks in-game
validation unresolved.

## Remaining Work

1. Reproduce the issue from a known save or scenario that opens Nyx's hero
   panel and completes one Spiritism battle.
2. Trace actual file opens and pack precedence while that scenario loads.
   The Chinese patch may copy, cache, or replace the loose text through another
   pack or localization plugin after startup.
3. Inspect live memory while the hero panel is visible to identify the exact
   name and specialty string pointers used by the Chinese UI.
4. Instrument or hook the actual post-battle formatting call near
   `0x004A7CF0`, rather than relying only on General Text table aliases.
5. Log each Spiritism creature-selection call and the formatter's final input
   string so call ordering and any later pointer restoration are observable.
6. Repeat the in-game test and only mark the port complete after the visible
   name, specialty, and `唤灵术` result sentence all pass.

## Rollback Records

- Original full installation backup:
  `ChineseConfluxSpiritismPatch/backups/20260815-230926`
- `唤灵术` runtime upgrade:
  `ChineseConfluxSpiritismPatch/backups/20260815-231353`
- Active-pack and dual-table corrective attempt:
  `ChineseConfluxSpiritismPatch/backups/20260815-232251`
