# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- Hero: Nyx, replacing Inteus
- Hero ID: 140, zero-based
- Existing class slot: Elementalist

Using an existing Elementalist slot is required for Inteus to remain in the
Conflux random-map starting-hero selector.

## Supported Original Hashes

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `b5f2f793af0986050fb41df7209c25d861ae0f837af52bb3bd6864ba4de84f41` |
| `h3hota HD.exe` | `5aaab925f06cccf23bb09814767590a95b84a557eb33d244800520be4f1f18de` |
| `HD_HOTA.dll` | `0ccb8e9eb0a43495c3a9dd09770f51ee41cc2ed79f9730298ac433c8432c4951` |
| `Data/HotA_lng.lod` | `f4ba08f4adfcfb3dcffdc8fa2063307ff2a6caa48212b11073ef43dc73d3047e` |
| `Data/H3sprite.lod` | `57caf2c50573f33a0d91e4222e51d3a73c136d44decf59dde21cacad88fe5d66` |
| `Data/H3ab_spr.lod` | `e0d5003742c8602827ef409966784483dece6eedde76aa2cfeee26cb12d25d67` |

The patcher accepts an original installation and the exact known states
produced by earlier patch versions. It refuses unknown or partially modified
installations.

## Executable Records

Both executables use the same HotA 1.8.0 record offsets.

At file offset `0x279A00`, Inteus's specialty changes from:

```text
type 3, spell 43 (Bloodlust)
```

to:

```text
type 1, creature 118 (Pixie)
```

HotA's built-in tier-one creature specialty logic also applies the bonus to
the upgraded Sprite. No custom combat formula is injected.

At file offset `0x27D020`, the hero record changes:

- Basic Wisdom to Basic Fire Magic, secondary skill ID 14
- Basic Fire Magic to Basic Tactics, secondary skill ID 19
- Starting spell 43 (Bloodlust) to spell 13 (Fire Wall)

The army creature IDs `118`, `112`, and `115` remain unchanged.

## Text Archive

In `Data/HotA_lng.lod`:

- `HeroSpec.txt`, row `142`, receives the Pixie/Sprite specialty name and text.
- `HeroBios.txt`, row `140`, receives the updated biography.

The replacements are appended uncompressed and their LOD directory entries
are redirected to the new data. Other archive entries are unchanged.

## Standard Specialty Pictures

`UN32.def` and `UN44.def` are per-hero specialty-picture atlases. Frame 140 is
Inteus's original Bloodlust picture in both. `UN32.def` supplies the 32x32
picture in standard selectors; `UN44.def` supplies the 44x44 picture in
standard larger specialty views.

Both `Data/H3sprite.lod` and `Data/H3ab_spr.lod` contain same-named copies of
these resources. Version 1.0.2 patches both copies rather than depending on
runtime archive-resolution order.

The patcher:

1. Extracts `TwCrPort.def`, `UN32.def`, and `UN44.def` from each sprite
   archive.
2. Decodes each archive's `TwCrPort.def` frame 120, the legacy-atlas Pixie
   portrait.
3. Takes a centered 58x58 crop and scales it to 32x32 and 44x44 with bilinear
   sampling.
4. Maps each result to its target DEF palette, excluding reserved indices
   0-7.
5. Appends an uncompressed frame to each DEF and redirects only frame 140.
6. Appends both updated DEFs and redirects only their LOD directory entries
   in each archive.
7. Adds an `IX44.def` entry containing the patched 44x44 atlas to both sprite
   LOD directories.
8. Extracts the patched resources to
   `_HD3_Data/Compability/#hota/` and registers them in `Files.ini`.
9. Redirects the normal executable specialty display to the unique
   `IX44.def` name so it cannot reuse a previously cached `UN44.def`.

No other hero's specialty frame is changed.

## HD Advanced Options Popup

The random-map Advanced Options hero popup is not the normal game hero
dialog. It is implemented by `HD.UI.Ext.HeroDlg` in `HD_HOTA.dll`.

At virtual address `0x0123598A`, the original HotA 1.8.0 code constructs the
specialty image with:

```text
resource = UN44.def
frame = hero ID
x = 72
y = 18
```

For Inteus, hero ID 140, that path continued to show the original Physical
Elemental image even after both game executables and both sprite archives
were patched.

Redirecting the DLL string from `UN44.def` to a new `IX44.def` name reached
the correct control but produced an empty slot. The HD dialog's resource
manager did not resolve the new name through loose files, `Files.ini`, or a
new sprite-LOD directory entry. Those approaches are not the final popup
solution.

The working patch changes the constructor arguments to a resource already
registered by the HD module:

```text
resource = CPRSMALL.def
frame = 120 (CPrS118E.pcx, Pixie)
constructor x = 78
constructor y = 18
```

The exact `HD_HOTA.dll` file changes are:

| File offset | Original | Patched | Purpose |
| --- | --- | --- | --- |
| `0x234D86` | `8B 45 08 50` | `6A 78 90 90` | Push fixed Pixie frame 120 instead of hero ID |
| `0x234D8B` | `3C 04 2A 01` | `AC 6D 29 01` | Point to `CPRSMALL.def` at VA `0x01296DAC` |
| `0x234D9A` | `6A 12 6A 48` | `6A 12 6A 4E` | Move the item right 6 |
| `0x234DC3` | original width/height copy block | optimized copy plus `xPos += 6`, `yPos += 4` | Place the Pixie inside the specialty cell |

