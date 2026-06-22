#include "KalmanPredictor.hpp"

namespace csx::tracking {

KalmanPredictor::KalmanPredictor() = default;

void KalmanPredictor::reset(const core::Point2f& position) {
    position_ = position;
    velocity_ = {};
    predicted_ = position;
    initialized_ = true;
}

void KalmanPredictor::predict() {
    if (!initialized_) {
        return;
    }

    predicted_.x = position_.x + velocity_.x;
    predicted_.y = position_.y + velocity_.y;
    position_ = predicted_;
    velocity_.x *= (1.0F - processNoise_);
    velocity_.y *= (1.0F - processNoise_);
}

void KalmanPredictor::update(const core::Point2f& measurement) {
    if (!initialized_) {
        reset(measurement);
        return;
    }

    const float gain = 1.0F / (1.0F + measurementNoise_);
    const core::Point2f innovation{measurement.x - position_.x, measurement.y - position_.y};

    position_.x += gain * innovation.x;
    position_.y += gain * innovation.y;
    velocity_.x = velocity_.x * (1.0F - gain) + innovation.x * gain;
    velocity_.y = velocity_.y * (1.0F - gain) + innovation.y * gain;
    predicted_ = {position_.x + velocity_.x, position_.y + velocity_.y};
}

core::Point2f KalmanPredictor::position() const noexcept {
    return position_;
}

core::Point2f KalmanPredictor::velocity() const noexcept {
    return velocity_;
}

core::Point2f KalmanPredictor::predictedPosition() const noexcept {
    return predicted_;
}

}  // namespace csx::tracking
