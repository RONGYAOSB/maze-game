#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/Maze.h"
#include "core/MazeGenerator.h"
#include "core/Monster.h"
#include "core/PathFinder.h"
#include "core/Player.h"

namespace {

int g_checks = 0;
int g_failures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            ++g_failures;                                                        \
            std::printf("[FAIL] %s:%d  %s\n", __FILE__, __LINE__, #cond);        \
        }                                                                        \
    } while (false)

#define CHECK_EQ(actual, expected)                                                       \
    do {                                                                                 \
        ++g_checks;                                                                      \
        const auto actualValue = (actual);                                               \
        const auto expectedValue = (expected);                                           \
        if (!(actualValue == expectedValue)) {                                           \
            ++g_failures;                                                                \
            std::printf("[FAIL] %s:%d  %s == %s  (actual=%lld expected=%lld)\n",         \
                        __FILE__, __LINE__, #actual, #expected,                          \
                        static_cast<long long>(actualValue),                             \
                        static_cast<long long>(expectedValue));                          \
        }                                                                                \
    } while (false)

std::filesystem::path g_tempDir;

std::string writeTempMaze(const std::string& name, const std::string& content) {
    const std::filesystem::path path = g_tempDir / name;
    std::ofstream out(path);
    out << content;
    out.close();
    return path.string();
}

void testParseAndGrassCost() {
    const std::string path = writeTempMaze("grass_line.txt",
                                           "3 5\n"
                                           "1 1 1 1 1\n"
                                           "-1 0 2 0 -2\n"
                                           "1 1 1 1 1\n");
    maze::Maze maze;
    std::string error;
    CHECK(maze.load(path, &error));
    CHECK(error.empty());
    CHECK_EQ(maze.rows(), 3);
    CHECK_EQ(maze.cols(), 5);
    CHECK_EQ(static_cast<int>(maze.tile(1, 0)), static_cast<int>(maze::Tile::Start));
    CHECK_EQ(static_cast<int>(maze.tile(1, 4)), static_cast<int>(maze::Tile::End));
    CHECK_EQ(maze.enterCost(1, 1), 1);
    CHECK_EQ(maze.enterCost(1, 2), 3);
    CHECK(maze.isWalkable(1, 2));
    CHECK(!maze.isWalkable(0, 0));

    const maze::PathResult result = maze::PathFinder::shortestPath(maze, maze.start(), maze.end());
    CHECK(result.found);
    CHECK_EQ(result.cost, 6);  // 1 + 3 + 1 + 1
    CHECK_EQ(static_cast<int>(result.cells.size()), 5);
    CHECK(result.cells.front() == maze.start());
    CHECK(result.cells.back() == maze.end());
}

void testLavaAvoided() {
    const std::string path = writeTempMaze("lava_detour.txt",
                                           "5 5\n"
                                           "1 1 1 1 1\n"
                                           "1 -1 3 0 -2\n"
                                           "1 0 0 0 1\n"
                                           "1 1 1 1 1\n"
                                           "1 1 1 1 1\n");
    maze::Maze maze;
    CHECK(maze.load(path));
    CHECK_EQ(maze.enterCost(1, 2), 1000);

    const maze::PathResult result = maze::PathFinder::shortestPath(maze, maze.start(), maze.end());
    CHECK(result.found);
    CHECK_EQ(result.cost, 5);  // 绕开熔岩：1+1+1+1+1
    CHECK_EQ(static_cast<int>(result.cells.size()), 6);
    for (const maze::Cell& cell : result.cells) {
        CHECK(maze.tile(cell.row, cell.col) != maze::Tile::Lava);
    }
}

void testPlayerRules() {
    const std::string path = writeTempMaze("player.txt",
                                           "3 5\n"
                                           "1 1 1 1 1\n"
                                           "-1 0 2 0 -2\n"
                                           "1 1 1 1 1\n");
    maze::Maze maze;
    CHECK(maze.load(path));

    maze::Player player;
    player.reset(maze.start());
    CHECK(!player.isMoving());
    CHECK_EQ(player.steps(), 0);

    CHECK(!player.tryStep(maze, -1, 0));  // 上方是墙
    CHECK_EQ(player.steps(), 0);
    CHECK(player.cell() == maze.start());

    CHECK(player.tryStep(maze, 0, 1));  // 向右是草地
    CHECK_EQ(player.steps(), 1);
    CHECK(player.isMoving());
    CHECK((player.cell() == maze::Cell{1, 1}));

    CHECK(!player.tryStep(maze, 0, 1));  // 动画期间拒绝移动
    CHECK_EQ(player.steps(), 1);

    player.update(0.05f);  // 动画进行中
    CHECK(player.progress() > 0.0f && player.progress() < 1.0f);
    player.update(0.5f);
    CHECK(!player.isMoving());
    CHECK(player.progress() >= 1.0f);

    CHECK(player.tryStep(maze, 0, 1));  // 动画结束后可以继续走
    CHECK_EQ(player.steps(), 2);

    player.reset(maze.start());
    CHECK_EQ(player.steps(), 0);
    CHECK(player.cell() == maze.start());
}

