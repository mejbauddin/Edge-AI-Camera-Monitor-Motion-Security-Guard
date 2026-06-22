# Phase 1: Core Module

## Classes

- **Enums** — ThreatLevel, DefconLevel, CameraState, BehaviorAnomalyType
- **Frame / Track / ThreatAssessment** — pipeline data transfer objects
- **Interfaces** — abstract contracts for all engines (DI targets)
- **EventBus** — thread-safe pub/sub
- **ConfigService** — nlohmann JSON with dotted-key access
- **Logger** — console + daily rotating file logs
- **RingBuffer** — lock-free SPSC for frame handoff
- **SafeQueue** — bounded MPMC for DB/voice
- **ThreadPool** — `std::packaged_task` worker pool
- **FrameSynchronizer** — aligns frames with tracks/faces by sequence ID
- **StageMetrics** — rolling per-stage latency windows
- **PipelineOrchestrator** — lifecycle + health broadcast loop

## Dependencies

- nlohmann_json (FetchContent)
- spdlog (FetchContent) — header linked; Logger uses lightweight file IO for Phase 1

## Integration

```cpp
auto bus = csx::core::createEventBus();
auto config = csx::core::createConfigService(bus);
auto logger = csx::core::createLogger("logs");
config->load("config/default.json");

csx::core::PipelineOrchestrator pipeline(logger, bus);
pipeline.start();
```
