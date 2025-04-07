#ifndef UTILITY_H
#define UTILITY_H

#include "ShaderProgram.h"
#include <string>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/glm.hpp"
#include <vector>

#define FONTBANK_SIZE 16

namespace Utility {
    GLuint load_texture(const char* filepath);
    void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
                   float size, float spacing, glm::vec3 position);
}


#endif 

