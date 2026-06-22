# CYBER SENTINEL X — Architecture (v1.1)

See also: [UI_VISION.md](UI_VISION.md) for futuristic animation and HUD specifications.

## Module Map

```
core → utils → camera → motion → tracking → recognition → behavior → threat
                    ↘ database ↗              ↘ recording ↗
core → hud, radar, voice, alerts → ui → app
```

## Phase 1 Complete: `core`

| Component | Path | Role |
|-----------|------|------|
| Types | `src/core/types/` | Frame, Track, ThreatAssessment, SystemHealth, Enums |
| Interfaces | `src/core/interfaces/` | ICameraSource, IThreatEngine, IEventBus, etc. |
| EventBus | `src/core/events/` | Pub/sub decoupling |
| ConfigService | `src/core/config/` | JSON config, live reload |
| Logger | `src/core/logging/` | spdlog-style multi-sink logging |
| Threading | `src/core/threading/` | RingBuffer, SafeQueue, ThreadPool |
| Pipeline | `src/core/pipeline/` | FrameSynchronizer, StageMetrics, Orchestrator |

## Build

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release
```

## Tests (Phase 1)

- `ring_buffer_test` — SPSC order, capacity, clear
- `event_bus_test` — subscribe, publish, unsubscribe
- `config_service_test` — JSON load, reload events
- `thread_pool_test` — parallel task execution
- `stage_metrics_test` — rolling latency average

## Roadmap

| Phase | Module | Status |
|-------|--------|--------|
| 0 | Architecture | ✅ |
| 1 | core | ✅ |
| 2 | utils | ✅ |
| 3 | vision/camera | ✅ |
| 4 | vision/motion | ✅ |
| 5 | tracking | ✅ |
| 6 | recognition | ✅ |
| 7 | database | ✅ |
| 8 | behavior | ✅ |
| 9 | threat | ✅ |
| 10 | recording | ✅ |
| ... | ... | ... |
| 15 | ui/dashboard + animations | Pending |
