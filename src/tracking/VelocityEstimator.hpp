#pragma once

#include "types/Frame.hpp"

namespace csx::tracking {

class VelocityEstimator {
public:
    explicit VelocityEstimator(float smoothing = 0.6F);

    void reset(const core::Point2f& position);
    void update(const core::Point2f& position, core::Point2f& outVelocity, core::Point2f& outAcceleration,
                float& outDirectionDegrees);

private:
    float smoothing_;
    core::Point2f previousPosition_{};
    core::Point2f previousVelocity_{};
    bool initialized_{false};
};

}  // namespace csx::tracking
