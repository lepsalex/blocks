#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Default value definitions
//----------------------------------------------------------------------------------
#define PLAYER_LIVES             5
#define BRICKS_LINES             5
#define BRICKS_PER_LINE         20

#define BRICKS_POSITION_Y       50

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Window initialization and screens management
typedef enum GameScreen { LOGO, TITLE, GAMEPLAY, ENDING } GameScreen;

// Player structure
typedef struct Player {
    Vector2 position;
    Vector2 speed;
    Vector2 size;
    Rectangle bounds;
    int lives;
} Player;

// Ball structure
typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    float radius;
    bool active;
} Ball;

// Bricks structure
typedef struct Brick {
    Vector2 position;
    Vector2 size;
    Rectangle bounds;
    int resistance;
    bool active;
} Brick;

// Game resources
typedef struct GameResources {
    Texture2D texLogo;
    Texture2D texBall;
    Texture2D texPaddle;
    Texture2D texBrick;
    Font font;
    Sound fxStart;
    Sound fxBounce;
    Sound fxExplode;
    Music music;
} GameResources;

//----------------------------------------------------------------------------------
// Game State Declaration/Initialization
//----------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;
const float ballRadius = 10.0f;

// Game required variables
GameScreen screen = LOGO; // Current game screen state

int framesCounter = 0; // General purpose frames counter
int gameResult = -1; // Game result: 0 - Loose, 1 - Win, -1 - Not defined
bool gamePaused = false; // Game paused state toggle

// Init game state structs
Player player = { 0 };
Ball ball = { 0 };
Brick bricks[BRICKS_LINES][BRICKS_PER_LINE] = {0};
GameResources gameResources = {0};

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void); // Update and Draw one frame

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main() {
    // Window initialization and screens management
    InitWindow(screenWidth, screenHeight, "PROJECT: BLOCKS GAME");

    // Init Audio
    InitAudioDevice();

    // Load resources (textures, fonts, audio) after Window initialization
    gameResources.texLogo = LoadTexture("resources/raylib_logo.png");
    gameResources.texBall = LoadTexture("resources/ball.png");
    gameResources.texPaddle = LoadTexture("resources/paddle.png");
    gameResources.texBrick = LoadTexture("resources/brick.png");

    gameResources.font = LoadFont("resources/setback.png");

    gameResources.fxStart = LoadSound("resources/start.wav");
    gameResources.fxBounce = LoadSound("resources/bounce.wav");
    gameResources.fxExplode = LoadSound("resources/explosion.wav");

    gameResources.music = LoadMusicStream("resources/bkg_music.mp3");

    PlayMusicStream(gameResources.music); // Start music streaming

    // Initialize player
    player.position = (Vector2){ screenWidth/2, screenHeight*7/8 };
    player.speed = (Vector2){ 8.0f, 0.0f };
    player.size = (Vector2){ 100, 24 };
    player.lives = PLAYER_LIVES;

    // Initialize ball
    ball.radius = 10.0f;
    ball.active = false;
    ball.position = (Vector2){ player.position.x + player.size.x/2, player.position.y - ball.radius*2 };
    ball.speed = (Vector2){ 4.0f, 4.0f };

    // Initialize bricks
    for (int j = 0; j < BRICKS_LINES; j++) {
        for (int i = 0; i < BRICKS_PER_LINE; i++) {
            bricks[j][i].size = (Vector2){screenWidth / BRICKS_PER_LINE, 20};
            bricks[j][i].position = (Vector2){i * bricks[j][i].size.x, j * bricks[j][i].size.y + BRICKS_POSITION_Y};
            bricks[j][i].bounds = (Rectangle){
                bricks[j][i].position.x, bricks[j][i].position.y, bricks[j][i].size.x, bricks[j][i].size.y
            };
            bricks[j][i].active = true;
        }
    }

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60); // Set desired framerate (frames per second)
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload any loaded resources (texture, fonts, audio)
    UnloadTexture(gameResources.texBall);
    UnloadTexture(gameResources.texPaddle);
    UnloadTexture(gameResources.texBrick);

    UnloadFont(gameResources.font);

    UnloadSound(gameResources.fxStart);
    UnloadSound(gameResources.fxBounce);
    UnloadSound(gameResources.fxExplode);

    UnloadMusicStream(gameResources.music); // Unload music streaming buffers

    CloseAudioDevice(); // Close audio device connection

    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

