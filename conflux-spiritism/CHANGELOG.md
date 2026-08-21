# Changelog

## Unreleased - 2026-08-21

- Added runtime 19 with two narrow transfer right-click call-site hooks.
  The clicked skill is identified at `0x005B0342`; the popup call at
  `0x005B0863` temporarily changes only the `secsk82.def` filename and
  message pointer while HotA's native loader converts `SPIR82.def`.
- Confirmed the persistent transfer icon and Spiritism right-click name and
  large image in game. The runtime log recorded five successful
  `Spiritism native-loader scope` activations.
- Completed five consecutive normal launches without changing the latest
  crash log. Hermit's Shack remains disabled for separate validation.
- Confirmed in game that runtime 17 preserves the 32x32 Spiritism icon after
  opening the hero-transfer dialog.
- Withdrew runtime 17's right-click scope after it crashed at
  `HotA.dll+0x7978A`. The scope inserted raw `SPIR82.def` frame objects into
  HotA's converted native frame table, reproducing the previously documented
  raw-DEF-versus-renderer-object mismatch.
- Added runtime 18 as a transfer-icon-only candidate. It retains the working
  per-slot `SPIR32.def` construction and installs no exchange event hook.
  Transfer right-click therefore remains native until its popup construction
  path can select `SPIR82.def` before conversion.
- Confirmed runtime 18 in game: the transfer icon remains Spiritism and the
  native Necromancy right-click popup opens without crashing.
- Added transfer-only runtime 17 with the Hermit's Shack hook disabled.
- Removed every permanent 87-to-93 secondary-skill group replacement.
  Runtime 17 keeps HotA's native 93-frame groups and applies only bounded
  three-frame overlays for the existing 44x44 and 82x93 UI calls.
- Rebuilt `SPIR32.def` from HotA 1.8.0's native `SecSk32.def` with unique
  internal frame names `SP32BAS.PCX`, `SP32ADV.PCX`, and `SP32EXP.PCX`.
  The pixels are byte-identical to the prior 32x32 icons.
- Identified the previous transfer-atlas failure as a resource-cache name
  collision. Its serialized frames were 32x32, but their reused
  `SPIRBAS.PCX`, `SPIRADV.PCX`, and `SPIREXP.PCX` names resolved to the
  already loaded 44x44 objects from `SPIRIT.def`.
- Added `build_exchange_resource.py`, which rebuilds the atlas from the
  reviewed native resource and refuses colliding names or non-32x32 output.
- Installed the corrected transfer atlas and completed ten consecutive normal
  startup and clean-close cycles without changing the latest crash log.
- Withdrew runtime 16 after a later ordinary launch crashed in HotA's
  resource loader at `HotA.dll+0x205E34`. No exchange or Hermit interaction
  scope had run, so three clean launches were not sufficient validation.
- Restored the live Pixie Transformer companion to reviewed runtime 13,
  SHA-256
  `dceddce37d411022967deec8f401c5d8bbceb526c0f7e39d83ff6dc37be28a5c`.
- Recorded the permanent 87-to-93 `DefGroup` replacement as prohibited. It
  changed HotA-owned group metadata and redirected the frame table to static
  DLL storage, violating the resource manager's ownership assumptions.
- Recorded persistent modal resource aliases and broad cleanup/destructor
  hooks as prohibited after runtime 14 corrupted memory before any exchange
  interaction.
- Recorded `0x005AE900` as an incorrect exchange-image hook. Disassembly shows
  that it updates dialog state but does not construct the 32x32 skill-image
  controls.
- Split recovery into separately verified changes: transfer image and
  right-click behavior first, then Hermit's Shack in a later candidate.
- Required interaction evidence in addition to startup hook markers. A log
  saying that a hook was installed is not proof that its predicate or UI path
  executed.

## 0.3.3 - 2026-08-20

- Withdrawn. Runtime 16 later reproduced resource-loader memory corruption at
  `HotA.dll+0x205E34`, despite passing three consecutive startup tests.
- Withdrew runtime 14 and the runtime 15 release candidate after they caused
  earlier repeat-launch memory corruption in HotA's resource-copy path.
- Split the Hermit's Shack callback into its native display and apply phases.
  Spiritism aliases now cover only the display phase that reads the skill
  name and image; the skill-application phase always runs with native
  resources.
- Fixed failed UI alias setup leaving the nesting depth nonzero and silently
  disabling later Spiritism substitutions.
- Made each exchange skill control load `SPIR32.def` or `secsk32.def` from an
  immutable per-side constructor operand. The reviewed operands are restored
  immediately after construction, while the controls retain their loaded DEF
  objects through later transfer refreshes.
