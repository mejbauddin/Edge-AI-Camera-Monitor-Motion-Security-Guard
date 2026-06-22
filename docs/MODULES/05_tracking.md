# Phase 5: Tracking Module

## Pipeline

```
Motion blobs → IoU match → Kalman update → velocity estimate → Track IDs
```

## Classes

| Class | Role |
|-------|------|
| `ObjectTracker` | `ITracker` implementation |
| `TrackManager` | ID assignment, lifecycle, history |
| `IoUMatcher` | Greedy highest-IoU assignment |
| `KalmanPredictor` | Position/velocity prediction |
| `VelocityEstimator` | Smoothed velocity, acceleration, heading |

## Tests

```powershell
D:\CyberSentinelX-build\tests\Release\csx_tracking_tests.exe
```
