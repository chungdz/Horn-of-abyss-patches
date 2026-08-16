#!/usr/bin/env python3

import argparse
import csv
import hashlib
import io
import json
import re
import struct
import sys
import zlib
from datetime import datetime
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_SOURCE_GAME_DIR = SCRIPT_DIR.parent.parent
RUNTIME_ASSET = (
    SCRIPT_DIR / "assets" / "ConfluxSpiritismRuntime-cn-r10.dll"
)

BASELINE_HASHES = {
    "h3hota.exe": (
        "b5f2f793af0986050fb41df7209c25d861ae0f837af52bb3bd6864ba4de84f41"
    ),
    "h3hota HD.exe": (
        "5aaab925f06cccf23bb09814767590a95b84a557eb33d244800520be4f1f18de"
    ),
    "Data/HotA_lng.lod": (
        "46238ec4095e47b09c9a1729b6a4ffba4b7eaded2ab0199313ff01aca4df71be"
    ),
    "Data/H3sprite.lod": (
        "4f982510a381a8b1cca5ba443424fbf8ef361088d30450094663a24c39a82909"
    ),
    "Data/H3ab_spr.lod": (
        "e0d5003742c8602827ef409966784483dece6eedde76aa2cfeee26cb12d25d67"
    ),
    "Data/H3bitmap.lod": (
        "336c450cf15553f76c426ccad7896e715459f524c5a9cfcb698b3eef2385bca7"
    ),
    "Data/H3ab_bmp.lod": (
        "3cc4256e8844b3cd0af2706431dee320848bd1630b9f6c6022f6d4c78780f347"
    ),
    "_HD3_Data/Compability/#hota/Files.ini": (
        "1f3b7af922efccfd6273f8ad98c967fff764852ec0aeb900d9df58b0d4e944dc"
    ),
    "_HD3_Data/Compability/#hota15/Files.ini": (
        "9f21a6bc57c9462d405caba93687c8b1b94337dbbc94b8773727ecd2cb81f155"
    ),
    "_HD3_Data/Packs/H3中文-基础资源/HOTRAITS.TXT": (
        "361102906e4c8199c51f2be6948aa076c3d4d3a103efa0d23116c802d9922d11"
    ),
    "_HD3_Data/Packs/H3中文-基础资源/HeroSpec.txt": (
        "b4e1ab1d6e7f0c9d1f4c11c7735925a27f5b260642c6b0885a9285af8084bab4"
    ),
    "_HD3_Data/Packs/H3中文-基础资源/HeroBios.txt": (
        "71d1c53f357397e4940cdc709a75708a17ab1117972919244b1b4d7808547e27"
    ),
}

PROTECTED_HASHES = {
    "HotA.dll": (
        "0a48b6d8e2b1743bdc094f3c0dc5a0b4e995e06165993f07885252905b0be2d1"
    ),
    "HD_HOTA.dll": (
        "34c22f9ac460b57dd2ffcd205a80d0c118a42cf14005c9e4a042f4030e8e1bde"
    ),
}

FINAL_EXECUTABLE_HASHES = {
    "h3hota.exe": (
        "7aeb166c1976d87dd7b7ba43de033ec986bb4ee7d463816213f5d0a9afff7580"
    ),
    "h3hota HD.exe": (
        "110122278fb9a2ac66d39b5243d00561c6725fbda54bf23cf41c034baab6c080"
    ),
}

ARCHIVE_RESOURCE_HASHES = {
    "Data/H3sprite.lod": {
        "UN32.def": (
            "18977018c6c7efbf85fa9bc3439763be1e94b8c915ac4b62071e98c6db7e0158"
        ),
        "UN44.def": (
            "ae1da5b48b5100dfc53eacf3d267da47e4165031b7e1b48472ae86d179a44932"
        ),
        "IX32.def": (
            "4122fbc29f31d8950b958ed88f4315d02f29e194d9fa991964b8384acd1ac96e"
        ),
        "IX44.def": (
            "8efcba80e88b0266cdb1f2b076f97d67e684f2fe10ce69ca923ee8f92bc75064"
        ),
    },
    "Data/H3ab_spr.lod": {
        "UN32.def": (
            "bdba4ee42dbe8ac19f063c7307e9f709f7d3749e66e9689161ff4a9e52267314"
        ),
        "UN44.def": (
            "cd018113d38330cea5a2194c9086ba77f5135e05ffba8a423dde002f28313889"
        ),
        "IX32.def": (
            "63ff856d3ed52daaf3b834715c60a7e39da3223fce17357367cb45ee9f810198"
        ),
        "IX44.def": (
            "eeb281b6490e4ef7e786f40e8601ca64ffa05247cd4c0c7d337382da631cf807"
        ),
    },
    "Data/H3bitmap.lod": {
        "HPL004EL.pcx": (
            "d09e9632ffb46b2df90d924f8e83c132771b01511d9b4e38b31cb84bd78ba128"
        ),
        "HPS004EL.pcx": (
            "d4fc5762637db8d24139d7a3c329da98688ec9bcf01b67960a2eef4016a4ab38"
        ),
    },
    "Data/H3ab_bmp.lod": {
        "HPL004EL.pcx": (
            "d09e9632ffb46b2df90d924f8e83c132771b01511d9b4e38b31cb84bd78ba128"
        ),
        "HPS004EL.pcx": (
            "d4fc5762637db8d24139d7a3c329da98688ec9bcf01b67960a2eef4016a4ab38"
        ),
    },
}

