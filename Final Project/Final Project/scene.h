
#ifndef SCENE_H
#define SCENE_H

#include <SDL.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"
#include "Entity.h"
#include "map.h"
#include "utility.h"

struct GameState {
    int next_scene_id;
    Entity* player;
    Entity* enemies;
    int enemy_count = 0;
   

    Map* map;
    int lives = 3;
    bool level_complete = false;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
    Mix_Chunk* death_sfx;
    Mix_Chunk* win_sfx;
};

class Scene {
protected:
    GameState m_game_state;

public:
    int m_number_of_enemies = 1;
    

    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram* program) = 0;
    virtual void process_input(SDL_Event& event) = 0;

    GameState& get_state() { return m_game_state; }
    int const get_number_of_enemies() const { return m_number_of_enemies; }
};

#endif 
