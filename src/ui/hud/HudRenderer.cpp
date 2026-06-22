#include "HudRenderer.hpp"

#include "CvHelpers.hpp"
#include "types/Enums.hpp"
#include "utils/CvHelpers.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <sstream>

#if defined(CSX_HAS_OPENCV)
#include <opencv2/imgproc.hpp>
#endif

namespace csx::hud {

// ══════════════════════════════════════════════════════════════════════════════
// Helpers — not dependent on OpenCV
// ══════════════════════════════════════════════════════════════════════════════

namespace {

std::string currentTimestamp() {
    const auto now    = std::chrono::system_clock::now();
    const auto timeT  = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf{};
#if defined(_MSC_VER)
    localtime_s(&tmBuf, &timeT);
#else
    localtime_r(&timeT, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%H:%M:%S");
    return oss.str();
}

std::string defconString(core::DefconLevel defcon) {
    switch (defcon) {
        case core::DefconLevel::Defcon1: return "DEFCON 1";
        case core::DefconLevel::Defcon2: return "DEFCON 2";
        case core::DefconLevel::Defcon3: return "DEFCON 3";
        case core::DefconLevel::Defcon4: return "DEFCON 4";
        case core::DefconLevel::Defcon5: return "DEFCON 5";
    }
    return "DEFCON 5";
}

} // namespace

// ══════════════════════════════════════════════════════════════════════════════
// Constructor / interface helpers
// ══════════════════════════════════════════════════════════════════════════════

HudRenderer::HudRenderer(HudSettings settings) : settings_(std::move(settings)) {
    engineHealth_.status = core::EngineStatus::Online;
}

void HudRenderer::setRecording(bool recording) noexcept {
    recording_.store(recording, std::memory_order_relaxed);
}

core::EngineHealth HudRenderer::health() const {
    return engineHealth_;
}

HudRenderer::Rgb HudRenderer::reticleColor(core::IdentityClassification cls) const noexcept {
    switch (cls) {
        case core::IdentityClassification::Authorized:
            return {settings_.colorFriendB, settings_.colorFriendG, settings_.colorFriendR};
        case core::IdentityClassification::Foe:
            return {settings_.colorFoeB, settings_.colorFoeG, settings_.colorFoeR};
        default:
            return {settings_.colorUnknownB, settings_.colorUnknownG, settings_.colorUnknownR};
    }
}

HudRenderer::Rgb HudRenderer::threatColor(core::ThreatLevel level) const noexcept {
    switch (level) {
        case core::ThreatLevel::Green:
            return {settings_.colorFriendB, settings_.colorFriendG, settings_.colorFriendR};
        case core::ThreatLevel::Yellow:
            return {0, 230, 255}; // yellow
        case core::ThreatLevel::Orange:
            return {settings_.colorAmberB, settings_.colorAmberG, settings_.colorAmberR};
        case core::ThreatLevel::Red:
        case core::ThreatLevel::Critical:
            return {settings_.colorFoeB, settings_.colorFoeG, settings_.colorFoeR};
    }
    return {settings_.colorUnknownB, settings_.colorUnknownG, settings_.colorUnknownR};
}

// ══════════════════════════════════════════════════════════════════════════════
// render() — main entry point
// ══════════════════════════════════════════════════════════════════════════════

void HudRenderer::render(const core::Frame& inputFrame,
                         const std::vector<core::Track>& tracks,
                         const std::vector<core::FaceMatch>& faces,
                         const std::vector<core::BehaviorAnomaly>& anomalies,
                         const core::ThreatAssessment& assessment,
                         const core::SystemHealth& health,
                         core::Frame& outFrame)
{
    (void)anomalies; // used for threat meter data, not drawn per-anomaly in HUD

    ++frameCounter_;

    engineHealth_.status = core::EngineStatus::Online;
    engineHealth_.processedFrames = frameCounter_;

#if defined(CSX_HAS_OPENCV)
    // ── 1. Deep-copy the raw input frame into an OpenCV Mat ────────────────
    if (!inputFrame.valid()) {
        engineHealth_.status = core::EngineStatus::Degraded;
        engineHealth_.detail = "Invalid input frame";
        return;
    }

    std::shared_ptr<std::vector<std::uint8_t>> outBuf;
    {
        std::lock_guard<std::mutex> lock(csx::utils::openCvMutex());

        const int w = static_cast<int>(inputFrame.width);
        const int h = static_cast<int>(inputFrame.height);

        outBuf = std::make_shared<std::vector<std::uint8_t>>(
            inputFrame.bgrData->cbegin(), inputFrame.bgrData->cend());
        cv::Mat mat(h, w, CV_8UC3, outBuf->data());

        // ── 2. Draw layers bottom→top ──────────────────────────────────────────
        if (settings_.showGrid) {
            drawTacticalGrid(&mat);
        }

        // Target reticles (one per track, looking up matching face if any)
        if (settings_.showReticles) {
            for (const auto& track : tracks) {
                if (!track.active) continue;
                // Find matching face (if any)
                const core::FaceMatch* face = nullptr;
                for (const auto& fm : faces) {
                    if (fm.trackId == track.id) {
                        face = &fm;
                        break;
                    }
                }
                drawReticle(&mat, track, face, static_cast<int>(frameCounter_));
                if (settings_.showVelocityVectors) {
                    drawVelocityVector(&mat, track);
                }
                if (settings_.showPredictionVectors) {
                    drawPredictionVector(&mat, track);
                }
            }

            // Face detections without an active track still get a lock reticle
            for (const auto& fm : faces) {
                bool hasTrack = false;
                for (const auto& track : tracks) {
                    if (track.active && track.id == fm.trackId) {
                        hasTrack = true;
                        break;
                    }
                }
                if (hasTrack) {
                    continue;
                }
                core::Track faceTrack;
                faceTrack.id = fm.trackId;
                faceTrack.bbox = fm.bbox;
                faceTrack.center = fm.bbox.center();
                faceTrack.confidence = fm.confidence;
                faceTrack.active = true;
                drawReticle(&mat, faceTrack, &fm, static_cast<int>(frameCounter_));
            }
        }

        if (settings_.showScanline) {
            drawScanline(&mat, static_cast<int>(frameCounter_));
        }

        if (settings_.showThreatMeter) {
            drawThreatMeter(&mat, assessment);
        }

        if (settings_.showDefcon) {
            drawDefconPanel(&mat, assessment);
        }

        if (settings_.showTelemetry) {
            drawTelemetryStrip(&mat, health, assessment, static_cast<int>(frameCounter_));
        }

        // Critical-level edge pulse effect
        if (settings_.enableGlitchOnThreat &&
            assessment.level >= core::ThreatLevel::Red) {
            drawCriticalEdgePulse(&mat, static_cast<int>(frameCounter_));
        }
    }

    // ── 3. Populate outFrame ───────────────────────────────────────────────
    outFrame.sequence   = inputFrame.sequence;
    outFrame.cameraId   = inputFrame.cameraId;
    outFrame.captureTime= inputFrame.captureTime;
    outFrame.width      = inputFrame.width;
    outFrame.height     = inputFrame.height;
    outFrame.diagnostics= inputFrame.diagnostics;
    outFrame.bgrData    = std::move(outBuf);

#else
    // ── CPU fallback: pass through without overlay ─────────────────────────
    auto outBuf = std::make_shared<std::vector<std::uint8_t>>(
        inputFrame.bgrData->cbegin(), inputFrame.bgrData->cend());
    outFrame = inputFrame;
    outFrame.bgrData = std::move(outBuf);
    engineHealth_.detail = "OpenCV unavailable - pass-through mode";
#endif
}

// ══════════════════════════════════════════════════════════════════════════════
// Drawing helpers — only compiled when OpenCV is present
// ══════════════════════════════════════════════════════════════════════════════

#if defined(CSX_HAS_OPENCV)

// ── Tactical coordinate grid ──────────────────────────────────────────────────
void HudRenderer::drawTacticalGrid(void* matPtr) const {
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const int w = mat.cols;
    const int h = mat.rows;
    const cv::Scalar gridCol(settings_.colorGridB, settings_.colorGridG, settings_.colorGridR);

    // Vertical grid lines every 10% width
    const int colStep = w / 10;
    for (int x = colStep; x < w; x += colStep) {
        cv::line(mat, {x, 0}, {x, h}, gridCol, 1, cv::LINE_AA);
    }

    // Horizontal grid lines every 10% height
    const int rowStep = h / 10;
    for (int y = rowStep; y < h; y += rowStep) {
        cv::line(mat, {0, y}, {w, y}, gridCol, 1, cv::LINE_AA);
    }

    // Center crosshairs — slightly brighter
    const cv::Scalar crossCol(settings_.colorUnknownB, settings_.colorUnknownG,
                               settings_.colorUnknownR);
    const int cx = w / 2;
    const int cy = h / 2;
    const int halfGap = 12; // leave gap at centre

    cv::line(mat, {cx, 0},         {cx, cy - halfGap}, crossCol, 1, cv::LINE_AA);
    cv::line(mat, {cx, cy + halfGap}, {cx, h},          crossCol, 1, cv::LINE_AA);
    cv::line(mat, {0, cy},          {cx - halfGap, cy}, crossCol, 1, cv::LINE_AA);
    cv::line(mat, {cx + halfGap, cy}, {w, cy},           crossCol, 1, cv::LINE_AA);

    // Corner micro-tick marks
    const int tickLen = 8;
    const cv::Scalar tickCol(settings_.colorTextB, settings_.colorTextG, settings_.colorTextR);
    // top-left
    cv::line(mat, {0, 0}, {tickLen, 0},    tickCol, 1);
    cv::line(mat, {0, 0}, {0, tickLen},    tickCol, 1);
    // top-right
    cv::line(mat, {w-1, 0}, {w-1-tickLen, 0}, tickCol, 1);
    cv::line(mat, {w-1, 0}, {w-1, tickLen},   tickCol, 1);
    // bottom-left
    cv::line(mat, {0, h-1}, {tickLen, h-1},   tickCol, 1);
    cv::line(mat, {0, h-1}, {0, h-1-tickLen}, tickCol, 1);
    // bottom-right
    cv::line(mat, {w-1, h-1}, {w-1-tickLen, h-1},   tickCol, 1);
    cv::line(mat, {w-1, h-1}, {w-1, h-1-tickLen},   tickCol, 1);
}

// ── Animated horizontal scanline ───────────────────────────────────────────
void HudRenderer::drawScanline(void* matPtr, int frameIdx) const {
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const int h = mat.rows;
    const int w = mat.cols;

    // Phase: scanline cycles top→bottom every 120 frames
    const int period = 120;
    const int y = (static_cast<int>(frameIdx * settings_.scanlineSpeed) % h + h) % h;

    // Draw semi-transparent bright strip (3 px wide)
    const cv::Scalar scanCol(settings_.colorUnknownB, settings_.colorUnknownG,
                              settings_.colorUnknownR);
    const int halfW = static_cast<int>(settings_.scanlineAlpha * 255.0F);

    // Overlay via ROI blend
    const int y1 = std::max(0, y - 1);
    const int y2 = std::min(h - 1, y + 1);
    if (y2 > y1) {
        cv::Mat roi = mat(cv::Rect(0, y1, w, y2 - y1 + 1));
        cv::Mat overlay = roi.clone();
        overlay.setTo(cv::Scalar(settings_.colorUnknownB, settings_.colorUnknownG,
                                  settings_.colorUnknownR));
        cv::addWeighted(roi, 1.0 - settings_.scanlineAlpha,
                        overlay, settings_.scanlineAlpha, 0.0, roi);
    }
    (void)period; (void)halfW; // suppress unused warnings
}

// ── Target reticle (corner brackets + id label) ───────────────────────────
void HudRenderer::drawReticle(void* matPtr,
                               const core::Track& track,
                               const core::FaceMatch* face,
                               const int frameIdx) const
{
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const int w = mat.cols;
    const int h = mat.rows;

    // Clamp bounding box to frame
    const int x1 = std::max(0, static_cast<int>(track.bbox.x));
    const int y1 = std::max(0, static_cast<int>(track.bbox.y));
    const int x2 = std::min(w - 1, static_cast<int>(track.bbox.x + track.bbox.width));
    const int y2 = std::min(h - 1, static_cast<int>(track.bbox.y + track.bbox.height));
    if (x2 <= x1 || y2 <= y1) return;

    // Pick color
    auto cls = core::IdentityClassification::Unknown;
    if (face) cls = face->classification;
    const auto col = reticleColor(cls);
    const cv::Scalar cvCol(col.b, col.g, col.r);
    const int thick = static_cast<int>(settings_.bracketThickness);
    const int arm   = static_cast<int>(settings_.bracketLength);

    // ── Corner brackets ──
    // TL
    cv::line(mat, {x1, y1}, {x1 + arm, y1}, cvCol, thick, cv::LINE_AA);
    cv::line(mat, {x1, y1}, {x1, y1 + arm}, cvCol, thick, cv::LINE_AA);
    // TR
    cv::line(mat, {x2, y1}, {x2 - arm, y1}, cvCol, thick, cv::LINE_AA);
    cv::line(mat, {x2, y1}, {x2, y1 + arm}, cvCol, thick, cv::LINE_AA);
    // BL
    cv::line(mat, {x1, y2}, {x1 + arm, y2}, cvCol, thick, cv::LINE_AA);
    cv::line(mat, {x1, y2}, {x1, y2 - arm}, cvCol, thick, cv::LINE_AA);
    // BR
    cv::line(mat, {x2, y2}, {x2 - arm, y2}, cvCol, thick, cv::LINE_AA);
    cv::line(mat, {x2, y2}, {x2, y2 - arm}, cvCol, thick, cv::LINE_AA);

    // ── Rotating hex lock ring (sci-fi targeting) ──
    const int cx = (x1 + x2) / 2;
    const int cy = (y1 + y2) / 2;
    const int radius = std::max(12, std::min((x2 - x1), (y2 - y1)) / 2 + 8);
    const double angleOffset = (frameIdx % 360) * 0.05;
    std::vector<cv::Point> hex;
    hex.reserve(6);
    for (int i = 0; i < 6; ++i) {
        const double a = angleOffset + i * (3.14159265 / 3.0);
        hex.emplace_back(cx + static_cast<int>(radius * std::cos(a)),
                         cy + static_cast<int>(radius * std::sin(a)));
    }
    for (int i = 0; i < 6; ++i) {
        cv::line(mat, hex[i], hex[(i + 1) % 6], cvCol, 1, cv::LINE_AA);
    }

    // ── Confidence arc (small arc drawn in top-right corner) ──
    const double angle = track.confidence * 360.0;
    cv::ellipse(mat, {x2 - 10, y1 + 10}, {8, 8}, -90, 0, angle, cvCol, 1, cv::LINE_AA);

    // ── Label ──
    std::string label = "TGT-" + std::to_string(track.id);
    if (face) {
        if (cls == core::IdentityClassification::Authorized && !face->identityName.empty()) {
            label = "AUTH:" + face->identityName;
            if (!face->country.empty()) {
                label += " | " + face->country;
            }
        } else if (cls == core::IdentityClassification::Foe) {
            label = "UNAUTHORIZED FOE";
        } else {
            label = "BIO-SIGNATURE LOCK";
        }
    }

    const double fs   = settings_.textFontScale;
    const int    font = cv::FONT_HERSHEY_SIMPLEX;
    int baseline = 0;
    const auto sz = cv::getTextSize(label, font, fs, 1, &baseline);
    const int lx = x1;
    const int ly = std::max(sz.height + 2, y1 - 4);

    // Background chip
    cv::rectangle(mat,
                  {lx - 1, ly - sz.height - 2},
                  {lx + sz.width + 1, ly + 2},
                  cv::Scalar(10, 10, 10), cv::FILLED);

    cv::putText(mat, label, {lx, ly}, font, fs, cvCol, settings_.textThickness, cv::LINE_AA);
}

// ── Velocity arrow ──────────────────────────────────────────────────────────
void HudRenderer::drawVelocityVector(void* matPtr, const core::Track& track) const {
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const float vx = track.velocity.x * settings_.vectorScale;
    const float vy = track.velocity.y * settings_.vectorScale;
    if (std::abs(vx) < 0.5F && std::abs(vy) < 0.5F) return;

    const auto col = reticleColor(core::IdentityClassification::Unknown);
    const cv::Scalar cvCol(col.b, col.g, col.r);

    const cv::Point start(static_cast<int>(track.center.x),
                          static_cast<int>(track.center.y));
    const cv::Point end(static_cast<int>(track.center.x + vx),
                        static_cast<int>(track.center.y + vy));

    cv::arrowedLine(mat, start, end, cvCol, 1, cv::LINE_AA, 0, 0.25);
}

// ── Kalman prediction vector ───────────────────────────────────────────────
void HudRenderer::drawPredictionVector(void* matPtr, const core::Track& track) const {
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const cv::Point from(static_cast<int>(track.center.x),
                         static_cast<int>(track.center.y));
    const cv::Point to(static_cast<int>(track.predictedPosition.x),
                       static_cast<int>(track.predictedPosition.y));
    if (from == to) return;

    // Dashed line simulation via multiple short segments
    const cv::Scalar dashCol(100, 200, 255); // light cyan
    const int totalDx = to.x - from.x;
    const int totalDy = to.y - from.y;
    const int steps = 8;
    for (int i = 0; i < steps; i += 2) {
        const cv::Point p1(from.x + totalDx * i / steps,
                           from.y + totalDy * i / steps);
        const cv::Point p2(from.x + totalDx * (i + 1) / steps,
                           from.y + totalDy * (i + 1) / steps);
        cv::line(mat, p1, p2, dashCol, 1, cv::LINE_AA);
    }
}

// ── Telemetry strip ─────────────────────────────────────────────────────────
void HudRenderer::drawTelemetryStrip(void* matPtr,
                                      const core::SystemHealth& sysHealth,
                                      const core::ThreatAssessment& assessment,
                                      int frameIdx) const
{
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const int w = mat.cols;
    const int h = mat.rows;
    const cv::Scalar textCol(settings_.colorTextB, settings_.colorTextG, settings_.colorTextR);
    const int font  = cv::FONT_HERSHEY_SIMPLEX;
    const double fs = settings_.textFontScale * 0.85;
    const int    th = 1;

    // Dark backing strip — top 22 px
    cv::Mat strip = mat(cv::Rect(0, 0, w, 22));
    cv::Mat backing = strip.clone();
    backing.setTo(cv::Scalar(8, 8, 8));
    cv::addWeighted(strip, 0.35, backing, 0.65, 0, strip);

    // Separator line below strip
    cv::line(mat, {0, 22}, {w, 22},
             cv::Scalar(settings_.colorUnknownB, settings_.colorUnknownG,
                         settings_.colorUnknownR), 1);

    // Build telemetry string — left side
    std::ostringstream oss;
    oss << "AISOS v1.0  |  "
        << currentTimestamp()
        << "  |  FPS:" << std::fixed << std::setprecision(1) << sysHealth.fps
        << "  CPU:" << static_cast<int>(sysHealth.cpuPercent) << "%"
        << "  RAM:" << static_cast<int>(sysHealth.ramPercent) << "%"
        << "  FT:" << std::setprecision(1) << sysHealth.frameTimeMs << "ms";

    cv::putText(mat, oss.str(), {4, 15}, font, fs, textCol, th, cv::LINE_AA);

    // REC blink — top-right
    if (recording_.load(std::memory_order_relaxed)) {
        const bool blink = ((frameIdx / 15) % 2 == 0); // blink at ~2 Hz @30fps
        if (blink) {
            const std::string rec = "● REC";
            int baseline = 0;
            const auto sz = cv::getTextSize(rec, font, fs, th, &baseline);
            cv::putText(mat, rec, {w - sz.width - 6, 15}, font, fs,
                        cv::Scalar(60, 30, 255), th, cv::LINE_AA);
        }
    }

    // Threat label — bottom-left corner
    const auto tcol = threatColor(assessment.level);
    const std::string thrStr = "THREAT: " + core::toString(assessment.level)
                             + " [" + std::to_string(static_cast<int>(assessment.threatScore)) + "]";
    cv::putText(mat, thrStr, {4, h - 6}, font, fs,
                cv::Scalar(tcol.b, tcol.g, tcol.r), th, cv::LINE_AA);

    // Camera name — bottom right
    const std::string camStr = "CAM: SENTINEL-A1";
    int baseline = 0;
    const auto sz = cv::getTextSize(camStr, font, fs, th, &baseline);
    cv::putText(mat, camStr, {w - sz.width - 4, h - 6}, font, fs, textCol, th, cv::LINE_AA);
}

// ── Threat meter (arc gauge — right edge) ────────────────────────────────────
void HudRenderer::drawThreatMeter(void* matPtr,
                                   const core::ThreatAssessment& assessment) const
{
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const int w = mat.cols;
    const int cx = w - 30;
    const int cy = mat.rows / 2;
    const int radius = 22;

    // Background ring
    cv::ellipse(mat, {cx, cy}, {radius, radius}, -90, 0, 360,
                cv::Scalar(30, 30, 30), 3, cv::LINE_AA);

    // Filled arc proportional to threat score (0-100)
    const double arcAngle = (assessment.threatScore / 100.0) * 360.0;
    const auto tc = threatColor(assessment.level);
    cv::ellipse(mat, {cx, cy}, {radius, radius}, -90, 0, arcAngle,
                cv::Scalar(tc.b, tc.g, tc.r), 3, cv::LINE_AA);

    // Centre score text
    const std::string sc = std::to_string(static_cast<int>(assessment.threatScore));
    int baseline = 0;
    const auto sz = cv::getTextSize(sc, cv::FONT_HERSHEY_SIMPLEX, 0.35, 1, &baseline);
    cv::putText(mat, sc,
                {cx - sz.width / 2, cy + sz.height / 2},
                cv::FONT_HERSHEY_SIMPLEX, 0.35,
                cv::Scalar(tc.b, tc.g, tc.r), 1, cv::LINE_AA);
}

// ── DEFCON panel ───────────────────────────────────────────────────────────
void HudRenderer::drawDefconPanel(void* matPtr,
                                   const core::ThreatAssessment& assessment) const
{
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const int w = mat.cols;
    const int h = mat.rows;

    const std::string label = defconString(assessment.defcon);
    const auto tc = threatColor(assessment.level);
    const cv::Scalar col(tc.b, tc.g, tc.r);
    const int font = cv::FONT_HERSHEY_SIMPLEX;
    const double fs = settings_.textFontScale * 0.9;

    int baseline = 0;
    const auto sz = cv::getTextSize(label, font, fs, 1, &baseline);
    const int x = w - sz.width - 6;
    const int y = h - 22;

    // Dark chip
    cv::rectangle(mat, {x - 2, y - sz.height - 2}, {w - 2, y + 2},
                  cv::Scalar(10, 10, 10), cv::FILLED);
    cv::rectangle(mat, {x - 2, y - sz.height - 2}, {w - 2, y + 2},
                  col, 1);

    cv::putText(mat, label, {x, y}, font, fs, col, 1, cv::LINE_AA);
}

// ── Critical edge pulse ────────────────────────────────────────────────────
void HudRenderer::drawCriticalEdgePulse(void* matPtr, int frameIdx) const {
    auto& mat = *static_cast<cv::Mat*>(matPtr);
    const int w = mat.cols;
    const int h = mat.rows;

    // Oscillate alpha between 0.1 and 0.5
    const float phase  = std::abs(std::sin(frameIdx * 0.15F));
    const float alpha  = 0.1F + phase * 0.4F;
    const cv::Scalar redEdge(60, 30, 255);

    auto blendEdge = [&](cv::Rect roi) {
        cv::Mat region = mat(roi);
        cv::Mat over   = region.clone();
        over.setTo(redEdge);
        cv::addWeighted(region, 1.0F - alpha, over, alpha, 0, region);
    };

    const int thickness = 8;
    blendEdge({0,       0,      w,         thickness}); // top
    blendEdge({0,       h - thickness, w,  thickness}); // bottom
    blendEdge({0,       0,      thickness, h});          // left
    blendEdge({w - thickness, 0, thickness, h});         // right
}

#else
// ── Stubs for non-OpenCV build ─────────────────────────────────────────────
void HudRenderer::drawTacticalGrid(void*)        const {}
void HudRenderer::drawScanline(void*, int)       const {}
void HudRenderer::drawReticle(void*, const core::Track&, const core::FaceMatch*, int) const {}
void HudRenderer::drawVelocityVector(void*, const core::Track&)     const {}
void HudRenderer::drawPredictionVector(void*, const core::Track&)   const {}
void HudRenderer::drawTelemetryStrip(void*, const core::SystemHealth&,
                                     const core::ThreatAssessment&, int) const {}
void HudRenderer::drawThreatMeter(void*, const core::ThreatAssessment&) const {}
void HudRenderer::drawDefconPanel(void*, const core::ThreatAssessment&) const {}
void HudRenderer::drawCriticalEdgePulse(void*, int)                const {}
#endif

}  // namespace csx::hud
