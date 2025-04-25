#include "level1.h"
#include "utility.h"
#include "glm/gtc/matrix_transform.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#define MAP_WIDTH  20
#define MAP_HEIGHT 15
#define TILE_SIZE  1.0f
extern int playerHealth;          // Max 5
int playerHealth = 5;

static GLuint grassTex;
static GLuint waterTex;
static GLuint houseTex;
static GLuint treeTex;
static GLuint farmTex;
static GLuint carrotTex;
static GLuint heartTex;
static GLuint skeletonTex;

static unsigned int level1_data[MAP_WIDTH * MAP_HEIGHT];
static Mix_Chunk* hurt_sfx = nullptr;
static Mix_Chunk* eat_sfx = nullptr;




 // column 3, row 4 (0-based index)

static int farmingFrames[] = { 36, 37, 38, 39 };  // Player farming animation
static int farmingFrameCount = 4;

bool carrotPlanted[MAP_HEIGHT][MAP_WIDTH] = { false };
bool isFarming = false;
float farmingTimer = 0.0f;
float farmingDuration = 1.0f; // 1 second animation
int farmingTileX = -1, farmingTileY = -1;

static constexpr int PLAYER_COLS = 6;
static constexpr int PLAYER_ROWS = 10;

static int idleFrames[]  = { 0 };         // row 0, frame 0
static int rightFrames[] = { 6, 7, 8, 9, 10, 11 };   // row 1
static int upFrames[]    = { 12, 13, 14, 15, 16, 17 }; // row 2
static int downFrames[]  = { 18, 19, 20, 21, 22, 23 }; // row 3

static int skeletonFrames[] = { 48, 49, 50, 51};

static int skeleton2Frames[] = {36, 37, 38, 39};


