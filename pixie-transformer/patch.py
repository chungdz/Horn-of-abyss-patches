#!/usr/bin/env python3

import argparse
import hashlib
import json
import re
import struct
import sys
import zlib
from datetime import datetime
from pathlib import Path


VERSION = "0.1.14"
RUNTIME_LOG_MARKER = "Pixie Transformer runtime 7"
ORIGINAL_LANGUAGE_ARCHIVE_HASH = (
    "748b54cfac02ffc795f4b0c48c7cf6ef41ea0a6020f3cf41766271bd12eb81e9"
)
PIXIE_TRANSFORMER_015_LANGUAGE_ARCHIVE_HASH = (
    "750a3384ad1bef990ec723154731ca24482e44f7f1f3390330c34c8fc89f162d"
)
PATCHED_LANGUAGE_ARCHIVE_HASH = (
    "fff4e987f186673eab7e7b3f875db7a845ef3934448825ad85dff39843c3c9a2"
)
LEGACY_BUILDING_TEXT_HASH = (
    "b3656c794d73b5003d635df7567b8eee09e975ba3316ec7d49bdb3c0df9cfb67"
)
PIXIE_TRANSFORMER_015_BUILDING_TEXT_HASH = (
    "5c077d1592862dc5172eeab2ad9177aa4eefe13abbf1f052e4106bc0ba58b402"
)
PATCHED_BUILDING_TEXT_HASH = (
    "1c131975c28d7153b67de695cdbc97662681f90ed329d1986831d06a3026ed3d"
)
CONFLUX_SPIRITISM_029_RUNTIME_HASH = (
    "67c071790536f4186df0b348f59a7ce06b176168442d56454be7e96dde8507fd"
)
CONFLUX_SPIRITISM_030_RUNTIME_HASH = (
    "0cf45c1ecff979d9d147b9b3484646a0122c6493345febb5c6b96952d21ff41a"
)
CONFLUX_SPIRITISM_031_RUNTIME_HASH = (
    "dceddce37d411022967deec8f401c5d8bbceb526c0f7e39d83ff6dc37be28a5c"
)
CONFLUX_SPIRITISM_032_RUNTIME_HASH = (
    "c756333e63ea9cd2c375c7b19b569d5296de054f773b96f739fda82f2ffba1b2"
)
CONFLUX_SPIRITISM_033_RC1_RUNTIME_HASH = (
    "1ce6ab184321838b5ab58e466e05a092fa9e62a4f6bbc4dbe59e210498902b91"
)
CONFLUX_SPIRITISM_033_RUNTIME_HASH = (
    "169202d2fcc5981f7dcb814fd4a4a813c67ec0f6c3d72af5eed2932d58a13bb0"
)
CONFLUX_SPIRITISM_034_RC1_RUNTIME_HASH = (
    "8465c248c841bf6cb22c1774511b6968b5c45214e49b162f5758887e37e84af8"
)
CONFLUX_SPIRITISM_034_RUNTIME_HASH = (
    "a65f61a8bc2ee20af7861d8b65ee61f26409dc95b9e1e9a6aaabeefd94eb780b"
)
CONFLUX_SPIRITISM_RUNTIME_HASH = (
    "088b48db3b9d5339059ecb51b4fcdde89f2d7d084d4a9a1a877b6ea9778ca251"
)
PIXIE_TRANSFORMER_010_RUNTIME_HASH = (
    "8956f877bf50ea63338230e956438bc8a8f8c15ea2ee5ad64a91690ea6b22b6f"
)
PIXIE_TRANSFORMER_011_RUNTIME_HASH = (
    "ab7ba9cf873fd60a33c1ac8243591b1407d00b027d8885d808ef10e75e4ed336"
)
PIXIE_TRANSFORMER_012_RUNTIME_HASH = (
    "e8f2268d696761bb9ddf9a5c0b9fc1dd53f2370d5cb87beb4e863f333e7cafc3"
)
PIXIE_TRANSFORMER_013_RUNTIME_HASH = (
    "92c03e938cdd9d1354eb65146db15c286403ba50a2632445c24cd45bd07f73d8"
)
PIXIE_TRANSFORMER_014_RUNTIME_HASH = (
    "26d84b9c76d59bd5988d390ce020a0c53b0778fb7db8831fbcb25244907a5a45"
)
PIXIE_TRANSFORMER_016_RUNTIME_HASH = (
    "446aad2c399a457da1c24e48876f7d7673a60362ac0c341a3e2b50512bb75ca4"
)
PIXIE_TRANSFORMER_RUNTIME_HASH = (
    "418103cc28826cda225439b6594b2ccb9c85176697d79a142bb9a0dbf47f2509"
)
EXECUTABLE_HASHES = {
    "h3hota.exe": (
        "7aeb166c1976d87dd7b7ba43de033ec986bb4ee7d463816213f5d0a9afff7580"
    ),
    "h3hota HD.exe": (
        "110122278fb9a2ac66d39b5243d00561c6725fbda54bf23cf41c034baab6c080"
    ),
    "HotA.dll": (
        "e97aa25df70bc32c0cd5af20acec22207e86b13eb27fa5b705a102d5ef53fcec"
    ),
}

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_GAME_DIR = SCRIPT_DIR.parent.parent
RUNTIME_ASSET = SCRIPT_DIR / "assets" / "PixieTransformerRuntime.dll"
SPIRITISM_RUNTIME_ASSET = (
    SCRIPT_DIR.parent
    / "conflux-spiritism"
    / "assets"
    / "ConfluxSpiritismRuntime.dll"
)
RUNTIME_PATH = "_HD3_Data/Common/setseed.dll"
SPIRITISM_COMPANION_PATH = (
    "_HD3_Data/Common/ConfluxSpiritismRuntime.dll"
)
LOG_PATH = "_HD3_Data/Common/PixieTransformer.log"
BUILDING_TEXT_PATH = "Data/BldgSpec.txt"
LANGUAGE_ARCHIVE_PATH = "Data/HotA_lng.lod"
PATCH_FILES = (
    RUNTIME_PATH,
    SPIRITISM_COMPANION_PATH,
    LOG_PATH,
    BUILDING_TEXT_PATH,
    LANGUAGE_ARCHIVE_PATH,
)

