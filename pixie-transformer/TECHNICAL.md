# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- HD Mod: 5.6 R16
- Patch version: 0.1.17
- Runtime marker: `Pixie Transformer runtime 7`
- Prerequisite: Conflux Spiritism 0.2.9 through 0.3.8
- Town: Conflux, ID `8`
- Garden controls and built bits: Horde 1 IDs `18` and `19`
- Actual Magic Lantern dwelling controls: IDs `30` and `37`
- Normal conversion target: Pixie, creature ID `118`
- Native Bone Dragon-class target: Firebird, creature ID `130`

Reviewed prerequisite hashes:

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `7aeb166c1976d87dd7b7ba43de033ec986bb4ee7d463816213f5d0a9afff7580` |
| `h3hota HD.exe` | `110122278fb9a2ac66d39b5243d00561c6725fbda54bf23cf41c034baab6c080` |
| `HotA.dll` | `e97aa25df70bc32c0cd5af20acec22207e86b13eb27fa5b705a102d5ef53fcec` |
| Original `Data/HotA_lng.lod` | `748b54cfac02ffc795f4b0c48c7cf6ef41ea0a6020f3cf41766271bd12eb81e9` |
| Pixie-only `Data/HotA_lng.lod` | `750a3384ad1bef990ec723154731ca24482e44f7f1f3390330c34c8fc89f162d` |
| Pixie/Firebird `Data/HotA_lng.lod` | `fff4e987f186673eab7e7b3f875db7a845ef3934448825ad85dff39843c3c9a2` |
| Spiritism 0.3.0 runtime | `0cf45c1ecff979d9d147b9b3484646a0122c6493345febb5c6b96952d21ff41a` |
| Spiritism 0.3.1 runtime | `dceddce37d411022967deec8f401c5d8bbceb526c0f7e39d83ff6dc37be28a5c` |
| Spiritism 0.3.2 runtime | `c756333e63ea9cd2c375c7b19b569d5296de054f773b96f739fda82f2ffba1b2` |
| Spiritism 0.3.3 runtime 15 RC | `1ce6ab184321838b5ab58e466e05a092fa9e62a4f6bbc4dbe59e210498902b91` |
| Spiritism 0.3.3 runtime 16 | `169202d2fcc5981f7dcb814fd4a4a813c67ec0f6c3d72af5eed2932d58a13bb0` |
| Spiritism 0.3.6 runtime 19 | `c6fa88be84b1531747433794210570514a6983ea83ee6878f3922ca3b06f674c` |
| Spiritism 0.3.7 runtime 21 | `c0c25c8be7e69f55c4ece35e11b791934d58b6279799474159802826983c46d8` |
| Spiritism 0.3.8 runtime 22 | `2201c2a38ce4816d4678c5d3d5bc15dd4a0894bcc190d0d747dcc4e8b6ed904a` |
| Pixie Transformer 0.1.0 runtime | `8956f877bf50ea63338230e956438bc8a8f8c15ea2ee5ad64a91690ea6b22b6f` |
| Pixie Transformer 0.1.1 runtime | `ab7ba9cf873fd60a33c1ac8243591b1407d00b027d8885d808ef10e75e4ed336` |
| Pixie Transformer 0.1.2 runtime | `e8f2268d696761bb9ddf9a5c0b9fc1dd53f2370d5cb87beb4e863f333e7cafc3` |
| Pixie Transformer 0.1.3 runtime | `92c03e938cdd9d1354eb65146db15c286403ba50a2632445c24cd45bd07f73d8` |
| Pixie Transformer 0.1.4 runtime | `26d84b9c76d59bd5988d390ce020a0c53b0778fb7db8831fbcb25244907a5a45` |
| Pixie Transformer 0.1.6 runtime | `446aad2c399a457da1c24e48876f7d7673a60362ac0c341a3e2b50512bb75ca4` |
| Pixie Transformer 0.1.7 runtime | `418103cc28826cda225439b6594b2ccb9c85176697d79a142bb9a0dbf47f2509` |

## Live Installed State

The live game retains the Pixie Transformer loader and uses the confirmed
Spiritism runtime 22 companion:

- Pixie Transformer `setseed.dll`:
  `418103cc28826cda225439b6594b2ccb9c85176697d79a142bb9a0dbf47f2509`
- Spiritism companion:
  `2201c2a38ce4816d4678c5d3d5bc15dd4a0894bcc190d0d747dcc4e8b6ed904a`
- Pixie/Firebird `Data/BldgSpec.txt`:
  `1c131975c28d7153b67de695cdbc97662681f90ed329d1986831d06a3026ed3d`
