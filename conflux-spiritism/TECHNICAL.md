# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- HD Mod: 5.6 R16
- Patch version: 0.2.8
- Prerequisite: Nyx Spiritism 0.1.5/0.1.6 or Conflux Spiritism
  0.1.0/0.2.0/0.2.1/0.2.2/0.2.3/0.2.4/0.2.5/0.2.6/0.2.7
- Conflux hero IDs: `128` through `143`
- Internal secondary skill: Necromancy, ID `12`
- Raised creature without Cloak: Sprite, creature ID `119`
- Raised creatures with Cloak: Fire Elemental `114`, Earth Elemental `113`,
  Psychic Elemental `120` at Basic, Advanced, and Expert
- Spiritism base rates: 10%/20%/30%

## Live Installed State

The live installation uses runtime 10 with SHA-256
`6e1c82e0ba5100505bddb55a4f0089694a1ee9fe7b5144a3db3ad5098e0a694e`.
`patch.py status` reports both executables complete, all sixteen hero records
as Spiritism, all three custom skill resources registered, and truncated
specialty overrides removed. The runtime 10 launch log reports every gameplay,
UI, exchange-dialog, and inspection-guard hook installed.

Active image resources:

- `SPIRIT.def`: 44x44 Spiritism skill frames
- `SPIR32.def`: 32x32 Spiritism exchange-dialog frames
- `SPIR82.def`: 82x93 Spiritism detail-dialog frames
- `IX32.def`: 32x32 Nyx specialty frames, scoped to supported exchanges
- `IX44.def`: 44x44 Nyx specialty frames, scoped to hero dialogs

The `IX` resources are loaded as independent DEFs. Runtime 10 does not replace
a frame pointer or pixel buffer in HotA's shared specialty atlases.

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

`ConfluxSpiritismRuntime.dll` retains the accepted Nyx Spiritism runtime
behavior and adds a Conflux-only power hook.

| Address | Function | Change |
| --- | --- | --- |
| `0x004E3ED0` | `H3Hero::GetNecromancyCreatureId` | Return Sprite `119`, or map the native Cloak result to Fire `114`, Earth `113`, or Psychic `120` |
| `0x004E3F40` | `H3Hero::GetNecromancyPower` | Add 5% per Spiritism level after HotA's native calculation |
| `0x004E1A70` | Standard hero dialog | Scope the Spiritism text and resource aliases |
| `0x004DA990` | Hero level-up processing | Scope the Spiritism text and resource aliases |
| `0x005AAD90` | HD SwapMgr dialog builder | Scope `SPIR32.def` and Nyx's `IX32.def` while building the two-hero exchange UI |
| `HotA.dll+0x2E5D` | Hero inspection handler 1 | Guard the current-hero pointer before dereferencing offset `0x1A` |
| `HotA.dll+0x3F2D` | Hero inspection handler 2 | Guard the duplicate current-hero dereference used while switching heroes |

HotA changes the original Necromancy base table at `0x0063E9BC`,
`0x0063E9C0`, and `0x0063E9C4` from the SoD values to 5%, 10%, and 15%.
The new hook deliberately does not rewrite that global table because doing so
would also change ordinary Necromancy.

Instead, it calls the complete live HotA power function and applies:

```text
Spiritism power = HotA Necromancy power + 0.05 * skill level
```

The added values are therefore 5%, 10%, and 15%, producing final base rates
of 10%, 20%, and 30%. HotA's artifact, Necromancy Amplifier, specialty, town,
and other modifiers remain in the chained result. When the caller requests
the native cap, the wrapper caps the final value at 100%.

The power function normally retains its six-byte
`55 8B EC 83 EC 08` prologue after HotA changes the rate table. The runtime
copies those instructions into an executable trampoline and replaces them
with a five-byte jump plus one NOP. If another supported component has already
placed a relative jump there, the runtime chains that live target instead.
All code patches are validated before writing and rolled back together on
failure.