// 0 = grass, 1 = water, 2 = house
static int island_map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,0,0,3,0,0,0,0,0,0,3,3,3,0,0,1,1},
    {1,1,0,3,0,0,0,0,0,3,0,0,0,3,2,3,0,3,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,3,0,0,0,4,4,4,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,4,4,4,0,0,0,4,4,4,0,0,0,1},
    {1,0,3,0,0,0,0,0,0,3,0,0,0,4,4,4,0,0,0,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,3,0,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

static void flatten_map() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            level1_data[y * MAP_WIDTH + x] = island_map[y][x];
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

void Level1::initialise() {
    flatten_map();
    m_game_state.map = new Map(
        MAP_WIDTH,
        MAP_HEIGHT,
        level1_data,     // ← flattened array
        grassTex,        // ← texture
        TILE_SIZE,
        1, 1             // ← tile_count_x, tile_count_y (just use 1 if you're using separate textures)
    );


    m_game_state.next_scene_id = -1;
    m_game_state.level_complete = false;

    grassTex = Utility::load_texture("assets/grass-bright-1.png");
    waterTex = Utility::load_texture("assets/water.png");
    houseTex = Utility::load_texture("assets/House.png");
    treeTex = Utility::load_texture("assets/Oak_Tree.png");
    farmTex = Utility::load_texture("assets/FarmLand_Tile.png");
    carrotTex = Utility::load_texture("assets/carrot.png");
    GLuint playerTex = Utility::load_texture("assets/Player.png");
    heartTex = Utility::load_texture("assets/heart.png");
    skeletonTex = Utility::load_texture("assets/Skeleton.png");
    
    
    hurt_sfx = Mix_LoadWAV("assets/hurt.wav");
    eat_sfx = Mix_LoadWAV("assets/eat.wav");
    
    m_game_state.player = new Entity();
    m_game_state.player->set_entity_type(PLAYER);
    m_game_state.player->set_texture_id(playerTex);
    m_game_state.player->set_animation_cols(PLAYER_COLS);
    m_game_state.player->set_animation_rows(PLAYER_ROWS);
    m_game_state.player->set_animation_indices(idleFrames);
    m_game_state.player->set_animation_frames(1);
    m_game_state.player->set_position(glm::vec3(5, -5, 0));
    m_game_state.player->set_movement(glm::vec3(0));
    m_game_state.player->set_speed(3.0f);
    m_game_state.player->set_acceleration(glm::vec3(0));
    m_game_state.player->set_width(1.0f);
    m_game_state.player->set_height(1.0f);
    
    m_game_state.enemies = new Entity[2];
    m_game_state.enemies[0].set_entity_type(ENEMY);
    m_game_state.enemies[0].set_ai_type(WALKER);
    m_game_state.enemies[0].set_ai_state(WALKING);
    m_game_state.enemies[0].set_texture_id(skeletonTex);
   
    m_game_state.enemies[0].set_animation_cols(6);  // adjust if needed
    m_game_state.enemies[0].set_animation_rows(10);
    m_game_state.enemies[0].set_animation_indices(skeletonFrames);
    m_game_state.enemies[0].set_is_active(true);
    m_game_state.enemies[0].set_animation_frames(4);
    m_game_state.enemies[0].set_position(glm::vec3(7.0f, -2.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(-1.0, 0.0, 0.0));
    m_game_state.enemies[0].set_speed(1.5f);
    m_game_state.enemies->set_width(1.0f);
    m_game_state.enemies->set_height(1.0f);
    m_game_state.enemies[0].set_patrol_bounds(7.0f, 10.0f);
    
    m_game_state.enemies[1].set_entity_type(ENEMY);
    m_game_state.enemies[1].set_ai_type(FLYER);  // custom or default value to prevent movement
    m_game_state.enemies[1].set_ai_state(IDLE); // same as above
    m_game_state.enemies[1].set_texture_id(skeletonTex);
    m_game_state.enemies[1].set_animation_cols(6);
    m_game_state.enemies[1].set_animation_rows(10);
    m_game_state.enemies[1].set_animation_indices(skeleton2Frames);
    m_game_state.enemies[1].set_animation_frames(4);
    m_game_state.enemies[1].set_is_active(true);
    m_game_state.enemies[1].set_position(glm::vec3(3.0f, -5.0f, 0.0f)); // <- place on map
    m_game_state.enemies[1].set_movement(glm::vec3(0.0f)); // no movement
    m_game_state.enemies[1].set_speed(0.0f); // stationary
    m_game_state.enemies[1].set_width(1.0f);
    m_game_state.enemies[1].set_height(1.0f);
  

    m_game_state.enemy_count = 2;

    
}

void Level1::process_input(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE && !isFarming) {
        glm::vec3 pos = m_game_state.player->get_position();
        int tileX = static_cast<int>(round(pos.x));
        int tileY = static_cast<int>(-round(pos.y));

        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            if (island_map[tileY][tileX] == 4 && !carrotPlanted[tileY][tileX]) {
                isFarming = true;
                farmingTimer = 0.0f;
                farmingTileX = tileX;
                farmingTileY = tileY;

                m_game_state.player->set_animation_indices(farmingFrames);
                m_game_state.player->set_animation_frames(farmingFrameCount);
            }
        }
    }
    if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_E) {
        glm::vec3 pos = m_game_state.player->get_position();
        int tileX = static_cast<int>(round(pos.x));
        int tileY = static_cast<int>(-round(pos.y));

        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            if (carrotPlanted[tileY][tileX]) {
                carrotPlanted[tileY][tileX] = false;
                if (playerHealth < 5) playerHealth++;
                if (hurt_sfx) Mix_PlayChannel(-1, eat_sfx, 0);
            }
        }
    }
}

