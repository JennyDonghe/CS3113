#include "level2.h"
#include "utility.h"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"



extern ShaderProgram g_shader_program;
extern glm::mat4 g_view_matrix, g_projection_matrix;

#define MAP_WIDTH  9
#define MAP_HEIGHT 8
#define TILE_SIZE  1.0f
extern int playerHealth; 
static bool spotlight_on = false;
static float bed_timer = 0.0f;
static bool has_started_bed_timer = false;
static glm::vec3 cat_frozen_pos;
static bool cat_frozen = false;

static GLuint wallTex;
static GLuint floorTex;
static GLuint doorTex;
static GLuint bedTex;
static GLuint catTex;
static GLuint heartTex1;
static GLuint cattransTex;
static GLuint bookshelfTex;
static GLuint bookTex;



static constexpr int PLAYER_COLS = 6;
static constexpr int PLAYER_ROWS = 10;
static unsigned int level2_data[MAP_WIDTH * MAP_HEIGHT];

static int bedFrames[] = {54, 55, 56, 57};
static int catidleFrames[]  = { 0 , 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static int idleFrames[]  = { 0 };         // row 0, frame 0
static int rightFrames[] = { 6, 7, 8, 9, 10, 11 };   // row 1
static int upFrames[]    = { 12, 13, 14, 15, 16, 17 }; // row 2
static int downFrames[]  = { 18, 19, 20, 21, 22, 23 }; // row 3

static int catfrmes[] = {0};

static bool is_on_bed = false;
static bool play_bed_animation = false;
static bool show_book = false;
static bool near_bookshelf = false;

static int house_map[MAP_HEIGHT][MAP_WIDTH] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 4, 0, 0, 3, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 2, 1, 1, 1, 1,
};

static void flatten_map() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            level2_data[y * MAP_WIDTH + x] = house_map[y][x];
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

void Level2::initialise() {
    flatten_map();
    
   
    m_game_state.map = new Map(
        MAP_WIDTH,
        MAP_HEIGHT,
        level2_data,     // ← flattened array
        wallTex,        // ← texture
        TILE_SIZE,
        1, 1             // ← tile_count_x, tile_count_y (just use 1 if you're using separate textures)
    );


    m_game_state.next_scene_id = -1;
    m_game_state.level_complete = false;

    wallTex = Utility::load_texture("assets/wall.png");
    doorTex = Utility::load_texture("assets/door.png");
    floorTex = Utility::load_texture("assets/floor.png");
    bedTex = Utility::load_texture("assets/bed.png");
    catTex = Utility::load_texture("assets/cat_walk.png");
    heartTex1 = Utility::load_texture("assets/heart.png");
    cattransTex = Utility::load_texture("assets/TRANSITION.png");
    bookshelfTex = Utility::load_texture("assets/bookshelf.png");
    bookTex = Utility::load_texture("assets/book.png");
    
    
   
    GLuint playerTex = Utility::load_texture("assets/Player.png");
    
   
    
  
    
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
    m_game_state.enemies[0].set_ai_type(GUARD);
    m_game_state.enemies[0].set_ai_state(WALKING);
    m_game_state.enemies[0].set_texture_id(catTex);
   
    m_game_state.enemies[0].set_animation_cols(12);  // adjust if needed
    m_game_state.enemies[0].set_animation_rows(1);
    m_game_state.enemies[0].set_animation_indices(catidleFrames);
    m_game_state.enemies[0].set_is_active(true);
    m_game_state.enemies[0].set_animation_frames(12);
    m_game_state.enemies[0].set_position(glm::vec3(3.0f, -2.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(-1.0, 0.0, 0.0));
    m_game_state.enemies[0].set_speed(1.0f);
    m_game_state.enemies->set_width(2.0f);
    m_game_state.enemies->set_height(2.0f);
   
  

    m_game_state.enemy_count = 1;

    
}

void Level2::process_input(SDL_Event& event) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    glm::vec3 movement(0.0f);

    if (keys[SDL_SCANCODE_W]) movement.y =  1.0f;
    if (keys[SDL_SCANCODE_S]) movement.y = -1.0f;
    if (keys[SDL_SCANCODE_A]) movement.x = -1.0f;
    if (keys[SDL_SCANCODE_D]) movement.x =  1.0f;

    if (glm::length(movement) > 1.0f) {
        movement = glm::normalize(movement);
    }
    
    glm::vec3 player_pos = m_game_state.player->get_position();
        int tileX = static_cast<int>(round(player_pos.x));
        int tileY = static_cast<int>(-round(player_pos.y));

        is_on_bed = false;
        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            is_on_bed = (house_map[tileY][tileX] == 3);
        }

        play_bed_animation = false;
        if (is_on_bed && keys[SDL_SCANCODE_SPACE]) {
            play_bed_animation = true;
        }

    // Set animation based on direction
    if (!play_bed_animation) {
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
        static bool l_pressed_last_frame = false;

        if (keys[SDL_SCANCODE_L]) {
            if (!l_pressed_last_frame) {
                spotlight_on = !spotlight_on;
            }
            l_pressed_last_frame = true;
        } else {
            l_pressed_last_frame = false;
        }
    }
    near_bookshelf = false;
    if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
        near_bookshelf = (house_map[tileY][tileX] == 4);
    }

    if (near_bookshelf && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_R) {
        show_book = true;
    }
    if (show_book && event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
        show_book = false;
    }
    
    
    m_game_state.player->set_movement(movement);
    
}

