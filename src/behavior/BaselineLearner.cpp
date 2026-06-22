#include "BaselineLearner.hpp"

#include "Geometry.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace csx::behavior {

namespace {

float sampleAverageVelocity(const std::vector<core::Track>& tracks) {
    if (tracks.empty()) {
        return 0.0F;
    }

    float total = 0.0F;
    for (const auto& track : tracks) {
        total += velocityMagnitude(track.velocity);
    }
    return total / static_cast<float>(tracks.size());
}

}  // namespace

FrameStats analyzeFrame(const core::Frame& frame) {
    FrameStats stats;
    if (!frame.valid() || frame.bgrData == nullptr) {
        return stats;
    }

    const auto& pixels = *frame.bgrData;
    if (pixels.empty()) {
        return stats;
    }

    double brightnessSum = 0.0;
    double edgeSum = 0.0;
    std::size_t edgeSamples = 0;

    const std::uint32_t width = frame.width;
    const std::uint32_t height = frame.height;
    const std::size_t stride = static_cast<std::size_t>(width) * 3U;

    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            const std::size_t index = static_cast<std::size_t>(y) * stride + static_cast<std::size_t>(x) * 3U;
            const float b = static_cast<float>(pixels[index]) / 255.0F;
            const float g = static_cast<float>(pixels[index + 1]) / 255.0F;
            const float r = static_cast<float>(pixels[index + 2]) / 255.0F;
            brightnessSum += (r + g + b) / 3.0;

            if (x + 1 < width && y + 1 < height) {
                const std::size_t right = index + 3U;
                const std::size_t down = index + stride;
                const float current = (r + g + b) / 3.0F;
                const float rightLuma =
                    (static_cast<float>(pixels[right + 2]) + static_cast<float>(pixels[right + 1]) +
                     static_cast<float>(pixels[right])) /
                    (3.0F * 255.0F);
                const float downLuma =
                    (static_cast<float>(pixels[down + 2]) + static_cast<float>(pixels[down + 1]) +
                     static_cast<float>(pixels[down])) /
                    (3.0F * 255.0F);
                edgeSum += std::abs(current - rightLuma) + std::abs(current - downLuma);
                ++edgeSamples;
            }
        }
    }

    stats.brightness = static_cast<float>(brightnessSum / static_cast<double>(pixels.size() / 3U));
    stats.edgeEnergy =
        edgeSamples > 0 ? static_cast<float>(edgeSum / static_cast<double>(edgeSamples)) : 0.0F;
    return stats;
}

BaselineLearner::BaselineLearner(BehaviorSettings settings) : settings_(std::move(settings)) {}

void BaselineLearner::updateRunningStats(const float sample, float& mean, float& variance) {
    const float alpha = settings_.baselineAlpha;
    const float delta = sample - mean;
    mean += alpha * delta;
    variance = (1.0F - alpha) * (variance + alpha * delta * delta);
}

void BaselineLearner::observe(const core::Frame& frame, const std::vector<core::Track>& tracks,
                              const FrameStats& stats) {
    (void)frame;
    ++observedFrames_;

    const float avgVelocity = sampleAverageVelocity(tracks);
    const float trackCount = static_cast<float>(tracks.size());

    updateRunningStats(avgVelocity, meanVelocity_, velocityVariance_);
    updateRunningStats(trackCount, meanTrackCount_, trackCountVariance_);
    meanBrightness_ = meanBrightness_ == 0.0F
                          ? stats.brightness
                          : meanBrightness_ + settings_.baselineAlpha * (stats.brightness - meanBrightness_);
    meanEdgeEnergy_ = meanEdgeEnergy_ == 0.0F
                          ? stats.edgeEnergy
                          : meanEdgeEnergy_ + settings_.baselineAlpha * (stats.edgeEnergy - meanEdgeEnergy_);
}

bool BaselineLearner::ready() const noexcept {
    return observedFrames_ >= settings_.baselineLearningFrames;
}

std::uint32_t BaselineLearner::observedFrames() const noexcept {
    return observedFrames_;
}

float BaselineLearner::meanVelocity() const noexcept {
    return meanVelocity_;
}

float BaselineLearner::velocityStdDev() const noexcept {
    return std::sqrt(std::max(velocityVariance_, 1e-4F));
}

float BaselineLearner::meanTrackCount() const noexcept {
    return meanTrackCount_;
}

float BaselineLearner::meanBrightness() const noexcept {
    return meanBrightness_;
}

float BaselineLearner::meanEdgeEnergy() const noexcept {
    return meanEdgeEnergy_;
}

float BaselineLearner::velocityZScore(const float velocity) const noexcept {
    return (velocity - meanVelocity_) / velocityStdDev();
}

bool BaselineLearner::isUnusualVelocity(const float velocity) const noexcept {
    if (!ready()) {
        return false;
    }
    return std::abs(velocityZScore(velocity)) >= settings_.unusualVelocityZScore;
}

bool BaselineLearner::isUnusualTrackCount(const std::size_t trackCount) const noexcept {
    if (!ready()) {
        return false;
    }
    const float deviation = std::abs(static_cast<float>(trackCount) - meanTrackCount_);
    const float threshold = std::max(1.0F, std::sqrt(std::max(trackCountVariance_, 1e-4F)) * 2.0F);
    return deviation > threshold;
}

}  // namespace csx::behavior
