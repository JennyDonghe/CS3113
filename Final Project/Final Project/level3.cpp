#include "level3.h"
#include "utility.h"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"


#define MAP_WIDTH  9
#define MAP_HEIGHT 8
#define TILE_SIZE  1.0f
extern int playerHealth;
static int evilcat_health = 5;
static float hurt_cooldown = 0.0f;

static bool space_was_pressed_last_frame = false;

static GLuint wallsTex;
static GLuint floorsTex;
static GLuint evilcatTex;
static GLuint heartTex2;

static Mix_Chunk* attack_sfx = nullptr;
static Mix_Chunk* cathiss_sfx = nullptr;

static constexpr int PLAYER_COLS = 6;
static constexpr int PLAYER_ROWS = 10;
static unsigned int level3_data[MAP_WIDTH * MAP_HEIGHT];

static int attackFrames[] = {42, 43, 44, 45};


static int idleFrames[]  = { 0 };         // row 0, frame 0
static int rightFrames[] = { 6, 7, 8, 9, 10, 11 };   // row 1
static int upFrames[]    = { 12, 13, 14, 15, 16, 17 }; // row 2
static int downFrames[]  = { 18, 19, 20, 21, 22, 23 }; // row 3

static bool play_attack_animation = false;
static bool attack_triggered = false;



static int dream_map[MAP_HEIGHT][MAP_WIDTH] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static void flatten_map() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            level3_data[y * MAP_WIDTH + x] = dream_map[y][x];
        }
    }
}

