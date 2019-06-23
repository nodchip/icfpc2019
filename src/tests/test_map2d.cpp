#include "map2d.h"

#include <gtest/gtest.h>
#include <iostream>

TEST(Map, isConnected4) {
    Map2D m0(3, 2, {
        1, 1, 1,
        1, 1, 1,
    });
    EXPECT_TRUE(isConnected4(m0));

    Map2D m1(3, 2, {
        1, 1, 3,
        1, 0, 2,
    });
    EXPECT_TRUE(isConnected4(m1));

    Map2D m2(3, 2, {
        1, 1, 0,
        1, 0, 1,
    });
    EXPECT_FALSE(isConnected4(m2));

    Map2D m3(3, 2, {
        1, 2, 4,
        1, 3, 1,
    });
    EXPECT_TRUE(isConnected4(m3));
}
