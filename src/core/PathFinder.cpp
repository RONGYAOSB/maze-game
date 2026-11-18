#include "core/PathFinder.h"

#include <array>
#include <functional>
#include <limits>
#include <queue>
#include <utility>

namespace maze {
namespace {

constexpr int kUnreachable = std::numeric_limits<int>::max();

}  // namespace

PathResult PathFinder::shortestPath(const Maze& maze, Cell from, Cell to) {
    PathResult result;
    if (!maze.inBounds(from.row, from.col) || !maze.inBounds(to.row, to.col)) {
        return result;
    }
    if (maze.isWall(from.row, from.col) || maze.isWall(to.row, to.col)) {
        return result;
    }
    if (from == to) {
        result.found = true;
        result.cost = 0;
        result.cells.push_back(from);
        return result;
    }

    const int rows = maze.rows();
    const int cols = maze.cols();
    const int total = rows * cols;
    const auto index = [cols](int row, int col) { return row * cols + col; };

    std::vector<int> dist(static_cast<size_t>(total), kUnreachable);
    std::vector<int> prev(static_cast<size_t>(total), -1);

    using Entry = std::pair<int, int>;  // (cost, index)
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> queue;

    const int sourceIndex = index(from.row, from.col);
    dist[static_cast<size_t>(sourceIndex)] = 0;
    queue.push({0, sourceIndex});

    static constexpr std::array<std::pair<int, int>, 4> kDirs = {
        std::pair{-1, 0}, std::pair{1, 0}, std::pair{0, -1}, std::pair{0, 1}};

    while (!queue.empty()) {
        const auto [cost, node] = queue.top();
        queue.pop();
        if (cost > dist[static_cast<size_t>(node)]) {
            continue;
        }
        if (node == index(to.row, to.col)) {
            break;
        }
        const int row = node / cols;
        const int col = node % cols;
        for (const auto& [dr, dc] : kDirs) {
            const int nr = row + dr;
            const int nc = col + dc;
            if (!maze.isWalkable(nr, nc)) {
                continue;
            }
            const int next = index(nr, nc);
            const int candidate = cost + maze.enterCost(nr, nc);
            if (candidate < dist[static_cast<size_t>(next)]) {
                dist[static_cast<size_t>(next)] = candidate;
                prev[static_cast<size_t>(next)] = node;
                queue.push({candidate, next});
            }
        }
    }

    const int targetIndex = index(to.row, to.col);
    if (dist[static_cast<size_t>(targetIndex)] == kUnreachable) {
        return result;
    }

    result.found = true;
    result.cost = dist[static_cast<size_t>(targetIndex)];

    // 通过前驱数组回溯路径
    std::vector<Cell> reversed;
    for (int node = targetIndex; node != -1; node = prev[static_cast<size_t>(node)]) {
        reversed.push_back({node / cols, node % cols});
        if (node == sourceIndex) {
            break;
        }
    }
    result.cells.assign(reversed.rbegin(), reversed.rend());
    if (result.cells.empty() || result.cells.front() != from || result.cells.back() != to) {
        result.found = false;
        result.cells.clear();
        result.cost = 0;
    }
    return result;
}

}  // namespace maze
