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
- Basic Fire Magic to Basic Wisdom, secondary skill ID 7
- Starting spell 43 (Bloodlust) to spell 13 (Fire Wall)
- All three starting creature IDs to Pixie, creature ID 118

The three `H3HeroInfo::StartingCreatures` low/high pairs begin at file offset
`0x27D064`. The executable copies are kept at `22/25`, but HotA reloads the
displayed and instantiated starting amounts from `HOTRAITS.TXT`; changing only
these bytes did not affect the game.

## Text Archive

In `Data/HotA_lng.lod`:

- `HeroSpec.txt`, row `142`, receives the Pixie/Sprite specialty name and text.
- `HeroBios.txt`, row `140`, receives the updated biography.
- `HOTRAITS.TXT`, data row `142` (hero ID 140 plus two headers), changes from
  `15-25 Pixies, 3-5 Air Elementals, 3-5 Water Elementals` to three
  `22-25 Pixies` entries.

The replacements are appended uncompressed and their LOD directory entries
are redirected to the new data. Other archive entries are unchanged.

## Standard Specialty Pictures

`UN32.def` and `UN44.def` are per-hero specialty-picture atlases. Frame 140 is
Inteus's original Bloodlust picture in both. `UN32.def` supplies the 32x32
picture in standard selectors; `UN44.def` supplies the 44x44 picture in
standard larger specialty views.

Both `Data/H3sprite.lod` and `Data/H3ab_spr.lod` contain same-named copies of
these resources. The finalizer patches both copies rather than depending on
runtime archive-resolution order.

The final image generator:

1. Extracts `CPRSMALL.def`, `UN32.def`, and `UN44.def` from each sprite
   archive, plus `DiBoxBck.pcx` from `Data/H3bitmap.lod`.
2. Decodes `CPRSMALL.def` frame 120, internal name `CPrS118E.pcx`. This is the
   exact transparent Pixie sprite used by the random-map popup and measures
   30x32 pixels.
3. Mirrors the source horizontally so the Pixie faces left.
4. Standard specialty panels have a cyan placeholder under the original
   full-square artwork, so true transparency exposes cyan. The generator
   fills each canvas from the game's native 256x256 brown dialog texture
   before drawing the Pixie.
5. Maps the Pixie and dialog texture colors to the nearest target-atlas
   palette index from 8 through 255. Nonzero special Pixie indices are
   retained; transparent source index 0 leaves the brown texture visible.
6. Places the sprite without scaling at `(1,0)` on a 32x32 canvas and at
   `(7,6)` on a 44x44 canvas.
7. Appends each result as an uncompressed frame and redirects only frame 140.
8. Adds isolated `IX32.def` and `IX44.def` copies whose frame 140 internal
   names are `NYX32PIX.PCX` and `NYX44PIX.PCX`.
9. Installs all four atlases in both HD compatibility packs and registers
   them in `Files.ini`.

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

Version 1.5.0 changed the constructor to raw `CPRSMALL.def` frame 120. In-game
testing showed that this path still faced right and exposed the cyan
placeholder:

```text
resource = CPRSMALL.def
frame = 120 (CPrS118E.pcx, Pixie)
mirror = 1
constructor x = 78
constructor y = 18
```

The constructor's mirror field did not alter the displayed orientation on
this HD path. Version 1.6.0 restores the native constructor bytes:

| File offset | Original | Patched | Purpose |
| --- | --- | --- | --- |
| `0x234D83` | `01` | `00` | Remove the ineffective raw-frame mirror flag |
| `0x234D86` | `6A 78 90 90` | `8B 45 08 50` | Use the selected hero ID as the frame |
| `0x234D8B` | `AC 6D 29 01` | `3C 04 2A 01` | Point back to `UN44.def` |
| `0x234D9A` | `6A 12 6A 4E` | `6A 12 6A 48` | Restore the native position |
| `0x234DC3` | compact centering block | native width/height copy block | Restore native 44x44 layout |

The runtime DLL has already replaced loaded `UN44.def` frame 140 with the
unique, left-facing `IX44.def` frame before this dialog opens. The popup
therefore shares the same confirmed image as the scenario hero screen,
including the brown dialog-texture background. The resource text at file
offset `0x29EE3C` remains `UN44.def`.

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

