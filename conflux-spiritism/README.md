# Conflux Spiritism

Version 0.3.5 transfer-icon candidate

This HotA 1.8.0 upgrade gives every Conflux hero Spiritism in the first
secondary-skill slot. It builds on the reviewed
[Nyx Spiritism](../nyx-spiritism/README.md) 0.1.6 patch and reuses its custom
icons and loose resources.

Spiritism remains Necromancy internally. Nyx normally raises Sprites while
the other fifteen Conflux heroes raise Pixies at 10%/20%/30% base rates. All
retain HotA's native Amplifiers, artifacts, AI handling, rounding, save
format, and post-battle army logic. With the Cloak of the Undead King they
instead raise Fire Elementals at Basic, Earth Elementals at Advanced, and
Psychic Elementals at Expert. Heroes outside the Conflux retain normal
5%/10%/15% Necromancy and Skeleton raising.

Version 0.2.1 removes obsolete 156-frame specialty-atlas overrides that caused
HotA-added heroes to display unrelated specialty pictures.

Version 0.2.3 disables all runtime writes to `UN32.def` and `UN44.def`. The
earlier frame-pointer replacement crashed in the visiting-hero renderer, and
the later pixel-buffer update changed a renderer buffer shared by Nyx, Akka,
and other heroes.

Version 0.2.4 restores Nyx's Pixie specialty portrait without modifying the
shared HotA atlas. Runtime 6 temporarily redirects each Nyx-specific dialog
to the existing `IX44.def` or `IX32.def` resource, allowing the game to load
and convert it normally. It also installs `SPIR32.def` and scopes that
32x32 Spiritism icon around the HD hero-exchange dialog used for creature and
artifact transfers.

Version 0.2.5 attempted to extend the `IX32.def` resource redirect to the
original scenario selector, but the documented runtime trace had already
proved that view bypasses resource construction. In-game testing still showed
Bloodlust.

Version 0.2.6 removes that ineffective scenario hook and keeps the stable,
scoped image handling from version 0.2.4. The fixed-scenario Bloodlust image
is documented as a cosmetic limitation rather than risking the extended hero
atlas or HotA's converted renderer objects.

Version 0.2.7 chains HotA's native Cloak creature selector for English
Spiritism heroes. Its Walking Dead, Wight, and Lich results are translated to
Fire Elemental, Earth Elemental, and Psychic Elemental respectively. Without
the Cloak, Spiritism continues to raise Pixies.

Version 0.2.8 changes that default English result to Sprite. The three Cloak
results, ordinary Necromancy, and the unresolved Chinese Pixie-only runtime
remain unchanged.

Version 0.2.9 limits that Sprite result to Nyx. Other Conflux Spiritists
return to raising Pixies without the Cloak.

Version 0.3.0 preserves HotA's complete 30-skill image layout. The three
custom Spiritism atlases retain Spiritism at frames `39-41` and borrow the
native Interference frames `87-89` and Runes frames `90-92` after HotA has
loaded them. The aliases remain disabled if that exact layout cannot be
validated.

Version 0.3.1 keeps the exchange aliases active during control refreshes and
right-click event handling, not only initial construction. It also scopes the
Spiritism name and large icon around HotA's Hermit's Shack skill-upgrade
callback.

Version 0.3.5 is intentionally transfer-icon-only. Runtime 18 removes the unsafe
permanent loaded-group extension and does not install a Hermit's Shack hook.
Each Spiritism exchange skill control loads `SPIR32.def`; all other skills,
including Interference and Runes, retain HotA's native `secsk32.def`.
The exchange event/right-click hook is disabled after runtime 17 reproduced
the raw-DEF-versus-converted-renderer crash at `HotA.dll+0x7978A`.

The transfer atlas uses unique internal names `SP32BAS.PCX`,
`SP32ADV.PCX`, and `SP32EXP.PCX`. Reusing the 44x44 names caused HotA's
resource cache to return the already loaded 44x44 frame objects even though
the transfer file itself contained valid 32x32 data.

## Installed State

Installed release state:

- Runtime 18 transfer-icon candidate, SHA-256
  `088b48db3b9d5339059ecb51b4fcdde89f2d7d084d4a9a1a877b6ea9778ca251`
- Spiritism for all sixteen Conflux heroes
- Conflux-only 10%/20%/30% rates
- Sprite raising for Nyx and Pixie raising for other Conflux heroes
- Custom Cloak creatures for every Conflux Spiritist
- `SPIRIT.def`, `SPIR32.def`, and `SPIR82.def` skill icons
- Spiritism names, descriptions, level-up UI, pregame UI, and battle wording
- Nyx's Pixie specialty portrait in standard, pregame, and supported exchange
  dialogs
- Null guards for both HotA hero-inspection handlers

The guarded installer and static machine-code checks pass in the live Pixie
Transformer composition. The 32x32 transfer icon is confirmed in game.
Transfer right-click and Hermit's Shack remain native Necromancy for this
stage.

Removed or disabled:

- Truncated loose `UN32.def` and `UN44.def` compatibility overrides
- Runtime specialty frame-pointer replacement
- Runtime specialty pixel-buffer replacement

`IX32.def` and `IX44.def` remain separate resources. Runtime 18 references
them only while constructing a Nyx-specific dialog; it never copies their
frames or pixels into HotA's loaded `UN32.def` or `UN44.def` objects.

