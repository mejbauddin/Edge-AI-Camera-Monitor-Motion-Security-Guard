#pragma once

#include "ThreatSettings.hpp"
#include "types/Frame.hpp"

#include <deque>
#include <optional>

namespace csx::threat {

class ThreatHistory {
public:
    explicit ThreatHistory(ThreatSettings settings = {});

    [[nodiscard]] float smoothScore(float rawScore);
    [[nodiscard]] core::ThreatLevel peakLevel() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    void record(const core::ThreatAssessment& assessment);
    void clear();
    void setSettings(const ThreatSettings& settings);

private:
    ThreatSettings settings_;
    std::deque<core::ThreatAssessment> history_;
    float smoothedScore_{0.0F};
    core::ThreatLevel peakLevel_{core::ThreatLevel::Green};
    bool initialized_{false};
};

}  // namespace csx::threat
