#ifndef MAP_H
#define MAP_H

#include <vector>
#include "ShaderProgram.h"
#include "glm/glm.hpp"

class Map {
public:
    Map(int width, int height, unsigned int* level_data,
        GLuint texture_id, float tile_size,
        int tile_count_x, int tile_count_y);

    void render(ShaderProgram* program);
    bool is_solid(glm::vec3 position, float* penetration_x, float* penetration_y);
    void set_goal_texture(GLuint texture_id);

private:
    int m_width;
    int m_height;
    unsigned int* m_level_data;

    GLuint m_texture_id;
    GLuint m_goal_texture_id;
    float m_tile_size;
    int m_tile_count_x;
    int m_tile_count_y;

    std::vector<float> m_vertices;
    std::vector<float> m_texture_coordinates;

    float m_left_bound;
    float m_right_bound;
    float m_top_bound;
    float m_bottom_bound;

    void build();
};

#endif 
