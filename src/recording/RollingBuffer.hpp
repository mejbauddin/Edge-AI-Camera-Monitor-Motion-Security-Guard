#pragma once

#include "BufferedFrame.hpp"
#include "RecordingSettings.hpp"

#include <deque>
#include <vector>

namespace csx::recording {

class RollingBuffer {
public:
    explicit RollingBuffer(RecordingSettings settings = {});

    void push(BufferedFrame frame);
    [[nodiscard]] std::vector<BufferedFrame> snapshot() const;
    [[nodiscard]] std::size_t size() const noexcept;
    void clear();
    void setSettings(const RecordingSettings& settings);

private:
    void pruneExpired();

    RecordingSettings settings_;
    std::deque<BufferedFrame> frames_;
    std::size_t capacityFrames_{150};
};

}  // namespace csx::recording
