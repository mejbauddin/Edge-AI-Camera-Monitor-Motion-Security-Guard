#include "BackgroundSubtractors.hpp"

#include "CvHelpers.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <mutex>
#include <numeric>

#if defined(CSX_HAS_OPENCV)
#include <opencv2/video/background_segm.hpp>
#endif

namespace csx::motion {

namespace {

float pixelDistance(const std::uint8_t* a, const std::uint8_t* b) {
    const float db = static_cast<float>(a[0]) - static_cast<float>(b[0]);
    const float dg = static_cast<float>(a[1]) - static_cast<float>(b[1]);
    const float dr = static_cast<float>(a[2]) - static_cast<float>(b[2]);
    return std::sqrt(db * db + dg * dg + dr * dr);
}

}  // namespace

#if defined(CSX_HAS_OPENCV)

class OpenCvMog2Subtractor final : public IBackgroundSubtractor {
public:
    explicit OpenCvMog2Subtractor(const MotionSettings& settings)
        : subtractor_(cv::createBackgroundSubtractorMOG2(500, 16.0, settings.shadowRemoval)),
          threshold_(settings.threshold) {}

    void apply(const std::uint8_t* bgr, const std::uint32_t width, const std::uint32_t height,
               std::vector<std::uint8_t>& foregroundMask) override {
        std::lock_guard<std::mutex> lock(csx::utils::openCvMutex());
        const cv::Mat inputView(static_cast<int>(height), static_cast<int>(width), CV_8UC3,
                                const_cast<std::uint8_t*>(bgr));
        const cv::Mat input = inputView.clone();
        cv::Mat mask;
        subtractor_->apply(input, mask);
        foregroundMask.assign(mask.data, mask.data + mask.total());
        for (auto& value : foregroundMask) {
            value = value >= static_cast<std::uint8_t>(threshold_) ? 255 : 0;
        }
    }

    void reset() override { subtractor_ = cv::createBackgroundSubtractorMOG2(); }

private:
    cv::Ptr<cv::BackgroundSubtractorMOG2> subtractor_;
    int threshold_;
};

class OpenCvKnnSubtractor final : public IBackgroundSubtractor {
public:
    explicit OpenCvKnnSubtractor(const MotionSettings& settings)
        : subtractor_(cv::createBackgroundSubtractorKNN(500, 400.0, settings.shadowRemoval)),
          threshold_(settings.threshold) {}

    void apply(const std::uint8_t* bgr, const std::uint32_t width, const std::uint32_t height,
               std::vector<std::uint8_t>& foregroundMask) override {
        std::lock_guard<std::mutex> lock(csx::utils::openCvMutex());
        const cv::Mat inputView(static_cast<int>(height), static_cast<int>(width), CV_8UC3,
                                const_cast<std::uint8_t*>(bgr));
        const cv::Mat input = inputView.clone();
        cv::Mat mask;
        subtractor_->apply(input, mask);
        foregroundMask.assign(mask.data, mask.data + mask.total());
        for (auto& value : foregroundMask) {
            value = value >= static_cast<std::uint8_t>(threshold_) ? 255 : 0;
        }
    }

