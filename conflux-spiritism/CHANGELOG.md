# Changelog

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
