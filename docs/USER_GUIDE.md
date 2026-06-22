# Cyber Sentinel X - User Guide

## Overview

Cyber Sentinel X is an AI-powered security operating system that transforms webcams or IP cameras into intelligent monitoring systems with real-time threat detection, facial recognition, and a futuristic command-center interface.

## Quick Start

1. **Launch the Application**
   - Double-click `CyberSentinelX.exe`
   - Wait for the 7-phase boot sequence to complete

2. **Camera Feed**
   - Main panel shows live video with HUD overlay
   - Green brackets = authorized persons
   - Red brackets = unauthorized/unknown
   - Cyan brackets = unknown but not classified

3. **Radar Panel**
   - Bottom-right shows polar radar view
   - Green blips = friends
   - Red blips = foes
   - Cyan blips = unknown
   - Sweep line rotates at configured RPM

4. **Threat Dashboard**
   - Top-right shows current threat level
   - Color-coded: Green → Yellow → Orange → Red → Critical
   - DEFCON level auto-adjusts based on threat

## Dashboard Panels

### Mission Overview
- System status summary
- Active camera count
- Current DEFCON level
- System uptime

### Live Camera Feed
- Main video display with HUD overlay
- Picture-in-picture for secondary cameras
- Target reticles with classification
- Velocity and prediction vectors

### Threat Dashboard
- Real-time threat meter (0-100)
- Severity histogram
- Recent threat assessments
- Friend/Foe ratio chart

### AI Core Panel
- Neural pipeline topology
- Per-engine confidence rings
- CPU/RAM/GPU gauges
- Temperature monitoring

### Radar Panel
- 360° polar display
- Configurable sweep speed
- Target blips with IFF colors
- Distance rings and direction ticks

### Timeline
- Horizontal event stream
- Threat events color-coded
- Click to jump to recording

### Notification Center
- Floating toast notifications
- Priority queue (Critical interrupts)
- Alert history

### Face Database
- Enrollment wizard
- Authorized face grid
- Match confidence display

### Recording Manager
- Storage usage gauge
- Clip list with playback
- Automatic cleanup controls

## Threat Levels

| Level | DEFCON | Description | Response |
|-------|--------|-------------|----------|
| Green | 5 | Normal operation | Monitoring only |
| Yellow | 4 | Elevated activity | Voice alert, increased logging |
| Orange | 3 | Potential threat | Auto-recording, voice alert |
| Red | 2 | High threat | Full recording, priority alerts |
| Critical | 1 | Immediate danger | Maximum alerts, continuous recording |

## Face Recognition

### Enrollment
1. Click "Face Database" panel
2. Click "Enroll New Face"
3. Position face in camera frame
3. Click "Capture"
4. Enter name and save

### Classification
- **Authorized**: Green brackets, name displayed
- **Foe**: Red brackets, "UNAUTHORIZED" alert
- **Unknown**: Cyan brackets, no name

## Recording

### Automatic Recording
- Triggers at Orange+ threat level
- Pre-buffer: 5 seconds before event
- Post-buffer: 10 seconds after event
- H.264/MP4 format

### Manual Recording
- Click "REC" button in telemetry strip
- Click again to stop
- Clips saved to `recordings/` folder

### Playback
- Open Recording Manager panel
- Select clip from list
- Click "Play"

## Voice Alerts

Voice announcements for:
- System startup: "Cyber Sentinel X online"
- Authorized user: "Authorized user detected"
- Unknown person: "Unauthorized individual detected"
- Threat elevation: "Threat level elevated"
- Critical: "Critical threat - immediate attention required"

Configure in `config/default.json`:
```json
{
  "voice": {
    "enabled": true,
    "volume": 90,
    "cooldown_ms": 3000
  }
}
```

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Space` | Pause/Resume monitoring |
| `R` | Toggle manual recording |
| `M` | Mute voice alerts |
| `H` | Toggle HUD overlay |
| `F` | Toggle fullscreen |
| `Esc` | Exit fullscreen |
| `Ctrl+S` | Save snapshot |
| `Ctrl+Q` | Quit application |

## Tips

### Optimize Performance
- Use 720p resolution for better FPS
- Disable face recognition if not needed
- Reduce particle density in UI settings
- Close other applications

### Improve Accuracy
- Ensure good lighting
- Position camera at eye level
- Enroll faces from multiple angles
- Keep face database up to date

### Reduce False Alarms
- Adjust threat thresholds in config
- Enable baseline learning for behavior
- Set appropriate intrusion zones
- Tune motion sensitivity

## Advanced Features

### Behavior Analysis
The system learns normal room behavior and detects:
- Intrusion in restricted zones
- Loitering (lingering in one area)
- Running/sudden movement
- Camera tampering
- Unusual activity patterns

### Multi-Camera Support
Add additional cameras in config:
```json
{
  "cameras": [
    {"id": "cam1", "source": "usb:0"},
    {"id": "cam2", "source": "rtsp://192.168.1.100:554/stream"}
  ]
}
```

### Remote Monitoring
Access via network by configuring RTSP streams
- Ensure firewall allows traffic
- Use secure connections
- Monitor bandwidth usage

## Troubleshooting

### No Camera Feed
- Check camera connection
- Verify camera not in use by other apps
- Try different camera index

### Poor FPS
- Reduce resolution
- Disable features not needed
- Check system resources

### False Positives
- Adjust threat thresholds
- Improve lighting
- Recalibrate motion sensitivity

### Voice Not Working
- Check audio settings
- Verify SAPI installed
- Check voice enabled in config

## Safety & Privacy

- All processing is offline (no cloud)
- Face database stored locally
- Recordings stored on local disk
- No data transmitted externally
- Secure by design - air-gapped operation

## License

Proprietary software. All rights reserved.
