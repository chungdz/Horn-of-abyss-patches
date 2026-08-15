# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- Existing prerequisite patch: Nyx Pixie Elementalist 1.7.0 installation
- Test hero: Nyx
- Hero ID: `140`
- Raised creature: Pixie
- Creature ID: `118`
- Internal secondary skill: Necromancy
- Secondary skill ID: `12`

## Design

Spiritism is an alias over Necromancy rather than a new secondary-skill ID.
This keeps HotA's native implementation responsible for:

- Basic, Advanced, and Expert power
- Necromancy Amplifiers
- Necromancy artifact bonuses
- Hero specialty bonuses, if later assigned
- Defeated-stack eligibility
- Rounding and minimum results
- Adding the raised stack to the army
- AI combat results
- Save and load

Only the returned creature and Nyx-facing presentation are changed.

## Executable Record

Nyx's `H3HeroInfo` record begins at file offset `0x27D020` in both:

- `h3hota.exe`
- `h3hota HD.exe`

The first secondary-skill type is at record offset `0x0C`, absolute file
offset `0x27D02C`.

```text
Before: 07 00 00 00  Basic Wisdom
After:  0C 00 00 00  Basic Necromancy, presented as Spiritism
```

The first skill's level remains `1` for Basic. The second skill remains
Basic Fire Magic, skill ID `14`.

## Runtime Hooks

The existing Nyx patch already occupies HD Mod's
`_HD3_Data/Common/setseed.dll` loader slot. The new
`NyxSpiritismRuntime.dll` therefore merges the original Nyx specialty-frame
fix with Spiritism.

| Address | HotA function | Change |
| --- | --- | --- |
| `0x004E3ED0` | `H3Hero::GetNecromancyCreatureId` | Return Pixie `118` when the hero is Nyx and has Necromancy |
| `0x004E1A70` | Standard hero dialog | Scope the Spiritism text and resource-name aliases around Nyx's modal dialog |
| `0x004DA990` | Hero level-up processing | Scope the same aliases around Nyx's level-up UI |

HotA and HD Mod have already placed five-byte relative jumps at those three
function entries by the time the runtime installs. Runtime 8 waits until all
reviewed game and HD call bytes remain unchanged for one second, records each
existing jump target, and replaces the jump with one to the Spiritism wrapper.
The wrapper calls the recorded target for native behavior.

This direct chaining deliberately does not call `GetPatcher`,
`CreateInstance`, or `WriteHiHook` from the background loader thread. All five
relative patches are prepared and validated before the first write. If any
write or read-back verification fails, every written instruction is restored.
The Python installer separately checksum-verifies the supported files.

The creature hook is reached by both reviewed combat-result paths:

- Manual combat calls it at `0x00476EF4`.
- AI/quick combat calls it at `0x00426E29`.

The native power function at `0x004E3F40` is deliberately not hooked. It
reads Nyx's Necromancy level and applies HotA's existing artifact, town, and
other percentage modifiers.

The Cloak of the Undead King normally changes the raised creature according
to Necromancy level. Nyx's creature-selection hook takes precedence and
always returns Pixie while leaving the Cloak's percentage contribution in the
native power function.

## UI Alias

Virtual address `0x0067DCF0` stores a pointer to the secondary-skill text
table. The runtime reads that pointer, selects Necromancy entry `12`, and
temporarily replaces its four pointers during Nyx's hero or level-up UI with:

- Name: `Spiritism`
- Basic description: 5%, Pixies, and inherited building/artifact bonuses
- Advanced description: 10%, Pixies, and inherited building/artifact bonuses
- Expert description: 15%, Pixies, and inherited building/artifact bonuses

The displayed descriptions do not call the skill Necromancy. The original
four pointers are restored immediately after the modal UI call returns.

`Secskill.def` and `SecSk82.def` use three frames per skill. Necromancy's
Basic, Advanced, and Expert pictures are frames `39`, `40`, and `41` in both
atlases. `SPIRIT.def` is the 44x44 clone; `SPIR82.def` is the 82x93 clone used
by the right-click skill detail popup.

Virtual address `0x006601D0` is the writable 16-byte static filename literal
used by the game's secondary-skill image paths:

```text
secskill.def 00 00 00 00
```

For the synchronous duration of Nyx's modal hero or level-up call, runtime 8
replaces those 16 bytes with:

```text
SPIRIT.def 00 00 00 00 00 00
```

The detail-popup image switch independently reads the 12-byte filename at
`0x006600F8`. Runtime 8 changes `secsk82.def` to `SPIR82.def` in the same
scope. A nesting counter keeps both aliases active for nested UI calls. The
original bytes and text pointers are restored before the outer call returns,
so combat construction and normal heroes see only native resource names.

The HD pregame hero panel is built by the stdcall function at
`HD_HOTA.dll` RVA `0x002350E0`. Its two reviewed call sites are RVAs
`0x0023708A` and `0x00237C9D`; runtime 8 redirects only those calls. For hero
ID `140`, the wrapper scopes the normal Spiritism text aliases and changes the
HD module's 16-byte `Secskill.def` literal at RVA `0x002A0450` to
`SPIRIT.def` while the panel controls are constructed. The literal resides in
read-only image data, so the runtime temporarily changes page protection and
restores both bytes and protection afterward.

## Post-Battle Message

The native plural and singular Necromancy result formats are General Text
entries `171` and `172`. The General Text object pointer is stored at
`0x006A5DC4`, its string table begins at object offset `0x20`, and the two
entry-pointer offsets are `0x2AC` and `0x2B0`.

