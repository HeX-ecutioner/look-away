@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build
set BUILD_TYPE=Release
set DEBUG_TIMER=OFF

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="debug" (
    set BUILD_TYPE=Debug
    set DEBUG_TIMER=ON
)
if /i "%~1"=="clean" (
    echo Cleaning build directory...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    goto end
)
shift
goto parse_args
:end_parse

echo Building Look Away! (%BUILD_TYPE%)

taskkill /f /im LookAway.exe >nul 2>&1

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DDEBUG_TIMER=%DEBUG_TIMER%
cmake --build . --config %BUILD_TYPE%

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build failed. Please fix the errors above.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Build complete!
echo.

if exist Release\LookAway.exe (
    echo Running LookAway...
    start "" Release\LookAway.exe
) else if exist LookAway.exe (
    echo Running LookAway...
    start "" LookAway.exe
)

:end
echo.
pause