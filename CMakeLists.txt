cmake_minimum_required(VERSION 3.25)
project(maze_game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------- raylib 6.0（随工程附带的源码，直接用本机 MinGW 编译） ----------------
set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib-6.0")
if(NOT EXISTS "${RAYLIB_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "未找到 raylib 6.0 源码：${RAYLIB_DIR}")
endif()
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(PLATFORM "Desktop" CACHE STRING "" FORCE)
add_subdirectory("${RAYLIB_DIR}" "${CMAKE_BINARY_DIR}/raylib" EXCLUDE_FROM_ALL)

# 资源与迷宫复制到 build 目录，方便直接运行 build/maze_game.exe
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/mazes" DESTINATION "${CMAKE_BINARY_DIR}")

# 编译警告选项：GCC/Clang 与 MSVC 的写法不同
if(MSVC)
    set(MAZE_WARN_FLAGS /W4)
    # 源码统一用 UTF-8 保存，MSVC 需要显式指定，否则中文注释会按 GBK 解析
    add_compile_options(/utf-8)
else()
    set(MAZE_WARN_FLAGS -Wall -Wextra)
endif()

# ---------------- 核心逻辑（不含绘制，可独立单元测试） ----------------
add_library(maze_core STATIC
    src/core/Maze.cpp
    src/core/PathFinder.cpp
    src/core/Player.cpp
    src/core/Monster.cpp
    src/core/MazeGenerator.cpp
)
target_include_directories(maze_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_options(maze_core PRIVATE ${MAZE_WARN_FLAGS})

# ---------------- 游戏程序 ----------------
add_executable(maze_game
    src/main.cpp
    src/game/GameApp.cpp
    src/render/Renderer.cpp
    src/render/TextRenderer.cpp
    src/render/AssetPaths.cpp
)
target_include_directories(maze_game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${RAYLIB_DIR}/src")
target_link_libraries(maze_game PRIVATE maze_core raylib opengl32 gdi32 winmm)
target_compile_options(maze_game PRIVATE ${MAZE_WARN_FLAGS})

# ---------------- 单元测试 ----------------
enable_testing()
add_executable(core_tests src/tests/core_tests.cpp)
target_link_libraries(core_tests PRIVATE maze_core)
target_compile_options(core_tests PRIVATE ${MAZE_WARN_FLAGS})
target_compile_definitions(core_tests PRIVATE MAZE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME core_tests COMMAND core_tests)
