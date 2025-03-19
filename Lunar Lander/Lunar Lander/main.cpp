/**
* Author: Jenny Dong
* Assignment: Lunar Lander
* Date due: 2025/ 03/18 11: 59 pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* game_lost;
    Entity* game_won;
};

constexpr int WINDOW_WIDTH  = 640,
              WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.1922f,
                BG_BLUE    = 0.549f,
                BG_GREEN   = 0.9059f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
              VIEWPORT_Y = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr char SPRITESHEET_FILEPATH[] = "assets/player.png",
               PLATFORM_FILEPATH[]    = "assets/platform.png",
               TARGET_FILEPATH[]      = "assets/wintile.png",
               GAME_WON_FILEPATH[]    = "assets/missioncomplete.png",
               GAME_FAIL_FILEPATH[]   = "assets/missionfailed.png";

constexpr int PLATFORM_COUNT = 10;
float g_gravity = -3.0f;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;

GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

bool g_game_win = false;
bool g_game_over = false;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;

void initialise();
void process_input();
void update();
void render();
void shutdown();

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
};

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Lunar Lander",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
        
    g_game_state.platforms = new Entity[PLATFORM_COUNT];
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    
    GLuint target_texture_id = load_texture(TARGET_FILEPATH);
    int trap_index = PLATFORM_COUNT / 2;
    float start_x = -4.5f;
    float end_x = 4.5f;
    float spacing = (end_x - start_x) / (PLATFORM_COUNT - 1);

    for (int i = 0; i < PLATFORM_COUNT; i++) {
        g_game_state.platforms[i] = Entity();
        g_game_state.platforms[i].set_texture_id(platform_texture_id);

        float x_position = start_x + i * spacing;
        g_game_state.platforms[i].set_position(glm::vec3(x_position, -3.0f, 0.0f));

        if (i == trap_index) {
            g_game_state.platforms[i].set_texture_id(target_texture_id);
            g_game_state.platforms[i].set_platform_type(TRAP);
        } else {
            g_game_state.platforms[i].set_platform_type(NORMAL);
        }

        g_game_state.platforms[i].update(0.0f, NULL, 0);
    }
    
    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);
    
    g_game_state.player = new Entity();
    g_game_state.player->set_position(glm::vec3(0.0f, 2.0f, 0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, g_gravity * 0.1, 0.0f));
    g_game_state.player->set_speed(2.0f);
    g_game_state.player->set_texture_id(player_texture_id);
    g_game_state.player->set_height(0.8f);
    g_game_state.player->set_width(0.8f);
    g_game_state.player->set_collisioin_type(NOCOLLISION);
    
    GLuint game_fail_texture_id = load_texture(GAME_FAIL_FILEPATH);
    g_game_state.game_lost = new Entity();
    g_game_state.game_lost->set_position(glm::vec3(0.0f));
    g_game_state.game_lost->set_texture_id(game_fail_texture_id);
    g_game_state.game_lost->scale(glm::vec3(4.0f, 3.0f, 0.0f));
    
    GLuint game_won_texture_id = load_texture(GAME_WON_FILEPATH);
    g_game_state.game_won = new Entity();
    g_game_state.game_won->set_position(glm::vec3(0.0f));
    g_game_state.game_won->set_texture_id(game_won_texture_id);
    g_game_state.game_won->scale(glm::vec3(4.0f, 3.0f, 0.0f));
    
    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (!g_game_over) {

        if (key_state[SDL_SCANCODE_LEFT])
        {
            g_game_state.player->set_acceleration(glm::vec3(-1.5f, 0.0f, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_RIGHT])
        {
            g_game_state.player->set_acceleration(glm::vec3(1.5f, 0.0f, 0.0f));
        }
        else
        {
            g_game_state.player->set_acceleration(glm::vec3(0.0f, g_gravity * 0.1, 0.0f));
        }
        
        if (key_state[SDL_SCANCODE_UP])
        {
            g_game_state.player->set_acceleration(glm::vec3(0.0f, 1.5f + g_gravity * 0.1, 0.0f));
        }
        else if (key_state[SDL_SCANCODE_DOWN])
        {
            g_game_state.player->set_acceleration(glm::vec3(0.0f, -1.5f - g_gravity * 0.1, 0.0f));
        }

        if (glm::length(g_game_state.player->get_movement()) > 1.5f)
        {
            g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_time_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        CollisionType result = g_game_state.player->check_collision_y(g_game_state.platforms, PLATFORM_COUNT);
        
        if (!g_game_over) {
            result = g_game_state.player->update(FIXED_TIMESTEP, g_game_state.platforms, PLATFORM_COUNT);
        }
        
        if (result == GROUND) {
            g_game_over = true;
            g_game_win = false;
            g_game_state.player->set_movement(glm::vec3(0.0f));
            g_game_state.player->deactivate();
        
            g_game_state.game_lost->render(&g_shader_program);
        }
        else if (result == WIN) {
            g_game_over = true;
            g_game_win = true;
            g_game_state.player->set_movement(glm::vec3(0.0f));
            g_game_state.player->deactivate();
        
            g_game_state.game_won->render(&g_shader_program);
        }
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_time_accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.player->render(&g_shader_program);
    
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_game_state.platforms[i].render(&g_shader_program);
    }

    if (g_game_over)
    {
        if (g_game_win)
        {
            g_game_state.game_won->render(&g_shader_program);
        }
        else
        {
            g_game_state.game_lost->render(&g_shader_program);
        }
        
    }
    
    SDL_GL_SwapWindow(g_display_window);

}

void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}

