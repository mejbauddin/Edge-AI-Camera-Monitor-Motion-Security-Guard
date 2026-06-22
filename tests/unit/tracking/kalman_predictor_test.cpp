#include <gtest/gtest.h>

#include "KalmanPredictor.hpp"

using csx::tracking::KalmanPredictor;

TEST(KalmanPredictorTest, PredictsForwardMotion) {
    KalmanPredictor kalman;
    kalman.reset({10.0F, 10.0F});
    kalman.update({12.0F, 10.0F});
    kalman.predict();

    const auto predicted = kalman.predictedPosition();
    EXPECT_GT(predicted.x, 12.0F);
    EXPECT_NEAR(predicted.y, 10.0F, 2.0F);
}
