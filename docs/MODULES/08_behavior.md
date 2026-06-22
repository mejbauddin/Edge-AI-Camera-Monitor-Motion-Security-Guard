# Phase 8: Behavior Module

## Pipeline

```
Tracks + Frame → BaselineLearner → Detectors → AnomalyScorer → BehaviorAnomaly[]
```

## Classes

| Class | Role |
|-------|------|
| `BehaviorEngine` | `IBehaviorEngine` orchestrator |
| `BaselineLearner` | Learns normal velocity, occupancy, brightness, edge energy |
| `IntrusionDetector` | Restricted-zone polygon breach detection |
| `LoiteringDetector` | Dwell-time analysis with low-velocity threshold |
| `MotionPatternAnalyzer` | Running, falling, unusual movement, night activity |
| `TamperDetector` | Brightness/blur tamper + idle camera detection |
| `AnomalyScorer` | Score normalization, deduplication, threshold filter |

## Anomaly Types

- Intrusion, Loitering, Running, Falling
- Night activity, Unusual movement
- Camera tampering, Idle camera
- Unexpected object, Unexpected disappearance

## Tests

```powershell
cmake --build build --config Release
build\tests\Release\csx_behavior_tests.exe
```

## Configuration

See `config/default.json` → `behavior` section for thresholds and night-hour window.

Restricted zones are configured as polygons in `BehaviorSettings::zones` (defaults to a center perimeter box).