ORIGINAL_GARDEN_ROWS = (
    (
        b"Garden of Life\t"
        b"The Garden of Life increases weekly production of Pixies "
        b"by 10 per week.\t"
    ),
    (
        b"Garden of Life\t"
        b"The Garden of Life increases weekly production of Sprites "
        b"10 per week.\t"
    ),
)
PIXIE_TRANSFORMER_015_GARDEN_ROW = (
    b"Garden of Life\t"
    b"The Garden of Life allows you to convert any creature "
    b"into a Pixie.\t"
)
GARDEN_TRANSFORMER_ROW = (
    b"Garden of Life\t"
    b"Converts creatures to Pixies or Firebirds.\t"
)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def read_required(path):
    if not path.is_file():
        raise RuntimeError(f"Missing required file: {path}")
    return path.read_bytes()


def path_hash(path):
    return sha256(path.read_bytes()) if path.is_file() else None


def assert_hota_180(game_dir):
    setup = read_required(game_dir / "HotA_Setup.ini").decode(
        "latin1", errors="replace"
    )
    if re.search(r"Main Version\s*=\s*1\.8\.0\b", setup) is None:
        raise RuntimeError("This patch supports Horn of the Abyss 1.8.0 only.")


def find_lod_entry(archive, requested_name):
    if archive[:4] != b"LOD\0":
        raise RuntimeError("Invalid LOD archive header.")
    entry_count = struct.unpack_from("<I", archive, 8)[0]
    for index in range(entry_count):
        entry_offset = 92 + index * 32
        name = archive[entry_offset : entry_offset + 16].split(b"\0", 1)[0]
        if name.decode("latin1").lower() == requested_name.lower():
            offset, size, _, compressed_size = struct.unpack_from(
                "<IIII", archive, entry_offset + 16
            )
            return entry_offset, offset, size, compressed_size
    raise RuntimeError(f"LOD entry not found: {requested_name}")


