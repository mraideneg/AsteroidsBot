#pragma once
#include <vector>
#include <array>

class asteroid_state{
public:
    std::array<float, 2> vel;
    std::array<float, 2> pos;
    float rad;

    asteroid_state();
    asteroid_state(const std::array<float, 2>& vel, const std::array<float, 2>& pos, float rad);
    asteroid_state(const asteroid_state& other);
    asteroid_state& operator=(const asteroid_state& other);
};

class asteroid_trajectory{
public:

    std::vector<asteroid_state> trajectories;
    int num_asteroids;
    int layers;

    asteroid_trajectory(const std::vector<asteroid_state>& asteroids, int num_asteroids, int layers);

private:
    asteroid_state transform(const asteroid_state& asteroid);
};
