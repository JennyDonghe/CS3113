// map.cpp
#include "map.h"
#include "ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

Map::Map(int width, int height, unsigned int* level_data,
         GLuint texture_id, float tile_size,
         int tile_count_x, int tile_count_y)
    : m_width(width), m_height(height), m_level_data(level_data),
      m_texture_id(texture_id), m_tile_size(tile_size),
      m_tile_count_x(tile_count_x), m_tile_count_y(tile_count_y) {
    build();
}

void Map::build() {
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            int tile = m_level_data[y * m_width + x];
            if (tile == 0) continue;

            float u = (float)(tile % m_tile_count_x) / (float)m_tile_count_x;
            float v = (float)(tile / m_tile_count_x) / (float)m_tile_count_y;

            float tile_width = 1.0f / m_tile_count_x;
            float tile_height = 1.0f / m_tile_count_y;

            float x_offset = -((float)m_width / 2) * m_tile_size + m_tile_size / 2;
            float y_offset = ((float)m_height / 2) * m_tile_size - m_tile_size / 2;

            glm::vec3 offset(x * m_tile_size + x_offset, -y * m_tile_size + y_offset, 0);

            float vertices[] = {
                -0.5f * m_tile_size + offset.x, -0.5f * m_tile_size + offset.y,
                 0.5f * m_tile_size + offset.x, -0.5f * m_tile_size + offset.y,
                 0.5f * m_tile_size + offset.x,  0.5f * m_tile_size + offset.y,
                -0.5f * m_tile_size + offset.x, -0.5f * m_tile_size + offset.y,
                 0.5f * m_tile_size + offset.x,  0.5f * m_tile_size + offset.y,
                -0.5f * m_tile_size + offset.x,  0.5f * m_tile_size + offset.y
            };

            float tex_coords[] = {
                u, v + tile_height, u + tile_width, v + tile_height, u + tile_width, v,
                u, v + tile_height, u + tile_width, v, u, v
            };

            for (int i = 0; i < 12; i++) m_vertices.push_back(vertices[i]);
            for (int i = 0; i < 12; i++) m_texture_coordinates.push_back(tex_coords[i]);
        }
    }
}

void Map::render(ShaderProgram* program) {
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, m_vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, m_texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, (int)(m_vertices.size() / 2));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool Map::is_solid(glm::vec3 position, float* penetration_x, float* penetration_y) {
    float x = position.x;
    float y = -position.y;

    int tile_x = (int)(x / m_tile_size);
    int tile_y = (int)(y / m_tile_size);

    if (tile_x < 0 || tile_x >= m_width || tile_y < 0 || tile_y >= m_height) return false;

    int tile = m_level_data[tile_y * m_width + tile_x];
    if (tile != 1) return false;

    float tile_center_x = tile_x * m_tile_size + m_tile_size / 2 - ((float)m_width / 2);
    float tile_center_y = -tile_y * m_tile_size - m_tile_size / 2 + ((float)m_height / 2);

    *penetration_x = fabs(position.x - tile_center_x) - m_tile_size / 2;
    *penetration_y = fabs(position.y - tile_center_y) - m_tile_size / 2;

    return true;
}
