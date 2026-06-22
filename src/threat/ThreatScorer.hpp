#pragma once

#include "ThreatSettings.hpp"
#include "types/Frame.hpp"

#include <vector>

namespace csx::threat {

struct SignalScores {
    float motion{0.0F};
    float track{0.0F};
    float face{0.0F};
    float behavior{0.0F};
    float context{0.0F};
    float rawTotal{0.0F};
};

class ThreatScorer {
public:
    explicit ThreatScorer(ThreatSettings settings = {});

    [[nodiscard]] float scoreMotion(const std::vector<core::Track>& tracks) const;
    [[nodiscard]] float scoreTrackActivity(const std::vector<core::Track>& tracks) const;
    [[nodiscard]] float scoreFaces(const std::vector<core::FaceMatch>& faces) const;
    [[nodiscard]] float scoreContext(bool nightHours) const;

    [[nodiscard]] SignalScores compose(const std::vector<core::Track>& tracks,
                                       const std::vector<core::FaceMatch>& faces,
                                       float behaviorScore, bool nightHours) const;

    void setSettings(const ThreatSettings& settings);

private:
    ThreatSettings settings_;
};

}  // namespace csx::threat
