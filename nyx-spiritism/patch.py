#!/usr/bin/env python3

import argparse
import hashlib
import json
import math
import re
import shutil
import struct
import sys
import zlib
from datetime import datetime
from pathlib import Path


HERO_ID = 140
HERO_RECORD_OFFSET = 0x27D020
REPLACED_SKILL_TYPE_OFFSET = HERO_RECORD_OFFSET + 0x0C
WISDOM = 7
NECROMANCY = 12
NECROMANCY_ICON_FRAMES = (39, 40, 41)
RESOURCE_NAME = "SPIRIT.def"
SECSKILL_HASH = (
    "298f31e75e045fcb1195d870efbed8d7f5ecb81bab18e0ffc89ccc6a81c91aee"
)
NYX_RUNTIME_HASH = (
    "be7fb2e8a715b3abaa80eee4d6f24b6e19279c5c1dd9c624f620be396b3dab2d"
)
NYX_EXECUTABLE_HASHES = {
    "h3hota.exe": (
        "5ceb6ffb0d6517e361582ad2c6662a18132ecb706a2dd4776e1d489b02f68bcb"
    ),
    "h3hota HD.exe": (
        "38d85cfa3a5c05e815ba7613f9a414ce59926cdc17c8f44efbff6e98925d018c"
    ),
}

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_GAME_DIR = SCRIPT_DIR.parent.parent
RUNTIME_ASSET = SCRIPT_DIR / "assets" / "NyxSpiritismRuntime.dll"
ICON_ASSETS = (
    (
        SCRIPT_DIR / "assets" / "basic-spiritism.png",
        "SPIRBAS.PCX",
    ),
    (
        SCRIPT_DIR / "assets" / "advanced-spiritism.png",
        "SPIRADV.PCX",
    ),
    (
        SCRIPT_DIR / "assets" / "expert-spiritism.png",
        "SPIREXP.PCX",
    ),
)
PATCH_FILES = (
    "h3hota.exe",
    "h3hota HD.exe",
    "Data/SPIRIT.def",
    "_HD3_Data/Compability/#hota/SPIRIT.def",
    "_HD3_Data/Compability/#hota15/SPIRIT.def",
    "_HD3_Data/Compability/#hota/Files.ini",
    "_HD3_Data/Compability/#hota15/Files.ini",
    "_HD3_Data/Common/setseed.dll",
    "_HD3_Data/Common/NyxSpiritism.log",
)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def read_required(path):
    if not path.is_file():
        raise RuntimeError(f"Missing required file: {path}")
    return path.read_bytes()


def assert_hota_180(game_dir):
    setup = read_required(game_dir / "HotA_Setup.ini").decode(
        "latin1", errors="replace"
    )
    if re.search(r"Main Version\s*=\s*1\.8\.0\b", setup) is None:
        raise RuntimeError("This patch supports Horn of the Abyss 1.8.0 only.")


def png_chunks(data):
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise RuntimeError("Invalid PNG signature.")
    offset = 8
    while offset < len(data):
        if offset + 12 > len(data):
            raise RuntimeError("Truncated PNG chunk.")
        size = struct.unpack_from(">I", data, offset)[0]
        kind = data[offset + 4 : offset + 8]
        start = offset + 8
        end = start + size
        if end + 4 > len(data):
            raise RuntimeError("Truncated PNG payload.")
        payload = data[start:end]
        expected_crc = struct.unpack_from(">I", data, end)[0]
        actual_crc = zlib.crc32(kind)
        actual_crc = zlib.crc32(payload, actual_crc) & 0xFFFFFFFF
        if actual_crc != expected_crc:
            raise RuntimeError(f"PNG CRC mismatch in {kind!r}.")
        yield kind, payload
        offset = end + 4
        if kind == b"IEND":
            return
    raise RuntimeError("PNG IEND chunk is missing.")


def paeth(left, above, upper_left):
    estimate = left + above - upper_left
    left_distance = abs(estimate - left)
    above_distance = abs(estimate - above)
    upper_left_distance = abs(estimate - upper_left)
    if left_distance <= above_distance and left_distance <= upper_left_distance:
        return left
    if above_distance <= upper_left_distance:
        return above
    return upper_left


