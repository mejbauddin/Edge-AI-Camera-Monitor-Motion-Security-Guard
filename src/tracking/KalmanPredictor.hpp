#pragma once

#include "types/Frame.hpp"

namespace csx::tracking {

class KalmanPredictor {
public:
    KalmanPredictor();

    void reset(const core::Point2f& position);
    void predict();
    void update(const core::Point2f& measurement);

    [[nodiscard]] core::Point2f position() const noexcept;
    [[nodiscard]] core::Point2f velocity() const noexcept;
    [[nodiscard]] core::Point2f predictedPosition() const noexcept;

private:
    core::Point2f position_{};
    core::Point2f velocity_{};
    core::Point2f predicted_{};
    bool initialized_{false};
    float processNoise_{0.05F};
    float measurementNoise_{0.8F};
};

}  // namespace csx::tracking