This avoided the original executable resource name and any cached same-name
atlas, but in-game testing still showed Bloodlust. That result proves the
standard scenario selector does not use this executable construction path.

`HD_HOTA.dll` contains its own single `un32.def` string at virtual address
`0x012975F0`, file offset `0x295FF0`. Four code sites pass it to the standard
image constructor:

```text
0x010337F1
0x01033B88
0x01037658
0x010379EC
```

The controls are constructed with initial frames 105 or 106. They are
persistent dialog controls whose frame may later be changed to the selected
hero, which explains why the constructor sites do not directly contain hero
ID 140.

Version 1.3.2 redirects this shared HD DLL string to `ix32.def` as well. The
new atlas is byte-identical to `UN32.def` except for the already-patched Nyx
frame, so the change preserves every other hero. The finalizer guards both
the original and already-patched DLL states and includes `HD_HOTA.dll` in its
timestamped backup. In-game testing still showed Bloodlust in both standard
views.

A read-only runtime trace then hooked image construction at `0x004EA800`.
Neither the standard scenario selector nor the entered scenario's hero screen
called that constructor for the specialty picture. Both paths therefore use
already-loaded DEF objects and change frames directly.

The final fix loads these four resources through the game's own loader at
`0x0055C9C0`:

```text
UN32.def
IX32.def
UN44.def
IX44.def
```

The relevant 32-bit runtime layouts were validated before writing:

```text
H3LoadedDef = 0x38 bytes
H3LoadedDefGroup = 0x0C bytes
H3LoadedDefFrame = 0x48 bytes
```

After a 1.5-second startup delay, the custom DLL changes only frame-table
pointer 140 in loaded `UN32.def` and `UN44.def`. Each target receives the
corresponding frame pointer from `IX32.def` or `IX44.def`. Reads, writes, and
the resulting pointer values are checked with `ReadProcessMemory` and
`WriteProcessMemory`; the DLL records the result in
`_HD3_Data/Common/NyxRuntimeFix.log`.

The first runtime attempt still displayed Bloodlust. Its log revealed that
the separate `UN` and `IX` DEF objects had identical frame-140 pointers.
Although the atlases had different resource names, their copied frame names
were still `Un32Lust.PCX` and `Un44Lust.PCX`. Heroes III cached those frames
by the internal PCX name and reused the already-loaded Bloodlust objects.

Renaming only the isolated frame entries to `NYX32PIX.PCX` and
`NYX44PIX.PCX` produced distinct source pointers. The next runtime log
reported both frame-table swaps as patched, and in-game testing confirmed the
Pixie picture in both the standard scenario hero selector and the entered
scenario's hero screen.

HotA disables general HD Mod plugin-directory scanning with executable flag
`0x8`. The verified optional loader path is
`_HD3_Data/Common/setseed.dll`. The finalizer installs the reviewed custom
runtime DLL there, backs up any previous state, verifies its SHA-256, and
refuses to overwrite an unrelated DLL.

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
- Verification that the sole `HD_HOTA.dll` `un32.def` string and all four of
  its image-constructor references use the isolated resource
- Runtime constructor trace proving that the two standard specialty views
  bypass the expected image-constructor path
- Runtime logs verifying distinct `UN`/`IX` DEF objects, the initial
  same-name frame-cache collision, unique replacement frame names, and both
  successful frame-table swaps
- In-game verification of Pixie pictures in the standard scenario selector
  and the entered scenario's hero screen
- Installed runtime DLL hash verification:
  `be7fb2e8a715b3abaa80eee4d6f24b6e19279c5c1dd9c624f620be396b3dab2d`
- Mirrored Pixie frame verification: 30x32 source at `(1,0)` in the 32x32
  atlas and `(7,6)` in the 44x44 atlas, composited over `DiBoxBck.pcx`
- Verification that both generated frames contain zero cyan-key index-0
  pixels
- Byte verification of Fire Magic/Wisdom, three Pixie creature IDs, and three
  `22/25` executable starting-amount pairs
- Extraction and exact field verification of the authoritative
  `HOTRAITS.TXT` row:
  `Nyx, 22, 25, Pixies, 22, 25, Pixies, 22, 25, Pixies`
- Byte verification that the random-map popup uses native `UN44.def`,
  selected-hero frame, position, mirror, and layout arguments
