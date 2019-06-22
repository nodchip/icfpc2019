#include "../fill_polygon.h"

#include <ostream>
#include <iostream>
#include <gtest/gtest.h>

TEST(fill_polygon, Map_equivalence) {
    Map2D map0(2, 2, {
        1, 1,
        1, 1,
    });
    Map2D map1(2, 2, {
        1, 1,
        1, 1,
    });
    Map2D map2(2, 2, {
        1, 1,
        1, 0,
    });
    Map2D map3(3, 3, {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    });
    EXPECT_EQ(map0, map1);
    EXPECT_NE(map0, map2);
    EXPECT_NE(map0, map3);
}

TEST(fill_polygon, fillPolygon_full) {
    Polygon polygon = {
        {0, 0},
        {4, 0},
        {4, 4},
        {0, 4},
    };
    
    Map2D map(4, 4);
    EXPECT_TRUE(fillPolygon(map, polygon, 1));

    Map2D ground_truth(4, 4, {
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
    });
    EXPECT_EQ(map, ground_truth);

    //std::cout << map << std::endl;
}

TEST(fill_polygon, fillPolygon_rect) {
    Polygon polygon = {
        { 2,  1},
        { 9,  1},
        { 9,  8},
        { 2,  8},
    };
    
    Map2D map(10, 10);
    EXPECT_TRUE(fillPolygon(map, polygon, 1));

    Map2D ground_truth(10, 10, {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    });
    EXPECT_EQ(map, ground_truth);

    //std::cout << map << std::endl;
}

TEST(fill_polygon, fillPolygon_concave) {
    Polygon polygon = {
        {1, 1}, {4, 1}, {4, 3}, {3, 3}, {3, 2}, {2, 2}, {2, 4}, {1, 4}
    };
    
    Map2D map(5, 5);
    EXPECT_TRUE(fillPolygon(map, polygon, 2));

    Map2D ground_truth(5, 5, {
        0, 0, 0, 0, 0,
        0, 2, 2, 2, 0,
        0, 2, 0, 2, 0,
        0, 2, 0, 0, 0,
        0, 0, 0, 0, 0,
    });
    EXPECT_EQ(map, ground_truth);

    //std::cout << map << std::endl;
}

TEST(fill_polygon, parsePolygon) {
    Map2D simple0(3, 3, {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    });
    Polygon polygon0;
    EXPECT_TRUE(parsePolygon(polygon0, simple0, 1));
    for (auto p : polygon0) {
        std::cout << p;
    }


    Map2D simple1(3, 3, {
        2, 2, 2,
        2, 0, 0,
        2, 2, 0,
    });
    Polygon polygon1;
    EXPECT_TRUE(parsePolygon(polygon1, simple1, 2));
    for (auto p : polygon1) {
        std::cout << p;
    }
}