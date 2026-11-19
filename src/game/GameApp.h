#pragma once

#include <string>
#include <vector>

#include "core/Maze.h"
#include "core/MazeGenerator.h"
#include "core/Monster.h"
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
    std::string walk;      // 调试用：启动时按 "U/D/L/R" 序列自动走几步
    bool monsters = true;  // 是否启用追击的怪物
    bool randomMaze = false;
    unsigned seed = 0;  // 随机迷宫种子（0 = 自动生成）
};

// 画面状态：开始界面 -> 游戏中
enum class Screen {
    Title,
    Playing,
};

// 游戏主控：组织迷宫、小人、怪物、路径提示与界面绘制，并处理键盘输入。
class GameApp {
public:
    bool init(const Options& options, std::string* error);
    int run();
    void shutdown();

    bool loadMaze(const std::string& path, std::string* error = nullptr);
    // 生成并切换到一个随机迷宫；seed 为 0 时随机取种子
    bool loadRandomMaze(unsigned seed, std::string* error = nullptr);

    const maze::Maze& maze() const { return maze_; }
    const maze::Player& player() const { return player_; }
    const std::vector<maze::Monster>& monsters() const { return monsters_; }

private:
    void handleInput(float deltaSeconds);
    void update(float deltaSeconds);
    void draw();
    void recomputePath();
    void resetPlayer();
    void startGame();
    void restart();
    void spawnMonsters();
    void updateMonsters(float deltaSeconds);
    void checkMonsterCatch();

    Options options_;
    maze::Maze maze_;
    maze::Player player_;
    std::vector<maze::Monster> monsters_;
    maze::PathResult path_;
    Renderer renderer_;
    Screen screen_ = Screen::Title;
    bool showPath_ = false;
    bool win_ = false;
    bool caught_ = false;
    bool monstersEnabled_ = true;
    unsigned seed_ = 0;
    float timeSeconds_ = 0.0f;
    float moveCooldown_ = 0.0f;
};

}  // namespace game
