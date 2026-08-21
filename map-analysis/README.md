# Heroes III Human Map Analysis

This folder contains a reproducible batch analysis of every `.h3m` file in
the game installation's `Maps` folder. It avoids opening maps manually.

Run:

```bash
python3 analyze_maps.py
```

The default input is `../../Maps`, relative to this repository checkout. Use
`--maps /path/to/Maps` for another installation.

Outputs:

- `MAP_CATALOG.md`: one compact feature profile per map.
- `DESIGN_PATTERNS.md`: cross-map design findings and a scenario design recipe.
- `data/maps.json`: complete normalized dataset.
- `data/maps.csv`: spreadsheet-friendly summary.
- `data/failures.json`: files that could not be analyzed.

The script uses FreeHeroes `MapToolCLI.exe` when available for complete
RoE/AB/SoD and older HotA object decoding. For HotA 1.8 subversion 9 it uses a
version-independent structural pass over terrain, object definitions, and the
declared object stream. The `object_detail` field records which confidence
level applies.
