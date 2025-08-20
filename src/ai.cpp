#include "ai.hpp"
#include "ship.hpp"
#include "utils.hpp"
#include "constants.hpp"

ship_trajectory::ship_trajectory(int layers)
    : layers(layers),
        pool((std::pow(num_branches, layers) - 1) / (num_branches-1))
        {}

int ship_trajectory::best_move(float ship_x, float ship_y, float ship_vx, float ship_vy, float ship_angle, const asteroid_trajectory& asteroid_trajectories){
    // Calculates the move which results in the maximum possible minimum distance to an asteroid along all trajectories
    pool.reset();

    int num_asteroids = asteroid_trajectories.num_asteroids;

    ship_state* current_ship = pool.get_next();
    current_ship->pos[0] = ship_x;
    current_ship->pos[1] = ship_y;
    current_ship->vel[0] = ship_vx;
    current_ship->vel[1] = ship_vy;
    current_ship->angle  = ship_angle;

    construct_tree(pool, *current_ship, asteroid_trajectories, num_asteroids, layers);

    float max_score = 0.0f;
    int best_move = 0;
    for (int i = 0; i < num_branches; i++) {
        if (current_ship->next[i]) {
            float score = score_trajectory(current_ship->next[i], 1, layers);
            if (score >= max_score) {
                best_move = i;
                max_score = score;
            }
        }
    }
    return best_move;
}

void ship_trajectory::construct_tree(ship_state_pool& pool, ship_state& state, const asteroid_trajectory& asteroid_trajectories, int num_asteroids, int layers, int layer){
    // Creates a tree representing all possible trajectories using ship states from the pool
    state.distance = dist_to_collision(&state, &asteroid_trajectories.trajectories[layer * num_asteroids], num_asteroids);
    if ((state.distance > 0) && (layer+1 < layers)){
        state.next[0] = accelerate_node(state, pool);
        construct_tree(pool, *state.next[0], asteroid_trajectories, num_asteroids, layers, layer+1);
        state.next[1] = left_node(state, pool);
        construct_tree(pool, *state.next[1], asteroid_trajectories, num_asteroids, layers, layer+1);
        state.next[2] = right_node(state, pool);
        construct_tree(pool, *state.next[2], asteroid_trajectories, num_asteroids, layers, layer+1);
    } else {
        state.next[0] = state.next[1] = state.next[2] = nullptr;
    }
}

float ship_trajectory::score_trajectory(const ship_state* state, int layer, int layers){
    // Returns the max minimum distance to asteroids along a ships possible trajectories
    if (state->distance <= 0.0f){
        return 0.0f;
    }
    float max_child_score = 0.0f;
    bool has_child = false;
    for (int i=0; i<3; i++){
        if (state->next[i] != nullptr){
            has_child = true;
            float child_score = score_trajectory(state->next[i], layer+1, layers);
            if (child_score > max_child_score){
                max_child_score = child_score;
            }
        }
    }
    if (has_child){
        return std::min(state->distance, max_child_score);
    }else{
        return state->distance;
    }
};

float ship_trajectory::dist_to_collision(const ship_state* ship, const asteroid_state* asteroids, int num_asteroids){
    // Calculates and returns distance from ship to closest asteroid
    float min_dist = 0.0f;
    if (num_asteroids > 0){
        min_dist = dist(*ship, asteroids[0]);
    }
    for (int i=0; i<num_asteroids; i++){
        float new_dist = dist(*ship, asteroids[i]);
        if (new_dist < min_dist){
            min_dist = new_dist;
        }
    }
    return min_dist;
};

float ship_trajectory::dist(const ship_state& ship, const asteroid_state& asteroid){
    // Calculates distance between ship state and asteroid state
    float dx = std::fabs(ship.pos[0] - asteroid.pos[0]);
    float x_dist = std::min(dx, screen_width - dx);
    float dy = std::fabs(ship.pos[1] - asteroid.pos[1]);
    float y_dist = std::min(dy, screen_height - dy);
    return std::sqrt(x_dist * x_dist + y_dist * y_dist) - asteroid.rad - ship_radius;
};