The fixed-scenario selector remains unresolved and still displays Bloodlust.
The original working installation depended on a loose 156-frame `UN32.def`
override, which was removed because it breaks HotA-added hero frames. The
native extended atlas exposes converted renderer objects instead of the raw
frames used by that older repair.

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
contributing health at the raised creature's health, then rounds down. Pixies
and Sprites both have 3 health:

```text
raised tier-one creatures from one stack =
floor(casualties * min(defeated creature health, 3)
      * final rate / 3)
```

For example, 28 Sprites at 3 health each produce 2 creatures with Basic
Spiritism: `floor(28 * 3 * 0.10 / 3) = floor(2.8) = 2`. Nyx raises 2 Sprites;
another Conflux hero raises 2 Pixies. The same battle would produce only 1 at
the former 5% rate.

At Expert Spiritism, 56 Hobgoblins at 5 health each produce 16 Sprites for
Nyx or 16 Pixies for another Conflux hero:
`floor(56 * min(5, 3) * 0.30 / 3) = floor(16.8) = 16`.

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

For a fresh installation, apply and test Nyx Spiritism 0.1.6 first. The
installer also supports in-place upgrades from reviewed Conflux Spiritism
0.1.0 through 0.2.8 installations. It requires the exact:

- Executable checksums
- Runtime DLL checksum
- `SPIRIT.def`, `SPIR82.def`, `IX32.def`, and `IX44.def` resources
- HD compatibility-pack registrations
- Known legacy `UN32.def` and `UN44.def` override checksums, when present

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
3. In a fixed scenario's hero selector, confirm the known limitation:
   Nyx currently displays Bloodlust.
4. Confirm Lacus and Fiur show Advanced Spiritism.
5. Confirm every second skill in the table above is unchanged.
6. Enter a map with a non-Nyx Conflux hero and inspect the hero screen and
   right-click Spiritism detail dialog.
7. Win battles against living creatures and confirm Nyx raises Sprites while
   another Conflux hero raises Pixies, both with a Spiritism result message.
8. Open a non-Conflux Necromancy hero and confirm normal Necromancy text,
   icons, and Skeleton raising remain unchanged.
9. Inspect HotA-added Cove, Factory, and Vori heroes and confirm their
   specialty pictures match their specialties. Vehr must display her Gold
   specialty rather than Intelligence.
10. Open the tavern with Invite a Hero enabled, cycle through the available
   heroes repeatedly, and confirm the dialog remains stable.
11. Put two Conflux heroes, including Nyx, in a town. Swap visiting and
    garrison positions and confirm Spiritism uses its custom 32x32 icon in
    the creature/artifact exchange dialog.
12. Transfer a creature or artifact and confirm the Spiritism icon remains
    after the exchange controls refresh. Right-click Spiritism and confirm
    the native Necromancy popup opens without crashing.
13. Visit a Hermit's Shack and confirm its still-native Necromancy display
    does not affect the transfer icon.
14. Confirm Akka and HotA-added heroes retain their native specialty images.
    Confirm Nyx uses the custom Pixie specialty portrait in her hero and
    supported exchange dialogs.
15. Equip the Cloak of the Undead King and confirm Basic, Advanced, and Expert
    Spiritism raise Fire, Earth, and Psychic Elementals respectively. Remove
    the Cloak and confirm Nyx returns to Sprites while the other heroes return
    to Pixies.
16. Equip the Ring of Oblivion and confirm Spiritism raises nothing, matching
    HotA's native suppression of ordinary Necromancy and all other restoration.
    Move the Ring to the backpack and confirm raising resumes.

For a deterministic rate test, create a battle against exactly 120 Peasants
with no artifacts or Amplifiers. Because each Peasant has 1 health and both
Pixies and Sprites have 3 health, the expected results are:

| Spiritism level | Nyx Sprites | Other Conflux Pixies |
| --- | --- | --- |
| Basic | 4 | 4 |
| Advanced | 8 | 8 |
| Expert | 12 | 12 |

Adding Vampire's Cowl raises the Basic result to 8 Sprites for Nyx or 8
Pixies for another Conflux hero.

With the Cloak of the Undead King, the summoned creature is:

| Spiritism level | Cloak creature |
| --- | --- |
| Basic | Fire Elemental |
| Advanced | Earth Elemental |
| Expert | Psychic Elemental |

Use a new map or newly recruited heroes when checking starting skills.
Existing heroes in saved games retain the skills serialized in those saves.

## Known Limitations

- Spiritism is stored as Necromancy in maps and saves.
- Conflux heroes cannot possess separate Necromancy and Spiritism skills.
- The Ring of Oblivion suppresses Spiritism exactly as it suppresses ordinary
  Necromancy. Because equipment is serialized, closing and reopening the game
  does not remove this effect; move the Ring to the backpack or unequip it.
- Transfer right-click and Hermit's Shack still use the native Necromancy
  name and large icon.
- The exchange dialog keeps Nyx's native specialty image when the other hero
  has ID 156 or later. This prevents the 156-frame `IX32.def` resource from
  replacing HotA's extended specialty atlas.
- Multiplayer requires every player to use identical executables, resources,
  and runtime DLLs.
- The patch is version-locked to HotA 1.8.0, HD Mod 5.6 R16, and the reviewed
  Nyx Spiritism 0.1.6 installation.

Exact offsets, checksums, runtime behavior, and rollback details are in
[TECHNICAL.md](TECHNICAL.md). Release history is in
[CHANGELOG.md](CHANGELOG.md).
