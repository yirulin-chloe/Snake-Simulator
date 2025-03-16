#ifndef __SNAKE_HEAD_AGENT__H
#define __SNAKE_HEAD_AGENT__H 

#include "enviro.h"
#include <set>

using namespace enviro;

class snake_headController : public Process, public AgentInterface {
    public:
    snake_headController() : Process(), AgentInterface() {
        score = 0;
        step_size = 25;  // Movement step size
        move_interval = 0.01;  // Move every _ seconds
        last_move_time = 0;
        UP = false;
        DOWN = false;
        LEFT = false;
        RIGHT = true;  // Start moving right
        is_food_eaten = false;
        pending_segment = true;  // Flag for new segment
        current_direction = "right";  // Initial direction
    }
    
    void init() {
        //only want to move vertical or horizontal
        prevent_rotation();
        //add head to list
        body_segments.push_back(this->agent);

        // Watch for initial body segments
        watch("snake_body_created", [this](Event e) {
            int segment_id = e.value()["id"];
            if (agent_exists(segment_id)) {
                Agent& segment = find_agent(segment_id);
                body_segments.push_back(&segment);
                pending_segment = false; // indicate finished updating 
            }
            
        });

        //set flag to let update function know
        watch("eaten", [this](Event e) {
            food_position.x = e.value()["x"]; // for debug
            food_position.y = e.value()["y"]; // for debug
            is_food_eaten = true;
        });

        //control the movement of snake: can't go in reverse direction
        watch("keydown", [&](Event &e) {
            std::string k = e.value()["key"];
            // Prevent reverse movement based on current direction
            if (k == "w" && current_direction != "down") { 
                UP = true; DOWN = false; LEFT = false; RIGHT = false; 
                current_direction = "up";
            } else if (k == "s" && current_direction != "up") { 
                DOWN = true; UP = false; LEFT = false; RIGHT = false; 
                current_direction = "down";
            } else if (k == "a" && current_direction != "right") { 
                LEFT = true; RIGHT = false; UP = false; DOWN = false; 
                current_direction = "left";
            } else if (k == "d" && current_direction != "left") { 
                RIGHT = true; LEFT = false; UP = false; DOWN = false; 
                current_direction = "right";
            }
        });
        //decorate eyes on snake head 
        decorate(R"(<g>
            <circle cx=-5 cy=-3 r=2 style='fill:black'></circle>
            <circle cx=5 cy=-3 r=2 style='fill:black'></circle></g>)");
    }


    //if food is eaten, add a new body to snake
    void food_eaten() {
        pending_segment = true; //set flag for update function
        Agent* last_segment = body_segments.back();
        json style = {{"fill", "blue"}, {"stroke", "black"}};
        Agent& new_segment = add_agent("snake_body", last_segment->x(), last_segment->y(), 0, style);
        body_segments.push_back(&new_segment);
        pending_segment = false; // inidicate finished updating 
    }

    void start() {}

    void update() {
        //if food is eaten, we need to add one body to it and update new score
        if (is_food_eaten) {
            food_eaten();
            //update score
            score += 10;
            emit(Event("update_score", {{ "new_score", score}}));
            //wait for any pending body to be added if needed
            while(pending_segment) {
                continue;
            }
            //indicating finishing adding new block
            is_food_eaten = false; 
        }

        //control snake to move every time interval
        double current_time = (double)(clock()) / CLOCKS_PER_SEC;
        if (current_time - last_move_time >= move_interval) {
            move_snake();
            last_move_time = current_time;

            //when moving, checking if step is valid.
            if (!check_valid_state()) {
                std::cout << "Game Over: Snake hit boundary or formed a loop!\n";
            }
        }
    }

    void stop() {}

    std::vector<Agent*> body_segments;
    cpVect food_position;
    int score;
    double step_size, move_interval, last_move_time;
    bool is_food_eaten;
    bool UP, DOWN, LEFT, RIGHT;
    bool pending_segment;
    std::string current_direction;

    private:

    //move snake head/body to define location.
    void move_snake() {
        //store prev body location for moving snake body
        std::vector<std::pair<double, double>> prev_positions(body_segments.size());
        for (size_t i = 0; i < body_segments.size(); i++) {
            prev_positions[i] = {body_segments[i]->x(), body_segments[i]->y()};
        }

        double new_x = x();
        double new_y = y();
        if (RIGHT) { new_x += step_size; }
        else if (LEFT) { new_x -= step_size; }
        else if (UP) { new_y -= step_size; }
        else if (DOWN) { new_y += step_size; }
        //move head to new location
        body_segments[0]->teleport(new_x, new_y, 0);
        //move body to prev block location
        for (size_t i = 1; i < body_segments.size(); i++) {
            body_segments[i]->teleport(prev_positions[i - 1].first, prev_positions[i - 1].second, 0);
        }

    }
    // end game if snake head hit its body or hit boundary
    bool check_valid_state() {
        double head_x = x();
        double head_y = y();
        //check if snake moves over boundary
        if (head_x < -340 || head_x > 340 || head_y < -190 || head_y > 190) {
            std::cout << "GAME OVER: SNAKE HIT BOUNDARY!!!!!!!! \n";
            exit(0); // Exit
            return false;
        }
        //check if head hit body
        notice_collisions_with("snake_body", [&](Event& e) { 
            int collision_body_id = e.value()["id"];
            //ignore collision with first body of snake. 
            if (body_segments.size() > 1 && collision_body_id == body_segments[1]->get_id()) {
                //std::cout << "Ignoring collision with first body segment at (" 
                //    << body_segments[1]->x() << ", " << body_segments[1]->y() << ")\n";
                
            } else {
                //check if collistion happens with snake and the bodies(except the 1st one)
                if (agent_exists(collision_body_id)) {
                    Agent& collision = find_agent(collision_body_id);          
                    // for debuging collision
                    //std::cout << "Collision with head: " << x() << ", " << y() << std::endl;
                    //std::cout << "Collision with body: " << collision.x() << ", " << collision.y() << std::endl;
                    printf("\nGAME OVER: SNAKE HEAD HITS ITS BODY!\n");
                    exit(0);
                }
            }
        });
        return true;
    }
};

class snake_head : public Agent {
    public:
    snake_head(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    snake_headController c;
};

DECLARE_INTERFACE(snake_head)

#endif