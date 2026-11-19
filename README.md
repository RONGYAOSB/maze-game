# 迷宫游戏（Raylib 6.0 · 基础任务 + 进阶任务 1 + 高级算法）

《程序设计综合实践》迷宫游戏：用 Raylib 6.0 绘制图形化迷宫，方向键控制小人行走，
`P` 键显示从当前位置到终点的带权最短路径；含开始界面（空格开始）与到达终点的胜利提示。
高级算法部分实现了「怪物沿最短路径追击」与「随机迷宫生成」。

![开始界面](docs/screenshots/00-title.png)

## 1. 已完成的基础任务

| 说明书要求 | 实现情况 | 代码位置 |
| --- | --- | --- |
| 1. 创建窗口并显示读取的图片文件 | 创建 1360×900 窗口；读取 `assets/ui/title.png` 作为标题横幅、`assets/ui/icon.png` 作为窗口图标、7 张地块贴图 | `src/main.cpp`、`src/render/Renderer.cpp` |
| 2. 根据迷宫源文件用图片资源显示图形化迷宫 | 解析 txt 迷宫，按地块类型绘制对应贴图，并动态适配窗口尺寸 | `src/core/Maze.cpp`、`src/render/Renderer.cpp` |
| 3. 小人在迷宫中用方向键行走，不能穿墙，起点开局 | 网格移动 + 短动画；目标格为墙或越界时拒绝移动；初始位置取迷宫起点 `-1` | `src/core/Player.cpp`、`src/game/GameApp.cpp` |
| 4. 路径提示开关，显示到终点的最短路径 | `P` 键开关；用 Dijkstra 计算带权最短路并画成金色虚线；面板显示步数与代价 | `src/core/PathFinder.cpp`、`src/render/Renderer.cpp` |
| 5. 自设计 20×20 迷宫并测试 | `mazes/maze20.txt`（起点 (2,2)、终点 (18,18)，0 起算）；最短路径 38 步 / 代价 42 | `tools/gen_maze.py`、`mazes/maze20.txt` |

另外附带一个 11×11 演示迷宫 `mazes/maze11_demo.txt`，可用 `1` / `2` 键切换。

## 2. 已完成的进阶任务 1：完整游戏流程

| 说明书要求 | 实现情况 | 代码位置 |
| --- | --- | --- |
| (a) 开始界面，提示按下空格键开始游戏 | 独立的开始画面：标题、呼吸闪烁的“按 空格 键 开始游戏”、操作说明与地块图鉴；按空格（或回车）进入游戏 | `src/render/Renderer.cpp` `drawTitleScreen()`、`src/game/GameApp.cpp` |
| (b) 游戏开始时，小人处于“起点”地块 | 开始游戏 / 重开时调用 `Player::reset(maze.start())`，位置取自迷宫文件里的 `-1` | `src/game/GameApp.cpp` `startGame()` / `resetPlayer()` |
| (c) 小人处于“终点”地块时提示游戏胜利 | 走到终点弹出“到达终点！”遮罩，可按 `R` 重新开始、按空格返回开始界面 | `src/render/Renderer.cpp` `drawWinOverlay()` |

## 3. 已完成的高级算法

| 说明书要求 | 实现情况 | 代码位置 |
| --- | --- | --- |
| (a) 在迷宫中加入怪物，怪物能够自主行走，且每次移动时都向小人当前位置的最短路径方向移动一步 | 20×20 迷宫 2 只、11×11 迷宫 1 只怪物，出生点取“离起点足够远”的格子；每 0.63 秒行动一次（0.18s 移动动画 + 0.45s 冷却）。每次行动用 Dijkstra 求出「怪物 → 小人当前位置」的带权最短路径，沿路径第一格迈一步；若该格被同伴占据，则在四邻格中挑一个“到小人更近”的格子绕行，绕不过去就原地等待。怪物踩到小人即失败（红色遮罩提示），`M` 键可开关怪物 | `src/core/Monster.cpp`、`src/game/GameApp.cpp` `updateMonsters()` |
| (b) 尝试设计生成随机迷宫的算法 | 递归回溯（随机深度优先）生成“完美迷宫”：奇数坐标是通道、偶数坐标是墙，从 (1,1) 出发随机打通相邻格，走到死路回溯。任意两格之间恰有一条通路，因此必然有解。生成后随机撒 3.5% 草地、0.8% 熔岩，起点/终点保持干净；按 `3` 键生成新迷宫，并把结果存成 `mazes/random_<种子>.txt` 便于复现 | `src/core/MazeGenerator.cpp`、`src/game/GameApp.cpp` `loadRandomMaze()` |