void testInvalidFiles() {
    maze::Maze maze;
    std::string error;

    const std::string extra = writeTempMaze("bad_extra.txt", "3 3\n0 0 0\n0 0 0\n0 0 0\n0\n");
    CHECK(!maze.load(extra, &error));
    CHECK(!error.empty());

    const std::string missingStart = writeTempMaze("bad_no_start.txt", "3 3\n0 0 0\n0 0 0\n0 0 -2\n");
    CHECK(!maze.load(missingStart, &error));

    const std::string badValue = writeTempMaze("bad_value.txt", "3 3\n-1 0 5\n0 0 0\n0 0 -2\n");
    CHECK(!maze.load(badValue, &error));

    const std::string badSize = writeTempMaze("bad_size.txt", "3 3\n-1 0\n0 0 0\n0 0 -2\n");
    CHECK(!maze.load(badSize, &error));

    const std::string missingEnd = writeTempMaze("bad_no_end.txt", "3 3\n-1 0 0\n0 0 0\n0 0 0\n");
    CHECK(!maze.load(missingEnd, &error));

    CHECK(!maze.load("no_such_maze_file.txt", &error));
}

std::string findShippedMaze(const char* name) {
    const std::vector<std::string> candidates = {
        std::string("mazes/") + name,
        std::string("../mazes/") + name,
        std::string(MAZE_SOURCE_DIR) + "/mazes/" + name,
        std::string("../../mazes/") + name,
    };
    for (const std::string& candidate : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(std::filesystem::path(candidate), ec)) {
            return candidate;
        }
    }
    return candidates.front();
}

void testShippedMazes() {
    maze::Maze maze20;
    std::string error;
    CHECK(maze20.load(findShippedMaze("maze20.txt"), &error));
    if (error.empty()) {
        CHECK_EQ(maze20.rows(), 20);
        CHECK_EQ(maze20.cols(), 20);
        CHECK_EQ(maze20.count(maze::Tile::Start), 1);
        CHECK_EQ(maze20.count(maze::Tile::End), 1);
        CHECK_EQ(maze20.count(maze::Tile::Grass), 15);
        CHECK_EQ(maze20.count(maze::Tile::Lava), 6);

        const maze::PathResult result =
            maze::PathFinder::shortestPath(maze20, maze20.start(), maze20.end());
        CHECK(result.found);
        CHECK_EQ(result.cost, 42);
        CHECK_EQ(static_cast<int>(result.cells.size()), 39);  // 38 步
        CHECK_EQ(static_cast<int>(result.cells.size()) - 1, 38);
        CHECK(result.cells.front() == maze20.start());
        CHECK(result.cells.back() == maze20.end());

        // 路径合法性：连续、可通行、不踩熔岩
        int grassOnPath = 0;
        for (size_t i = 0; i < result.cells.size(); ++i) {
            const maze::Cell& cell = result.cells[i];
            CHECK(maze20.isWalkable(cell.row, cell.col));
            CHECK(maze20.tile(cell.row, cell.col) != maze::Tile::Lava);
            if (maze20.tile(cell.row, cell.col) == maze::Tile::Grass) {
                ++grassOnPath;
            }
            if (i > 0) {
                const maze::Cell& prev = result.cells[i - 1];
                CHECK_EQ(std::abs(prev.row - cell.row) + std::abs(prev.col - cell.col), 1);
            }
        }
        CHECK_EQ(grassOnPath, 2);

        // 权重一致性：成本 = 步数 + 2 * 草地数（普通地块成本 1，草地成本 3）
        CHECK_EQ(result.cost, 38 + 2 * 2);
    }

    maze::Maze maze11;
    CHECK(maze11.load(findShippedMaze("maze11_demo.txt"), &error));
    if (error.empty()) {
        CHECK_EQ(maze11.rows(), 11);
        CHECK_EQ(maze11.cols(), 11);
        const maze::PathResult result =
            maze::PathFinder::shortestPath(maze11, maze11.start(), maze11.end());
        CHECK(result.found);
        CHECK_EQ(result.cost, 24);
        CHECK_EQ(static_cast<int>(result.cells.size()) - 1, 20);
    }
}

void testWallTarget() {
    const std::string path = writeTempMaze("wall_target.txt",
                                           "3 3\n"
                                           "1 1 1\n"
                                           "1 -1 -2\n"
                                           "1 1 1\n");
    maze::Maze maze;
    CHECK(maze.load(path));
    const maze::PathResult result = maze::PathFinder::shortestPath(maze, maze.start(), maze::Cell{0, 0});
    CHECK(!result.found);
    CHECK_EQ(result.cost, 0);
    CHECK(result.cells.empty());
}

