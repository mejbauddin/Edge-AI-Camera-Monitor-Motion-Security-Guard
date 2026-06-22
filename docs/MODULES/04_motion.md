# Phase 4: Motion Module

## Pipeline

```
Frame → downscale ROI → background subtract (MOG2/KNN)
      → shadow removal → morphology (open/close) → blob analysis → Rect2f[]
```

## Classes

| Class | Role |
|-------|------|
| `MotionEngine` | `IMotionEngine` orchestrator |
| `Mog2Subtractor` | MOG2 (OpenCV or CPU fallback) |
| `KnnSubtractor` | KNN (OpenCV or CPU fallback) |
| `MorphologyFilter` | Erode/dilate noise cleanup |
| `ShadowRemover` | Brightness-based shadow suppression |
| `BlobAnalyzer` | Connected-component blob extraction |

## Configuration (`config/default.json` → `motion`)

- `algorithm`: `mog2` or `knn`
- `learning_rate`, `threshold`, `min_blob_area`, `shadow_removal`

## Tests

```powershell
D:\CyberSentinelX-build\tests\Release\csx_motion_tests.exe
```
