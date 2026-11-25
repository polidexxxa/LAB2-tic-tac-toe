#include <gtest/gtest.h>

#include "game/Point.hpp"

TEST(PointTests, DefaultConstructorCreatesOrigin) {
    Point point;

    EXPECT_EQ(point.x, 0);
    EXPECT_EQ(point.y, 0);
}

TEST(PointTests, ConstructorStoresCoordinates) {
    Point point(-7, 12);

    EXPECT_EQ(point.x, -7);
    EXPECT_EQ(point.y, 12);
}

TEST(PointTests, EqualityComparesBothCoordinates) {
    EXPECT_TRUE(Point(2, 3) == Point(2, 3));
    EXPECT_FALSE(Point(2, 3) == Point(2, 4));
    EXPECT_FALSE(Point(2, 3) == Point(4, 3));
}

TEST(PointTests, InequalityComparesBothCoordinates) {
    EXPECT_TRUE(Point(2, 3) != Point(3, 2));
    EXPECT_FALSE(Point(2, 3) != Point(2, 3));
}

TEST(PointTests, OrderingComparesXBeforeY) {
    EXPECT_TRUE(Point(-1, 100) < Point(0, -100));
    EXPECT_FALSE(Point(1, -100) < Point(0, 100));
}

TEST(PointTests, OrderingComparesYWhenXIsEqual) {
    EXPECT_TRUE(Point(2, 3) < Point(2, 4));
    EXPECT_FALSE(Point(2, 4) < Point(2, 3));
    EXPECT_FALSE(Point(2, 3) < Point(2, 3));
}