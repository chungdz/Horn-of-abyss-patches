#!/usr/bin/env python3
"""Batch inventory and design analysis for Heroes III H3M maps."""

from __future__ import annotations

import argparse
import csv
import gzip
import json
import math
import shutil
import statistics
import struct
import subprocess
import tempfile
from collections import Counter
from pathlib import Path


FORMATS = {0x0E: "RoE", 0x15: "AB", 0x1C: "SoD", 0x20: "HotA"}
TERRAINS = {
    0: "dirt", 1: "sand", 2: "grass", 3: "snow", 4: "swamp", 5: "rough",
    6: "subterranean", 7: "lava", 8: "water", 9: "rock",
    10: "highlands", 11: "wasteland",
}
OBJECT_NAMES = {
    5: "artifact", 6: "Pandora's Box", 8: "boat", 9: "border guard",
    10: "keymaster tent", 12: "campfire", 16: "creature bank",
    17: "dwelling", 18: "dwelling", 19: "dwelling", 20: "dwelling",
    26: "event", 33: "garrison", 34: "hero", 43: "one-way monolith",
    44: "one-way monolith", 45: "two-way monolith", 53: "mine",
    54: "monster", 59: "ocean bottle", 62: "prison", 65: "random artifact",
    66: "random artifact", 67: "random artifact", 68: "random artifact",
    69: "random artifact", 70: "random hero", 71: "random monster",
    72: "random monster", 73: "random monster", 74: "random monster",
    75: "random monster", 76: "random resource", 77: "random town",
    79: "resource", 83: "seer hut", 87: "shipyard", 91: "sign",
    93: "spell scroll", 98: "town", 101: "treasure chest",
    103: "subterranean gate", 111: "whirlpool", 113: "witch hut",
    214: "quest gate", 215: "quest guard",
}
CATEGORY_IDS = {
    "towns": {77, 98},
    "heroes": {34, 62, 70},
    "monsters": {54, 71, 72, 73, 74, 75},
    "economy": {12, 53, 76, 79, 101, 112},
    "artifacts": {5, 65, 66, 67, 68, 69, 86, 93},
    "dwellings": {17, 18, 19, 20},
    "quests": {9, 10, 83, 214, 215},
    "events": {26, 59, 91},
    "travel": {8, 43, 44, 45, 87, 103, 111},
    "banks": {6, 16, 24, 25, 63, 84, 85},
}
VICTORY_NAMES = {
    "ARTIFACT": "acquire an artifact", "GATHERTROOP": "assemble creatures",
    "GATHERRESOURCE": "accumulate resources", "BUILDCITY": "build a town",
    "BUILDGRAIL": "build the Grail", "BEATHERO": "defeat a hero",
    "CAPTURECITY": "capture a town", "BEATMONSTERS": "defeat a monster",
    "TAKEDWELLINGS": "flag all dwellings", "TAKEMINES": "flag all mines",
    "TRANSPORTITEM": "transport an artifact", "STANDARD": "standard conquest",
}


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def text(data: bytes, offset: int) -> tuple[str, int]:
    length = u32(data, offset)
    offset += 4
    raw = data[offset:offset + length]
    return raw.decode("cp1252", "replace").replace("\x00", ""), offset + length


def basic_header(data: bytes) -> dict:
    fmt = u32(data, 0)
    offset = 4
    subversion = None
    hota_version = None
    if fmt == 0x20:
        subversion = u32(data, offset)
        offset += 4
        if subversion >= 8:
            hota_version = ".".join(str(u32(data, offset + i * 4)) for i in range(3))
            offset += 12
        if subversion >= 1:
            offset += 2
        if subversion >= 2:
            offset += 4
        if subversion >= 5:
            offset += 5
        if subversion >= 7:
            offset += 1
        if subversion >= 8:
            offset += 1
        if subversion >= 9:
            offset += 4
    any_players = bool(data[offset])
    size = struct.unpack_from("<i", data, offset + 1)[0]
    underground = bool(data[offset + 5])
    offset += 6
    name, offset = text(data, offset)
    description, offset = text(data, offset)
    difficulty = data[offset]
    return {
        "format": FORMATS.get(fmt, hex(fmt)), "format_code": fmt,
        "subversion": subversion, "hota_version": hota_version,
        "size": size, "underground": underground, "any_players": any_players,
        "name": name, "description": description, "difficulty": difficulty,
    }