- Pixie/Firebird `Data/HotA_lng.lod`:
  `fff4e987f186673eab7e7b3f875db7a845ef3934448825ad85dff39843c3c9a2`
- Reviewed executables and `HotA.dll`
- Collision-free `SPIR32.def`:
  `22e030b0bef348c5afa682d693f14cbe3e7886b9dfa01b319b33eb323d3290a8`
- Transfer icon and right-click popup confirmed in game
- Hermit's Shack Spiritism description and large icon confirmed in game
- Standard/town hero page and right-click popup confirmed in game
- Runtime 22 passed five normal startup and clean-close cycles without a new
  crash

Backup `PixieTransformerPatch/backups/20260820-162535` contains the exact
prior 0.1.9 plus Spiritism 0.3.1 composition.

## Runtime Composition

The game has one `_HD3_Data/Common/setseed.dll` loader slot. The installer
copies the reviewed Spiritism runtime to
`ConfluxSpiritismRuntime.dll`, installs the Pixie Transformer overlay as
`setseed.dll`, and lets the overlay call `LoadLibraryA` on the companion.

The overlay does not reproduce or patch the Spiritism code. If the companion
cannot be loaded, the Garden hook is not installed.

## Town Click Hook

The town-screen dispatcher converts the clicked control into a building ID
and uses two compact jump tables. Decoding those tables from the reviewed
executable shows:

- Garden IDs `18` and `19` both dispatch to `0x005D38E1`.
- Dwelling IDs `30` through `43` dispatch to `0x005D42C4`.

The Garden handler itself reuses native creature-recruitment behavior. The
reviewed handler begins:

```text
0x005D38E1  8B 45 08 85 C0
```

The runtime verifies those five original bytes before replacing them with a
relative jump to an allocated stub. The stub checks:

- `[ebp+8] == 0`, the native left-click path
- `edi == 18 || edi == 19`, the Garden variants

The helper reads the current `H3Town*` from `[ebx+0x38]`, requires its town
type byte at offset `0x04` to equal Conflux ID `8`, then reads its built
building bitfield at offset `0x150`. Mask `0x000C0000` requires Garden bit
`18` or `19`.

Nonmatching calls replay the original five bytes and continue through native
behavior at `0x005D38E6`. The helper returns false, allowing native behavior,
only when the town is not Conflux or no Garden bit is built. Once the Garden
is present, the helper returns true and the stub jumps to native town-handler
cleanup at `0x005D4617`, even if the transformer table or dialog resources
are unavailable. Creature recruitment is therefore disabled for the
Garden control while the actual Magic Lantern dwelling remains unchanged.

The native Skeleton Transformer town function is:

```text
0x005D3390
```

Calling the native function preserves its modal UI, stack movement,
conversion timing, army updates, sounds, and 1:1 stack-count behavior.

## Conversion Target

The original game reads its Skeleton/Bone Dragon target mapping from the
table beginning at `0x0064130C`. HotA 1.8.0 expands that table:

1. `HotA.dll+0x6354BC` stores the configured creature count.
2. Initializer `HotA.dll+0x148140` allocates `count * 4` bytes.
3. HotA copies the original `0x244` bytes and initializes its added creature
   entries.
4. HotA redirects the native transformer lookup sites to the expanded table.
5. `HotA.dll+0x6354E4` stores the final expanded target-table pointer.

Version 0.1.0 attempted to resolve this table while the loader initialized.
The first live log showed that the table was not yet available, so the runtime
withheld its click hook. Version 0.1.1 defers table resolution until an
eligible Garden click, after the town interface and HotA runtime state are
initialized.

Versions 0.1.1 through 0.1.3 read `HotA.dll+0x6354CC`. Disassembly of the
initializer shows that offset belongs to a different expanded structure; the
transformer mapping is stored at `+0x6354E4` immediately before the
initializer returns. Version 0.1.4 uses that verified store.

The helper validates that the count includes Pixie/Sprite and does not exceed
its guarded capacity of 1024 entries. If validation fails after a Garden has
been identified, the click is consumed and the precise failure is appended to
the runtime log; it never falls back to recruitment. While a Garden dialog is
open, the helper:

1. Saves the complete expanded table.
2. Replaces native Bone Dragon `68` targets with Firebird `130`.
3. Replaces every other native target with Pixie `118`.
4. Calls the native transformer.
5. Restores every original target immediately after the modal dialog closes.

The table is never changed outside the Garden dialog. The Necropolis
transformer therefore retains Skeleton `56`, Bone Dragon `68`, and HotA's
added-creature handling.

