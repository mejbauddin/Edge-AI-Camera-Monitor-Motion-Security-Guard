#include "RollingBuffer.hpp"

namespace csx::recording {

RollingBuffer::RollingBuffer(RecordingSettings settings) : settings_(std::move(settings)) {
    capacityFrames_ =
        static_cast<std::size_t>(settings_.preBufferSeconds) * static_cast<std::size_t>(settings_.assumedFps);
    if (capacityFrames_ < 1) {
        capacityFrames_ = 1;
    }
}

void RollingBuffer::setSettings(const RecordingSettings& settings) {
    settings_ = settings;
    capacityFrames_ =
        static_cast<std::size_t>(settings_.preBufferSeconds) * static_cast<std::size_t>(settings_.assumedFps);
    if (capacityFrames_ < 1) {
        capacityFrames_ = 1;
    }
    pruneExpired();
}

void RollingBuffer::push(BufferedFrame frame) {
    if (!frame.valid()) {
        return;
    }
    if (frame.timestamp == std::chrono::steady_clock::time_point{}) {
        frame.timestamp = frame.frame.captureTime;
    }
    frames_.push_back(std::move(frame));
    pruneExpired();
}

void RollingBuffer::pruneExpired() {
    if (frames_.empty()) {
        return;
    }

    const auto now = frames_.back().timestamp;
    const auto maxAge = std::chrono::seconds(settings_.preBufferSeconds);
    while (!frames_.empty() && now - frames_.front().timestamp > maxAge) {
        frames_.pop_front();
    }

    while (frames_.size() > capacityFrames_) {
        frames_.pop_front();
    }
}

std::vector<BufferedFrame> RollingBuffer::snapshot() const {
    return {frames_.begin(), frames_.end()};
}

std::size_t RollingBuffer::size() const noexcept {
    return frames_.size();
}

void RollingBuffer::clear() {
    frames_.clear();
}

}  // namespace csx::recording
