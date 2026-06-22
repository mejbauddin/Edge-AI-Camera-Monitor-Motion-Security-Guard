#pragma once

#include "types/Frame.hpp"

#include <chrono>

namespace csx::recording {

struct BufferedFrame {
    core::Frame frame;
    std::chrono::steady_clock::time_point timestamp{};

    [[nodiscard]] bool valid() const noexcept { return frame.valid(); }
};

}  // namespace csx::recording
