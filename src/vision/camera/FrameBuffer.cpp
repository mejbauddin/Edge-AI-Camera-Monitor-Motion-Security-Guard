#include "FrameBuffer.hpp"

namespace csx::camera {

FrameBuffer::FrameBuffer(const std::size_t capacity) : capacity_(capacity == 0 ? 1 : capacity) {}

bool FrameBuffer::push(core::Frame frame) {
    std::lock_guard lock(mutex_);
    if (frames_.size() >= capacity_) {
        frames_.pop_front();
        ++droppedFrames_;
    }
    frames_.push_back(std::move(frame));
    condition_.notify_one();
    return true;
}

bool FrameBuffer::pop(core::Frame& outFrame, const std::chrono::milliseconds timeout) {
    std::unique_lock lock(mutex_);
    if (!condition_.wait_for(lock, timeout, [this] { return !frames_.empty(); })) {
        return false;
    }
    outFrame = std::move(frames_.front());
    frames_.pop_front();
    return true;
}

std::size_t FrameBuffer::size() const {
    std::lock_guard lock(mutex_);
    return frames_.size();
}

std::size_t FrameBuffer::capacity() const noexcept {
    return capacity_;
}

std::uint64_t FrameBuffer::droppedFrames() const noexcept {
    std::lock_guard lock(mutex_);
    return droppedFrames_;
}

void FrameBuffer::clear() {
    std::lock_guard lock(mutex_);
    frames_.clear();
}

void FrameBuffer::reset(const std::size_t capacity) {
    std::lock_guard lock(mutex_);
    frames_.clear();
    capacity_ = capacity == 0 ? 1 : capacity;
    droppedFrames_ = 0;
}

}  // namespace csx::camera
