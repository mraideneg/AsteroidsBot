#pragma once
#include <cmath>

inline float wrap(float x, float maxv) {
    x = std::fmod(x, maxv);
    if (x < 0) x += maxv;
    return x;
}