static void draw_textured_quad(ShaderProgram* program, GLuint texture) {
    float vertices[] = {
        -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f
    };

    float tex_coords[] = {
        0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,
        0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f
    };

    glBindTexture(GL_TEXTURE_2D, texture);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Level3::initialise() {
    flatten_map();
    
   
    m_game_state.map = new Map(
        MAP_WIDTH,
        MAP_HEIGHT,
        level3_data,     // ← flattened array
        wallsTex,        // ← texture
        TILE_SIZE,
        1, 1             // ← tile_count_x, tile_count_y (just use 1 if you're using separate textures)
    );


    m_game_state.next_scene_id = -1;
    m_game_state.level_complete = false;
    
    evilcatTex = Utility::load_texture("assets/evilcat.png");
    wallsTex = Utility::load_texture("assets/walls.png");
    
    floorsTex = Utility::load_texture("assets/floors.png");
   
    heartTex2 = Utility::load_texture("assets/heart.png");
    
    
   
    GLuint playerTex = Utility::load_texture("assets/Player.png");
    
   
    attack_sfx = Mix_LoadWAV("assets/attack.mp3");
    cathiss_sfx = Mix_LoadWAV("assets/cathurt.wav");
    
    m_game_state.player = new Entity();
    m_game_state.player->set_entity_type(PLAYER);
    m_game_state.player->set_texture_id(playerTex);
    m_game_state.player->set_animation_cols(PLAYER_COLS);
    m_game_state.player->set_animation_rows(PLAYER_ROWS);
    m_game_state.player->set_animation_indices(idleFrames);
    m_game_state.player->set_animation_frames(1);
    m_game_state.player->set_position(glm::vec3(5, -2, 0));
    m_game_state.player->set_movement(glm::vec3(0));
    m_game_state.player->set_speed(3.0f);
    m_game_state.player->set_acceleration(glm::vec3(0));
    m_game_state.player->set_width(1.0f);
    m_game_state.player->set_height(1.0f);
    
    m_game_state.enemies = new Entity[1];
    m_game_state.enemies[0].set_entity_type(ENEMY);
    m_game_state.enemies[0].set_ai_type(FLYER);
    m_game_state.enemies[0].set_ai_state(WALKING);
    m_game_state.enemies[0].set_texture_id(evilcatTex);
    m_game_state.enemies[0].set_is_active(true);
    m_game_state.enemies[0].set_position(glm::vec3(3.0f, -5.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(-1.0, 0.0, 0.0));
    m_game_state.enemies[0].set_speed(1.0f);
    m_game_state.enemies->set_width(3.0f);
    m_game_state.enemies->set_height(3.0f);
    m_game_state.enemies[0].set_patrol_bounds(3.0f, 6.0f);
   
  

    m_game_state.enemy_count = 1;

    
}

void Level3::process_input(SDL_Event& event) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    glm::vec3 movement(0.0f);

    if (keys[SDL_SCANCODE_W]) movement.y =  1.0f;
    if (keys[SDL_SCANCODE_S]) movement.y = -1.0f;
    if (keys[SDL_SCANCODE_A]) movement.x = -1.0f;
    if (keys[SDL_SCANCODE_D]) movement.x =  1.0f;

    if (glm::length(movement) > 1.0f) {
        movement = glm::normalize(movement);
    }
    
    
        

    if (keys[SDL_SCANCODE_SPACE] && !space_was_pressed_last_frame) {
        play_attack_animation = true;
        attack_triggered = true; // set flag for update()
        if (attack_sfx) Mix_PlayChannel(-1, attack_sfx, 0);
    } else {
        play_attack_animation = false;
    }
    

    // Set animation based on direction
    
        if (movement.x > 0.0f) {
            m_game_state.player->set_animation_indices(rightFrames);
            m_game_state.player->set_animation_frames(6);
            m_game_state.player->set_flip(false);
        } else if (movement.x < 0.0f) {
            m_game_state.player->set_animation_indices(rightFrames);
            m_game_state.player->set_animation_frames(6);
            m_game_state.player->set_flip(true);
        } else if (movement.y > 0.0f) {
            m_game_state.player->set_animation_indices(upFrames);
            m_game_state.player->set_animation_frames(6);
            m_game_state.player->set_flip(false);
        } else if (movement.y < 0.0f) {
            m_game_state.player->set_animation_indices(downFrames);
            m_game_state.player->set_animation_frames(6);
            m_game_state.player->set_flip(false);
        } else {
            m_game_state.player->set_animation_indices(idleFrames);
            m_game_state.player->set_animation_frames(1);
            m_game_state.player->set_flip(false);
        }
        
    
    
    
    m_game_state.player->set_movement(movement);
    
}

void Level3::update(float delta_time) {
    if (hurt_cooldown > 0.0f) {
        hurt_cooldown -= delta_time;
    }
    
    glm::vec3 movement = m_game_state.player->get_movement();
    glm::vec3 pos = m_game_state.player->get_position();
    pos += movement * m_game_state.player->m_speed * delta_time;
    m_game_state.player->set_position(pos);
    
    if (play_attack_animation) {
        m_game_state.player->set_animation_indices(attackFrames);
        m_game_state.player->set_animation_frames(4); // or however many frames you have
        
       
        }
        
        
        
        // Update player logic (includes animation timing)
        m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, m_game_state.enemy_count, m_game_state.map);
        
    glm::vec3 clamped_pos = m_game_state.player->get_position();
    float half_width = 0.5f;
    float half_height = 0.5f;

    clamped_pos.x = glm::clamp(clamped_pos.x, half_width, MAP_WIDTH - half_width);
    clamped_pos.y = glm::clamp(clamped_pos.y, -(MAP_HEIGHT - half_height), -half_height);

    m_game_state.player->set_position(clamped_pos);
        
    for (int i = 0; i < m_game_state.enemy_count; i++) {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, m_game_state.enemies, m_game_state.enemy_count, m_game_state.map);
        if (m_game_state.player->check_collision(&m_game_state.enemies[i])) {
            if (hurt_cooldown <= 0.0f && playerHealth > 0) {
                playerHealth--;
                m_game_state.lives = playerHealth;
                hurt_cooldown = 2.0f; // 1 second cooldown between hits
            }
        }
        
        if (attack_triggered) {
            glm::vec3 player_pos = m_game_state.player->get_position();
            glm::vec3 enemy_pos = m_game_state.enemies[i].get_position();
            float attack_range = 1.2f;

            if (glm::distance(player_pos, enemy_pos) <= attack_range) {
                evilcat_health--;
                if (attack_sfx) Mix_PlayChannel(-1, cathiss_sfx, 0);
                if (evilcat_health <= 0) {
                    m_game_state.enemies[i].set_is_active(false);
                    m_game_state.level_complete = true;
                }
            }
            attack_triggered = false;
        }
    }
   
            
}
    
    
        void Level3::render(ShaderProgram* program) {
        
        // Draw island
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                // Draw base (grass or water)
                GLuint baseTex;
                switch (dream_map[y][x]) {
                    case 1: baseTex = wallsTex; break;
                        
                    default: baseTex = floorsTex; break;
                }
                
                glm::mat4 baseModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                program->set_model_matrix(baseModel);
                draw_textured_quad(program, baseTex);
                
                
                // Overlay house on top of floo
                
                
            }
        }
        
        // Draw player
        m_game_state.player->render(program);
        
        for (int i = 0; i < m_game_state.enemy_count; i++) {
            m_game_state.enemies[i].render(program);
        }
        
        glm::mat4 original_view = glm::translate(glm::mat4(1.0f), -m_game_state.player->get_position()); // Your current camera setup
        program->set_view_matrix(glm::mat4(1.0f));  // Reset camera to identity (screen-space)
        for (int i = 0; i < playerHealth; i++) {
            glm::mat4 uiModel = glm::mat4(1.0f);
            uiModel = glm::translate(uiModel, glm::vec3(-4.5f + i * 0.6f, 3.5f, 0.0f));
            uiModel = glm::scale(uiModel, glm::vec3(0.5f, 0.5f, 1.0f));
            program->set_model_matrix(uiModel);
            draw_textured_quad(program, heartTex2);
        }
        
        
        // Restore map view matrix
        program->set_view_matrix(original_view);
        
        
    }



