#include "FrameImageProvider.hpp"

#include <cstring>

#if defined(CSX_HAS_OPENCV)
#include <opencv2/imgproc.hpp>
#endif

namespace csx::ui {

FrameImageProvider::FrameImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

FrameImageProvider::~FrameImageProvider() = default;

QImage FrameImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    (void)id;
    (void)requestedSize;

    std::lock_guard<std::mutex> lock(mutex_);

    if (!currentFrame_ || width_ == 0 || height_ == 0) {
        if (size) {
            *size = QSize(640, 480);
        }
        return QImage(640, 480, QImage::Format_RGB888);
    }

    QImage image(static_cast<int>(width_), static_cast<int>(height_), QImage::Format_RGB888);

#if defined(CSX_HAS_OPENCV)
    cv::Mat bgrMat(static_cast<int>(height_), static_cast<int>(width_), CV_8UC3,
                   const_cast<std::uint8_t*>(currentFrame_->data()));
    cv::Mat rgbMat;
    cv::cvtColor(bgrMat, rgbMat, cv::COLOR_BGR2RGB);
    std::memcpy(image.bits(), rgbMat.data, static_cast<std::size_t>(rgbMat.total() * rgbMat.elemSize()));
#else
    const auto* bgr = currentFrame_->data();
    auto* rgb = image.bits();
    const auto pixelCount = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
    for (std::size_t i = 0; i < pixelCount; ++i) {
        const auto src = i * 3U;
        const auto dst = i * 3U;
        rgb[dst + 0] = bgr[src + 2];
        rgb[dst + 1] = bgr[src + 1];
        rgb[dst + 2] = bgr[src + 0];
    }
#endif

    if (size) {
        *size = image.size();
    }
    return image;
}

QImage FrameImageProvider::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!currentFrame_ || width_ == 0 || height_ == 0) {
        return {};
    }

    QImage image(static_cast<int>(width_), static_cast<int>(height_), QImage::Format_RGB888);

#if defined(CSX_HAS_OPENCV)
    cv::Mat bgrMat(static_cast<int>(height_), static_cast<int>(width_), CV_8UC3,
                   const_cast<std::uint8_t*>(currentFrame_->data()));
    cv::Mat rgbMat;
    cv::cvtColor(bgrMat, rgbMat, cv::COLOR_BGR2RGB);
    std::memcpy(image.bits(), rgbMat.data, static_cast<std::size_t>(rgbMat.total() * rgbMat.elemSize()));
#else
    const auto* bgr = currentFrame_->data();
    auto* rgb = image.bits();
    const auto pixelCount = static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
    for (std::size_t i = 0; i < pixelCount; ++i) {
        const auto src = i * 3U;
        const auto dst = i * 3U;
        rgb[dst + 0] = bgr[src + 2];
        rgb[dst + 1] = bgr[src + 1];
        rgb[dst + 2] = bgr[src + 0];
    }
#endif

    return image;
}

void FrameImageProvider::updateFrame(const std::shared_ptr<const std::vector<std::uint8_t>>& bgrData,
                                     const std::uint32_t width, const std::uint32_t height)
{
    std::lock_guard<std::mutex> lock(mutex_);
    currentFrame_ = bgrData;
    width_ = width;
    height_ = height;
    frameCount_.fetch_add(1, std::memory_order_relaxed);
}

void FrameImageProvider::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    currentFrame_.reset();
    width_ = 0;
    height_ = 0;
}

}  // namespace csx::ui
