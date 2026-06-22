#include <gtest/gtest.h>

#include "BehaviorFusion.hpp"

using csx::core::BehaviorAnomaly;
using csx::core::BehaviorAnomalyType;
using csx::threat::BehaviorFusion;

TEST(BehaviorFusionTest, EmptyAnomaliesReturnZero) {
    BehaviorFusion fusion;
    const auto result = fusion.fuse({});
    EXPECT_FLOAT_EQ(result.behaviorScore, 0.0F);
    EXPECT_TRUE(result.summary.empty());
}

TEST(BehaviorFusionTest, IntrusionProducesHighBehaviorScore) {
    BehaviorFusion fusion;
    BehaviorAnomaly anomaly;
    anomaly.type = BehaviorAnomalyType::Intrusion;
    anomaly.score = 0.9F;

    const auto result = fusion.fuse({anomaly});
    EXPECT_GT(result.behaviorScore, 80.0F);
    EXPECT_FALSE(result.summary.empty());
}