## Dialog Text

The native transformer constructor reads four contiguous general-text
pointers through `0x006A5DC4`, table offset `0x20`, entries
`0x798` through `0x7A4`. During the Garden dialog only, the overlay changes
them to:

```text
Holding Area
Pixie Transformer
Move creatures to the Transformer to create Pixies or Firebirds
Creatures in the Transformer will become Pixies or Firebirds
```

The four original pointers are restored when the dialog closes.

## Growth Removal

H3API commit `92255ab18da784a5842ecc2b8bc0ce00e19a0c56` documents the
`H3CreatureInformation` layout used by this executable:

```text
record size = 0x74
town        = +0x00
level       = +0x04
hGrow       = +0x48
```

The creature-information pointer is read from `0x006747B0`. Before writing,
the runtime requires:

- Pixie `118`: town `8`, level `0`, `hGrow == 10`
- Sprite `119`: town `8`, level `0`, `hGrow == 10`

It then changes both `hGrow` fields to zero. If either validation or write
fails, the click hook is not installed and any partial growth write is
restored.

Base creature growth is not changed. Only the Garden horde bonus is removed.

## Building Text

Version 0.1.4 generated a loose `Data/BldgSpec.txt`, but the in-game town
screen continued to use the archived Garden name and `+10` description.
Version 0.1.5 therefore validates the exact English `Data/HotA_lng.lod`,
extracts `BldgSpec.txt` in memory, and requires exactly one copy of each
original Garden row. Both rows become:

```text
Garden of Life
The Garden of Life allows you to convert any creature into a Pixie.
```

Only the description changes; the town building name remains Garden of Life.
The modal conversion window is still titled Pixie Transformer.

Version 0.1.6 changes the description again while retaining the original
building name:

```text
Garden of Life
Converts creatures to Pixies or Firebirds.
```

The revised 0.1.6 text compresses to 2153 bytes, fitting inside the original
2155-byte archive slot. The installer overwrites that slot, zero-fills its
unused 2 bytes, and changes only the entry's uncompressed and compressed size
fields. Every LOD entry offset remains unchanged. It verifies the revised
entry after decompression and also installs matching loose text.

## Backup And Restore

Each apply records these paths under
`PixieTransformerPatch/backups/<timestamp>/`:

- `_HD3_Data/Common/setseed.dll`
- `_HD3_Data/Common/ConfluxSpiritismRuntime.dll`
- `_HD3_Data/Common/PixieTransformer.log`
- `Data/BldgSpec.txt`
- `Data/HotA_lng.lod`

The manifest stores whether each path originally existed and the SHA-256 of
every saved file. Restore verifies each backup hash and reinstates the exact
state saved by the selected backup. Older manifests predate language-archive
tracking; when one is selected from a later installation, restore retrieves
the verified original archive from the 0.1.5 checkpoint. Restoring backup
`20260819-212836` returns to version 0.1.1, backup `20260819-212254` returns to
version 0.1.0, and the original `20260819-211141` backup returns to the
pre-overlay Spiritism state.

## Version 0.1.0 Failure

The first in-game test clicked the Garden and opened its native associated
creature recruitment. Its runtime log recorded:

```text
HotA transformer target table=failed
Garden click hook=failed
final=one or more runtime operations failed
```

The two causes were independent:

1. The table lookup ran too early during loader initialization.
2. The table failure prevented the otherwise correct Garden handler hook from
   being installed.

Version 0.1.1 deferred the table lookup but incorrectly moved the hook away
from the Garden handler.

## Version 0.1.1 Failure

Runtime 2 reported that its growth and click hooks installed successfully,
but clicking the Garden still opened recruitment and no per-click diagnostic
was appended. Disassembly showed that the runtime stub compared
`[manager+0x110]` with Conflux ID `8`; that field is not the town type. The
native handler instead reads the current town from `[manager+0x38]` and its
type byte at town offset `0x04`.

Version 0.1.2 moves the town-type validation into the guarded C helper and
changes a built Garden from a conditional transformation attempt into a
consumed click, but that behavior remained attached to the wrong dispatch
branch.

## Version 0.1.2 Failure

Runtime 3 reported all startup hooks installed, but its log contained no
`Garden dialog` or `Garden click` line after the user clicked the Garden. This
proved the click never entered the hook at `0x005D42C4`.

Decoding the executable jump tables showed the earlier control-overlap model
was incorrect: `0x005D42C4` is the generic dwelling branch, while Garden IDs
`18/19` use `0x005D38E1`. Runtime 4 retains the deferred table resolution and
click-consuming helper, but installs it at the real Garden branch.

