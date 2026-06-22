# Phase 11 — `ui/hud` Module

## Role

Composites a neon tactical HUD overlay directly onto raw BGR frames at ≤ 4 ms/frame budget.

## Class Map

| Class | Interface | Purpose |
|-------|-----------|---------|
| `HudRenderer` | `IHudRenderer` | Full compositor; delegated to by app/pipeline |

## Drawing Layers (bottom → top)

| Layer | Feature |
|-------|---------|
| 1 | Tactical coordinate grid (10×10 cell) + center crosshairs |
| 2 | Target acquisition reticles (corner brackets, confidence arc, ID label) |
| 3 | Velocity arrows (neon, magnitude-scaled) |
| 4 | Kalman prediction vectors (dashed cyan) |
| 5 | Horizontal scanline sweep |
| 6 | Threat meter (arc gauge, right edge) |
| 7 | DEFCON panel label (right edge) |
| 8 | Telemetry strip (top bar: FPS, CPU, RAM, time + REC blink) |
| 9 | Critical edge pulse (red border, oscillating alpha — Red/Critical only) |

## Color Palette

| Token | Hex | Usage |
|-------|-----|-------|
| Cyan | `#00E5FF` | Grid lines, unknown targets, scanline |
| Green | `#00E676` | Authorized / Friend reticles |
| Crimson | `#FF1744` | Foe reticles, critical edge pulse |
| Amber | `#FF6D00` | Orange/Red threat indicators |
| Ice White | `#E0F7FA` | Telemetry text |
| Void | `#0A0E17` | Panel backgrounds |

## OpenCV Gating

All OpenCV draw calls are gated with `#if defined(CSX_HAS_OPENCV)`. When OpenCV is absent, `render()` performs a plain frame buffer copy so the pipeline continues operating.

## Performance Notes

- All draws are single-pass (no frame copies except the initial owned-buffer allocation).
- `cv::addWeighted` blends are applied to minimal-sized ROIs, not the full frame.
- Grid draw: O(20) lines.
- Reticle draw: O(tracks) — ~8 line segments per track.
- Total measured budget target: ≤ 4 ms at 640×480.

## Build

Compiled as part of `csx_hud` static library. Links `csx_core`, `csx_tracking`, `csx_threat`, and optionally OpenCV via `csx_link_camera_deps`.

## Tests

10 test cases in `tests/unit/ui/hud/hud_renderer_test.cpp`:

| Test | Verifies |
|------|---------|
| `RendersPassThroughOnValidFrame` | Output is valid, same dimensions |
| `OutputIsIndependentCopy` | Input frame not mutated |
| `HandlesEmptyTracksGracefully` | No crash on empty vectors |
| `RendersWithAuthorizedFaceTrack` | Friend (green) reticle |
| `RendersWithFoeFaceTrack` | Foe (crimson) reticle + red threat |
| `RendersMultipleTracks` | 5 concurrent tracks, 640×480 |
| `CriticalThreatDoesNotCrash` | Critical edge pulse |
| `RecordingIndicatorToggle` | REC blink atomic flag |
| `HealthReturnsOnline` | `EngineStatus::Online` |
| `MultipleSequentialFrames` | 30 frames without leak |
| `TrackOutsideFrameBoundsHandled` | Clamping of out-of-bounds bbox |
