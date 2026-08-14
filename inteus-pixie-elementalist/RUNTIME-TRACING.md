# Runtime Tracing Workflow

This record describes how the Nyx patch moved from static file guesses to a
minimal verified runtime fix. It is intended as a repeatable workflow for
future HotA 1.8.0 and HD Mod investigations.

## 1. Establish A Reversible Baseline

Before changing a binary or archive:

1. Confirm the exact HotA and HD Mod versions.
2. Record SHA-256 hashes for every candidate file.
3. Create a timestamped manifest backup before each write.
4. Guard the exact original and known prior-patch bytes.
5. Refuse unknown mixed states.

Do not infer a virtual address directly from a file offset. Use the PE section
table or disassembler mapping. Several early annotations were wrong because
the raw-file and virtual section offsets differ.

## 2. Separate Static Resources From Live Objects

Replacing `UN32.def` and `UN44.def` in sprite LODs and HD compatibility packs
did not change the standard scenario views. Redirecting resource strings also
did not help. This established that those dialogs were reusing already-loaded
DEF objects rather than constructing a new image from disk.

Negative results are useful evidence. Revert the unsuccessful write, record
the tested path, and move the investigation closer to the live object.

## 3. Find A Verified DLL Loader

HotA sets HD Mod flag `0x8`, disabling normal plugin-directory scanning. Tests
under normal plugin names and compatibility packs did not load.

HD Mod explicitly checks `_HD3_Data/Common/setseed.dll`. That optional slot
was unused in the tested installation and became the diagnostic loader.

Rules for using it:

- refuse to overwrite an unrelated DLL;
- build a 32-bit PE DLL with minimal imports;
- keep diagnostic hooks read-only;
- remove the diagnostic DLL after collecting the log;
- reinstall only the reviewed final runtime component.

## 4. Probe Exported HD State Read-Only

`runtime-hook/probe.c` loads through the verified slot and calls
`HdCommon_Get` from `_HD3_.dll`. It records:

- `HotA.HPL_tbl`
- `HotA.HPS_tbl`
- `HotA.PortraitsCount`
- `HotA.HeroesDefaultPortraits`

All table reads use `ReadProcessMemory`. The probe showed that hero 140 already
requested `HPL004el.PCX` and `HPS004el.PCX`, so rewriting portrait tables was
unnecessary. The working portrait solution was an HD-registered BMP override.

## 5. Trace Image Construction Without Changing It

`runtime-hook/image-trace.cpp` installs a Patcher_x86 low-level hook at the
game image constructor `0x004EA800`. For each `.def` request it records:

- calling thread and return address;
- object pointer;
- x, y, width, and height;
- frame number;
- resource string and pointer.

The hook returns `EXEC_DEFAULT` and never changes arguments or memory.

Reproducing both the scenario selector and entered hero screen produced no
matching constructor event. This proved those views bypassed the expected
construction path and invalidated the executable/DLL string redirects.

## 6. Inspect Loaded DEF Frame Tables

The next probe used the game's DEF loader at `0x0055C9C0` and validated these
32-bit layouts before reading or writing:

```text
H3LoadedDef      = 0x38 bytes
H3LoadedDefGroup = 0x0C bytes
H3LoadedDefFrame = 0x48 bytes
```

It loaded:

```text
UN32.def
IX32.def
UN44.def
IX44.def
```

The initial log showed distinct `UN` and `IX` DEF objects but identical frame
140 pointers. The copied replacement frames still had the internal names
`Un32Lust.PCX` and `Un44Lust.PCX`; Heroes III cached the individual frames by
those names and reused Bloodlust.

Renaming only the isolated frames to `NYX32PIX.PCX` and `NYX44PIX.PCX`
produced distinct pointers. That was the decisive cache diagnosis.

## 7. Keep The Runtime Write Minimal

`runtime-hook/runtime-fix.cpp` waits for startup resource initialization, then:

1. Loads each `UN` target and `IX` source through the game loader.
2. Validates the DEF, group, frame count, and frame 140 pointers.
3. Replaces only the target frame-table pointer for hero 140.
4. Reads the pointer back and verifies it.
5. Writes `NyxRuntimeFix.log`.

It does not patch instructions, global resource names, neighboring hero
frames, portrait tables, or dialog objects.

## 8. Trace Authoritative Data Sources

Correct executable creature IDs did not change the displayed army ranges.
The random-map UI still showed `15-25, 3-5, 3-5`.

Extracting `HOTRAITS.TXT` exposed that exact sequence on Nyx's row. Updating
the table to three `22-25 Pixies` entries changed both displayed and created
armies. This demonstrated an important rule: matching static structure bytes
do not prove that HotA uses them after initialization. Search text archives,
runtime tables, and module-owned copies when only part of a record changes.

## 9. Remove Diagnostic And Failed Paths

Once the working mechanism is confirmed:

- restore failed executable and DLL redirects to original bytes;
- remove diagnostic DLLs and their probe/trace logs;
- keep source code and written findings in the repository;
- retain only assets and runtime writes required by the confirmed mechanism;
- verify unchanged modules against stock hashes.

The final Nyx installation keeps `NyxRuntimeFix.log` for operational
verification. `NyxImageTrace.log` and `NyxRuntimeProbe.log` are removed.

## 10. Validation Checklist

For future runtime patches:

1. Test from a fully restarted process.
2. Reproduce every affected UI path.
3. Capture module paths and exact caller addresses.
4. Prefer read-only probes before memory writes.
5. Validate structure sizes and every pointer boundary.
6. Give replacement resources unique internal cache names.
7. Make the final write as narrow as possible.
8. Remove probes and failed experiments from the installed game.
9. Verify installer idempotence.
10. Test full original restoration and restoration of its safety backup.
