# Phase 9: Threat Module

## Pipeline

```
Tracks + FaceMatches + BehaviorAnomalies
  → BehaviorFusion → ThreatScorer → ThreatHistory → PriorityCalculator
  → ThreatAssessment (level, DEFCON, score, priority, reason)
```

## Classes

| Class | Role |
|-------|------|
| `ThreatEngine` | `IThreatEngine` orchestrator |
| `ThreatScorer` | Weighted motion/track/face/behavior/context fusion |
| `BehaviorFusion` | Aggregates behavior anomalies into behavior score |
| `PriorityCalculator` | Priority, severity, confidence, decision reason |
| `ThreatHistory` | Score smoothing with gradual de-escalation |

## Scoring Weights (default)

| Signal | Weight |
|--------|--------|
| Motion | 0.15 |
| Track | 0.20 |
| Face | 0.30 |
| Behavior | 0.25 |
| Context (night) | 0.10 |

## Threat Levels

| Level | Score Range |
|-------|-------------|
| Green | 0–20 |
| Yellow | 21–40 |
| Orange | 41–60 |
| Red | 61–80 |
| Critical | 81–100 |

DEFCON maps automatically: Green=DEFCON 5 → Critical=DEFCON 1.

## Tests

```powershell
cmake --build D:\CyberSentinelX-build --config Release
D:\CyberSentinelX-build\tests\Release\csx_threat_tests.exe
```

## Configuration

See `config/default.json` → `threat` section for weights, level thresholds, and auto-record level.
