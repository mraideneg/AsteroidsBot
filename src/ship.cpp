#include <cassert>
#include "ship.hpp"
#include "constants.hpp"

ship_state::ship_state()
    : vel{0.0f, 0.0f}, pos{0.0f, 0.0f}, angle(0.0f), distance(0.0f) {
    for (int i = 0; i < num_branches; i++) {
        next[i] = nullptr;
    }
}

ship_state_pool::ship_state_pool(int num_nodes)
    : pool(num_nodes),
        index(0),
        num_nodes(num_nodes)
        {}

ship_state* ship_state_pool::get_next(){
        // Returns the address of the next ship state object to be used  in the pool
        assert(index < this->num_nodes && "Ship state pool overflow!");
        return &this->pool[this->index++];
    }

void ship_state_pool::reset(){
        // Resets the pool of ship state objects
        this->index = 0;
    }