def extract_lod_entry(archive, name):
    _, offset, size, compressed_size = find_lod_entry(archive, name)
    stored_size = compressed_size or size
    stored = archive[offset : offset + stored_size]
    if len(stored) != stored_size:
        raise RuntimeError(f"Truncated LOD entry: {name}")
    data = zlib.decompress(stored) if compressed_size else bytes(stored)
    if len(data) != size:
        raise RuntimeError(f"Unexpected extracted size for LOD entry: {name}")
    return data


def lod_entry_capacity(archive, offset):
    entry_count = struct.unpack_from("<I", archive, 8)[0]
    following_offsets = []
    for index in range(entry_count):
        entry_offset = 92 + index * 32
        candidate = struct.unpack_from("<I", archive, entry_offset + 16)[0]
        if candidate > offset:
            following_offsets.append(candidate)
    next_offset = min(following_offsets) if following_offsets else len(archive)
    return next_offset - offset


def patched_resources(game_dir):
    archive = read_required(game_dir / LANGUAGE_ARCHIVE_PATH)
    archive_hash = sha256(archive)
    if archive_hash not in (
        ORIGINAL_LANGUAGE_ARCHIVE_HASH,
        PIXIE_TRANSFORMER_015_LANGUAGE_ARCHIVE_HASH,
        PATCHED_LANGUAGE_ARCHIVE_HASH,
    ):
        raise RuntimeError(
            "The installed HotA_lng.lod checksum is not the reviewed "
            "English HotA 1.8.0 or a reviewed Pixie Transformer archive."
        )
    building_text = extract_lod_entry(archive, "BldgSpec.txt")
    if archive_hash == ORIGINAL_LANGUAGE_ARCHIVE_HASH:
        for original in ORIGINAL_GARDEN_ROWS:
            if building_text.count(original) != 1:
                raise RuntimeError(
                    "The Garden of Life text rows do not match the reviewed "
                    "English HotA 1.8.0 data."
                )
            building_text = building_text.replace(
                original,
                GARDEN_TRANSFORMER_ROW,
                1,
            )
    elif archive_hash == PIXIE_TRANSFORMER_015_LANGUAGE_ARCHIVE_HASH:
        if building_text.count(PIXIE_TRANSFORMER_015_GARDEN_ROW) != 2:
            raise RuntimeError(
                "The Garden descriptions do not match Pixie Transformer "
                "0.1.5."
            )
        building_text = building_text.replace(
            PIXIE_TRANSFORMER_015_GARDEN_ROW,
            GARDEN_TRANSFORMER_ROW,
        )
    elif (
        sha256(building_text) != PATCHED_BUILDING_TEXT_HASH
        or building_text.count(GARDEN_TRANSFORMER_ROW) != 2
    ):
        raise RuntimeError(
            "The Garden descriptions do not match Pixie Transformer 0.1.6."
        )
    if sha256(building_text) != PATCHED_BUILDING_TEXT_HASH:
        raise RuntimeError("The generated Garden description is unexpected.")

    entry_offset, offset, size, compressed_size = find_lod_entry(
        archive,
        "BldgSpec.txt",
    )
    if compressed_size == 0:
        raise RuntimeError(
            "The reviewed BldgSpec.txt archive layout is unexpected."
        )
    compressed = zlib.compress(building_text, 9)
    capacity = lod_entry_capacity(archive, offset)
    if capacity < compressed_size or len(compressed) > capacity:
        raise RuntimeError(
            "The revised BldgSpec.txt does not fit its reviewed archive slot."
        )
    patched_archive = bytearray(archive)
    patched_archive[offset : offset + capacity] = (
        compressed + b"\0" * (capacity - len(compressed))
    )
    struct.pack_into(
        "<I",
        patched_archive,
        entry_offset + 20,
        len(building_text),
    )
    struct.pack_into(
        "<I",
        patched_archive,
        entry_offset + 28,
        len(compressed),
    )
    patched_archive = bytes(patched_archive)
    if sha256(patched_archive) != PATCHED_LANGUAGE_ARCHIVE_HASH:
        raise RuntimeError("The generated HotA_lng.lod checksum is unexpected.")
    if extract_lod_entry(patched_archive, "BldgSpec.txt") != building_text:
        raise RuntimeError("The revised language archive failed verification.")
    return patched_archive, building_text


