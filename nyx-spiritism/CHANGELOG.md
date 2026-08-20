# Changelog

## 0.1.6 - 2026-08-20

- Changed Nyx's Spiritism result creature from Pixie `118` to Sprite `119`.
- Updated the Basic, Advanced, and Expert descriptions to name Sprites.
- Advanced the packaged runtime marker to runtime 9.
- Kept the existing percentages, artifacts, Amplifiers, UI hooks, icons,
  post-battle wording, and Nyx specialty handling unchanged.
- Retained checksum-gated upgrade support for reviewed earlier Spiritism
  runtimes.

## 0.1.5 - 2026-08-15

- Added `SPIR82.def`, a full `SecSk82.def` clone whose Basic, Advanced, and
  Expert Necromancy frames are replaced with brown-backed Spiritism art for
  the right-click skill detail dialog.
- Added a Nyx-only wrapper around the two `HD_HOTA.dll` pregame hero-panel
  builder call sites, so the panel displays Spiritism text and `SPIRIT.def`.
- Restores the HD resource-name literal immediately after Nyx's panel is
  constructed; combat and other hero panels retain native resources.
- Removed all runtime `patcher_x86` API calls after a startup race crashed
  inside `patcher_x86.dll` before the Spiritism log completed.
- Removed the now-unused bundled `patcher_x86.hpp` dependency.
- Chains the already-installed HotA/HD relative jumps directly after their
  target bytes remain stable for one second.
- Validates and atomically installs all five relative patches, rolling them
  back if any write or verification fails.
- Temporarily restored the live installation to the original Nyx runtime
  while the startup-safe runtime was built.
- Verified a clean HD launch, Nyx's pregame Spiritism label and icon, the
  brown-backed hero-page icon, the custom 82x93 skill-detail image and text,
  battle startup and completion, Pixie raising, and the Spiritism result
  message on HotA 1.8.0 with HD Mod 5.6 R16.

## 0.1.4 - 2026-08-15

- Removed the global `H3DlgDef` constructor hook after runtime 6 caused combat
  startup to crash.
- Temporarily changes the normal `Secskill.def` filename literal to
  `SPIRIT.def` only while Nyx's modal hero or level-up UI is active.
- Restores both the filename and skill-text aliases before the UI call returns,
  so battle construction never sees the custom definition.
- Covers both the hero-page tile and the right-click skill detail dialog
  through their native resource-loading paths.
- Reworded Spiritism descriptions without displaying Necromancy terminology.
- Added Nyx-scoped Spiritism post-battle plural and singular messages.
- Composites transparent icon pixels over the same brown `DiBoxBck.pcx`
  texture used by the Nyx specialty patch, removing the cyan background.
- Added checksum-gated upgrades for both the generated resource and runtime.

## 0.1.3 - 2026-08-15

- Replaced shared `Secskill.def` frame-table mutation with a typed high-level
  hook on the normal `H3DlgDef` constructor.
- Passes `SPIRIT.def` as the constructor filename only for Nyx's secondary
  skill controls and Spiritism level-up controls.
- Leaves the original loaded `Secskill.def`, its converted surfaces, and its
  frame ownership untouched.
- Restored the installed game to stable runtime v3 after the rejected v5 test
  crashed when opening a hero page.
- Rejected after the global constructor hook also intercepted battle dialog
  creation and crashed combat before it started.

## 0.1.2 - 2026-08-15

- This test-only revision kept Nyx's Spiritism text and shared icon-frame
  aliases active after the original dialog call returned.
- Replaced the two Necromancy General Text pointers before Nyx's post-battle
  result is formatted, so the message begins with Spiritism without patching
  executable instructions.
- Rejected after changing live `Secskill.def` frame pointers caused HotA's
  converted-surface renderer to crash when a hero page opened.
- Added checksum-gated runtime upgrades for existing v0.1.0 and v0.1.1
  Spiritism installations and the rejected test runtime.

## 0.1.1 - 2026-08-15

- Removed an overly strict runtime-prologue check that prevented all
  Spiritism hooks after HD Mod had already patched a target function.
- Let `patcher_x86` chain each hook against the live runtime code.
- Added live target-byte and per-hook installation diagnostics.

## 0.1.0 - 2026-08-15

- Added the Nyx-only Spiritism alias over Necromancy.
- Replaced Nyx's Basic Wisdom starting slot with Basic Spiritism.
- Kept Nyx's Basic Fire Magic starting skill unchanged.
- Changed Nyx's Necromancy result creature to Pixie.
- Preserved native Necromancy percentages, artifacts, Amplifiers, AI behavior,
  rounding, save data, and army handling.
- Added Nyx-scoped Spiritism text and Basic, Advanced, and Expert icons.
- Merged Spiritism hooks with the existing Nyx specialty-frame runtime DLL.
- Added reproducible icon conversion, apply, status, backup, and restore tools.
- Preserved the three supplied full-resolution source icons under `assets/`.
