#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成迷宫源文件（txt），并做可达性 / 最短路径校验。

格式（见任务说明书第 4 节）：
  第一行：行数 列数
  之后每行：各列地块类型，用空格分隔
  地块：-2 终点  -1 起点  0 普通地面  1 墙  2 草地  3 熔岩

用法：python tools/gen_maze.py
输出：mazes/maze20.txt（20×20 自设计迷宫）、mazes/maze11_demo.txt（11×11 演示迷宫）
"""
from __future__ import annotations

import heapq
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MAZES = ROOT / "mazes"

WALL, GROUND, GRASS, LAVA, START, END = 1, 0, 2, 3, -1, -2
COST = {GROUND: 1, GRASS: 3, LAVA: 1000, START: 1, END: 1}


def cell_coords(dim: int) -> list[int]:
    """地块中心坐标：奇数尺寸用 1,3,...,dim-2；偶数尺寸用 2,4,...,dim-2。"""
    first = 1 if dim % 2 == 1 else 2
    return list(range(first, dim - 1, 2))


def carve(rows: int, cols: int, rng: random.Random) -> tuple[list[list[int]], tuple, tuple]:
    """递归回溯挖出树状迷宫（外墙厚度 1），再随机打通少量墙形成回路。"""
    g = [[WALL] * cols for _ in range(rows)]
    rows_c, cols_c = cell_coords(rows), cell_coords(cols)
    cells = [(r, c) for r in rows_c for c in cols_c]
    start = cells[0]
    g[start[0]][start[1]] = GROUND
    stack = [start]
    seen = {start}
    while stack:
        r, c = stack[-1]
        nbrs = []
        for dr, dc in ((-2, 0), (2, 0), (0, -2), (0, 2)):
            nr, nc = r + dr, c + dc
            if nr in rows_c and nc in cols_c and (nr, nc) not in seen:
                nbrs.append(((nr, nc), (r + dr // 2, c + dc // 2)))
        if not nbrs:
            stack.pop()
            continue
        (nr, nc), (wr, wc) = rng.choice(nbrs)
        g[wr][wc] = GROUND
        g[nr][nc] = GROUND
        seen.add((nr, nc))
        stack.append((nr, nc))

    # 打通若干墙，制造环路（多条可行路径，让“最短路”更有意义）
    loops = 14 if rows >= 20 else 4
    walls = [(r, c) for r in range(1, rows - 1) for c in range(1, cols - 1) if g[r][c] == WALL]
    rng.shuffle(walls)
    opened = 0
    for r, c in walls:
        if opened >= loops:
            break
        horiz = g[r][c - 1] != WALL and g[r][c + 1] != WALL
        vert = g[r - 1][c] != WALL and g[r + 1][c] != WALL
        if horiz ^ vert:
            g[r][c] = GROUND
            opened += 1
    return g, start, cells[-1]


def dijkstra(g: list[list[int]], src: tuple[int, int]):
    rows, cols = len(g), len(g[0])
    inf = float("inf")
    dist = [[inf] * cols for _ in range(rows)]
    prev = [[None] * cols for _ in range(rows)]
    dist[src[0]][src[1]] = 0
    pq = [(0, src)]
    while pq:
        d, (r, c) = heapq.heappop(pq)
        if d > dist[r][c]:
            continue
        for dr, dc in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            nr, nc = r + dr, c + dc
            if 0 <= nr < rows and 0 <= nc < cols and g[nr][nc] != WALL:
                nd = d + COST[g[nr][nc]]
                if nd < dist[nr][nc]:
                    dist[nr][nc] = nd
                    prev[nr][nc] = (r, c)
                    heapq.heappush(pq, (nd, (nr, nc)))
    return dist, prev


def trace(prev, src, dst) -> list[tuple[int, int]]:
    if dst == src:
        return [src]
    path = [dst]
    while path[-1] != src:
        p = prev[path[-1][0]][path[-1][1]]
        if p is None:
            return []
        path.append(p)
    return path[::-1]


def build(rows: int, cols: int, seed: int) -> tuple[list[list[int]], dict]:
    rng = random.Random(seed)
    g, start, end = carve(rows, cols, rng)
    g[start[0]][start[1]] = START
    g[end[0]][end[1]] = END

    dist, prev = dijkstra(g, start)
    path = trace(prev, start, end)
    if not path:
        raise RuntimeError("迷宫不可达")
    path_set = set(path)

    def candidate(r: int, c: int) -> bool:
        if g[r][c] in (START, END, WALL):
            return False
        if abs(r - start[0]) + abs(c - start[1]) <= 1:
            return False
        if abs(r - end[0]) + abs(c - end[1]) <= 1:
            return False
        return (r, c) not in path_set

    walkable = [(r, c) for r in range(1, rows - 1) for c in range(1, cols - 1) if candidate(r, c)]
    rng.shuffle(walkable)
    grass_n = 10 if rows >= 20 else 4
    lava_n = 6 if rows >= 20 else 2
    for r, c in walkable[:grass_n]:
        g[r][c] = GRASS
    for r, c in walkable[grass_n:grass_n + lava_n]:
        g[r][c] = LAVA

    # 在最短路径旁边的岔路上再放几块草地，使“路径提示”的代价计算更直观
    side = [(r, c) for (r, c) in walkable[grass_n + lava_n:]
            if any((r + dr, c + dc) in path_set for dr, dc in ((-1, 0), (1, 0), (0, -1), (0, 1)))]
    for r, c in side[:3]:
        g[r][c] = GRASS

    # 再在最短路径本身放 2 块草地，让路径成本大于步数，直观体现带权最短路径
    path_cells = [p for p in path if p not in (start, end)]
    for r, c in path_cells[len(path_cells) // 3: len(path_cells) // 3 + 2]:
        g[r][c] = GRASS

    dist, prev = dijkstra(g, start)
    path = trace(prev, start, end)
    assert path, "加入草地/熔岩后不可达"
    assert all(g[r][c] != LAVA for r, c in path), "最短路径不应踩到熔岩"
    grass_total = sum(row.count(GRASS) for row in g)
    lava_total = sum(row.count(LAVA) for row in g)
    assert grass_total >= 2 and lava_total >= 1
    stats = {
        "cost": int(dist[end[0]][end[1]]),
        "steps": len(path) - 1,
        "grass": grass_total,
        "lava": lava_total,
        "grass_on_path": sum(1 for r, c in path if g[r][c] == GRASS),
    }
    return g, stats


def write_maze(g: list[list[int]], dest: Path, note: str) -> None:
    rows, cols = len(g), len(g[0])
    lines = [f"{rows} {cols}"]
    lines += [" ".join(str(v) for v in row) for row in g]
    dest.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"已写入 {dest}  ({rows}x{cols})  {note}")


def main() -> None:
    MAZES.mkdir(parents=True, exist_ok=True)
    g20, s20 = build(20, 20, seed=2026)
    write_maze(g20, MAZES / "maze20.txt", "自设计 20×20 迷宫")
    print(f"  20×20: 最短路径 {s20['steps']} 步, 成本 {s20['cost']}, "
          f"草地 {s20['grass']} 块(路径上 {s20['grass_on_path']} 块), 熔岩 {s20['lava']} 块")

    g11, s11 = build(11, 11, seed=7)
    write_maze(g11, MAZES / "maze11_demo.txt", "11×11 演示迷宫")
    print(f"  11×11: 最短路径 {s11['steps']} 步, 成本 {s11['cost']}, "
          f"草地 {s11['grass']} 块, 熔岩 {s11['lava']} 块")


if __name__ == "__main__":
    main()
