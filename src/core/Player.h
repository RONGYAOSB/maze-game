#pragma once

#include "core/Maze.h"

namespace maze {

// 小人：保存所在格子，并把“上一步 -> 当前格”的换位过程做成短动画。
// 只负责自身状态，输入与碰撞规则由 GameApp 调用 tryStep 决定。
class Player {
public:
    explicit Player(float stepSeconds = 0.10f);

    void reset(Cell start);

    Cell cell() const { return cell_; }
    Cell previous() const { return previous_; }
    Cell facing() const { return facing_; }  // 最近一次移动方向，供绘制使用

    bool isMoving() const { return progress_ < 1.0f; }
    float progress() const { return progress_; }  // 0.0 -> 1.0

    int steps() const { return steps_; }

    // 若目标格在迷宫内且不是墙，则迈出一步并返回 true；否则返回 false。
    bool tryStep(const Maze& maze, int dRow, int dCol);

    void update(float deltaSeconds);

private:
    Cell cell_{};
    Cell previous_{};
    Cell facing_{0, 1};
    float progress_ = 1.0f;
    float stepSeconds_ = 0.10f;
    int steps_ = 0;
};

}  // namespace maze