- Kept the exchange event scope only for right-click controls `200-215`,
  selecting the corresponding hero before replacing Spiritism text and the
  large skill atlas.
- Advanced the packaged marker to runtime 16.

## 0.3.2 - 2026-08-20

- Withdrawn: the persistent exchange lifetime and broad Hermit callback scope
  caused launch-time memory corruption and did not fix the reported UI paths.
- Replaced the ineffective exchange refresh/event scopes from 0.3.1 with one
  lifetime scope from SwapMgr construction at `0x005AAD90` through cleanup at
  `0x005AF0B0`.
- Kept Spiritism text, `SPIR32.def`, and `SPIR82.def` active for all exchange
  redraws and right-click details, then restored the native aliases when the
  modal exchange closes.
- Removed the unnecessary pre-upgrade mastery check from the Hermit's Shack
  wrapper. The callback now scopes Spiritism from the verified hero ID and
  selected skill ID `12`.
- Added interaction diagnostics for exchange scope open/close and Hermit
  skill-ID-12 decisions.
- Advanced the packaged marker to runtime 14.

## 0.3.1 - 2026-08-20

- Attempted to extend the HD exchange fix beyond initial dialog construction.
- Scoped `SPIR32.def` during every SwapMgr refresh at `0x005AE900`, preserving
  the Spiritism icon after creature and artifact transfers.
- Scoped Spiritism text and `SPIR82.def` through the exchange event handler at
  `0x005B0100`, fixing right-click skill details.
- Hooked HotA's Hermit's Shack skill-upgrade callback at English RVA
  `0x17A920` and Chinese R10 RVA `0x17B140`.
- Limited the Hermit alias to skill ID `12` on a Conflux Spiritist, leaving
  ordinary Necromancy unchanged.
- Added validated entry trampolines that chain an existing HD hook or require
  the reviewed six-byte native prologue.
- Advanced the packaged marker to runtime 13.
- Verified all hooks in a live minimized startup and byte-exact restoration
  for standalone and Pixie companion upgrades.
- In-game testing showed that the internal refresh/event scopes did not cover
  the complete exchange lifetime and that the Hermit predicate was too
  restrictive. Version 0.3.2 supersedes them.

## 0.3.0 - 2026-08-20

- Identified HotA 1.8.0's two added secondary skills as Interference ID `28`
  and Runes ID `29`.
- Fixed missing Interference/Runes images and the resulting level-up crash.
- Extended the loaded `SPIRIT.def`, `SPIR82.def`, and `SPIR32.def` groups from
  87 to 93 frames by preserving the custom Spiritism frames and borrowing
  HotA's six native added-skill frame objects.
- Validated all six added frames and all three atlas dimensions before
  enabling any Spiritism resource alias.
- Made every alias fail closed to the native skill atlas if HotA's expected
  93-frame layout is unavailable.
- Deferred resource loading until after hook installation to avoid startup
  contention with the Pixie Transformer loader.
- Added companion-runtime detection so status, backup, upgrade, and restore
  target `ConfluxSpiritismRuntime.dll` when Pixie Transformer owns
  `setseed.dll`.
- Advanced the packaged marker to runtime 12.

## 0.2.9 - 2026-08-20

- Limited default Sprite `119` raising to Nyx.
- Restored default Pixie `118` raising for the other fifteen Conflux heroes.
- Updated all three English Spiritism descriptions to explain the Nyx
  exception.
- Preserved the 10%/20%/30% rates and the existing Cloak mapping to Fire,
  Earth, and Psychic Elementals for every Conflux Spiritist.
- Left ordinary Necromancy and the unresolved Chinese Pixie-only runtime
  unchanged.
- Advanced the packaged runtime marker to runtime 11.
- Added guarded in-place upgrade support for the reviewed 0.2.8 runtime.
- Confirmed runtime 11 installs every gameplay and UI hook on launch.

## 0.2.8 - 2026-08-20

- Changed the default English Spiritism result creature from Pixie `118` to
  Sprite `119`.
- Updated all three English Spiritism descriptions to name Sprites.
- Preserved the 10%/20%/30% rates and the existing Cloak mapping to Fire,
  Earth, and Psychic Elementals.
- Left ordinary Necromancy and the unresolved Chinese Pixie-only runtime
  unchanged.
- Advanced the packaged runtime marker to runtime 10.
- Added guarded in-place upgrade support for the reviewed 0.2.7 runtime and
  both Nyx Spiritism 0.1.5 and 0.1.6 prerequisites.
- Validated Sprite raising in game with runtime 10.

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
