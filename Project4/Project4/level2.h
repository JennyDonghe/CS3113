#ifndef LEVEL2_H
#define LEVEL2_H

#include "scene.h"
#include "map.h"
#include "Entity.h"

class Level2 : public Scene {
public:
    GameState state;

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
    void process_input(SDL_Event& event) override;
};

#endif 