void Level1::update(float delta_time) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    glm::vec3 movement(0.0f);

    if (keys[SDL_SCANCODE_W]) movement.y =  1.0f;
    if (keys[SDL_SCANCODE_S]) movement.y = -1.0f;
    if (keys[SDL_SCANCODE_A]) movement.x = -1.0f;
    if (keys[SDL_SCANCODE_D]) movement.x =  1.0f;

    if (glm::length(movement) > 1.0f) {
        movement = glm::normalize(movement);
    }

    // Set animation based on direction
    if (!isFarming) {
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
        
        glm::vec3 player_pos = m_game_state.player->get_position();
        int tileX = static_cast<int>(round(player_pos.x));
        int tileY = static_cast<int>(-round(player_pos.y));

        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            if (island_map[tileY][tileX] == 2) { // 2 = house tile
                m_game_state.level_complete = true;
            }
        }
    }

    // Move the player
    glm::vec3 pos = m_game_state.player->get_position();
    pos += movement * m_game_state.player->m_speed * delta_time;
    m_game_state.player->set_position(pos);

    // Update player logic (includes animation timing)
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, m_game_state.enemy_count, m_game_state.map);
    
    // Clamp player inside map bounds (hardcoded width/height = 1.0f)
    glm::vec3 clamped_pos = m_game_state.player->get_position();
    float half_width = 0.5f;
    float half_height = 0.5f;

    clamped_pos.x = glm::clamp(clamped_pos.x, half_width, MAP_WIDTH - half_width);
    clamped_pos.y = glm::clamp(clamped_pos.y, -(MAP_HEIGHT - half_height), -half_height);

    m_game_state.player->set_position(clamped_pos);

 
   
    if (isFarming) {
        farmingTimer += delta_time;
        if (farmingTimer >= farmingDuration) {
            isFarming = false;
            if (farmingTileX >= 0 && farmingTileY >= 0) {
                carrotPlanted[farmingTileY][farmingTileX] = true;
            }

            // Return to idle
            m_game_state.player->set_animation_indices(idleFrames);
            m_game_state.player->set_animation_frames(1);
        }
    }
    
    for (int i = 0; i < m_game_state.enemy_count; i++) {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, m_game_state.enemies, m_game_state.enemy_count, m_game_state.map);
        if (m_game_state.player->check_collision(&m_game_state.enemies[i])) {
            if (playerHealth > 0) {
                playerHealth--;
                m_game_state.lives = playerHealth;
                if (hurt_sfx) Mix_PlayChannel(-1, hurt_sfx, 0);
                glm::vec3 enemy_pos = m_game_state.enemies[i].get_position();
                            glm::vec3 player_pos = m_game_state.player->get_position();

                            glm::vec3 knockback_dir = glm::normalize(player_pos - enemy_pos);
                            glm::vec3 knockback_offset = knockback_dir * 1.0f;  // You can adjust the distance

                            m_game_state.player->set_position(player_pos + knockback_offset);
            }
        }
    }


}



void Level1::render(ShaderProgram* program) {
    // Draw island
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            // Draw base (grass or water)
            GLuint baseTex;
                        switch (island_map[y][x]) {
                            case 1: baseTex = waterTex; break;
                            case 4: baseTex = farmTex; break;
                            default: baseTex = grassTex; break;
                        }

                        glm::mat4 baseModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                        program->set_model_matrix(baseModel);
                        draw_textured_quad(program, baseTex);


            // Overlay house on top of grass
            if (island_map[y][x] == 2) {
                glm::mat4 houseModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                program->set_model_matrix(houseModel);
                draw_textured_quad(program, houseTex);
            }
            if (island_map[y][x] == 3) {
                            glm::mat4 treeModel = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.0f));
                            program->set_model_matrix(treeModel);
                            draw_textured_quad(program, treeTex);
            }
          
            if (carrotPlanted[y][x]) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x * TILE_SIZE, -y * TILE_SIZE, 0.1f));
                program->set_model_matrix(model);
                draw_textured_quad(program, carrotTex);
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
        draw_textured_quad(program, heartTex);
    }

    // Restore map view matrix
    program->set_view_matrix(original_view);
    
   
    }