![怪物追击](docs/screenshots/06-monsters.png)

![随机迷宫（种子 12345）](docs/screenshots/07-random.png)

![被怪物抓住](docs/screenshots/08-caught.png)

进阶任务 2（草地减速、熔岩失败）与 4（其他趣味功能）尚未实现。

## 4. 运行环境

- Windows 10/11 x64
- **CMake ≥ 3.25**、**Ninja**
- 编译器二选一：
  - MinGW-w64 `g++`（推荐 13 及以上，需在 `PATH` 中），或
  - Visual Studio 2022 的 MSVC（勾选“使用 C++ 的桌面开发”即可，CMake/Ninja 用 VS 自带的）
- Raylib **6.0**：源码已随工程放在 `third_party/raylib-6.0`，编译时由本机编译器一起构建

> `run.bat` 会自动查找 CMake / Ninja：先找 `PATH`，找不到就用 Visual Studio 自带的；
> 检测到 MSVC 时会自动调用 `vcvars64.bat` 准备 `cl.exe` 环境。

## 5. 编译与运行

最简单的方式是双击（或在终端执行）`run.bat`，它会自动完成配置、编译和运行：

```bat
run.bat
```

仓库里还附带了一份已经编译好的 `build\maze_game.exe`（构建产物，已被 `.gitignore` 忽略），
想直接玩可以双击它；改了代码后再运行 `run.bat` 重新编译。