void ship_trajectory::accelerate(const ship_state& current, ship_state& next){
    // Updates next ship state to be the state of the ship after accelerating for dt
    float dx = std::cos(TAU * current.angle / 360.0);
    float dy = -std::sin(TAU * current.angle / 360.0);
    next.vel[0] = current.vel[0] + dx * SHIP_THRUST * dt;
    next.vel[1] = current.vel[1] + dy * SHIP_THRUST * dt;
    float speed = std::sqrt(next.vel[0] * next.vel[0] + next.vel[1] * next.vel[1]);
    float fric_accel = FRICTION * dt;
    if (fric_accel > speed){
        next.vel[0] = 0.0;
        next.vel[1] = 0.0;
        next.pos[0] = current.pos[0];
        next.pos[1] = current.pos[1];
    }else{
        next.vel[0] -= next.vel[0] * FRICTION * dt / speed;
        next.vel[1] -= next.vel[1] * FRICTION * dt / speed;
        next.pos[0] = wrap(current.pos[0] + next.vel[0] * dt, screen_width);
        next.pos[1] = wrap(current.pos[1] + next.vel[1] * dt, screen_height);
    }
    next.angle = current.angle;
}

ship_state* ship_trajectory::accelerate_node(const ship_state& current, ship_state_pool& pool){
    // Updates next ship state from pool to be the state of the ship after accelerating for dt
    ship_state* next_node = pool.get_next();
    accelerate(current, *next_node);
    return next_node;
}

void ship_trajectory::right(const ship_state& current, ship_state& next){
    // Updates next ship state to be the state of the ship after rotating right for dt
    next.pos[0] = current.pos[0];
    next.pos[1] = current.pos[1];
    next.vel[0] = current.vel[0];
    next.vel[1] = current.vel[1];
    next.angle = current.angle;

    float speed = std::sqrt(next.vel[0] * next.vel[0] + next.vel[1] * next.vel[1]);
    float fric_accel = FRICTION * dt;
    if (fric_accel > speed){
        next.vel[0] = 0.0;
        next.vel[1] = 0.0;
        next.pos[0] = current.pos[0];
        next.pos[1] = current.pos[1];
    }else{
        next.vel[0] -= next.vel[0] * FRICTION * dt / speed;
        next.vel[1] -= next.vel[1] * FRICTION * dt / speed;
        next.pos[0] = wrap(current.pos[0] + next.vel[0] * dt, screen_width);
        next.pos[1] = wrap(current.pos[1] + next.vel[1] * dt, screen_height);
    }
    next.angle = current.angle + SHIP_ROT_SPEED * dt;
}

ship_state* ship_trajectory::right_node(const ship_state& current, ship_state_pool& pool){
    // Updates next ship state from pool to be the state of the ship after rotating right for dt
    ship_state* next_node = pool.get_next();
    right(current, *next_node);
    return next_node;
}

void ship_trajectory::left(const ship_state& current, ship_state& next){
    // Updates next ship state to be the state of the ship after rotating left for dt
    next.pos[0] = current.pos[0];
    next.pos[1] = current.pos[1];
    next.vel[0] = current.vel[0];
    next.vel[1] = current.vel[1];
    next.angle = current.angle;

    float speed = std::sqrt(next.vel[0] * next.vel[0] + next.vel[1] * next.vel[1]);
    float fric_accel = FRICTION * dt;
    if (fric_accel > speed){
        next.vel[0] = 0.0;
        next.vel[1] = 0.0;
        next.pos[0] = current.pos[0];
        next.pos[1] = current.pos[1];
    }else{
        next.vel[0] -= next.vel[0] * FRICTION * dt / speed;
        next.vel[1] -= next.vel[1] * FRICTION * dt / speed;
        next.pos[0] = wrap(current.pos[0] + next.vel[0] * dt, screen_width);
        next.pos[1] = wrap(current.pos[1] + next.vel[1] * dt, screen_height);
    }
    next.angle = current.angle - SHIP_ROT_SPEED * dt;
}

ship_state* ship_trajectory::left_node(const ship_state& current, ship_state_pool& pool){
    // Updates next ship state from pool to be the state of the ship after rotating left for dt
    ship_state* next_node = pool.get_next();
    left(current, *next_node);
    return next_node;
}