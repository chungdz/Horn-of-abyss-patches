# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- Hero: Inteus
- Hero ID: 140, zero-based
- Existing class slot: Elementalist

Using an existing Elementalist slot is required for Inteus to remain in the
Conflux random-map starting-hero selector.

## Supported Original Hashes

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `b5f2f793af0986050fb41df7209c25d861ae0f837af52bb3bd6864ba4de84f41` |
| `h3hota HD.exe` | `5aaab925f06cccf23bb09814767590a95b84a557eb33d244800520be4f1f18de` |
| `Data/HotA_lng.lod` | `f4ba08f4adfcfb3dcffdc8fa2063307ff2a6caa48212b11073ef43dc73d3047e` |
| `Data/H3sprite.lod` | `57caf2c50573f33a0d91e4222e51d3a73c136d44decf59dde21cacad88fe5d66` |
| `Data/H3ab_spr.lod` | `e0d5003742c8602827ef409966784483dece6eedde76aa2cfeee26cb12d25d67` |

The patcher accepts either an original installation or the exact version
1.0.1 state. It refuses other unknown or partially modified installations.

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

## Specialty Picture

`UN32.def` and `UN44.def` are per-hero specialty-picture atlases. Frame 140 is
Inteus's original Bloodlust picture in both. `UN32.def` supplies the 32x32
picture in the random-map starting-hero selector; `UN44.def` supplies the
44x44 picture in larger specialty views.

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

No other hero's specialty frame is changed.

## Validation Performed

- Fresh-install and version 1.0.1-upgrade apply/status/restore rehearsals
- Exact recovery of the original and pre-upgrade states after restore
- All 4,013 entries in the patched base sprite archive decompressed and
  size-checked
- All 569 entries in the patched expansion sprite archive decompressed and
  size-checked
- All 224 entries in the patched language archive decompressed and size-checked
- Executable differences restricted to Inteus's specialty and hero records
- DEF differences in both archives restricted to frame 140 in `UN32.def` and
  `UN44.def`
- Pixel verification of all four generated frames against the Pixie source

The game was not launched during validation.
