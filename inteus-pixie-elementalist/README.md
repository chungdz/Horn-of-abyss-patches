# Inteus Pixie Elementalist

Version 1.0.3

This patch reworks Inteus, hero ID 140, in Horn of the Abyss 1.8.0.
It uses Inteus's existing Elementalist slot, so he remains available in the
Conflux starting-hero selector for random maps.

## Result

- Class: Elementalist
- Specialty: Pixies and Sprites
- Specialty effect: Speed +1 and Attack/Defense +10% per hero level, rounded up
- Starting skills: Basic Fire Magic and Basic Tactics
- Starting spell: Fire Wall
- Specialty picture: Pixie
- Starting army, name, class, and hero portrait: unchanged

## Requirements

- Horn of the Abyss 1.8.0
- The exact supported English game files listed in
  [TECHNICAL.md](TECHNICAL.md)
- Node.js 18 or newer
- Approximately 100 MB of free space for each backup

Use a separate HotA installation. All players in a multiplayer game must use
identical game files.

## Usage

From this directory:

```bash
node patch.js status --game-dir "C:\Games\HoMM 3 Complete"
node patch.js apply --game-dir "C:\Games\HoMM 3 Complete"
```

The patcher verifies the HotA version, original SHA-256 hashes, executable
records, text rows, and resource structures before writing. `apply` creates a
timestamped backup under:

```text
<game-dir>\ConfluxElementalistPatch\backups\
```

Restore the newest complete backup with:

```bash
node patch.js restore --game-dir "C:\Games\HoMM 3 Complete"
```

Select a specific backup when needed:

```bash
node patch.js restore --game-dir "C:\Games\HoMM 3 Complete" --backup "C:\Games\HoMM 3 Complete\ConfluxElementalistPatch\backups\YYYYMMDD-HHMMSS"
```

Start a new random map after applying the patch and select Inteus from the
Conflux starting heroes. Fully exit the game before applying or restoring;
reopening only the random-map dialog does not reload the sprite archive.

## Scope

The patch modifies:

- `h3hota.exe`
- `h3hota HD.exe`
- `Data/HotA_lng.lod`
- `Data/H3sprite.lod`
- `Data/H3ab_spr.lod`
- `_HD3_Data/Compability/#hota/Files.ini`
- `_HD3_Data/Compability/#hota/UN32.def`
- `_HD3_Data/Compability/#hota/UN44.def`

No game files are distributed by this repository. The patcher derives the
Pixie specialty pictures from the user's installed game resources. Both
sprite archives are patched so the result does not depend on which copy the
game resolves. The two generated loose DEF files give the HD Mod an explicit
HotA-specific override.
