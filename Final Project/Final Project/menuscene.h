#ifndef MENUSCENE_H
#define MENUSCENE_H

#include "scene.h"
#include "utility.h"

class MenuScene : public Scene {
public:
    GLuint background_texture;
    bool start_game = false;

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
    void process_input(SDL_Event& event) override;
};

#endif
