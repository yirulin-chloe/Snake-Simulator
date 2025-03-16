#ifndef __SNAKE_BODY_AGENT__H
#define __SNAKE_BODY_AGENT__H 

#include "enviro.h"

using namespace enviro;

class snake_bodyController : public Process, public AgentInterface {

    public:
    snake_bodyController() : Process(), AgentInterface() {}

    void init() {
        //it will only move vertical/horizontal
        prevent_rotation();
        //Emit an event to notify the snake_head
        int body_id = id();
        emit(Event("snake_body_created", { { "id", body_id}}));

    }
    void start() {}
    void update() {}
    void stop() {}

};

class snake_body : public Agent {
    public:
    snake_body(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }

    private:
    snake_bodyController c;
};

DECLARE_INTERFACE(snake_body)

#endif