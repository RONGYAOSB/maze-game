#pragma once

#include <string>
#include <vector>

#include "raylib.h"

namespace game {

// 中文文字绘制：从 assets/fonts/simhei.ttf 载入所需字形（含 ASCII 与界面用到的汉字）。
class TextRenderer {
public:
    bool load(const std::string& fontPath, std::string* error = nullptr);
    void unload();

    void draw(const char* text, float x, float y, float fontSize, Color color) const;
    void drawCentered(const char* text, float centerX, float y, float fontSize, Color color) const;
    // 右对齐：value 的右端对齐到 rightX
    void drawRight(const char* text, float rightX, float y, float fontSize, Color color) const;
    float measure(const char* text, float fontSize) const;

    bool loaded() const { return loaded_; }
    const Font& font() const { return font_; }

private:
    static std::vector<int> buildCodepoints();

    Font font_{};
    bool loaded_ = false;
};

}  // namespace game
