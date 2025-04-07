/**
* Author: [JennyDong]
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "winscene.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL_mixer.h>

void WinScene::initialise() {
    background_texture = Utility::load_texture("assets/win_screen.png");
}

void WinScene::update(float delta_time) {
    
}

void WinScene::render(ShaderProgram* program) {
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::scale(model_matrix, glm::vec3(10.0f, 7.5f, 1.0f));

    program->set_view_matrix(glm::mat4(1.0f));
    program->set_model_matrix(model_matrix);
    glBindTexture(GL_TEXTURE_2D, background_texture);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };
    float tex_coords[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void WinScene::process_input(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
        
    }
}

