#ifndef LEVEL1_H
#define LEVEL1_H

#include "scene.h"

class Level1 : public Scene {
public:
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
    void process_input(SDL_Event& event) override;
    
};

#endif 