The English creature wrapper first calls the complete live HotA selector.
HotA 1.8.0 returns Skeleton `56` without the Cloak, Walking Dead `58` at Basic,
Wight `60` at Advanced, and Lich `64` at Expert. The wrapper maps the three
Cloak results to Fire Elemental `114`, Earth Elemental `113`, and Psychic
Elemental `120`; every other result becomes Sprite `119`. The
`CHINESE_HOTA_R10` build retains its previously tested Pixie-only branch.

The wrapper does not bypass HotA's later post-battle eligibility rules. In
particular, the Ring of Oblivion, artifact ID `158`, makes battle losses
irrevocable and suppresses both ordinary Necromancy and Spiritism regardless
of the selected raised creature. The Ring must be unequipped, not merely
followed by an application restart, because equipped artifacts are serialized
in the saved game.

The gameplay and UI eligibility check accepts hero IDs `128–143` and requires
the hero's internal Necromancy level to be nonzero. This affects:

- Raised-creature selection
- Conflux-only 10%/20%/30% power calculation
- Standard hero dialogs
- Hero level-up dialogs
- HD pregame hero panels
- HD creature/artifact exchange dialogs
- Spiritism name and descriptions
- 32x32, 44x44, and 82x93 custom icons
- Spiritism post-battle result wording

Runtime 10 does not modify a specialty atlas, frame-table pointer, frame
object, or pixel buffer. For Nyx's standard hero dialog it temporarily
changes the `un44.def` resource literal at `0x00679D90` to `IX44.def`. For
the HD pregame panel it scopes the equivalent literal at
`HD_HOTA.dll+0x2A043C`.

HD Mod replaces the original SwapMgr builder at `0x005AAD90` with a
thiscall-compatible relative jump. Runtime 10 chains that live target and
uses the builder's existing `H3Hero*[2]` argument to decide whether to scope:

- `HD_HOTA.dll+0x297650`: `secsk32.def` to `SPIR32.def`
- `HD_HOTA.dll+0x2975F0`: `un32.def` to `IX32.def`

The Spiritism alias is enabled only when at least one displayed hero is a
Conflux Spiritist and neither displayed hero is an ordinary Necromancer. The
Nyx alias is enabled only when Nyx is displayed and both hero IDs are below
156, the exact frame count of `IX32.def`. The original names are restored as
soon as dialog construction returns.

The original Inteus/Nyx runtime trace proved that the fixed-scenario selector
does not call the expected image constructor at `0x004EA800`; it reuses an
already-loaded `UN32.def` object and changes frames directly. Runtime 7 hooks
the apparent construction instruction at `0x0051DCCB`, but in-game testing
still shows Bloodlust because the actual scenario view bypasses that path.
Runtime 8 removes the ineffective hook.

The older working installation also had a loose, registered 156-frame
`UN32.def` override. Its runtime logs showed raw target frame
`Un32Lust.PCX`, which could be replaced with raw `NYX32PIX.PCX`. After version
0.2.1 removed that truncated override to preserve HotA-added hero frames, the
native extended atlas instead exposed converted target `un32.def_018`.

A post-0.2.5 diagnostic experiment restored only the `UN32.def` pointer swap.
Its log verified a write from raw `NYX32PIX.PCX` to converted
`un32.def_018`, but the scenario still displayed Bloodlust. The experiment
was rolled back. A future repair must reach the persistent scenario control
or create a compatible converted frame without restoring the truncated atlas.

### Extended Hero Specialty Atlases

Earlier Nyx and Conflux releases registered loose `UN32.def` and `UN44.def`
overrides under both HD compatibility packs. Each override contained only
156 frames, covering hero IDs `0–155`. HotA 1.8.0 added heroes beyond that
range, and HD Mod indexes the specialty atlas directly by hero ID. Vehr is
hero ID `212`, so the truncated override returned an unrelated frame; the
same defect affected every HotA-added hero whose ID was beyond frame 155.

