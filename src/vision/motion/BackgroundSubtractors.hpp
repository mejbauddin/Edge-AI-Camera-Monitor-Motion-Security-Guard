#pragma once

#include "MotionSettings.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace csx::motion {

class IBackgroundSubtractor {
public:
    virtual ~IBackgroundSubtractor() = default;
    virtual void apply(const std::uint8_t* bgr, std::uint32_t width, std::uint32_t height,
                       std::vector<std::uint8_t>& foregroundMask) = 0;
    virtual void reset() = 0;
};

class Mog2Subtractor final : public IBackgroundSubtractor {
public:
    explicit Mog2Subtractor(const MotionSettings& settings);

    void apply(const std::uint8_t* bgr, std::uint32_t width, std::uint32_t height,
               std::vector<std::uint8_t>& foregroundMask) override;
    void reset() override;

private:
    MotionSettings settings_;
    std::vector<float> background_;
    std::uint32_t width_{0};
    std::uint32_t height_{0};
    bool initialized_{false};
};

class KnnSubtractor final : public IBackgroundSubtractor {
public:
    explicit KnnSubtractor(const MotionSettings& settings);

    void apply(const std::uint8_t* bgr, std::uint32_t width, std::uint32_t height,
               std::vector<std::uint8_t>& foregroundMask) override;
    void reset() override;

private:
    MotionSettings settings_;
    std::vector<std::vector<float>> samples_;
    std::uint32_t width_{0};
    std::uint32_t height_{0};
    bool initialized_{false};
};

std::unique_ptr<IBackgroundSubtractor> createBackgroundSubtractor(const MotionSettings& settings);

}  // namespace csx::motion
