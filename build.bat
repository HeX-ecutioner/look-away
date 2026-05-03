@echo off
setlocal

rem Usage:
rem build.bat         -> release build (20min work / 20s break)
rem build.bat debug   -> debug build (10s work / 5s break)

set DEBUG_FLAG=OFF
set BUILD_LABEL=Release
if /I "%1"=="debug" (
    set DEBUG_FLAG=ON
    set BUILD_LABEL=Debug
)

echo =========================
echo Cleaning old build...
echo =========================

if exist build (
    rmdir /s /q build
)

echo.
echo ==============================
echo Building LookAway (%BUILD_LABEL%)...
echo ==============================

mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=%BUILD_LABEL% -DDEBUG_TIMER=%DEBUG_FLAG%
cmake --build .

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build failed. Fix errors above.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo =======================
echo %BUILD_LABEL% Build complete!
echo =======================
echo.
echo =======================
echo Running LookAway...
echo =======================

LookAway.exe

pause