也可以手动执行（以下为 MSVC + Ninja 的写法；MinGW 环境把 cmake/ninja 换成 `PATH` 里的即可）：

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
build\maze_game.exe
```

编译产物 `build/maze_game.exe` 旁边会自动复制 `assets/` 与 `mazes/`，所以直接在 `build` 目录运行也能找到资源。

> **OneDrive 目录提示**：如果工程放在 OneDrive 同步目录里，个别机器上 CMake 配置阶段会异常退出
> （`0xC0000409`）。`run.bat` 检测到配置失败时，会自动把工程复制到 `%PUBLIC%\MazeGameBuild\src`
> 重新构建并运行；也可以手动把工程复制到普通目录（例如 `C:\dev\`）后再编译。

## 6. 操作说明

| 按键 | 功能 |
| --- | --- |
| 空格 / 回车 | 开始界面进入游戏；胜利或被抓后返回开始界面 |
| ↑ ↓ ← → / W A S D | 移动小人（按住可连续行走） |
| P | 显示 / 隐藏最短路径提示 |
| M | 怪物开 / 关（切换后重新布置怪物） |
| R | 重新开始（小人回到起点，怪物重新出生） |
| 1 / 2 | 切换 20×20 迷宫 / 11×11 演示迷宫 |
| 3 | 生成新的随机迷宫（21×21） |
| ESC | 退出游戏 |

路径提示打开时，金色虚线从“小人当前格”连到终点，右侧面板同时显示最短路径的步数与行走代价。
小人走到终点会显示“到达终点！”；被怪物抓住会显示“被怪物抓住了！”。

## 7. 界面截图

![开始界面](docs/screenshots/00-title.png)

| 路径提示关闭 | 路径提示打开（38 步 / 代价 42） |
| --- | --- |
| ![](docs/screenshots/01-basic.png) | ![](docs/screenshots/02-path-hint.png) |

| 走了 18 步后（剩余 20 步 / 代价 20） | 到达终点 |
| --- | --- |
| ![](docs/screenshots/03-walk.png) | ![](docs/screenshots/04-win.png) |

| 11×11 演示迷宫（20 步 / 代价 24） | 怪物追击 |
| --- | --- |
| ![](docs/screenshots/05-maze11.png) | ![](docs/screenshots/06-monsters.png) |

| 随机迷宫（种子 12345） | 被怪物抓住 |
| --- | --- |
| ![](docs/screenshots/07-random.png) | ![](docs/screenshots/08-caught.png) |

## 8. 目录结构

```
迷宫游戏/
├─ CMakeLists.txt          # 构建配置（同时编译 raylib 6.0）
├─ run.bat                 # 一键配置 + 编译 + 运行（自动找 CMake/Ninja/MSVC）
├─ assets/
│  ├─ tiles/               # 地面、墙、草地、熔岩、起点、终点、小人贴图（64×64 PNG）
│  ├─ ui/                  # 标题横幅、窗口图标
│  └─ fonts/simhei.ttf     # 界面中文字体
├─ mazes/
│  ├─ maze20.txt           # 自设计 20×20 迷宫（任务 5）
│  ├─ maze11_demo.txt      # 11×11 演示迷宫
│  └─ random_<种子>.txt    # 游戏中按 3 生成的随机迷宫存档
├─ src/
│  ├─ main.cpp             # 入口：解析参数、创建窗口、设置图标
│  ├─ core/                # 与绘制无关的核心逻辑（可独立单测）
│  │  ├─ Maze.h/.cpp       #   迷宫数据的读取 / 写入 / 校验
│  │  ├─ PathFinder.h/.cpp #   Dijkstra 带权最短路径
│  │  ├─ Player.h/.cpp     #   小人位置、行走规则、换位动画
│  │  ├─ Monster.h/.cpp    #   怪物：沿最短路追击、移动动画与冷却
│  │  └─ MazeGenerator.h/.cpp  # 随机迷宫生成（递归回溯）
│  ├─ game/GameApp.h/.cpp  # 主循环、键盘输入、画面状态、怪物调度
│  ├─ render/              # 渲染层
│  │  ├─ Renderer.*        #   开始界面/迷宫/路径/小人/怪物/状态面板绘制
│  │  ├─ TextRenderer.*    #   中文文字绘制（按需载入字形）
│  │  └─ AssetPaths.*      #   资源路径解析
│  └─ tests/core_tests.cpp # 核心逻辑单元测试
├─ tools/                  # 资源生成与验证脚本（Python，非必需）
│  ├─ gen_assets.py        #   生成地块与界面图片
│  ├─ gen_maze.py          #   生成并校验迷宫文件
│  ├─ render_preview.py    #   把迷宫源文件渲染成 PNG 预览
│  └─ solve_maze.py        #   求迷宫解法并输出方向序列
├─ docs/screenshots/       # 运行截图
└─ third_party/raylib-6.0/ # Raylib 6.0 源码（MIT 许可）
```

## 9. 迷宫源文件格式

与任务说明书第 4 节一致：第一行是 `行数 列数`，之后每行是各列地块类型，用空格分隔。

| 数字 | 含义 | 行走成本 |
| --- | --- | --- |
| -2 | 迷宫终点 | 1 |
| -1 | 迷宫起点 | 1 |
| 0 | 普通地面 | 1 |
| 1 | 墙 | 不可通行 |
| 2 | 草地 | 3 |
| 3 | 熔岩 | 1000 |

读取时会校验：首行尺寸、数据个数、地块取值、起点/终点各恰好一个、行列数是否匹配。
`mazes/maze20.txt` 的第一行是 `20 20`。随机迷宫会用同样的格式写回 `mazes/random_<种子>.txt`。

## 10. 算法说明

### 10.1 最短路径：Dijkstra

`PathFinder::shortestPath` 使用 Dijkstra 算法（优先队列）：

- 节点为全部非墙地块，边为上下左右相邻；
- 进入相邻格的成本由 `Maze::enterCost` 给出：普通/起点/终点 1、草地 3、熔岩 1000；
- 用 `dist[]` 记录最小代价、`prev[]` 记录前驱，终点出队后回溯得到完整路径；
- 返回路径总代价和逐格路径（`PathResult`）。

因为草地上“绕路”可能比“直穿”更便宜，所以 20×20 迷宫中刻意在最短路径上放了两块草地，
界面显示 38 步 / 代价 42（38 步普通成本 38，其中 2 步踩草地，每步多 2）。

### 10.2 怪物追击：复用 Dijkstra 的第一步

`Monster::stepToward` 每次行动都重新计算一次「怪物 → 小人当前位置」的最短路：

1. 用 `PathFinder::shortestPath(maze, 怪物格, 小人格)` 得到完整路径；
2. 取路径第 2 个格子作为下一步，做 0.18 秒的移动动画；
3. 若下一步被另一只怪物占据，就在四邻格中挑选“到小人代价更小”的格子绕行；
   绕不过去则原地等待，避免两只怪物重叠。

因此怪物会“贴着墙”沿最短路径转弯追击，而不是简单地朝小人的方向直线移动；
熔岩（代价 1000）也会被怪物自动绕开，和路径提示的走向一致。

### 10.3 随机迷宫：递归回溯（随机 DFS）

`MazeGenerator::generate` 的过程：

1. 行、列向上取整为奇数，先把整个网格填成墙；
2. 从 (1,1) 出发，把当前格标记为通道，随机选择一个“隔一格”的未访问格，
   打通两者之间的墙并走过去；四个方向都走不通就回溯到上一格；
3. 直到所有奇数坐标的格子都被访问，得到一棵生成树——也就是任意两格之间恰有一条通路的“完美迷宫”；
4. 起点 (1,1)、终点 (行-2, 列-2)，再随机撒草地（3.5%）与熔岩（0.8%）；
5. 用 `std::mt19937` + 种子的随机数生成器，相同种子必然生成完全相同的迷宫；
   游戏中按 `3` 生成的迷宫会存档为 `mazes/random_<种子>.txt`。

`MazeGenerator::isPerfect` 会做 BFS 连通性检查，并验证邻接边数 = 可走格数 - 1（无环），单元测试用它校验生成结果。

## 11. 测试与验证

### 11.1 核心逻辑单元测试（235 项检查）

```bat
build\core_tests.exe
```

覆盖：迷宫解析与各类非法数据、草地/熔岩成本、Dijkstra 绕开熔岩、路径连续性与可通行性、
自设计 20×20 与 11×11 迷宫的最短路径结果（38 步/42 代价、20 步/24 代价）、
小人撞墙/动画期间拒绝移动/重开，以及高级算法部分：

- 怪物在走廊中每走一步到小人的最短代价都严格下降，5 步后抓到小人；
- 怪物不会踩到同伴所在格，被堵住时原地等待；
- 随机迷宫：`isPerfect` 校验（连通 + 无环）、起点/终点唯一、可解、同种子结果完全一致、
  偶数尺寸自动取整为奇数、草地/熔岩确实被撒入。

### 11.2 界面自动化截图与流程验证

```bat
build\maze_game.exe --shot=shot.png --frames=40
build\maze_game.exe --skip-title --path-on --shot=shot.png --frames=40
build\maze_game.exe --random=12345 --skip-title --shot=shot.png --frames=40
```

- `--shot` 渲染若干帧后保存截图并退出，用于检查界面；
- `--skip-title` 跳过开始界面直接进入游戏（截图/调试用）；
- `--random[=种子]` 直接生成随机迷宫启动，`--no-monsters` 关闭怪物；
- `--walk=UDLR...` 启动时按方向序列自动走若干步，例如 `--walk=DDRRRRRRRRDDDDDDDD` 后截图即为“走了 18 步”的状态；
- 开始界面的空格键通过向窗口发送 `WM_KEYDOWN/WM_KEYUP` 验证，确认能正常进入游戏。

`docs/screenshots/` 中的 9 张截图均由这种方式生成（`08-caught.png` 为强制进入失败状态后截图）。

## 12. 说明与后续

- 已完成基础任务 1–5、进阶任务 1，以及高级算法的 (a) 怪物追击与 (b) 随机迷宫生成；
- 进阶任务 2（草地减速、熔岩失败）与 4（其他趣味功能）尚未实现，可按 `Player`、`GameApp`、
  `Renderer` 现有结构继续扩展；
- 图片资源由 `tools/gen_assets.py` 用 Pillow 生成；`simhei.ttf` 取自本机 Windows 字体，
  仅用于本机展示中文界面；`tools/` 脚本不是编译运行所必需的；
- Raylib 6.0 以源码形式附带，遵循其 MIT 许可证。
