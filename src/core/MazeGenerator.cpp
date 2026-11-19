#include "core/MazeGenerator.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <deque>
#include <random>
#include <vector>

namespace maze {
namespace {

// 奇数化：行、列都必须为奇数，保证“奇数坐标是通道、偶数坐标是墙”的网格成立。
int oddSize(int value, int minimum) {
    value = std::max(value, minimum);
    if (value % 2 == 0) {
        ++value;
    }
    return value;
}

std::string titleForImpl(int rows, int cols, unsigned seed) {
    char buffer[96];
    std::snprintf(buffer, sizeof(buffer), "随机 %dx%d 种子 %u", rows, cols, seed);
    return std::string(buffer);
}

}  // namespace

std::string MazeGenerator::titleFor(int rows, int cols, unsigned seed) {
    return titleForImpl(rows, cols, seed);
}

Maze MazeGenerator::generate(int rows, int cols, unsigned seed) {
    GenerateOptions options;
    options.rows = rows;
    options.cols = cols;
    options.seed = seed;
    return generate(options);
}

Maze MazeGenerator::generate(const GenerateOptions& optionsIn, unsigned* usedSeed) {
    GenerateOptions options = optionsIn;
    options.rows = oddSize(std::min(options.rows, 399), 5);
    options.cols = oddSize(std::min(options.cols, 399), 5);

    unsigned seed = options.seed;
    if (seed == 0) {
        seed = std::random_device{}();
        if (seed == 0) {
            seed = 20260101u;
        }
    }
    if (usedSeed != nullptr) {
        *usedSeed = seed;
    }

    std::mt19937 rng(seed);
    const int rows = options.rows;
    const int cols = options.cols;
    std::vector<Tile> tiles(static_cast<size_t>(rows) * static_cast<size_t>(cols), Tile::Wall);
    const auto at = [&tiles, cols](int row, int col) -> Tile& {
        return tiles[static_cast<size_t>(row) * static_cast<size_t>(cols) +
                     static_cast<size_t>(col)];
    };

    // ---- 递归回溯（随机深度优先）：从 (1,1) 出发，每次随机挑一个“隔一格”的未访问格，
    //      打通中间的墙，然后继续深入；走到死路就退回上一个格。 ----
    static constexpr std::array<std::pair<int, int>, 4> kStep = {
        std::pair{-2, 0}, std::pair{2, 0}, std::pair{0, -2}, std::pair{0, 2}};

    at(1, 1) = Tile::Ground;
    std::vector<Cell> stack;
    stack.push_back({1, 1});
    std::array<int, 4> order{0, 1, 2, 3};

    while (!stack.empty()) {
        const Cell current = stack.back();
        std::shuffle(order.begin(), order.end(), rng);
        bool carved = false;
        for (const int index : order) {
            const int nextRow = current.row + kStep[static_cast<size_t>(index)].first;
            const int nextCol = current.col + kStep[static_cast<size_t>(index)].second;
            if (nextRow <= 0 || nextRow >= rows - 1 || nextCol <= 0 || nextCol >= cols - 1) {
                continue;
            }
            if (at(nextRow, nextCol) != Tile::Wall) {
                continue;
            }
            at((current.row + nextRow) / 2, (current.col + nextCol) / 2) = Tile::Ground;
            at(nextRow, nextCol) = Tile::Ground;
            stack.push_back({nextRow, nextCol});
            carved = true;
            break;
        }
        if (!carved) {
            stack.pop_back();
        }
    }

    const Cell start{1, 1};
    const Cell end{rows - 2, cols - 2};
    at(start.row, start.col) = Tile::Start;
    at(end.row, end.col) = Tile::End;

    // ---- 撒一些草地与熔岩，让最短路求解用到带权地图；起点、终点保持干净 ----
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    for (int row = 1; row < rows - 1; ++row) {
        for (int col = 1; col < cols - 1; ++col) {
            if (at(row, col) != Tile::Ground) {
                continue;
            }
            if ((row == start.row && col == start.col) || (row == end.row && col == end.col)) {
                continue;
            }
            const double dice = unit(rng);
            if (dice < options.lavaRatio) {
                at(row, col) = Tile::Lava;
            } else if (dice < options.lavaRatio + options.grassRatio) {
                at(row, col) = Tile::Grass;
            }
        }
    }

    Maze maze;
    maze.assign(rows, cols, std::move(tiles), start, end, titleForImpl(rows, cols, seed));
    return maze;
}

bool MazeGenerator::isPerfect(const Maze& maze) {
    const int rows = maze.rows();
    const int cols = maze.cols();
    if (rows <= 0 || cols <= 0) {
        return false;
    }

    int walkable = 0;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (maze.isWalkable(row, col)) {
                ++walkable;
            }
        }
    }
    if (walkable == 0) {
        return false;
    }

    // 连通性：从起点做 BFS
    std::vector<int> visited(static_cast<size_t>(rows) * static_cast<size_t>(cols), 0);
    std::deque<Cell> queue;
    queue.push_back(maze.start());
    visited[static_cast<size_t>(maze.start().row) * static_cast<size_t>(cols) +
            static_cast<size_t>(maze.start().col)] = 1;
    int reached = 0;
    int edges = 0;  // 只统计“向右/向下”的边，避免重复
    static constexpr std::array<std::pair<int, int>, 2> kHalfDirs = {
        std::pair{0, 1}, std::pair{1, 0}};
    while (!queue.empty()) {
        const Cell current = queue.front();
        queue.pop_front();
        ++reached;
        for (const auto& [dr, dc] : kHalfDirs) {
            const int nr = current.row + dr;
            const int nc = current.col + dc;
            if (maze.isWalkable(nr, nc)) {
                ++edges;
            }
        }
        static constexpr std::array<std::pair<int, int>, 4> kDirs = {
            std::pair{-1, 0}, std::pair{1, 0}, std::pair{0, -1}, std::pair{0, 1}};
        for (const auto& [dr, dc] : kDirs) {
            const int nr = current.row + dr;
            const int nc = current.col + dc;
            if (!maze.isWalkable(nr, nc)) {
                continue;
            }
            int& flag = visited[static_cast<size_t>(nr) * static_cast<size_t>(cols) +
                                static_cast<size_t>(nc)];
            if (flag == 0) {
                flag = 1;
                queue.push_back({nr, nc});
            }
        }
    }

    // 连通的完美迷宫：所有可走格都能到达，且边数 = 格数 - 1（无环）
    return reached == walkable && edges == walkable - 1;
}

}  // namespace maze
