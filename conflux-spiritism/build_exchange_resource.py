#!/usr/bin/env python3

import argparse
import hashlib
import importlib.util
import struct
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
REPOSITORY_DIR = SCRIPT_DIR.parent
DEFAULT_GAME_DIR = REPOSITORY_DIR.parent
DEFAULT_OUTPUT = SCRIPT_DIR / "assets" / "SPIR32.def"
NATIVE_EXCHANGE_HASH = (
    "e56aeeaa81e36d08aaeb86b296ef9903b7f3100ecd81e11c6df8922937afd213"
)
DIALOG_BACKGROUND_HASH = (
    "4488f5b228850bddea2f92d25a22a71f7c5387f1ee6e92a7cf69717190677083"
)
SPIRITISM_FRAMES = (39, 40, 41)
EXCHANGE_FRAME_NAMES = (
    "SP32BAS.PCX",
    "SP32ADV.PCX",
    "SP32EXP.PCX",
)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def load_resource_helpers():
    helper_path = REPOSITORY_DIR / "nyx-spiritism" / "patch.py"
    spec = importlib.util.spec_from_file_location(
        "nyx_spiritism_resource_helpers",
        helper_path,
    )
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Cannot import resource helpers: {helper_path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def validate_exchange_resource(helpers, definition):
    reserved_names = {
        frame_name.lower()
        for _, frame_name in helpers.ICON_ASSETS
    }
    reserved_names.update(
        frame_name.lower()
        for frame_name in helpers.LARGE_ICON_FRAME_NAMES
    )
    if reserved_names.intersection(
        frame_name.lower()
        for frame_name in EXCHANGE_FRAME_NAMES
    ):
        raise RuntimeError("Exchange frame names collide with another atlas.")

    for frame_index, expected_name in zip(
        SPIRITISM_FRAMES,
        EXCHANGE_FRAME_NAMES,
    ):
        name_offset, table_offset = helpers.find_def_frame(
            definition,
            frame_index,
        )
        actual_name = (
            definition[name_offset : name_offset + 13]
            .split(b"\0", 1)[0]
            .decode("latin1")
        )
        frame_offset = struct.unpack_from("<I", definition, table_offset)[0]
        header = struct.unpack_from("<IIIIIIII", definition, frame_offset)
        if actual_name != expected_name:
            raise RuntimeError(
                f"Frame {frame_index} has unexpected name {actual_name!r}."
            )
        if header != (1024, 0, 32, 32, 32, 32, 0, 0):
            raise RuntimeError(
                f"Frame {frame_index} is not an uncompressed 32x32 frame."
            )


def build_exchange_resource(game_dir):
    helpers = load_resource_helpers()
    sprite_archive = helpers.read_required(game_dir / "Data" / "H3sprite.lod")
    definition = helpers.extract_lod_entry(sprite_archive, "SecSk32.def")
    if sha256(definition) != NATIVE_EXCHANGE_HASH:
        raise RuntimeError(
            "The installed SecSk32.def is not the supported HotA 1.8.0 version."
        )

    bitmap_archive = helpers.read_required(game_dir / "Data" / "H3bitmap.lod")
    background_data = helpers.extract_lod_entry(
        bitmap_archive,
        "DiBoxBck.pcx",
    )
    if sha256(background_data) != DIALOG_BACKGROUND_HASH:
        raise RuntimeError(
            "The installed DiBoxBck.pcx is not the supported version."
        )
    background = helpers.decode_lod_pcx(background_data)

    updated = definition
    for frame_index, icon_asset, frame_name in zip(
        SPIRITISM_FRAMES,
        helpers.ICON_ASSETS,
        EXCHANGE_FRAME_NAMES,
    ):
        icon_path = icon_asset[0]
        width, height, rgba = helpers.decode_png_rgba(icon_path)
        resized = helpers.resize_rgba_area(width, height, rgba, 32, 32)
        indexed = helpers.map_rgba_to_palette(
            definition,
            resized,
            background,
            32,
            32,
        )
        updated = helpers.replace_def_frame(
            updated,
            frame_index,
            frame_name,
            indexed,
            32,
            32,
        )

    validate_exchange_resource(helpers, updated)
    return updated


def main():
    parser = argparse.ArgumentParser(
        description="Build the collision-free 32x32 Spiritism transfer atlas."
    )
    parser.add_argument(
        "--game-dir",
        type=Path,
        default=DEFAULT_GAME_DIR,
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
    )
    args = parser.parse_args()

    output = build_exchange_resource(args.game_dir.resolve())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    print(f"Wrote {args.output}")
    print(f"SHA-256 {sha256(output)}")


if __name__ == "__main__":
    main()
