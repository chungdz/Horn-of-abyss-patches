# Nyx Pixie Elementalist

Version 1.6.0

This patch replaces Inteus, hero ID 140, with Nyx in Horn of the Abyss 1.8.0.
It uses Inteus's existing Elementalist slot, so he remains available in the
Conflux starting-hero selector for random maps.

## Patch Result

- Class: Elementalist
- Specialty: Pixies and Sprites
- Specialty effect: Speed +1 and Attack/Defense +10% per hero level, rounded up
- Starting skills: Basic Fire Magic and Basic Wisdom
- Starting spell: Fire Wall
- Starting army: three stacks of 22-25 Pixies
- Random-map Advanced Options specialty picture: Pixie
- Random-map starting-hero portrait: custom Nyx portrait
- Standard scenario hero-selection specialty picture: Pixie
- Standard in-game hero specialty picture: Pixie
- Hero name: Nyx

## Known Limitations

The standard scenario and in-game specialty-picture fix uses HD Mod's optional
`_HD3_Data/Common/setseed.dll` loader slot. The finalizer refuses to overwrite
that path if another DLL already occupies it. Combining this patch with
another mod that needs the same loader slot requires merging the DLL behavior.

Standard specialty panels place a cyan placeholder underneath their original
full-square artwork. A transparent Pixie therefore reveals cyan rather than
the dialog beneath it. The patch mirrors the Pixie to face left and composites
it over the game's native brown dialog texture. The generated frames contain
no cyan-key pixels. Version 1.6.0's random-map image and corrected starting
army still need an in-game check.

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
node finalize-nyx.js "C:\Games\HoMM 3 Complete"
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

Start a new random map after applying the patch and select Nyx from the
Conflux starting heroes. Fully exit the game before applying or restoring;
reopening only the random-map dialog does not reload the sprite archive.

## Scope

The patch modifies:

- `h3hota.exe`
- `h3hota HD.exe`
- `HD_HOTA.dll`
- `Data/HotA_lng.lod`
- `Data/H3sprite.lod`
- `Data/H3ab_spr.lod`
- `Data/H3bitmap.lod`
- `Data/H3ab_bmp.lod`
- `Data/IX32.def`
- `Data/IX44.def`
- `_HD3_Data/Compability/#hota/Files.ini`
- `_HD3_Data/Compability/#hota/UN32.def`
- `_HD3_Data/Compability/#hota/UN44.def`
- `_HD3_Data/Compability/#hota/IX32.def`
- `_HD3_Data/Compability/#hota/IX44.def`
- `_HD3_Data/Compability/#hota/HPL004EL.bmp`
- `_HD3_Data/Compability/#hota/HPS004EL.bmp`
- `_HD3_Data/Compability/#hota15/Files.ini`
- `_HD3_Data/Compability/#hota15/UN32.def`
- `_HD3_Data/Compability/#hota15/UN44.def`
- `_HD3_Data/Compability/#hota15/IX32.def`
- `_HD3_Data/Compability/#hota15/IX44.def`
- `_HD3_Data/Compability/#hota15/HPL004EL.bmp`
- `_HD3_Data/Compability/#hota15/HPS004EL.bmp`
- `_HD3_Data/Common/setseed.dll`

No original game files are distributed by this repository. The patcher
derives the Pixie specialty pictures from the user's installed resources.
The included `NyxRuntimeFix.dll` is custom code built from the source under
`runtime-hook/`.

Standard scenario and in-game views cache the original `UN32.def` and
`UN44.def` frames. At startup the custom runtime fix replaces only frame 140
in those loaded atlases with uniquely named frames from `IX32.def` and
`IX44.def`. The random-map Advanced Options popup now uses that same loaded
`UN44.def` frame instead of drawing a separate raw creature portrait. This
keeps all specialty views left-facing and removes the cyan background. HD Mod
serves the custom hero portrait from registered BMP replacements for HotA's
HPL/HPS PCX requests.
