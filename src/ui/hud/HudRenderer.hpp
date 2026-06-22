#pragma once

#include "interfaces/Interfaces.hpp"
#include "types/Frame.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace csx::hud {

// ──────────────────────────────────────────────────────────────────────────────
// HudSettings — mirrors config/default.json hud.* section
// ──────────────────────────────────────────────────────────────────────────────
struct HudSettings {
    // Palette (BGR order)
    std::uint8_t colorFriendB{0};   std::uint8_t colorFriendG{230}; std::uint8_t colorFriendR{118};   // #00E676 green
    std::uint8_t colorFoeB{71};     std::uint8_t colorFoeG{23};     std::uint8_t colorFoeR{255};       // #FF1744 crimson
    std::uint8_t colorUnknownB{255};std::uint8_t colorUnknownG{229};std::uint8_t colorUnknownR{0};     // #00E5FF cyan
    std::uint8_t colorGridB{180};   std::uint8_t colorGridG{100};   std::uint8_t colorGridR{30};       // dim cyan grid
    std::uint8_t colorTextB{240};   std::uint8_t colorTextG{247};   std::uint8_t colorTextR{224};      // #E0F7FA ice white
    std::uint8_t colorAmberB{0};    std::uint8_t colorAmberG{109};  std::uint8_t colorAmberR{255};     // #FF6D00 amber
    std::uint8_t colorCritB{71};    std::uint8_t colorCritG{23};    std::uint8_t colorCritR{255};      // crimson critical

    // Feature toggles
    bool showGrid{true};
    bool showScanline{true};
    bool showTelemetry{true};
    bool showReticles{true};
    bool showVelocityVectors{true};
    bool showPredictionVectors{true};
    bool showThreatMeter{true};
    bool showDefcon{true};
    bool enableGlitchOnThreat{true};

    // Sizing
    float bracketLength{20.0F};     // corner bracket arm length in pixels
    float bracketThickness{2.0F};   // bracket line thickness
    float gridAlpha{0.18F};         // scanline grid transparency (0-1)
    float scanlineAlpha{0.35F};     // scanline strip transparency
    float scanlineSpeed{2.0F};      // pixels per frame the scanline moves
    float vectorScale{2.5F};        // multiplier for velocity arrow length
    int   textFontFace{0};          // cv::FONT_HERSHEY_SIMPLEX = 0
    double textFontScale{0.45};
    int   textThickness{1};
};

// ──────────────────────────────────────────────────────────────────────────────
// HudRenderer
// ──────────────────────────────────────────────────────────────────────────────
class HudRenderer final : public core::IHudRenderer {
public:
    explicit HudRenderer(HudSettings settings = {});
    ~HudRenderer() override = default;

    // IHudRenderer
    void render(const core::Frame& inputFrame,
                const std::vector<core::Track>& tracks,
                const std::vector<core::FaceMatch>& faces,
                const std::vector<core::BehaviorAnomaly>& anomalies,
                const core::ThreatAssessment& assessment,
                const core::SystemHealth& health,
                core::Frame& outFrame) override;

    void setRecording(bool recording) noexcept override;
    [[nodiscard]] core::EngineHealth health() const override;

private:
    // ── drawing helpers (implemented with OpenCV when CSX_HAS_OPENCV defined) ──
    void drawTacticalGrid(void* mat) const;
    void drawScanline(void* mat, int frameIdx) const;
    void drawReticle(void* mat,
                     const core::Track& track,
                     const core::FaceMatch* face,
                     int frameIdx) const;
    void drawVelocityVector(void* mat, const core::Track& track) const;
    void drawPredictionVector(void* mat, const core::Track& track) const;
    void drawTelemetryStrip(void* mat,
                            const core::SystemHealth& health,
                            const core::ThreatAssessment& assessment,
                            int frameIdx) const;
    void drawThreatMeter(void* mat, const core::ThreatAssessment& assessment) const;
    void drawDefconPanel(void* mat, const core::ThreatAssessment& assessment) const;
    void drawCriticalEdgePulse(void* mat, int frameIdx) const;

    // helper: pick bracket BGR by classification
    struct Rgb { std::uint8_t b, g, r; };
    [[nodiscard]] Rgb reticleColor(core::IdentityClassification cls) const noexcept;
    [[nodiscard]] Rgb threatColor(core::ThreatLevel level) const noexcept;

    HudSettings settings_;
    std::atomic<bool> recording_{false};
    std::uint64_t frameCounter_{0};
    mutable core::EngineHealth engineHealth_;
};

}  // namespace csx::hud
