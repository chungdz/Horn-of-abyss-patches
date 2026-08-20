# Technical Record

## Target

- Game: Horn of the Abyss 1.8.0
- HD Mod: 5.6 R16
- Patch version: 0.1.5
- Runtime marker: `Pixie Transformer runtime 5`
- Prerequisite: Conflux Spiritism 0.2.9, runtime 11
- Town: Conflux, ID `8`
- Garden controls and built bits: Horde 1 IDs `18` and `19`
- Actual Magic Lantern dwelling controls: IDs `30` and `37`
- Conversion target: Pixie, creature ID `118`

Reviewed prerequisite hashes:

| File | SHA-256 |
| --- | --- |
| `h3hota.exe` | `7aeb166c1976d87dd7b7ba43de033ec986bb4ee7d463816213f5d0a9afff7580` |
| `h3hota HD.exe` | `110122278fb9a2ac66d39b5243d00561c6725fbda54bf23cf41c034baab6c080` |
| `HotA.dll` | `e97aa25df70bc32c0cd5af20acec22207e86b13eb27fa5b705a102d5ef53fcec` |
| Original `Data/HotA_lng.lod` | `748b54cfac02ffc795f4b0c48c7cf6ef41ea0a6020f3cf41766271bd12eb81e9` |
| Description-only `Data/HotA_lng.lod` | `750a3384ad1bef990ec723154731ca24482e44f7f1f3390330c34c8fc89f162d` |
| Spiritism `setseed.dll` | `67c071790536f4186df0b348f59a7ce06b176168442d56454be7e96dde8507fd` |
| Pixie Transformer 0.1.0 runtime | `8956f877bf50ea63338230e956438bc8a8f8c15ea2ee5ad64a91690ea6b22b6f` |
| Pixie Transformer 0.1.1 runtime | `ab7ba9cf873fd60a33c1ac8243591b1407d00b027d8885d808ef10e75e4ed336` |
| Pixie Transformer 0.1.2 runtime | `e8f2268d696761bb9ddf9a5c0b9fc1dd53f2370d5cb87beb4e863f333e7cafc3` |
| Pixie Transformer 0.1.3 runtime | `92c03e938cdd9d1354eb65146db15c286403ba50a2632445c24cd45bd07f73d8` |
| Pixie Transformer 0.1.4 runtime | `26d84b9c76d59bd5988d390ce020a0c53b0778fb7db8831fbcb25244907a5a45` |

## Live Installed State

Version 0.1.5 is installed in the live game directory. Static status verifies:

- Pixie Transformer `setseed.dll`:
  `26d84b9c76d59bd5988d390ce020a0c53b0778fb7db8831fbcb25244907a5a45`
- Spiritism companion:
  `67c071790536f4186df0b348f59a7ce06b176168442d56454be7e96dde8507fd`
- Description-only `Data/BldgSpec.txt`:
  `5c077d1592862dc5172eeab2ad9177aa4eefe13abbf1f052e4106bc0ba58b402`
- Description-only `Data/HotA_lng.lod`:
  `750a3384ad1bef990ec723154731ca24482e44f7f1f3390330c34c8fc89f162d`
- Reviewed executables and `HotA.dll`
- Complete static patch state

Backup `PixieTransformerPatch/backups/20260819-222310` contains the exact
prior version 0.1.4 runtime, successful runtime log, Spiritism companion,
renamed loose building text, and original language archive.

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
2. Replaces every target with Pixie `118`.
3. Calls the native transformer.
4. Restores every original target immediately after the modal dialog closes.

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
Move creatures to the Transformer to create Pixies
Creatures in the Transformer will become Pixies
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

The revised text compresses to 2142 bytes, fitting inside the original
2155-byte archive slot. The installer overwrites that slot, zero-fills its
unused 13 bytes, and changes only the entry's uncompressed and compressed
size fields. Every LOD entry offset remains unchanged. It verifies the
revised entry after decompression and also installs matching loose text.

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
tracking; when one is selected from a 0.1.5 installation, restore retrieves
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

Pending:

- Version 0.1.5 Garden description display after a fresh launch
- Unchanged Magic Lantern recruitment
- Weekly growth confirmation
- HotA-added creature conversion
- Unchanged Necropolis transformer and Spiritism gameplay checks