void Level2::update(float delta_time) {
    
    glm::vec3 movement = m_game_state.player->get_movement();
    glm::vec3 pos = m_game_state.player->get_position();
    pos += movement * m_game_state.player->m_speed * delta_time;
    m_game_state.player->set_position(pos);
    
    if (play_bed_animation) {
        m_game_state.player->set_animation_indices(bedFrames);
        m_game_state.player->set_animation_frames(4);
        
        m_game_state.enemies[0].set_texture_id(cattransTex);
        m_game_state.enemies[0].set_animation_indices(catfrmes); // single frame
        m_game_state.enemies[0].set_animation_frames(1);
        m_game_state.enemies[0].set_animation_cols(2);
        m_game_state.enemies[0].set_animation_rows(1);
        m_game_state.enemies[0].set_animation_index(0);
        
        if (!cat_frozen) {
            cat_frozen_pos = m_game_state.enemies[0].get_position();
            cat_frozen = true;
        }
        m_game_state.enemies[0].set_position(cat_frozen_pos);
        
        if (bed_timer >= 0.3f) {
                m_game_state.player->set_animation_frames(1);      // only one frame
                m_game_state.player->set_animation_index(3);       // frame 57 (bedFrames[3])
            }

        if (!has_started_bed_timer) {
            bed_timer = 0.0f;  // reset timer
            has_started_bed_timer = true;
        } else {
            bed_timer += delta_time;
            if (bed_timer >= 3.0f) {
                m_game_state.next_scene_id = 3;  // switch to Level3
                m_game_state.level_complete = true;
            }
        }
    } else {
        has_started_bed_timer = false;
        bed_timer = 0.0f;
        
        cat_frozen = false;
        m_game_state.enemies[0].set_texture_id(catTex);
        m_game_state.enemies[0].set_animation_cols(12);
        m_game_state.enemies[0].set_animation_rows(1);
        m_game_state.enemies[0].set_animation_indices(catidleFrames);
        m_game_state.enemies[0].set_animation_frames(12);
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
            
            
            if (i == 0 && m_game_state.enemies[i].get_ai_type() == GUARD) {
                glm::vec3 enemy_pos = m_game_state.enemies[i].get_position();
                glm::vec3 player_pos = m_game_state.player->get_position();
                float distance = glm::distance(enemy_pos, player_pos);
                
                if (distance < 3.0f) {
                    glm::vec3 direction = glm::normalize(player_pos - enemy_pos);
                    m_game_state.enemies[i].set_movement(glm::vec3(direction.x, direction.y, 0.0f));
                } else {
                    m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
                }
            }
            
            
            
        }
        glm::vec3 player_pos = m_game_state.player->get_position();
        int tileX = static_cast<int>(round(player_pos.x));
        int tileY = static_cast<int>(-round(player_pos.y));
        
        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            if (house_map[tileY][tileX] == 2) { // 2 = door tile
                m_game_state.next_scene_id = 1; // back to level1
                m_game_state.level_complete = true;
            }
        }
    
    g_shader_program.set_use_spotlight(spotlight_on);

    if (spotlight_on) {
        g_shader_program.set_light_position_matrix(m_game_state.player->get_position());
    } else {
        g_shader_program.set_light_position_matrix(glm::vec3(-100.0f, -100.0f, 0.0f)); // Far away
    }
        
    }
    

    
    void Level2::render(ShaderProgram* program) {
        
        // Draw island
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                // Draw base (grass or water)
                GLuint baseTex;
                switch (house_map[y][x]) {
                    case 1: baseTex = wallTex; break;
                        
                    default: baseTex = floorTex; break;
                }
                
                glm::mat4 baseModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                program->set_model_matrix(baseModel);
                draw_textured_quad(program, baseTex);
                
                
                // Overlay house on top of floor
                if (house_map[y][x] == 2) {
                    glm::mat4 doorModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                    program->set_model_matrix(doorModel);
                    draw_textured_quad(program, doorTex);
                }
                if (house_map[y][x] == 3) {
                    glm::mat4 bedModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                    program->set_model_matrix(bedModel);
                    draw_textured_quad(program, bedTex);
                }
                
                if (house_map[y][x] == 4) {
                    glm::mat4 shelfModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                    program->set_model_matrix(shelfModel);
                    draw_textured_quad(program, bookshelfTex);
                }
                
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
            draw_textured_quad(program, heartTex1);
        }
        
        if (show_book) {
            program->set_view_matrix(glm::mat4(1.0f)); // UI mode
            glm::mat4 bookModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
            bookModel = glm::scale(bookModel, glm::vec3(8.0f, 7.0f, 2.0f));
            program->set_model_matrix(bookModel);
            draw_textured_quad(program, bookTex);
        }
        
        // Restore map view matrix
        program->set_view_matrix(original_view);
        
        
    }

