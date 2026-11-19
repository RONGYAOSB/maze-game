#pragma once

#include <string>
#include <vector>

#include "core/Maze.h"
#include "core/Monster.h"
#include "core/PathFinder.h"
#include "core/Player.h"
#include "raylib.h"
#include "render/TextRenderer.h"

namespace game {

struct MazeLayout {
    int tile = 32;
    int originX = 0;
    int originY = 0;
    Rectangle mazeRect{};
    Rectangle panelRect{};
};

// 渲染器：只负责把迷宫、路径、小人、怪物、状态面板画到屏幕上，不修改游戏状态。
class Renderer {
public:
    bool loadAssets(std::string* error = nullptr);
    void unload();

    void draw(const maze::Maze& maze, const maze::Player& player,
              const std::vector<maze::Monster>& monsters, const maze::PathResult& path,
              bool showPath, float timeSeconds, bool win, bool caught, bool monstersEnabled);

    // 开始界面（进阶任务 1：按空格开始游戏）
    void drawTitleScreen(float timeSeconds);

    const MazeLayout& layout() const { return layout_; }

private:
    void computeLayout(int windowWidth, int windowHeight, int rows, int cols);
    Texture2D* textureFor(maze::Tile tile);
    void drawTile(maze::Tile tile, float x, float y, float size);
    void drawMaze(const maze::Maze& maze);
    void drawPath(const maze::PathResult& path, float timeSeconds);
    void drawPlayer(const maze::Player& player);
    void drawMonsters(const std::vector<maze::Monster>& monsters, float timeSeconds);
    void drawMonster(const maze::Monster& monster, float timeSeconds);
    void drawWinOverlay();
    void drawCaughtOverlay();
    void drawPanel(const maze::Maze& maze, const maze::Player& player,
                   const maze::PathResult& path, bool showPath, bool monstersEnabled,
                   int monsterCount);
    void drawLegendRow(float x, float y, maze::Tile tile, const char* label, const char* value);

    Texture2D ground_{};
    Texture2D wall_{};
    Texture2D grass_{};
    Texture2D lava_{};
    Texture2D start_{};
    Texture2D end_{};
    Texture2D player_{};
    Texture2D banner_{};
    TextRenderer text_;
    MazeLayout layout_;
    bool loaded_ = false;
};

}  // namespace game