Version 0.2.1 checksum-verifies and removes these four obsolete files:

```text
_HD3_Data/Compability/#hota/UN32.def
_HD3_Data/Compability/#hota/UN44.def
_HD3_Data/Compability/#hota15/UN32.def
_HD3_Data/Compability/#hota15/UN44.def
```

It also removes the exact `"UN32.def"` and `"UN44.def"` registration lines
from both compatibility-pack `Files.ini` files. Startup additionally verifies
that Vehr's frame 212 is available before reporting success.

### Rejected Specialty-Picture Repairs

Runtime 3 retained the older Nyx implementation that replaced the
`UN32.def` and `UN44.def` frame-table pointer for hero 140 with the
corresponding raw `IX32.def` or `IX44.def` frame. The town visiting-hero
renderer exposed an ownership mismatch that earlier UI tests had not reached.

The August 15 visiting-hero crash occurred at `HotA.dll+0x7978A`:

```text
cmp dword ptr [eax+0x35], 0
```

The crash context had `EAX=0x51` and `ESI` equal to the logged raw
`IX44.def` frame pointer. Direct process-memory inspection showed:

- HotA's `UN44.def` entry is a native `D32P` object with a 7,744-byte
  44x44x4 pixel buffer and a linked renderer object at offset `0x48`.
- The `IX44.def` source is a 72-byte indexed `DefFrame`; the bytes at offset
  `0x48` are unrelated heap data whose value happened to be `0x51`.

Runtime 4 stopped changing frame-table pointers. For each of `UN32.def` and
`UN44.def`, it:

1. Validates hero 140's target as a native `D32P` frame.
2. Validates the target dimensions, four-byte pixel format, stride, pixel
   buffer, and renderer pointer.
3. Validates the corresponding `IX` source as an uncompressed indexed frame
   with matching dimensions and zero margins.
4. Validates the loaded DEF palette header.
5. Maps each indexed source pixel through the game-loaded RGB palette into
   the existing renderer-owned 32-bit pixel buffer with opaque alpha.
6. Reads the complete buffer back and confirms the frame-table pointer was
   never changed.

The exact writes were 4,096 bytes for the 32x32 frame and 7,744 bytes for the
44x44 frame. Live testing showed that these renderer buffers were shared:
Akka and other heroes inherited the blue Pixie image, and switching Conflux
heroes could still crash. Runtime 5 removes both the frame-pointer and
pixel-buffer implementations. No `UN32.def` or `UN44.def` memory is written.

Runtime 6 resolves the problem by redirecting the dialog's resource name
before the control is constructed. The renderer loads `IX32.def` or
`IX44.def` normally as an independent native object, so neither the shared
HotA atlas nor another hero's converted frame is changed.

### Hero Inspection Null Guards

The August 15 crash report records an access violation at
`HotA.dll+0x2E63`, reading address `0x0000001A` with `ECX=0`. The enabled HD
setting was `<UI.Tavern.InviteHero>=1`. HotA 1.8.0's live instruction
sequence is:

```text
HotA.dll+0x2E5D  mov ecx, [HotA.dll+0x250E74]
HotA.dll+0x2E63  mov ecx, [ecx+0x1A]
```

Version 0.2.1 validated all nine original bytes and guarded this first site.
A non-null pointer executes the original dereference and continues at
`HotA.dll+0x2E66`. A null pointer jumps to the function's existing cleanup
path at `HotA.dll+0x2EED`.

The later visiting/garrison hero-switch crash occurred at the duplicate
sequence:

```text
HotA.dll+0x3F2D  mov ecx, [HotA.dll+0x250E74]
HotA.dll+0x3F33  mov ecx, [ecx+0x1A]
```

