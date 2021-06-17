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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, ballMatrix, paddle1Matrix, paddle2Martix, projectionMatrix;

//Paddle 1
// Start at 0, 0, 0
glm::vec3 player1_position = glm::vec3(9.5f,0.0f,0.0f);
// Don’t go anywhere (yet).
glm::vec3 player1_movement = glm::vec3(0, 0, 0);
 
float player1_speed = 1.0f;

//Paddle2
// Start at 0, 0, 0
glm::vec3 player2_position = glm::vec3(-9.5f,0.0f,0.0f);
// Don’t go anywhere (yet).
glm::vec3 player2_movement = glm::vec3(0, 0, 0);
 
float player2_speed = 1.0f;

//Ball
// Start at 0, 0, 0
glm::vec3 ball_position = glm::vec3(0, 0, 0);
// Don’t go anywhere (yet).
glm::vec3 ball_movement = glm::vec3(0, 0, 0);
 
float ball_speed = 2.0f;

GLuint ballTextureID;
GLuint paddle1TextureID;
GLuint paddle2TextureID;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}



void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    ballMatrix = glm::mat4(1.0f);
    paddle1Matrix = glm::mat4(1.0f);
    paddle2Martix = glm::mat4(1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    ballTextureID = LoadTexture("blue.png");
    paddle1TextureID = LoadTexture("blue.png");
    paddle2TextureID = LoadTexture("blue.png");
    
}

void ProcessInput() {
    player1_movement = glm::vec3(0);
    player2_movement = glm::vec3(0);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type){
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym){
                    case SDLK_LEFT:
                        break;
                    case SDLK_RIGHT:
                        break;
                    case SDLK_SPACE:
                        break;
                }
                break;
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if (keys[SDL_SCANCODE_UP] && (player1_position.y < 1.35f)) {
        player1_movement.y = 1.0;
    }
    else if (keys[SDL_SCANCODE_DOWN] && (player1_position.y > -1.35f)) {
        player1_movement.y = -1.0;
    }
    
    if(glm::length(player1_movement)>1.0f){
        player1_movement = glm::normalize(player1_movement);
    }
    
    if (keys[SDL_SCANCODE_W] && (player2_position.y < 1.35f)) {
        player2_movement.y = 1.0;
    }
    else if (keys[SDL_SCANCODE_S] && (player2_position.y > -1.35f)) {
        player2_movement.y = -1.0;
    }
    
    if(glm::length(player1_movement)>1.0f){
        player2_movement = glm::normalize(player2_movement);
    }
    
    if (keys[SDL_SCANCODE_SPACE]) {
        ball_movement.y = 1.0;
        ball_movement.x = 1.0;
    }
}

bool collision_check(){
    float xdist1 = fabs(player1_position.x - ball_position.x) - ((6.3f+0.5f)/2.0f);
    float ydist1 = fabs(player1_position.y - ball_position.y) - ((6.3f+0.5f)/2.0f);
    float xdist2 = fabs(player2_position.x - ball_position.x) - ((6.3f+0.5f)/2.0f);
    float ydist2 = fabs(player2_position.y - ball_position.y) - ((6.3f+0.5f)/2.0f);

    if ((xdist1<0 && ydist1<0) || (xdist2<0 && ydist2<0)){
        return true;
    }
    return false;
}


float lastTicks = 0.0f;

void Update() {
    paddle1Matrix = glm::mat4(1.0f);
    paddle1Matrix = glm::scale(paddle1Matrix, glm::vec3(0.5f,2.0f,1.0f));
    
    paddle2Martix = glm::mat4(1.0f);
    paddle2Martix = glm::scale(paddle2Martix, glm::vec3(0.5f,2.0f,1.0f));
    
    ballMatrix = glm::mat4(1.0f);
    ballMatrix = glm::scale(ballMatrix, glm::vec3(0.5f,0.5f,1.0f));
    
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    // Add (direction * units per second * elapsed time)
    player1_position += player1_movement * player1_speed * deltaTime;
    paddle1Matrix = glm::translate(paddle1Matrix, player1_position);
    
    player2_position += player2_movement * player2_speed * deltaTime;
    paddle2Martix = glm::translate(paddle2Martix, player2_position);
    
    if (ball_position.y < -7.0f || ball_position.y > 7.0f){
        ball_movement.y *= -1;
    }
    
    if (ball_position.x < -8.5f || ball_position.x > 8.5f){
        if (collision_check()){
            ball_movement.x *= -1;
            ball_position.x = ball_movement.x * -8.5f;
        }
    }
    
    if (ball_position.x < -9.5f || ball_position.x > 9.5f){
        ball_movement = glm::vec3(0);
    }
    
    ball_position += ball_movement * ball_speed * deltaTime;
    ballMatrix = glm::translate(ballMatrix, ball_position);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    program.SetModelMatrix(ballMatrix);
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    program.SetModelMatrix(paddle1Matrix);
    glBindTexture(GL_TEXTURE_2D, paddle1TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    program.SetModelMatrix(paddle2Martix);
    glBindTexture(GL_TEXTURE_2D, paddle2TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
