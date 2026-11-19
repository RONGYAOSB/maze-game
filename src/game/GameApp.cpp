#include "game/GameApp.h"

#include <algorithm>
#include <array>
#include <deque>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include "raylib.h"
#include "render/AssetPaths.h"

namespace game {

bool GameApp::init(const Options& options, std::string* error) {
    options_ = options;
    showPath_ = options.showPath;
    monstersEnabled_ = options.monsters;
    screen_ = options.skipTitle ? Screen::Playing : Screen::Title;
    seed_ = options.seed;

    std::string loadError;
    if (options_.randomMaze) {
        if (!loadRandomMaze(options_.seed, &loadError)) {
            if (error != nullptr) {
                *error = loadError;
            }
            return false;
        }
    } else if (!maze_.load(resolveAssetPath(options_.mazePath), &loadError)) {
        if (error != nullptr) {
            *error = loadError;
        }
        return false;
    }

    player_.reset(maze_.start());
    win_ = false;
    caught_ = false;
    spawnMonsters();

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

    checkMonsterCatch();
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
    seed_ = 0;
    restart();
    return true;
}

bool GameApp::loadRandomMaze(unsigned seed, std::string* error) {
    maze::GenerateOptions generateOptions;
    generateOptions.rows = 21;
    generateOptions.cols = 21;
    generateOptions.seed = seed;

    unsigned usedSeed = 0;
    maze::Maze candidate = maze::MazeGenerator::generate(generateOptions, &usedSeed);
    if (candidate.rows() <= 0 || candidate.cols() <= 0) {
        if (error != nullptr) {
            *error = "随机迷宫生成失败";
        }
        return false;
    }
    maze_ = std::move(candidate);
    seed_ = usedSeed;

    // 存档一份 txt，便于复现和提交作业
    const std::string folder = "mazes";
    const std::string file = folder + "/random_" + std::to_string(usedSeed) + ".txt";
    std::error_code ec;
    std::filesystem::create_directories(folder, ec);
    std::string saveError;
    if (!maze_.save(file, &saveError)) {
        TraceLog(LOG_WARNING, "随机迷宫存档失败：%s", saveError.c_str());
    } else {
        TraceLog(LOG_INFO, "随机迷宫已保存到 %s（种子 %u）", file.c_str(), usedSeed);
    }
    return true;
}

void GameApp::restart() {
    player_.reset(maze_.start());
    win_ = false;
    caught_ = false;
    moveCooldown_ = 0.0f;
    spawnMonsters();
    recomputePath();
}

void GameApp::resetPlayer() {
    player_.reset(maze_.start());
    win_ = false;
    caught_ = false;
    moveCooldown_ = 0.0f;
    recomputePath();
}

void GameApp::recomputePath() {
    path_ = maze::PathFinder::shortestPath(maze_, player_.cell(), maze_.end());
}

// 怪物出生点：优先挑“离起点足够远”的可走格，让小人有反应时间。
void GameApp::spawnMonsters() {
    monsters_.clear();
    if (!monstersEnabled_) {
        return;
    }

    const int rows = maze_.rows();
    const int cols = maze_.cols();
    if (rows <= 0 || cols <= 0) {
        return;
    }

    // 从起点做 BFS，得到每一格的最短步数，作为“距离起点有多远”的度量
    std::vector<int> distance(static_cast<size_t>(rows) * static_cast<size_t>(cols), -1);
    std::deque<maze::Cell> queue;
    const maze::Cell start = maze_.start();
    distance[static_cast<size_t>(start.row) * cols + static_cast<size_t>(start.col)] = 0;
    queue.push_back(start);

    static constexpr std::array<std::pair<int, int>, 4> kDirs = {
        std::pair{-1, 0}, std::pair{1, 0}, std::pair{0, -1}, std::pair{0, 1}};
    while (!queue.empty()) {
        const maze::Cell current = queue.front();
        queue.pop_front();
        const int base = distance[static_cast<size_t>(current.row) * cols +
                                  static_cast<size_t>(current.col)];
        for (const auto& [dr, dc] : kDirs) {
            const int nr = current.row + dr;
            const int nc = current.col + dc;
            if (!maze_.isWalkable(nr, nc)) {
                continue;
            }
            int& slot = distance[static_cast<size_t>(nr) * cols + static_cast<size_t>(nc)];
            if (slot < 0) {
                slot = base + 1;
                queue.push_back({nr, nc});
            }
        }
    }

    std::mt19937 rng(seed_ != 0 ? seed_ : 20260101u);
    const int wanted = (rows >= 15 && cols >= 15) ? 2 : 1;

    const auto collect = [&](int minDistance) {
        std::vector<maze::Cell> pool;
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                if (!maze_.isWalkable(row, col)) {
                    continue;
                }
                const maze::Cell cell{row, col};
                if (cell == maze_.end() || cell == start) {
                    continue;
                }
                const int d = distance[static_cast<size_t>(row) * cols + static_cast<size_t>(col)];
                if (d >= minDistance) {
                    pool.push_back(cell);
                }
            }
        }
        return pool;
    };

    std::vector<maze::Cell> pool = collect(std::max(12, rows / 2));
    if (static_cast<int>(pool.size()) < wanted) {
        pool = collect(8);
    }
    if (static_cast<int>(pool.size()) < wanted) {
        pool = collect(4);
    }
    if (pool.empty()) {
        return;
    }
    std::shuffle(pool.begin(), pool.end(), rng);
    for (int i = 0; i < wanted && i < static_cast<int>(pool.size()); ++i) {
        maze::Monster monster(0.18f, 0.45f);
        monster.reset(pool[static_cast<size_t>(i)]);
        monsters_.push_back(monster);
    }
}

