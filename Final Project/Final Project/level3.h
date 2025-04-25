#ifndef LEVEL3_H
#define LEVEL3_H

#include "scene.h"
#include "map.h"
#include "Entity.h"

class Level3 : public Scene {
public:
    GameState state;

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
    void process_input(SDL_Event& event) override;
};

#endif 


