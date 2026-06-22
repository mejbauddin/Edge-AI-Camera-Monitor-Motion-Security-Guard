# Cyber Sentinel X - Installation Guide

## System Requirements

### Minimum
- **OS**: Windows 10 (64-bit)
- **CPU**: Intel Core i5 / AMD Ryzen 5
- **RAM**: 4 GB
- **Storage**: 2 GB free space
- **Camera**: USB webcam or IP camera

### Recommended
- **OS**: Windows 11 (64-bit)
- **CPU**: Intel Core i7 / AMD Ryzen 7
- **RAM**: 8 GB
- **GPU**: NVIDIA RTX series (for AI acceleration)
- **Storage**: 10 GB SSD
- **Camera**: HD webcam with 720p+ resolution

## Installation

### Method 1: Installer (Recommended)
1. Download `CyberSentinelX-Setup.exe`
2. Run the installer
3. Follow the on-screen instructions
4. Launch from Start Menu

### Method 2: Portable ZIP
1. Download `CyberSentinelX-Windows.zip`
2. Extract to a folder (e.g., `C:\CyberSentinelX`)
3. Run `CyberSentinelX.exe`

## First-Time Setup

### 1. Camera Configuration
Edit `config/default.json`:
```json
{
  "camera": {
    "default_source": "usb:0",
    "width": 1280,
    "height": 720,
    "fps": 60
  }
}
```

### 2. Face Database Setup
Place authorized face images in `assets/faces/authorized/`:
```
assets/faces/authorized/
├── user1.jpg
├── user2.jpg
└── user3.jpg
```

### 3. AI Models
Ensure ONNX models are in `assets/models/`:
- `yunet_2023mar.onnx` (face detection)
- `arcface_w600k_r50.onnx` (face recognition)

## Configuration

### Threat Thresholds
Adjust sensitivity in `config/default.json`:
```json
{
  "threat": {
    "levels": {
      "green_max": 20,
      "yellow_max": 40,
      "orange_max": 60,
      "red_max": 80
    }
  }
}
```

### HUD Customization
Modify appearance:
```json
{
  "hud": {
    "neon_primary": "#00E5FF",
    "show_velocity_vectors": true,
    "show_prediction_trails": true
  }
}
```

### Recording Settings
Configure video recording:
```json
{
  "recording": {
    "pre_buffer_seconds": 5,
    "post_buffer_seconds": 10,
    "max_storage_gb": 50
  }
}
```

## Verification

After installation, verify:
1. Camera feed displays correctly
2. Motion detection responds to movement
3. HUD overlay renders properly
4. Recording creates MP4 files on threat
5. Voice alerts play (if enabled)

## Uninstallation

### Installer Version
Use "Add or Remove Programs" in Windows Settings

### Portable Version
Delete the extracted folder and all contents

## Troubleshooting

### Camera Not Detected
- Check camera is connected and not in use by other apps
- Try different camera index in config (usb:0, usb:1, etc.)
- Verify camera drivers are up to date

### Poor Performance
- Reduce resolution in config (e.g., 640x480)
- Disable face recognition if not needed
- Close other applications
- Ensure GPU drivers are current

### AI Models Not Loading
- Verify ONNX files are in `assets/models/`
- Check file permissions
- Ensure models are compatible with your ONNX Runtime version

### Voice Not Working
- Check Windows audio settings
- Verify SAPI is installed (default on Windows)
- Check voice is enabled in config

## Support

For issues or questions:
- Check documentation in `docs/` folder
- Review logs in `logs/` directory
- Enable developer mode for detailed diagnostics
