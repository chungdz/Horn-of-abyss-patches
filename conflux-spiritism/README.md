# Conflux Spiritism

Version 0.1.0

This HotA 1.8.0 upgrade gives every Conflux hero Spiritism in the first
secondary-skill slot. It builds on the tested
[Nyx Spiritism](../nyx-spiritism/README.md) 0.1.5 patch and reuses its custom
icons and loose resources.

Spiritism remains Necromancy internally. Conflux heroes raise Pixies while
using HotA's native Necromancy percentages, Amplifiers, artifacts, AI
handling, rounding, save format, and post-battle army logic. Heroes outside
the Conflux retain normal Necromancy and Skeleton raising.

## Hero Changes

The replaced skill's level is preserved. Lacus and Fiur therefore receive
Advanced Spiritism; all other Conflux heroes receive Basic Spiritism.

| ID | Hero | Replaced first skill | New first skill | Preserved second skill |
| --- | --- | --- | --- | --- |
| 128 | Pasis | Basic Offense | Basic Spiritism | Basic Artillery |
| 129 | Thunar | Basic Tactics | Basic Spiritism | Basic Estates |
| 130 | Ignissa | Basic Offense | Basic Spiritism | Basic Artillery |
| 131 | Lacus | Advanced Tactics | Advanced Spiritism | None |
| 132 | Monere | Basic Offense | Basic Spiritism | Basic Logistics |
| 133 | Erdamon | Basic Tactics | Basic Spiritism | Basic Estates |
| 134 | Fiur | Advanced Offense | Advanced Spiritism | None |
| 135 | Kalt | Basic Tactics | Basic Spiritism | Basic Learning |
| 136 | Luna | Basic Wisdom | Basic Spiritism | Basic Fire Magic |
| 137 | Brissa | Basic Wisdom | Basic Spiritism | Basic Air Magic |
| 138 | Ciele | Basic Wisdom | Basic Spiritism | Basic Water Magic |
| 139 | Labetha | Basic Wisdom | Basic Spiritism | Basic Earth Magic |
| 140 | Nyx | Basic Spiritism | Basic Spiritism | Basic Fire Magic |
| 141 | Aenain | Basic Wisdom | Basic Spiritism | Basic Air Magic |
| 142 | Gelare | Basic Wisdom | Basic Spiritism | Basic Water Magic |
| 143 | Grindan | Basic Wisdom | Basic Spiritism | Basic Earth Magic |

## Prerequisite

Apply and test Nyx Spiritism 0.1.5 first. This installer requires its exact:

- Executable checksums
- Runtime DLL checksum
- `SPIRIT.def` and `SPIR82.def` resources
- HD compatibility-pack registrations

Unknown or partially modified installations are refused.

## Usage

Fully exit the game before applying or restoring.

From this directory:

```bash
python3 patch.py status --game-dir "../.."
python3 patch.py apply --game-dir "../.."
```

The installer creates a timestamped rollback under:

```text
<game-dir>/ConfluxSpiritismPatch/backups/
```

Restore the newest backup, returning to the Nyx-only Spiritism installation:

```bash
python3 patch.py restore --game-dir "../.."
```

Restore a specific backup:

```bash
python3 patch.py restore --game-dir "../.." \
  --backup "<game-dir>/ConfluxSpiritismPatch/backups/YYYYMMDD-HHMMSS"
```

## Test Checklist

1. Start `h3hota HD.exe` and confirm
   `_HD3_Data/Common/ConfluxSpiritism.log` reports all hooks installed.
2. In the random-map hero selector, inspect several Conflux heroes and confirm
   their first skill is Spiritism with the custom icon.
3. Confirm Lacus and Fiur show Advanced Spiritism.
4. Confirm every second skill in the table above is unchanged.
5. Enter a map with a non-Nyx Conflux hero and inspect the hero screen and
   right-click Spiritism detail dialog.
6. Win a battle against living creatures and confirm Pixies are raised with a
   Spiritism result message.
7. Open a non-Conflux Necromancy hero and confirm normal Necromancy text,
   icons, and Skeleton raising remain unchanged.

Use a new map or newly recruited heroes when checking starting skills.
Existing heroes in saved games retain the skills serialized in those saves.

## Known Limitations

- Spiritism is stored as Necromancy in maps and saves.
- Conflux heroes cannot possess separate Necromancy and Spiritism skills.
- Less common skill notifications outside the scoped hero, level-up, and
  pregame paths can still use the word `Necromancy`.
- Multiplayer requires every player to use identical executables, resources,
  and runtime DLLs.
- The patch is version-locked to HotA 1.8.0, HD Mod 5.6 R16, and the tested
  Nyx Spiritism 0.1.5 installation.

Exact offsets, checksums, runtime behavior, and rollback details are in
[TECHNICAL.md](TECHNICAL.md). Release history is in
[CHANGELOG.md](CHANGELOG.md).
