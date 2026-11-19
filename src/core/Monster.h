#pragma once

#include <vector>

#include "core/Maze.h"

namespace maze {

// 怪物：每次行动都沿“自己 → 小人当前位置”的最短路径迈出一步。
// 与小人类似，移动过程做成短动画，渲染层可以复用同一套插值方式。
class Monster {
public:
    explicit Monster(float stepSeconds = 0.20f, float restSeconds = 0.40f);

    void reset(Cell position);

    Cell cell() const { return cell_; }
    Cell previous() const { return previous_; }
    Cell facing() const { return facing_; }
    bool isMoving() const { return progress_ < 1.0f; }
    float progress() const { return progress_; }  // 0.0 -> 1.0
    int steps() const { return steps_; }

    // 动画结束且冷却完毕，才可以开始下一次行动
    bool ready() const { return !isMoving() && cooldown_ <= 0.0f; }

    // 向 target 走一步（沿 Dijkstra 最短路）。occupied 中的格子视为暂时不可进入；
    // 返回是否真的移动了。若已走到 target 上（抓住小人）同样返回 true。
    bool stepToward(const Maze& maze, Cell target, const std::vector<Cell>& occupied = {});

    void update(float deltaSeconds);

private:
    Cell cell_{};
    Cell previous_{};
    Cell facing_{0, 1};
    float progress_ = 1.0f;
    float cooldown_ = 0.0f;     // 两次移动之间的休息时间
    float stepSeconds_ = 0.20f; // 单格移动动画时长
    float restSeconds_ = 0.40f;
    int steps_ = 0;
};

}  // namespace maze
