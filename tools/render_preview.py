#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""把迷宫源文件渲染成 PNG 预览图（用于美术/布局检查，也方便写文档）。

用法：python tools/render_preview.py mazes/maze20.txt [out.png] [tile_px]
"""
from __future__ import annotations

import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
TILE_DIR = ROOT / "assets" / "tiles"
MAP = {
    -2: "end.png",
    -1: "start.png",
    0: "ground.png",
    1: "wall.png",
    2: "grass.png",
    3: "lava.png",
}


def load(path: Path):
    lines = [ln.strip() for ln in path.read_text(encoding="utf-8").splitlines() if ln.strip()]
    rows, cols = map(int, lines[0].split())
    grid = [list(map(int, ln.split())) for ln in lines[1:]]
    assert len(grid) == rows and all(len(r) == cols for r in grid), "迷宫尺寸与首行不一致"
    return rows, cols, grid


def main() -> None:
    maze = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "mazes" / "maze20.txt"
    out = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / ".tmp_maze_preview.png"
    size = int(sys.argv[3]) if len(sys.argv) > 3 else 28
    rows, cols, grid = load(maze)
    cache = {}
    out_img = Image.new("RGB", (cols * size, rows * size), (18, 20, 28))
    for r in range(rows):
        for c in range(cols):
            name = MAP[grid[r][c]]
            if name not in cache:
                cache[name] = Image.open(TILE_DIR / name).convert("RGBA").resize((size, size), Image.NEAREST)
            out_img.paste(cache[name], (c * size, r * size), cache[name])
    out_img.save(out)
    print(f"{maze} -> {out}  ({cols}x{rows}, tile={size}px)")


if __name__ == "__main__":
    main()
