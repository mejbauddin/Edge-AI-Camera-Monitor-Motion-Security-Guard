#include "MotionEngine.hpp"

#include "BackgroundSubtractors.hpp"

#include "CvHelpers.hpp"
#include "Timer.hpp"

namespace csx::motion {

MotionEngine::MotionEngine(MotionSettings settings)
    : settings_(std::move(settings)),
      subtractor_(createBackgroundSubtractor(settings_)),
      morphology_(settings_.morphKernelSize, settings_.morphIterations),
      blobAnalyzer_(settings_.minBlobArea) {}

MotionEngine::~MotionEngine() = default;

void MotionEngine::setSettings(const MotionSettings& settings) {
    settings_ = settings;
    subtractor_ = createBackgroundSubtractor(settings_);
    morphology_ = MorphologyFilter(settings_.morphKernelSize, settings_.morphIterations);
    blobAnalyzer_ = BlobAnalyzer(settings_.minBlobArea);
}

const std::vector<std::uint8_t>& MotionEngine::lastMask() const noexcept {
    return foregroundMask_;
}

void MotionEngine::ensureProcessingBuffer(const std::uint32_t width, const std::uint32_t height) {
    processingBgr_.resize(csx::utils::bgrBufferSize(width, height));
}

void MotionEngine::process(const core::Frame& frame, std::vector<core::Rect2f>& outBlobs) {
    outBlobs.clear();
    if (!frame.valid()) {
        health_.status = core::EngineStatus::Degraded;
        health_.detail = "Invalid frame";
        return;
    }

    csx::utils::Timer timer;

    const auto sourceWidth = frame.width;
    const auto sourceHeight = frame.height;
    const auto processWidth = std::min(sourceWidth, settings_.processingWidth);
    const auto processHeight = std::min(sourceHeight, settings_.processingHeight);

    ensureProcessingBuffer(processWidth, processHeight);
    csx::utils::resizeBgrNearestNeighbor(frame.bgrData->data(), sourceWidth, sourceHeight,
                                         processingBgr_.data(), processWidth, processHeight);

    subtractor_->apply(processingBgr_.data(), processWidth, processHeight, foregroundMask_);

    if (settings_.shadowRemoval) {
        shadowRemover_.apply(processingBgr_.data(), foregroundMask_, processWidth, processHeight);
    }

    morphology_.open(foregroundMask_, processWidth, processHeight);
    morphology_.close(foregroundMask_, processWidth, processHeight);

    const float scaleX = static_cast<float>(sourceWidth) / static_cast<float>(processWidth);
    const float scaleY = static_cast<float>(sourceHeight) / static_cast<float>(processHeight);
    blobAnalyzer_.analyze(foregroundMask_, processWidth, processHeight, scaleX, scaleY, outBlobs);

    ++processedFrames_;
    lastLatencyMs_ = timer.elapsedMs();
    health_.status = core::EngineStatus::Online;
    health_.detail = "Motion detection active";
    health_.confidence = 1.0F;
    health_.lastLatencyMs = static_cast<float>(lastLatencyMs_);
    health_.processedFrames = processedFrames_;
}

core::EngineHealth MotionEngine::health() const {
    return health_;
}

std::shared_ptr<core::IMotionEngine> createMotionEngine(MotionSettings settings) {
    return std::make_shared<MotionEngine>(std::move(settings));
}

}  // namespace csx::motion
