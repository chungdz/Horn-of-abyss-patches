# Changelog

## 1.0.2 - 2026-08-14

- Patch `UN32.def` and `UN44.def` in both `Data/H3sprite.lod` and
  `Data/H3ab_spr.lod`, avoiding dependence on archive-resolution order.
- Support guarded in-place upgrades from the exact version 1.0.1 state.
- Record pre-apply file states so restore returns to either a clean
  installation or version 1.0.1 exactly.

## 1.0.1 - 2026-08-14

- Patch `UN32.def` and `UN44.def` in `Data/H3ab_spr.lod`.
- Add the 32x32 Pixie picture used by random-map starting-hero selection.
- Leave `Data/H3sprite.lod` unchanged.

## 1.0.0 - 2026-08-14

- Initial Inteus Pixie Elementalist patch for HotA 1.8.0.
