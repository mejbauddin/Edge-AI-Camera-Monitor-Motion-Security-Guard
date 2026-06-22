#pragma once

#include "BehaviorSettings.hpp"
#include "types/Frame.hpp"

#include <cstdint>
#include <optional>

namespace csx::behavior {

struct FrameStats {
    float brightness{0.0F};
    float edgeEnergy{0.0F};
};

class BaselineLearner {
public:
    explicit BaselineLearner(BehaviorSettings settings);

    void observe(const core::Frame& frame, const std::vector<core::Track>& tracks,
                 const FrameStats& stats);
    [[nodiscard]] bool ready() const noexcept;
    [[nodiscard]] std::uint32_t observedFrames() const noexcept;

    [[nodiscard]] float meanVelocity() const noexcept;
    [[nodiscard]] float velocityStdDev() const noexcept;
    [[nodiscard]] float meanTrackCount() const noexcept;
    [[nodiscard]] float meanBrightness() const noexcept;
    [[nodiscard]] float meanEdgeEnergy() const noexcept;

    [[nodiscard]] float velocityZScore(float velocity) const noexcept;
    [[nodiscard]] bool isUnusualVelocity(float velocity) const noexcept;
    [[nodiscard]] bool isUnusualTrackCount(std::size_t trackCount) const noexcept;

private:
    void updateRunningStats(float sample, float& mean, float& variance);

    BehaviorSettings settings_;
    std::uint32_t observedFrames_{0};
    float meanVelocity_{0.0F};
    float velocityVariance_{0.0F};
    float meanTrackCount_{0.0F};
    float trackCountVariance_{0.0F};
    float meanBrightness_{0.0F};
    float meanEdgeEnergy_{0.0F};
};

[[nodiscard]] FrameStats analyzeFrame(const core::Frame& frame);

}  // namespace csx::behavior
