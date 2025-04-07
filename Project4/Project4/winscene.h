#ifndef WINSCENE_H
#define WINSCENE_H

#include "scene.h"
#include "utility.h"
#include <SDL_mixer.h>

class WinScene : public Scene {
public:
    GLuint background_texture;

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
    void process_input(SDL_Event& event) override;
};

#endif 

