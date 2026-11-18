#include "game/GameApp.h"

#include <string>

#include "raylib.h"
#include "render/AssetPaths.h"

namespace game {

bool GameApp::init(const Options& options, std::string* error) {
    options_ = options;
    showPath_ = options.showPath;
    screen_ = options.skipTitle ? Screen::Playing : Screen::Title;

    const std::string resolved = resolveAssetPath(options.mazePath);
    if (!maze_.load(resolved, error)) {
        return false;
    }
    player_.reset(maze_.start());
    win_ = false;

    for (const char step : options_.walk) {
        int dRow = 0;
        int dCol = 0;
        switch (step) {
            case 'U': case 'u': dRow = -1; break;
            case 'D': case 'd': dRow = 1; break;
            case 'L': case 'l': dCol = -1; break;
            case 'R': case 'r': dCol = 1; break;
            default: break;
        }
        if ((dRow != 0 || dCol != 0) && player_.tryStep(maze_, dRow, dCol)) {
            player_.update(1.0f);  // 动画立即结束，便于连续走
        }
    }
    win_ = (player_.cell() == maze_.end());

    if (!renderer_.loadAssets(error)) {
        return false;
    }

    recomputePath();
    return true;
}

bool GameApp::loadMaze(const std::string& path, std::string* error) {
    maze::Maze candidate;
    std::string loadError;
    if (!candidate.load(resolveAssetPath(path), &loadError)) {
        TraceLog(LOG_WARNING, "切换迷宫失败：%s", loadError.c_str());
        if (error != nullptr) {
            *error = loadError;
        }
        return false;
    }
    maze_ = std::move(candidate);
    resetPlayer();
    return true;
}

void GameApp::resetPlayer() {
    player_.reset(maze_.start());
    win_ = false;
    moveCooldown_ = 0.0f;
    recomputePath();
}

void GameApp::recomputePath() {
    path_ = maze::PathFinder::shortestPath(maze_, player_.cell(), maze_.end());
}

void GameApp::handleInput(float deltaSeconds) {
    if (IsKeyPressed(KEY_P)) {
        showPath_ = !showPath_;
    }
    if (IsKeyPressed(KEY_R)) {
        resetPlayer();
    }
    if (IsKeyPressed(KEY_ONE)) {
        loadMaze("mazes/maze20.txt");
    }
    if (IsKeyPressed(KEY_TWO)) {
        loadMaze("mazes/maze11_demo.txt");
    }

    if (player_.isMoving()) {
        return;
    }

    int dRow = 0;
    int dCol = 0;
    bool pressed = false;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        dRow = -1;
        pressed = true;
    } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        dRow = 1;
        pressed = true;
    } else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        dCol = -1;
        pressed = true;
    } else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        dCol = 1;
        pressed = true;
    }

    moveCooldown_ -= deltaSeconds;
    if (!pressed) {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
            dRow = -1;
        } else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
            dRow = 1;
        } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            dCol = -1;
        } else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            dCol = 1;
        }
        if ((dRow == 0 && dCol == 0) || moveCooldown_ > 0.0f) {
            return;
        }
    }

    if (dRow == 0 && dCol == 0) {
        return;
    }

    if (player_.tryStep(maze_, dRow, dCol)) {
        moveCooldown_ = 0.15f;
        win_ = (player_.cell() == maze_.end());
        recomputePath();
    } else {
        moveCooldown_ = 0.10f;  // 撞墙后短暂停顿，避免按住时抖动
    }
}

void GameApp::update(float deltaSeconds) {
    player_.update(deltaSeconds);
    timeSeconds_ += deltaSeconds;
}

void GameApp::draw() {
    BeginDrawing();
    if (screen_ == Screen::Title) {
        renderer_.drawTitleScreen(timeSeconds_);
    } else {
        renderer_.draw(maze_, player_, path_, showPath_, timeSeconds_, win_);
    }
    EndDrawing();
}

int GameApp::run() {
    int frames = 0;
    while (!WindowShouldClose()) {
        const float deltaSeconds = GetFrameTime();
        if (screen_ == Screen::Title) {
            // 开始界面：按空格（或回车）开始游戏
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
                startGame();
            }
        } else {
            handleInput(deltaSeconds);
            // 胜利后：空格返回开始界面
            if (win_ && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))) {
                screen_ = Screen::Title;
                resetPlayer();
            }
        }
        update(deltaSeconds);
        draw();

        ++frames;
        if (options_.shotAfterFrames > 0 && frames >= options_.shotAfterFrames) {
            if (!options_.shotPath.empty()) {
                TakeScreenshot(options_.shotPath.c_str());
            }
            break;
        }
    }
    return 0;
}

void GameApp::startGame() {
    resetPlayer();
    screen_ = Screen::Playing;
}

void GameApp::shutdown() {
    renderer_.unload();
}

}  // namespace game
