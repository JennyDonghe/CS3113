/**
* Author: [JennyDong]
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#include "level2.h"
#include "utility.h"

#define LEVEL2_WIDTH 14
#define LEVEL2_HEIGHT 8

unsigned int level2_data[] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1
};

void Level2::initialise() {
    m_game_state.next_scene_id = -1;
    m_game_state.level_complete = false;

    GLuint map_texture_id = Utility::load_texture("assets/platform.png");
    

    m_game_state.map = new Map(LEVEL2_WIDTH, LEVEL2_HEIGHT, level2_data, map_texture_id, 1.0f, 1, 1);
   

    // Player
    GLuint player_texture_id = Utility::load_texture("assets/WALK.png");
    m_game_state.player = new Entity();
    m_game_state.player->set_entity_type(PLAYER);
    m_game_state.player->set_texture_id(player_texture_id);
    int walk_animation[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };  
    m_game_state.player->set_animation_indices(walk_animation);
    m_game_state.player->set_animation_frames(12);
    m_game_state.player->set_animation_cols(12);
    m_game_state.player->set_animation_rows(1);
    m_game_state.player->set_position(glm::vec3(2.0f, 0.0f, 0.0f));
    m_game_state.player->set_movement(glm::vec3(0));
    m_game_state.player->set_speed(2.5f);
    m_game_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.player->set_height(0.8f);
    m_game_state.player->set_width(0.8f);
    m_game_state.player->set_jumping_power(7.0f);

   
    GLuint bird_texture_id = Utility::load_texture("assets/bird.png");
    m_game_state.enemies = new Entity[1];
    m_game_state.enemies[0].set_entity_type(ENEMY);
    m_game_state.enemies[0].set_ai_type(FLYER);
    m_game_state.enemies[0].set_ai_state(WALKING);
    m_game_state.enemies[0].set_texture_id(bird_texture_id);
    m_game_state.enemies[0].set_position(glm::vec3(3.0f, -1.1f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(1.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_speed(1.5f);
    m_game_state.enemies[0].set_patrol_bounds(4.5f, 9.0f);
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f));

    m_game_state.enemy_count = 1;
}

void Level2::update(float delta_time) {
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, m_game_state.enemy_count, m_game_state.map);

    for (int i = 0; i < m_game_state.enemy_count; i++) {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, m_game_state.enemies, m_game_state.enemy_count, m_game_state.map);
        if (m_game_state.player->check_collision(&m_game_state.enemies[i])) {
            m_game_state.lives--;
            if (m_game_state.lives <= 0) return;
            m_game_state.player->set_position(glm::vec3(2.0f, 0.0f, 0.0f));
        }
    }

    if(m_game_state.player->get_position().y < -10.0f){
        m_game_state.level_complete = true;
    }

}

void Level2::render(ShaderProgram* program) {
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < m_game_state.enemy_count; i++) {
        m_game_state.enemies[i].render(program);
    }
}

void Level2::process_input(SDL_Event& event) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    m_game_state.player->set_movement(glm::vec3(0));

    if (keys[SDL_SCANCODE_A]) {
        m_game_state.player->move_left();
    } else if (keys[SDL_SCANCODE_D]) {
        m_game_state.player->move_right();
    }
    if (keys[SDL_SCANCODE_W]) {
        if (m_game_state.player->get_collided_bottom()) {
            m_game_state.player->jump();
        }
    }
}
