#include "core/Maze.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace maze {
namespace {

void setError(std::string* error, const std::string& message) {
    if (error != nullptr) {
        *error = message;
    }
}

}  // namespace

bool Maze::load(const std::string& fileName, std::string* error) {
    // 使用 std::filesystem::path 构造流，Windows 下可正确处理中文路径
    std::ifstream in{std::filesystem::path(fileName)};
    if (!in) {
        setError(error, "无法打开迷宫文件：" + fileName);
        return false;
    }

    int rows = 0;
    int cols = 0;
    if (!(in >> rows >> cols)) {
        setError(error, "迷宫文件首行应包含行数和列数");
        return false;
    }
    if (rows < 3 || cols < 3 || rows > 400 || cols > 400) {
        setError(error, "迷宫尺寸不合法（应在 3~400 之间）：" + std::to_string(rows) + "x" +
                            std::to_string(cols));
        return false;
    }

    std::vector<Tile> tiles(static_cast<size_t>(rows) * cols);
    Cell startCell{};
    Cell endCell{};
    int startCount = 0;
    int endCount = 0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int value = 0;
            if (!(in >> value)) {
                setError(error, "迷宫数据不足，缺少第 " + std::to_string(r + 1) + " 行第 " +
                                    std::to_string(c + 1) + " 列的数据");
                return false;
            }
            if (value < -2 || value > 3) {
                setError(error, "第 " + std::to_string(r + 1) + " 行第 " + std::to_string(c + 1) +
                                    " 列的地块类型非法：" + std::to_string(value));
                return false;
            }
            const auto type = static_cast<Tile>(value);
            tiles[static_cast<size_t>(r) * cols + c] = type;
            if (type == Tile::Start) {
                startCell = {r, c};
                ++startCount;
            } else if (type == Tile::End) {
                endCell = {r, c};
                ++endCount;
            }
        }
    }

    int extra = 0;
    if (in >> extra) {
        setError(error, "迷宫数据多于首行声明的尺寸");
        return false;
    }

    if (startCount != 1) {
        setError(error, "迷宫必须且只能有一个起点（-1），当前为 " + std::to_string(startCount) + " 个");
        return false;
    }
    if (endCount != 1) {
        setError(error, "迷宫必须且只能有一个终点（-2），当前为 " + std::to_string(endCount) + " 个");
        return false;
    }

    rows_ = rows;
    cols_ = cols;
    tiles_ = std::move(tiles);
    start_ = startCell;
    end_ = endCell;
    source_ = fileName;
    title_ = std::filesystem::path(fileName).filename().string();
    return true;
}

Tile Maze::tile(int row, int col) const {
    if (!inBounds(row, col)) {
        return Tile::Wall;
    }
    return tiles_[static_cast<size_t>(index(row, col))];
}

int Maze::enterCost(int row, int col) const {
    switch (tile(row, col)) {
        case Tile::Ground:
        case Tile::Start:
        case Tile::End:
            return 1;
        case Tile::Grass:
            return 3;  // 行走成本至少为 3
        case Tile::Lava:
            return 1000;  // 熔岩代价极高
        case Tile::Wall:
        default:
            return 1000000;  // 墙不可进入
    }
}

int Maze::count(Tile type) const {
    int n = 0;
    for (const Tile t : tiles_) {
        if (t == type) {
            ++n;
        }
    }
    return n;
}

bool Maze::assign(int rows, int cols, std::vector<Tile> tiles, Cell start, Cell end,
                  const std::string& title) {
    if (rows < 3 || cols < 3 || rows > 400 || cols > 400) {
        return false;
    }
    if (tiles.size() != static_cast<size_t>(rows) * static_cast<size_t>(cols)) {
        return false;
    }
    if (start.row < 0 || start.row >= rows || start.col < 0 || start.col >= cols) {
        return false;
    }
    if (end.row < 0 || end.row >= rows || end.col < 0 || end.col >= cols) {
        return false;
    }
    if (tiles[static_cast<size_t>(start.row) * cols + start.col] == Tile::Wall) {
        return false;
    }
    if (tiles[static_cast<size_t>(end.row) * cols + end.col] == Tile::Wall) {
        return false;
    }

    rows_ = rows;
    cols_ = cols;
    tiles_ = std::move(tiles);
    start_ = start;
    end_ = end;
    source_.clear();
    title_ = title;
    return true;
}

bool Maze::save(const std::string& fileName, std::string* error) const {
    if (rows_ <= 0 || cols_ <= 0 || tiles_.empty()) {
        setError(error, "迷宫为空，无法保存");
        return false;
    }
    std::ofstream out{std::filesystem::path(fileName)};
    if (!out) {
        setError(error, "无法写入迷宫文件：" + fileName);
        return false;
    }
    out << rows_ << ' ' << cols_ << '\n';
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            out << static_cast<int>(tiles_[static_cast<size_t>(index(r, c))]);
            out << (c + 1 == cols_ ? '\n' : ' ');
        }
    }
    return true;
}

}  // namespace maze
