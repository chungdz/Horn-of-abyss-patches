# Changelog

## 0.1.7 - 2026-08-20

- Recorded the live runtime-6 result: normal classes became Pixies, while all
  16 detected Bone Dragon classes became `NOT USED (2)`.
- Read HotA's active `CRTRAITS.TXT` and confirmed ID `124` is
  `NOT USED (2)` and Firebird is ID `130`.
- Corrected only the Firebird result ID from `124` to `130`.
- Published runtime marker 7 without changing the Garden text or other hooks.
- Verified reproducible DLL output and the exact 0.1.6-to-0.1.7 upgrade and
  byte-exact restore in an isolated fixture.
- Installed version 0.1.7 and validated both normal-to-Pixie and native Bone
  Dragon-class-to-Firebird conversion in game.

## 0.1.6 - 2026-08-20

- Preserved each creature's native Skeleton Transformer result before
  temporarily replacing the target table.
- Attempted to map native Bone Dragon `68` results to Firebird, but used
  unused creature ID `124`.
- Mapped every other native result to Pixie `118`.
- Preserved 1:1 stack counts and restored the complete native table after the
  modal dialog closes.
- Updated the Garden description and transformer dialog for both outputs.
- Published runtime marker 6 with exact runtime and language-resource hashes.
- Verified reproducible DLL output, the exact 0.1.5 upgrade and byte-exact
  restore, the direct 0.1.4 upgrade, and a fresh Conflux Spiritism install in
  isolated fixtures.
- Installed version 0.1.6; its live test produced Pixies for normal classes
  and `NOT USED (2)` for the 16 Bone Dragon classes.

## 0.1.5 - 2026-08-20

- Recorded that runtime 5 opens the Garden transformer dialog successfully.
- Kept both town building names as `Garden of Life`.
- Changed only their descriptions to explain conversion into a Pixie.
- Confirmed that HotA ignores the loose `Data/BldgSpec.txt` for this town
  display and loads the entry from `Data/HotA_lng.lod`.
- Recompressed the revised entry into its existing archive slot, updated only
  its uncompressed and compressed sizes, and preserved all archive offsets.
- Added the language archive to guarded status, backup, and restore handling.
- Verified the exact 0.1.4-to-0.1.5 upgrade and byte-exact restore in an
  isolated fixture.
- Installed version 0.1.5; its revised description remains pending in-game
  display validation.

## 0.1.4 - 2026-08-20

- Recorded five runtime 4 Garden clicks, each consumed at the correct handler
  but failing with `transformer target table unavailable`.
- Traced HotA's expanded transformer initializer at
  `HotA.dll+0x148140`.
- Confirmed the routine allocates `creature_count * 4` bytes, copies the
  original `0x244`-byte mapping, fills HotA-added entries, and patches five
  native consumers.
- Corrected the expanded target-table pointer from unrelated global
  `HotA.dll+0x6354CC` to the initializer's final store at
  `HotA.dll+0x6354E4`.
- Published runtime marker 5 and added a checksum-gated upgrade path from
  version 0.1.3.
- Verified the exact 0.1.3-to-0.1.4 upgrade and byte-exact restore in an
  isolated fixture.
- Installed version 0.1.4; runtime 5 subsequently opened the transformer
  dialog successfully in game.

## 0.1.3 - 2026-08-20

- Read the runtime 3 launch log after the failed click test; startup hooks
  installed, but no per-click diagnostic appeared.
- Decoded both town-screen jump tables from the reviewed executable.
- Confirmed Garden IDs `18` and `19` dispatch through `0x005D38E1`.
- Confirmed actual dwelling controls `30` through `43` dispatch separately
  through `0x005D42C4`.
- Moved the direct transformer hook to the real Garden handler and retained
  the click-consuming behavior.
- Left the real Magic Lantern dwelling and all other creature dwellings
  unchanged.
- Published runtime marker 4 and added a checksum-gated upgrade path from
  version 0.1.2.
- Verified the exact 0.1.2-to-0.1.3 upgrade and byte-exact restore in an
  isolated fixture.
- Installed version 0.1.3. Its Garden hook consumed clicks correctly, but the
  log reported that the transformer target table was unavailable.

## 0.1.2 - 2026-08-20

- Recorded that runtime 2 installed successfully but never called its Garden
  helper during the in-game click test.
- Corrected the Conflux check from invalid town-manager offset `0x110` to the
  current `H3Town` type byte at `[manager+0x38]+0x04`.
- Kept Magic Lantern control IDs `30` and `37` as the direct interception
  points.
- Changed the Garden handler contract so a built Garden always consumes the
  left-click; creature recruitment cannot open from that control.
- Preserved native recruitment only when the current town is not Conflux or
  does not have Garden building bit `18` or `19`.
- Added explicit runtime diagnostics for unavailable transformer tables,
  text resources, memory updates, and duplicate modal clicks.
- Published runtime marker 3 and added a checksum-gated upgrade path from
  versions 0.1.0 and 0.1.1.
- Verified the exact 0.1.1-to-0.1.2 upgrade and byte-exact restore in an
  isolated fixture.
- Installed version 0.1.2. Its startup hooks installed, but the Garden click
  never entered the separately hooked Magic Lantern dwelling branch.

## 0.1.1 - 2026-08-20

- Recorded the failed 0.1.0 in-game test: the visible Garden click opened
  Magic Lantern recruitment and the runtime log reported that its transformer
  table lookup and click hook had failed.
- Moved interception from the Horde 1 dispatch to the Magic Lantern dwelling
  handler used by the overlapping Conflux town control.
- Redirected Magic Lantern IDs `30` and `37` only when Garden building bit
  `18` or `19` is present in the current Conflux town.
- Preserved native Magic Lantern recruitment when the Garden is absent.
- Deferred HotA's expanded transformer-table lookup until the Garden click,
  after the town and HotA runtime state are initialized.
- Added guarded fallback to native recruitment whenever the Garden,
  transformer table, or helper prerequisites are unavailable.
- Published runtime marker 2 and added a checksum-gated upgrade path from
  version 0.1.0.
- Verified fresh installation, 0.1.0 upgrade, restoration, and reproducible
  runtime output in isolated game-state fixtures.
- Installed version 0.1.1. Its hook installed in game, but an invalid
  town-manager type offset caused every Garden click to fall through to
  Magic Lantern recruitment.

## 0.1.0 - 2026-08-20

- Replaced Conflux Garden of Life clicks with the native Skeleton Transformer
  interface.
- Scoped the native target table to Pixie `118` only while the Garden dialog
  is open.
- Used HotA's expanded transformer table so added creature IDs are included.
- Preserved stack counts for 1:1 conversion.
- Removed the Garden's former `+10` weekly Pixie/Sprite horde-growth bonus.
- Kept the Necropolis Skeleton Transformer unchanged.
- Added Pixie-specific dialog wording and generated building text.
- Preserved Conflux Spiritism 0.2.9 through a companion-runtime loader.
- Added exact-version checks, status reporting, timestamped backups, and
  restoration.
- Passed static and build validation, but failed the first in-game test:
  clicking the Garden opened Magic Lantern recruitment and the runtime log
  reported that its target-table lookup and click hook had failed.
