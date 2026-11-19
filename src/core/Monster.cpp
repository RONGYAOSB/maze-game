#include "core/Monster.h"

#include <algorithm>
#include <array>
#include <limits>

#include "core/PathFinder.h"

namespace maze {
namespace {

bool isOccupied(const std::vector<Cell>& occupied, Cell cell) {
    return std::find(occupied.begin(), occupied.end(), cell) != occupied.end();
}

}  // namespace

Monster::Monster(float stepSeconds, float restSeconds)
    : stepSeconds_(stepSeconds), restSeconds_(restSeconds) {}

void Monster::reset(Cell position) {
    cell_ = position;
    previous_ = position;
    facing_ = {0, 1};
    progress_ = 1.0f;
    cooldown_ = 0.0f;
    steps_ = 0;
}

void Monster::update(float deltaSeconds) {
    if (progress_ < 1.0f) {
        progress_ += deltaSeconds / stepSeconds_;
        if (progress_ >= 1.0f) {
            // 动画结束；剩余的时间继续用来抵消冷却，避免大 dt 时冷却被拖长
            const float overflow = (progress_ - 1.0f) * stepSeconds_;
            progress_ = 1.0f;
            cooldown_ -= overflow;
        }
    } else if (cooldown_ > 0.0f) {
        cooldown_ -= deltaSeconds;
    }
}

bool Monster::stepToward(const Maze& maze, Cell target, const std::vector<Cell>& occupied) {
    if (!ready()) {
        return false;
    }
    if (cell_ == target) {
        return false;
    }
    // 核心算法：用 Dijkstra 求出“怪物 → 小人当前位置”的带权最短路径，取它的第 2 个格子。
    const PathResult path = PathFinder::shortestPath(maze, cell_, target);
    if (!path.found || path.cells.size() < 2) {
        return false;
    }
    Cell next = path.cells[1];

    // 若最短路的下一步被同伴占据，就在可走邻格中挑一个到目标代价最小、
    // 但必须比现在更近的格子绕过去；绕不过去就原地等待。
    if (isOccupied(occupied, next)) {
        static constexpr std::array<std::pair<int, int>, 4> kDirs = {
            std::pair{-1, 0}, std::pair{1, 0}, std::pair{0, -1}, std::pair{0, 1}};
        int bestCost = path.cost;
        Cell best{};
        bool found = false;
        for (const auto& [dr, dc] : kDirs) {
            const Cell candidate{cell_.row + dr, cell_.col + dc};
            if (!maze.isWalkable(candidate.row, candidate.col)) {
                continue;
            }
            if (isOccupied(occupied, candidate)) {
                continue;
            }
            const PathResult alt = PathFinder::shortestPath(maze, candidate, target);
            if (!alt.found) {
                continue;
            }
            if (alt.cost < bestCost) {
                bestCost = alt.cost;
                best = candidate;
                found = true;
            }
        }
        if (!found) {
            return false;  // 被同伴堵住，原地等待
        }
        next = best;
    }

    previous_ = cell_;
    facing_ = {next.row - cell_.row, next.col - cell_.col};
    cell_ = next;
    progress_ = 0.0f;
    cooldown_ = restSeconds_;
    ++steps_;
    return true;
}

}  // namespace maze
