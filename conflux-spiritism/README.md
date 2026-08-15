# Conflux Spiritism

Version 0.2.0

This HotA 1.8.0 upgrade gives every Conflux hero Spiritism in the first
secondary-skill slot. It builds on the tested
[Nyx Spiritism](../nyx-spiritism/README.md) 0.1.5 patch and reuses its custom
icons and loose resources.

Spiritism remains Necromancy internally. Conflux heroes raise Pixies at
10%/20%/30% base rates while retaining HotA's native Amplifiers, artifacts,
AI handling, rounding, save format, and post-battle army logic. Heroes
outside the Conflux retain normal 5%/10%/15% Necromancy and Skeleton raising.

## Spiritism Rates

| Level | Base rate |
| --- | --- |
| Basic | 10% |
| Advanced | 20% |
| Expert | 30% |

The runtime first asks HotA for its complete Necromancy percentage, including
the normal skill rate, artifacts, Amplifiers, specialties, and other native
modifiers. It then adds another `5%` per Spiritism level for Conflux heroes.
When HotA requests a capped result, the final percentage remains capped at
100%.

This means artifact and building bonuses remain additive. For example, Basic
Spiritism plus Vampire's Cowl uses 20%: the 10% Spiritism base plus the Cowl's
normal 10% bonus.

For each defeated stack, the native army calculation caps a creature's
contributing health at the raised creature's health, then rounds down:

```text
raised Pixies from one stack =
floor(casualties * min(defeated creature health, Pixie health)
      * final rate / Pixie health)
```

For example, 28 Sprites at 3 health each produce 2 Pixies with Basic
Spiritism: `floor(28 * 3 * 0.10 / 3) = floor(2.8) = 2`. The same battle would
produce only 1 Pixie at the former 5% rate.

At Expert Spiritism, 56 Hobgoblins at 5 health each produce 16 Pixies:
`floor(56 * min(5, 3) * 0.30 / 3) = floor(16.8) = 16`. Their health is capped
at the Pixie's 3 health for this calculation.

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

For a fresh installation, apply and test Nyx Spiritism 0.1.5 first. The
installer also supports an in-place upgrade from reviewed Conflux Spiritism
0.1.0. It requires the exact:

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
   `_HD3_Data/Common/ConfluxSpiritism.log` reports the creature and rate hooks
   installed.
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

For a deterministic rate test, create a battle against exactly 120 Peasants
with no artifacts or Amplifiers. Because each Peasant has 1 health and each
Pixie has 3 health, the expected results are:

| Spiritism level | Raised Pixies |
| --- | --- |
| Basic | 4 |
| Advanced | 8 |
| Expert | 12 |

Adding Vampire's Cowl raises the Basic result to 8 Pixies.

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
