# Cyber Sentinel X — launch script (build + deploy Qt + run)
# Tip: if MSVC build fails with "'d:\Edge' is not recognized", use junction D:\CSX
param(
    [string]$BuildType = "Release",
    [string]$VcpkgRoot = "C:\vcpkg",
    [string]$ProjectRoot = ""
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$realRoot = Split-Path -Parent $scriptDir

if (-not $ProjectRoot) {
    if (Test-Path "D:\CSX\CMakeLists.txt") {
        $ProjectRoot = "D:\CSX"
    } else {
        $ProjectRoot = $realRoot
    }
}
Set-Location $ProjectRoot

& (Join-Path $realRoot "scripts\download-models.ps1")

$BuildDir = if ($ProjectRoot -eq "D:\CSX") { "build-csx" } else { "build" }
$env:TEMP = Join-Path $ProjectRoot "$BuildDir\tmp"
$env:TMP = $env:TEMP
New-Item -ItemType Directory -Force -Path $env:TEMP | Out-Null

$Toolchain = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
if (-not (Test-Path $Toolchain)) {
    Write-Error "vcpkg not found at $VcpkgRoot. Install vcpkg first."
}

Write-Host "=== Configuring CMake ($ProjectRoot) ===" -ForegroundColor Cyan
cmake -B $BuildDir -DCMAKE_BUILD_TYPE=$BuildType `
    -DCMAKE_TOOLCHAIN_FILE="$Toolchain" `
    -DCMAKE_PREFIX_PATH="C:\Qt\6.7.3\msvc2019_64" `
    -DOpenCV_DIR="C:/opencv/build/opencv/build" `
    -DCSX_BUILD_UI=ON

Write-Host "=== Building ===" -ForegroundColor Cyan
cmake --build $BuildDir --config $BuildType --target CyberSentinelX

$ExeDir = Join-Path $ProjectRoot "$BuildDir\$BuildType"
$Exe = Join-Path $ExeDir "CyberSentinelX.exe"

if (-not (Test-Path $Exe)) {
    Write-Error "Build failed — executable not found at $Exe"
}

$onlineQt = "C:\Qt\6.7.3\msvc2019_64\bin"
$vcpkgQt = Join-Path $VcpkgRoot "installed\x64-windows\tools\Qt6\bin"
if (Test-Path (Join-Path $onlineQt "windeployqt.exe")) {
    $QtBin = $onlineQt
} elseif (Test-Path (Join-Path $vcpkgQt "windeployqt.exe")) {
    $QtBin = $vcpkgQt
} else {
    $candidates = Get-ChildItem "C:\Qt\*\msvc*\bin\windeployqt.exe" -ErrorAction SilentlyContinue
    if ($candidates) { $QtBin = $candidates[0].DirectoryName }
}

if ($QtBin) {
    Write-Host "=== Deploying Qt runtime ===" -ForegroundColor Cyan
    & (Join-Path $QtBin "windeployqt.exe") $Exe --qmldir (Join-Path $ExeDir "qml") 2>&1 | Out-Null
}

$OpenCvBin = "C:\opencv\build\opencv\build\x64\vc16\bin"
if (Test-Path $OpenCvBin) {
    Write-Host "=== Copying OpenCV DLLs ===" -ForegroundColor Cyan
    Copy-Item (Join-Path $OpenCvBin "opencv_world490.dll") $ExeDir -Force
}

Write-Host "=== Syncing assets ===" -ForegroundColor Cyan
foreach ($folder in @("assets\models", "assets\faces", "config")) {
    $src = Join-Path $realRoot $folder
    $dst = Join-Path $ExeDir $folder
    if (Test-Path $src) {
        New-Item -ItemType Directory -Force -Path $dst | Out-Null
        Copy-Item -Path (Join-Path $src "*") -Destination $dst -Recurse -Force
    }
}

Write-Host "=== Launching Cyber Sentinel X ===" -ForegroundColor Green
Set-Location $ExeDir
& $Exe
