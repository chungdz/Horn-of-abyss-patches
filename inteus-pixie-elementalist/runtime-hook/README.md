# Nyx Runtime Probe

This directory contains a read-only diagnostic DLL for HotA 1.8.0 with
HD Mod 5.6 R16. It inspects the runtime portrait tables that bypass the
patched bitmap LOD files.

The probe:

- loads through HD Mod's optional `_HD3_Data/Common/setseed.dll` slot;
- calls the exported `_HD3_.dll` function `HdCommon_Get`;
- reads `HotA.HPL_tbl`, `HotA.HPS_tbl`, `HotA.PortraitsCount`, and
  `HotA.HeroesDefaultPortraits`;
- uses `ReadProcessMemory` rather than directly dereferencing table values;
- records hero entries 136 through 143 and up to 96 readable bytes behind
  each HPL/HPS entry;
- never writes to game memory.

## Build

The verified cross-compiler is Zig 0.15.2:

```bash
./build.sh
```

The output must be a PE32 Intel 80386 DLL:

```text
build/NyxRuntimeProbe.dll
```

## Run

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

Remove the diagnostic `setseed.dll` after collecting the log. Do not leave
it in the final patch.

HotA sets HD Mod executable flag `0x8`, which disables the normal `*.dll`
plugin-directory scans. Placing the probe in `#hota15`, under its own name in
`Common`, or in a normal plugin pack does not load it. HD Mod explicitly
loads `cursors.dll` and the optional `setseed.dll` from `Common` for HotA;
the latter is the only unused verified loader path in the tested install.

No runtime memory modification should be attempted until entry 140 and the
neighboring entries have a validated structure.
