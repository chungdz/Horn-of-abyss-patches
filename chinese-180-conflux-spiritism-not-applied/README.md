# Chinese HotA 1.8.0 Conflux Spiritism Port

Version 0.1.2, 2026-08-16.

> **Unresolved:** This port passes file and runtime smoke checks, but the user
> reported that its visible Chinese changes still do not work in game. See
> [UNRESOLVED.md](UNRESOLVED.md) for the attempted changes and remaining work.

This folder records and applies the Nyx Pixie Elementalist plus final Conflux
Spiritism modification to:

```text
C:\Users\yunfanhu\workplace\英雄无敌3深渊号角1.80汉化正式版1.3
```

The target is Horn of the Abyss 1.8.0 with the Chinese GBK patch and HD Mod
5.6 R10. The original English patch was tested on HD Mod 5.6 R16, so its
runtime DLL could not safely be copied unchanged.

## What Was Ported

- Replaces Inteus with Nyx, localized as `倪克斯`.
- Gives Nyx the Pixie/Sprite specialty and three `22-25 Pixies` stacks.
- Gives every Conflux hero Spiritism in the first secondary-skill slot.
- Uses Conflux-only `10% / 20% / 30%` raising rates.
- Raises Pixies while preserving native Necromancy artifacts, Amplifiers,
  AI handling, save data, rounding, and army insertion.
- Installs the tested Nyx portraits, specialty pictures, and Spiritism icons.
- Localizes Spiritism as `唤灵术` and supplies GBK descriptions and result
  messages.

## Compatibility Work

The two game executables are byte-identical to the supported HotA 1.8.0
baseline. The Chinese package uses different `HotA.dll` and `HD_HOTA.dll`
builds, however:

- Chinese `HD_HOTA.dll`: HD Mod 5.6 R10
- English tested `HD_HOTA.dll`: HD Mod 5.6 R16

The R10 runtime profile maps the equivalent hero-panel calls, resource-name
literals, exchange-dialog resources, and hero-inspection guards. The Chinese
DLLs remain untouched and are checksum-verified before and after installation.

The Chinese HD pack also supplies loose `HOTRAITS.TXT`, `HeroSpec.txt`, and
`HeroBios.txt` files under `H3中文-基础资源`. They appeared to override the
patched `HotA_lng.lod`, so version 0.1.2 patches and verifies those loose rows
as well as the archive copies. The failed in-game test shows that another
source or later localization step still needs to be identified.

HotA keeps separate primary and adventure/UI General Text objects. The
stock executable's result path reads entries 171 and 172 from the adventure/UI
table at `0x00696A68`, while other paths use the table at `0x006A5DC4`.
Runtime 9 attempts to alias both tables only after a Conflux Spiritism hero is
confirmed to be raising Pixies. Ordinary Necromancy messages remain `招魂术`.

## Files

- `patch.py`: status, apply, backup, verification, and restore.
- `runtime-hook/runtime-spiritism-cn-r10.cpp`: selects the R10/GBK build
  profile from the shared runtime source.
- `runtime-hook/build.sh`: reproducible x86 Windows DLL build.
- `assets/ConfluxSpiritismRuntime-cn-r10.dll`: generated R10 runtime.

The installer updates only the relevant executable records, LOD entries,
loose resources, and HD compatibility registrations. It preserves unrelated
Chinese archive entries instead of replacing whole localized archives.

## Usage

```bash
python3 patch.py status \
  --game-dir "/mnt/c/Users/yunfanhu/workplace/英雄无敌3深渊号角1.80汉化正式版1.3"

python3 patch.py apply \
  --game-dir "/mnt/c/Users/yunfanhu/workplace/英雄无敌3深渊号角1.80汉化正式版1.3"
```

Each apply creates a rollback under:

```text
<game-dir>/ChineseConfluxSpiritismPatch/backups/YYYYMMDD-HHMMSS/
```

Restore the newest backup:

```bash
python3 patch.py restore \
  --game-dir "/mnt/c/Users/yunfanhu/workplace/英雄无敌3深渊号角1.80汉化正式版1.3"
```

The installed game also receives:

```text
ChineseConfluxSpiritismPatch/installation.json
```

That record contains the installation time, source payload, backup path,
runtime checksum, preserved binary checksums, and localization notes.

## Verification

`status` verifies mechanical installation only:

- Both final executable hashes
- GBK Nyx name, biography, specialty text, and starting-army row
- The active `H3中文-基础资源` hero, biography, and specialty overrides
- Patched `UN32.def`, `UN44.def`, `IX32.def`, and `IX44.def` archive entries
- Nyx portrait entries in both bitmap archives
- All loose Spiritism and Nyx resources
- Both HD compatibility registrations
- Removal of unsafe loose `UN32.def` and `UN44.def` overrides
- The R10 runtime checksum
- Both attempted post-battle General Text tables are available
- Unchanged Chinese `HotA.dll` and `HD_HOTA.dll`
- Runtime hook success after a game launch

These checks do not establish in-game success. The status command reports
mechanical application and user-facing validation separately.

## Known Limitation

The fixed-scenario starting-hero selector retains the same cosmetic Bloodlust
specialty-picture limitation documented by the final English patch. The
shared extended specialty atlas is deliberately not replaced at runtime.
