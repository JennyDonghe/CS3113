#ifndef LOSESCENE_H
#define LOSESCENE_H

#include "scene.h"
#include "utility.h"

class LoseScene : public Scene {
public:
    GLuint background_texture;

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
    void process_input(SDL_Event& event) override;
};

#endif 
