#include <gtest/gtest.h>

#include "Geometry.hpp"

using csx::behavior::pointInPolygon;
using csx::core::Point2f;

TEST(GeometryTest, PointInsideSquare) {
    const std::vector<Point2f> square{{0.0F, 0.0F}, {100.0F, 0.0F}, {100.0F, 100.0F}, {0.0F, 100.0F}};
    EXPECT_TRUE(pointInPolygon({50.0F, 50.0F}, square));
    EXPECT_FALSE(pointInPolygon({150.0F, 50.0F}, square));
}

TEST(GeometryTest, PointInsideTriangle) {
    const std::vector<Point2f> triangle{{0.0F, 0.0F}, {100.0F, 0.0F}, {50.0F, 100.0F}};
    EXPECT_TRUE(pointInPolygon({50.0F, 30.0F}, triangle));
    EXPECT_FALSE(pointInPolygon({10.0F, 90.0F}, triangle));
}
