/**
* Author: [JennyDong]
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

Entity::Entity() {}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index) {
    float u = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v = (float)(index / m_animation_cols) / (float)m_animation_rows;

    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;
    float tex_coords[] = {
        u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v
    };

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool Entity::check_collision(Entity* other) const {
    if (!m_is_active || !other->m_is_active) return false;

    float xdist = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float ydist = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);
    return (xdist < 0.0f && ydist < 0.0f);
}

void Entity::check_collision_y(Map* map) {
    float penetration_x, penetration_y;
    glm::vec3 top = m_position + glm::vec3(0, m_height / 2, 0);
    glm::vec3 bottom = m_position - glm::vec3(0, m_height / 2, 0);

    if (m_velocity.y > 0) {
        if (map->is_solid(top, &penetration_x, &penetration_y)) {
            //m_position.y -= penetration_y;
            m_velocity.y = 0;
            m_collided_top = true;
        }
    } else if (m_velocity.y < 0) {
        if (map->is_solid(bottom, &penetration_x, &penetration_y)) {
            //m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
        }
    }
}

void Entity::check_collision_x(Map* map) {
    float penetration_x, penetration_y;
    glm::vec3 left = m_position - glm::vec3(m_width / 2, 0, 0);
    glm::vec3 right = m_position + glm::vec3(m_width / 2, 0, 0);

    if (m_velocity.x < 0) {
        if (map->is_solid(left, &penetration_x, &penetration_y)) {
            //m_position.x += penetration_x;
            m_velocity.x = 0;
            m_collided_left = true;
        }
    } else if (m_velocity.x > 0) {
        if (map->is_solid(right, &penetration_x, &penetration_y)) {
            //m_position.x -= penetration_x;
            m_velocity.x = 0;
            m_collided_right = true;
        }
    }
}

void Entity::ai_activate(Entity* player) {
    switch (m_ai_type) {
        case WALKER:
            if (m_ai_state == WALKING) {
                // Reverse direction if we've gone past a bound
                if (m_position.x <= m_min_x && m_movement.x < 0) {
                    m_position.x = m_min_x;
                    m_movement.x = 1.0f;
                } else if (m_position.x >= m_max_x && m_movement.x > 0) {
                    m_position.x = m_max_x;
                    m_movement.x = -1.0f;
                }
            }
            break;
        case GUARD:
            ai_guard(player);
            break;
        case FLYER:
            if (m_ai_state == WALKING) {
                if (m_position.x < m_min_x) {
                    m_movement.x = 1.0f;
                } else if (m_position.x > m_max_x) {
                    m_movement.x = -1.0f;
                }
            }
            break;
        default:
            break;
    }
}

void Entity::update(float delta_time, Entity* player, Entity* enemies, int enemy_count, Map* map) {
    if (!m_is_active) return;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    // Only run AI logic if this is an enemy and player is not itself
    if (m_entity_type == ENEMY && player != this && player != nullptr) {
        ai_activate(player);
    }

    update_animation(delta_time);

    m_velocity.x = m_movement.x * m_speed;
    m_velocity.y = m_movement.y * m_speed;
    m_velocity += m_acceleration * delta_time;

    m_position.y += m_velocity.y * delta_time;
    check_collision_y(map);

    m_position.x += m_velocity.x * delta_time;
    check_collision_x(map);

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    if (m_flip) {
        m_model_matrix = glm::scale(m_model_matrix, glm::vec3(-1.0f, 1.0f, 1.0f));
    }
}



void Entity::render(ShaderProgram* program) {
    if (!m_is_active) return;
    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != nullptr) {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_texture_id);

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
}

void Entity::set_patrol_bounds(float min_x, float max_x) {
    m_min_x = min_x;
    m_max_x = max_x;
}

void Entity::ai_guard(Entity *player) {
    switch (m_ai_state) {
        case IDLE:
            if (glm::distance(m_position, player->get_position()) < 3.0f) m_ai_state = WALKING;
            break;
        case WALKING: {
            glm::vec3 direction = glm::normalize(player->get_position() - m_position);
            m_movement = glm::vec3(direction.x, direction.y, 0.0f);
            break;
        }
            
        default:
            break;
    }
}

void Entity::update_animation(float delta_time) {
    if (m_animation_frames <= 1 || m_animation_indices == nullptr) return;
    m_animation_time += delta_time;
    if (m_animation_time >= 0.1f) {
        m_animation_time = 0.0f;
        m_animation_index++;
        if (m_animation_index >= m_animation_frames) m_animation_index = 0;
    }
}

