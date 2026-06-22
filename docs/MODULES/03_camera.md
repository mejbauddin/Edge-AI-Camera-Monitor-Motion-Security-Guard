# Phase 3: Camera Module

## Classes

| Class | Role |
|-------|------|
| `CameraEngine` | Capture thread, reconnect, frame buffer, health |
| `FrameBuffer` | Thread-safe bounded frame queue |
| `CameraDiagnostics` | FPS, latency, dropped frames, state |
| `SyntheticCameraSource` | Software-generated BGR feed for tests |
| `UsbCameraSource` | USB webcam via OpenCV DirectShow |
| `RtspCameraSource` | RTSP IP cameras via OpenCV FFMPEG |
| `IpCameraSource` | HTTP/MJPEG streams |
| `createCameraSource` | URI factory (`usb:0`, `rtsp://`, `http://`, `synthetic:0`) |

## Source URI formats

| URI | Backend |
|-----|---------|
| `usb:0` | USB camera index 0 |
| `rtsp://host/stream` | RTSP |
| `http://host/mjpeg` | IP/HTTP |
| `synthetic:0` | Test pattern (always available) |

## OpenCV (optional)

Install for real cameras:

```powershell
vcpkg install opencv4:x64-windows
cmake -B D:\CyberSentinelX-build -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

Without OpenCV, `synthetic:` sources still work.

## Tests

```powershell
D:\CyberSentinelX-build\tests\Release\csx_camera_tests.exe
```
