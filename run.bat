@echo off
setlocal
cd /d "%~dp0"

rem ===========================================================================
rem  One-click configure + build + run.
rem  Uses CMake/Ninja from PATH first, then the ones shipped with Visual Studio.
rem  If configure fails in this folder (e.g. OneDrive sync folders), the project
rem  is copied to %PUBLIC%\MazeGameBuild and built there instead.
rem ===========================================================================

set "CMAKE="
set "NINJA="
set "VCVARS="
set "VSDIR="

where cmake >nul 2>nul
if not errorlevel 1 for /f "delims=" %%i in ('where cmake') do set "CMAKE=%%i"

where ninja >nul 2>nul
if not errorlevel 1 for /f "delims=" %%i in ('where ninja') do set "NINJA=%%~dpi"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
)

if defined VSDIR (
    if not defined CMAKE if exist "%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "CMAKE=%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if not defined NINJA if exist "%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" set "NINJA=%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\"
    if exist "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=%VSDIR%\VC\Auxiliary\Build\vcvars64.bat"
)

if not defined CMAKE (
    echo [ERROR] CMake not found. Install it with: winget install Kitware.CMake
    echo         or install Visual Studio 2022 with the "Desktop development with C++" workload.
    pause
    exit /b 1
)
if not defined NINJA (
    echo [ERROR] Ninja not found. Install it with: winget install Ninja-build.Ninja
    echo         or install Visual Studio 2022 with the "Desktop development with C++" workload.
    pause
    exit /b 1
)

rem Prepare the MSVC environment (cl.exe / link.exe are not on PATH by default)
if defined VCVARS (
    where cl >nul 2>nul
    if errorlevel 1 call "%VCVARS%" >nul
)

echo [1/3] Configuring (CMake + Ninja) ...
"%CMAKE%" -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
set "BUILD_RC=%errorlevel%"
if not "%BUILD_RC%"=="0" goto :fallback

echo [2/3] Building ...
"%CMAKE%" --build build
set "BUILD_RC=%errorlevel%"
if not "%BUILD_RC%"=="0" goto :fail

echo [3/3] Starting game ...
start "" "build\maze_game.exe"
exit /b 0

:fallback
echo.
echo [WARN] Configure failed in the current folder. Trying %PUBLIC%\MazeGameBuild ...
set "FALLBACK=%PUBLIC%\MazeGameBuild"
if not exist "%FALLBACK%\src" mkdir "%FALLBACK%\src" >nul 2>nul
robocopy "%~dp0." "%FALLBACK%\src" /E /XD .git build shots /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 (
    echo [ERROR] Copying the project to %FALLBACK%\src failed.
    pause
    exit /b 1
)
cd /d "%FALLBACK%\src"
echo [2/3] Configuring and building in the fallback folder ...
"%CMAKE%" -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
set "BUILD_RC=%errorlevel%"
if not "%BUILD_RC%"=="0" goto :fail
"%CMAKE%" --build build
set "BUILD_RC=%errorlevel%"
if not "%BUILD_RC%"=="0" goto :fail
echo [3/3] Starting game from %FALLBACK%\src\build ...
start "" "%FALLBACK%\src\build\maze_game.exe"
exit /b 0

:fail
echo.
echo [ERROR] Build failed. See the messages above.
pause
exit /b 1
