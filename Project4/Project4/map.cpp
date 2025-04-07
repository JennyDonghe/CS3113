/**
* Author: [JennyDong]
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "map.h"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>

Map::Map(int width, int height, unsigned int* level_data, GLuint texture_id, float tile_size, int tile_count_x, int tile_count_y)
    : m_width(width), m_height(height),
      m_level_data(level_data), m_texture_id(texture_id),
      m_tile_size(tile_size), m_tile_count_x(tile_count_x), m_tile_count_y(tile_count_y),
      m_goal_texture_id(0) {
    build();
}

void Map::set_goal_texture(GLuint texture_id) {
    m_goal_texture_id = texture_id;
}

void Map::build() {
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            int tile = m_level_data[y * m_width + x];
            if (tile == 0) continue;

            float u = (float)(tile % m_tile_count_x) / m_tile_count_x;
            float v = (float)(tile / m_tile_count_x) / m_tile_count_y;

            float tile_width = 1.0f / m_tile_count_x;
            float tile_height = 1.0f / m_tile_count_y;

            float x_offset = -(m_tile_size / 2);
            float y_offset =  (m_tile_size / 2);

            m_vertices.insert(m_vertices.end(), {
                x_offset + (m_tile_size * x), y_offset + -m_tile_size * y,
                x_offset + (m_tile_size * x), y_offset + (-m_tile_size * y) - m_tile_size,
                x_offset + (m_tile_size * x) + m_tile_size, y_offset + (-m_tile_size * y) - m_tile_size,

                x_offset + (m_tile_size * x), y_offset + -m_tile_size * y,
                x_offset + (m_tile_size * x) + m_tile_size, y_offset + (-m_tile_size * y) - m_tile_size,
                x_offset + (m_tile_size * x) + m_tile_size, y_offset + -m_tile_size * y
            });

            m_texture_coordinates.insert(m_texture_coordinates.end(), {
                u, v,
                u, v + tile_height,
                u + tile_width, v + tile_height,

                u, v,
                u + tile_width, v + tile_height,
                u + tile_width, v
            });
        }
    }

    m_left_bound   = 0 - (m_tile_size / 2);
    m_right_bound  = (m_tile_size * m_width) - (m_tile_size / 2);
    m_top_bound    = 0 + (m_tile_size / 2);
    m_bottom_bound = -(m_tile_size * m_height) + (m_tile_size / 2);
}

void Map::render(ShaderProgram* program) {
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);

    glUseProgram(program->get_program_id());

    int tile_count = (int)m_vertices.size() / 12; // 12 floats per tile (6 vertices * 2)
    for (int i = 0; i < tile_count; i++) {
        int tile_index = m_level_data[i];

        GLuint current_texture = (tile_index == 2 && m_goal_texture_id != 0) ? m_goal_texture_id : m_texture_id;

        glBindTexture(GL_TEXTURE_2D, current_texture);

        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, &m_vertices[i * 12]);
        glEnableVertexAttribArray(program->get_position_attribute());

        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, &m_texture_coordinates[i * 12]);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }
}

bool Map::is_solid(glm::vec3 position, float* penetration_x, float* penetration_y) {
    *penetration_x = 0;
    *penetration_y = 0;

    if (position.x < m_left_bound || position.x > m_right_bound ||
        position.y > m_top_bound || position.y < m_bottom_bound) {
        return false;
    }

    int tile_x = floor((position.x + (m_tile_size / 2)) / m_tile_size);
    int tile_y = -(ceil(position.y - (m_tile_size / 2)) / m_tile_size);

    if (tile_x < 0 || tile_x >= m_width || tile_y < 0 || tile_y >= m_height) return false;

    int tile = m_level_data[tile_y * m_width + tile_x];
    if (tile == 0) return false;

    float tile_center_x = tile_x * m_tile_size;
    float tile_center_y = -tile_y * m_tile_size;

    *penetration_x = (m_tile_size / 2) - fabs(position.x - tile_center_x);
    *penetration_y = (m_tile_size / 2) - fabs(position.y - tile_center_y);

    return true;
}
