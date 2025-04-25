#ifndef ENTITY_H
#define ENTITY_H

#include "glm/glm.hpp"
#include "ShaderProgram.h"
#include "map.h"

enum EntityType { PLAYER, ENEMY };
enum AIType { WALKER, GUARD, FLYER };

enum AIState { IDLE, WALKING };

class Entity {
private:
    
    float m_animation_time = 0.0f; // how long current frame has been displayed
    float m_animation_frame_duration = 0.25f; // seconds per frame
    int m_current_frame = 0;
    bool m_flip = false;


public:
    Entity();

    EntityType m_entity_type;
    AIType m_ai_type;
    AIState m_ai_state;

    glm::vec3 m_position;
    glm::vec3 m_movement;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    float m_speed;

    float m_width = 1;
    float m_height = 1;

    GLuint m_texture_id;

    glm::mat4 m_model_matrix;

    int* m_animation_indices = nullptr;
    int m_animation_frames = 0;
    int m_animation_index = 0;
   
    int m_animation_cols = 0;
    int m_animation_rows = 0;

    bool m_is_active = true;

    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    bool m_is_jumping = false;
    float m_jumping_power = 0;

    float m_min_x = 0;
    float m_max_x = 0;

    void update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map);
    void render(ShaderProgram* program);
    bool check_collision(Entity* other) const;

    void check_collision_y(Map* map);
    void check_collision_x(Map* map);

    void ai_activate(Entity* player);

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);

    void move_left() { m_movement.x = -1.0f; }
    void move_right() { m_movement.x = 1.0f; }
    void jump() { m_is_jumping = true; }

    void set_position(glm::vec3 position) { m_position = position; }
    glm::vec3 get_position() const { return m_position; }

    void set_movement(glm::vec3 movement) { m_movement = movement; }
    void set_speed(float speed) { m_speed = speed; }
    void set_acceleration(glm::vec3 acceleration) { m_acceleration = acceleration; }

    void set_texture_id(GLuint texture_id) { m_texture_id = texture_id; }
    void set_height(float height) { m_height = height; }
    void set_width(float width) { m_width = width; }

    void set_animation_frames(int frames) { m_animation_frames = frames; }
    void set_animation_index(int index) { m_animation_index = index; }
    void set_animation_indices(int* indices) { m_animation_indices = indices; }
    void set_animation_cols(int cols) { m_animation_cols = cols; }
    void set_animation_rows(int rows) { m_animation_rows = rows; }

    void set_entity_type(EntityType type) { m_entity_type = type; }
    
    void set_ai_type(AIType type) { m_ai_type = type; }
    void set_ai_state(AIState state) { m_ai_state = state; }
   
    void set_jumping_power(float power) { m_jumping_power = power; }
    AIType get_ai_type() const { return m_ai_type; }
    bool get_collided_bottom() const { return m_collided_bottom; }

    void set_patrol_bounds(float min_x, float max_x);

    glm::ivec2 get_tile_position_below(float tile_size) const {
        int tile_x = (int)(m_position.x / tile_size);
        int tile_y = (int)((m_position.y - (m_height / 2)) / tile_size);
        return glm::ivec2(tile_x, tile_y);
    }
    void ai_guard(Entity *player);
  
    void request_jump();
    bool get_jump_requested() const;
    void clear_jump_request();
    void update_animation(float delta_time);
    void set_flip(bool flip) { m_flip = flip; }
    bool get_flip() const { return m_flip; }
    glm::vec3 get_movement() const { return m_movement; }
    void set_is_active(bool value) { m_is_active = value; }
};

#endif
