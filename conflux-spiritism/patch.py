#!/usr/bin/env python3

import argparse
import hashlib
import json
import re
import struct
import sys
from datetime import datetime
from pathlib import Path


HERO_FIRST_ID = 128
HERO_RECORD_STRIDE = 0x5C
FIRST_SKILL_TYPE_OFFSET = 0x27CBDC
NECROMANCY = 12
EMPTY_SKILL = 0xFFFFFFFF
RUNTIME_LOG_MARKER = "Conflux Spiritism runtime 19 transfer-right-click"

HEROES = (
    (128, "Pasis", 22, 1, 20, 1),
    (129, "Thunar", 19, 1, 13, 1),
    (130, "Ignissa", 22, 1, 20, 1),
    (131, "Lacus", 19, 2, EMPTY_SKILL, 0),
    (132, "Monere", 22, 1, 2, 1),
    (133, "Erdamon", 19, 1, 13, 1),
    (134, "Fiur", 22, 2, EMPTY_SKILL, 0),
    (135, "Kalt", 19, 1, 21, 1),
    (136, "Luna", 7, 1, 14, 1),
    (137, "Brissa", 7, 1, 15, 1),
    (138, "Ciele", 7, 1, 16, 1),
    (139, "Labetha", 7, 1, 17, 1),
    (140, "Nyx", NECROMANCY, 1, 14, 1),
    (141, "Aenain", 7, 1, 15, 1),
    (142, "Gelare", 7, 1, 16, 1),
    (143, "Grindan", 7, 1, 17, 1),
)

SKILL_NAMES = {
    2: "Logistics",
    7: "Wisdom",
    12: "Spiritism",
    13: "Estates",
    14: "Fire Magic",
    15: "Air Magic",
    16: "Water Magic",
    17: "Earth Magic",
    19: "Tactics",
    20: "Artillery",
    21: "Learning",
    22: "Offense",
    EMPTY_SKILL: "None",
}

