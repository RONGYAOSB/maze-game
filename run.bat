@echo off
chcp 65001 >nul
setlocal
cd /d "%~dp0"

where ninja >nul 2>nul
if errorlevel 1 (
    echo [ERROR] ninja not found. Install it with: winget install Ninja-build.Ninja
    echo         or: pip install ninja
    pause
    exit /b 1
)

echo [1/2] Configuring...
cmake -S . -B build -G Ninja
if errorlevel 1 goto :fail

echo [2/2] Building...
cmake --build build
if errorlevel 1 goto :fail

echo Starting game...
build\maze_game.exe
exit /b 0

:fail
echo Build failed. Check the messages above.
pause
exit /b 1
