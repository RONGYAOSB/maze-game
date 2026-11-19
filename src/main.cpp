#include <string>

#include "game/GameApp.h"
#include "raylib.h"
#include "render/AssetPaths.h"

namespace {

void printUsage() {
    TraceLog(LOG_INFO, "迷宫游戏（基础任务 + 进阶任务 1 + 高级算法） - Raylib 6.0");
    TraceLog(LOG_INFO, "用法：maze_game [选项] [迷宫文件]");
    TraceLog(LOG_INFO, "  --maze=<文件>   指定迷宫源文件，默认 mazes/maze20.txt");
    TraceLog(LOG_INFO, "  --random[=<种子>]  启动时生成随机迷宫（可指定种子复现）");
    TraceLog(LOG_INFO, "  --no-monsters   关闭怪物追击");
    TraceLog(LOG_INFO, "  --path-on       启动时打开路径提示");
    TraceLog(LOG_INFO, "  --skip-title    跳过开始界面直接进入游戏（调试/截图用）");
    TraceLog(LOG_INFO, "  --shot=<png>    渲染若干帧后截图并退出（自动化验证用）");
    TraceLog(LOG_INFO, "  --frames=<n>    截图前等待的帧数，默认 45");
    TraceLog(LOG_INFO, "  --walk=<UDLR>   启动时按方向序列自动移动若干步（调试用）");
    TraceLog(LOG_INFO, "  --help          显示本帮助");
}

}  // namespace

int main(int argc, char** argv) {
    game::Options options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
        if (arg == "--path-on" || arg == "-p") {
            options.showPath = true;
        } else if (arg == "--skip-title") {
            options.skipTitle = true;
        } else if (arg.rfind("--maze=", 0) == 0) {
            options.mazePath = arg.substr(7);
        } else if (arg == "--maze" && i + 1 < argc) {
            options.mazePath = argv[++i];
        } else if (arg.rfind("--shot=", 0) == 0) {
            options.shotPath = arg.substr(7);
            options.shotAfterFrames = 45;
        } else if (arg == "--shot" && i + 1 < argc) {
            options.shotPath = argv[++i];
            options.shotAfterFrames = 45;
        } else if (arg.rfind("--frames=", 0) == 0) {
            options.shotAfterFrames = std::stoi(arg.substr(9));
        } else if (arg.rfind("--seed=", 0) == 0) {
            options.randomMaze = true;
            options.seed = static_cast<unsigned>(std::stoul(arg.substr(7)));
        } else if (arg == "--random") {
            options.randomMaze = true;
        } else if (arg.rfind("--random=", 0) == 0) {
            options.randomMaze = true;
            options.seed = static_cast<unsigned>(std::stoul(arg.substr(9)));
        } else if (arg == "--no-monsters") {
            options.monsters = false;
        } else if (arg.rfind("--walk=", 0) == 0) {
            options.walk = arg.substr(7);
        } else if (!arg.empty() && arg[0] != '-') {
            options.mazePath = arg;
        } else {
            TraceLog(LOG_WARNING, "忽略未知参数：%s", arg.c_str());
        }
    }

    SetTraceLogLevel(LOG_INFO);
    InitWindow(game::kWindowWidth, game::kWindowHeight,
               "迷宫游戏 - 基础任务 + 进阶任务 1 + 高级算法 (Raylib 6.0)");
    if (!IsWindowReady()) {
        TraceLog(LOG_ERROR, "窗口初始化失败");
        return 1;
    }
    SetTargetFPS(60);

    const std::string iconPath = game::resolveAssetPath("assets/ui/icon.png");
    if (FileExists(iconPath.c_str())) {
        Image icon = LoadImage(iconPath.c_str());
        if (icon.data != nullptr) {
            SetWindowIcon(icon);
        }
        UnloadImage(icon);
    }

    game::GameApp app;
    std::string error;
    if (!app.init(options, &error)) {
        TraceLog(LOG_ERROR, "%s", error.c_str());
        app.shutdown();
        CloseWindow();
        return 1;
    }

    const int result = app.run();
    app.shutdown();
    CloseWindow();
    return result;
}
