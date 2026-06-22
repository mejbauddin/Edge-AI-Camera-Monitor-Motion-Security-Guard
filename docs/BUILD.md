# Cyber Sentinel X - Build Instructions

## Prerequisites

### Required Software
- **CMake** 3.24 or later
- **Visual Studio 2022** with C++ development tools
- **Qt 6** (Qt Quick, Qt Charts)
- **vcpkg** package manager

### Required Libraries (via vcpkg)
```bash
vcpkg install opencv4:x64-windows
vcpkg install onnxruntime:x64-windows
vcpkg install ffmpeg:x64-windows
vcpkg install sqlite3:x64-windows
vcpkg install spdlog:x64-windows
vcpkg install nlohmann-json:x64-windows
```

## Build Steps

### 1. Configure CMake
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

### 2. Build
```powershell
cmake --build build --config Release
```

### 3. Run Tests
```powershell
ctest --test-dir build -C Release
```

### 4. Package
```powershell
.\scripts\package.ps1
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CSX_BUILD_TESTS` | ON | Build unit and integration tests |
| `CSX_ENABLE_DEV_MODE` | ON | Enable developer diagnostics overlay |
| `CSX_STATIC_LINK` | ON | Prefer static linking of dependencies |

## Module-Specific Builds

To build specific modules only:
```powershell
cmake -B build -DCSX_BUILD_TESTS=OFF
cmake --build build --target csx_core --config Release
cmake --build build --target csx_camera --config Release
```

## Troubleshooting

### Qt Not Found
Set `CMAKE_PREFIX_PATH` to your Qt installation:
```powershell
cmake -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2022_64"
```

### OpenCV Not Found
Ensure vcpkg toolchain is used:
```powershell
cmake -B build -DCMAKE_TOOLCHAIN_FILE="path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### Missing ONNX Models
Download required models to `assets/models/`:
- `yunet_2023mar.onnx` - Face detection
- `arcface_w600k_r50.onnx` - Face recognition

## Performance Targets

The application is designed to run at **60 FPS** on modern hardware:
- Camera capture: ≤ 8 ms
- Motion detection: ≤ 10 ms
- Object tracking: ≤ 10 ms
- Face recognition: ≤ 25 ms (amortized @ 5 Hz)
- Threat assessment: ≤ 5 ms
- HUD rendering: ≤ 6 ms

Run performance tests to validate:
```powershell
.\build\Release\performance_tests.exe
```