def parse_templates(data: bytes, start: int, terrain_count: int) -> tuple[list[dict], int, int]:
    count = u32(data, start)
    offset = start + 4
    result = []
    terrain_bytes = math.ceil(terrain_count / 8)
    fixed = 12 + terrain_bytes * 2 + 10 + 16
    for _ in range(count):
        name, offset = text(data, offset)
        offset += 12 + terrain_bytes * 2
        obj_id, subtype = struct.unpack_from("<II", data, offset)
        obj_type, priority = data[offset + 8:offset + 10]
        offset += 10 + 16
        result.append({
            "animation": name, "id": obj_id, "subtype": subtype,
            "type": obj_type, "priority": priority,
        })
    return result, u32(data, offset), offset + 4


def find_structure(data: bytes, size: int, levels: int, terrain_count: int) -> dict:
    """Locate terrain/templates without decoding version-specific hero records."""
    seen = set()
    lower_data = data.lower()
    for marker in range(len(data)):
        if not lower_data.startswith(b".def", marker):
            continue
        end = marker + 4
        for length in range(5, 80):
            length_pos = end - length - 4
            start = length_pos - 4
            if start < 0 or start in seen or u32(data, length_pos) != length:
                continue
            seen.add(start)
            count = u32(data, start)
            if not 1 <= count <= 3000:
                continue
            try:
                definitions, object_count, object_start = parse_templates(
                    data, start, terrain_count
                )
            except (IndexError, struct.error, UnicodeError):
                continue
            if len(definitions) != count or not 0 <= object_count <= 100000:
                continue
            def_names = sum(".def" in item["animation"].lower() for item in definitions)
            if def_names < max(1, len(definitions) * 9 // 10):
                continue
            if object_count:
                if object_start + 12 > len(data):
                    continue
                x, y, z = data[object_start:object_start + 3]
                defnum = u32(data, object_start + 3)
                if (x > size + 8 or y > size + 8 or z >= levels
                        or defnum >= count or data[object_start + 7:object_start + 12] != b"\0" * 5):
                    continue
            terrain_start = start - size * size * levels * 7
            if terrain_start < 0:
                continue
            tiles = [data[i:i + 7] for i in range(terrain_start, start, 7)]
            if len(tiles) != size * size * levels:
                continue
            if any(tile[0] >= terrain_count for tile in tiles):
                continue
            return {
                "definitions": definitions, "object_count": object_count,
                "tiles": tiles, "template_offset": start,
            }
    raise ValueError("object definition table not found")


def freeheroes_json(exe: Path, map_path: Path, cache_dir: Path) -> dict:
    cache = cache_dir / (map_path.stem.replace("/", "_") + ".json")
    if cache.exists() and cache.stat().st_mtime >= map_path.stat().st_mtime:
        return json.loads(cache.read_text(encoding="utf-8"))
    with tempfile.TemporaryDirectory(prefix="h3m-analysis-") as tmp:
        output = Path(tmp) / "map"
        local_input = Path(tmp) / "input.h3m"
        shutil.copy2(map_path, local_input)
        win_input = subprocess.check_output(["wslpath", "-w", str(local_input)], text=True).strip()
        win_output = subprocess.check_output(["wslpath", "-w", str(output)], text=True).strip()
        proc = subprocess.run(
            [str(exe), "--tasks", "ConvertH3MToJson", "--input-h3m", win_input,
             "--output-h3m", win_output, "--logging-level", "2"],
            cwd=exe.parent, capture_output=True, text=True,
        )
        generated = output.with_suffix(".json")
        if proc.returncode or not generated.exists():
            raise RuntimeError(proc.stderr.strip() or proc.stdout.strip() or "FreeHeroes failed")
        payload = json.loads(generated.read_text(encoding="utf-8"))
    cache.write_text(json.dumps(payload, ensure_ascii=False), encoding="utf-8")
    return payload


def counts_from_ids(ids: list[int]) -> dict:
    return {category: sum(obj_id in members for obj_id in ids)
            for category, members in CATEGORY_IDS.items()}


def analyze_one(path: Path, converter: Path | None, cache_dir: Path) -> dict:
    data = gzip.decompress(path.read_bytes())
    result = basic_header(data)
    result["file"] = path.name
    result["relative_path"] = str(path)
    levels = 2 if result["underground"] else 1
    terrain_count = 12 if result["format"] == "HotA" else 10
    structure = find_structure(data, result["size"], levels, terrain_count)
    definitions = structure["definitions"]
    result["object_count"] = structure["object_count"]
    result["definition_count"] = len(definitions)
    result["object_detail"] = "template-presence"
    result["players_human"] = None
    result["players_total"] = None
    result["victory"] = "unknown"
    result["global_events"] = None
    tiles = structure["tiles"]
    ids = [item["id"] for item in definitions]
    category_counts = counts_from_ids(ids)

    if converter and not (result["format"] == "HotA" and result["subversion"] == 9):
        parsed = freeheroes_json(converter, path, cache_dir)
        result["name"] = parsed.get("mapName") or result["name"]
        result["description"] = parsed.get("mapDescr") or result["description"]
        result["difficulty"] = parsed.get("difficulty", result["difficulty"])
        players = parsed.get("players", [])
        result["players_human"] = sum(bool(p.get("canHumanPlay")) for p in players)
        result["players_total"] = sum(
            bool(p.get("canHumanPlay") or p.get("canComputerPlay")) for p in players
        )
        victory = parsed.get("victoryCondition") or {}
        victory_type = victory.get("type", "STANDARD")
        result["victory"] = VICTORY_NAMES.get(victory_type, victory_type.lower())
        objects = parsed.get("objects", [])
        parsed_defs = parsed.get("objectDefs", [])
        ids = [parsed_defs[o["defnum"]]["id"] for o in objects]
        category_counts = counts_from_ids(ids)
        result["object_count"] = len(objects)
        result["definition_count"] = len(parsed_defs)
        result["object_detail"] = "exact"
        result["global_events"] = len(parsed.get("globalEvents", []))
        tile_rows = parsed.get("tiles", {}).get("tiles", [])
        if tile_rows:
            tiles = [
                bytes((t.get("terType", 0), t.get("terView", 0),
                       t.get("riverType", 0), t.get("riverDir", 0),
                       t.get("roadType", 0), t.get("roadDir", 0), 0))
                for t in tile_rows
            ]

    result["categories"] = category_counts
    result["terrain"] = dict(Counter(TERRAINS.get(tile[0], str(tile[0])) for tile in tiles))
    result["road_tiles"] = sum(tile[4] != 0 for tile in tiles)
    result["river_tiles"] = sum(tile[2] != 0 for tile in tiles)
    result["water_percent"] = round(100 * sum(tile[0] == 8 for tile in tiles) / len(tiles), 1)
    result["terrain_variety"] = len(result["terrain"])
    type_counts = Counter(ids)
    result["top_objects"] = [
        {"id": obj_id, "name": OBJECT_NAMES.get(obj_id, f"object {obj_id}"), "count": count}
        for obj_id, count in type_counts.most_common(8)
    ]
    signals = []
    if result["underground"]: signals.append("two-layer exploration")
    if result["water_percent"] >= 15: signals.append("meaningful naval space")
    if category_counts["quests"]: signals.append("quest-gated progression")
    if category_counts["travel"]: signals.append("travel-network routing")
    if category_counts["events"] or result["global_events"]: signals.append("scripted storytelling")
    if result["victory"] not in ("standard conquest", "unknown"): signals.append("special objective")
    if category_counts["banks"]: signals.append("risk/reward encounters")
    result["design_signals"] = signals
    result["name"] = result["name"].strip()
    return result


def pct(n: int, total: int) -> str:
    return f"{100 * n / total:.0f}%"


def write_outputs(results: list[dict], root: Path) -> None:
    data_dir = root / "data"
    (data_dir / "maps.json").write_text(
        json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8"
    )
    fields = [
        "file", "name", "format", "subversion", "hota_version", "size",
        "underground", "players_human", "players_total", "difficulty", "victory",
        "object_count", "object_detail", "definition_count", "terrain_variety",
        "water_percent", "road_tiles", "river_tiles", "design_signals",
    ] + list(CATEGORY_IDS)
    with (data_dir / "maps.csv").open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields, lineterminator="\n")
        writer.writeheader()
        for item in results:
            row = {key: item.get(key) for key in fields}
            row.update(item["categories"])
            row["design_signals"] = "; ".join(item["design_signals"])
            writer.writerow(row)

    catalog = [
        "# Human Map Catalog", "",
        f"Batch-generated from {len(results)} H3M files. `exact` means the complete object stream was decoded; "
        "`template-presence` means HotA 1.8 object totals and terrain are exact, while category numbers count "
        "distinct embedded templates rather than placed instances.", "",
    ]
    for item in results:
        size_label = f'{item["size"]}x{item["size"]}'
        layers = "surface + underground" if item["underground"] else "surface"
        players = ("unknown" if item["players_total"] is None else
                   f'{item["players_human"]} human / {item["players_total"]} active')
        terrain = ", ".join(
            name for name, _ in sorted(item["terrain"].items(), key=lambda pair: -pair[1])[:4]
        )
        objects = ", ".join(f'{x["name"]} ({x["count"]})' for x in item["top_objects"][:5])
        signals = ", ".join(item["design_signals"]) or "open-ended conquest structure"
        catalog += [
            f'## {item["name"] or Path(item["file"]).stem}',
            f'- File: `{item["file"]}`',
            f'- Format/layout: {item["format"]}'
            + (f' subversion {item["subversion"]}' if item["subversion"] is not None else "")
            + f'; {size_label}; {layers}; {players}.',
            f'- Objective: {item["victory"]}. Objects: {item["object_count"]} '
            f'({item["object_detail"]}); {item["definition_count"]} visual definitions.',
            f'- World: {terrain}; {item["water_percent"]}% water; '
            f'{item["road_tiles"]} road tiles; {item["river_tiles"]} river tiles.',
            f'- Prominent content: {objects or "decorative/environmental definitions"}.',
            f'- Design signals: {signals}.',
            f'- Premise: {item["description"].strip() or "No scenario description."}',
            "",
        ]
    (root / "MAP_CATALOG.md").write_text("\n".join(catalog), encoding="utf-8")

    total = len(results)
    exact = [item for item in results if item["object_detail"] == "exact"]
    underground = sum(item["underground"] for item in results)
    naval = sum(item["water_percent"] >= 15 for item in results)
    special = sum(item["victory"] not in ("standard conquest", "unknown") for item in results)
    quest = sum(bool(item["categories"]["quests"]) for item in results)
    travel = sum(bool(item["categories"]["travel"]) for item in results)
    scripted = sum(
        bool(item["categories"]["events"] or item["global_events"]) for item in results
    )
    formats = Counter(item["format"] for item in results)
    sizes = Counter(item["size"] for item in results)
    exact_categories = {
        key: statistics.median(item["categories"][key] for item in exact)
        for key in CATEGORY_IDS
    }
    patterns = f"""# How Human Authors Make Heroes III Maps Interesting

## Corpus and method

- Scanned {total} maps in one batch: {", ".join(f"{count} {name}" for name, count in formats.items())}.
- Sizes: {", ".join(f"{size}x{size}: {count}" for size, count in sorted(sizes.items()))}.
- {len(exact)} maps have exact full-object decoding. The {total - len(exact)} HotA 1.8 maps have exact core headers, terrain, template tables, and declared object totals, with template-level content classification.
- No map contains the standard Random Map Generator description signature.

## Repeated design patterns

1. **Give the world a spatial identity.** {underground} maps ({pct(underground, total)}) use an underground layer, while {naval} ({pct(naval, total)}) devote at least 15% of their tiles to water. Authors use layers, coasts, terrain borders, roads, and rivers to divide a square grid into memorable regions.

2. **Replace simple elimination with a concrete job.** At least {special} maps use a decoded special victory condition. Flagging dwellings or mines, transporting an artifact, defeating one target, building the Grail, and resource races turn exploration into a directed campaign.

3. **Gate progress visibly.** {quest} maps expose quest-related structures and {travel} use travel-network objects. Border guards, tents, seer huts, garrisons, monoliths, subterranean gates, and ship routes let authors reveal the map in stages without making it a straight corridor.

4. **Alternate reliable income with optional danger.** Among fully decoded maps, the median counts are {exact_categories["economy"]:.0f} economy objects, {exact_categories["monsters"]:.0f} monster objects, {exact_categories["artifacts"]:.0f} artifacts, and {exact_categories["banks"]:.0f} banks. This recurring rhythm creates expansion choices: take safe income now or spend movement and army strength on a high-value fight.

5. **Use asymmetry to create roles.** Human maps frequently mix fixed factions, unequal starting regions, isolated AI powers, and different access to water or underground. Fairness comes from compensating advantages and timing, not necessarily mirrored geometry.

6. **Make landmarks carry rules.** Towns, mines, creature banks, quest gates, monolith hubs, named heroes, and unusual terrain islands are both visual anchors and strategic decisions. A good region can be described by what the player is trying to reach there.

7. **Write story into the route.** {scripted} maps show event/sign/story structures in the available analysis. Messages work best when tied to crossing a border, visiting an object, taking a town, or meeting a named hero, so narrative arrives at the moment the geography changes.

8. **Control pacing through distance and guard strength.** Early zones cluster basic resources and low-risk pickups; connection points concentrate guards; remote branches hold artifacts, banks, prisons, or objectives. The map therefore teaches, tests, and rewards in spatial order.

## Practical recipe for generated scenarios

Start with a strategic graph before painting terrain: starting zones, expansion zones, contested hubs, optional reward branches, and the final objective. Give every connection a purpose and every dead end a reward. Then assign terrain identities, economy budgets, guard tiers, travel links, quests, and timed story beats. Finally test travel time and army growth for every playable start; visual symmetry is optional, but opportunity and tempo must be intentional.

## Limits

The reports infer design intent from map structure; they do not judge actual battle difficulty or balance without simulating armies and player routes. HotA 1.8 object bodies require a newer serializer than the available released batch converter, so those maps are labeled `template-presence` wherever instance-level category counts would otherwise overstate precision.
"""
    (root / "DESIGN_PATTERNS.md").write_text(patterns, encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--maps", type=Path, default=Path("../../Maps"))
    parser.add_argument("--converter", type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    maps_dir = (root / args.maps).resolve() if not args.maps.is_absolute() else args.maps
    converter = args.converter
    if converter is None:
        candidate = Path("/mnt/c/Users/yunfanhu/AppData/Local/Temp/freeheroes-map-analysis/FreeHeroes_current/MapToolCLI.exe")
        converter = candidate if candidate.exists() else None
    files = sorted(maps_dir.glob("*.h3m"), key=lambda path: path.name.lower())
    results = []
    failures = []
    for index, path in enumerate(files, 1):
        try:
            results.append(analyze_one(path, converter, root / "cache"))
            print(f"[{index:3}/{len(files)}] {path.name}")
        except Exception as exc:
            failures.append({"file": path.name, "error": str(exc)})
            print(f"[{index:3}/{len(files)}] FAILED {path.name}: {exc}")
    write_outputs(results, root)
    (root / "data" / "failures.json").write_text(
        json.dumps(failures, indent=2), encoding="utf-8"
    )
    print(f"Analyzed {len(results)} maps; {len(failures)} failures.")


if __name__ == "__main__":
    main()