NYX_EXECUTABLE_HASHES = {
    "h3hota.exe": (
        "0e710b6de65ec869b333a18a858c9f2c1fe3e1d3eb2ba2a076db159720d66b61"
    ),
    "h3hota HD.exe": (
        "c0f7d8801e2e475260ac573b1a2457cfd4e8bde8ad892bc952f4cb95930d24b5"
    ),
}
CONFLUX_EXECUTABLE_HASHES = {
    "h3hota.exe": (
        "7aeb166c1976d87dd7b7ba43de033ec986bb4ee7d463816213f5d0a9afff7580"
    ),
    "h3hota HD.exe": (
        "110122278fb9a2ac66d39b5243d00561c6725fbda54bf23cf41c034baab6c080"
    ),
}
NYX_015_RUNTIME_HASH = (
    "54c997f1aebc081f2b944cbb9cecb366121c91ab83e3963056ac3641b4656a9f"
)
NYX_RUNTIME_HASH = (
    "1d08e5c9eee7d56a0be7dd141adca706ebf56ded99e42b5674f80fd8dc29f180"
)
CONFLUX_010_RUNTIME_HASH = (
    "eabfbe0bf6e98612895359e2d96746fc9405ddd32c43aad901e07753ab0670d0"
)
CONFLUX_020_RUNTIME_HASH = (
    "c5ce44bddf87e5e2a581c3574adf3f4a67a44ab9f35c3f1594227bae58f254d5"
)
CONFLUX_021_RUNTIME_HASH = (
    "3391a5df7297f8f9bccbb414b45d7f2fccb6b61f2894c1a787ef572a02c20d5a"
)
CONFLUX_022_RUNTIME_HASH = (
    "0d07e59563addc1b4cf0e3d80c62a73c9bb2ff7761206aafc4e5a233908f9749"
)
CONFLUX_023_RUNTIME_HASH = (
    "78ebff46f61197e46cca580a159dbea42f64053bfdef12b1fce479163d06434a"
)
CONFLUX_024_RUNTIME_HASH = (
    "2a6c35a1188a1018a5f6e598e6ecec6b9bd42355892d91038d6c140c17056163"
)
CONFLUX_025_RUNTIME_HASH = (
    "acee00f853c68da4d260ae7bb51b9dcf14f3ed4f6cd6a201a16441f792161dab"
)
CONFLUX_026_RUNTIME_HASH = (
    "8841bc03b8a2e9cc39aacce50d963c55c37b2c635e701521ce6d07b85c6e396c"
)
CONFLUX_027_RUNTIME_HASH = (
    "938d53c27c298a4d856bef7d793724858a3fdcf262aff1a00cb1fd3488473a7a"
)
CONFLUX_028_RUNTIME_HASH = (
    "6e1c82e0ba5100505bddb55a4f0089694a1ee9fe7b5144a3db3ad5098e0a694e"
)
CONFLUX_029_RUNTIME_HASH = (
    "67c071790536f4186df0b348f59a7ce06b176168442d56454be7e96dde8507fd"
)
CONFLUX_030_RUNTIME_HASH = (
    "0cf45c1ecff979d9d147b9b3484646a0122c6493345febb5c6b96952d21ff41a"
)
CONFLUX_031_RUNTIME_HASH = (
    "dceddce37d411022967deec8f401c5d8bbceb526c0f7e39d83ff6dc37be28a5c"
)
CONFLUX_032_RUNTIME_HASH = (
    "c756333e63ea9cd2c375c7b19b569d5296de054f773b96f739fda82f2ffba1b2"
)
CONFLUX_033_RC1_RUNTIME_HASH = (
    "1ce6ab184321838b5ab58e466e05a092fa9e62a4f6bbc4dbe59e210498902b91"
)
CONFLUX_033_RUNTIME_HASH = (
    "169202d2fcc5981f7dcb814fd4a4a813c67ec0f6c3d72af5eed2932d58a13bb0"
)
CONFLUX_034_RC1_RUNTIME_HASH = (
    "8465c248c841bf6cb22c1774511b6968b5c45214e49b162f5758887e37e84af8"
)
CONFLUX_034_RUNTIME_HASH = (
    "a65f61a8bc2ee20af7861d8b65ee61f26409dc95b9e1e9a6aaabeefd94eb780b"
)
CONFLUX_035_RUNTIME_HASH = (
    "088b48db3b9d5339059ecb51b4fcdde89f2d7d084d4a9a1a877b6ea9778ca251"
)
SMALL_RESOURCE_HASH = (
    "ba4ba357d2859b8e5dc8077bce00b1effc0a40b42fb25fa9f53ed76dd0d85eb3"
)
LARGE_RESOURCE_HASH = (
    "8016d09158fee026bcccc83a5c43dd9d8a4cf6a42db113f8e51e81270b63392f"
)
EXCHANGE_RESOURCE_HASH = (
    "22e030b0bef348c5afa682d693f14cbe3e7886b9dfa01b319b33eb323d3290a8"
)

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_GAME_DIR = SCRIPT_DIR.parent.parent
RUNTIME_ASSET = SCRIPT_DIR / "assets" / "ConfluxSpiritismRuntime.dll"
RUNTIME_PATH = "_HD3_Data/Common/setseed.dll"
COMPANION_RUNTIME_PATH = (
    "_HD3_Data/Common/ConfluxSpiritismRuntime.dll"
)
EXCHANGE_RESOURCE_ASSET = SCRIPT_DIR / "assets" / "SPIR32.def"
CORE_RESOURCE_HASHES = {
    "Data/SPIRIT.def": SMALL_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota/SPIRIT.def": SMALL_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota15/SPIRIT.def": SMALL_RESOURCE_HASH,
    "Data/SPIR82.def": LARGE_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota/SPIR82.def": LARGE_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota15/SPIR82.def": LARGE_RESOURCE_HASH,
}
NYX_SPECIALTY_RESOURCE_HASHES = {
    "Data/IX32.def": (
        "63ff856d3ed52daaf3b834715c60a7e39da3223fce17357367cb45ee9f810198"
    ),
    "_HD3_Data/Compability/#hota/IX32.def": (
        "63ff856d3ed52daaf3b834715c60a7e39da3223fce17357367cb45ee9f810198"
    ),
    "_HD3_Data/Compability/#hota15/IX32.def": (
        "63ff856d3ed52daaf3b834715c60a7e39da3223fce17357367cb45ee9f810198"
    ),
    "Data/IX44.def": (
        "eeb281b6490e4ef7e786f40e8601ca64ffa05247cd4c0c7d337382da631cf807"
    ),
    "_HD3_Data/Compability/#hota/IX44.def": (
        "eeb281b6490e4ef7e786f40e8601ca64ffa05247cd4c0c7d337382da631cf807"
    ),
    "_HD3_Data/Compability/#hota15/IX44.def": (
        "eeb281b6490e4ef7e786f40e8601ca64ffa05247cd4c0c7d337382da631cf807"
    ),
}
EXCHANGE_RESOURCE_PATHS = (
    "Data/SPIR32.def",
    "_HD3_Data/Compability/#hota/SPIR32.def",
    "_HD3_Data/Compability/#hota15/SPIR32.def",
)
RESOURCE_HASHES = {
    **CORE_RESOURCE_HASHES,
    **NYX_SPECIALTY_RESOURCE_HASHES,
    **{
        relative: EXCHANGE_RESOURCE_HASH
        for relative in EXCHANGE_RESOURCE_PATHS
    },
}
REGISTRATION_PATHS = (
    "_HD3_Data/Compability/#hota/Files.ini",
    "_HD3_Data/Compability/#hota15/Files.ini",
)
LEGACY_SPECIALTY_OVERRIDE_HASHES = {
    "_HD3_Data/Compability/#hota/UN32.def": (
        "bdba4ee42dbe8ac19f063c7307e9f709f7d3749e66e9689161ff4a9e52267314"
    ),
    "_HD3_Data/Compability/#hota/UN44.def": (
        "cd018113d38330cea5a2194c9086ba77f5135e05ffba8a423dde002f28313889"
    ),
    "_HD3_Data/Compability/#hota15/UN32.def": (
        "bdba4ee42dbe8ac19f063c7307e9f709f7d3749e66e9689161ff4a9e52267314"
    ),
    "_HD3_Data/Compability/#hota15/UN44.def": (
        "cd018113d38330cea5a2194c9086ba77f5135e05ffba8a423dde002f28313889"
    ),
}
PATCH_FILES = (
    "h3hota.exe",
    "h3hota HD.exe",
    RUNTIME_PATH,
    COMPANION_RUNTIME_PATH,
    "_HD3_Data/Common/ConfluxSpiritism.log",
    *REGISTRATION_PATHS,
    *LEGACY_SPECIALTY_OVERRIDE_HASHES,
    *EXCHANGE_RESOURCE_PATHS,
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


def hero_skill_offset(hero_id):
    return FIRST_SKILL_TYPE_OFFSET + (
        hero_id - HERO_FIRST_ID
    ) * HERO_RECORD_STRIDE


def read_hero_skills(executable, hero_id):
    offset = hero_skill_offset(hero_id)
    if offset < 0 or offset + 16 > len(executable):
        raise RuntimeError(f"Hero {hero_id} record is outside the executable.")
    return struct.unpack_from("<IIII", executable, offset)


def hero_states(executable):
    states = {}
    for (
        hero_id,
        name,
        original_first,
        expected_first_level,
        expected_second,
        expected_second_level,
    ) in HEROES:
        first, first_level, second, second_level = read_hero_skills(
            executable, hero_id
        )
        if (
            first_level != expected_first_level
            or second != expected_second
            or second_level != expected_second_level
        ):
            states[name] = (
                "unexpected",
                first,
                first_level,
                second,
                second_level,
            )
        elif first == NECROMANCY:
            states[name] = (
                "spiritism",
                first,
                first_level,
                second,
                second_level,
            )
        elif first == original_first:
            states[name] = (
                "original",
                first,
                first_level,
                second,
                second_level,
            )
        else:
            states[name] = (
                "unexpected",
                first,
                first_level,
                second,
                second_level,
            )
    return states


def patch_executable(executable, expected_hash):
    states = hero_states(executable)
    unexpected = [
        name for name, state in states.items() if state[0] == "unexpected"
    ]
    if unexpected:
        raise RuntimeError(
            "Unexpected Conflux hero records: " + ", ".join(unexpected)
        )
    updated = bytearray(executable)
    for hero_id, *_ in HEROES:
        struct.pack_into(
            "<I",
            updated,
            hero_skill_offset(hero_id),
            NECROMANCY,
        )
    result = bytes(updated)
    if sha256(result) != expected_hash:
        raise RuntimeError("Generated executable checksum is unexpected.")
    return result


def resources_ready(game_dir, resource_hashes=RESOURCE_HASHES):
    return all(
        path_hash(game_dir / relative) == expected
        for relative, expected in resource_hashes.items()
    )


def spiritism_registrations_ready(game_dir, include_exchange=True):
    targets = {
        '"ix32.def"',
        '"ix44.def"',
        '"spirit.def"',
        '"spir82.def"',
    }
    if include_exchange:
        targets.add('"spir32.def"')
    for relative in REGISTRATION_PATHS:
        path = game_dir / relative
        if not path.is_file():
            return False
        lines = {
            line.strip().lower()
            for line in path.read_text(encoding="latin1")
            .replace("\r\n", "\n")
            .split("\n")
            if line.strip()
        }
        if not targets.issubset(lines):
            return False
    return True


def registered_files_ini(original):
    newline = b"\r\n" if b"\r\n" in original else b"\n"
    existing = {
        item.strip().lower()
        for item in original.replace(b"\r\n", b"\n").split(b"\n")
        if item.strip()
    }
    updated = original
    for resource_name in ("SPIRIT.def", "SPIR32.def", "SPIR82.def"):
        line = f'"{resource_name}"'.encode("latin1")
        if line.lower() in existing:
            continue
        suffix = (
            b""
            if not updated or updated.endswith((b"\r", b"\n"))
            else newline
        )
        updated += suffix + line + newline
        existing.add(line.lower())
    return updated


def specialty_override_state(game_dir):
    present = 0
    for relative, expected_hash in LEGACY_SPECIALTY_OVERRIDE_HASHES.items():
        actual_hash = path_hash(game_dir / relative)
        if actual_hash is None:
            continue
        if actual_hash != expected_hash:
            return "unknown"
        present += 1

    registered = 0
    for relative in REGISTRATION_PATHS:
        path = game_dir / relative
        if not path.is_file():
            return "unknown"
        lines = {
            line.strip().lower()
            for line in path.read_text(encoding="latin1")
            .replace("\r\n", "\n")
            .split("\n")
            if line.strip()
        }
        registered += sum(
            target in lines for target in ('"un32.def"', '"un44.def"')
        )

    if present == 0 and registered == 0:
        return "removed"
    if present == len(LEGACY_SPECIALTY_OVERRIDE_HASHES) and registered == 4:
        return "legacy-installed"
    return "mixed-known"


def remove_specialty_overrides(game_dir):
    for relative, expected_hash in LEGACY_SPECIALTY_OVERRIDE_HASHES.items():
        path = game_dir / relative
        actual_hash = path_hash(path)
        if actual_hash is None:
            continue
        if actual_hash != expected_hash:
            raise RuntimeError(f"Refusing to remove unknown override: {path}")
        path.unlink()

    removed = {'"un32.def"', '"un44.def"'}
    for relative in REGISTRATION_PATHS:
        path = game_dir / relative
        original = read_required(path).decode("latin1")
        lines = original.replace("\r\n", "\n").split("\n")
        updated = [
            line
            for line in lines
            if line.strip().lower() not in removed
        ]
        text = "\r\n".join(updated)
        if original.endswith(("\n", "\r")) and not text.endswith("\r\n"):
            text += "\r\n"
        path.write_text(text, encoding="latin1", newline="")


def last_launch_state(game_dir):
    path = game_dir / "_HD3_Data/Common/ConfluxSpiritism.log"
    if not path.is_file():
        return "not-run"
    log = path.read_text(encoding="latin1", errors="replace")
    if (
        RUNTIME_LOG_MARKER in log
        and "necromancy hook=installed" in log
        and "necromancy rate hook=installed" in log
        and "hero dialog hook=installed" in log
        and "level-up hook=installed" in log
        and "HD hero selection hook=installed" in log
        and "HD exchange dialog hook=installed" in log
        and (
            "HD exchange skill right-click call hooks=installed"
            in log
        )
        and (
            "Hermit skill upgrade hook=disabled for transfer-only candidate"
            in log
        )
        and "hero inspection null guards=installed" in log
        and "specialty atlas mutation=disabled" in log
        and "scoped Nyx specialty aliases=ready" in log
        and "HD exchange skill literal=ready" in log
        and "exchange control pointer sites=ready" in log
        and "extended specialty Vehr frame=available" in log
        and (
            "hook backend=relative chaining; "
            "no exchange event entry or Hermit hook"
            in log
        )
        and (
            "small skill frame overlay=ready; native group unchanged"
            in log
        )
        and (
            "large skill frame overlay=ready; native group unchanged"
            in log
        )
        and (
            "exchange skill resource pair=ready; native group unchanged"
            in log
        )
        and "secondary skill group mutation=disabled" in log
        and (
            "exchange right-click scope="
            "native popup loader filename only"
            in log
        )
        and "Hermit scope=disabled" in log
        and (
            "final=transfer icon+right-click hooks installed; "
            "native HotA groups unchanged"
            in log
        )
    ):
        return "hooks-installed"
    if RUNTIME_LOG_MARKER in log:
        return "hooks-failed"
    return "stale-log"


def collect_status(game_dir):
    expected_runtime_hash = sha256(read_required(RUNTIME_ASSET))
    result = {
        "executables": {},
        "heroes": {},
        "runtime": "unknown",
        "resources": resources_ready(game_dir),
        "registrations": spiritism_registrations_ready(game_dir),
        "specialty_overrides": specialty_override_state(game_dir),
        "last_launch": last_launch_state(game_dir),
    }
    for name in ("h3hota.exe", "h3hota HD.exe"):
        path = game_dir / name
        if not path.is_file():
            result["executables"][name] = "missing"
            result["heroes"][name] = {}
            continue
        data = path.read_bytes()
        digest = sha256(data)
        result["executables"][name] = (
            "conflux-spiritism"
            if digest == CONFLUX_EXECUTABLE_HASHES[name]
            else "nyx-spiritism"
            if digest == NYX_EXECUTABLE_HASHES[name]
            else "unknown"
        )
        result["heroes"][name] = hero_states(data)

    def classify_runtime(runtime_hash):
        return (
            "conflux-spiritism"
            if runtime_hash == expected_runtime_hash
            else "conflux-spiritism-0.1.0"
            if runtime_hash == CONFLUX_010_RUNTIME_HASH
            else "conflux-spiritism-0.2.0"
            if runtime_hash == CONFLUX_020_RUNTIME_HASH
            else "conflux-spiritism-0.2.1"
            if runtime_hash == CONFLUX_021_RUNTIME_HASH
            else "conflux-spiritism-0.2.2"
            if runtime_hash == CONFLUX_022_RUNTIME_HASH
            else "conflux-spiritism-0.2.3"
            if runtime_hash == CONFLUX_023_RUNTIME_HASH
            else "conflux-spiritism-0.2.4"
            if runtime_hash == CONFLUX_024_RUNTIME_HASH
            else "conflux-spiritism-0.2.5"
            if runtime_hash == CONFLUX_025_RUNTIME_HASH
            else "conflux-spiritism-0.2.6"
            if runtime_hash == CONFLUX_026_RUNTIME_HASH
            else "conflux-spiritism-0.2.7"
            if runtime_hash == CONFLUX_027_RUNTIME_HASH
            else "conflux-spiritism-0.2.8"
            if runtime_hash == CONFLUX_028_RUNTIME_HASH
            else "conflux-spiritism-0.2.9"
            if runtime_hash == CONFLUX_029_RUNTIME_HASH
            else "conflux-spiritism-0.3.0"
            if runtime_hash == CONFLUX_030_RUNTIME_HASH
            else "conflux-spiritism-0.3.1"
            if runtime_hash == CONFLUX_031_RUNTIME_HASH
            else "conflux-spiritism-0.3.2"
            if runtime_hash == CONFLUX_032_RUNTIME_HASH
            else "conflux-spiritism-0.3.3-rc1"
            if runtime_hash == CONFLUX_033_RC1_RUNTIME_HASH
            else "conflux-spiritism-0.3.3-withdrawn"
            if runtime_hash == CONFLUX_033_RUNTIME_HASH
            else "conflux-spiritism-0.3.4-transfer-rc1"
            if runtime_hash == CONFLUX_034_RC1_RUNTIME_HASH
            else "conflux-spiritism-0.3.4-transfer-withdrawn"
            if runtime_hash == CONFLUX_034_RUNTIME_HASH
            else "conflux-spiritism-0.3.5-transfer-icon"
            if runtime_hash == CONFLUX_035_RUNTIME_HASH
            else "nyx-spiritism"
            if runtime_hash == NYX_RUNTIME_HASH
            else "nyx-spiritism-0.1.5"
            if runtime_hash == NYX_015_RUNTIME_HASH
            else "missing"
            if runtime_hash is None
            else "unknown"
        )

    loader_runtime = classify_runtime(path_hash(game_dir / RUNTIME_PATH))
    companion_runtime = classify_runtime(
        path_hash(game_dir / COMPANION_RUNTIME_PATH)
    )
    if loader_runtime not in ("missing", "unknown"):
        result["runtime"] = loader_runtime
        result["runtime_path"] = RUNTIME_PATH
    elif companion_runtime not in ("missing", "unknown"):
        result["runtime"] = companion_runtime
        result["runtime_path"] = COMPANION_RUNTIME_PATH
    else:
        result["runtime"] = loader_runtime
        result["runtime_path"] = RUNTIME_PATH
    return result


def fully_applied(status):
    return (
        all(
            state == "conflux-spiritism"
            for state in status["executables"].values()
        )
        and all(
            hero[0] == "spiritism"
            for heroes in status["heroes"].values()
            for hero in heroes.values()
        )
        and status["runtime"] == "conflux-spiritism"
        and status["resources"]
        and status["registrations"]
        and status["specialty_overrides"] == "removed"
    )


def level_name(level):
    return {0: "None", 1: "Basic", 2: "Advanced", 3: "Expert"}.get(
        level, f"Level {level}"
    )


def skill_name(skill_id):
    return SKILL_NAMES.get(skill_id, f"Skill {skill_id}")


def print_status(status):
    print("Conflux Spiritism status")
    print("  Conflux base rates: 10% / 20% / 30%")
    print("  target defaults: Nyx Sprite (119), others Pixie (118)")
    for executable, state in status["executables"].items():
        heroes = status["heroes"].get(executable, {})
        spiritists = sum(
            1 for hero in heroes.values() if hero[0] == "spiritism"
        )
        print(f"  {executable}: {state}, {spiritists}/16 Spiritism")
    print(
        f"  runtime DLL: {status['runtime']} "
        f"({status['runtime_path']})"
    )
    print(f"  last launch: {status['last_launch']}")
    print(
        "  Spiritism resources: "
        + ("installed" if status["resources"] else "missing or unknown")
    )
    print(
        "  HD registrations: "
        + ("registered" if status["registrations"] else "incomplete")
    )
    print(
        "  truncated specialty overrides: "
        + status["specialty_overrides"]
    )
    print(f"  complete: {'yes' if fully_applied(status) else 'no'}")

    first_heroes = status["heroes"].get("h3hota.exe", {})
    if first_heroes:
        print("  hero slot-one skills:")
        for _, name, *_ in HEROES:
            state, first, first_level, second, second_level = first_heroes[name]
            second_text = (
                "None"
                if second == EMPTY_SKILL
                else f"{level_name(second_level)} {skill_name(second)}"
            )
            print(
                f"    {name}: {level_name(first_level)} "
                f"{skill_name(first)}; second {second_text} ({state})"
            )


def validate_prerequisite(game_dir, status):
    nyx_executables = all(
        state == "nyx-spiritism"
        for state in status["executables"].values()
    )
    conflux_executables = all(
        state == "conflux-spiritism"
        for state in status["executables"].values()
    )
    if (
        not resources_ready(game_dir, CORE_RESOURCE_HASHES)
        or not resources_ready(game_dir, NYX_SPECIALTY_RESOURCE_HASHES)
        or not spiritism_registrations_ready(
            game_dir,
            include_exchange=False,
        )
    ):
        raise RuntimeError(
            "The reviewed Nyx Spiritism resources are not fully installed."
        )
    if status["specialty_overrides"] == "unknown":
        raise RuntimeError(
            "An unknown UN32.def or UN44.def HD override is installed."
        )
    if nyx_executables:
        if status["runtime"] not in (
            "nyx-spiritism",
            "nyx-spiritism-0.1.5",
        ):
            raise RuntimeError(
                "The installed runtime is not the reviewed Nyx Spiritism DLL."
            )
        for heroes in status["heroes"].values():
            for _, name, original_first, *_ in HEROES:
                expected = NECROMANCY if name == "Nyx" else original_first
                state = heroes[name]
                if state[1] != expected or state[0] == "unexpected":
                    raise RuntimeError(
                        f"Unexpected prerequisite skill state for {name}."
                    )
        return
    if conflux_executables:
        if status["runtime"] not in (
            "conflux-spiritism",
            "conflux-spiritism-0.1.0",
            "conflux-spiritism-0.2.0",
            "conflux-spiritism-0.2.1",
            "conflux-spiritism-0.2.2",
            "conflux-spiritism-0.2.3",
            "conflux-spiritism-0.2.4",
            "conflux-spiritism-0.2.5",
            "conflux-spiritism-0.2.6",
            "conflux-spiritism-0.2.7",
            "conflux-spiritism-0.2.8",
            "conflux-spiritism-0.2.9",
            "conflux-spiritism-0.3.0",
            "conflux-spiritism-0.3.1",
            "conflux-spiritism-0.3.2",
            "conflux-spiritism-0.3.3-rc1",
            "conflux-spiritism-0.3.3-withdrawn",
            "conflux-spiritism-0.3.4-transfer-rc1",
            "conflux-spiritism-0.3.4-transfer-withdrawn",
            "conflux-spiritism-0.3.5-transfer-icon",
        ):
            raise RuntimeError(
                "The installed Conflux runtime is not a reviewed upgrade source."
            )
        for heroes in status["heroes"].values():
            if any(hero[0] != "spiritism" for hero in heroes.values()):
                raise RuntimeError(
                    "The installed Conflux hero records are incomplete."
                )
        return
    raise RuntimeError(
        "Apply Nyx Spiritism 0.1.6 or a reviewed Conflux Spiritism release "
        "first."
    )


def create_backup(game_dir):
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    backup_dir = game_dir / "ConfluxSpiritismPatch" / "backups" / stamp
    counter = 1
    while backup_dir.exists():
        backup_dir = (
            game_dir
            / "ConfluxSpiritismPatch"
            / "backups"
            / f"{stamp}-{counter}"
        )
        counter += 1
    manifest = {
        "patch": "Conflux Spiritism",
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
    status = collect_status(game_dir)
    if fully_applied(status):
        print("Conflux Spiritism is already fully applied.")
        return
    validate_prerequisite(game_dir, status)
    runtime = read_required(RUNTIME_ASSET)
    exchange_resource = read_required(EXCHANGE_RESOURCE_ASSET)
    if sha256(exchange_resource) != EXCHANGE_RESOURCE_HASH:
        raise RuntimeError("The packaged SPIR32.def checksum is unexpected.")
    backup_dir = create_backup(game_dir)

    for name in ("h3hota.exe", "h3hota HD.exe"):
        path = game_dir / name
        path.write_bytes(
            patch_executable(
                path.read_bytes(),
                CONFLUX_EXECUTABLE_HASHES[name],
            )
        )
    runtime_path = game_dir / status["runtime_path"]
    runtime_path.parent.mkdir(parents=True, exist_ok=True)
    runtime_path.write_bytes(runtime)
    for relative in EXCHANGE_RESOURCE_PATHS:
        path = game_dir / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(exchange_resource)
    for relative in REGISTRATION_PATHS:
        path = game_dir / relative
        path.write_bytes(registered_files_ini(read_required(path)))
    remove_specialty_overrides(game_dir)

    final_status = collect_status(game_dir)
    if not fully_applied(final_status):
        raise RuntimeError(
            f"Post-write verification failed. Restore from {backup_dir}."
        )
    print_status(final_status)
    print(f"Backup: {backup_dir}")


def latest_backup(game_dir):
    backup_root = game_dir / "ConfluxSpiritismPatch" / "backups"
    if not backup_root.is_dir():
        raise RuntimeError("No Conflux Spiritism backups were found.")
    backups = sorted(
        path
        for path in backup_root.iterdir()
        if path.is_dir() and (path / "manifest.json").is_file()
    )
    if not backups:
        raise RuntimeError("No complete Conflux Spiritism backups were found.")
    return backups[-1]


def restore_patch(game_dir, requested_backup):
    backup_dir = (
        Path(requested_backup).resolve()
        if requested_backup is not None
        else latest_backup(game_dir)
    )
    manifest_path = backup_dir / "manifest.json"
    manifest = json.loads(read_required(manifest_path).decode("ascii"))
    if manifest.get("patch") != "Conflux Spiritism":
        raise RuntimeError(f"Not a Conflux Spiritism backup: {backup_dir}")
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
        description=(
            "Give every Conflux hero 10%/20%/30% Spiritism over Necromancy."
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
        help="Specific ConfluxSpiritismPatch backup directory to restore",
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
    except (OSError, RuntimeError, ValueError) as error:
        print(f"Error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
