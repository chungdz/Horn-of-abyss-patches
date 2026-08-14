#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
zig="${ZIG:-/tmp/zig-x86_64-linux-0.15.2/zig}"
output_dir="$script_dir/build"
output="$output_dir/NyxRuntimeProbe.dll"
object="$output_dir/probe.o"

if [[ ! -x "$zig" ]]; then
  printf 'Zig compiler not found: %s\n' "$zig" >&2
  exit 1
fi

mkdir -p "$output_dir"
ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-/tmp/zig-global-cache}" \
ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-/tmp/zig-local-cache}" \
"$zig" cc \
  -target x86-windows-gnu \
  -Os \
  -fno-stack-protector \
  -fno-builtin \
  -c \
  -o "$object" \
  "$script_dir/probe.c"

ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-/tmp/zig-global-cache}" \
ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-/tmp/zig-local-cache}" \
"$zig" cc \
  -target x86-windows-gnu \
  -shared \
  -nostdlib \
  -Wl,--entry,DllMain@12 \
  -Wl,--subsystem,windows \
  -Wl,--strip-all \
  -o "$output" \
  "$object" \
  -lkernel32

file "$output"
objdump -p "$output" |
  sed -n '/The Import Tables/,$ { /PE File Base Relocations/q; p; }'
