#include <gtest/gtest.h>

#include "ThreatHistory.hpp"

using csx::core::ThreatAssessment;
using csx::core::ThreatLevel;
using csx::threat::ThreatHistory;
using csx::threat::ThreatSettings;

TEST(ThreatHistoryTest, EscalatesImmediately) {
    ThreatSettings settings;
    settings.deescalationRate = 0.2F;
    ThreatHistory history(settings);

    EXPECT_FLOAT_EQ(history.smoothScore(10.0F), 10.0F);
    EXPECT_FLOAT_EQ(history.smoothScore(80.0F), 80.0F);
}

TEST(ThreatHistoryTest, DeescalatesGradually) {
    ThreatSettings settings;
    settings.deescalationRate = 0.5F;
    ThreatHistory history(settings);

    const float first = history.smoothScore(80.0F);
    const float smoothed = history.smoothScore(10.0F);
    EXPECT_GT(smoothed, 10.0F);
    EXPECT_LT(smoothed, first);
}

TEST(ThreatHistoryTest, RecordsPeakLevel) {
    ThreatHistory history;
    ThreatAssessment high;
    high.level = ThreatLevel::Red;
    high.threatScore = 75.0F;
    history.record(high);

    EXPECT_EQ(history.peakLevel(), ThreatLevel::Red);
}
