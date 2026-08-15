# Nyx Spiritism

Version 0.1.5

This HotA 1.8.0 patch gives Nyx a Nyx-only secondary skill named Spiritism.
It is implemented as Necromancy internally, preserving the game's existing
save format, percentage calculation, artifact bonuses, town bonuses, AI
handling, and post-battle army logic.

The
[Nyx Pixie Elementalist](../inteus-pixie-elementalist/README.md)
patch must already be applied.

## Patch Result

- Starting skills: Basic Spiritism, then Basic Fire Magic
- Internal skill ID: Necromancy, secondary skill ID `12`
- Basic ratio: 5%
- Advanced ratio: 10%
- Expert ratio: 15%
- Raised creature for Nyx: Pixie, creature ID `118`
- Necromancy Amplifiers: handled by HotA's native calculation
- Amulet of the Undertaker: handled by HotA's native calculation
- Vampire's Cowl: handled by HotA's native calculation
- Dead Man's Boots: handled by HotA's native calculation
- Cloak of the Undead King: percentage bonus retained; Nyx still raises Pixies

The ratio is the percentage of defeated-creature health used by Necromancy.
The game then divides that value by the raised creature's health. Pixies have
less health than Skeletons, so the same percentage can produce more individual
Pixies than Skeletons. This is the native Necromancy formula using Pixies as
the result creature.

## Supplied Icons

The three original RGBA images are preserved under `assets/`:

- `basic-spiritism.png`: one spirit
- `advanced-spiritism.png`: two spirits
- `expert-spiritism.png`: three spirits

During installation, `patch.py` builds two palette-matched resources:

- `SPIRIT.def`, with 44x44 icons for hero and pregame panels
- `SPIR82.def`, with 82x93 icons for the right-click skill detail dialog

Both use the game's brown dialog texture behind the supplied transparent art.
No original Heroes III resource is stored in this repository.

## Usage

Fully exit the game before applying or restoring.

From this directory:

```bash
python3 patch.py status --game-dir "../.."
python3 patch.py apply --game-dir "../.."
```

Running `apply` on a reviewed earlier Spiritism installation upgrades the
generated resources and runtime DLL in place after creating another backup.

The patcher creates a timestamped backup under:

```text
<game-dir>/NyxSpiritismPatch/backups/
```

Restore the newest backup with:

```bash
python3 patch.py restore --game-dir "../.."
```

Restore a specific backup with:

```bash
python3 patch.py restore --game-dir "../.." \
  --backup "<game-dir>/NyxSpiritismPatch/backups/YYYYMMDD-HHMMSS"
```

## Test Checklist

1. Start a new map with Nyx.
2. Confirm her skills are Basic Spiritism and Basic Fire Magic.
3. Open Nyx's hero screen and confirm the one-spirit icon has a brown
   background and the label reads Spiritism.
4. In the pregame hero-selection panel, confirm Nyx shows Basic Spiritism and
   the one-spirit icon.
5. Right-click Spiritism and confirm the large custom icon and description
   appear.
6. Start and finish a battle against living creatures without a crash.
7. Confirm Pixies are raised and the result message begins with
   `Practicing the art of Spiritism`.
8. Equip Vampire's Cowl and confirm the raised amount increases.
9. Own a Necromancy Amplifier and confirm its bonus increases the amount.
10. Advance Spiritism and confirm the two-spirit and three-spirit icons appear.
11. Open a normal Necromancy hero and confirm Necromancy still has its original
   name, description, icons, and raised creature.

## Known Limitations

- Spiritism is stored as Necromancy in saves and maps.
- Nyx cannot possess separate Necromancy and Spiritism skills.
- Nyx's hero and level-up dialogs temporarily use the Spiritism text table and
  custom resource names. The pregame panel uses the same scoped alias while
  constructing Nyx's controls. Every alias is restored before combat or
  another hero screen can use it.
- A less common notification generated outside the hero and level-up paths,
  such as a map object teaching the skill, can still use the word
  `Necromancy`.
- The patch is version-locked to the reviewed HotA 1.8.0 executables and the
  currently installed Nyx runtime DLL.
- Multiplayer requires every player to use identical files.

## Future Conflux Expansion

The runtime keeps the hero decision in `is_spiritist_hero()`. Nyx is currently
the only accepted hero ID. Adding another Conflux hero later requires:

1. Adding that hero ID to the runtime allowlist.
2. Replacing one of that hero's starting skills with Necromancy, or otherwise
   granting Necromancy to the hero.
3. Extending the patcher's verified executable-record changes.

The shared percentage and UI machinery does not need to be duplicated.

Implementation details and exact changes are recorded in
[TECHNICAL.md](TECHNICAL.md). Release history is in
[CHANGELOG.md](CHANGELOG.md).
