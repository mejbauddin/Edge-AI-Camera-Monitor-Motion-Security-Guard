# Cyber Sentinel X

**Edge AI Camera Monitor & Motion Security Guard** — a real-time tactical security dashboard for Windows. It captures live camera video, detects motion, tracks objects, recognizes faces (YuNet + SFace), scores threats, renders a cyberpunk HUD, and records incidents.

> **Tagline:** Observe · Analyze · Predict · Protect

---

## Table of Contents

1. [Features](#features)
2. [System Requirements](#system-requirements)
3. [Project Structure](#project-structure)
4. [Prerequisites (Windows)](#prerequisites-windows)
5. [Quick Start — One-Command Build & Run](#quick-start--one-command-build--run)
6. [Manual Build (Step by Step)](#manual-build-step-by-step)
7. [Running the Application](#running-the-application)
8. [First-Time Setup](#first-time-setup)
9. [Configuration](#configuration)
10. [Face Enrollment](#face-enrollment)
11. [Packaging for Distribution](#packaging-for-distribution)
12. [Troubleshooting](#troubleshooting)
13. [Development](#development)
14. [License](#license)

---

## Features

| Module | Description |
|--------|-------------|
| **Camera** | USB webcam, RTSP/IP, or synthetic fallback |
| **Motion** | MOG2 background subtraction with shadow removal |
| **Tracking** | Multi-object IoU tracker (up to 32 targets) |
| **Recognition** | YuNet face detection + SFace embeddings, friend/foe classification |
| **Behavior** | Loitering, running, tamper, night-mode anomaly scoring |
| **Threat AI** | Weighted threat matrix with DEFCON-style levels |
| **HUD** | Live tactical overlay on camera feed |
| **Radar** | 8-sector perimeter scope with blip telemetry |
| **Recording** | Pre/post-buffer incident clips (H.264 when FFmpeg available) |
| **Voice** | Spoken alerts on high-threat events |
| **UI** | Qt Quick dashboard — maximized on launch, **F11** toggles fullscreen |

---

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| OS | Windows 10 (64-bit) | Windows 11 |
| CPU | 4-core x64 | 8-core with AVX2 |
| RAM | 4 GB | 8 GB+ |
| GPU | Not required | NVIDIA/AMD for faster DNN |
| Camera | USB webcam or IP stream | 720p+ webcam |
| Disk | 2 GB free (build + models) | 10 GB+ for recordings |

**Software dependencies (build time):**

- Visual Studio 2019 or 2022 with **Desktop development with C++**
- CMake 3.24+
- Qt 6.5+ (Quick, QuickControls2) — tested with **Qt 6.7.3 msvc2019_64**
- OpenCV 4.x (with `videoio`, `imgproc`, `dnn`)
- vcpkg (recommended for dependency management)
- Git (for CMake FetchContent and model download)

**Optional:**

- ONNX Runtime — faster inference
- FFmpeg — H.264 MP4 recording

---

## Project Structure

```
Edge AI Camera Monitor & Motion Security Guard/
├── CMakeLists.txt          # Root build definition
├── config/
│   └── default.json        # Runtime configuration
├── assets/
│   ├── qml/main.qml        # Tactical dashboard UI
│   ├── models/             # YuNet + SFace ONNX (downloaded)
│   └── faces/authorized/   # Authorized face photos
├── src/
│   ├── app/                # Application entry, UI bootstrap
│   ├── vision/             # Camera + motion detection
│   ├── tracking/           # Object tracker
│   ├── recognition/        # Face pipeline
│   ├── behavior/           # Anomaly detection
│   ├── threat/             # Threat scoring
│   ├── hud/                # On-frame overlay renderer
│   ├── radar/              # Perimeter radar model
│   ├── recording/          # Clip recorder
│   ├── voice/              # TTS alerts
│   ├── database/           # SQLite persistence
│   └── ui/                 # Qt bridges + image providers
├── scripts/
│   ├── run.ps1             # Build, deploy Qt, launch
│   ├── download-models.ps1 # Fetch ONNX models
│   └── package.ps1         # Create distributable ZIP
└── tests/                  # GoogleTest unit tests
```

---

## Prerequisites (Windows)

### 1. Install Visual Studio

Install [Visual Studio 2022](https://visualstudio.microsoft.com/) (or 2019) with workload **"Desktop development with C++"**.

Open **"x64 Native Tools Command Prompt"** or use PowerShell — CMake will find MSVC automatically.

### 2. Install CMake

```powershell
winget install Kitware.CMake
```

Or download from [cmake.org](https://cmake.org/download/).

### 3. Install Qt 6

Download the [Qt Online Installer](https://www.qt.io/download-qt-installer) and install:

- **Qt 6.7.x** → **MSVC 2019 64-bit**
- **Qt Quick** and **Qt Quick Controls 2** components

Default install path used in scripts: `C:\Qt\6.7.3\msvc2019_64`

### 4. Install OpenCV

**Option A — Pre-built (used by project scripts):**

1. Download OpenCV Windows pack from [opencv.org](https://opencv.org/releases/)
2. Extract to `C:\opencv\build\opencv\build`
3. Add `C:\opencv\build\opencv\build\x64\vc16\bin` to your **PATH** (or copy DLLs next to the `.exe` after build)

**Option B — vcpkg:**

```powershell
vcpkg install opencv4:x64-windows
```

### 5. Install vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

Set environment variable (optional):

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

### 6. Paths with spaces (important)

If the project folder name contains spaces (e.g. `Edge AI Camera Monitor & Motion Security Guard`), some MSVC tools can fail. The provided `run.ps1` script supports a junction workaround:

```powershell
# Run once as Administrator
mklink /J D:\CSX "D:\Edge AI Camera Monitor & Motion Security Guard"
```

Then build from `D:\CSX` — the script detects this automatically.

---

## Quick Start — One-Command Build & Run

From PowerShell in the project root:

```powershell
.\scripts\run.ps1
```

This script will:

1. Download YuNet + SFace ONNX models
2. Configure CMake with vcpkg + Qt + OpenCV
3. Build `CyberSentinelX.exe` (Release)
4. Run `windeployqt` to copy Qt runtime DLLs
5. Copy OpenCV DLLs and sync `assets/` + `config/`
6. Launch the application **maximized**

**Custom paths:**

```powershell
.\scripts\run.ps1 -BuildType Release -VcpkgRoot "C:\vcpkg" -ProjectRoot "D:\CSX"
```

---

## Manual Build (Step by Step)

### Step 1 — Clone or extract source

```powershell
cd "D:\Edge AI Camera Monitor & Motion Security Guard"
# Or: cd D:\CSX  (if using junction)
```

### Step 2 — Download AI models

```powershell
.\scripts\download-models.ps1
```

This creates:

- `assets/models/yunet_2023mar.onnx`
- `assets/models/sface_2021dec.onnx`

### Step 3 — Configure CMake

```powershell
cmake -B build-csx -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.7.3\msvc2019_64" `
  -DOpenCV_DIR="C:/opencv/build/opencv/build" `
  -DCSX_BUILD_UI=ON
```

Adjust `CMAKE_PREFIX_PATH` and `OpenCV_DIR` to match your installation.

### Step 4 — Build

```powershell
cmake --build build-csx --config Release --target CyberSentinelX
```

### Step 5 — Deploy Qt runtime

```powershell
$ExeDir = "build-csx\Release"
C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe `
  "$ExeDir\CyberSentinelX.exe" `
  --qmldir "$ExeDir\qml"
```

### Step 6 — Copy runtime dependencies

```powershell
# OpenCV (adjust version if different)
Copy-Item "C:\opencv\build\opencv\build\x64\vc16\bin\opencv_world490.dll" $ExeDir -Force

# Sync assets and config (POST_BUILD also copies these, but re-sync after model download)
Copy-Item -Recurse -Force assets\models "$ExeDir\assets\models"
Copy-Item -Recurse -Force assets\faces   "$ExeDir\assets\faces"
Copy-Item -Force config\default.json     "$ExeDir\config\default.json"
```

### Step 7 — Build tests (optional)

```powershell
cmake --build build-csx --config Release
ctest --test-dir build-csx -C Release
```

---

## Running the Application

```powershell
cd build-csx\Release
.\CyberSentinelX.exe
```

Or from project root after a successful `run.ps1`:

```powershell
& ".\build-csx\Release\CyberSentinelX.exe"
```

**UI controls:**

| Key / Action | Effect |
|--------------|--------|
| **F11** | Toggle fullscreen / maximized |
| **◈ ENROLL** (top bar) | Open biometric enrollment drawer |
| **Ctrl+C** (console) | Graceful shutdown |

The window opens **maximized** so all dashboard panels (radar, event stream, telemetry) fit on screen.

---

## First-Time Setup

1. **Connect a webcam** (or configure RTSP in `config/default.json`).
2. **Run the app** — a boot sequence plays for ~4 seconds, then the live feed appears.
3. **Add authorized faces** (see [Face Enrollment](#face-enrollment)).
4. **Check logs** in `build-csx/Release/logs/` if something fails.

On first launch the app creates:

- `data/cyber_sentinel.db` — SQLite database for faces and events
- `recordings/` — incident video clips
- `logs/` — application log files

---

## Configuration

Edit `config/default.json` (copied next to the executable after build).

### Camera

```json
"camera": {
  "default_source": "usb:0",
  "width": 640,
  "height": 480,
  "fps": 30
}
```

| `default_source` | Meaning |
|------------------|---------|
| `usb:0` | First USB webcam |
| `usb:1` | Second USB webcam |
| `rtsp://user:pass@192.168.1.100/stream` | IP camera RTSP URL |
| `synthetic` | Test pattern (no camera needed) |

### Recognition thresholds

```json
"recognition": {
  "enabled": true,
  "match_threshold": 0.42,
  "foe_threshold": 0.30,
  "import_dir": "assets/faces/authorized"
}
```

### Threat levels

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

Restart the application after changing config.

---

## Face Enrollment

### Method 1 — UI drawer

1. Click **◈ ENROLL** in the top command bar.
2. Enter name, country, and role.
3. **BROWSE** for a photo, or click **◉ CAPTURE FROM CAMERA**.
4. Click **◈ ENROLL FROM FILE** or use **↻ IMPORT** for bulk import.

### Method 2 — Drop files in folder

Place photos in `assets/faces/authorized/` using this naming pattern:

```
FirstName_LastName_COUNTRY.jpg
FirstName_LastName_COUNTRY_role.jpg
```

Examples:

```
John_Smith_USA.jpg
Mejba_Hossain_Bangladesh.jpg
```

Restart the app or use **IMPORT** in the enrollment drawer.

---

## Packaging for Distribution

To create a ZIP package for another Windows machine:

```powershell
.\scripts\package.ps1 -BuildType Release -OutputDir package
```

Output: `package/CyberSentinelX-Windows.zip`

**Important:** Recipients still need:

- Visual C++ Redistributable (usually already installed)
- Qt and OpenCV DLLs — run `windeployqt` and copy OpenCV DLLs **before** packaging, or use `run.ps1` which does this automatically

For a complete portable build, run `run.ps1` first, then zip the entire `build-csx\Release\` folder.

---

## Troubleshooting

### UI panels cut off at the bottom

Fixed in current version: window starts **maximized** and the right column scrolls on small displays. Press **F11** for fullscreen.

### `'d:\Edge' is not recognized` during build

Path contains spaces. Create junction `D:\CSX` and build from there (see [Paths with spaces](#6-paths-with-spaces-important)).

### `QML not found` or blank window

Ensure `qml/main.qml` exists next to `CyberSentinelX.exe`:

```powershell
windeployqt.exe CyberSentinelX.exe --qmldir qml
```

### Camera not opening

- Close other apps using the webcam.
- Try `"default_source": "synthetic"` to verify the pipeline without hardware.
- Check `logs/cyber_sentinel_*.log` for `videoio` errors.

### `opencv_world*.dll` not found

Copy the OpenCV DLL from `opencv\build\x64\vc16\bin\` into the same folder as the `.exe`.

### Face recognition not working

1. Run `.\scripts\download-models.ps1`
2. Verify files exist: `assets/models/yunet_2023mar.onnx` and `sface_2021dec.onnx`
3. Ensure OpenCV DNN backend is available (OpenCV build with DNN module)

### Qt platform plugin errors

Run `windeployqt` on the executable. Ensure `platforms/qwindows.dll` is beside the exe.

---

## Development

| CMake option | Default | Description |
|--------------|---------|-------------|
| `CSX_BUILD_UI` | ON | Build Qt Quick dashboard |
| `CSX_BUILD_TESTS` | ON | Build GoogleTest suite |
| `CSX_ENABLE_DEV_MODE` | ON | Extra diagnostics |
| `CSX_STATIC_LINK` | ON | Prefer static linking where possible |

**Tech stack:** C++20 · Qt 6 Quick · OpenCV 4 · SQLite · spdlog · nlohmann/json · ONNX (optional)

**Module libraries:** `csx_core`, `csx_camera`, `csx_motion`, `csx_tracking`, `csx_recognition`, `csx_behavior`, `csx_threat`, `csx_hud`, `csx_radar`, `csx_recording`, `csx_voice`, `csx_alerts`, `csx_ui`

---

## License

Proprietary — All rights reserved.

---

## Support Checklist (hand off source to someone new)

- [ ] Install VS 2022, CMake, Qt 6.7 MSVC 64-bit, OpenCV, vcpkg
- [ ] Clone/copy project (use `D:\CSX` junction if path has spaces)
- [ ] Run `.\scripts\download-models.ps1`
- [ ] Run `.\scripts\run.ps1`
- [ ] Connect webcam, add face photos to `assets/faces/authorized/`
- [ ] Adjust `config/default.json` as needed
- [ ] Press **F11** for fullscreen tactical view
