#pragma once

#include "types/Frame.hpp"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>

namespace csx::camera {

class FrameBuffer {
public:
    explicit FrameBuffer(std::size_t capacity = 3);

    bool push(core::Frame frame);
    bool pop(core::Frame& outFrame, std::chrono::milliseconds timeout = std::chrono::milliseconds{50});
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::uint64_t droppedFrames() const noexcept;
    void clear();
    void reset(std::size_t capacity);

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::deque<core::Frame> frames_;
    std::size_t capacity_;
    std::uint64_t droppedFrames_{0};
};

}  // namespace csx::camera
