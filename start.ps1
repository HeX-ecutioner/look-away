param(
    [switch]$Debug,
    [switch]$Clean,
    [switch]$NoRun
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir    = Join-Path $ProjectRoot "build"

if ($Debug) {
    $BuildType  = "Debug"
    $DebugTimer = "ON"
}
else {
    $BuildType  = "Release"
    $DebugTimer = "OFF"
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "        Look Away! Build Script"          -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Build type : $BuildType"
Write-Host "Debug timer: $DebugTimer"
Write-Host "Build dir  : $BuildDir"
Write-Host ""

if ($Clean) {
    if (Test-Path $BuildDir) {
        Write-Host "Cleaning build directory..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force $BuildDir
        Write-Host "Build directory removed." -ForegroundColor Green
    }
    else {
        Write-Host "Build directory does not exist. Nothing to clean." -ForegroundColor DarkGray
    }
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "CMake was not found in PATH."
}

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Write-Host ""
Write-Host "Configuring CMake ($BuildType)..." -ForegroundColor Cyan

cmake `
    -S $ProjectRoot `
    -B $BuildDir `
    "-DCMAKE_BUILD_TYPE=$BuildType" `
    "-DDEBUG_TIMER=$DebugTimer"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

Write-Host ""
Write-Host "Building Look Away! ($BuildType)..." -ForegroundColor Cyan

cmake `
    --build $BuildDir `
    --config $BuildType `
    --parallel

if ($LASTEXITCODE -ne 0) {
    throw "Build failed."
}

Write-Host ""
Write-Host "Build completed successfully." -ForegroundColor Green

$IsMultiConfig = $false
$CacheFile = Join-Path $BuildDir "CMakeCache.txt"
if (Test-Path $CacheFile) {
    $match = Select-String -Path $CacheFile -Pattern "^CMAKE_CONFIGURATION_TYPES:STRING=(.+)"
    if ($match -and $match.Matches.Groups[1].Value.Trim()) {
        $IsMultiConfig = $true
    }
}

if ($IsMultiConfig) {
    $Candidates = @(
        (Join-Path $BuildDir "$BuildType\LookAway.exe"),
        (Join-Path $BuildDir "LookAway.exe")
    )
}
else {
    $Candidates = @(
        (Join-Path $BuildDir "LookAway.exe"),
        (Join-Path $BuildDir "$BuildType\LookAway.exe")
    )
}

$Executable = $Candidates |
    Where-Object { Test-Path $_ } |
    Select-Object -First 1

if (-not $Executable) {
    throw "Build succeeded, but LookAway.exe could not be found."
}

Write-Host "Executable : $Executable" -ForegroundColor DarkGray

if ($NoRun) {
    Write-Host ""
    Write-Host "Skipping launch (-NoRun)." -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "Starting Look Away!..." -ForegroundColor Cyan
Write-Host ""

Start-Process -FilePath $Executable