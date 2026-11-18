#pragma once

#include <string>

#include "core/Maze.h"
#include "core/PathFinder.h"
#include "core/Player.h"
#include "render/Renderer.h"

namespace game {

inline constexpr int kWindowWidth = 1360;
inline constexpr int kWindowHeight = 900;

struct Options {
    std::string mazePath = "mazes/maze20.txt";
    bool showPath = false;
    bool skipTitle = false;   // 调试/截图用：跳过开始界面直接进入游戏
    int shotAfterFrames = 0;  // >0 时在若干帧后截图并退出（用于自动化验证）
    std::string shotPath;
    std::string walk;  // 调试用：启动时按 "U/D/L/R" 序列自动走几步
};

// 画面状态：开始界面 -> 游戏中
enum class Screen {
    Title,
    Playing,
};

// 游戏主控：组织迷宫、小人、路径提示与界面绘制，并处理键盘输入。
class GameApp {
public:
    bool init(const Options& options, std::string* error);
    int run();
    void shutdown();

    bool loadMaze(const std::string& path, std::string* error = nullptr);

    const maze::Maze& maze() const { return maze_; }
    const maze::Player& player() const { return player_; }

private:
    void handleInput(float deltaSeconds);
    void update(float deltaSeconds);
    void draw();
    void recomputePath();
    void resetPlayer();
    void startGame();

    Options options_;
    maze::Maze maze_;
    maze::Player player_;
    maze::PathResult path_;
    Renderer renderer_;
    Screen screen_ = Screen::Title;
    bool showPath_ = false;
    bool win_ = false;
    float timeSeconds_ = 0.0f;
    float moveCooldown_ = 0.0f;
};

}  // namespace game
