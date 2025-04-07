#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "utility.h"
#include "scene.h"
#include "menuscene.h"
#include "level1.h"
#include "level2.h"
#include "level3.h"
#include "winscene.h"
#include "losescene.h"

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
constexpr int PLAY_ONCE   =  0;
constexpr int NEXT_CHNL   = -1;  // next available channel


SDL_Window* g_display_window;
Scene* g_current_scene;
Scene* g_menu;
Scene* g_level1;
Scene* g_level2;
Scene* g_level3;
Scene* g_win;
Scene* g_lose;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;
float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;
const float FIXED_TIMESTEP = 0.0166666f;

GameState g_game_state;

Mix_Music* g_bgm;
Mix_Chunk* g_jump_sfx;
Mix_Chunk* g_death_sfx;
Mix_Chunk* g_win_sfx;

GLuint g_background_texture_id;

void switch_to_scene(Scene* scene) {
    g_current_scene = scene;
    g_current_scene->initialise();
    g_game_state = g_current_scene->get_state();

    g_game_state.jump_sfx = g_jump_sfx;
    g_game_state.death_sfx = g_death_sfx;
    g_game_state.win_sfx = g_win_sfx;

    g_game_state.bgm = g_bgm;

    if (g_game_state.player) {
        glm::vec3 player_pos = g_game_state.player->get_position();
        float cam_x = -player_pos.x;
        float cam_y = -player_pos.y;
        g_view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(cam_x, cam_y, 0.0f));
        g_shader_program.set_view_matrix(g_view_matrix);
    }
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Cat Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif
    

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    g_shader_program.load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    glUseProgram(g_shader_program.get_program_id());

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    g_bgm = Mix_LoadMUS("assets/background.ogg");
    Mix_PlayMusic(g_bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    

    g_jump_sfx = Mix_LoadWAV("assets/jump.wav");
    Mix_VolumeChunk(g_jump_sfx, MIX_MAX_VOLUME);
    Mix_PlayChannel(
           NEXT_CHNL,       // using the first channel that is not currently in use...
           g_jump_sfx,  // ...play this chunk of audio...
           PLAY_ONCE        // ...once.
           );
    g_death_sfx = Mix_LoadWAV("assets/death.wav");
    g_win_sfx = Mix_LoadWAV("assets/win.wav");

    g_background_texture_id = Utility::load_texture("assets/background.png");

    g_game_state.lives = 3;

    g_menu = new MenuScene();
    g_level1 = new Level1();
    g_level2 = new Level2();
    g_level3 = new Level3();
    g_win = new WinScene();
    g_lose = new LoseScene();

    switch_to_scene(g_menu);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            exit(0);
        }
        g_current_scene->process_input(event);
    }
}

void update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    delta_time += g_accumulator;
    if (delta_time < FIXED_TIMESTEP) {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_game_state = g_current_scene->get_state();
    g_accumulator = delta_time;

    

    if (g_current_scene == g_menu && static_cast<MenuScene*>(g_current_scene)->start_game) {
        switch_to_scene(g_level1);
    }

    if (g_current_scene == g_level1 && g_game_state.level_complete) {
        switch_to_scene(g_level2);
    }

    if (g_current_scene == g_level2 && g_game_state.level_complete) {
        switch_to_scene(g_level3);
    }

    if (g_current_scene == g_level3 && g_game_state.level_complete) {
        Mix_PlayChannel(-1, g_win_sfx, 0);
        switch_to_scene(g_win);
    }

    if (g_game_state.lives <= 0) {
        Mix_PlayChannel(-1, g_death_sfx, 0);
        switch_to_scene(g_lose);
    }

    if (g_game_state.player) {
        glm::vec3 player_pos = g_game_state.player->get_position();
        float cam_x = -player_pos.x;
        float cam_y = -player_pos.y;
        g_view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(cam_x, cam_y, 0.0f));
        g_shader_program.set_view_matrix(g_view_matrix);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    ShaderProgram* program = &g_shader_program;
    glm::mat4 background_model = glm::inverse(g_view_matrix);
    program->set_model_matrix(background_model);

    float background_vertices[] = {
        -5.0f, -3.75f, 5.0f, -3.75f, 5.0f, 3.75f,
        -5.0f, -3.75f, 5.0f, 3.75f, -5.0f, 3.75f
    };

    float background_tex_coords[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };

    glBindTexture(GL_TEXTURE_2D, g_background_texture_id);
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, background_vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, background_tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());

    g_current_scene->render(&g_shader_program);
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    Mix_FreeChunk(g_jump_sfx);
    Mix_FreeChunk(g_death_sfx);
    Mix_FreeChunk(g_win_sfx);
    Mix_FreeMusic(g_bgm);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    initialise();
    while (true) {
        process_input();
        update();
        render();
    }
    return 0;
}
