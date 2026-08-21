# Pixie Transformer

Version 0.1.15

This HotA 1.8.0 overlay replaces the Conflux Garden of Life with a Pixie
Transformer:

- Garden building IDs `18` and `19` normally dispatch through a native
  creature-recruitment handler.
- Left-clicking either Garden now replaces that recruitment action with the
  native Skeleton Transformer interface.
- The real Magic Lantern dwelling controls remain unchanged.
- Creatures the native Skeleton Transformer maps to Skeletons become Pixies,
  creature ID `118`.
- Creatures it maps to Bone Dragons become Firebirds, creature ID `130`.
- Stack counts are preserved.
- The Garden's former `+10` weekly Pixie/Sprite growth bonus is removed.
- The Necropolis Skeleton Transformer remains unchanged.
- Conflux Spiritism 0.3.6 remains active. The transfer icon and right-click
  popup are confirmed; Hermit's Shack remains disabled.

The town building keeps its original `Garden of Life` name. The patch changes
only the two Garden descriptions to:

```text
Converts creatures to Pixies or Firebirds.
```

HotA reads this town text from the `BldgSpec.txt` entry inside
`Data/HotA_lng.lod`, so the installer updates that reviewed archive entry in
place and also generates a matching loose `Data/BldgSpec.txt`. The repository
does not contain the extracted game text. The modal conversion window remains
titled `Pixie Transformer`.

## Prerequisite

Install and validate
[Conflux Spiritism 0.2.9 through 0.3.6](../conflux-spiritism/README.md)
first.
The installer requires the exact reviewed hashes for:

- `h3hota.exe`
- `h3hota HD.exe`
- `HotA.dll`
- `_HD3_Data/Common/setseed.dll`
- `Data/HotA_lng.lod`

Unknown or partially modified installations are refused.

## Runtime Layout

HotA exposes one `_HD3_Data/Common/setseed.dll` loader slot. The installer
therefore:

1. Preserves the reviewed Spiritism runtime as
   `_HD3_Data/Common/ConfluxSpiritismRuntime.dll`.
2. Installs `PixieTransformerRuntime.dll` in the `setseed.dll` slot.
3. Loads the preserved Spiritism runtime before applying the Garden hooks.

This keeps the Spiritism hero, creature, rate, Cloak, and transfer UI behavior
intact.

## Usage

Fully exit the game before applying or restoring.

From this directory:

```bash
python3 patch.py status --game-dir "../.."
python3 patch.py apply --game-dir "../.."
```

The installer creates a timestamped rollback under:

```text
<game-dir>/PixieTransformerPatch/backups/
```

Restore the newest backup:

```bash
python3 patch.py restore --game-dir "../.."
```

Restore a specific backup:

```bash
python3 patch.py restore --game-dir "../.." \
  --backup "<game-dir>/PixieTransformerPatch/backups/YYYYMMDD-HHMMSS"
```

## Test Checklist

1. Launch `h3hota HD.exe`.
2. Confirm `_HD3_Data/Common/PixieTransformer.log` ends with
   `final=Pixie Transformer installed`.
3. Confirm `_HD3_Data/Common/ConfluxSpiritism.log` reports runtime 19,
   the exchange resource pair ready, right-click call hooks installed, and
   the Hermit scope disabled.
4. Confirm a real Magic Lantern still opens its normal Pixie/Sprite
   recruitment window.
5. Build either Garden variant and confirm its name remains Garden of Life.
6. Confirm its description explains conversion into Pixies or Firebirds and
   no longer claims to add `+10` weekly growth.
7. Left-click the Garden and confirm the native transformer window opens with
   Pixie/Firebird wording instead of creature recruitment.
8. Transform a creature that the Skeleton Transformer turns into a Skeleton;
   confirm it returns as the same number of Pixies.
9. Transform a creature that the Skeleton Transformer turns into a Bone
   Dragon; confirm it returns as the same number of Firebirds.
10. Repeat both classes with HotA-added creatures where available.
11. Confirm existing Pixies cannot be transformed again.
12. Advance to a new week and confirm the former `+10` Garden growth bonus is
    absent.
13. Open a Necropolis Skeleton Transformer and confirm it still produces
    Skeletons or Bone Dragons normally.
14. Transfer something between two Conflux Spiritists and confirm the
    Spiritism exchange icon persists; right-click it and confirm the
    Spiritism name and large image.
15. Confirm Hermit's Shack remains on its native Necromancy display.
16. Win battles with Nyx and another Conflux Spiritist and confirm the
    Spiritism 0.3.3 Sprite/Pixie split is unchanged.

The Pixie-only transformer dialog and conversion path were validated in game
with runtime 5. Runtime 6 correctly identified the Bone Dragon classes but
produced `NOT USED (2)` because it targeted unused creature ID `124`. Runtime
7 corrects the target to Firebird ID `130`. Both normal-to-Pixie and native
Bone Dragon-class-to-Firebird results are validated in game.

## Installed State

Version 0.1.15 is installed in the live game directory. Static status reports
the overlay runtime, Spiritism companion, reviewed executables, `HotA.dll`,
Pixie/Firebird building text, and patched language archive complete. The
pre-upgrade 0.1.9 plus Spiritism 0.3.1 state is saved under:

```text
PixieTransformerPatch/backups/20260820-162535
```

Version 0.1.0 failed its first in-game test: clicking the visible Garden
opened Magic Lantern recruitment, and its log showed that the transformer
table lookup and Garden click hook had failed. Version 0.1.1 moves the hook to
the actual Magic Lantern dwelling path and defers the HotA target-table
lookup, but its town-type check read the wrong manager offset and every click
still fell through to recruitment. Version 0.1.2 corrected the town-type read,
but its in-game log proved the Garden click never reached the separate Magic
Lantern dwelling branch. Version 0.1.3 hooks the executable's actual Garden
dispatch at `0x005D38E1`, but its click log exposed an incorrect HotA
target-table global. Version 0.1.4 reads the pointer from the initializer's
verified storage location at `HotA.dll+0x6354E4`; its runtime log confirms the
Garden dialog opens. Version 0.1.5 retains that runtime and moves the
description-only override into the language archive HotA actually loads.
Version 0.1.6 preserves the native Skeleton/Bone Dragon classification,
but mistakenly maps the latter to unused ID `124`. Version 0.1.7 retains the
classification and corrects the Firebird target to ID `130`.

## Files

- `patch.py`: guarded status, apply, backup, and restore tool
- `runtime-hook/runtime-pixie-transformer.cpp`: town and growth runtime hooks
- `runtime-hook/build.sh`: reproducible 32-bit Windows DLL build
- `assets/PixieTransformerRuntime.dll`: packaged runtime
- `CHANGELOG.md`: release history
- `TECHNICAL.md`: implementation record
