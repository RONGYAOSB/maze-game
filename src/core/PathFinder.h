#pragma once

#include <vector>

#include "core/Maze.h"

namespace maze {

struct PathResult {
    bool found = false;
    int cost = 0;              // 路径总行走成本（含草地、熔岩的加权成本）
    std::vector<Cell> cells;   // 从起点到终点（含两端）的逐格路径
};

// 最短路求解器：使用 Dijkstra 算法处理带权地图（草地 3、熔岩 1000）。
class PathFinder {
public:
    static PathResult shortestPath(const Maze& maze, Cell from, Cell to);
};

}  // namespace maze
