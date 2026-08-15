# Changelog

## 0.2.0 - 2026-08-15

- Increased the Conflux-only Spiritism base rates from 5%/10%/15% to
  10%/20%/30%.
- Added a direct `H3Hero::GetNecromancyPower` hook that chains HotA's complete
  calculation, then adds 5% per Spiritism level.
- Preserved HotA's artifact, Necromancy Amplifier, specialty, rounding, AI,
  and optional 100% cap behavior.
- Kept ordinary Necromancy at HotA's native 5%/10%/15% rates.
- Updated the Basic, Advanced, and Expert Spiritism descriptions to display
  10%, 20%, and 30%.
- Added in-place installer support for the reviewed 0.1.0 runtime.
- Added deterministic rate-test instructions using 120 Peasants.
- Verified the live Basic rate: 28 Sprites at 3 health each raised 2 Pixies,
  matching `floor(28 * 3 * 0.10 / 3)`.
- Documented the native per-creature health cap after an Expert test raised
  16 Pixies from 56 Hobgoblins:
  `floor(56 * min(5, 3) * 0.30 / 3)`.

## 0.1.0 - 2026-08-15

- Added a separate Conflux-wide upgrade over Nyx Spiritism 0.1.5.
- Replaced the first secondary-skill type for Conflux heroes `128–143` with
  internal Necromancy, presented as Spiritism.
- Preserved every first-skill level and every second skill.
- Gave Lacus and Fiur Advanced Spiritism by preserving their original
  Advanced first-skill levels.
- Expanded Pixie raising, Spiritism text, both icon sizes, level-up dialogs,
  standard hero dialogs, and HD pregame panels to all Conflux heroes.
- Kept normal Necromancy behavior unchanged for heroes outside the Conflux.
- Retained Nyx's runtime specialty-picture repair.
- Added strict prerequisite checks, executable output checksums, status
  reporting, timestamped backups, and restoration to the Nyx-only patch.
- Added the dedicated `ConfluxSpiritism.log` runtime report.
- Verified a complete apply, status, and restore round trip against an
  isolated copy of the accepted Nyx Spiritism 0.1.5 installation.
- Verified the live installation in game with all runtime hook categories
  installed and the Conflux-wide behavior working.
