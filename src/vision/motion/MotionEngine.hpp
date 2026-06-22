#pragma once

#include "BlobAnalyzer.hpp"
#include "MotionSettings.hpp"
#include "MorphologyFilter.hpp"
#include "ShadowRemover.hpp"
#include "interfaces/Interfaces.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace csx::motion {

class MotionEngine final : public core::IMotionEngine {
public:
    explicit MotionEngine(MotionSettings settings = {});
    ~MotionEngine() override;

    void process(const core::Frame& frame, std::vector<core::Rect2f>& outBlobs) override;
    [[nodiscard]] core::EngineHealth health() const override;

    void setSettings(const MotionSettings& settings);
    [[nodiscard]] const std::vector<std::uint8_t>& lastMask() const noexcept;

private:
    void ensureProcessingBuffer(std::uint32_t width, std::uint32_t height);

    MotionSettings settings_;
    std::unique_ptr<class IBackgroundSubtractor> subtractor_;
    MorphologyFilter morphology_;
    ShadowRemover shadowRemover_;
    BlobAnalyzer blobAnalyzer_;
    std::vector<std::uint8_t> processingBgr_;
    std::vector<std::uint8_t> foregroundMask_;
    core::EngineHealth health_;
    std::uint64_t processedFrames_{0};
    double lastLatencyMs_{0.0};
};

std::shared_ptr<core::IMotionEngine> createMotionEngine(MotionSettings settings = {});

}  // namespace csx::motion
