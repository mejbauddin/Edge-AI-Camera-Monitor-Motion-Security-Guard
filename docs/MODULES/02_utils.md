# Phase 2: Utils Module

## Classes

| Class | File | Role |
|-------|------|------|
| `Timer` | `Timer.hpp/.cpp` | High-resolution elapsed time measurement |
| `ScopedTimer` | `Timer.hpp/.cpp` | RAII scope timer writing milliseconds to a reference |
| `FpsCounter` | `FpsCounter.hpp/.cpp` | Rolling-window FPS and frame time |
| `PathResolver` | `PathResolver.hpp/.cpp` | Install-relative paths for config, models, DB, logs |
| `SystemMetrics` | `SystemMetrics.hpp/.cpp` | Windows CPU and RAM sampling |
| `CvHelpers` | `CvHelpers.hpp/.cpp` | Raw BGR buffer resize, fill, region copy (no OpenCV dep) |

## Dependencies

- `csx_core` (types only — no circular dependency)

## Usage

```cpp
csx::utils::PathResolver paths;
paths.setRootOverride("D:/CyberSentinelX-root");
const auto configPath = paths.configFile();

csx::utils::FpsCounter fps;
fps.tick();

csx::utils::SystemMetrics metrics;
metrics.sample();
const auto cpu = metrics.latest().cpuPercent;
```

## Tests

```powershell
cmake --build D:\CyberSentinelX-build --config Release
D:\CyberSentinelX-build\tests\Release\csx_utils_tests.exe
```
