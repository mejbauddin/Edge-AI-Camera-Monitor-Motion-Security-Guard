# Download YuNet + SFace ONNX models (OpenCV Zoo) for face detection & recognition
$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$ModelsDir = Join-Path (Split-Path -Parent $ProjectRoot) "assets\models"
New-Item -ItemType Directory -Force -Path $ModelsDir | Out-Null

$models = @{
    "yunet_2023mar.onnx" = "https://github.com/opencv/opencv_zoo/raw/main/models/face_detection_yunet/face_detection_yunet_2023mar.onnx"
    "sface_2021dec.onnx" = "https://github.com/opencv/opencv_zoo/raw/main/models/face_recognition_sface/face_recognition_sface_2021dec.onnx"
}

foreach ($entry in $models.GetEnumerator()) {
    $dest = Join-Path $ModelsDir $entry.Key
    if (Test-Path $dest) {
        $size = (Get-Item $dest).Length
        if ($size -gt 100000) {
            Write-Host "OK  $($entry.Key) ($([math]::Round($size/1KB)) KB)" -ForegroundColor Green
            continue
        }
    }
    Write-Host "Downloading $($entry.Key) ..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri $entry.Value -OutFile $dest -UseBasicParsing
    Write-Host "Saved $dest" -ForegroundColor Green
}

Write-Host "`nModels ready in $ModelsDir" -ForegroundColor Green
