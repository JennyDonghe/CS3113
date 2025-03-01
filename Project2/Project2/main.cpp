/**
* Author: [Jenny Dong]
* Assignment: Pong Clone
* Date due: 2025/3/1 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

enum AppStatus { RUNNING, TERMINATED };

constexpr float WINDOW_SIZE_MULT = 2.0f;

constexpr int WINDOW_WIDTH  = 640 * WINDOW_SIZE_MULT,
              WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;


constexpr int VIEWPORT_X = 0,
              VIEWPORT_Y = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL    = 0;
constexpr GLint TEXTURE_BORDER     = 0;



constexpr char SPONGE_FILEPATH[] = "player1.png",
               PATRICK_FILEPATH[]  = "player2.png",
               BURGER_FILEPATH[]  = "ball.png";

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;

constexpr glm::vec3 INIT_SCALE_BURGER      = glm::vec3(0.5f, 0.5f, 0.0f),
                    INIT_POS_BURGER       = glm::vec3(0.0f, 0.0f, 0.0f),
                    INIT_SCALE_PLAYER    = glm::vec3(2.0f, 2.0f, 0.0f),
                    INIT_POS_SPONGE  = glm::vec3(-4.0f, 0.0f, 0.0f),
                    INIT_POS_PATRICK  = glm::vec3(4.0f, 0.0f, 0.0f);

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_sponge_matrix, g_patrick_matrix,g_projection_matrix,g_burger_matrix;

float g_previous_ticks = 0.0f;

float screen_height_boundary = 3.75f;
float paddle_height = INIT_SCALE_PLAYER.y;
float paddle_width = INIT_SCALE_PLAYER.x;
float burger_radius = INIT_SCALE_BURGER.x * 0.5f;
bool twoplayers = true;
bool change_x = false;
bool change_y = false;
constexpr float BURGER_SPEED = 1.3f, PLAYER_SPEED = 3.0f;

GLuint g_sponge_texture_id;
GLuint g_patrick_texture_id;
GLuint g_burger_texture_id;

glm::vec3 g_sponge_position  = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_sponge_movement  = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_patrick_position  = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_patrick_movement  = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_burger_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_burger_movement = glm::vec3(0.0f, 0.0f, 0.0f);

void initialise();
void process_input();
void paddle_wall_collision();
void paddle_ball_collision();
void ball_wall_collision();
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

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("SPONGE BOB SQUARE PANTS",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr) shutdown();

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_sponge_matrix = glm::mat4(1.0f);
    g_patrick_matrix = glm::mat4(1.0f);
    g_burger_matrix = glm::mat4(1.0f);
        
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    
    g_sponge_texture_id = load_texture(SPONGE_FILEPATH);
    g_patrick_texture_id = load_texture(PATRICK_FILEPATH);
    g_burger_texture_id = load_texture(BURGER_FILEPATH);
    glClearColor(0.9f, 0.9f, 0.1f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_sponge_movement = glm::vec3(0.0f);
    g_patrick_movement = glm::vec3(0.0f);
    g_burger_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;
                    case SDLK_w:
                        g_sponge_movement.y = 1.0f;
                        break;
                    case SDLK_s:
                        g_sponge_movement.y = -1.0f;
                        break;
                    case SDLK_UP:
                        g_patrick_movement.y = 1.0f;
                        break;
                    case SDLK_DOWN:
                        g_patrick_movement.y = -1.0f;
                        break;
                    case SDLK_t:
                        twoplayers = false;
                        break;
                }
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                    case SDLK_w:
                    case SDLK_s:
                        g_sponge_movement.y = 0.0f;
                        break;
                    case SDLK_UP:
                    case SDLK_DOWN:
                        g_patrick_movement.y = 0.0f;
                        break;
                }
                break;
            default:
                break;
        }
    }
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    if (glm::length(g_burger_movement) > 1.0f)
    {
        g_burger_movement = glm::normalize(g_burger_movement);
    }
    // player 1 keys
    if (key_state[SDL_SCANCODE_W])
    {
        g_sponge_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        g_sponge_movement.y = -1.0f;
    }
    //player 2 keys
    if (!twoplayers)
    {
        if (g_burger_position.y > g_patrick_position.y + INIT_SCALE_PLAYER.y * 0.5f)
        {
            g_patrick_movement.y = 1.0f;
        }
        else if (g_burger_position.y < g_patrick_position.y - INIT_SCALE_PLAYER.y * 0.5f)
        {
            g_patrick_movement.y = -1.0f;
        }
        else
        {
            g_patrick_movement.y = 0.0f;
        }
    }
    else {
        if (key_state[SDL_SCANCODE_UP])
        {
            g_patrick_movement.y = 1.0f;
        }
        else if (key_state[SDL_SCANCODE_DOWN])
        {
            g_patrick_movement.y = -1.0f;
        }
    }
}

void paddle_wall_collision() {
    glm::vec3* paddles[] = { &g_sponge_position, &g_patrick_position };
    glm::vec3* movements[] = { &g_sponge_movement, &g_patrick_movement };
    
    for (int i = 0; i < 2; i++) {
        if (paddles[i]->y > screen_height_boundary - (INIT_SCALE_PLAYER.y * 0.5f)) {
            paddles[i]->y = screen_height_boundary - (INIT_SCALE_PLAYER.y * 0.5f);
            *movements[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        if (paddles[i]->y < -screen_height_boundary + (INIT_SCALE_PLAYER.y * 0.5f)) {
            paddles[i]->y = -screen_height_boundary + (INIT_SCALE_PLAYER.y * 0.5f);
            *movements[i] = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
}


void paddle_ball_collision() {
    glm::vec3 paddles[] = { g_sponge_position, g_patrick_position };
    glm::vec3 init_positions[] = { INIT_POS_SPONGE, INIT_POS_PATRICK };
    
    for (int i = 0; i < 2; i++) {
        float x_distance = (paddles[i].x + init_positions[i].x) - (g_burger_position.x + INIT_POS_BURGER.x);
        float y_distance = (paddles[i].y + init_positions[i].y) - (g_burger_position.y + INIT_POS_BURGER.y);
        float combined_width = (INIT_SCALE_BURGER.x + INIT_SCALE_PLAYER.x) / 2.0f;
        float combined_height = (INIT_SCALE_BURGER.y + INIT_SCALE_PLAYER.y) / 2.0f;
        
        if (-combined_width < x_distance && x_distance < combined_width &&
            -combined_height < y_distance && y_distance < combined_height) {
            change_x = !change_x;
            change_y = !change_y;
            g_burger_movement.x *= -1;
        }
    }
}



void ball_wall_collision() {
    if (g_burger_position.x - burger_radius <= -5.0f)
    {
        g_burger_position = glm::vec3(0.0f, 0.0f, 0.0f);
        g_burger_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    else if (g_burger_position.x + burger_radius >= 5.0f)
    {
        g_burger_position = glm::vec3(0.0f, 0.0f, 0.0f);
        g_burger_movement = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    
    if (g_burger_position.y + burger_radius >= screen_height_boundary)
    {
        change_y = true;
    }
    else if (g_burger_position.y - burger_radius <= -screen_height_boundary)
    {
        change_y = false;
    }
    
    if (change_x && change_y)
    {
        g_burger_movement = glm::vec3(-BURGER_SPEED, -BURGER_SPEED, 0.0f);
    }
    else if (change_x)
    {
        g_burger_movement = glm::vec3(-BURGER_SPEED, BURGER_SPEED, 0.0f);
    }
    else if (change_y)
    {
        g_burger_movement = glm::vec3(BURGER_SPEED, -BURGER_SPEED, 0.0f);
    }
    else
    {
        g_burger_movement = glm::vec3(BURGER_SPEED, BURGER_SPEED, 0.0f);
    }
}


void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    g_sponge_matrix = glm::mat4(1.0f);
    g_sponge_matrix = glm::translate(g_sponge_matrix, INIT_POS_SPONGE);
    g_sponge_matrix = glm::translate(g_sponge_matrix, g_sponge_position);
    g_sponge_position += g_sponge_movement * PLAYER_SPEED * delta_time;

    g_patrick_matrix = glm::mat4(1.0f);
    g_patrick_matrix = glm::translate(g_patrick_matrix, INIT_POS_PATRICK);
    g_patrick_matrix = glm::translate(g_patrick_matrix, g_patrick_position);
    g_patrick_position += g_patrick_movement * PLAYER_SPEED * delta_time;
    
    g_burger_matrix = glm::mat4(1.0f);
    g_burger_matrix = glm::translate(g_burger_matrix, INIT_POS_BURGER);

    paddle_wall_collision();
    ball_wall_collision();
    paddle_ball_collision();
    
    g_burger_position += g_burger_movement * BURGER_SPEED * delta_time;
    g_burger_matrix = glm::translate(g_burger_matrix, g_burger_position);
    
    g_sponge_matrix = glm::scale(g_sponge_matrix, INIT_SCALE_PLAYER);
    g_patrick_matrix = glm::scale(g_patrick_matrix, INIT_SCALE_PLAYER);
    g_burger_matrix  = glm::scale(g_burger_matrix, INIT_SCALE_BURGER);
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_burger_matrix, g_burger_texture_id);
    draw_object(g_sponge_matrix, g_sponge_texture_id);
    draw_object(g_patrick_matrix, g_patrick_texture_id);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}