    void reset() override { subtractor_ = cv::createBackgroundSubtractorKNN(); }

private:
    cv::Ptr<cv::BackgroundSubtractorKNN> subtractor_;
    int threshold_;
};

#endif

Mog2Subtractor::Mog2Subtractor(const MotionSettings& settings) : settings_(settings) {}

void Mog2Subtractor::apply(const std::uint8_t* bgr, const std::uint32_t width,
                           const std::uint32_t height, std::vector<std::uint8_t>& foregroundMask) {
    const auto pixelCount = static_cast<std::size_t>(width) * height;
    foregroundMask.assign(pixelCount, 0);
    if (bgr == nullptr || pixelCount == 0) {
        return;
    }

    if (!initialized_ || width_ != width || height_ != height) {
        width_ = width;
        height_ = height;
        background_.assign(pixelCount * 3U, 0.0F);
        for (std::size_t i = 0; i < pixelCount; ++i) {
            background_[i * 3U + 0] = static_cast<float>(bgr[i * 3U + 0]);
            background_[i * 3U + 1] = static_cast<float>(bgr[i * 3U + 1]);
            background_[i * 3U + 2] = static_cast<float>(bgr[i * 3U + 2]);
        }
        initialized_ = true;
        return;
    }

    const auto alpha = static_cast<float>(settings_.learningRate);
    for (std::size_t i = 0; i < pixelCount; ++i) {
        const auto* pixel = bgr + i * 3U;
        float variance = 0.0F;
        for (int channel = 0; channel < 3; ++channel) {
            const auto bg = background_[i * 3U + static_cast<std::size_t>(channel)];
            const auto current = static_cast<float>(pixel[channel]);
            const auto delta = current - bg;
            variance += delta * delta;
            background_[i * 3U + static_cast<std::size_t>(channel)] =
                bg * (1.0F - alpha) + current * alpha;
        }

        const auto distance = std::sqrt(variance);
        if (distance > static_cast<float>(settings_.threshold)) {
            foregroundMask[i] = 255;
        }
    }
}

void Mog2Subtractor::reset() {
    background_.clear();
    initialized_ = false;
}

KnnSubtractor::KnnSubtractor(const MotionSettings& settings) : settings_(settings) {}

void KnnSubtractor::apply(const std::uint8_t* bgr, const std::uint32_t width,
                          const std::uint32_t height, std::vector<std::uint8_t>& foregroundMask) {
    const auto pixelCount = static_cast<std::size_t>(width) * height;
    foregroundMask.assign(pixelCount, 0);
    if (bgr == nullptr || pixelCount == 0) {
        return;
    }

    if (!initialized_ || width_ != width || height_ != height) {
        width_ = width;
        height_ = height;
        samples_.assign(pixelCount, std::vector<float>{});
        for (std::size_t i = 0; i < pixelCount; ++i) {
            samples_[i] = {static_cast<float>(bgr[i * 3U + 0]), static_cast<float>(bgr[i * 3U + 1]),
                           static_cast<float>(bgr[i * 3U + 2])};
        }
        initialized_ = true;
        return;
    }

    for (std::size_t i = 0; i < pixelCount; ++i) {
        const auto* pixel = bgr + i * 3U;
        auto& sample = samples_[i];
        float distanceSquared = 0.0F;
        for (int channel = 0; channel < 3; ++channel) {
            const auto delta =
                static_cast<float>(pixel[channel]) - sample[static_cast<std::size_t>(channel)];
            distanceSquared += delta * delta;
        }
        if (std::sqrt(distanceSquared) > static_cast<float>(settings_.threshold)) {
            foregroundMask[i] = 255;
            const auto alpha = static_cast<float>(settings_.learningRate);
            for (int channel = 0; channel < 3; ++channel) {
                sample[static_cast<std::size_t>(channel)] =
                    sample[static_cast<std::size_t>(channel)] * (1.0F - alpha) +
                    static_cast<float>(pixel[channel]) * alpha;
            }
        }
    }
}

void KnnSubtractor::reset() {
    samples_.clear();
    initialized_ = false;
}

std::unique_ptr<IBackgroundSubtractor> createBackgroundSubtractor(const MotionSettings& settings) {
#if defined(CSX_HAS_OPENCV)
    if (settings.algorithm == "knn") {
        return std::make_unique<OpenCvKnnSubtractor>(settings);
    }
    return std::make_unique<OpenCvMog2Subtractor>(settings);
#else
    if (settings.algorithm == "knn") {
        return std::make_unique<KnnSubtractor>(settings);
    }
    return std::make_unique<Mog2Subtractor>(settings);
#endif
}

}  // namespace csx::motion
