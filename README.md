<div align="center">

![Cyber Sentinel X Banner](docs/images/banner.png)

# CYBER SENTINEL X

### Edge AI Camera Monitor & Motion Security Guard

*Real-time tactical security dashboard for Windows — motion detection, face recognition, threat scoring, and cyberpunk HUD.*

<br>

[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11-0078D4?style=for-the-badge&logo=windows&logoColor=white)]()
[![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)]()
[![Qt](https://img.shields.io/badge/Qt-6.7-41CD52?style=for-the-badge&logo=qt&logoColor=white)]()
[![OpenCV](https://img.shields.io/badge/OpenCV-4.x-5C3EE8?style=for-the-badge&logo=opencv&logoColor=white)]()
[![CMake](https://img.shields.io/badge/CMake-3.24+-064F8C?style=for-the-badge&logo=cmake&logoColor=white)]()

**Observe · Analyze · Predict · Protect**

[Quick Start](#-quick-start) ·
[Download & Build](#-download--full-build-guide) ·
[Clean Rebuild](#-complete-clean-rebuild) ·
[Configuration](#%EF%B8%8F-configuration) ·
[Troubleshooting](#-troubleshooting)

</div>

---

## Overview

**Cyber Sentinel X** (AISOS v1.0) transforms a standard webcam into a full **AI Security Operating System**. It runs a multi-stage vision pipeline on the edge — no cloud required — and displays everything on a tactical Qt Quick dashboard inspired by cyberpunk command interfaces.

| Capability | Technology |
|:-----------|:-----------|
| Live camera feed | OpenCV `videoio` — USB, RTSP/IP, synthetic fallback |
| Motion detection | MOG2 background subtraction |
| Object tracking | IoU tracker (up to 32 targets) |
| Face recognition | YuNet detector + SFace embeddings |
| Friend / Foe | SQLite biometric vault + live classification |
| Threat engine | Weighted DEFCON-style scoring |
| HUD overlay | Tactical reticles, velocity vectors, threat halos |
| Perimeter radar | 8-sector sweep scope with blip telemetry |
| Incident recording | Pre/post-buffer clips (H.264 with FFmpeg) |
| Voice alerts | Spoken warnings on high-threat events |

---

## Screenshots & Architecture

<div align="center">

![Pipeline Architecture](docs/images/architecture.png)

*Vision pipeline: Camera → Motion → Tracking → Recognition → Threat AI → Dashboard*

</div>

```mermaid
flowchart LR
    CAM[📷 Camera] --> MOT[Motion MOG2]
    MOT --> TRK[Object Tracker]
    TRK --> REC[YuNet + SFace]
    REC --> THR[Threat AI]
    THR --> HUD[HUD Overlay]
    THR --> RAD[Perimeter Radar]
    THR --> DB[(SQLite Vault)]
    HUD --> UI[Qt Dashboard]
    RAD --> UI
    THR --> REC2[Incident Recorder]
```

---

## System Requirements

| | Minimum | Recommended |
|:--|:--------|:------------|
| **OS** | Windows 10 64-bit | Windows 11 |
| **CPU** | 4-core x64 | 8-core + AVX2 |
| **RAM** | 4 GB | 8 GB+ |
| **GPU** | Not required | Discrete GPU for faster DNN |
| **Camera** | USB webcam or IP stream | 720p+ webcam |
| **Disk** | 3 GB free | 10 GB+ (recordings) |

### Build-time dependencies

| Tool | Version | Notes |
|:-----|:--------|:------|
| [Visual Studio](https://visualstudio.microsoft.com/) | 2019 or 2022 | Workload: **Desktop development with C++** |
| [CMake](https://cmake.org/download/) | 3.24+ | `winget install Kitware.CMake` |
| [Qt](https://www.qt.io/download-qt-installer) | 6.5+ | **MSVC 2019 64-bit** + Qt Quick + Quick Controls 2 |
| [OpenCV](https://opencv.org/releases/) | 4.x | `videoio`, `imgproc`, `dnn` modules |
| [vcpkg](https://github.com/microsoft/vcpkg) | latest | Dependency toolchain |
| [Git](https://git-scm.com/) | latest | Clone repo + CMake FetchContent |

> **Optional:** ONNX Runtime (faster inference) · FFmpeg (H.264 MP4 recording)

---

## Project Structure

```
CyberSentinelX/
├── CMakeLists.txt              # Root build
├── README.md                   # You are here
├── config/
│   └── default.json            # Runtime settings
├── assets/
│   ├── qml/main.qml            # Tactical dashboard UI
│   ├── models/                 # YuNet + SFace ONNX (downloaded)
│   └── faces/authorized/       # Authorized face photos
├── docs/
│   └── images/                 # README banner & diagrams
├── src/
│   ├── app/                    # Entry point + UI bootstrap
│   ├── vision/                 # Camera + motion
│   ├── tracking/               # Multi-object tracker
│   ├── recognition/            # Face pipeline
│   ├── behavior/               # Anomaly detection
│   ├── threat/                 # Threat scoring engine
│   ├── hud/                    # On-frame overlay
│   ├── radar/                  # Perimeter scope
│   ├── recording/              # Clip recorder
│   ├── voice/                  # TTS alerts
│   ├── database/               # SQLite persistence
│   └── ui/                     # Qt bridges + image providers
├── scripts/
│   ├── run.ps1                 # ⭐ One-command build + run
│   ├── download-models.ps1     # Fetch ONNX models
│   └── package.ps1             # Create distributable ZIP
└── tests/                      # GoogleTest unit tests
```

---

## Quick Start

> **Already have all prerequisites installed?** Three commands:

```powershell
git clone <your-repo-url> CyberSentinelX
cd CyberSentinelX
.\scripts\run.ps1
```

The app launches automatically from `build-csx\Release\CyberSentinelX.exe`.

---

## Download & Full Build Guide

Follow every step below if you are **cloning this repo for the first time** and want to build and run from source.

### Step 0 — Clone the repository

```powershell
git clone <your-repo-url> CyberSentinelX
cd CyberSentinelX
```

Or download the ZIP from GitHub, extract it, then:

```powershell
cd path\to\CyberSentinelX
```

<details>
<summary><strong>⚠️ Folder path contains spaces?</strong> (click to expand)</summary>

MSVC can fail on paths like `D:\Edge AI Camera Monitor & Motion Security Guard`. Create a junction **once** (Administrator PowerShell):

```powershell
mklink /J D:\CSX "D:\Edge AI Camera Monitor & Motion Security Guard"
cd D:\CSX
```

The `run.ps1` script auto-detects `D:\CSX` and uses `build-csx` as the build directory.

</details>

---

### Step 1 — Install prerequisites

#### 1a. Visual Studio

Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with workload:

> **Desktop development with C++**

#### 1b. CMake

```powershell
winget install Kitware.CMake
```

#### 1c. Qt 6

1. Download the [Qt Online Installer](https://www.qt.io/download-qt-installer)
2. Install **Qt 6.7.x → MSVC 2019 64-bit**
3. Include components: **Qt Quick**, **Qt Quick Controls 2**

Default path used by scripts:

```
C:\Qt\6.7.3\msvc2019_64
```

#### 1d. OpenCV

1. Download the Windows pack from [opencv.org/releases](https://opencv.org/releases/)
2. Extract to:

```
C:\opencv\build\opencv\build
```

3. (Optional) Add to PATH:

```
C:\opencv\build\opencv\build\x64\vc16\bin
```

#### 1e. vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

Optional — set environment variable:

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

---

### Step 2 — Download AI models

```powershell
.\scripts\download-models.ps1
```

This downloads into `assets\models\`:

| File | Purpose |
|:-----|:--------|
| `yunet_2023mar.onnx` | Face detection |
| `sface_2021dec.onnx` | Face embeddings / recognition |

---

### Step 3 — Build, deploy, and run (automated)

```powershell
.\scripts\run.ps1
```

`run.ps1` performs all of the following automatically:

| # | Action |
|:-:|:-------|
| 1 | Configure CMake with vcpkg + Qt + OpenCV |
| 2 | Compile `CyberSentinelX.exe` (Release) |
| 3 | Run `windeployqt` — copies Qt runtime DLLs |
| 4 | Copy OpenCV DLL next to the executable |
| 5 | Sync `assets/` and `config/` into the output folder |
| 6 | Launch the dashboard (maximized) |

**Custom paths** (if your tools are installed elsewhere):

```powershell
.\scripts\run.ps1 `
  -BuildType Release `
  -VcpkgRoot "C:\vcpkg" `
  -ProjectRoot "D:\CSX"
```

---

### Step 4 — Run the application

After a successful build:

```powershell
cd build-csx\Release
.\CyberSentinelX.exe
```

Or from the project root:

```powershell
.\build-csx\Release\CyberSentinelX.exe
```

| Control | Action |
|:--------|:-------|
| **F11** | Toggle fullscreen ↔ maximized |
| **◈ ENROLL** | Open biometric enrollment drawer |
| **Ctrl+C** | Graceful shutdown (console) |

On first launch the app creates next to the `.exe`:

```
data/cyber_sentinel.db    ← face vault + events
logs/                     ← application logs
recordings/               ← incident video clips
```

---

### Step 5 — First-time setup checklist

- [ ] Connect a USB webcam (or set `synthetic` in config to test without hardware)
- [ ] Wait for the ~4 second boot sequence
- [ ] Enroll authorized faces via **◈ ENROLL** or drop photos in `assets/faces/authorized/`
- [ ] Press **F11** for fullscreen tactical view

---

## Complete Clean Rebuild

Use this when you want to **wipe all build artifacts** and compile everything from scratch — after pulling new code, fixing a broken build, or cleaning up disk space.

### Method A — One command (recommended)

```powershell
# 1. Close CyberSentinelX if running
# 2. From project root:
cd CyberSentinelX          # or: cd D:\CSX

Remove-Item -Recurse -Force build-csx -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force build     -ErrorAction SilentlyContinue

.\scripts\run.ps1
```

That's it. Old object files, CMake cache, and stale DLLs are removed, then everything is rebuilt and launched.

---

### Method B — Manual clean rebuild

```powershell
cd CyberSentinelX

# ── 1. Remove old build output ──────────────────────────
Remove-Item -Recurse -Force build-csx -ErrorAction SilentlyContinue

# ── 2. Download / refresh AI models ─────────────────────
.\scripts\download-models.ps1

# ── 3. Configure CMake ──────────────────────────────────
cmake -B build-csx -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.7.3\msvc2019_64" `
  -DOpenCV_DIR="C:/opencv/build/opencv/build" `
  -DCSX_BUILD_UI=ON

# ── 4. Compile ──────────────────────────────────────────
cmake --build build-csx --config Release --target CyberSentinelX

# ── 5. Deploy Qt runtime (required for GUI) ─────────────
$ExeDir = "build-csx\Release"
C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe `
  "$ExeDir\CyberSentinelX.exe" `
  --qmldir "$ExeDir\qml"

# ── 6. Copy OpenCV DLL ──────────────────────────────────
Copy-Item "C:\opencv\build\opencv\build\x64\vc16\bin\opencv_world490.dll" `
  $ExeDir -Force

# ── 7. Sync assets & config ─────────────────────────────
Copy-Item -Recurse -Force assets\models "$ExeDir\assets\models"
Copy-Item -Recurse -Force assets\faces   "$ExeDir\assets\faces"
New-Item -ItemType Directory -Force -Path "$ExeDir\config" | Out-Null
Copy-Item -Force config\default.json     "$ExeDir\config\default.json"

# ── 8. Run ──────────────────────────────────────────────
cd $ExeDir
.\CyberSentinelX.exe
```

> Adjust `CMAKE_PREFIX_PATH`, `OpenCV_DIR`, and the `opencv_world490.dll` filename if your versions differ.

---

### Rebuild unit tests (optional)

```powershell
cmake --build build-csx --config Release
ctest --test-dir build-csx -C Release
```

---

## Face Enrollment

### Via UI

1. Click **◈ ENROLL** in the top command bar
2. Enter **name**, **country**, and **role**
3. **BROWSE** for a photo or **◉ CAPTURE FROM CAMERA**
4. Click **◈ ENROLL FROM FILE** or **↻ IMPORT** for bulk import

### Via folder drop

Place photos in `assets/faces/authorized/` using this naming pattern:

```
FirstName_LastName_COUNTRY.jpg
FirstName_LastName_COUNTRY_role.jpg
```

**Examples:**

```
John_Smith_USA.jpg
Mejba_Hossain_Bangladesh_operator.jpg
```

Restart the app or use **↻ IMPORT** in the enrollment drawer.

---

## Configuration

Edit `config/default.json` (copied next to the `.exe` after build). Restart the app after changes.

<details>
<summary><strong>Camera settings</strong></summary>

```json
"camera": {
  "default_source": "usb:0",
  "width": 640,
  "height": 480,
  "fps": 30
}
```

| `default_source` | Description |
|:-----------------|:----------|
| `usb:0` | First USB webcam |
| `usb:1` | Second USB webcam |
| `rtsp://user:pass@192.168.1.100/stream` | IP camera RTSP URL |
| `synthetic` | Test pattern — no camera needed |

</details>

<details>
<summary><strong>Recognition thresholds</strong></summary>

```json
"recognition": {
  "enabled": true,
  "match_threshold": 0.42,
  "foe_threshold": 0.30,
  "import_dir": "assets/faces/authorized"
}
```

</details>

<details>
<summary><strong>Threat levels & auto-record</strong></summary>

```json
"threat": {
  "levels": {
    "green_max": 20,
    "yellow_max": 40,
    "orange_max": 60,
    "red_max": 80
  },
  "auto_record_level": "orange"
}
```

</details>

---

## Packaging for Distribution

To ship a portable build to another Windows PC:

```powershell
# 1. Full build with deployment
.\scripts\run.ps1

# 2. Zip the output folder
Compress-Archive -Path "build-csx\Release\*" `
  -DestinationPath "CyberSentinelX-Portable.zip" -Force
```

Or use the packaging script:

```powershell
.\scripts\package.ps1 -BuildType Release -OutputDir package
```

The recipient needs **Visual C++ Redistributable** (usually pre-installed on Windows). Qt and OpenCV DLLs are included if you ran `windeployqt` and copied OpenCV before zipping.

---

## Troubleshooting

<details>
<summary><strong>Build fails: <code>'d:\Edge' is not recognized</code></strong></summary>

Your project path contains spaces. Create a junction and build from `D:\CSX`:

```powershell
mklink /J D:\CSX "D:\Full\Path\To\Project"
cd D:\CSX
.\scripts\run.ps1
```

</details>

<details>
<summary><strong>GUI does not appear / QML not found</strong></summary>

Deploy Qt DLLs next to the executable:

```powershell
cd build-csx\Release
C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe CyberSentinelX.exe --qmldir qml
```

Ensure `qml/main.qml` exists beside `CyberSentinelX.exe`.

</details>

<details>
<summary><strong>Camera not opening</strong></summary>

- Close other apps using the webcam (Zoom, Teams, etc.)
- Set `"default_source": "synthetic"` in config to test without hardware
- Check `logs/cyber_sentinel_*.log` for `videoio` errors

</details>

<details>
<summary><strong><code>opencv_world*.dll</code> not found</strong></summary>

```powershell
Copy-Item "C:\opencv\build\opencv\build\x64\vc16\bin\opencv_world490.dll" `
  "build-csx\Release\" -Force
```

</details>

<details>
<summary><strong>Face recognition not working</strong></summary>

1. Run `.\scripts\download-models.ps1`
2. Verify these files exist in `build-csx\Release\assets\models\`:
   - `yunet_2023mar.onnx`
   - `sface_2021dec.onnx`
3. Ensure OpenCV was built with the DNN module

</details>

<details>
<summary><strong>UI panels cut off</strong></summary>

The window opens **maximized** by default. Press **F11** for fullscreen. The right column scrolls on smaller displays.

</details>

---

## Development

| CMake option | Default | Description |
|:-------------|:--------|:------------|
| `CSX_BUILD_UI` | `ON` | Build Qt Quick dashboard |
| `CSX_BUILD_TESTS` | `ON` | Build GoogleTest suite |
| `CSX_ENABLE_DEV_MODE` | `ON` | Extra diagnostics |
| `CSX_STATIC_LINK` | `ON` | Prefer static linking |

**Tech stack:** C++20 · Qt 6 Quick · OpenCV 4 · SQLite · spdlog · nlohmann/json

**Core modules:** `csx_core` · `csx_camera` · `csx_motion` · `csx_tracking` · `csx_recognition` · `csx_behavior` · `csx_threat` · `csx_hud` · `csx_radar` · `csx_recording` · `csx_voice` · `csx_alerts` · `csx_ui`

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for module-level design notes.

---

## New Developer Checklist

```
[ ] Install VS 2022, CMake, Qt 6.7 MSVC 64-bit, OpenCV, vcpkg, Git
[ ] Clone repo (use D:\CSX junction if path has spaces)
[ ] Run .\scripts\download-models.ps1
[ ] Run .\scripts\run.ps1
[ ] Connect webcam · enroll faces · tune config/default.json
[ ] Press F11 for fullscreen tactical view
```

---

## License

Proprietary — All rights reserved.

---

<div align="center">

**CYBER SENTINEL X** — *AISOS v1.0*

*Observe · Analyze · Predict · Protect*

</div>
