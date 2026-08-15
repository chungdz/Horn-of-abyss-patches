# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- Existing prerequisite patch: Nyx Pixie Elementalist 1.7.0 installation
- Test hero: Nyx
- Hero ID: `140`
- Raised creature: Pixie
- Creature ID: `118`
- Internal secondary skill: Necromancy
- Secondary skill ID: `12`

## Design

Spiritism is an alias over Necromancy rather than a new secondary-skill ID.
This keeps HotA's native implementation responsible for:

- Basic, Advanced, and Expert power
- Necromancy Amplifiers
- Necromancy artifact bonuses
- Hero specialty bonuses, if later assigned
- Defeated-stack eligibility
- Rounding and minimum results
- Adding the raised stack to the army
- AI combat results
- Save and load

Only the returned creature and Nyx-facing presentation are changed.

## Executable Record

Nyx's `H3HeroInfo` record begins at file offset `0x27D020` in both:

- `h3hota.exe`
- `h3hota HD.exe`

The first secondary-skill type is at record offset `0x0C`, absolute file
offset `0x27D02C`.

```text
Before: 07 00 00 00  Basic Wisdom
After:  0C 00 00 00  Basic Necromancy, presented as Spiritism
```

The first skill's level remains `1` for Basic. The second skill remains
Basic Fire Magic, skill ID `14`.

## Runtime Hooks

The existing Nyx patch already occupies HD Mod's
`_HD3_Data/Common/setseed.dll` loader slot. The new
`NyxSpiritismRuntime.dll` therefore merges the original Nyx specialty-frame
fix with Spiritism.

| Address | HotA function | Change |
| --- | --- | --- |
| `0x004E3ED0` | `H3Hero::GetNecromancyCreatureId` | Return Pixie `118` when the hero is Nyx and has Necromancy |
| `0x004E1A70` | Standard hero dialog | Temporarily alias Necromancy text and frames for hero ID `140` |
| `0x004DA990` | Hero level-up processing | Temporarily alias Necromancy text and frames while Nyx levels |

Hooks are installed through the existing `patcher_x86.dll` interface. Each
target prologue is checked before installation. Unsupported bytes cause the
runtime to log a failure rather than patch an unknown executable.

The creature hook is reached by both reviewed combat-result paths:

- Manual combat calls it at `0x00476EF4`.
- AI/quick combat calls it at `0x00426E29`.

The native power function at `0x004E3F40` is deliberately not hooked. It
reads Nyx's Necromancy level and applies HotA's existing artifact, town, and
other percentage modifiers.

The Cloak of the Undead King normally changes the raised creature according
to Necromancy level. Nyx's creature-selection hook takes precedence and
always returns Pixie while leaving the Cloak's percentage contribution in the
native power function.

## UI Alias

Virtual address `0x0067DCF0` stores a pointer to the secondary-skill text
table. The runtime reads that pointer, selects Necromancy entry `12`, and
temporarily replaces its four pointers during a Nyx UI scope with:

- Name: `Spiritism`
- Basic description: 5% and Pixies
- Advanced description: 10% and Pixies
- Expert description: 15% and Pixies

The original pointers are restored immediately after the dialog closes.
Normal Necromancy heroes therefore retain their original text.

`Secskill.def` uses three frames per skill. Necromancy's Basic, Advanced, and
Expert pictures are frames `39`, `40`, and `41`. The runtime temporarily
points those loaded frame-table entries to the corresponding uniquely named
frames in `SPIRIT.def`, then restores the original pointers.

Nested UI calls are protected by a depth counter so a dialog opened during
level-up does not restore the alias prematurely.

## Icon Generation

The installer:

1. Extracts `Secskill.def` from the installed `Data/H3sprite.lod`.
2. Requires SHA-256
   `298f31e75e045fcb1195d870efbed8d7f5ecb81bab18e0ffc89ccc6a81c91aee`.
3. Decodes the three supplied non-interlaced 8-bit RGBA PNG files.
4. Area-downsamples each image from 1254x1254 to 44x44.
5. Converts visible pixels to the nearest `Secskill.def` palette index from
   `8` through `255`; palette index `0` remains transparent.
6. Appends uncompressed replacement frames with unique internal names:
   `SPIRBAS.PCX`, `SPIRADV.PCX`, and `SPIREXP.PCX`.
7. Writes the generated `SPIRIT.def` as a loose resource and registers it in
   both HotA HD compatibility packs.

Generated resource SHA-256 for the supplied images:

```text
20ccb9bf8067b5483dda26ed77df6388f291ce3828d06a4cd4abb3dbe98a6e32
```

## Runtime Build

Source:

```text
runtime-hook/runtime-spiritism.cpp
```

Build:

```bash
runtime-hook/build.sh
```

The build uses Zig 0.15.2 as an x86 Windows GNU C++ cross-compiler and imports
only `KERNEL32.dll`. The reviewed compiled asset is:

```text
assets/NyxSpiritismRuntime.dll
SHA-256 54230707e8c864e1007deb6632c15238c4dadbfc78c9660432aa45262b361e46
```

At runtime it writes:

```text
_HD3_Data/Common/NyxSpiritism.log
```

## Files Changed By Apply

- `h3hota.exe`
- `h3hota HD.exe`
- `Data/SPIRIT.def`
- `_HD3_Data/Compability/#hota/SPIRIT.def`
- `_HD3_Data/Compability/#hota15/SPIRIT.def`
- `_HD3_Data/Compability/#hota/Files.ini`
- `_HD3_Data/Compability/#hota15/Files.ini`
- `_HD3_Data/Common/setseed.dll`

The installer backs up those paths, plus the runtime log path, under
`NyxSpiritismPatch/backups/` before writing.

The installed Spiritism executable hashes are:

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `0e710b6de65ec869b333a18a858c9f2c1fe3e1d3eb2ba2a076db159720d66b61` |
| `h3hota HD.exe` | `c0f7d8801e2e475260ac573b1a2457cfd4e8bde8ad892bc952f4cb95930d24b5` |

## Source Icon Hashes

| Asset | SHA-256 |
| --- | --- |
| `basic-spiritism.png` | `e54b3cdad077bc6c7ed5bf1b7ac4b7cbdd65b85cc281bfada35825d05b011672` |
| `advanced-spiritism.png` | `26bf6e76d761b78f35a0249161c8308374762be90bd480467055599e4dc6b596` |
| `expert-spiritism.png` | `23af594cfb1d9e9488ca47cec9961ad9f339145c0ccaac7178d33a0800308eaf` |
