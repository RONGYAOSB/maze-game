#include "render/TextRenderer.h"

#include <algorithm>
#include <string>
#include <vector>

namespace game {
namespace {

// 界面中会出现的全部汉字（集中列出，便于一次性载入字形）
const char* kUiText =
    "状态路径提示开关步数位置迷宫行列尺寸最短代价无可行控制方向键移动显示隐藏重新开始切换图例"
    "普通地面墙草地熔岩起点终点不可通行到达按当前自设计演示失败载入已最佳从到棋盘格子退出"
    "空格进阶返回界面基础游戏任务说明小人出发即胜利！，；";

constexpr int kBaseFontSize = 48;
constexpr float kSpacing = 0.5f;

}  // namespace

std::vector<int> TextRenderer::buildCodepoints() {
    std::vector<int> codepoints;
    for (int c = 32; c <= 126; ++c) {
        codepoints.push_back(c);
    }
    const char* cursor = kUiText;
    while (*cursor != '\0') {
        int size = 0;
        const int cp = GetCodepointNext(cursor, &size);
        if (cp > 126) {
            codepoints.push_back(cp);
        }
        cursor += (size > 0) ? size : 1;
    }
    std::sort(codepoints.begin(), codepoints.end());
    codepoints.erase(std::unique(codepoints.begin(), codepoints.end()), codepoints.end());
    return codepoints;
}

bool TextRenderer::load(const std::string& fontPath, std::string* error) {
    unload();
    const std::vector<int> codepoints = buildCodepoints();
    if (FileExists(fontPath.c_str())) {
        font_ = LoadFontEx(fontPath.c_str(), kBaseFontSize, codepoints.data(),
                           static_cast<int>(codepoints.size()));
    }
    if (font_.texture.id == 0) {
        font_ = GetFontDefault();
        loaded_ = false;
        if (error != nullptr) {
            *error = "字体载入失败（界面中文会显示乱码）：" + fontPath;
        }
        return false;
    }
    SetTextureFilter(font_.texture, TEXTURE_FILTER_BILINEAR);
    loaded_ = true;
    if (error != nullptr) {
        error->clear();
    }
    return true;
}

void TextRenderer::unload() {
    if (loaded_ && font_.texture.id != 0) {
        UnloadFont(font_);
    }
    font_ = Font{};
    loaded_ = false;
}

void TextRenderer::draw(const char* text, float x, float y, float fontSize, Color color) const {
    if (text == nullptr || *text == '\0') {
        return;
    }
    DrawTextEx(font_, text, Vector2{x, y}, fontSize, kSpacing, color);
}

float TextRenderer::measure(const char* text, float fontSize) const {
    if (text == nullptr || *text == '\0') {
        return 0.0f;
    }
    return MeasureTextEx(font_, text, fontSize, kSpacing).x;
}

void TextRenderer::drawCentered(const char* text, float centerX, float y, float fontSize,
                                Color color) const {
    draw(text, centerX - measure(text, fontSize) * 0.5f, y, fontSize, color);
}

void TextRenderer::drawRight(const char* text, float rightX, float y, float fontSize,
                             Color color) const {
    draw(text, rightX - measure(text, fontSize), y, fontSize, color);
}

}  // namespace game
