# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- HD Mod: 5.6 R16
- Prerequisite: Nyx Spiritism 0.1.5
- Conflux hero IDs: `128` through `143`
- Internal secondary skill: Necromancy, ID `12`
- Raised creature: Pixie, creature ID `118`

## Executable Records

The sixteen Conflux `H3HeroInfo` records are contiguous. Their first
secondary-skill type fields use:

```text
offset(hero_id) = 0x27CBDC + (hero_id - 128) * 0x5C
```

Only that four-byte type field is replaced with `12`. The adjacent first-skill
level and the complete second-skill pair are checksum- and value-verified but
not modified.

| ID | Hero | Type offset | Original slot one | Preserved level | Preserved slot two |
| --- | --- | --- | --- | --- | --- |
| 128 | Pasis | `0x27CBDC` | Offense `22` | Basic `1` | Artillery `20`, Basic `1` |
| 129 | Thunar | `0x27CC38` | Tactics `19` | Basic `1` | Estates `13`, Basic `1` |
| 130 | Ignissa | `0x27CC94` | Offense `22` | Basic `1` | Artillery `20`, Basic `1` |
| 131 | Lacus | `0x27CCF0` | Tactics `19` | Advanced `2` | Empty |
| 132 | Monere | `0x27CD4C` | Offense `22` | Basic `1` | Logistics `2`, Basic `1` |
| 133 | Erdamon | `0x27CDA8` | Tactics `19` | Basic `1` | Estates `13`, Basic `1` |
| 134 | Fiur | `0x27CE04` | Offense `22` | Advanced `2` | Empty |
| 135 | Kalt | `0x27CE60` | Tactics `19` | Basic `1` | Learning `21`, Basic `1` |
| 136 | Luna | `0x27CEBC` | Wisdom `7` | Basic `1` | Fire Magic `14`, Basic `1` |
| 137 | Brissa | `0x27CF18` | Wisdom `7` | Basic `1` | Air Magic `15`, Basic `1` |
| 138 | Ciele | `0x27CF74` | Wisdom `7` | Basic `1` | Water Magic `16`, Basic `1` |
| 139 | Labetha | `0x27CFD0` | Wisdom `7` | Basic `1` | Earth Magic `17`, Basic `1` |
| 140 | Nyx | `0x27D02C` | Spiritism `12` | Basic `1` | Fire Magic `14`, Basic `1` |
| 141 | Aenain | `0x27D088` | Wisdom `7` | Basic `1` | Air Magic `15`, Basic `1` |
| 142 | Gelare | `0x27D0E4` | Wisdom `7` | Basic `1` | Water Magic `16`, Basic `1` |
| 143 | Grindan | `0x27D140` | Wisdom `7` | Basic `1` | Earth Magic `17`, Basic `1` |

The prerequisite Nyx-only executable hashes are:

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `0e710b6de65ec869b333a18a858c9f2c1fe3e1d3eb2ba2a076db159720d66b61` |
| `h3hota HD.exe` | `c0f7d8801e2e475260ac573b1a2457cfd4e8bde8ad892bc952f4cb95930d24b5` |

The generated Conflux-wide hashes are:

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `7aeb166c1976d87dd7b7ba43de033ec986bb4ee7d463816213f5d0a9afff7580` |
| `h3hota HD.exe` | `110122278fb9a2ac66d39b5243d00561c6725fbda54bf23cf41c034baab6c080` |

## Runtime

`ConfluxSpiritismRuntime.dll` is derived from the accepted Nyx Spiritism
runtime 8. The hook addresses, direct relative-chaining backend, scoped text
and resource aliases, post-battle message handling, and safety checks are
unchanged.

The gameplay and UI eligibility check accepts hero IDs `128–143` and requires
the hero's internal Necromancy level to be nonzero. This affects:

- Raised-creature selection
- Standard hero dialogs
- Hero level-up dialogs
- HD pregame hero panels
- Spiritism name and descriptions
- 44x44 and 82x93 custom icons
- Spiritism post-battle result wording

The original Nyx specialty-picture repair remains restricted to frame `140`
in `UN32.def` and `UN44.def`.

The runtime writes:

```text
_HD3_Data/Common/ConfluxSpiritism.log
```

Its success log begins with:

```text
Conflux Spiritism runtime 1
heroes=128-143 creature=118 underlying-skill=12
```

and must report all four hook categories installed, the direct-relative
backend, both specialty frames patched, and the final success line.

## Shared Resources

The upgrade reuses the exact loose resources installed by Nyx Spiritism:

| Resource | SHA-256 |
| --- | --- |
| `SPIRIT.def` | `ba4ba357d2859b8e5dc8077bce00b1effc0a40b42fb25fa9f53ed76dd0d85eb3` |
| `SPIR82.def` | `8016d09158fee026bcccc83a5c43dd9d8a4cf6a42db113f8e51e81270b63392f` |

The installer verifies all three copies of each resource and confirms both
names are registered in `#hota/Files.ini` and `#hota15/Files.ini`. It does not
rewrite or duplicate them.

## Runtime Build

Source:

```text
runtime-hook/runtime-spiritism.cpp
```

Build:

```bash
runtime-hook/build.sh
```

The deterministic Zig 0.15.2 x86 Windows build imports only `KERNEL32.dll`.

```text
assets/ConfluxSpiritismRuntime.dll
SHA-256 eabfbe0bf6e98612895359e2d96746fc9405ddd32c43aad901e07753ab0670d0
```

Two consecutive default builds reproduce this checksum byte for byte.

## Installer Verification

The complete installer was exercised against an isolated copy of the tested
Nyx Spiritism 0.1.5 installation:

- `apply` produced the documented Conflux-wide executable and runtime hashes.
- `status` reported all sixteen first-skill slots as Spiritism.
- `restore` reproduced the original Nyx-only executable and runtime hashes.
- Restore removed the Conflux runtime log when it did not exist before apply.

The live installation was subsequently tested in game. Its runtime log
reported all four hook categories installed through the direct-relative
backend, and the Conflux-wide Spiritism behavior was confirmed working.

## Files Changed By Apply

- `h3hota.exe`
- `h3hota HD.exe`
- `_HD3_Data/Common/setseed.dll`

The installer also creates
`_HD3_Data/Common/ConfluxSpiritism.log` on first launch. All four paths are
represented in the timestamped backup manifest so restore returns exactly to
the Nyx-only installation and removes a newly created log when appropriate.

Backups are stored under:

```text
ConfluxSpiritismPatch/backups/
```
