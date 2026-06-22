#pragma once

#include <QQuickImageProvider>
#include <QImage>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace csx::ui {

class FrameImageProvider : public QQuickImageProvider {
public:
    FrameImageProvider();
    ~FrameImageProvider() override;

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    void updateFrame(const std::shared_ptr<const std::vector<std::uint8_t>>& bgrData,
                     std::uint32_t width, std::uint32_t height);

    [[nodiscard]] QImage snapshot() const;

    void clear();

private:
    mutable std::mutex mutex_;
    std::shared_ptr<const std::vector<std::uint8_t>> currentFrame_;
    std::uint32_t width_{0};
    std::uint32_t height_{0};
    std::atomic<std::uint64_t> frameCount_{0};
};

}  // namespace csx::ui
