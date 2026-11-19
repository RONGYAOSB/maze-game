#pragma once

#include <string>
#include <vector>

namespace maze {

// 地块类型，数值与迷宫源文件中的数字一一对应：
//   -2 终点   -1 起点   0 普通地面   1 墙   2 草地   3 熔岩
enum class Tile : int {
    End = -2,
    Start = -1,
    Ground = 0,
    Wall = 1,
    Grass = 2,
    Lava = 3,
};

struct Cell {
    int row = 0;
    int col = 0;
};

inline bool operator==(const Cell& a, const Cell& b) { return a.row == b.row && a.col == b.col; }
inline bool operator!=(const Cell& a, const Cell& b) { return !(a == b); }

// 迷宫数据：负责从 txt 源文件读取并校验数据，只做数据管理，不涉及绘制。
class Maze {
public:
    // 读取成功返回 true；失败时通过 error 返回中文错误说明。
    bool load(const std::string& fileName, std::string* error = nullptr);

    // 由内存数据构造迷宫（供随机迷宫生成器使用），不读写文件。
    bool assign(int rows, int cols, std::vector<Tile> tiles, Cell start, Cell end,
                const std::string& title);

    // 把当前迷宫写成与源文件同格式的 txt，便于把随机迷宫存档复现。
    bool save(const std::string& fileName, std::string* error = nullptr) const;

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    bool inBounds(int row, int col) const {
        return row >= 0 && row < rows_ && col >= 0 && col < cols_;
    }

    Tile tile(int row, int col) const;
    bool isWall(int row, int col) const { return tile(row, col) == Tile::Wall; }
    bool isWalkable(int row, int col) const { return inBounds(row, col) && !isWall(row, col); }

    // 进入某个地块的行走成本：普通地面/起点/终点 1，草地 3，熔岩 1000，墙不可进入。
    int enterCost(int row, int col) const;

    Cell start() const { return start_; }
    Cell end() const { return end_; }

    int count(Tile type) const;
    const std::string& source() const { return source_; }
    const std::string& title() const { return title_; }

private:
    int index(int row, int col) const { return row * cols_ + col; }

    int rows_ = 0;
    int cols_ = 0;
    std::vector<Tile> tiles_;
    Cell start_{};
    Cell end_{};
    std::string source_;
    std::string title_;
};

}  // namespace maze
