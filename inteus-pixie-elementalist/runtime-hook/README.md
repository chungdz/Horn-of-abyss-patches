# Nyx Runtime Tools

This directory contains the diagnostic and final runtime components used for
HotA 1.8.0 with HD Mod 5.6 R16.

## Final Runtime Fix

`runtime-fix.cpp` loads `UN32.def`, `IX32.def`, `UN44.def`, and `IX44.def`
after startup. It replaces only hero frame 140 in the two loaded `UN` frame
tables with the corresponding uniquely named `IX` frames. This reaches the
standard scenario selector and the entered scenario's hero screen, both of
which bypassed disk-resource and image-constructor redirections.

Build it with:

```bash
./build-runtime-fix.sh
```

The installer deploys the reviewed build as:

```text
_HD3_Data/Common/setseed.dll
```

It writes `NyxRuntimeFix.log` beside the DLL. The finalizer refuses to replace
an unrelated DLL already using this optional loader slot.

## Diagnostic Probe

The read-only portrait probe:

- loads through HD Mod's optional `_HD3_Data/Common/setseed.dll` slot;
- calls the exported `_HD3_.dll` function `HdCommon_Get`;
- reads `HotA.HPL_tbl`, `HotA.HPS_tbl`, `HotA.PortraitsCount`, and
  `HotA.HeroesDefaultPortraits`;
- uses `ReadProcessMemory` rather than directly dereferencing table values;
- records hero entries 136 through 143 and up to 96 readable bytes behind
  each HPL/HPS entry;
- never writes to game memory.

Build it with:

The verified cross-compiler is Zig 0.15.2:

```bash
./build.sh
```

The output must be a PE32 Intel 80386 DLL:

```text
build/NyxRuntimeProbe.dll
```

## Running A Diagnostic

First verify that the installation does not already have a `setseed.dll`.
Then copy the probe using the loader-recognized name:

```text
_HD3_Data/Common/setseed.dll
```

Fully exit HotA, start `h3hota HD.exe`, reach the main menu, and exit. The
probe writes:

```text
_HD3_Data/Common/NyxRuntimeProbe.log
```

Remove a diagnostic `setseed.dll` after collecting its log, then reinstall
the final runtime fix.

HotA sets HD Mod executable flag `0x8`, which disables the normal `*.dll`
plugin-directory scans. Placing the probe in `#hota15`, under its own name in
`Common`, or in a normal plugin pack does not load it. HD Mod explicitly
loads `cursors.dll` and the optional `setseed.dll` from `Common` for HotA;
the latter is the only unused verified loader path in the tested install.

`image-trace.cpp` is the read-only constructor trace used to prove the
standard specialty views do not call the expected `0x004EA800` constructor.
It builds with `build-image-trace.sh`.

The complete investigation sequence and lessons for future patches are in
[`../RUNTIME-TRACING.md`](../RUNTIME-TRACING.md).