LOOSE_PAYLOAD_HASHES = {
    "Data/IX32.def": (
        "63ff856d3ed52daaf3b834715c60a7e39da3223fce17357367cb45ee9f810198"
    ),
    "Data/IX44.def": (
        "eeb281b6490e4ef7e786f40e8601ca64ffa05247cd4c0c7d337382da631cf807"
    ),
    "Data/SPIRIT.def": (
        "ba4ba357d2859b8e5dc8077bce00b1effc0a40b42fb25fa9f53ed76dd0d85eb3"
    ),
    "Data/SPIR32.def": (
        "0ab002201dcb81a18c989f0e49e4d37b716ff209dd86ada651f9a51c5078511c"
    ),
    "Data/SPIR82.def": (
        "8016d09158fee026bcccc83a5c43dd9d8a4cf6a42db113f8e51e81270b63392f"
    ),
    "_HD3_Data/Compability/#hota/HPL004EL.bmp": (
        "a468b5ccdce7d1b6362ad5035054d2ee3a24dcf4af821f506e054e6b9d9dc8e4"
    ),
    "_HD3_Data/Compability/#hota/HPS004EL.bmp": (
        "f76981a19b970a8428902b44d9f6477e7d00df56fe92a4da544255566e0f7a07"
    ),
}

RUNTIME_HASH = (
    "550f5004718d7e03e8018a23e6f467ff7187bcebc0f43f93d0ad2daad8b7b028"
)
PREVIOUS_RUNTIME_HASHES = {
    "c383431af5ec4a1dac13faf0f546b3cb1423f6755e6e879fd864ad3f6f0104ba",
    "fc8fa7dbdc1b31910b4ff7ac2e5af2f34cdaca44216c4c358e52147d29921d02",
}
PORT_VERSION = "0.1.2"
IN_GAME_VERIFIED = False

RESOURCE_NAMES = (
    "IX32.def",
    "IX44.def",
    "HPL004EL.bmp",
    "HPS004EL.bmp",
    "SPIRIT.def",
    "SPIR32.def",
    "SPIR82.def",
)

REMOVED_COMPATIBILITY_NAMES = (
    "UN32.def",
    "UN44.def",
    "HPL004EL.pcx",
    "HPS004EL.pcx",
)

PACKS = ("#hota", "#hota15")
INSTALL_RECORD = "ChineseConfluxSpiritismPatch/installation.json"
RUNTIME_PATH = "_HD3_Data/Common/setseed.dll"
RUNTIME_LOG_PATH = "_HD3_Data/Common/ConfluxSpiritism.log"
ACTIVE_PACK_ROOT = "_HD3_Data/Packs/H3中文-基础资源"
ACTIVE_HERO_TRAITS_PATH = f"{ACTIVE_PACK_ROOT}/HOTRAITS.TXT"
ACTIVE_HERO_SPEC_PATH = f"{ACTIVE_PACK_ROOT}/HeroSpec.txt"
ACTIVE_HERO_BIOS_PATH = f"{ACTIVE_PACK_ROOT}/HeroBios.txt"
ACTIVE_LANGUAGE_PATHS = (
    ACTIVE_HERO_TRAITS_PATH,
    ACTIVE_HERO_SPEC_PATH,
    ACTIVE_HERO_BIOS_PATH,
)

PATCH_PATHS = (
    *BASELINE_HASHES,
    "Data/IX32.def",
    "Data/IX44.def",
    "Data/SPIRIT.def",
    "Data/SPIR32.def",
    "Data/SPIR82.def",
    RUNTIME_PATH,
    RUNTIME_LOG_PATH,
    INSTALL_RECORD,
    *tuple(
        f"_HD3_Data/Compability/{pack}/{name}"
        for pack in PACKS
        for name in (*RESOURCE_NAMES, *REMOVED_COMPATIBILITY_NAMES)
    ),
)

HERO_SPEC_ROW = 142
HERO_BIO_ROW = 140
HERO_TRAITS_ROW = 142

