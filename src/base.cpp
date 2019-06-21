#include "base.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const Map2D& map) {
    for (int y = 0; y < map.H; ++y) {
        for (int x = 0; x < map.W; ++x) {
            std::cout << map(x, y);
        }
        std::cout << std::endl;
    }
    return os;
}