## Version 0.1.3 Failure

Runtime 4 received and consumed five Garden clicks. Every attempt appended:

```text
Garden click=consumed; transformer target table unavailable
```

The handler and town validation were therefore working. Tracing HotA's
initializer showed the runtime was reading the wrong global. The allocation
held in `esi` is written to `HotA.dll+0x6354E4` at `HotA.dll+0x14826C`;
version 0.1.4 reads that pointer and retains validation that its first entry is
Skeleton `56`.

## Version 0.1.6 Failure

Runtime 6 successfully preserved the native classification and reported 16
Bone Dragon-class entries, but their live result was `NOT USED (2)`. The
active HotA `CRTRAITS.TXT` confirms:

```text
creature 124 = NOT USED (2)
creature 130 = Firebird
```

Runtime 7 changes only the Firebird result constant from `124` to `130`.

## Validation State

Completed:

- Exact prerequisite checksum validation
- Python syntax and status checks
- 32-bit Windows DLL build using Zig 0.15.2
- `KERNEL32.dll`-only import audit
- Compiler calling-convention inspection for the town helper
- HotA expanded transformer-table trace
- Packaged/runtime byte comparison
- Fresh Spiritism-to-0.1.1 isolated install and byte-exact restore
- Exact 0.1.0-to-0.1.1 isolated upgrade and byte-exact restore
- Reproducible runtime 2 build
- Live guarded upgrade to version 0.1.1
- Runtime 2 in-game launch with all startup hooks installed
- Exact 0.1.1-to-0.1.2 isolated upgrade and byte-exact restore
- Runtime 3 `KERNEL32.dll`-only import audit
- Live guarded upgrade to version 0.1.2
- Runtime 3 in-game startup with no per-click event
- Town-screen low/high jump-table decoding
- Exact 0.1.2-to-0.1.3 isolated upgrade and byte-exact restore
- Runtime 4 `KERNEL32.dll`-only import audit
- Live guarded upgrade to version 0.1.3
- Runtime 4 in-game Garden clicks reaching the helper
- HotA transformer allocation, patch-site, and final-store trace
- Exact 0.1.3-to-0.1.4 isolated upgrade and byte-exact restore
- Runtime 5 `KERNEL32.dll`-only import audit
- Live guarded upgrade to version 0.1.4
- Runtime 5 Garden dialog and conversion launch in game
- Exact 0.1.4-to-0.1.5 isolated upgrade and byte-exact restore
- Revised LOD entry decompression and exact checksum verification
- Live guarded upgrade to version 0.1.5
- Reproducible runtime 6 build and `KERNEL32.dll`-only import audit
- Fresh Conflux Spiritism-to-0.1.6 isolated install
- Exact 0.1.5-to-0.1.6 isolated upgrade and byte-exact restore
- Direct 0.1.4-to-0.1.6 isolated upgrade
- Live guarded upgrade to version 0.1.6
- Runtime 6 live Pixie success and `NOT USED (2)` Firebird-class failure
- Active `CRTRAITS.TXT` verification of IDs `124` and `130`
- Reproducible runtime 7 build and `KERNEL32.dll`-only import audit
- Exact 0.1.6-to-0.1.7 isolated upgrade and byte-exact restore
- Live guarded upgrade to version 0.1.7
- Runtime 7 normal-class conversion to Pixie in game
- Runtime 7 native Bone Dragon-class conversion to Firebird in game
- Exact 0.1.8-to-0.1.9 companion upgrade and byte-exact restore
- Live 0.1.9 startup with Spiritism runtime 13 builder, refresh, event, and
  Hermit hooks installed
- Exact 0.1.9-to-0.1.10 companion upgrade path
- Spiritism runtime 16 repeated-launch validation and display-only Hermit
  scope
- Runtime 17 transfer icon confirmation and right-click crash reproduction
- Runtime 18 icon-only startup with exchange event and Hermit hooks disabled
- Runtime 19 five-launch validation and confirmed Spiritism transfer
  right-click popup
- Runtime 20 uncalled Hermit's Shack callback diagnosis
- Runtime 21 five-launch validation and confirmed Hermit's Shack Spiritism
  popup
- Runtime 22 five-launch validation and confirmed town visiting/garrison hero
  page using native 44x44 and 82x93 loaders

Pending:

- Unchanged Magic Lantern recruitment
- Weekly growth confirmation
- HotA-added creature conversion
- Unchanged Necropolis transformer and Spiritism gameplay checks
