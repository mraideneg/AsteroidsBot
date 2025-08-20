#pragma once
#include "ship.hpp"
#include "asteroid.hpp"

class ship_trajectory {
public:
    ship_state_pool pool;
    int layers;

    ship_trajectory(int layers);
    int best_move(float ship_x, float ship_y, float ship_vx, float ship_vy, float ship_angle, const asteroid_trajectory& asteroid_trajectories);

private:
    void construct_tree(ship_state_pool& pool, ship_state& state, const asteroid_trajectory& asteroid_trajectories, int num_asteroids, int layers, int layer = 0);
    float score_trajectory(const ship_state* state, int layer = 0, int layers = 10);
    float dist_to_collision(const ship_state* ship, const asteroid_state* asteroids, int num_asteroids);
    float dist(const ship_state& ship, const asteroid_state& asteroid);
    void accelerate(const ship_state& current, ship_state& next);
    void left(const ship_state& current, ship_state& next);
    void right(const ship_state& current, ship_state& next);
    ship_state* accelerate_node(const ship_state& current, ship_state_pool& pool);
    ship_state* left_node(const ship_state& current, ship_state_pool& pool);
    ship_state* right_node(const ship_state& current, ship_state_pool& pool);
};