# Phase 6: Recognition Module

Offline face detection, embedding, friend/foe matching, and enrollment.

## Pipeline

```
Frame + Tracks → YuNet detect → ArcFace embed → FaceDatabase match → Friend/Foe
```

Uses OpenCV DNN when models are present; CPU fallback otherwise.

## Tests

```powershell
D:\CyberSentinelX-build\tests\Release\csx_recognition_tests.exe
```

Place ONNX models in `assets/models/` for production inference.
