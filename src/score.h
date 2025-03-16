#ifndef __SCORE_AGENT__H
#define __SCORE_AGENT__H 

#include "enviro.h"

using namespace enviro;

class scoreController : public Process, public AgentInterface {

    public:
    scoreController() : Process(), AgentInterface() {}

    void init() {
        watch("update_score", [this](Event e) {
            int score = e.value()["new_score"];
            label("Score: " + std::to_string(score), -5, 10);
        });

    }
    void start() {}
    void update() {}
    void stop() {}

};

class score : public Agent {
    public:
    score(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    scoreController c;
};

DECLARE_INTERFACE(score)

#endif