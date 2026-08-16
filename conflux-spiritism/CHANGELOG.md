# Changelog

## 0.2.7 - 2026-08-16

- Chained HotA's native raised-creature selector for English Spiritism heroes.
- Translated the Cloak of the Undead King's Basic Walking Dead result to Fire
  Elementals, Advanced Wight result to Earth Elementals, and Expert Lich result
  to Psychic Elementals.
- Kept no-Cloak Spiritism raising Pixies and left ordinary Necromancy
  unchanged.
- Kept the unresolved Chinese runtime on its existing Pixie-only behavior.
- Added guarded in-place upgrade support for the reviewed 0.2.6 runtime.
- Validated Pixie raising and all three custom Cloak results in game.
- Documented that HotA's Ring of Oblivion, artifact ID `158`, suppresses both
  Spiritism and ordinary Necromancy while equipped.
- Traced the apparent late-game generation failure across saves and confirmed
  it was serialized Ring equipment rather than a runtime hook failure.

## 0.2.6 - 2026-08-16

- Removed runtime 7's ineffective fixed-scenario selector hook.
- Kept the confirmed Nyx portrait aliases for standard, pregame, and
  supported exchange dialogs.
- Left HotA's loaded `UN32.def` and `UN44.def` atlases untouched.
- Documented the fixed-scenario Bloodlust image as a cosmetic limitation:
  the old repair depended on a truncated loose atlas that breaks HotA-added
  heroes, while raw frames are incompatible with the native converted atlas.
- Added in-place upgrade support for the reviewed 0.2.5 runtime.

## 0.2.5 - 2026-08-16

- Attempted to fix Nyx's Bloodlust specialty portrait by replacing the
  selector's five-byte `UN32.def` resource push with an exact-site-validated
  hook.
- In-game testing later confirmed this did not work because the scenario view
  bypasses that resource-construction path.
- Added runtime log, installer status, rollback verification, and in-place
  upgrade support for the reviewed 0.2.4 runtime.

## 0.2.4 - 2026-08-16

- Restored Nyx's Pixie specialty portrait through scoped `IX44.def` and
  `IX32.def` resource-name aliases instead of modifying HotA's shared
  specialty atlas, native frame objects, or renderer pixel buffers.
- Added `SPIR32.def`, replacing Necromancy frames 39-41 in the native
  32x32 secondary-skill atlas with the three Spiritism icons.
- Hooked the HD SwapMgr builder used while heroes transfer creatures and
  artifacts, chaining its live thiscall-compatible target and inspecting the
  existing two-hero array.
- Scoped the exchange Spiritism icon only when no ordinary Necromancer is
  displayed in the same dialog.
- Scoped Nyx's exchange portrait only when both hero IDs fit the 156-frame
  `IX32.def`, preserving HotA's extended atlas for later heroes.
- Added installer registration, checksum verification, backup, restore,
  status, and in-place upgrade support for the reviewed 0.2.3 runtime.

## 0.2.3 - 2026-08-16

- Removed every runtime write to the shared `UN32.def` and `UN44.def`
  specialty atlases.
- Disabled the custom Nyx Pixie specialty portrait after runtime 4 showed
  that its renderer pixel buffer was shared with Akka and other heroes,
  producing blue Pixie images outside Nyx.
- Added a second exact-byte-validated null guard for the duplicate hero
  inspection handler at `HotA.dll+0x3F2D`; the first guarded handler remains
  at `HotA.dll+0x2E5D`.
- Fixed crashes while cycling tavern heroes and while swapping and inspecting
  visiting/garrison Conflux heroes.
- Preserved the Spiritism skill icons, descriptions, Pixie raising,
  10%/20%/30% rates, and ordinary non-Conflux Necromancy.
- Verified runtime 5 in game: Akka and HotA-added specialty images recovered,
  repeated Conflux hero switching did not crash, and all runtime hooks
  reported installed.
