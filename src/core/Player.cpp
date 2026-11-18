#include "core/Player.h"

namespace maze {

Player::Player(float stepSeconds) : stepSeconds_(stepSeconds) {}

void Player::reset(Cell start) {
    cell_ = start;
    previous_ = start;
    facing_ = {0, 1};
    progress_ = 1.0f;
    steps_ = 0;
}

bool Player::tryStep(const Maze& maze, int dRow, int dCol) {
    if (isMoving()) {
        return false;  // 动画未结束，避免连跳
    }
    if (dRow == 0 && dCol == 0) {
        return false;
    }
    const int targetRow = cell_.row + dRow;
    const int targetCol = cell_.col + dCol;
    if (!maze.isWalkable(targetRow, targetCol)) {
        return false;  // 墙 / 越界
    }
    previous_ = cell_;
    cell_ = {targetRow, targetCol};
    facing_ = {dRow, dCol};
    progress_ = 0.0f;
    ++steps_;
    return true;
}

void Player::update(float deltaSeconds) {
    if (progress_ < 1.0f) {
        progress_ += deltaSeconds / stepSeconds_;
        if (progress_ > 1.0f) {
            progress_ = 1.0f;
        }
    }
}

}  // namespace maze
