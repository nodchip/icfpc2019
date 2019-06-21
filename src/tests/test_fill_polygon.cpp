#include "../fill_polygon.h"

#include <ostream>
#include <iostream>
#include <gtest/gtest.h>

TEST(fill_polygon, FillPolygon_full) {
    Polygon polygon = {
        { 0,  0},
        {10,  0},
        {10, 10},
        { 0, 10},
    };
    
    Map2D map(10, 10);
    EXPECT_TRUE(FillPolygon(map, polygon, 1));

    std::cout << map << std::endl;
}

TEST(fill_polygon, FillPolygon_rect) {
    Polygon polygon = {
        { 2,  1},
        { 9,  1},
        { 9,  8},
        { 2,  8},
    };
    
    Map2D map(10, 10);
    EXPECT_TRUE(FillPolygon(map, polygon, 1));

    std::cout << map << std::endl;
}

TEST(fill_polygon, FillPolygon_concave) {
    Polygon polygon = {
        {1, 1}, {4, 1}, {4, 3}, {3, 3}, {3, 2}, {2, 2}, {2, 4}, {1, 4}
    };
    
    Map2D map(5, 5);
    EXPECT_TRUE(FillPolygon(map, polygon, 2));

    std::cout << map << std::endl;
}