Runtime 5 validates and guards both nine-byte sequences. The second guard
continues at `HotA.dll+0x3F36` for a non-null hero and uses the function's
cleanup path at `HotA.dll+0x3FBD` for null. All eight runtime code patches are
prepared before any are written and are rolled back together if validation
or verification fails.

The runtime writes:

```text
_HD3_Data/Common/ConfluxSpiritism.log
```

Its success log begins with:

```text
Conflux Spiritism runtime 10
heroes=128-143 creature=119 cloak=114/113/120 rates=10/20/30 underlying-skill=12
```

and must report the creature, rate, hero-dialog, level-up, HD hero-selection,
and both hero-inspection guards installed. It must also report:

```text
scoped Nyx specialty aliases=ready
scoped exchange Spiritism alias=ready
HD exchange dialog hook=installed
specialty atlas mutation=disabled
extended specialty Vehr frame=available
final=Spiritism and Nyx UI hooks installed; shared atlas untouched
```

## Shared Resources

The upgrade reuses the exact loose resources installed by Nyx Spiritism:

| Resource | SHA-256 |
| --- | --- |
| `SPIRIT.def` | `ba4ba357d2859b8e5dc8077bce00b1effc0a40b42fb25fa9f53ed76dd0d85eb3` |
| `SPIR32.def` | `0ab002201dcb81a18c989f0e49e4d37b716ff209dd86ada651f9a51c5078511c` |
| `SPIR82.def` | `8016d09158fee026bcccc83a5c43dd9d8a4cf6a42db113f8e51e81270b63392f` |
| `IX32.def` | `63ff856d3ed52daaf3b834715c60a7e39da3223fce17357367cb45ee9f810198` |
| `IX44.def` | `eeb281b6490e4ef7e786f40e8601ca64ffa05247cd4c0c7d337382da631cf807` |

The installer verifies all three copies of each resource and confirms all
three names are registered in `#hota/Files.ini` and `#hota15/Files.ini`.
`SPIR32.def` is built from HotA 1.8.0's `SECSK32.def` with only Necromancy
frames 39, 40, and 41 replaced by the Basic, Advanced, and Expert Spiritism
art.

