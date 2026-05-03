@echo off

echo =========================
echo Cleaning old build...
echo =========================

if exist build (
    rmdir /s /q build
)

echo.

echo =========================
echo Building LookAway...
echo =========================

if not exist build mkdir build
cd build

cmake ..
cmake --build .

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build failed. Fix errors above.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo =========================
echo Running LookAway...
echo =========================

.\LookAway.exe

pause