#include "render/AssetPaths.h"

#include <string>

#include "raylib.h"

namespace game {

std::string resolveAssetPath(const std::string& relativePath) {
    if (FileExists(relativePath.c_str())) {
        return relativePath;
    }
    const std::string appDir = GetApplicationDirectory();
    const std::string besideExe = appDir + relativePath;
    if (FileExists(besideExe.c_str())) {
        return besideExe;
    }
    const std::string parentDir = appDir + "../" + relativePath;
    if (FileExists(parentDir.c_str())) {
        return parentDir;
    }
    return relativePath;
}

}  // namespace game
