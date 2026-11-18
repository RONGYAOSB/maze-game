#include "render/Renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "render/AssetPaths.h"

namespace game {
namespace {

const Color kBgTop = {18, 22, 34, 255};
const Color kBgBottom = {9, 11, 17, 255};
const Color kGold = {246, 208, 120, 255};
const Color kText = {228, 234, 244, 255};
const Color kDim = {148, 160, 182, 255};
const Color kPanelFill = {20, 24, 35, 242};
const Color kPanelLine = {246, 208, 120, 70};
const Color kGreen = {126, 220, 142, 255};
const Color kRed = {240, 120, 110, 255};
const Color kPath = {255, 214, 120, 255};

constexpr int kTopBar = 124;
constexpr int kBottomMargin = 26;
constexpr int kMargin = 22;
constexpr int kPanelWidth = 344;

Texture2D loadTextureChecked(const char* relativePath, bool* ok) {
    const std::string path = resolveAssetPath(relativePath);
    Texture2D texture{};
    if (FileExists(path.c_str())) {
        texture = LoadTexture(path.c_str());
    }
    if (texture.id == 0) {
        *ok = false;
    }
    return texture;
}

}  // namespace

bool Renderer::loadAssets(std::string* error) {
    unload();
    bool ok = true;
    ground_ = loadTextureChecked("assets/tiles/ground.png", &ok);
    wall_ = loadTextureChecked("assets/tiles/wall.png", &ok);
    grass_ = loadTextureChecked("assets/tiles/grass.png", &ok);
    lava_ = loadTextureChecked("assets/tiles/lava.png", &ok);
    start_ = loadTextureChecked("assets/tiles/start.png", &ok);
    end_ = loadTextureChecked("assets/tiles/end.png", &ok);
    player_ = loadTextureChecked("assets/tiles/player.png", &ok);
    banner_ = loadTextureChecked("assets/ui/title.png", &ok);

    for (Texture2D* t : {&ground_, &wall_, &grass_, &lava_, &start_, &end_, &player_, &banner_}) {
        if (t->id != 0) {
            SetTextureFilter(*t, TEXTURE_FILTER_BILINEAR);
        }
    }

    std::string fontError;
    const bool fontOk = text_.load(resolveAssetPath("assets/fonts/simhei.ttf"), &fontError);

    loaded_ = ok;
    if (!ok || !fontOk) {
        if (error != nullptr) {
            *error = "资源载入失败：assets/tiles/*.png 或 assets/ui/title.png 缺失";
            if (!fontOk) {
                *error += "；" + fontError;
            }
        }
        return false;
    }
    if (error != nullptr) {
        error->clear();
    }
    return true;
}

void Renderer::unload() {
    for (Texture2D* t : {&ground_, &wall_, &grass_, &lava_, &start_, &end_, &player_, &banner_}) {
        if (t->id != 0) {
            UnloadTexture(*t);
        }
        *t = Texture2D{};
    }
    text_.unload();
    loaded_ = false;
}

void Renderer::computeLayout(int windowWidth, int windowHeight, int rows, int cols) {
    const int areaX = kMargin;
    const int areaY = kTopBar;
    const int areaW = windowWidth - kPanelWidth - kMargin * 2;
    const int areaH = windowHeight - kTopBar - kBottomMargin;
    int tile = std::min(areaW / std::max(cols, 1), areaH / std::max(rows, 1));
    tile = std::max(tile, 8);

    layout_.tile = tile;
    layout_.originX = areaX + (areaW - tile * cols) / 2;
    layout_.originY = areaY + (areaH - tile * rows) / 2;
    layout_.mazeRect = {static_cast<float>(layout_.originX), static_cast<float>(layout_.originY),
                        static_cast<float>(tile * cols), static_cast<float>(tile * rows)};
    layout_.panelRect = {static_cast<float>(windowWidth - kPanelWidth - kMargin),
                         static_cast<float>(kTopBar), static_cast<float>(kPanelWidth),
                         static_cast<float>(windowHeight - kTopBar - kBottomMargin)};
}

Texture2D* Renderer::textureFor(maze::Tile tile) {
    switch (tile) {
        case maze::Tile::Ground: return &ground_;
        case maze::Tile::Wall: return &wall_;
        case maze::Tile::Grass: return &grass_;
        case maze::Tile::Lava: return &lava_;
        case maze::Tile::Start: return &start_;
        case maze::Tile::End: return &end_;
    }
    return &ground_;
}

void Renderer::drawTile(maze::Tile tile, float x, float y, float size) {
    Texture2D* texture = textureFor(tile);
    if (texture->id == 0) {
        return;
    }
    const Rectangle source = {0.0f, 0.0f, static_cast<float>(texture->width),
                              static_cast<float>(texture->height)};
    const Rectangle dest = {x, y, size, size};
    DrawTexturePro(*texture, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}

void Renderer::drawMaze(const maze::Maze& maze) {
    const int tile = layout_.tile;
    for (int row = 0; row < maze.rows(); ++row) {
        for (int col = 0; col < maze.cols(); ++col) {
            const float x = static_cast<float>(layout_.originX + col * tile);
            const float y = static_cast<float>(layout_.originY + row * tile);
            drawTile(maze.tile(row, col), x, y, static_cast<float>(tile));
        }
    }
}

void Renderer::drawPath(const maze::PathResult& path, float timeSeconds) {
    if (!path.found || path.cells.size() < 2) {
        return;
    }
    const int tile = layout_.tile;
    const auto center = [this, tile](const maze::Cell& cell) {
        return Vector2{static_cast<float>(layout_.originX + cell.col * tile) + tile * 0.5f,
                       static_cast<float>(layout_.originY + cell.row * tile) + tile * 0.5f};
    };

    const float phase = 0.5f + 0.5f * std::sin(timeSeconds * 3.5f);
    const float lineWidth = std::max(3.0f, tile * 0.14f);
    const float dotRadius = std::max(2.0f, tile * 0.11f);

    for (size_t i = 1; i < path.cells.size(); ++i) {
        DrawLineEx(center(path.cells[i - 1]), center(path.cells[i]), lineWidth,
                   Color{255, 214, 120, 120});
    }
    for (size_t i = 1; i < path.cells.size(); ++i) {
        DrawCircleV(center(path.cells[i]), dotRadius,
                    Color{255, 232, 156, static_cast<unsigned char>(130 + 80.0f * phase)});
    }
    const Vector2 goal = center(path.cells.back());
    DrawCircleLinesV(goal, tile * 0.36f, Color{255, 232, 156, 210});
    DrawCircleLinesV(goal, tile * 0.44f, Color{255, 232, 156, 90});
}

void Renderer::drawPlayer(const maze::Player& player) {
    if (player_.id == 0) {
        return;
    }
    const int tile = layout_.tile;
    const float t = player.progress();
    const float eased = t * t * (3.0f - 2.0f * t);
    const maze::Cell from = player.previous();
    const maze::Cell to = player.cell();
    const float fromX = static_cast<float>(layout_.originX + from.col * tile) + tile * 0.5f;
    const float fromY = static_cast<float>(layout_.originY + from.row * tile) + tile * 0.5f;
    const float toX = static_cast<float>(layout_.originX + to.col * tile) + tile * 0.5f;
    const float toY = static_cast<float>(layout_.originY + to.row * tile) + tile * 0.5f;
    const float bob = (t < 1.0f) ? std::sin(t * PI) * tile * 0.10f : 0.0f;

    const float size = tile * 0.94f;
    const Rectangle source = {0.0f, 0.0f, static_cast<float>(player_.width),
                              static_cast<float>(player_.height)};
    const Rectangle dest = {fromX + (toX - fromX) * eased - size * 0.5f,
                            fromY + (toY - fromY) * eased - size * 0.5f - bob, size, size};
    DrawTexturePro(player_, source, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}

void Renderer::drawWinOverlay() {
    const Rectangle mazeRect = layout_.mazeRect;
    DrawRectangleRec(mazeRect, Color{6, 8, 14, 170});
    const Rectangle box = {mazeRect.x + mazeRect.width * 0.5f - 230.0f,
                           mazeRect.y + mazeRect.height * 0.5f - 82.0f, 460.0f, 164.0f};
    DrawRectangleRounded(box, 0.10f, 10, Color{22, 26, 38, 248});
    DrawRectangleRoundedLinesEx(box, 0.10f, 10, 2.0f, kGold);
    text_.drawCentered("到达终点！", mazeRect.x + mazeRect.width * 0.5f, box.y + 22.0f, 46.0f,
                       kGold);
    text_.drawCentered("按 R 重新开始     按 空格 返回开始界面",
                       mazeRect.x + mazeRect.width * 0.5f, box.y + 96.0f, 20.0f, kText);
}

// 开始界面：标题图片 + 呼吸闪烁的“按 空格 键 开始游戏”提示
void Renderer::drawTitleScreen(float timeSeconds) {
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    DrawRectangleGradientV(0, 0, width, height, kBgTop, kBgBottom);
    DrawRectangleRoundedLinesEx(
        Rectangle{40.0f, 40.0f, static_cast<float>(width) - 80.0f,
                  static_cast<float>(height) - 80.0f},
        0.01f, 4, 1.0f, Color{246, 208, 120, 45});

    if (banner_.id != 0) {
        const float bannerWidth = std::min(900.0f, static_cast<float>(width) * 0.72f);
        const float scale = bannerWidth / static_cast<float>(banner_.width);
        const float bannerHeight = static_cast<float>(banner_.height) * scale;
        DrawTexturePro(banner_, Rectangle{0.0f, 0.0f, static_cast<float>(banner_.width),
                                          static_cast<float>(banner_.height)},
                       Rectangle{(width - bannerWidth) * 0.5f, height * 0.24f - bannerHeight * 0.5f,
                                 bannerWidth, bannerHeight},
                       Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    }

    const float centerX = width * 0.5f;
    const float phase = 0.55f + 0.45f * (0.5f + 0.5f * std::sin(timeSeconds * 2.8f));
    text_.drawCentered("按 空格 键 开始游戏", centerX, height * 0.47f, 40.0f,
                       Color{248, 214, 132, static_cast<unsigned char>(255.0f * phase)});
    text_.drawCentered("从起点出发，到达终点即胜利；按 P 显示最短路径", centerX,
                       height * 0.47f + 52.0f, 20.0f, kDim);

    DrawLineEx(Vector2{centerX - 260.0f, height * 0.47f + 84.0f},
               Vector2{centerX + 260.0f, height * 0.47f + 84.0f}, 1.0f,
               Color{246, 208, 120, 60});

    text_.drawCentered("方向键 移动    P 路径提示    R 重新开始    1 / 2 切换迷宫    ESC 退出",
                       centerX, height * 0.47f + 106.0f, 21.0f, kDim);

    // 地块图鉴：填充中部空白，同时说明各种地块
    struct LegendItem {
        maze::Tile tile;
        const char* label;
    };
    static const LegendItem kItems[] = {
        {maze::Tile::Ground, "普通地面"}, {maze::Tile::Grass, "草地"},
        {maze::Tile::Lava, "熔岩"},       {maze::Tile::Wall, "墙"},
        {maze::Tile::Start, "起点"},      {maze::Tile::End, "终点"},
    };
    constexpr int kItemCount = static_cast<int>(sizeof(kItems) / sizeof(kItems[0]));
    constexpr float kItemWidth = 96.0f;
    const float legendY = height * 0.66f;
    const float legendStartX = centerX - kItemWidth * kItemCount * 0.5f;
    for (int i = 0; i < kItemCount; ++i) {
        const float itemX = legendStartX + kItemWidth * i + kItemWidth * 0.5f;
        drawTile(kItems[i].tile, itemX - 16.0f, legendY, 32.0f);
        text_.drawCentered(kItems[i].label, itemX, legendY + 40.0f, 17.0f, kDim);
    }

    text_.drawCentered("20 x 20 自设计迷宫    Raylib 6.0", centerX, height - 104.0f, 19.0f, kDim);
    text_.drawCentered("基础任务 + 进阶任务 1", centerX, height - 74.0f, 19.0f,
                       Color{246, 208, 120, 170});
}

void Renderer::drawLegendRow(float x, float y, maze::Tile tile, const char* label,
                             const char* value) {
    drawTile(tile, x, y + 2.0f, 20.0f);
    text_.draw(label, x + 30.0f, y, 18.0f, kText);
    if (value != nullptr && value[0] != '\0') {
        text_.drawRight(value, layout_.panelRect.x + layout_.panelRect.width - 20.0f, y, 18.0f,
                        kDim);
    }
}

void Renderer::drawPanel(const maze::Maze& maze, const maze::Player& player,
                         const maze::PathResult& path, bool showPath) {
    const Rectangle panel = layout_.panelRect;
    DrawRectangleRounded(panel, 0.03f, 8, kPanelFill);
    DrawRectangleRoundedLinesEx(panel, 0.03f, 8, 1.5f, kPanelLine);

    const float x = panel.x + 20.0f;
    const float right = panel.x + panel.width - 20.0f;
    float y = panel.y + 16.0f;

    text_.draw("状态", x, y, 24.0f, kGold);
    y += 34.0f;
    DrawLineEx(Vector2{x, y}, Vector2{right, y}, 1.0f, Color{246, 208, 120, 50});
    y += 12.0f;

    const auto statRow = [&](const char* label, const char* value, Color color) {
        text_.draw(label, x, y, 19.0f, kDim);
        text_.draw(value, x + 118.0f, y, 19.0f, color);
        y += 27.0f;
    };

    statRow("路径提示", showPath ? "开" : "关", showPath ? kGreen : kDim);
    statRow("步数", TextFormat("%d", player.steps()), kText);
    statRow("位置", TextFormat("%d, %d", player.cell().row + 1, player.cell().col + 1), kText);
    statRow("迷宫", maze.title().c_str(), kText);
    statRow("尺寸", TextFormat("%d x %d", maze.rows(), maze.cols()), kText);
    if (showPath) {
        if (player.cell() == maze.end()) {
            statRow("最短路径", "已到达终点", kGreen);
        } else if (path.found) {
            statRow("最短路径", TextFormat("%d 步 / 代价 %d", static_cast<int>(path.cells.size()) - 1,
                                            path.cost),
                    kText);
        } else {
            statRow("最短路径", "无可行路径", kRed);
        }
    } else {
        statRow("最短路径", "按 P 显示", kDim);
    }

    y += 6.0f;
    DrawLineEx(Vector2{x, y}, Vector2{right, y}, 1.0f, Color{246, 208, 120, 50});
    y += 14.0f;

    text_.draw("控制", x, y, 24.0f, kGold);
    y += 34.0f;
    const auto keyRow = [&](const char* key, const char* action) {
        text_.draw(key, x, y, 19.0f, kGold);
        text_.draw(action, x + 84.0f, y, 19.0f, kText);
        y += 27.0f;
    };
    keyRow("方向键", "移动");
    keyRow("P", "显示/隐藏路径");
    keyRow("R", "重新开始");
    keyRow("1 / 2", "切换迷宫");
    keyRow("ESC", "退出");

    y += 6.0f;
    DrawLineEx(Vector2{x, y}, Vector2{right, y}, 1.0f, Color{246, 208, 120, 50});
    y += 14.0f;

    text_.draw("图例", x, y, 24.0f, kGold);
    y += 36.0f;
    drawLegendRow(x, y, maze::Tile::Ground, "普通地面", "1");
    y += 28.0f;
    drawLegendRow(x, y, maze::Tile::Grass, "草地", "3");
    y += 28.0f;
    drawLegendRow(x, y, maze::Tile::Lava, "熔岩", "1000");
    y += 28.0f;
    drawLegendRow(x, y, maze::Tile::Wall, "墙", "不可通行");
    y += 28.0f;
    drawLegendRow(x, y, maze::Tile::Start, "起点", "");
    y += 28.0f;
    drawLegendRow(x, y, maze::Tile::End, "终点", "");

    text_.drawRight(TextFormat("FPS %d", GetFPS()), right, panel.y + panel.height - 26.0f, 16.0f,
                    kDim);
}

void Renderer::draw(const maze::Maze& maze, const maze::Player& player,
                    const maze::PathResult& path, bool showPath, float timeSeconds, bool win) {
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    computeLayout(width, height, maze.rows(), maze.cols());

    DrawRectangleGradientV(0, 0, width, height, kBgTop, kBgBottom);

    // 标题图片：作为“读取并显示图片文件”的展示，同时充当界面横幅
    if (banner_.id != 0) {
        const float bannerWidth =
            std::min(700.0f, std::max(460.0f, layout_.mazeRect.width + 40.0f));
        const float scale = bannerWidth / static_cast<float>(banner_.width);
        const float bannerHeight = static_cast<float>(banner_.height) * scale;
        const float bx = layout_.mazeRect.x + (layout_.mazeRect.width - bannerWidth) * 0.5f;
        DrawTexturePro(banner_, Rectangle{0.0f, 0.0f, static_cast<float>(banner_.width),
                                          static_cast<float>(banner_.height)},
                       Rectangle{bx, 8.0f, bannerWidth, bannerHeight}, Vector2{0.0f, 0.0f}, 0.0f,
                       WHITE);
    }

    const Rectangle frame = {layout_.mazeRect.x - 6.0f, layout_.mazeRect.y - 6.0f,
                             layout_.mazeRect.width + 12.0f, layout_.mazeRect.height + 12.0f};
    DrawRectangleRounded(frame, 0.02f, 4, Color{0, 0, 0, 100});
    DrawRectangleRoundedLinesEx(frame, 0.02f, 4, 2.0f, Color{246, 208, 120, 85});

    drawMaze(maze);
    if (showPath) {
        drawPath(path, timeSeconds);
    }
    drawPlayer(player);
    if (win) {
        drawWinOverlay();
    }
    drawPanel(maze, player, path, showPath);
}

}  // namespace game
