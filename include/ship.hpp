#pragma once
#include <array>
#include <vector>
#include "constants.hpp"

class ship_state {
public:
    std::array<float, 2> vel;
    std::array<float, 2> pos;
    float angle;
    float distance;
    std::array<ship_state*, num_branches> next;

    ship_state();
};

class ship_state_pool {
public:
    std::vector<ship_state> pool;
    int index;
    int num_nodes;

    ship_state_pool(int num_nodes);
    ship_state* get_next();
    void reset();
};