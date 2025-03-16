#ifndef __RANDOM_BLOCKS_AGENT__H
#define __RANDOM_BLOCKS_AGENT__H 

#include <ctime>
#include <cstdlib> 

#include "enviro.h"

using namespace enviro;

class random_blocksController : public Process, public AgentInterface {

    public:
    random_blocksController() : Process(), AgentInterface() {
        // Seed once when object is created
        std::srand(std::time(0)); 
    }

    // If detect block is eaten, update new random block position
    void init() {
        // Notice collision with the snake
        notice_collisions_with("snake_head", [&](Event& e) {
            // Emit event when the snake eats a block
            emit(Event("eaten", { { "x", x()}, {"y", y() }}));
            // Update new block position
            generateRandomPosition();          
            teleport(new_x, new_y, 0);
        });
    }
    //function to generate food new_x and new_y value within boundary range
    void generateRandomPosition() {
        new_x = -330 + (std::rand() % (330 - (-30) + 1)); 
        new_y = -180 + (std::rand() % (180 - (-180) + 1)); 
    }    

    void start() {}

    void update() {}

    void stop() {}

    private:
    int new_x;
    int new_y;

};

class random_blocks : public Agent {
    public:
    random_blocks(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    random_blocksController c;
};

DECLARE_INTERFACE(random_blocks)

#endif