def decode_png_rgba(path):
    width = height = None
    compressed = bytearray()
    for kind, payload in png_chunks(read_required(path)):
        if kind == b"IHDR":
            if len(payload) != 13:
                raise RuntimeError(f"Invalid IHDR in {path}.")
            (
                width,
                height,
                bit_depth,
                color_type,
                compression,
                filter_method,
                interlace,
            ) = struct.unpack(">IIBBBBB", payload)
            if (
                bit_depth != 8
                or color_type != 6
                or compression != 0
                or filter_method != 0
                or interlace != 0
            ):
                raise RuntimeError(
                    f"{path.name} must be a non-interlaced 8-bit RGBA PNG."
                )
        elif kind == b"IDAT":
            compressed.extend(payload)

    if width is None or height is None:
        raise RuntimeError(f"PNG dimensions are missing in {path}.")
    scanlines = zlib.decompress(bytes(compressed))
    stride = width * 4
    expected = height * (stride + 1)
    if len(scanlines) != expected:
        raise RuntimeError(f"Unexpected decompressed PNG size in {path}.")

    pixels = bytearray(width * height * 4)
    source_offset = 0
    previous = bytearray(stride)
    for row in range(height):
        filter_type = scanlines[source_offset]
        source_offset += 1
        current = bytearray(scanlines[source_offset : source_offset + stride])
        source_offset += stride
        for column in range(stride):
            left = current[column - 4] if column >= 4 else 0
            above = previous[column]
            upper_left = previous[column - 4] if column >= 4 else 0
            if filter_type == 1:
                current[column] = (current[column] + left) & 0xFF
            elif filter_type == 2:
                current[column] = (current[column] + above) & 0xFF
            elif filter_type == 3:
                current[column] = (
                    current[column] + ((left + above) // 2)
                ) & 0xFF
            elif filter_type == 4:
                current[column] = (
                    current[column] + paeth(left, above, upper_left)
                ) & 0xFF
            elif filter_type != 0:
                raise RuntimeError(
                    f"Unsupported PNG filter {filter_type} in {path}."
                )
        start = row * stride
        pixels[start : start + stride] = current
        previous = current
    return width, height, bytes(pixels)


def resize_rgba_area(width, height, pixels, target_width=44, target_height=44):
    result = bytearray(target_width * target_height * 4)
    x_scale = width / target_width
    y_scale = height / target_height

    for target_y in range(target_height):
        source_y0 = target_y * y_scale
        source_y1 = (target_y + 1) * y_scale
        first_y = math.floor(source_y0)
        last_y = math.ceil(source_y1)
        for target_x in range(target_width):
            source_x0 = target_x * x_scale
            source_x1 = (target_x + 1) * x_scale
            first_x = math.floor(source_x0)
            last_x = math.ceil(source_x1)
            alpha_sum = 0.0
            red_sum = green_sum = blue_sum = 0.0
            area = (source_x1 - source_x0) * (source_y1 - source_y0)

            for source_y in range(first_y, last_y):
                if source_y < 0 or source_y >= height:
                    continue
                y_weight = max(
                    0.0,
                    min(source_y + 1, source_y1) - max(source_y, source_y0),
                )
                for source_x in range(first_x, last_x):
                    if source_x < 0 or source_x >= width:
                        continue
                    x_weight = max(
                        0.0,
                        min(source_x + 1, source_x1) - max(source_x, source_x0),
                    )
                    weight = x_weight * y_weight
                    source = (source_y * width + source_x) * 4
                    alpha = pixels[source + 3] / 255.0
                    weighted_alpha = weight * alpha
                    alpha_sum += weighted_alpha
                    red_sum += pixels[source] * weighted_alpha
                    green_sum += pixels[source + 1] * weighted_alpha
                    blue_sum += pixels[source + 2] * weighted_alpha

            target = (target_y * target_width + target_x) * 4
            if alpha_sum > 0:
                result[target] = round(red_sum / alpha_sum)
                result[target + 1] = round(green_sum / alpha_sum)
                result[target + 2] = round(blue_sum / alpha_sum)
            result[target + 3] = round(255.0 * alpha_sum / area)
    return bytes(result)


def find_lod_entry(archive, requested_name):
    if archive[:4] != b"LOD\0":
        raise RuntimeError("Invalid LOD archive header.")
    entry_count = struct.unpack_from("<I", archive, 8)[0]
    for index in range(entry_count):
        entry_offset = 92 + index * 32
        name = archive[entry_offset : entry_offset + 16].split(b"\0", 1)[0]
        if name.decode("latin1").lower() == requested_name.lower():
            offset, size, file_type, compressed_size = struct.unpack_from(
                "<IIII", archive, entry_offset + 16
            )
            return offset, size, file_type, compressed_size
    raise RuntimeError(f"LOD entry not found: {requested_name}")


def extract_lod_entry(archive, name):
    offset, size, _, compressed_size = find_lod_entry(archive, name)
    stored_size = compressed_size or size
    stored = archive[offset : offset + stored_size]
    if len(stored) != stored_size:
        raise RuntimeError(f"Truncated LOD entry: {name}")
    return zlib.decompress(stored) if compressed_size else bytes(stored)


def find_def_frame(definition, requested_frame):
    if len(definition) < 784 or struct.unpack_from("<I", definition, 0)[0] != 71:
        raise RuntimeError("Invalid secondary-skill DEF resource.")
    group_count = struct.unpack_from("<I", definition, 12)[0]
    directory_offset = 784
    first_frame = 0
    for _ in range(group_count):
        if directory_offset + 16 > len(definition):
            raise RuntimeError("Invalid DEF group directory.")
        frame_count = struct.unpack_from("<I", definition, directory_offset + 4)[0]
        names_offset = directory_offset + 16
        offsets_offset = names_offset + frame_count * 13
        next_directory_offset = offsets_offset + frame_count * 4
        if next_directory_offset > len(definition):
            raise RuntimeError("Invalid DEF frame directory.")
        if first_frame <= requested_frame < first_frame + frame_count:
            local_frame = requested_frame - first_frame
            return (
                names_offset + local_frame * 13,
                offsets_offset + local_frame * 4,
            )
        first_frame += frame_count
        directory_offset = next_directory_offset
    raise RuntimeError(f"DEF frame not found: {requested_frame}")


def map_rgba_to_palette(definition, rgba, alpha_threshold=40):
    palette = [
        tuple(definition[16 + index * 3 : 19 + index * 3])
        for index in range(256)
    ]
    cache = {}
    mapped = bytearray(len(rgba) // 4)
    for pixel in range(len(mapped)):
        offset = pixel * 4
        alpha = rgba[offset + 3]
        if alpha < alpha_threshold:
            mapped[pixel] = 0
            continue
        color = tuple(rgba[offset : offset + 3])
        if color not in cache:
            best_index = 8
            best_distance = None
            for palette_index in range(8, 256):
                candidate = palette[palette_index]
                distance = (
                    (color[0] - candidate[0]) ** 2
                    + (color[1] - candidate[1]) ** 2
                    + (color[2] - candidate[2]) ** 2
                )
                if best_distance is None or distance < best_distance:
                    best_distance = distance
                    best_index = palette_index
            cache[color] = best_index
        mapped[pixel] = cache[color]
    return bytes(mapped)


def replace_def_frame(definition, frame_index, frame_name, pixels):
    if len(frame_name.encode("latin1")) > 12:
        raise RuntimeError(f"DEF frame name is too long: {frame_name}")
    if len(pixels) != 44 * 44:
        raise RuntimeError("Spiritism icon must contain 44x44 indexed pixels.")
    name_offset, offset_table_entry = find_def_frame(definition, frame_index)
    updated = bytearray(definition)
    updated[name_offset : name_offset + 13] = b"\0" * 13
    encoded_name = frame_name.encode("latin1")
    updated[name_offset : name_offset + len(encoded_name)] = encoded_name
    frame_offset = len(updated)
    struct.pack_into("<I", updated, offset_table_entry, frame_offset)
    frame = struct.pack(
        "<IIIIIIII",
        len(pixels),
        0,
        44,
        44,
        44,
        44,
        0,
        0,
    )
    updated.extend(frame)
    updated.extend(pixels)
    return bytes(updated)


def build_spiritism_def(game_dir):
    archive = read_required(game_dir / "Data" / "H3sprite.lod")
    definition = extract_lod_entry(archive, "Secskill.def")
    if sha256(definition) != SECSKILL_HASH:
        raise RuntimeError("The installed Secskill.def is not the supported version.")
    updated = definition
    for frame_index, (icon_path, frame_name) in zip(
        NECROMANCY_ICON_FRAMES, ICON_ASSETS
    ):
        width, height, rgba = decode_png_rgba(icon_path)
        resized = resize_rgba_area(width, height, rgba)
        indexed = map_rgba_to_palette(definition, resized)
        updated = replace_def_frame(
            updated,
            frame_index,
            frame_name,
            indexed,
        )
    return updated


def registered_files_ini(original):
    newline = b"\r\n" if b"\r\n" in original else b"\n"
    line = f'"{RESOURCE_NAME}"'.encode("latin1")
    existing = {
        item.strip().lower()
        for item in original.replace(b"\r\n", b"\n").split(b"\n")
        if item.strip()
    }
    if line.lower() in existing:
        return original
    suffix = b"" if not original or original.endswith((b"\r", b"\n")) else newline
    return original + suffix + line + newline


def skill_state(executable):
    end = REPLACED_SKILL_TYPE_OFFSET + 4
    if len(executable) < end:
        return "unknown"
    skill_id = struct.unpack_from("<I", executable, REPLACED_SKILL_TYPE_OFFSET)[0]
    if skill_id == WISDOM:
        return "wisdom"
    if skill_id == NECROMANCY:
        return "spiritism"
    return f"unknown({skill_id})"


def patch_executable(executable):
    state = skill_state(executable)
    if state not in ("wisdom", "spiritism"):
        raise RuntimeError(f"Unexpected Nyx first-skill state: {state}")
    updated = bytearray(executable)
    struct.pack_into("<I", updated, REPLACED_SKILL_TYPE_OFFSET, NECROMANCY)
    return bytes(updated)


def path_hash(path):
    return sha256(path.read_bytes()) if path.is_file() else None


def resource_registered(path):
    if not path.is_file():
        return False
    target = f'"{RESOURCE_NAME}"'.lower()
    lines = path.read_text(encoding="latin1").replace("\r\n", "\n").split("\n")
    return target in {line.strip().lower() for line in lines}


def collect_status(game_dir, generated_resource=None):
    runtime_hash = path_hash(game_dir / "_HD3_Data/Common/setseed.dll")
    expected_runtime_hash = sha256(read_required(RUNTIME_ASSET))
    result = {
        "version": "HotA 1.8.0",
        "executables": {},
        "runtime": (
            "spiritism"
            if runtime_hash == expected_runtime_hash
            else "nyx-original"
            if runtime_hash == NYX_RUNTIME_HASH
            else "missing"
            if runtime_hash is None
            else "unknown"
        ),
        "resources": {},
        "registrations": {},
    }
    for name in ("h3hota.exe", "h3hota HD.exe"):
        path = game_dir / name
        result["executables"][name] = (
            skill_state(read_required(path)) if path.is_file() else "missing"
        )
    if generated_resource is not None:
        expected_resource_hash = sha256(generated_resource)
        for relative in (
            "Data/SPIRIT.def",
            "_HD3_Data/Compability/#hota/SPIRIT.def",
            "_HD3_Data/Compability/#hota15/SPIRIT.def",
        ):
            actual = path_hash(game_dir / relative)
            result["resources"][relative] = (
                "installed"
                if actual == expected_resource_hash
                else "missing"
                if actual is None
                else "unknown"
            )
    for relative in (
        "_HD3_Data/Compability/#hota/Files.ini",
        "_HD3_Data/Compability/#hota15/Files.ini",
    ):
        result["registrations"][relative] = resource_registered(
            game_dir / relative
        )
    return result


def fully_applied(status):
    return (
        all(value == "spiritism" for value in status["executables"].values())
        and status["runtime"] == "spiritism"
        and all(value == "installed" for value in status["resources"].values())
        and all(status["registrations"].values())
    )


def print_status(status):
    print("Nyx Spiritism status")
    for name, state in status["executables"].items():
        print(f"  {name}: {state}")
    print(f"  runtime DLL: {status['runtime']}")
    for name, state in status["resources"].items():
        print(f"  {name}: {state}")
    for name, registered in status["registrations"].items():
        print(f"  {name}: {'registered' if registered else 'not registered'}")
    print(f"  complete: {'yes' if fully_applied(status) else 'no'}")


def create_backup(game_dir):
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    backup_dir = game_dir / "NyxSpiritismPatch" / "backups" / stamp
    counter = 1
    while backup_dir.exists():
        backup_dir = (
            game_dir
            / "NyxSpiritismPatch"
            / "backups"
            / f"{stamp}-{counter}"
        )
        counter += 1
    manifest = {
        "patch": "Nyx Spiritism",
        "createdAt": datetime.now().astimezone().isoformat(),
        "files": {},
    }
    for relative in PATCH_FILES:
        source = game_dir / relative
        if not source.is_file():
            manifest["files"][relative] = None
            continue
        data = source.read_bytes()
        destination = backup_dir / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
        manifest["files"][relative] = sha256(data)
    backup_dir.mkdir(parents=True, exist_ok=True)
    (backup_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n",
        encoding="ascii",
    )
    return backup_dir


def validate_nyx_baseline(game_dir):
    for relative, expected_hash in NYX_EXECUTABLE_HASHES.items():
        executable = read_required(game_dir / relative)
        if sha256(executable) != expected_hash:
            raise RuntimeError(
                f"{relative} is not the supported installed Nyx version."
            )
        if skill_state(executable) != "wisdom":
            raise RuntimeError(f"{relative} no longer has Nyx's Wisdom slot.")
    runtime = read_required(game_dir / "_HD3_Data/Common/setseed.dll")
    if sha256(runtime) != NYX_RUNTIME_HASH:
        raise RuntimeError(
            "setseed.dll is not the reviewed Nyx runtime DLL; refusing to overwrite it."
        )


def apply_patch(game_dir):
    assert_hota_180(game_dir)
    generated_resource = build_spiritism_def(game_dir)
    status = collect_status(game_dir, generated_resource)
    if fully_applied(status):
        print("Nyx Spiritism is already fully applied.")
        return
    validate_nyx_baseline(game_dir)
    runtime = read_required(RUNTIME_ASSET)
    backup_dir = create_backup(game_dir)

    for relative in ("h3hota.exe", "h3hota HD.exe"):
        path = game_dir / relative
        path.write_bytes(patch_executable(path.read_bytes()))

    for relative in (
        "Data/SPIRIT.def",
        "_HD3_Data/Compability/#hota/SPIRIT.def",
        "_HD3_Data/Compability/#hota15/SPIRIT.def",
    ):
        path = game_dir / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(generated_resource)

    for relative in (
        "_HD3_Data/Compability/#hota/Files.ini",
        "_HD3_Data/Compability/#hota15/Files.ini",
    ):
        path = game_dir / relative
        path.write_bytes(registered_files_ini(read_required(path)))

    (game_dir / "_HD3_Data/Common/setseed.dll").write_bytes(runtime)
    final_status = collect_status(game_dir, generated_resource)
    if not fully_applied(final_status):
        raise RuntimeError(
            f"Post-write verification failed. Restore from {backup_dir}."
        )
    print_status(final_status)
    print(f"Backup: {backup_dir}")


def latest_backup(game_dir):
    backup_root = game_dir / "NyxSpiritismPatch" / "backups"
    if not backup_root.is_dir():
        raise RuntimeError("No Nyx Spiritism backups were found.")
    backups = sorted(
        path
        for path in backup_root.iterdir()
        if path.is_dir() and (path / "manifest.json").is_file()
    )
    if not backups:
        raise RuntimeError("No complete Nyx Spiritism backups were found.")
    return backups[-1]


def restore_patch(game_dir, requested_backup):
    backup_dir = (
        Path(requested_backup).resolve()
        if requested_backup is not None
        else latest_backup(game_dir)
    )
    manifest_path = backup_dir / "manifest.json"
    manifest = json.loads(read_required(manifest_path).decode("ascii"))
    if manifest.get("patch") != "Nyx Spiritism":
        raise RuntimeError(f"Not a Nyx Spiritism backup: {backup_dir}")
    for relative, expected_hash in manifest["files"].items():
        destination = game_dir / relative
        if expected_hash is None:
            if destination.is_file():
                destination.unlink()
            continue
        source = backup_dir / relative
        data = read_required(source)
        if sha256(data) != expected_hash:
            raise RuntimeError(f"Backup checksum mismatch: {source}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
    print(f"Restored: {backup_dir}")


def main():
    parser = argparse.ArgumentParser(
        description="Apply the Nyx-only Spiritism alias to HotA 1.8.0."
    )
    parser.add_argument("command", choices=("status", "apply", "restore"))
    parser.add_argument(
        "--game-dir",
        default=str(DEFAULT_GAME_DIR),
        help="Horn of the Abyss installation directory",
    )
    parser.add_argument(
        "--backup",
        help="Specific NyxSpiritismPatch backup directory to restore",
    )
    args = parser.parse_args()
    game_dir = Path(args.game_dir).resolve()
    try:
        assert_hota_180(game_dir)
        if args.command == "status":
            resource = build_spiritism_def(game_dir)
            print_status(collect_status(game_dir, resource))
        elif args.command == "apply":
            apply_patch(game_dir)
        else:
            restore_patch(game_dir, args.backup)
    except (OSError, RuntimeError, ValueError, zlib.error) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