// ---------------- 高级算法 (a)：怪物沿最短路径追击 ----------------
void testMonsterChases() {
    // 一条 5 格的走廊：小人 (-1) 在左端，怪物从右端出发
    const std::string path = writeTempMaze("monster_corridor.txt",
                                           "3 7\n"
                                           "1 1 1 1 1 1 1\n"
                                           "-1 0 0 0 0 0 -2\n"
                                           "1 1 1 1 1 1 1\n");
    maze::Maze maze;
    CHECK(maze.load(path));
    const maze::Cell playerCell{1, 0};

    maze::Monster monster;
    monster.reset(maze::Cell{1, 5});
    CHECK(monster.ready());

    int previousCost = maze::PathFinder::shortestPath(maze, monster.cell(), playerCell).cost;
    CHECK(previousCost > 0);

    // 每走一步，怪物到小人的最短路径代价都必须严格下降，且每次只移动一格
    for (int step = 0; step < 5; ++step) {
        monster.update(1.0f);  // 推进动画并清掉冷却，模拟“隔一段时间走一步”
        const maze::Cell before = monster.cell();
        CHECK(monster.stepToward(maze, playerCell));
        const maze::Cell after = monster.cell();
        CHECK_EQ(std::abs(after.row - before.row) + std::abs(after.col - before.col), 1);

        const maze::PathResult remaining =
            maze::PathFinder::shortestPath(maze, after, playerCell);
        CHECK(remaining.found);
        CHECK(remaining.cost < previousCost);
        previousCost = remaining.cost;
    }
    CHECK(monster.cell() == playerCell);  // 5 步后抓住小人
}

void testMonsterAvoidsFriendly() {
    const std::string path = writeTempMaze("monster_block.txt",
                                           "3 7\n"
                                           "1 1 1 1 1 1 1\n"
                                           "-1 0 0 0 0 0 -2\n"
                                           "1 1 1 1 1 1 1\n");
    maze::Maze maze;
    CHECK(maze.load(path));

    maze::Monster front;
    front.reset(maze::Cell{1, 3});
    maze::Monster back;
    back.reset(maze::Cell{1, 4});

    // 后面的怪物不能踩到前面的怪物所在格；走廊里没有别的路，原地等待也是正确行为
    const maze::Cell before = back.cell();
    back.stepToward(maze, maze::Cell{1, 0}, {front.cell()});
    CHECK(!(back.cell() == front.cell()));
    CHECK(back.cell() == before);
}

// ---------------- 高级算法 (b)：随机迷宫生成 ----------------
void testRandomMazeGenerator() {
    maze::GenerateOptions options;
    options.rows = 21;
    options.cols = 21;
    options.seed = 12345;

    unsigned usedSeed = 0;
    const maze::Maze mazeA = maze::MazeGenerator::generate(options, &usedSeed);
    CHECK_EQ(usedSeed, 12345u);
    CHECK_EQ(mazeA.rows(), 21);
    CHECK_EQ(mazeA.cols(), 21);
    CHECK(mazeA.count(maze::Tile::Start) == 1);
    CHECK(mazeA.count(maze::Tile::End) == 1);
    CHECK(maze::MazeGenerator::isPerfect(mazeA));

    const maze::PathResult path = maze::PathFinder::shortestPath(mazeA, mazeA.start(), mazeA.end());
    CHECK(path.found);
    CHECK(path.cost > 0);

    // 相同种子必须得到完全一样的迷宫（可复现）
    const maze::Maze mazeB = maze::MazeGenerator::generate(options);
    bool identical = mazeA.rows() == mazeB.rows() && mazeA.cols() == mazeB.cols();
    if (identical) {
        for (int row = 0; row < mazeA.rows() && identical; ++row) {
            for (int col = 0; col < mazeA.cols() && identical; ++col) {
                identical = mazeA.tile(row, col) == mazeB.tile(row, col);
            }
        }
    }
    CHECK(identical);

    // 至少要有草地和熔岩，才用得上带权最短路
    CHECK(mazeA.count(maze::Tile::Grass) + mazeA.count(maze::Tile::Lava) > 0);
}

void testGeneratedMazeSizes() {
    // 传入偶数尺寸时应自动向上取整为奇数
    const maze::Maze maze = maze::MazeGenerator::generate(20, 20, 777);
    CHECK_EQ(maze.rows(), 21);
    CHECK_EQ(maze.cols(), 21);
    CHECK(maze::MazeGenerator::isPerfect(maze));

    const maze::Maze small = maze::MazeGenerator::generate(5, 5, 1);
    CHECK_EQ(small.rows(), 5);
    CHECK_EQ(small.cols(), 5);
}

}  // namespace

int main() {
    g_tempDir = std::filesystem::temp_directory_path() / "maze_game_core_tests";
    std::filesystem::create_directories(g_tempDir);

    testParseAndGrassCost();
    testLavaAvoided();
    testPlayerRules();
    testInvalidFiles();
    testShippedMazes();
    testWallTarget();
    testMonsterChases();
    testMonsterAvoidsFriendly();
    testRandomMazeGenerator();
    testGeneratedMazeSizes();

    std::printf("core_tests: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
