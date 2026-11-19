#pragma once

#include <string>

#include "core/Maze.h"

namespace maze {

// 随机迷宫参数：行、列都会被向上取整到奇数，便于做“细胞 + 墙”的网格。
struct GenerateOptions {
    int rows = 21;
    int cols = 21;
    unsigned seed = 0;          // 0 表示自动取随机种子
    float grassRatio = 0.035f;  // 草地占比
    float lavaRatio = 0.008f;   // 熔岩占比
};

// 随机迷宫生成器：递归回溯（随机深度优先）算法。
// 生成的是一棵“完美迷宫”——任意两个可走格之间恰有一条通路，因此一定有解。
class MazeGenerator {
public:
    static Maze generate(const GenerateOptions& options, unsigned* usedSeed = nullptr);

    static Maze generate(int rows, int cols, unsigned seed);

    // 校验：所有可走格连通，且可走格之间的邻接边数 = 格数 - 1（即无环的树）。
    static bool isPerfect(const Maze& maze);

    static std::string titleFor(int rows, int cols, unsigned seed);
};

}  // namespace maze
