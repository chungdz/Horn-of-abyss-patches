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

The patcher refuses unknown or partially modified installations.

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

`UN44.def` is a per-hero specialty-picture atlas. Frame 140 is
`Un44Lust.PCX`, Inteus's original Bloodlust picture.

The patcher:

1. Extracts `TwCrPort.def` and `UN44.def` from `Data/H3sprite.lod`.
2. Decodes `TwCrPort.def` frame 120, the legacy-atlas Pixie portrait.
3. Takes a centered 58x58 crop and scales it to 44x44 with bilinear sampling.
4. Maps the result to the `UN44.def` palette, excluding reserved indices 0-7.
5. Appends an uncompressed DEF frame and redirects only frame 140 to it.
6. Appends the updated `UN44.def` and redirects only its LOD directory entry.

No other hero's specialty frame is changed.

## Validation Performed

- Full apply/status/restore rehearsal on copies of all four supported files
- Exact SHA-256 recovery after restore
- All 4,013 entries in the patched sprite archive decompressed and size-checked
- All 224 entries in the patched language archive decompressed and size-checked
- Executable differences restricted to Inteus's specialty and hero records
- Visual verification of the generated 44x44 Pixie frame

The game was not launched during validation.