Once the creature-selection hook confirms that Nyx is raising Pixies, the
runtime replaces those two string pointers before the result formatter reads
them. The custom messages begin with:

```text
Practicing the art of Spiritism
```

An ordinary Necromancy creature calculation restores both original pointers
before its result is formatted, so other heroes continue to use the native
Necromancy messages.

## Rejected Runtime Approach

Runtime 4 used a low-level hook at the dialog DEF constructor and edited its
raw stack. It installed successfully but corrupted the live call context and
crashed HotA after entering the game.

Runtime 5 removed that hook but placed raw replacement-frame pointers into the
already loaded `Secskill.def` table. HotA 1.8.0 and HD Mod keep converted
surface state tied to those frame objects, so opening a hero page crashed in
HotA's draw path.

Runtime 6 used a correctly typed high-level hook on the global `H3DlgDef`
constructor. It rendered the hero-page icon, but the hook also intercepted
unrelated dialog creation during combat startup and crashed before battle.

Runtime 7 has no dialog-constructor hook and never mutates a loaded DEF frame
or converted surface.

Runtime 7 still installed its three high-level hooks through `patcher_x86`
from a worker thread after a fixed 1.5-second delay. One launch collided with
HD/HotA startup patching and crashed in `patcher_x86.dll` while it copied a
string from invalid address `0x00000005`. Runtime 8 removes that API usage and
uses stabilized direct relative chaining instead.

## Icon Generation

The installer:

1. Extracts `Secskill.def` and `SecSk82.def` from the installed
   `Data/H3sprite.lod`.
2. Requires SHA-256
   `298f31e75e045fcb1195d870efbed8d7f5ecb81bab18e0ffc89ccc6a81c91aee`.
   The large atlas must have SHA-256
   `a04c3bc1871fca84e66692e19cda5b77ecd67d524ebb86bd8564dbc5b6578892`.
3. Decodes the three supplied non-interlaced 8-bit RGBA PNG files.
4. Area-downsamples each image to 44x44 and 82x82. The large version is
   centered in an 82x93 canvas.
5. Extracts the 256x256 `DiBoxBck.pcx` dialog texture from
   `Data/H3bitmap.lod`.
6. Fills each canvas from the brown texture at crop origin `(84, 0)`, blends
   antialiased source edges over it, and maps the result to the nearest source
   DEF palette index from `8` through `255`.
7. Appends uncompressed replacement frames with unique internal names.
8. Writes `SPIRIT.def` and `SPIR82.def` as loose resources and registers both
   in the two HotA HD compatibility packs.

Generated resource SHA-256 for the supplied images:

```text
SPIRIT.def  ba4ba357d2859b8e5dc8077bce00b1effc0a40b42fb25fa9f53ed76dd0d85eb3
SPIR82.def  8016d09158fee026bcccc83a5c43dd9d8a4cf6a42db113f8e51e81270b63392f
```

## Runtime Build

Source:

```text
runtime-hook/runtime-spiritism.cpp
```

Build:

```bash
runtime-hook/build.sh
```

The build uses Zig 0.15.2 as an x86 Windows GNU C++ cross-compiler, fixes
`SOURCE_DATE_EPOCH` to the reviewed release timestamp by default, and imports
only `KERNEL32.dll`. Repeated default builds are byte-identical to:

```text
assets/NyxSpiritismRuntime.dll
SHA-256 54c997f1aebc081f2b944cbb9cecb366121c91ab83e3963056ac3641b4656a9f
```

At runtime it writes:

```text
_HD3_Data/Common/NyxSpiritism.log
```

The accepted runtime-8 test log reported all five direct hooks installed,
both resource aliases ready, both Nyx specialty frames patched, and the final
status `specialty fix and Spiritism hooks installed`. Manual testing then
covered a clean HD launch, the pregame panel, hero page, skill-detail popup,
battle startup and completion, Pixie raising, and the Spiritism result text.

## Files Changed By Apply

- `h3hota.exe`
- `h3hota HD.exe`
- `Data/SPIRIT.def`
- `Data/SPIR82.def`
- `_HD3_Data/Compability/#hota/SPIRIT.def`
- `_HD3_Data/Compability/#hota/SPIR82.def`
- `_HD3_Data/Compability/#hota15/SPIRIT.def`
- `_HD3_Data/Compability/#hota15/SPIR82.def`
- `_HD3_Data/Compability/#hota/Files.ini`
- `_HD3_Data/Compability/#hota15/Files.ini`
- `_HD3_Data/Common/setseed.dll`

The installer backs up those paths, plus the runtime log path, under
`NyxSpiritismPatch/backups/` before writing.

The installed Spiritism executable hashes are:

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `0e710b6de65ec869b333a18a858c9f2c1fe3e1d3eb2ba2a076db159720d66b61` |
| `h3hota HD.exe` | `c0f7d8801e2e475260ac573b1a2457cfd4e8bde8ad892bc952f4cb95930d24b5` |

## Source Icon Hashes

| Asset | SHA-256 |
| --- | --- |
| `basic-spiritism.png` | `e54b3cdad077bc6c7ed5bf1b7ac4b7cbdd65b85cc281bfada35825d05b011672` |
| `advanced-spiritism.png` | `26bf6e76d761b78f35a0249161c8308374762be90bd480467055599e4dc6b596` |
| `expert-spiritism.png` | `23af594cfb1d9e9488ca47cec9961ad9f339145c0ccaac7178d33a0800308eaf` |