SPECIALTY_FIELDS = (
    "花妖",
    "生物奖励：花妖",
    "{花妖和花仙子}\n\n"
    "使英雄部队中的花妖和花仙子速度+1，并且英雄每升一级，"
    "它们的攻击力和防御力提高10%（向上取整）。",
)
HERO_NAME = "倪克斯"
HERO_BIOGRAPHY = (
    "倪克斯对火焰的驾驭吸引了元素城中最弱小的精灵。"
    "她用活生生的火墙庇护花妖与花仙子，并训练它们以惊人的"
    "速度和力量发起攻击。"
)
ORIGINAL_TRAIT_FIELDS = (
    "15",
    "25",
    "Pixies",
    "3",
    "5",
    "Air Elementals",
    "3",
    "5",
    "Water Elementals",
)
ACTIVE_ORIGINAL_TRAIT_FIELDS = (
    "15",
    "25",
    "Pixies",
    "3",
    "5",
    "Air Elementals",
    "2",
    "3",
    "Water Elementals",
)
PATCHED_TRAIT_FIELDS = (
    "22",
    "25",
    "Pixies",
    "22",
    "25",
    "Pixies",
    "22",
    "25",
    "Pixies",
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
        raise RuntimeError("This port supports Horn of the Abyss 1.8.0 only.")


def find_lod_entry(archive, requested_name, required=True):
    if archive[:4] != b"LOD\0":
        raise RuntimeError("Invalid LOD archive header.")
    count = struct.unpack_from("<I", archive, 8)[0]
    for index in range(count):
        directory_offset = 92 + index * 32
        name = archive[directory_offset : directory_offset + 16].split(
            b"\0", 1
        )[0]
        if name.decode("latin1").lower() == requested_name.lower():
            offset, size, file_type, compressed_size = struct.unpack_from(
                "<IIII", archive, directory_offset + 16
            )
            return {
                "directory_offset": directory_offset,
                "offset": offset,
                "size": size,
                "type": file_type,
                "compressed_size": compressed_size,
            }
    if required:
        raise RuntimeError(f"LOD entry not found: {requested_name}")
    return None


def extract_lod_entry(archive, name):
    entry = find_lod_entry(archive, name)
    stored_size = entry["compressed_size"] or entry["size"]
    stored = archive[entry["offset"] : entry["offset"] + stored_size]
    return zlib.decompress(stored) if entry["compressed_size"] else bytes(stored)


def add_or_replace_lod_entry(archive, name, replacement, file_type):
    entry = find_lod_entry(archive, name, required=False)
    if entry is not None:
        if extract_lod_entry(archive, name) == replacement:
            return archive
        updated = bytearray(archive)
        struct.pack_into("<I", updated, entry["directory_offset"] + 16, len(updated))
        struct.pack_into("<I", updated, entry["directory_offset"] + 20, len(replacement))
        struct.pack_into("<I", updated, entry["directory_offset"] + 28, 0)
        return bytes(updated) + replacement

    if len(name.encode("latin1")) > 15:
        raise RuntimeError(f"LOD entry name is too long: {name}")
    count = struct.unpack_from("<I", archive, 8)[0]
    directory_offset = 92 + count * 32
    first_data_offset = len(archive)
    for index in range(count):
        current = struct.unpack_from("<I", archive, 92 + index * 32 + 16)[0]
        if 0 < current < first_data_offset:
            first_data_offset = current
    if directory_offset + 32 > first_data_offset:
        raise RuntimeError(f"No free LOD directory slot for {name}.")

    updated = bytearray(archive)
    updated[directory_offset : directory_offset + 32] = b"\0" * 32
    updated[directory_offset : directory_offset + len(name)] = name.encode(
        "latin1"
    )
    struct.pack_into("<I", updated, directory_offset + 16, len(updated))
    struct.pack_into("<I", updated, directory_offset + 20, len(replacement))
    struct.pack_into("<I", updated, directory_offset + 24, file_type)
    struct.pack_into("<I", updated, directory_offset + 28, 0)
    struct.pack_into("<I", updated, 8, count + 1)
    return bytes(updated) + replacement


def replace_from_source(target_archive, source_archive, names):
    updated = target_archive
    for name in names:
        source_entry = find_lod_entry(source_archive, name)
        replacement = extract_lod_entry(source_archive, name)
        updated = add_or_replace_lod_entry(
            updated,
            name,
            replacement,
            source_entry["type"],
        )
    return updated


def parse_tsv(data):
    text = data.decode("gbk")
    return list(
        csv.reader(
            io.StringIO(text, newline=""),
            delimiter="\t",
            quotechar='"',
        )
    )


def serialize_tsv(rows):
    output = io.StringIO(newline="")
    writer = csv.writer(
        output,
        delimiter="\t",
        quotechar='"',
        lineterminator="\r\n",
        quoting=csv.QUOTE_MINIMAL,
    )
    writer.writerows(rows)
    return output.getvalue().encode("gbk")


def patched_language_archive(archive):
    hero_spec = parse_tsv(extract_lod_entry(archive, "HeroSpec.txt"))
    if len(hero_spec) <= HERO_SPEC_ROW:
        raise RuntimeError("HeroSpec.txt is missing the Inteus row.")
    current_specialty = tuple(hero_spec[HERO_SPEC_ROW])
    if current_specialty[0] not in ("嗜血", SPECIALTY_FIELDS[0]):
        raise RuntimeError("Unexpected Chinese Inteus specialty row.")
    hero_spec[HERO_SPEC_ROW] = list(SPECIALTY_FIELDS)

    hero_bios_data = extract_lod_entry(archive, "HeroBios.txt")
    hero_bios = hero_bios_data.decode("gbk").split("\r\n")
    if len(hero_bios) <= HERO_BIO_ROW:
        raise RuntimeError("HeroBios.txt is missing the Inteus row.")
    if (
        not hero_bios[HERO_BIO_ROW].startswith("在他很小的时候，恩托斯")
        and hero_bios[HERO_BIO_ROW] != HERO_BIOGRAPHY
    ):
        raise RuntimeError("Unexpected Chinese Inteus biography.")
    hero_bios[HERO_BIO_ROW] = HERO_BIOGRAPHY

    hero_traits = parse_tsv(extract_lod_entry(archive, "HOTRAITS.TXT"))
    if len(hero_traits) <= HERO_TRAITS_ROW:
        raise RuntimeError("HOTRAITS.TXT is missing the Inteus row.")
    traits = hero_traits[HERO_TRAITS_ROW]
    if (
        len(traits) != 10
        or traits[0] not in ("恩托斯", HERO_NAME, "Nyx")
        or tuple(traits[1:]) not in (ORIGINAL_TRAIT_FIELDS, PATCHED_TRAIT_FIELDS)
    ):
        raise RuntimeError("Unexpected Chinese Inteus hero-traits row.")
    hero_traits[HERO_TRAITS_ROW] = [HERO_NAME, *PATCHED_TRAIT_FIELDS]

    updated = archive
    replacements = (
        ("HeroSpec.txt", serialize_tsv(hero_spec)),
        ("HeroBios.txt", "\r\n".join(hero_bios).encode("gbk")),
        ("HOTRAITS.TXT", serialize_tsv(hero_traits)),
    )
    for name, replacement in replacements:
        entry = find_lod_entry(updated, name)
        updated = add_or_replace_lod_entry(
            updated, name, replacement, entry["type"]
        )
    return updated


def language_ready(archive):
    try:
        hero_spec = parse_tsv(extract_lod_entry(archive, "HeroSpec.txt"))
        hero_bios = (
            extract_lod_entry(archive, "HeroBios.txt")
            .decode("gbk")
            .split("\r\n")
        )
        hero_traits = parse_tsv(extract_lod_entry(archive, "HOTRAITS.TXT"))
        return (
            tuple(hero_spec[HERO_SPEC_ROW]) == SPECIALTY_FIELDS
            and hero_bios[HERO_BIO_ROW] == HERO_BIOGRAPHY
            and hero_traits[HERO_TRAITS_ROW]
            == [HERO_NAME, *PATCHED_TRAIT_FIELDS]
        )
    except (IndexError, RuntimeError, UnicodeDecodeError, zlib.error):
        return False


def patched_active_language_files(game_dir):
    hero_spec = parse_tsv(read_required(game_dir / ACTIVE_HERO_SPEC_PATH))
    if len(hero_spec) <= HERO_SPEC_ROW:
        raise RuntimeError("The active HeroSpec.txt is missing the Inteus row.")
    current_specialty = tuple(hero_spec[HERO_SPEC_ROW])
    if (
        current_specialty != SPECIALTY_FIELDS
        and current_specialty[0] not in ("嗜血", "嗜血奇术")
    ):
        raise RuntimeError("Unexpected active Chinese Inteus specialty row.")
    hero_spec[HERO_SPEC_ROW] = list(SPECIALTY_FIELDS)

    hero_bios_data = read_required(game_dir / ACTIVE_HERO_BIOS_PATH)
    hero_bios = hero_bios_data.decode("gbk").split("\r\n")
    if len(hero_bios) <= HERO_BIO_ROW:
        raise RuntimeError("The active HeroBios.txt is missing the Inteus row.")
    if (
        not hero_bios[HERO_BIO_ROW].startswith("恩托斯以前")
        and hero_bios[HERO_BIO_ROW] != HERO_BIOGRAPHY
    ):
        raise RuntimeError("Unexpected active Chinese Inteus biography.")
    hero_bios[HERO_BIO_ROW] = HERO_BIOGRAPHY

    hero_traits = parse_tsv(read_required(game_dir / ACTIVE_HERO_TRAITS_PATH))
    if len(hero_traits) <= HERO_TRAITS_ROW:
        raise RuntimeError("The active HOTRAITS.TXT is missing the Inteus row.")
    traits = hero_traits[HERO_TRAITS_ROW]
    if (
        len(traits) != 10
        or traits[0] not in ("恩托斯", HERO_NAME, "Nyx")
        or tuple(traits[1:])
        not in (ACTIVE_ORIGINAL_TRAIT_FIELDS, PATCHED_TRAIT_FIELDS)
    ):
        raise RuntimeError("Unexpected active Chinese Inteus hero-traits row.")
    hero_traits[HERO_TRAITS_ROW] = [HERO_NAME, *PATCHED_TRAIT_FIELDS]

    return {
        ACTIVE_HERO_SPEC_PATH: serialize_tsv(hero_spec),
        ACTIVE_HERO_BIOS_PATH: "\r\n".join(hero_bios).encode("gbk"),
        ACTIVE_HERO_TRAITS_PATH: serialize_tsv(hero_traits),
    }


def active_language_ready(game_dir):
    try:
        hero_spec = parse_tsv(read_required(game_dir / ACTIVE_HERO_SPEC_PATH))
        hero_bios = (
            read_required(game_dir / ACTIVE_HERO_BIOS_PATH)
            .decode("gbk")
            .split("\r\n")
        )
        hero_traits = parse_tsv(
            read_required(game_dir / ACTIVE_HERO_TRAITS_PATH)
        )
        return (
            tuple(hero_spec[HERO_SPEC_ROW]) == SPECIALTY_FIELDS
            and hero_bios[HERO_BIO_ROW] == HERO_BIOGRAPHY
            and hero_traits[HERO_TRAITS_ROW]
            == [HERO_NAME, *PATCHED_TRAIT_FIELDS]
        )
    except (IndexError, RuntimeError, UnicodeDecodeError):
        return False


def registered_files_ini(original):
    text = original.decode("latin1")
    newline = "\r\n" if "\r\n" in text else "\n"
    remove = {name.lower() for name in REMOVED_COMPATIBILITY_NAMES}
    lines = [
        line
        for line in text.replace("\r\n", "\n").split("\n")
        if line
        and line.strip().strip('"').lower() not in remove
    ]
    existing = {line.strip().strip('"').lower() for line in lines}
    for name in RESOURCE_NAMES:
        if name.lower() not in existing:
            lines.append(f'"{name}"')
            existing.add(name.lower())
    return (newline.join(lines) + newline).encode("latin1")


def registrations_ready(game_dir):
    expected = {name.lower() for name in RESOURCE_NAMES}
    removed = {name.lower() for name in REMOVED_COMPATIBILITY_NAMES}
    for pack in PACKS:
        path = game_dir / f"_HD3_Data/Compability/{pack}/Files.ini"
        if not path.is_file():
            return False
        names = {
            line.strip().strip('"').lower()
            for line in path.read_text(encoding="latin1").splitlines()
            if line.strip()
        }
        if not expected.issubset(names) or names.intersection(removed):
            return False
    return True


def source_payload_paths():
    paths = {
        "h3hota.exe": "h3hota.exe",
        "h3hota HD.exe": "h3hota HD.exe",
        "Data/IX32.def": "Data/IX32.def",
        "Data/IX44.def": "Data/IX44.def",
        "Data/SPIRIT.def": "Data/SPIRIT.def",
        "Data/SPIR32.def": "Data/SPIR32.def",
        "Data/SPIR82.def": "Data/SPIR82.def",
    }
    for pack in PACKS:
        for name in RESOURCE_NAMES:
            paths[
                f"_HD3_Data/Compability/{pack}/{name}"
            ] = f"_HD3_Data/Compability/{pack}/{name}"
    return paths


def validate_runtime_asset():
    runtime = read_required(RUNTIME_ASSET)
    if sha256(runtime) != RUNTIME_HASH:
        raise RuntimeError("The Chinese R10 runtime asset checksum is unexpected.")


def validate_source_payload(source_game_dir):
    for relative, expected in FINAL_EXECUTABLE_HASHES.items():
        actual = path_hash(source_game_dir / relative)
        if actual != expected:
            raise RuntimeError(
                f"English source payload mismatch: {relative} ({actual})"
            )
    for relative, expected in LOOSE_PAYLOAD_HASHES.items():
        actual = path_hash(source_game_dir / relative)
        if actual != expected:
            raise RuntimeError(
                f"English source payload mismatch: {relative} ({actual})"
            )
    validate_runtime_asset()


def validate_baseline(game_dir):
    for relative, expected in BASELINE_HASHES.items():
        actual = path_hash(game_dir / relative)
        if actual != expected:
            raise RuntimeError(
                f"Chinese baseline mismatch: {relative}\n"
                f"Expected {expected}\nActual   {actual}"
            )
    for relative, expected in PROTECTED_HASHES.items():
        actual = path_hash(game_dir / relative)
        if actual != expected:
            raise RuntimeError(
                f"Protected Chinese binary mismatch: {relative}\n"
                f"Expected {expected}\nActual   {actual}"
            )


def build_updates(game_dir, source_game_dir):
    updates = {
        "h3hota.exe": read_required(source_game_dir / "h3hota.exe"),
        "h3hota HD.exe": read_required(source_game_dir / "h3hota HD.exe"),
        "Data/HotA_lng.lod": patched_language_archive(
            read_required(game_dir / "Data/HotA_lng.lod")
        ),
        RUNTIME_PATH: read_required(RUNTIME_ASSET),
    }
    updates.update(patched_active_language_files(game_dir))

    for relative, names in ARCHIVE_RESOURCE_HASHES.items():
        target_archive = read_required(game_dir / relative)
        source_archive = read_required(source_game_dir / relative)
        updates[relative] = replace_from_source(
            target_archive, source_archive, names
        )

    for destination, source in source_payload_paths().items():
        if destination in ("h3hota.exe", "h3hota HD.exe"):
            continue
        updates[destination] = read_required(source_game_dir / source)

    for pack in PACKS:
        ini = f"_HD3_Data/Compability/{pack}/Files.ini"
        updates[ini] = registered_files_ini(read_required(game_dir / ini))
        for name in REMOVED_COMPATIBILITY_NAMES:
            updates[f"_HD3_Data/Compability/{pack}/{name}"] = None

    return updates


def create_backup(game_dir, paths=PATCH_PATHS, purpose="full install"):
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    root = game_dir / "ChineseConfluxSpiritismPatch" / "backups"
    backup_dir = root / stamp
    counter = 1
    while backup_dir.exists():
        backup_dir = root / f"{stamp}-{counter}"
        counter += 1
    manifest = {
        "patch": "Chinese HotA 1.8.0 Conflux Spiritism",
        "purpose": purpose,
        "createdAt": datetime.now().astimezone().isoformat(),
        "files": {},
    }
    for relative in paths:
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
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    return backup_dir


def archive_resources_ready(game_dir):
    for relative, resources in ARCHIVE_RESOURCE_HASHES.items():
        path = game_dir / relative
        if not path.is_file():
            return False
        archive = path.read_bytes()
        try:
            for name, expected in resources.items():
                if sha256(extract_lod_entry(archive, name)) != expected:
                    return False
        except (RuntimeError, zlib.error):
            return False
    return True


def loose_resources_ready(game_dir):
    for pack in PACKS:
        for name in RESOURCE_NAMES:
            relative = f"_HD3_Data/Compability/{pack}/{name}"
            reference = (
                f"_HD3_Data/Compability/#hota/{name}"
                if name in ("HPL004EL.bmp", "HPS004EL.bmp")
                else f"Data/{name}"
            )
            if path_hash(game_dir / relative) != path_hash(game_dir / reference):
                return False
    return all(
        path_hash(game_dir / relative) == expected
        for relative, expected in LOOSE_PAYLOAD_HASHES.items()
    )


def compatibility_overrides_removed(game_dir):
    return all(
        not (game_dir / f"_HD3_Data/Compability/{pack}/{name}").exists()
        for pack in PACKS
        for name in REMOVED_COMPATIBILITY_NAMES
    )


def last_launch_state(game_dir):
    path = game_dir / RUNTIME_LOG_PATH
    if not path.is_file():
        return "not-run"
    log = path.read_text(encoding="latin1", errors="replace")
    required = (
        "Conflux Spiritism runtime 9",
        "post-battle message tables=primary+adventure ready",
        "necromancy hook=installed",
        "necromancy rate hook=installed",
        "hero dialog hook=installed",
        "level-up hook=installed",
        "HD hero selection hook=installed",
        "HD exchange dialog hook=installed",
        "hero inspection null guards=installed",
        "final=Spiritism and Nyx UI hooks installed; shared atlas untouched",
    )
    if all(marker in log for marker in required):
        return "hooks-installed"
    if required[0] in log:
        return "hooks-failed"
    return "stale-log"


def collect_status(game_dir):
    protected = {
        relative: path_hash(game_dir / relative) == expected
        for relative, expected in PROTECTED_HASHES.items()
    }
    executables = {
        relative: (
            "patched"
            if path_hash(game_dir / relative) == expected
            else "baseline"
            if path_hash(game_dir / relative) == BASELINE_HASHES[relative]
            else "unknown"
        )
        for relative, expected in FINAL_EXECUTABLE_HASHES.items()
    }
    language = False
    language_path = game_dir / "Data/HotA_lng.lod"
    if language_path.is_file():
        language = language_ready(language_path.read_bytes())
    runtime_hash = path_hash(game_dir / RUNTIME_PATH)
    active_language = active_language_ready(game_dir)
    status = {
        "executables": executables,
        "language": language,
        "active_language": active_language,
        "archives": archive_resources_ready(game_dir),
        "loose_resources": loose_resources_ready(game_dir),
        "registrations": registrations_ready(game_dir),
        "overrides_removed": compatibility_overrides_removed(game_dir),
        "runtime": runtime_hash == RUNTIME_HASH,
        "runtime_previous": runtime_hash in PREVIOUS_RUNTIME_HASHES,
        "runtime_hash": runtime_hash,
        "protected": protected,
        "last_launch": last_launch_state(game_dir),
    }

    status["core_payload_complete"] = (
        all(state == "patched" for state in executables.values())
        and language
        and status["archives"]
        and status["loose_resources"]
        and status["registrations"]
        and status["overrides_removed"]
        and all(protected.values())
    )
    status["payload_complete"] = (
        status["core_payload_complete"] and active_language
    )
    status["mechanically_complete"] = (
        status["payload_complete"] and status["runtime"]
    )
    status["complete"] = (
        status["mechanically_complete"] and IN_GAME_VERIFIED
    )
    return status


def print_status(status):
    print("Chinese HotA 1.8.0 Conflux Spiritism status")
    for relative, state in status["executables"].items():
        print(f"  {relative}: {state}")
    print(f"  GBK Nyx text: {'installed' if status['language'] else 'missing'}")
    print(
        "  active Chinese HD-pack text: "
        + ("installed" if status["active_language"] else "missing")
    )
    print(
        "  patched archive resources: "
        + ("installed" if status["archives"] else "missing")
    )
    print(
        "  loose UI resources: "
        + ("installed" if status["loose_resources"] else "missing")
    )
    print(
        "  HD registrations: "
        + ("installed" if status["registrations"] else "incomplete")
    )
    print(
        "  unsafe compatibility overrides: "
        + ("removed" if status["overrides_removed"] else "present")
    )
    print(
        "  HD Mod 5.6 R10 runtime: "
        + (
            "installed"
            if status["runtime"]
            else "previous localization"
            if status["runtime_previous"]
            else "missing or unknown"
        )
    )
    for relative, unchanged in status["protected"].items():
        print(f"  {relative}: {'preserved' if unchanged else 'changed'}")
    print(f"  last launch: {status['last_launch']}")
    print(
        "  mechanical application: "
        + ("yes" if status["mechanically_complete"] else "no")
    )
    print("  in-game validation: unresolved (user reported failure)")
    print(f"  complete: {'yes' if status['complete'] else 'no'}")


def apply_patch(game_dir, source_game_dir):
    assert_hota_180(game_dir)
    status = collect_status(game_dir)
    if status["mechanically_complete"]:
        print(
            "The Chinese port is mechanically applied, but its visible "
            "in-game localization remains unresolved."
        )
        print_status(status)
        return
    if (
        status["core_payload_complete"]
        and (status["runtime"] or status["runtime_previous"])
    ):
        validate_runtime_asset()
        backup_dir = create_backup(
            game_dir,
            (
                *ACTIVE_LANGUAGE_PATHS,
                RUNTIME_PATH,
                RUNTIME_LOG_PATH,
                INSTALL_RECORD,
            ),
            purpose=(
                "active Chinese text and post-battle message correction"
            ),
        )
        for relative, data in patched_active_language_files(game_dir).items():
            (game_dir / relative).write_bytes(data)
        runtime_path = game_dir / RUNTIME_PATH
        runtime_path.write_bytes(read_required(RUNTIME_ASSET))
        log_path = game_dir / RUNTIME_LOG_PATH
        if log_path.is_file():
            log_path.unlink()

        record_path = game_dir / INSTALL_RECORD
        record = (
            json.loads(record_path.read_text(encoding="utf-8"))
            if record_path.is_file()
            else {"patch": "Chinese HotA 1.8.0 Conflux Spiritism"}
        )
        record["version"] = PORT_VERSION
        record["updatedAt"] = datetime.now().astimezone().isoformat()
        record["runtimeSha256"] = RUNTIME_HASH
        record["correctiveUpgradeBackup"] = str(backup_dir)
        if status["runtime_hash"] != RUNTIME_HASH:
            record["previousRuntimeSha256"] = status["runtime_hash"]
        record["notes"] = [
            "Nyx is localized as 倪克斯.",
            "Spiritism is localized as 唤灵术.",
            "The active H3中文-基础资源 loose text overrides are patched.",
            (
                "Both HotA General Text tables are scoped for the "
                "Spiritism battle-result message."
            ),
            "HD Mod 5.6 R10-specific hook RVAs are used.",
            "HotA.dll and HD_HOTA.dll are checksum-verified and not modified.",
        ]
        record_path.write_text(
            json.dumps(record, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

        final_status = collect_status(game_dir)
        if not final_status["mechanically_complete"]:
            raise RuntimeError(
                f"Corrective upgrade verification failed. Restore from {backup_dir}."
            )
        print_status(final_status)
        print(f"Corrective localization upgrade applied. Backup: {backup_dir}")
        print(f"Installation record: {record_path}")
        return

    validate_baseline(game_dir)
    validate_source_payload(source_game_dir)
    updates = build_updates(game_dir, source_game_dir)
    backup_dir = create_backup(game_dir)

    try:
        for relative, data in updates.items():
            destination = game_dir / relative
            if data is None:
                if destination.is_file():
                    destination.unlink()
                continue
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(data)

        record = {
            "patch": "Chinese HotA 1.8.0 Conflux Spiritism",
            "version": PORT_VERSION,
            "installedAt": datetime.now().astimezone().isoformat(),
            "sourceGameDirectory": str(source_game_dir),
            "backup": str(backup_dir),
            "runtimeSha256": RUNTIME_HASH,
            "preservedChineseBinaries": PROTECTED_HASHES,
            "notes": [
                "Nyx is localized as 倪克斯.",
                "Spiritism is localized as 唤灵术.",
                "The active H3中文-基础资源 loose text overrides are patched.",
                (
                    "Both HotA General Text tables are scoped for the "
                    "Spiritism battle-result message."
                ),
                "HD Mod 5.6 R10-specific hook RVAs are used.",
                "HotA.dll and HD_HOTA.dll are checksum-verified and not modified.",
            ],
        }
        record_path = game_dir / INSTALL_RECORD
        record_path.parent.mkdir(parents=True, exist_ok=True)
        record_path.write_text(
            json.dumps(record, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

        final_status = collect_status(game_dir)
        if not final_status["mechanically_complete"]:
            raise RuntimeError("Post-write verification did not reach a complete state.")
    except Exception as error:
        raise RuntimeError(
            f"{error}\nRestore from the safety backup: {backup_dir}"
        ) from error

    print_status(final_status)
    print(f"Backup: {backup_dir}")
    print(f"Installation record: {game_dir / INSTALL_RECORD}")


def latest_backup(game_dir):
    root = game_dir / "ChineseConfluxSpiritismPatch" / "backups"
    if not root.is_dir():
        raise RuntimeError("No Chinese Conflux Spiritism backups were found.")
    backups = sorted(
        path
        for path in root.iterdir()
        if path.is_dir() and (path / "manifest.json").is_file()
    )
    if not backups:
        raise RuntimeError("No complete Chinese port backups were found.")
    return backups[-1]


def restore_patch(game_dir, requested_backup):
    backup_dir = (
        Path(requested_backup).resolve()
        if requested_backup is not None
        else latest_backup(game_dir)
    )
    manifest = json.loads(
        read_required(backup_dir / "manifest.json").decode("utf-8")
    )
    if manifest.get("patch") != "Chinese HotA 1.8.0 Conflux Spiritism":
        raise RuntimeError(f"Not a Chinese port backup: {backup_dir}")
    for relative, expected in manifest["files"].items():
        destination = game_dir / relative
        if expected is None:
            if destination.is_file():
                destination.unlink()
            continue
        source = backup_dir / relative
        data = read_required(source)
        if sha256(data) != expected:
            raise RuntimeError(f"Backup checksum mismatch: {source}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
    print(f"Restored: {backup_dir}")


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Port the tested Nyx and Conflux Spiritism patches to the "
            "Chinese HotA 1.8.0 / HD Mod 5.6 R10 package."
        )
    )
    parser.add_argument("command", choices=("status", "apply", "restore"))
    parser.add_argument("--game-dir", required=True)
    parser.add_argument(
        "--source-game-dir",
        default=str(DEFAULT_SOURCE_GAME_DIR),
        help="Tested English installation that supplies final patch resources",
    )
    parser.add_argument("--backup")
    args = parser.parse_args()

    game_dir = Path(args.game_dir).resolve()
    source_game_dir = Path(args.source_game_dir).resolve()
    try:
        assert_hota_180(game_dir)
        if args.command == "status":
            print_status(collect_status(game_dir))
        elif args.command == "apply":
            apply_patch(game_dir, source_game_dir)
        else:
            restore_patch(game_dir, args.backup)
    except (OSError, RuntimeError, ValueError, zlib.error) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
