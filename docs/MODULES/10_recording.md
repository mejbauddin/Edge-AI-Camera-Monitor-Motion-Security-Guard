# Phase 10: Recording Module

## Pipeline

```
Frame feed → RollingBuffer (pre-roll)
Threat trigger → pre-buffer + live + post-buffer → Mp4Writer/H264Encoder → disk
                ↘ SnapshotCapture (PPM threat still)
                ↘ StorageManager (quota enforcement)
                ↘ Database recordings table
```

## Classes

| Class | Role |
|-------|------|
| `Recorder` | `IRecorder` orchestrator |
| `RollingBuffer` | Time-window pre-roll frame buffer |
| `H264Encoder` | FFmpeg H264 encoder (optional) |
| `Mp4Writer` | MP4 when FFmpeg available, `.csxclip` fallback otherwise |
| `SnapshotCapture` | PPM/BGR snapshot export |
| `StorageManager` | Storage scan + oldest-first quota cleanup |

## Clip formats

| Format | When |
|--------|------|
| `.mp4` (H264) | FFmpeg available at build time |
| `.csxclip` | Offline fallback (raw BGR frames + metadata) |

## Tests

```powershell
cmake --build D:\CyberSentinelX-build --config Release
D:\CyberSentinelX-build\tests\Release\csx_recording_tests.exe
```

## Configuration

See `config/default.json` → `recording` for pre/post buffer seconds, output directory, and storage quota.
