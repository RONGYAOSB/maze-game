#pragma once

#include <string>

namespace game {

// 依次在当前工作目录、可执行文件目录、可执行文件上级目录中查找资源，
// 这样从工程根目录运行和从 build 目录运行都能找到 assets/、mazes/。
std::string resolveAssetPath(const std::string& relativePath);

}  // namespace game