void UpdateDrawFrame(void) {
    // Update
    //----------------------------------------------------------------------------------
    switch (screen) {
        case LOGO: {
            // Update LOGO screen data here!

            framesCounter++;

            if (framesCounter > 60 * 2) {
                screen = TITLE; // Change to TITLE screen after 2 seconds
                framesCounter = 0;
            }
        }
        break;
        case TITLE: {
            // Update TITLE screen data here!

            framesCounter++;

            // LESSON 03: Inputs management (keyboard, mouse)
            if (IsKeyPressed(KEY_ENTER)) {
                screen = GAMEPLAY;
                PlaySound(gameResources.fxStart);
            }
        }
        break;
        case GAMEPLAY: {
            // Update GAMEPLAY screen data here!

            if (IsKeyPressed('P')) gamePaused = !gamePaused; // Pause button logic

            if (!gamePaused) {
                // Player movement logic
                if (IsKeyDown(KEY_LEFT)) player.position.x -= player.speed.x;
                if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed.x;

                if ((player.position.x) <= 0) player.position.x = 0;
                if ((player.position.x + player.size.x) >= screenWidth)
                    player.position.x = screenWidth - player.size.x;

                player.bounds = (Rectangle){player.position.x, player.position.y, player.size.x, player.size.y};

                if (ball.active) {
                    // Ball movement logic
                    ball.position.x += ball.speed.x;
                    ball.position.y += ball.speed.y;

                    // Collision logic: ball vs screen-limits
                    if (((ball.position.x + ball.radius) >= screenWidth) || ((ball.position.x - ball.radius) <= 0))
                        ball.speed.x *= -1;
                    if ((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;

                    // Collision logic: ball vs player
                    if (CheckCollisionCircleRec(ball.position, ball.radius, player.bounds)) {
                        ball.speed.y *= -1;
                        ball.speed.x = (ball.position.x - (player.position.x + player.size.x / 2)) / player.size.x *
                                       5.0f;
                        PlaySound(gameResources.fxBounce);
                    }

                    // Collision logic: ball vs bricks
                    for (int j = 0; j < BRICKS_LINES; j++) {
                        for (int i = 0; i < BRICKS_PER_LINE; i++) {
                            if (bricks[j][i].active && (CheckCollisionCircleRec(
                                    ball.position, ball.radius, bricks[j][i].bounds))) {
                                bricks[j][i].active = false;
                                ball.speed.y *= -1;
                                PlaySound(gameResources.fxExplode);

                                break;
                            }
                        }
                    }

                    // Game ending logic
                    if ((ball.position.y + ball.radius) >= screenHeight) {
                        ball.position.x = player.position.x + player.size.x / 2;
                        ball.position.y = player.position.y - ball.radius - 1.0f;
                        ball.speed = (Vector2){0, 0};
                        ball.active = false;

                        player.lives--;
                    }

                    if (player.lives < 0) {
                        screen = ENDING;
                        player.lives = 5;
                        framesCounter = 0;
                    }
                } else {
                    // Reset ball position
                    ball.position.x = player.position.x + player.size.x / 2;

                    // Inputs management (keyboard, mouse)
                    if (IsKeyPressed(KEY_SPACE)) {
                        // Activate ball logic
                        ball.active = true;
                        ball.speed = (Vector2){0, -5.0f};
                    }
                }
            }
        }
        break;
        case ENDING: {
            // Update END screen data here!

            framesCounter++;

            // LESSON 03: Inputs management (keyboard, mouse)
            if (IsKeyPressed(KEY_ENTER)) screen = TITLE;
        }
        break;
        default: break;
    }

    // NOTE: Music buffers must be refilled if consumed
    UpdateMusicStream(gameResources.music);

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    switch (screen) {
        case LOGO: {
            // Draw LOGO screen here!
            DrawTexture(gameResources.texLogo, screenWidth / 2 - gameResources.texLogo.width / 2, screenHeight / 2 - gameResources.texLogo.height / 2, WHITE);
        }
        break;
        case TITLE: {
            // Draw TITLE screen here!

            DrawTextEx(gameResources.font, "BLOCKS", (Vector2){100, 80}, 160, 10, MAROON); // Draw Title

            if ((framesCounter / 30) % 2 == 0)
                DrawText("PRESS [ENTER] to START",
                         GetScreenWidth() / 2 - MeasureText(
                             "PRESS [ENTER] to START", 20) / 2,
                         GetScreenHeight() / 2 + 60, 20, DARKGRAY);
        }
        break;
        case GAMEPLAY: {
            // Draw GAMEPLAY screen here!

            // Draw player
            DrawTextureEx(gameResources.texPaddle, player.position, 0.0f, 1.0f, WHITE);

            // Draw ball
            DrawTexture(gameResources.texBall, ball.position.x - ball.radius / 2, ball.position.y - ball.radius / 2, MAROON);

            // Draw bricks
            for (int j = 0; j < BRICKS_LINES; j++) {
                for (int i = 0; i < BRICKS_PER_LINE; i++) {
                    if (bricks[j][i].active) {
                        // NOTE: Texture is not scaled, just using original size

                        if ((i + j) % 2 == 0) DrawTextureEx(gameResources.texBrick, bricks[j][i].position, 0.0f, 1.0f, GRAY);
                        else DrawTextureEx(gameResources.texBrick, bricks[j][i].position, 0.0f, 1.0f, DARKGRAY);
                    }
                }
            }

            // Draw GUI: player lives
            for (int i = 0; i < player.lives; i++) DrawRectangle(20 + 40 * i, screenHeight - 30, 35, 10, LIGHTGRAY);

            // Draw pause message when required
            if (gamePaused)
                DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2,
                         screenHeight / 2 + 60, 40, GRAY);
        }
        break;
        case ENDING: {
            // Draw END screen here!

            DrawTextEx(gameResources.font, "GAME FINISHED", (Vector2){80, 100}, 80, 6, MAROON);

            if ((framesCounter / 30) % 2 == 0)
                DrawText("PRESS [ENTER] TO PLAY AGAIN",
                         GetScreenWidth() / 2 - MeasureText(
                             "PRESS [ENTER] TO PLAY AGAIN", 20) / 2,
                         GetScreenHeight() / 2 + 80, 20, GRAY);
        }
        break;
        default: break;
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
}
