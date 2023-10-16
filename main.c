#include "raylib.h"

typedef struct Player {
    Rectangle rec;
    Vector2 speed;
    Color color;
} Player;

typedef struct Enemy {
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
} Enemy;

typedef struct Shoot {
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
} Shoot;

static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;
static int score = 0;
static int wave = 1;
static int enemiesKilled = 0;
static int nextWaveScore = 10;

static Player player = { 0 };
static Enemy enemy[10] = { 0 };
static Shoot shoot[5] = { 0 };

void InitGame(void) {
    // Initialize game variables
    pause = false;
    gameOver = false;
    score = 0;
    wave = 1;
    enemiesKilled = 0;
    nextWaveScore = 10;

    // Initialize player
    player.rec.x = 20;
    player.rec.y = 50;
    player.rec.width = 20;
    player.rec.height = 20;
    player.speed.x = 5;
    player.speed.y = 5;
    player.color = BLACK;

    // Initialize enemies
    for (int i = 0; i < 10; i++) {
        enemy[i].rec.width = 10;
        enemy[i].rec.height = 10;
        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
        enemy[i].speed.x = 5 + wave;  // Increase enemy speed with each wave
        enemy[i].speed.y = 5 + wave;
        enemy[i].active = true;
        enemy[i].color = GRAY;
    }

    // Initialize shoots
    for (int i = 0; i < 5; i++) {
        shoot[i].rec.x = player.rec.x;
        shoot[i].rec.y = player.rec.y + player.rec.height / 4;
        shoot[i].rec.width = 10;
        shoot[i].rec.height = 5;
        shoot[i].speed.x = 7;
        shoot[i].speed.y = 0;
        shoot[i].active = false;
        shoot[i].color = MAROON;
    }
}

void UpdateGame(void) {
    if (!gameOver) {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause) {
            // Player movement
            if (IsKeyDown(KEY_RIGHT)) player.rec.x += player.speed.x;
            if (IsKeyDown(KEY_LEFT)) player.rec.x -= player.speed.x;
            if (IsKeyDown(KEY_UP)) player.rec.y -= player.speed.y;
            if (IsKeyDown(KEY_DOWN)) player.rec.y += player.speed.y;

            // Player collision with enemy
            for (int i = 0; i < 10; i++) {
                if (CheckCollisionRecs(player.rec, enemy[i].rec)) gameOver = true;
            }

            // Enemy behavior
            for (int i = 0; i < 10; i++) {
                if (enemy[i].active) {
                    enemy[i].rec.x -= enemy[i].speed.x;
                    if (enemy[i].rec.x < 0) {
                        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
                    }
                }
            }

            // Wall behavior
            if (player.rec.x <= 0) player.rec.x = 0;
            if (player.rec.x + player.rec.width >= screenWidth) player.rec.x = screenWidth - player.rec.width;
            if (player.rec.y <= 0) player.rec.y = 0;
            if (player.rec.y + player.rec.height >= screenHeight) player.rec.y = screenHeight - player.rec.height;

            // Shoot initialization
            if (IsKeyDown(KEY_SPACE)) {
                for (int i = 0; i < 5; i++) {
                    if (!shoot[i].active) {
                        shoot[i].rec.x = player.rec.x;
                        shoot[i].rec.y = player.rec.y + player.rec.height / 4;
                        shoot[i].active = true;
                        break;
                    }
                }
            }

            // Shoot logic
            for (int i = 0; i < 5; i++) {
                if (shoot[i].active) {
                    // Movement
                    shoot[i].rec.x += shoot[i].speed.x;
                    if (shoot[i].rec.x + shoot[i].rec.width >= screenWidth) {
                        shoot[i].active = false;
                    }

                    // Collision with enemy
                    for (int j = 0; j < 10; j++) {
                        if (enemy[j].active) {
                            if (CheckCollisionRecs(shoot[i].rec, enemy[j].rec)) {
                                shoot[i].active = false;
                                enemy[j].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                                enemy[j].rec.y = GetRandomValue(0, screenHeight - enemy[j].rec.height);
                                score++;
                                enemiesKilled++;

                                if (score >= nextWaveScore) {
                                    wave++;
                                    nextWaveScore += 10;
                                    for (int k = 0; k < 10; k++) {
                                        enemy[k].speed.x += 1;  // Increase enemy speed with each wave
                                        enemy[k].speed.y += 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        if (IsKeyPressed(KEY_ENTER)) {
            InitGame();
            gameOver = false;
        }
    }
}

void DrawGame(void) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver) {
        DrawRectangleRec(player.rec, player.color);

        for (int i = 0; i < 10; i++) {
            if (enemy[i].active) DrawRectangleRec(enemy[i].rec, enemy[i].color);
        }

        for (int i = 0; i < 5; i++) {
            if (shoot[i].active) DrawRectangleRec(shoot[i].rec, shoot[i].color);
        }

        DrawText(TextFormat("Score: %04i", score), 20, 20, 30, GRAY);
        DrawText(TextFormat("Wave: %02i", wave), 20, 60, 30, GRAY);

        if (pause) DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
    }
    else {
        DrawText("GAME OVER - PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("GAME OVER - PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);
    }

    EndDrawing();
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Simple Space Invaders");

    InitGame();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }

    CloseWindow();

    return 0;
}