// 每只怪物独立计时：到位后沿“自己 → 小人”的最短路迈一步。
void GameApp::updateMonsters(float deltaSeconds) {
    for (maze::Monster& monster : monsters_) {
        monster.update(deltaSeconds);
    }
    if (!monstersEnabled_ || win_ || caught_) {
        return;
    }

    for (size_t i = 0; i < monsters_.size(); ++i) {
        if (!monsters_[i].ready()) {
            continue;
        }
        std::vector<maze::Cell> others;
        others.reserve(monsters_.size());
        for (size_t j = 0; j < monsters_.size(); ++j) {
            if (j != i) {
                others.push_back(monsters_[j].cell());
            }
        }
        monsters_[i].stepToward(maze_, player_.cell(), others);
    }
    checkMonsterCatch();
}

void GameApp::checkMonsterCatch() {
    if (caught_) {
        return;
    }
    for (const maze::Monster& monster : monsters_) {
        if (monster.cell() == player_.cell()) {
            caught_ = true;
            win_ = false;
            return;
        }
    }
}

void GameApp::handleInput(float deltaSeconds) {
    if (IsKeyPressed(KEY_P)) {
        showPath_ = !showPath_;
    }
    if (IsKeyPressed(KEY_M)) {
        monstersEnabled_ = !monstersEnabled_;
        spawnMonsters();
    }
    if (IsKeyPressed(KEY_R)) {
        restart();
        return;
    }
    if (IsKeyPressed(KEY_ONE)) {
        loadMaze("mazes/maze20.txt");
        return;
    }
    if (IsKeyPressed(KEY_TWO)) {
        loadMaze("mazes/maze11_demo.txt");
        return;
    }
    if (IsKeyPressed(KEY_THREE)) {
        if (loadRandomMaze(0)) {
            restart();
        }
        return;
    }

    if (win_ || caught_) {
        return;
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
        checkMonsterCatch();
        recomputePath();
    } else {
        moveCooldown_ = 0.10f;  // 撞墙后短暂停顿，避免按住时抖动
    }
}

void GameApp::update(float deltaSeconds) {
    player_.update(deltaSeconds);
    updateMonsters(deltaSeconds);
    timeSeconds_ += deltaSeconds;
}

void GameApp::draw() {
    BeginDrawing();
    if (screen_ == Screen::Title) {
        renderer_.drawTitleScreen(timeSeconds_);
    } else {
        renderer_.draw(maze_, player_, monsters_, path_, showPath_, timeSeconds_, win_, caught_,
                       monstersEnabled_);
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
            // 胜利/被抓后：空格返回开始界面
            if ((win_ || caught_) && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))) {
                screen_ = Screen::Title;
                restart();
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
    restart();
    screen_ = Screen::Playing;
}

void GameApp::shutdown() {
    renderer_.unload();
}

}  // namespace game
