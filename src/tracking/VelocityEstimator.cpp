#include "VelocityEstimator.hpp"

#include <cmath>

namespace csx::tracking {

VelocityEstimator::VelocityEstimator(const float smoothing) : smoothing_(smoothing) {}

void VelocityEstimator::reset(const core::Point2f& position) {
    previousPosition_ = position;
    previousVelocity_ = {};
    initialized_ = true;
}

void VelocityEstimator::update(const core::Point2f& position, core::Point2f& outVelocity,
                               core::Point2f& outAcceleration, float& outDirectionDegrees) {
    if (!initialized_) {
        reset(position);
        outVelocity = {};
        outAcceleration = {};
        outDirectionDegrees = 0.0F;
        return;
    }

    const core::Point2f rawVelocity{position.x - previousPosition_.x,
                                  position.y - previousPosition_.y};
    outVelocity.x = previousVelocity_.x * smoothing_ + rawVelocity.x * (1.0F - smoothing_);
    outVelocity.y = previousVelocity_.y * smoothing_ + rawVelocity.y * (1.0F - smoothing_);

    outAcceleration.x = outVelocity.x - previousVelocity_.x;
    outAcceleration.y = outVelocity.y - previousVelocity_.y;

    outDirectionDegrees =
        std::atan2(outVelocity.y, outVelocity.x) * 180.0F / 3.14159265F;

    previousPosition_ = position;
    previousVelocity_ = outVelocity;
}

}  // namespace csx::tracking