def last_launch_state(game_dir):
    path = game_dir / LOG_PATH
    if not path.is_file():
        return "not-run"
    log = path.read_text(encoding="latin1", errors="replace")
    if (
        RUNTIME_LOG_MARKER in log
        and "Conflux Spiritism runtime=loaded" in log
        and (
            "HotA transformer target table=deferred until Garden click"
            in log
        )
        and "Garden horde growth=disabled" in log
        and "Garden click hook=installed" in log
        and (
            "Garden left-click=recruitment disabled; click consumed"
            in log
        )
        and "Magic Lantern dwelling=unchanged" in log
        and "final=Pixie Transformer installed" in log
    ):
        return "hooks-installed"
    if RUNTIME_LOG_MARKER in log:
        return "hooks-failed"
    return "stale-log"


def collect_status(game_dir):
    runtime_hash = path_hash(game_dir / RUNTIME_PATH)
    companion_hash = path_hash(game_dir / SPIRITISM_COMPANION_PATH)
    building_text_hash = path_hash(game_dir / BUILDING_TEXT_PATH)
    language_archive_hash = path_hash(game_dir / LANGUAGE_ARCHIVE_PATH)
    return {
        "executables": {
            name: (
                "reviewed"
                if path_hash(game_dir / name) == expected_hash
                else "missing"
                if not (game_dir / name).is_file()
                else "unknown"
            )
            for name, expected_hash in EXECUTABLE_HASHES.items()
        },
        "runtime": (
            "pixie-transformer"
            if runtime_hash == PIXIE_TRANSFORMER_RUNTIME_HASH
            else "pixie-transformer-0.1.6"
            if runtime_hash == PIXIE_TRANSFORMER_016_RUNTIME_HASH
            else "pixie-transformer-runtime-5"
            if runtime_hash == PIXIE_TRANSFORMER_014_RUNTIME_HASH
            else "pixie-transformer-0.1.3"
            if runtime_hash == PIXIE_TRANSFORMER_013_RUNTIME_HASH
            else "pixie-transformer-0.1.2"
            if runtime_hash == PIXIE_TRANSFORMER_012_RUNTIME_HASH
            else "pixie-transformer-0.1.1"
            if runtime_hash == PIXIE_TRANSFORMER_011_RUNTIME_HASH
            else "pixie-transformer-0.1.0"
            if runtime_hash == PIXIE_TRANSFORMER_010_RUNTIME_HASH
            else "conflux-spiritism-0.3.5-transfer-icon"
            if runtime_hash == CONFLUX_SPIRITISM_RUNTIME_HASH
            else "conflux-spiritism-0.3.4-transfer-withdrawn"
            if runtime_hash == CONFLUX_SPIRITISM_034_RUNTIME_HASH
            else "conflux-spiritism-0.3.3"
            if runtime_hash == CONFLUX_SPIRITISM_033_RUNTIME_HASH
            else "conflux-spiritism-0.3.4-transfer-rc1"
            if runtime_hash == CONFLUX_SPIRITISM_034_RC1_RUNTIME_HASH
            else "conflux-spiritism-0.3.3-rc1"
            if runtime_hash == CONFLUX_SPIRITISM_033_RC1_RUNTIME_HASH
            else "conflux-spiritism-0.3.2"
            if runtime_hash == CONFLUX_SPIRITISM_032_RUNTIME_HASH
            else "conflux-spiritism-0.3.1"
            if runtime_hash == CONFLUX_SPIRITISM_031_RUNTIME_HASH
            else "conflux-spiritism-0.3.0"
            if runtime_hash == CONFLUX_SPIRITISM_030_RUNTIME_HASH
            else "conflux-spiritism-0.2.9"
            if runtime_hash == CONFLUX_SPIRITISM_029_RUNTIME_HASH
            else "missing"
            if runtime_hash is None
            else "unknown"
        ),
        "spiritism_companion": (
            "conflux-spiritism-0.3.5-transfer-icon"
            if companion_hash == CONFLUX_SPIRITISM_RUNTIME_HASH
            else "conflux-spiritism-0.3.4-transfer-withdrawn"
            if companion_hash == CONFLUX_SPIRITISM_034_RUNTIME_HASH
            else "conflux-spiritism-0.3.3"
            if companion_hash == CONFLUX_SPIRITISM_033_RUNTIME_HASH
            else "conflux-spiritism-0.3.4-transfer-rc1"
            if companion_hash == CONFLUX_SPIRITISM_034_RC1_RUNTIME_HASH
            else "conflux-spiritism-0.3.3-rc1"
            if companion_hash == CONFLUX_SPIRITISM_033_RC1_RUNTIME_HASH
            else "conflux-spiritism-0.3.2"
            if companion_hash == CONFLUX_SPIRITISM_032_RUNTIME_HASH
            else "conflux-spiritism-0.3.1"
            if companion_hash == CONFLUX_SPIRITISM_031_RUNTIME_HASH
            else "conflux-spiritism-0.3.0"
            if companion_hash == CONFLUX_SPIRITISM_030_RUNTIME_HASH
            else "conflux-spiritism-0.2.9"
            if companion_hash == CONFLUX_SPIRITISM_029_RUNTIME_HASH
            else "missing"
            if companion_hash is None
            else "unknown"
        ),
        "building_text": (
            "garden-description"
            if building_text_hash == PATCHED_BUILDING_TEXT_HASH
            else "garden-description-0.1.5"
            if building_text_hash == PIXIE_TRANSFORMER_015_BUILDING_TEXT_HASH
            else "legacy-renamed"
            if building_text_hash == LEGACY_BUILDING_TEXT_HASH
            else "original-archive"
            if building_text_hash is None
            else "unknown"
        ),
        "language_archive": (
            "garden-description"
            if language_archive_hash == PATCHED_LANGUAGE_ARCHIVE_HASH
            else "garden-description-0.1.5"
            if (
                language_archive_hash
                == PIXIE_TRANSFORMER_015_LANGUAGE_ARCHIVE_HASH
            )
            else "reviewed-original"
            if language_archive_hash == ORIGINAL_LANGUAGE_ARCHIVE_HASH
            else "missing"
            if language_archive_hash is None
            else "unknown"
        ),
        "last_launch": last_launch_state(game_dir),
    }