The installer verifies the exact `IX32.def` and `IX44.def` copies in `Data`
and both compatibility packs, plus both registration lines. Runtime 10
references them only through scoped resource-name aliases.

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
SHA-256 6e1c82e0ba5100505bddb55a4f0089694a1ee9fe7b5144a3db3ad5098e0a694e
```

Two consecutive default builds reproduce this checksum byte for byte.

## Installer Verification

The complete installer was exercised against an isolated copy of the tested
Nyx Spiritism 0.1.5 installation:

- `apply` produced the documented Conflux-wide executable and runtime hashes.
- `status` reported all sixteen first-skill slots as Spiritism.
- `restore` reproduced the original Nyx-only executable and runtime hashes.
- Restore removed the Conflux runtime log when it did not exist before apply.

Version 0.1.0 was subsequently tested in game. Its runtime log reported all
four original hook categories installed through the direct-relative backend,
and the Conflux-wide Spiritism behavior was confirmed working.

The 0.2.0 installer was also exercised against an isolated copy of the live
0.1.0 installation:

- `status` identified the reviewed 0.1.0 runtime as an upgrade source.
- `apply` retained both executable hashes and installed the 0.2.0 runtime.
- `restore` reproduced the 0.1.0 runtime and prior log hashes exactly.

The live 0.2.0 installation then reported all five hook categories installed,
using the validated entry trampoline for `GetNecromancyPower`. The Basic
description displayed 10%, and defeating 28 Sprites at 3 health each raised
2 Pixies:

```text
floor(28 * 3 * 0.10 / 3) = 2
```

The same battle would raise only 1 Pixie at HotA's native 5% Basic rate.

The manual-combat routine at `0x00476F67` also confirms that each defeated
creature's health contribution is capped at the raised creature's health
before multiplication by the rate. Its single-stack calculation is:

```text
floor(casualties * min(defeated HP, raised HP) * power / raised HP)
```

The live Expert test defeated 56 Hobgoblins at 5 health each and raised
16 Pixies at 3 health:

```text
floor(56 * min(5, 3) * 0.30 / 3) = 16
```

The 0.2.1 installer was exercised against an isolated copy of the live 0.2.0
installation:

- `status` identified the reviewed 0.2.0 runtime and all four legacy
  specialty overrides.
- `apply` retained both executable hashes, installed runtime 3, removed only
  the four checksum-matched truncated atlases, and removed only their four
  registration lines.
- `status` reported all sixteen Spiritism records complete and the specialty
  overrides removed.
- `restore` reproduced the prior runtime, log, executable, `Files.ini`, and
  all four override hashes exactly.

The 0.2.1 runtime then reproduced a separate crash when Nyx was inspected as
a town's visiting hero. Its runtime log confirmed every hook and Vehr's
extended frame were valid, while the crash context tied `ESI` to the raw
`IX44.def` frame. Runtime 4 was built from the live target/source object
layouts captured from that still-running crashed process.

Runtime 4 removed the object-layout mismatch but exposed shared renderer
state: Nyx's blue Pixie pixels appeared for Akka and other heroes. A later
hero-switch crash occurred at `HotA.dll+0x3F33`, the second copy of the null
current-hero dereference.

Runtime 5 was installed and tested in game:

- The packaged and live runtime hashes both matched
  `78ebff46f61197e46cca580a159dbea42f64053bfdef12b1fce479163d06434a`.
- The log reported all gameplay/UI hooks and both inspection guards installed.
- The log reported `specialty atlas mutation=disabled`.
- Akka and HotA-added heroes recovered their correct specialty images.
- Repeated tavern cycling and visiting/garrison Conflux hero switching did
  not crash.
- Spiritism names, descriptions, icons, Pixie raising, and 10%/20%/30% rates
  remained active.
- Nyx used HotA's native specialty image; the custom Pixie specialty portrait
  was not installed.

The 0.2.4 installer was exercised against an isolated copy of that runtime 5
installation:

- `status` identified runtime 5 as the reviewed 0.2.3 upgrade source.
- `apply` installed runtime 6 and all three `SPIR32.def` copies, adding only
  the missing registration line to each compatibility pack.
- `status` reported all sixteen Spiritism records and all resources complete.
- `restore` reproduced the runtime 5 files and removed every newly installed
  `SPIR32.def` copy.

The 0.2.5 installer was exercised against an isolated copy of the live
runtime 6 installation:

- `status` identified runtime 6 as the reviewed 0.2.4 upgrade source.
- `apply` retained both executable hashes and every custom resource while
  installing runtime 7.
- `restore` reproduced the runtime 6 installation exactly.
- A live startup reported the scenario Nyx specialty hook installed together
  with every runtime 6 hook, but subsequent in-game testing still displayed
  Bloodlust. This matched the earlier constructor trace showing that the
  scenario view bypasses that instruction path.

A follow-up diagnostic runtime tried only the historical `UN32.def` frame-140
pointer replacement. It logged converted target `un32.def_018` and raw source
`NYX32PIX.PCX`, but the scenario still displayed Bloodlust. The diagnostic
runtime was rolled back to runtime 7 rather than retained.

The 0.2.6 installer was then exercised against an isolated copy of that
runtime 7 installation:

- `status` identified runtime 7 as the reviewed 0.2.5 upgrade source.
- `apply` installed runtime 8 without changing either executable or any
  image resource.
- `status` reported all sixteen Spiritism records and all resources complete.
- `restore` reproduced the runtime 7 installation exactly.

Runtime 8 removes the ineffective scenario constructor hook and contains no
specialty-atlas mutation. A live startup reported all gameplay, hero-dialog,
level-up, HD selection, exchange-dialog, and inspection-guard hooks installed.
It also reported `specialty atlas mutation=disabled`, confirmed Vehr's
extended specialty frame remained available, and emitted no scenario-selector
hook entry. The packaged and live runtime hashes both matched
`8841bc03b8a2e9cc39aacce50d963c55c37b2c635e701521ce6d07b85c6e396c`.

The 0.2.7 installer was exercised against an isolated copy of the live runtime
8 installation:

- `status` identified runtime 8 as the reviewed 0.2.6 upgrade source.
- `apply` installed runtime 9 without changing either executable or any image
  resource.
- `status` reported all sixteen Spiritism records and all resources complete.
- `restore` reproduced every copied pre-upgrade file byte for byte.

The guarded installer then upgraded the live game and created backup
`ConfluxSpiritismPatch/backups/20260816-111049`. The packaged and installed
runtime 9 DLLs both match
`938d53c27c298a4d856bef7d793724858a3fdcf262aff1a00cb1fd3488473a7a`.

Runtime 9 then passed live gameplay validation. Spiritism raised Pixies
without the Cloak and used Fire, Earth, and Psychic Elementals for the three
Cloak skill levels. A later report that Psychic Elementals, Pixies, and
ordinary Skeletons had all stopped was traced to native HotA behavior:

- In save `211`, Nyx did not have artifact `158` equipped and the Psychic
  Elemental stack was still growing.
- In save `221`, artifact `158` first appeared in Nyx's second ring slot.
- The Ring remained equipped in saves `225` and `226`; the Psychic Elemental
  stack only decreased from combat losses and received no post-battle gains.
- Live process inspection still showed the correct Conflux hero ID, Expert
  internal Necromancy, free army slots, and the requested raised-creature
  selector result.
- Ordinary Necromancy also stopped, isolating the behavior from the Spiritism
  wrapper.

HotA's own artifact data identifies `art158` as the Ring of Oblivion and states
that all battle losses become irrevocable. Moving the Ring to the backpack
restores native Necromancy and Spiritism generation.

The 0.2.8 release was exercised in two upgrade configurations:

- An isolated reviewed Nyx Spiritism 0.1.5 fixture upgraded to Nyx Spiritism
  0.1.6, then accepted Conflux Spiritism 0.2.8 as a fresh Conflux-wide layer.
- The live Conflux Spiritism 0.2.7 installation upgraded in place to runtime
  10 and created rollback backup
  `ConfluxSpiritismPatch/backups/20260819-201247`.

Both paths reported the new runtime, all sixteen Spiritism records, resources,
registrations, and specialty-override state complete. The installed and
packaged runtime 10 DLLs both match
`6e1c82e0ba5100505bddb55a4f0089694a1ee9fe7b5144a3db3ad5098e0a694e`.
Runtime 10 then passed in-game validation of Sprite raising.

## Files Changed By Apply

- `h3hota.exe`
- `h3hota HD.exe`
- `_HD3_Data/Common/setseed.dll`
- `Data/SPIR32.def`
- `_HD3_Data/Compability/#hota/SPIR32.def`
- `_HD3_Data/Compability/#hota15/SPIR32.def`
- `_HD3_Data/Compability/#hota/Files.ini`
- `_HD3_Data/Compability/#hota15/Files.ini`

Version 0.2.1 removes these obsolete loose overrides:

- `_HD3_Data/Compability/#hota/UN32.def`
- `_HD3_Data/Compability/#hota/UN44.def`
- `_HD3_Data/Compability/#hota15/UN32.def`
- `_HD3_Data/Compability/#hota15/UN44.def`

The installer also creates
`_HD3_Data/Common/ConfluxSpiritism.log` on first launch. Every changed or
removed path is represented in the timestamped backup manifest, so restore
returns exactly to the previous installation and removes a newly created log
when appropriate.

Backups are stored under:

```text
ConfluxSpiritismPatch/backups/
```