The resource text at file offset `0x29EE3C` remains `UN44.def`; the working
patch changes the constructor pointer rather than globally renaming that
string.

## Nyx Name And Portrait

`HOTRAITS.TXT` row 140 changes the displayed name from Inteus to Nyx.
`HeroBios.txt` uses the same name. The supplied image is converted to the
Heroes III indexed-PCX format and replaces hero 140's unique resources
`HPL004EL.pcx` (58x64) and `HPS004EL.pcx` (48x32) in both bitmap archives.

HD Mod 5.6 R16 exposes the portrait runtime state through `HdCommon_Get`. A
read-only 32-bit diagnostic DLL queried:

- `HotA.HPL_tbl`
- `HotA.HPS_tbl`
- `HotA.PortraitsCount`
- `HotA.HeroesDefaultPortraits`

HotA disables HD Mod's general plugin-directory scans with executable flag
`0x8`. The probe therefore used HD Mod's optional
`_HD3_Data/Common/setseed.dll` loader slot and was removed after one run.
Entry 140 contained direct pointers to the static names `HPL004el.PCX` and
`HPS004el.PCX`; the default portrait index was 140. No table rewrite was
needed.

HD Mod's registered loose-resource convention maps a standard bitmap such as
`HStInf.bmp` to HotA's `HStInf.pcx` request. The working portrait override
uses the same path:

1. Validate the 12-byte LOD-PCX header, dimensions, pixel count, and
   768-byte palette.
2. Convert the top-down indexed pixels and RGB palette to a standard
   bottom-up 8-bit Windows BMP with four-byte row alignment.
3. Install `HPL004EL.bmp` and `HPS004EL.bmp` in both `#hota` and `#hota15`.
4. Register the BMP names in both `Files.ini` files.
5. Remove loose `HPL004EL.pcx` and `HPS004EL.pcx` files and registrations.

The custom Nyx portrait is confirmed in the random-map starting-hero
selector. Raw LOD-PCX files and standard three-plane ZSoft PCX files remain
unsafe as loose HD resources; both crash at `HD_TC2.dll+0x6710`.

## Standard Scenario Selector

The standard scenario selector continued to display Bloodlust even though
frame 140 differed from the original in all of these locations:

- `Data/H3sprite.lod`
- `Data/H3ab_spr.lod`
- `_HD3_Data/Compability/#hota`
- `_HD3_Data/Compability/#hota15`

Adding matching `UN32.def`, `UN44.def`, and `IX44.def` entries to
`Data/HotA_ext.lod` also had no visible effect and was reverted. These results
showed that replacing the shared `UN32.def` name did not reach the dialog.

Disassembly identified the dynamic scenario-selector image construction in
both executables at virtual address `0x0051DCCB`. It passes:

```text
resource = un32.def
frame = DWORD [hero data + 0x1A]
constructor = 0x004EA800
```

The resource string is the only `un32.def` occurrence in each executable. It
is at virtual address `0x006817DC`, file offset `0x2817DC`.

Version 1.3.1 changes the same-length string to `ix32.def` in `h3hota.exe` and
`h3hota HD.exe`. To cover each resource-resolution path, the finalizer:

1. Adds `IX32.def` to both sprite LOD directories as a byte-identical copy of
   that archive's patched `UN32.def`.
2. Installs the expansion archive's patched copy as `Data/IX32.def`.
3. Installs and registers the same copy in both `#hota` and `#hota15`.
4. Guards both the original and already-patched executable states.
5. Backs up all affected files before writing.

This avoids the original resource name and any cached same-name atlas. The
installation and byte-level validation succeeded; in-game confirmation in
the standard scenario selector is pending.

An earlier experimental rewrite of the HD portrait loader was reverted. It
crashed at `HD_HOTA.dll+0x23516A` while reading `0x9090911C`. The runtime
probe later showed that loader rewriting was unnecessary.

## Validation Performed

- Original-install and guarded earlier-version upgrade checks
- Timestamped backups include `HD_HOTA.dll`, both sprite archives, executable
  files, `Files.ini`, and generated loose resources
- All 4,014 entries in the patched base sprite archive decompressed and
  size-checked
- All 570 entries in the patched expansion sprite archive decompressed and
  size-checked
- All 224 entries in the patched language archive decompressed and size-checked
- Executable differences restricted to Inteus's specialty and hero records
- DEF image differences restricted to frame 140 in `UN32.def` and `UN44.def`
- Pixel verification of all four generated frames against the Pixie source
- Byte verification of `IX44.def` against the patched `UN44.def`
- Screenshot A/B comparison confirming that the first position immediate is
  horizontal and the second is vertical
- Disassembly verification of the direct `xPos`/`yPos` adjustments
- In-game verification of the Pixie specialty and position in the random-map
  Advanced Options popup
- Runtime verification of portrait-table entry 140 and neighboring entries
- Header, dimension, palette, pixel, padding, and byte verification of both
  generated BMP portrait overrides
- In-game verification of the custom Nyx random-map portrait
- All 4,015 base and 571 expansion sprite-LOD entries decompressed and
  size-checked after adding `IX32.def`
- Byte verification that each LOD's `IX32.def` equals its own patched
  `UN32.def`
- Verification that both executable lookups and all loose/HD resource paths
  use the isolated scenario resource
- In-game verification of the standard scenario selector remains pending