def fully_applied(status):
    return (
        all(
            state == "reviewed"
            for state in status["executables"].values()
        )
        and status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"]
        == "conflux-spiritism-0.3.5-transfer-icon"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    )


def print_status(status):
    print(f"Pixie Transformer {VERSION} status")
    for name, state in status["executables"].items():
        print(f"  {name}: {state}")
    print(f"  loader runtime: {status['runtime']}")
    print(f"  Spiritism companion: {status['spiritism_companion']}")
    print(f"  building text: {status['building_text']}")
    print(f"  language archive: {status['language_archive']}")
    print(f"  last launch: {status['last_launch']}")
    print(f"  complete: {'yes' if fully_applied(status) else 'no'}")


def validate_prerequisite(status):
    if not all(
        state == "reviewed"
        for state in status["executables"].values()
    ):
        raise RuntimeError(
            "The installed executables are not the reviewed Conflux "
            "Spiritism files."
        )
    if (
        status["runtime"] in (
            "conflux-spiritism-0.2.9",
            "conflux-spiritism-0.3.0",
            "conflux-spiritism-0.3.1",
            "conflux-spiritism-0.3.2",
            "conflux-spiritism-0.3.3",
            "conflux-spiritism-0.3.5-transfer-icon",
            "conflux-spiritism-0.3.4-transfer-withdrawn",
            "conflux-spiritism-0.3.4-transfer-rc1",
            "conflux-spiritism-0.3.3-rc1",
        )
        and status["spiritism_companion"] == "missing"
        and status["building_text"] == "original-archive"
        and status["language_archive"] == "reviewed-original"
    ):
        return "fresh"
    if (
        status["runtime"] == "pixie-transformer-0.1.0"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "legacy-renamed"
        and status["language_archive"] == "reviewed-original"
    ):
        return "upgrade-0.1.0"
    if (
        status["runtime"] == "pixie-transformer-0.1.1"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "legacy-renamed"
        and status["language_archive"] == "reviewed-original"
    ):
        return "upgrade-0.1.1"
    if (
        status["runtime"] == "pixie-transformer-0.1.2"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "legacy-renamed"
        and status["language_archive"] == "reviewed-original"
    ):
        return "upgrade-0.1.2"
    if (
        status["runtime"] == "pixie-transformer-0.1.3"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "legacy-renamed"
        and status["language_archive"] == "reviewed-original"
    ):
        return "upgrade-0.1.3"
    if (
        status["runtime"] == "pixie-transformer-runtime-5"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "legacy-renamed"
        and status["language_archive"] == "reviewed-original"
    ):
        return "upgrade-0.1.4"
    if (
        status["runtime"] == "pixie-transformer-runtime-5"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "garden-description-0.1.5"
        and status["language_archive"] == "garden-description-0.1.5"
    ):
        return "upgrade-0.1.5"
    if (
        status["runtime"] == "pixie-transformer-0.1.6"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.6"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"]
        == "conflux-spiritism-0.3.4-transfer-withdrawn"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.14"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"]
        == "conflux-spiritism-0.3.4-transfer-rc1"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.13-rc1"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"] == "conflux-spiritism-0.3.3"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.12"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"] == "conflux-spiritism-0.3.3-rc1"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.11"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"] == "conflux-spiritism-0.3.2"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.10"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"] == "conflux-spiritism-0.3.1"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.9"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"] == "conflux-spiritism-0.3.0"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.8"
    if (
        status["runtime"] == "pixie-transformer"
        and status["spiritism_companion"] == "conflux-spiritism-0.2.9"
        and status["building_text"] == "garden-description"
        and status["language_archive"] == "garden-description"
    ):
        return "upgrade-0.1.7"
    if status["runtime"] not in (
        "conflux-spiritism-0.2.9",
        "conflux-spiritism-0.3.0",
        "conflux-spiritism-0.3.1",
        "conflux-spiritism-0.3.2",
        "conflux-spiritism-0.3.3",
        "conflux-spiritism-0.3.5-transfer-icon",
        "conflux-spiritism-0.3.4-transfer-withdrawn",
        "conflux-spiritism-0.3.4-transfer-rc1",
        "conflux-spiritism-0.3.3-rc1",
        "pixie-transformer-0.1.0",
        "pixie-transformer-0.1.1",
        "pixie-transformer-0.1.2",
        "pixie-transformer-0.1.3",
        "pixie-transformer-runtime-5",
        "pixie-transformer-0.1.6",
        "pixie-transformer",
    ):
        raise RuntimeError(
            "Install a reviewed Conflux Spiritism release through the "
            "0.3.5 transfer-icon candidate or Pixie Transformer through "
            "0.1.14 first."
        )
    raise RuntimeError("The installed Pixie Transformer state is incomplete.")


