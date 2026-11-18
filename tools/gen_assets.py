#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成迷宫游戏所需的图片资源（像素风 64x64 地块 + 标题横幅 + 窗口图标）。

用法：python tools/gen_assets.py
输出：assets/tiles/*.png、assets/ui/*.png
"""
from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
TILES = ROOT / "assets" / "tiles"
UI = ROOT / "assets" / "ui"
FONT_PATH = Path(r"C:\Windows\Fonts\simhei.ttf")
S = 64  # tile size

# ------- 调色板 -------
GROUND = (214, 198, 158)
GROUND_D = (196, 178, 136)
GROUND_L = (230, 216, 180)
WALL = (86, 65, 48)
WALL_D = (52, 38, 28)
WALL_L = (112, 88, 66)
INK = (43, 29, 22)


def new_tile() -> Image.Image:
    return Image.new("RGBA", (S, S), (0, 0, 0, 0))


def jitter(px: tuple, amount: int, rng: random.Random) -> tuple:
    return (px[0] + rng.randint(-amount, amount),
            px[1] + rng.randint(-amount, amount),
            px[2] + rng.randint(-amount, amount))


def draw_ground_base(img: Image.Image, rng: random.Random) -> None:
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, S - 1, S - 1], fill=GROUND + (255,))
    # 6x6 的小石子颗粒
    for _ in range(46):
        x, y = rng.randrange(2, S - 2), rng.randrange(2, S - 2)
        w = rng.choice((1, 2, 2, 3))
        c = rng.choice((GROUND_D, GROUND_L, jitter(GROUND, 10, rng)))
        d.rectangle([x, y, x + w - 1, y + w - 1], fill=c + (255,))
    # 四周轻微描边，让网格更清晰
    d.rectangle([0, 0, S - 1, S - 1], outline=(176, 158, 118, 255), width=1)


def tile_ground() -> Image.Image:
    rng = random.Random(11)
    img = new_tile()
    draw_ground_base(img, rng)
    return img


def tile_wall() -> Image.Image:
    rng = random.Random(22)
    img = new_tile()
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, S - 1, S - 1], fill=WALL_D + (255,))
    # 错缝砖块：砖 32x16，砖缝 3px
    for row in range(4):
        y0 = row * 16
        offset = 0 if row % 2 == 0 else 16
        for k in range(-1, 3):
            x0 = offset + k * 32
            xs = max(0, x0)
            x1 = min(S - 1, x0 + 32 - 3)
            if x1 <= xs:
                continue
            d.rectangle([xs, y0 + 1, x1, y0 + 14], fill=jitter(WALL, 8, rng) + (255,))
            d.line([(xs, y0 + 1), (x1, y0 + 1)], fill=WALL_L + (255,), width=1)
            d.line([(xs, y0 + 13), (x1, y0 + 13)], fill=WALL_D + (255,), width=1)
    # 顶面高光
    d.line([(0, 0), (S - 1, 0)], fill=(132, 106, 82, 255), width=2)
    d.rectangle([0, 0, S - 1, S - 1], outline=(38, 27, 20, 255), width=1)
    return img


def tile_grass() -> Image.Image:
    rng = random.Random(33)
    img = tile_ground()
    d = ImageDraw.Draw(img)
    # 草地色块
    for _ in range(16):
        x, y = rng.randrange(3, S - 12), rng.randrange(3, S - 12)
        w, h = rng.randint(8, 14), rng.randint(6, 11)
        d.ellipse([x, y, x + w, y + h], fill=(86, 150, 66, 255))
    for _ in range(24):
        x = rng.randrange(4, S - 5)
        y = rng.randrange(8, S - 6)
        h = rng.randint(5, 10)
        # 草叶
        d.line([(x, y + h), (x, y)], fill=(58, 112, 44, 255), width=1)
        d.line([(x + 2, y + h - 1), (x + 4, y)], fill=(120, 190, 90, 255), width=1)
    d.rectangle([0, 0, S - 1, S - 1], outline=(150, 170, 120, 160), width=1)
    return img


def tile_lava() -> Image.Image:
    rng = random.Random(44)
    img = new_tile()
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, S - 1, S - 1], fill=(150, 34, 12, 255))
    # 熔岩流动的亮斑
    for _ in range(9):
        x, y = rng.randrange(0, S - 20), rng.randrange(0, S - 14)
        w, h = rng.randint(14, 28), rng.randint(8, 16)
        d.ellipse([x, y, x + w, y + h], fill=(226, 78, 20, 255))
    for _ in range(12):
        x, y = rng.randrange(2, S - 6), rng.randrange(2, S - 6)
        r = rng.randint(1, 3)
        d.ellipse([x, y, x + r, y + r], fill=(255, 168, 62, 255))
    for _ in range(6):
        x, y = rng.randrange(4, S - 8), rng.randrange(4, S - 8)
        d.line([(x, y), (x + rng.randint(3, 9), y - rng.randint(2, 6))],
               fill=(255, 214, 110, 255), width=1)
    d.rectangle([0, 0, S - 1, S - 1], outline=(92, 20, 8, 255), width=2)
    return img


def tile_start() -> Image.Image:
    rng = random.Random(55)
    img = tile_ground()
    d = ImageDraw.Draw(img)
    # 蓝色传送门
    d.ellipse([12, 12, 52, 52], fill=(30, 52, 96, 255))
    d.ellipse([16, 16, 48, 48], fill=(52, 96, 168, 255))
    d.ellipse([22, 22, 42, 42], fill=(120, 172, 240, 255))
    d.ellipse([27, 27, 37, 37], fill=(206, 232, 255, 255))
    # 向上箭头
    d.polygon([(32, 22), (42, 34), (36, 34), (36, 44), (28, 44), (28, 34), (22, 34)],
              fill=(255, 255, 255, 235), outline=(24, 40, 72, 255))
    d.rectangle([0, 0, S - 1, S - 1], outline=(40, 70, 120, 200), width=2)
    return img


def tile_end() -> Image.Image:
    rng = random.Random(66)
    img = tile_ground()
    d = ImageDraw.Draw(img)
    # 旗杆 + 方格旗
    cx, cy = 30, 14
    d.line([(cx, cy), (cx, 52)], fill=(90, 66, 46, 255), width=3)
    d.line([(cx + 1, cy), (cx + 1, 52)], fill=(160, 130, 90, 255), width=1)
    for row in range(3):
        for col in range(4):
            x = cx + 3 + col * 7
            y = cy + 2 + row * 7
            if (row + col) % 2 == 0:
                d.rectangle([x, y, x + 6, y + 6], fill=(255, 255, 255, 255))
            else:
                d.rectangle([x, y, x + 6, y + 6], fill=(46, 46, 52, 255))
    d.rectangle([cx + 3, cy + 2, cx + 30, cy + 23], outline=(30, 28, 30, 255), width=1)
    # 底座
    d.ellipse([18, 46, 46, 56], fill=(122, 100, 64, 220))
    d.rectangle([0, 0, S - 1, S - 1], outline=(190, 160, 90, 220), width=2)
    return img


def tile_player() -> Image.Image:
    img = new_tile()
    d = ImageDraw.Draw(img)
    # 影子
    d.ellipse([16, 50, 48, 58], fill=(0, 0, 0, 70))
    # 靴子
    d.rectangle([22, 44, 29, 53], fill=(96, 64, 40, 255), outline=INK + (255,))
    d.rectangle([35, 44, 42, 53], fill=(96, 64, 40, 255), outline=INK + (255,))
    # 裤腿
    d.rectangle([23, 37, 30, 45], fill=(60, 72, 104, 255))
    d.rectangle([34, 37, 41, 45], fill=(60, 72, 104, 255))
    # 上衣
    d.rectangle([21, 24, 43, 40], fill=(58, 112, 196, 255), outline=INK + (255,))
    d.rectangle([21, 24, 43, 28], fill=(92, 150, 226, 255))
    # 腰带
    d.rectangle([22, 38, 42, 42], fill=(150, 106, 60, 255), outline=INK + (255,))
    d.rectangle([30, 38, 34, 42], fill=(232, 196, 112, 255))
    # 手臂
    d.rectangle([16, 25, 21, 36], fill=(58, 112, 196, 255), outline=INK + (255,))
    d.rectangle([43, 25, 48, 36], fill=(58, 112, 196, 255), outline=INK + (255,))
    d.rectangle([16, 34, 21, 39], fill=(238, 196, 152, 255), outline=INK + (255,))
    d.rectangle([43, 34, 48, 39], fill=(238, 196, 152, 255), outline=INK + (255,))
    # 头
    d.rectangle([22, 10, 42, 26], fill=(240, 198, 154, 255), outline=INK + (255,))
    # 头发
    d.rectangle([21, 8, 43, 14], fill=(66, 44, 30, 255), outline=INK + (255,))
    d.rectangle([21, 12, 24, 20], fill=(66, 44, 30, 255))
    # 眼睛
    d.rectangle([27, 18, 29, 21], fill=INK + (255,))
    d.rectangle([35, 18, 37, 21], fill=INK + (255,))
    # 围巾 / 领口
    d.rectangle([24, 24, 40, 27], fill=(212, 84, 64, 255), outline=INK + (255,))
    return img


def title_banner() -> Image.Image:
    W, H = 760, 108
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    # 横幅底：半透明深色圆角 + 金边
    d.rounded_rectangle([1, 1, W - 2, H - 2], radius=18, fill=(22, 26, 38, 205),
                        outline=(214, 176, 96, 200), width=2)
    d.rounded_rectangle([6, 6, W - 7, H - 7], radius=14, outline=(214, 176, 96, 80), width=1)
    # 左右装饰小方块
    for x in (22, W - 34):
        d.rectangle([x, 46, x + 10, 56], fill=(214, 176, 96, 220))
    font_big = ImageFont.truetype(str(FONT_PATH), 52)
    font_small = ImageFont.truetype(str(FONT_PATH), 17)
    text = "迷宫游戏"
    tw = d.textlength(text, font=font_big)
    tx = (W - tw) / 2
    d.text((tx + 2, 20), text, font=font_big, fill=(0, 0, 0, 160))
    d.text((tx, 18), text, font=font_big, fill=(248, 214, 132, 255))
    sub = "MAZE GAME  ·  RAYLIB 6.0  ·  基础任务 + 进阶任务 1"
    sw = d.textlength(sub, font=font_small)
    d.text(((W - sw) / 2, 78), sub, font=font_small, fill=(168, 190, 220, 255))
    return img


def window_icon() -> Image.Image:
    img = Image.new("RGBA", (64, 64), (24, 28, 40, 255))
    d = ImageDraw.Draw(img)
    # 简易迷宫图案
    for x in range(0, 64, 10):
        for y in range(0, 64, 10):
            if (x * 3 + y * 7) % 23 < 9:
                d.rectangle([x, y, x + 9, y + 9], fill=(96, 126, 190, 255))
    d.line([(10, 54), (18, 54), (18, 44), (28, 44), (28, 34), (38, 34), (38, 24), (48, 24)],
           fill=(248, 214, 132, 255), width=4)
    d.ellipse([44, 20, 54, 30], fill=(255, 226, 150, 255))
    return img


def main() -> None:
    TILES.mkdir(parents=True, exist_ok=True)
    UI.mkdir(parents=True, exist_ok=True)
    assert FONT_PATH.exists(), f"缺少字体：{FONT_PATH}"

    tiles = {
        "ground": tile_ground(),
        "wall": tile_wall(),
        "grass": tile_grass(),
        "lava": tile_lava(),
        "start": tile_start(),
        "end": tile_end(),
        "player": tile_player(),
    }
    for name, img in tiles.items():
        img.save(TILES / f"{name}.png")

    title_banner().save(UI / "title.png")
    window_icon().save(UI / "icon.png")
    # 大地图缩略示例（用于 README 展示）
    print("资源生成完成：", len(tiles) + 2, "个文件 ->", TILES)


if __name__ == "__main__":
    main()
