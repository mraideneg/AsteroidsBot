#include <array>
#include<vector>
#include "asteroid.hpp"
#include "utils.hpp"
#include "constants.hpp"

asteroid_state::asteroid_state()
    : vel{0.0f, 0.0f}, pos{0.0f, 0.0f}, rad(0.0f) {}

asteroid_state::asteroid_state(const std::array<float, 2>& vel, const std::array<float, 2>& pos, float rad)
    : vel(vel), pos(pos), rad(rad) {}

asteroid_state::asteroid_state(const asteroid_state& other)
    : vel(other.vel), pos(other.pos), rad(other.rad) {}

asteroid_state& asteroid_state::operator=(const asteroid_state& other) {
    if (this != &other) {
        vel = other.vel;
        pos = other.pos;
        rad = other.rad;
    }
    return *this;
}

asteroid_trajectory::asteroid_trajectory(const std::vector<asteroid_state>& asteroids, int num_asteroids, int layers)
    : trajectories(num_asteroids * (layers + 1)),
        num_asteroids(num_asteroids),
        layers(layers)
    {
    for (int j=0; j<num_asteroids; j++){
        trajectories[j] = asteroids[j];
    }

    for (int i=1; i<layers+1; i++){
        for (int j=0; j<num_asteroids; j++){
            this->trajectories[i*num_asteroids + j] = this->transform(this->trajectories[(i-1) * num_asteroids + j]);
        }
    }
}

asteroid_state asteroid_trajectory::transform(const asteroid_state& asteroid){
    // Advances the asteroid along its trajectory over the next dt
    asteroid_state transformed;
    transformed.vel[0] = asteroid.vel[0];
    transformed.vel[1] = asteroid.vel[1];
    transformed.pos[0] = wrap(asteroid.pos[0]+asteroid.vel[0] * dt, screen_width);
    transformed.pos[1] = wrap(asteroid.pos[1]+asteroid.vel[1] * dt, screen_height);
    transformed.rad = asteroid.rad;
    return transformed;
}