def create_backup(game_dir):
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    backup_dir = game_dir / "PixieTransformerPatch" / "backups" / stamp
    counter = 1
    while backup_dir.exists():
        backup_dir = (
            game_dir
            / "PixieTransformerPatch"
            / "backups"
            / f"{stamp}-{counter}"
        )
        counter += 1
    manifest = {
        "patch": "Pixie Transformer",
        "version": VERSION,
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


def apply_patch(game_dir):
    assert_hota_180(game_dir)
    runtime = read_required(RUNTIME_ASSET)
    if sha256(runtime) != PIXIE_TRANSFORMER_RUNTIME_HASH:
        raise RuntimeError("The packaged runtime checksum is unexpected.")
    status = collect_status(game_dir)
    if fully_applied(status):
        print("Pixie Transformer is already fully applied.")
        print_status(status)
        return
    validate_prerequisite(status)
    language_archive, building_text = patched_resources(game_dir)
    spiritism_runtime = read_required(SPIRITISM_RUNTIME_ASSET)
    if sha256(spiritism_runtime) != CONFLUX_SPIRITISM_RUNTIME_HASH:
        raise RuntimeError(
            "The packaged Conflux Spiritism runtime checksum is unexpected."
        )
    backup_dir = create_backup(game_dir)

    companion_path = game_dir / SPIRITISM_COMPANION_PATH
    companion_path.parent.mkdir(parents=True, exist_ok=True)
    companion_path.write_bytes(spiritism_runtime)
    (game_dir / RUNTIME_PATH).write_bytes(runtime)
    building_path = game_dir / BUILDING_TEXT_PATH
    building_path.parent.mkdir(parents=True, exist_ok=True)
    building_path.write_bytes(building_text)
    (game_dir / LANGUAGE_ARCHIVE_PATH).write_bytes(language_archive)
    log_path = game_dir / LOG_PATH
    if log_path.is_file():
        log_path.unlink()

    final_status = collect_status(game_dir)
    if not fully_applied(final_status):
        raise RuntimeError(
            f"Post-write verification failed. Restore from {backup_dir}."
        )
    print_status(final_status)
    print(f"Backup: {backup_dir}")


def latest_backup(game_dir):
    backup_root = game_dir / "PixieTransformerPatch" / "backups"
    if not backup_root.is_dir():
        raise RuntimeError("No Pixie Transformer backups were found.")
    backups = sorted(
        path
        for path in backup_root.iterdir()
        if path.is_dir() and (path / "manifest.json").is_file()
    )
    if not backups:
        raise RuntimeError("No complete Pixie Transformer backups were found.")
    return backups[-1]


def original_language_archive_from_backup(game_dir):
    backup_root = game_dir / "PixieTransformerPatch" / "backups"
    if not backup_root.is_dir():
        return None
    for backup_dir in sorted(backup_root.iterdir(), reverse=True):
        manifest_path = backup_dir / "manifest.json"
        if not manifest_path.is_file():
            continue
        manifest = json.loads(read_required(manifest_path).decode("ascii"))
        expected_hash = manifest.get("files", {}).get(LANGUAGE_ARCHIVE_PATH)
        if expected_hash != ORIGINAL_LANGUAGE_ARCHIVE_HASH:
            continue
        archive = read_required(backup_dir / LANGUAGE_ARCHIVE_PATH)
        if sha256(archive) != expected_hash:
            raise RuntimeError(
                "Backup checksum mismatch: "
                f"{backup_dir / LANGUAGE_ARCHIVE_PATH}"
            )
        return archive
    return None


def restore_patch(game_dir, requested_backup):
    backup_dir = (
        Path(requested_backup).resolve()
        if requested_backup is not None
        else latest_backup(game_dir)
    )
    manifest_path = backup_dir / "manifest.json"
    manifest = json.loads(read_required(manifest_path).decode("ascii"))
    if manifest.get("patch") != "Pixie Transformer":
        raise RuntimeError(f"Not a Pixie Transformer backup: {backup_dir}")
    recovered_archive = None
    if (
        LANGUAGE_ARCHIVE_PATH not in manifest["files"]
        and path_hash(game_dir / LANGUAGE_ARCHIVE_PATH)
        in (
            PIXIE_TRANSFORMER_015_LANGUAGE_ARCHIVE_HASH,
            PATCHED_LANGUAGE_ARCHIVE_HASH,
        )
    ):
        recovered_archive = original_language_archive_from_backup(game_dir)
        if recovered_archive is None:
            raise RuntimeError(
                "This older backup predates language-archive tracking, and "
                "no verified original HotA_lng.lod checkpoint is available."
            )
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
    if recovered_archive is not None:
        (game_dir / LANGUAGE_ARCHIVE_PATH).write_bytes(recovered_archive)
    print(f"Restored: {backup_dir}")


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Replace the Conflux Garden of Life with a 1:1 "
            "Pixie/Firebird Transformer."
        )
    )
    parser.add_argument("command", choices=("status", "apply", "restore"))
    parser.add_argument(
        "--game-dir",
        default=str(DEFAULT_GAME_DIR),
        help="Horn of the Abyss installation directory",
    )
    parser.add_argument(
        "--backup",
        help="Specific PixieTransformerPatch backup directory to restore",
    )
    args = parser.parse_args()
    game_dir = Path(args.game_dir).resolve()
    try:
        assert_hota_180(game_dir)
        if args.command == "status":
            print_status(collect_status(game_dir))
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
