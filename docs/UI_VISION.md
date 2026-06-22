# CYBER SENTINEL X — Futuristic UI & Animation Vision

**Design language:** Original tactical-neural command interface — defense-contractor precision, autonomous AI defense grid, holographic operations center. Inspired by the *genre* of advanced military/industrial AI systems without reproducing copyrighted film or game assets.

---

## Visual Identity

| Element | Specification |
|---------|---------------|
| **Primary palette** | Cyan `#00E5FF`, ice white `#E0F7FA`, deep void `#0A0E17` |
| **Alert palette** | Amber `#FF6D00`, crimson `#FF1744`, authorized green `#00E676` |
| **Typography** | Monospace telemetry + condensed display headers |
| **Geometry** | Hex grids, concentric rings, angular brackets, thin 1px vector lines |
| **Depth** | Layered glass panels, parallax hologram offset, subtle scanline overlay |
| **Motion** | Ease-out cubic, 60 Hz UI, GPU-friendly Qt Quick animations |

---

## Boot Sequence (7 phases)

Cinematic system initialization on launch — each phase triggers `boot.phase` event for QML:

1. **POWER GRID** — Radial power ring fills; grid lines fade in from center
2. **NEURAL CORE** — Animated node graph wires up (camera → motion → threat)
3. **SENSOR ARRAY** — Camera handshake with decrypt-style text scramble
4. **BIOMETRIC VAULT** — Face DB integrity check with progress arc
5. **PERIMETER MESH** — Zone polygons draw onto tactical map
6. **THREAT MATRIX** — DEFCON panel initializes at DEFCON 5
7. **AISOS ONLINE** — Full dashboard panels slide in; voice: *"Cyber Sentinel X online. All systems nominal."*

---

## HUD Overlay (OpenCV + QML composite)

### Tactical grid layer
- Perspective coordinate grid with live X/Y readout under cursor
- Subtle drifting animation (0.5 px/s) for holographic feel
- Zone polygons for intrusion areas (configurable)

### Target acquisition
- **Hex reticle** — rotating outer ring + pulsing inner hex
- **Corner brackets** — spring-animated lock-on with overshoot
- **Circular lock indicator** — fills 0→100% as track confidence rises
- **Prediction vector** — dashed line to predicted position (Kalman)
- **Velocity arrow** — magnitude-scaled neon arrow from target center
- **Threat halo** — concentric pulse rings on elevated threats (Orange+)

### Telemetry strip
- Frame time, FPS, CPU, RAM, GPU, camera latency
- Recording REC blink (1 Hz square wave opacity)
- Timestamp + camera ID + DEFCON badge

### Scan effects
- Horizontal scanline sweep (configurable speed)
- Occasional glitch flicker on threat elevation (`glitch_on_threat`)
- Chromatic aberration pulse on Critical level (2-frame offset RGB)

---

## QML Animation Components (`assets/qml/animations/`)

| Component | Behavior |
|-----------|----------|
| `ScannerSweep.qml` | Rotating radar-style sweep with trailing gradient |
| `ThreatPulseRing.qml` | Expanding ring from threat origin, fade-out |
| `HexReticle.qml` | Rotating hex + tick marks, lock state color |
| `DataRain.qml` | Vertical data streams (original cyan glyphs) |
| `GlitchText.qml` | Character scramble → resolve on value change |
| `HologramPanel.qml` | Glass panel + parallax + edge glow |
| `RotatingGauge.qml` | Circular arc gauge with spring needle |
| `ParticleField.qml` | Sparse floating particles, depth layers |
| `LockOnBracket.qml` | 4-corner bracket with spring physics |
| `VelocityVector.qml` | Animated arrow along track velocity |
| `NeuralGraph.qml` | Live pipeline topology with flowing dots |
| `StatusOrbit.qml` | Engine status icons orbiting AI core |
| `DefconPanel.qml` | DEFCON 5→1 transition with color cascade |
| `BootSequence.qml` | Phase-driven boot orchestrator |
| `VoiceWaveform.qml` | SAPI speech visualizer bars |
| `TimelineScrubber.qml` | Event timeline with glitch rewind effect |
| `PerimeterMesh.qml` | Animated zone boundary wireframe |
| `DecryptText.qml` | Biometric match "decryption" reveal |
| `PowerRing.qml` | System health circular progress |
| `AlertCascade.qml` | Red alert border pulse + corner warnings |

---

## Dashboard Panels

### Mission Overview
- Live threat summary, active cameras, DEFCON, uptime
- Animated status orbs per subsystem

### Live Camera Feed
- Full-bleed video with HUD composite
- Picture-in-picture for secondary cameras

### Threat Dashboard
- Threat meter (0–100) with gradient arc animation
- Severity histogram, recent assessments list
- Friend/Foe ratio donut chart

### AI Core Panel
- Neural graph of all engines with live latency edges
- Confidence rings per engine (rotating status indicators)
- Temperature / CPU / RAM gauges

### Radar Panel
- 360° polar display, configurable sweep RPM
- Target blips with IFF colors (friend=green, foe=red, unknown=cyan)
- Prediction ghost dots, distance rings, direction ticks
- Pulse + fade on sweep pass

### Timeline
- Horizontal event stream with slide-in animations
- Threat events color-coded; click to jump recording

### Notification Center
- Floating toasts slide from top-right with glow
- Priority queue: Critical interrupts lower alerts

### Analytics
- Qt Charts: threat over time, motion heatmap, recognition stats

### Face Database
- Enrollment wizard with biometric scan sweep animation
- Grid of authorized faces with match confidence

### Recording Manager
- Storage gauge, clip list, playback preview

### Settings / Diagnostics / Developer Mode
- Live config reload indicator
- Per-stage latency histogram from `StageMetrics`

---

## Threat-Driven UI Reactions

| Threat Level | UI Response |
|--------------|-------------|
| Green | Calm cyan glow, slow sweep |
| Yellow | Amber accent pulse on threat meter |
| Orange | Panel border warning, voice alert, auto-record indicator |
| Red | Glitch overlays, faster sweep, DEFCON drop animation |
| Critical | Full `AlertCascade`, screen edge crimson pulse, voice priority |

---

## Performance Rules

- All animations use `Behavior` or `NumberAnimation` with `cached: true`
- Particle count capped by `ui.particle_density` config (0.0–1.0)
- HUD OpenCV draw budget ≤ 4 ms; QML scene graph diff only on state change
- `FrameImageProvider` delivers at display refresh, not per inference frame

---

*Phase 15 implements these QML components. HUD OpenCV layer in Phase 11.*