- Recorded the custom Nyx specialty portrait as currently uninstalled. The
  existing `IX32.def` and `IX44.def` files are inactive compatibility
  leftovers and are not used by runtime 5.
- Added in-place upgrade support for the reviewed 0.2.2 runtime.

## 0.2.2 - 2026-08-16

- Replaced the unsafe Nyx `UN32`/`UN44` frame-pointer swap with an in-place
  update of HotA's renderer-owned `D32P` pixel buffers.
- Avoided the raw-frame object-layout crash at `HotA.dll+0x7978A`, but was
  later rejected because the target pixel buffer was shared with other hero
  specialty views.
- Added exact validation for native dimensions, stride, four-byte pixel
  format, renderer pointer, indexed source layout, and loaded palette.
- Added in-place upgrade support for the reviewed 0.2.1 runtime.
- Documented the live process-memory comparison that identified the raw
  `DefFrame` versus native `D32P` object mismatch.

## 0.2.1 - 2026-08-16

- Removed the obsolete 156-frame `UN32.def` and `UN44.def` compatibility
  overrides that caused all HotA-added heroes to display incorrect specialty
  pictures.
- Kept Nyx's specialty image replacement scoped to frame 140 at runtime.
- Added a startup check that Vehr's extended specialty frame 212 is
  available.
- Added a checksum-validated null guard for the HotA 1.8.0 Invite-a-Hero
  tavern handler after the crash log showed `HotA.dll+0x2E63` dereferencing a
  null current-hero pointer.
- Added in-place upgrade support for the reviewed 0.2.0 runtime.
- Expanded rollback backups to include both HD `Files.ini` files and all four
  removed atlas overrides.
- Verified an isolated 0.2.0 apply/status/restore round trip with exact
  restoration of every affected checksum.

## 0.2.0 - 2026-08-15

- Increased the Conflux-only Spiritism base rates from 5%/10%/15% to
  10%/20%/30%.
- Added a direct `H3Hero::GetNecromancyPower` hook that chains HotA's complete
  calculation, then adds 5% per Spiritism level.
- Preserved HotA's artifact, Necromancy Amplifier, specialty, rounding, AI,
  and optional 100% cap behavior.
- Kept ordinary Necromancy at HotA's native 5%/10%/15% rates.
- Updated the Basic, Advanced, and Expert Spiritism descriptions to display
  10%, 20%, and 30%.
- Added in-place installer support for the reviewed 0.1.0 runtime.
- Added deterministic rate-test instructions using 120 Peasants.
- Verified the live Basic rate: 28 Sprites at 3 health each raised 2 Pixies,
  matching `floor(28 * 3 * 0.10 / 3)`.
- Documented the native per-creature health cap after an Expert test raised
  16 Pixies from 56 Hobgoblins:
  `floor(56 * min(5, 3) * 0.30 / 3)`.

## 0.1.0 - 2026-08-15

- Added a separate Conflux-wide upgrade over Nyx Spiritism 0.1.5.
- Replaced the first secondary-skill type for Conflux heroes `128–143` with
  internal Necromancy, presented as Spiritism.
- Preserved every first-skill level and every second skill.
- Gave Lacus and Fiur Advanced Spiritism by preserving their original
  Advanced first-skill levels.
- Expanded Pixie raising, Spiritism text, both icon sizes, level-up dialogs,
  standard hero dialogs, and HD pregame panels to all Conflux heroes.
- Kept normal Necromancy behavior unchanged for heroes outside the Conflux.
- Retained Nyx's runtime specialty-picture repair.
- Added strict prerequisite checks, executable output checksums, status
  reporting, timestamped backups, and restoration to the Nyx-only patch.
- Added the dedicated `ConfluxSpiritism.log` runtime report.
- Verified a complete apply, status, and restore round trip against an
  isolated copy of the accepted Nyx Spiritism 0.1.5 installation.
- Verified the live installation in game with all runtime hook categories
  installed and the Conflux-wide behavior working.
