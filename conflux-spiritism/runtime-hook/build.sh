#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
zig="${ZIG:-/tmp/zig-x86_64-linux-0.15.2/zig}"
output_dir="$script_dir/build"
output="$output_dir/ConfluxSpiritismRuntime.dll"
object="$output_dir/runtime-spiritism.o"

if [[ ! -x "$zig" ]]; then
  printf 'Zig compiler not found: %s\n' "$zig" >&2
  exit 1
fi

: "${SOURCE_DATE_EPOCH:=1786757395}"
export SOURCE_DATE_EPOCH

mkdir -p "$output_dir"
ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-/tmp/zig-global-cache}" \
ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-/tmp/zig-local-cache}" \
"$zig" c++ \
  -target x86-windows-gnu \
  -Os \
  -fno-exceptions \
  -fno-rtti \
  -fno-stack-protector \
  -fno-builtin \
  -c \
  -o "$object" \
  "$script_dir/runtime-spiritism.cpp"

ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-/tmp/zig-global-cache}" \
ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-/tmp/zig-local-cache}" \
"$zig" c++ \
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
