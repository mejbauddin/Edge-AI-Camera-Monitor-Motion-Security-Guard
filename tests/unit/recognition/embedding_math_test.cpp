#include <gtest/gtest.h>

#include "EmbeddingMath.hpp"
#include "FaceDatabase.hpp"

using csx::recognition::FaceDatabase;
using csx::recognition::StoredFaceIdentity;
using csx::recognition::averageEmbeddings;
using csx::recognition::l2Distance;
using csx::recognition::normalizeEmbedding;

TEST(EmbeddingMathTest, ComputesDistanceAndAverage) {
    std::vector<float> a{1.0F, 0.0F, 0.0F};
    std::vector<float> b{0.0F, 1.0F, 0.0F};
    EXPECT_GT(l2Distance(a, b), 1.0F);

    const auto average = averageEmbeddings({a, b});
    ASSERT_EQ(average.size(), 3U);
    EXPECT_GT(average[0], 0.0F);
    EXPECT_GT(average[1], 0.0F);
}

TEST(FaceDatabaseTest, FindsAuthorizedMatch) {
    FaceDatabase database;
    StoredFaceIdentity identity;
    identity.userId = 1;
    identity.userName = "Alpha";
    identity.embedding = std::vector<float>(512, 0.0F);
    identity.embedding[0] = 1.0F;
    normalizeEmbedding(identity.embedding);
    database.addAuthorized(identity);

    std::vector<float> probe = identity.embedding;
    probe[1] = 0.01F;
    normalizeEmbedding(probe);

    const auto match = database.findBestMatch(probe, 0.5F);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->identity.userName, "Alpha");
}
