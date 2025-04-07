#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define NUMBER_OF_TEXTURES 1
#define LEVEL_OF_DETAIL    0
#define TEXTURE_BORDER     0

#include "utility.h"
#include <SDL_image.h>
#include "stb_image.h"
#include <cassert>
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"

GLuint Utility::load_texture(const char* filepath) {
    int width, height, num_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &num_components, STBI_rgb_alpha);

    if (image == nullptr) {
        LOG("ERROR: Failed to load image at " << filepath);
        assert(false);
    }

    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);
    return texture_id;
}

void Utility::draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
                        float size, float spacing, glm::vec3 position) {
    float character_width = 1.0f / FONTBANK_SIZE;
    float character_height = 1.0f / FONTBANK_SIZE;

    std::vector<float> vertices;
    std::vector<float> tex_coords;

    for (int i = 0; i < text.length(); i++) {
        int char_index = (int)text[i];
        float x_offset = (size + spacing) * i;

        float u = (float)(char_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v = (float)(char_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        vertices.insert(vertices.end(), {
            x_offset + (-0.5f * size),  0.5f * size,
            x_offset + (-0.5f * size), -0.5f * size,
            x_offset + ( 0.5f * size),  0.5f * size,
            x_offset + ( 0.5f * size), -0.5f * size,
            x_offset + ( 0.5f * size),  0.5f * size,
            x_offset + (-0.5f * size), -0.5f * size
        });

        tex_coords.insert(tex_coords.end(), {
            u, v,
            u, v + character_height,
            u + character_width, v,
            u + character_width, v + character_height,
            u + character_width, v,
            u, v + character_height
        });
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    program->set_model_matrix(model_matrix);

    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.length() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
