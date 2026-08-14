# Nyx Pixie Elementalist

Version 1.3.2

This patch replaces Inteus, hero ID 140, with Nyx in Horn of the Abyss 1.8.0.
It uses Inteus's existing Elementalist slot, so he remains available in the
Conflux starting-hero selector for random maps.

## Confirmed Result

- Class: Elementalist
- Specialty: Pixies and Sprites
- Specialty effect: Speed +1 and Attack/Defense +10% per hero level, rounded up
- Starting skills: Basic Fire Magic and Basic Tactics
- Starting spell: Fire Wall
- Random-map Advanced Options specialty picture: Pixie
- Random-map starting-hero portrait: custom Nyx portrait
- Hero name: Nyx
- Starting army and class: unchanged

## Known Limitations

The following still requires in-game verification in HotA 1.8.0 with
HD Mod 5.6 R16:

- The standard scenario hero selector now redirects both the game executable
  and HD Mod's dialog controls to an isolated `IX32.def` resource, but the
  HD DLL redirection has not yet been confirmed in-game.
- Standard in-game specialty views may report that the redirected picture is
  missing.

The earlier same-name `UN32.def` overrides and the version 1.3.1
executable-only redirection were ignored by the standard scenario dialog.
Version 1.3.2 also redirects the shared resource used by `HD_HOTA.dll`'s
32x32 dialog controls. See [TECHNICAL.md](TECHNICAL.md) for the exact call
sites and validation.

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

No game files are distributed by this repository. The patcher derives the
Pixie specialty pictures from the user's installed game resources.

The random-map Advanced Options popup is a separate `HD_HOTA.dll`
implementation. Its specialty control is patched directly to use built-in
`CPRSMALL.def` frame 120 and is positioned correctly. HD Mod serves the custom
hero portrait from registered BMP replacements for HotA's HPL/HPS PCX
requests. The standard scenario selector now requests an isolated `IX32.def`
copy of the patched atlas through both the executable and HD DLL paths;
in-game confirmation of the HD DLL change is pending.
