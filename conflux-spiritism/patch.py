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
RUNTIME_LOG_MARKER = "Conflux Spiritism runtime 2"

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
NYX_RUNTIME_HASH = (
    "54c997f1aebc081f2b944cbb9cecb366121c91ab83e3963056ac3641b4656a9f"
)
CONFLUX_010_RUNTIME_HASH = (
    "eabfbe0bf6e98612895359e2d96746fc9405ddd32c43aad901e07753ab0670d0"
)
SMALL_RESOURCE_HASH = (
    "ba4ba357d2859b8e5dc8077bce00b1effc0a40b42fb25fa9f53ed76dd0d85eb3"
)
LARGE_RESOURCE_HASH = (
    "8016d09158fee026bcccc83a5c43dd9d8a4cf6a42db113f8e51e81270b63392f"
)

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_GAME_DIR = SCRIPT_DIR.parent.parent
RUNTIME_ASSET = SCRIPT_DIR / "assets" / "ConfluxSpiritismRuntime.dll"
PATCH_FILES = (
    "h3hota.exe",
    "h3hota HD.exe",
    "_HD3_Data/Common/setseed.dll",
    "_HD3_Data/Common/ConfluxSpiritism.log",
)
RESOURCE_HASHES = {
    "Data/SPIRIT.def": SMALL_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota/SPIRIT.def": SMALL_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota15/SPIRIT.def": SMALL_RESOURCE_HASH,
    "Data/SPIR82.def": LARGE_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota/SPIR82.def": LARGE_RESOURCE_HASH,
    "_HD3_Data/Compability/#hota15/SPIR82.def": LARGE_RESOURCE_HASH,
}
REGISTRATION_PATHS = (
    "_HD3_Data/Compability/#hota/Files.ini",
    "_HD3_Data/Compability/#hota15/Files.ini",
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


def resources_ready(game_dir):
    return all(
        path_hash(game_dir / relative) == expected
        for relative, expected in RESOURCE_HASHES.items()
    )


def registrations_ready(game_dir):
    targets = {'"spirit.def"', '"spir82.def"'}
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
        and "hook backend=direct relative chaining" in log
        and "final=specialty fix and Spiritism hooks installed" in log
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
        "registrations": registrations_ready(game_dir),
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

    runtime_hash = path_hash(game_dir / "_HD3_Data/Common/setseed.dll")
    result["runtime"] = (
        "conflux-spiritism"
        if runtime_hash == expected_runtime_hash
        else "conflux-spiritism-0.1.0"
        if runtime_hash == CONFLUX_010_RUNTIME_HASH
        else "nyx-spiritism"
        if runtime_hash == NYX_RUNTIME_HASH
        else "missing"
        if runtime_hash is None
        else "unknown"
    )
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
    for executable, state in status["executables"].items():
        heroes = status["heroes"].get(executable, {})
        spiritists = sum(
            1 for hero in heroes.values() if hero[0] == "spiritism"
        )
        print(f"  {executable}: {state}, {spiritists}/16 Spiritism")
    print(f"  runtime DLL: {status['runtime']}")
    print(f"  last launch: {status['last_launch']}")
    print(
        "  Spiritism resources: "
        + ("installed" if status["resources"] else "missing or unknown")
    )
    print(
        "  HD registrations: "
        + ("registered" if status["registrations"] else "incomplete")
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
    if not status["resources"] or not status["registrations"]:
        raise RuntimeError(
            "The reviewed Nyx Spiritism resources are not fully installed."
        )
    if nyx_executables:
        if status["runtime"] != "nyx-spiritism":
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
        if status["runtime"] != "conflux-spiritism-0.1.0":
            raise RuntimeError(
                "The installed Conflux runtime is not the reviewed 0.1.0 DLL."
            )
        for heroes in status["heroes"].values():
            if any(hero[0] != "spiritism" for hero in heroes.values()):
                raise RuntimeError(
                    "The installed Conflux hero records are incomplete."
                )
        return
    raise RuntimeError(
        "Apply Nyx Spiritism 0.1.5 or Conflux Spiritism 0.1.0 first."
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
    backup_dir = create_backup(game_dir)

    for name in ("h3hota.exe", "h3hota HD.exe"):
        path = game_dir / name
        path.write_bytes(
            patch_executable(
                path.read_bytes(),
                CONFLUX_EXECUTABLE_HASHES[name],
            )
        )
    (game_dir / "_HD3_Data/Common/setseed.dll").write_bytes(runtime)

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
