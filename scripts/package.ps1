# Cyber Sentinel X - Packaging Script
# Creates installer package with all dependencies

param(
    [string]$BuildType = "Release",
    [string]$OutputDir = "package"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Cyber Sentinel X Packaging Script ===" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType" -ForegroundColor Gray
Write-Host "Output Directory: $OutputDir" -ForegroundColor Gray
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$PackageDir = Join-Path $OutputDir "CyberSentinelX"
New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null

Write-Host "[1/6] Building application..." -ForegroundColor Yellow
# Build the application
cmake -B build -DCMAKE_BUILD_TYPE=$BuildType
cmake --build build --config $BuildType

Write-Host "[2/6] Copying executable..." -ForegroundColor Yellow
# Copy executable
Copy-Item -Path "build\$BuildType\CyberSentinelX.exe" -Destination $PackageDir -Force

Write-Host "[3/6] Copying configuration files..." -ForegroundColor Yellow
# Copy config
New-Item -ItemType Directory -Force -Path (Join-Path $PackageDir "config") | Out-Null
Copy-Item -Path "config\*.json" -Destination (Join-Path $PackageDir "config") -Force

Write-Host "[4/6] Copying assets..." -ForegroundColor Yellow
# Copy assets (models, fonts, audio if they exist)
if (Test-Path "assets") {
    Copy-Item -Path "assets" -Destination $PackageDir -Recurse -Force
} else {
    New-Item -ItemType Directory -Force -Path (Join-Path $PackageDir "assets") | Out-Null
    New-Item -ItemType Directory -Force -Path (Join-Path $PackageDir "assets\models") | Out-Null
    Write-Host "  Created placeholder assets directory" -ForegroundColor Gray
}

Write-Host "[5/6] Copying documentation..." -ForegroundColor Yellow
# Copy docs
New-Item -ItemType Directory -Force -Path (Join-Path $PackageDir "docs") | Out-Null
Copy-Item -Path "docs\*.md" -Destination (Join-Path $PackageDir "docs") -Force -ErrorAction SilentlyContinue

# Create README
$ReadmeContent = @"
# Cyber Sentinel X - AI Security Operating System

## Installation
1. Extract all files to a folder
2. Run CyberSentinelX.exe
3. Configure camera settings in config/default.json

## Requirements
- Windows 10/11
- Webcam or IP camera
- 4GB RAM minimum
- GPU recommended for AI acceleration

## Configuration
Edit `config/default.json` to customize:
- Camera settings
- Threat thresholds
- HUD appearance
- Recording options

## Documentation
See `docs/` folder for detailed documentation.

## License
Proprietary - All rights reserved
"@
Set-Content -Path (Join-Path $PackageDir "README.txt") -Value $ReadmeContent

Write-Host "[6/6] Creating ZIP archive..." -ForegroundColor Yellow
# Create ZIP
$ZipPath = Join-Path $OutputDir "CyberSentinelX-Windows.zip"
Compress-Archive -Path $PackageDir\* -DestinationPath $ZipPath -Force

Write-Host ""
Write-Host "=== Packaging Complete ===" -ForegroundColor Green
Write-Host "Package: $ZipPath" -ForegroundColor Cyan
Write-Host "Size: $((Get-Item $ZipPath).Length / 1MB) MB" -ForegroundColor Gray
