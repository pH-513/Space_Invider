#include "raylib.h"
#include <stdio.h>

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

typedef struct PowerUp {
    Rectangle rec;
    bool active;
    Color color;
} PowerUp;

static const int screenWidth = 800;
static const int screenHeight = 450;

static Texture2D mapTexture;
static Texture2D enemyTexture;
static Texture2D playerTexture;


static bool gameOver = false;
static bool pause = false;
static int score = 0;
static int wave = 1;
static int enemiesKilled = 0;
static int nextWaveScore = 10;

static Player player = { 0 };
static Enemy enemy[10] = { 0 };
static Shoot shoot[5] = { 0 };
static PowerUp powerUp = { 0 };
static bool powerUpActive = false;
static int powerUpDuration = 300; // 아이템 지속 시간 (프레임)

// 로컬 리더보드
typedef struct LocalLeaderboard {
    char name[30];
    int score;
} LocalLeaderboard;

#define MAX_LEADERBOARD_ENTRIES 10

static LocalLeaderboard leaderboard[MAX_LEADERBOARD_ENTRIES];

void InitLocalLeaderboard(void) {
    for (int i = 0; i < MAX_LEADERBOARD_ENTRIES; i++) {
        strcpy(leaderboard[i].name, "Player");
        leaderboard[i].score = 0;
    }
}

void SaveLocalLeaderboard(void) {
    FILE* file;
    if (fopen_s(&file, "leaderboard.dat", "wb") == 0) {
        fwrite(leaderboard, sizeof(LocalLeaderboard), MAX_LEADERBOARD_ENTRIES, file);
        fclose(file);
    }
}

void LoadLocalLeaderboard(void) {
    FILE* file;
    if (fopen_s(&file, "leaderboard.dat", "rb") == 0) {
        fread(leaderboard, sizeof(LocalLeaderboard), MAX_LEADERBOARD_ENTRIES, file);
        fclose(file);
    }
}

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
    player.color = BLANK;

    // Initialize enemies
    for (int i = 0; i < 10; i++) {
        enemy[i].rec.width = 10;
        enemy[i].rec.height = 10;
        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
        enemy[i].speed.x = 1 + wave;  // Increase enemy speed with each wave
        enemy[i].speed.y = 1 + wave;
        enemy[i].active = true;
        enemy[i].color = BLANK;
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

    // Initialize power-up item
    powerUp.rec.width = 15;
    powerUp.rec.height = 15;
    powerUp.active = false;
    powerUp.color = BLUE;

    InitLocalLeaderboard();
    LoadLocalLeaderboard();
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

            // 파워업 아이템 재생성
            if (!powerUp.active && !powerUpActive) {
                if (GetRandomValue(0, 1000) < 1) {
                    powerUp.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    powerUp.rec.y = GetRandomValue(0, screenHeight - powerUp.rec.height);
                    powerUp.active = true;
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

            // 파워업 아이템 충돌
            if (CheckCollisionRecs(player.rec, powerUp.rec) && powerUp.active) {
                powerUpActive = true;
                powerUp.active = false;
                powerUpDuration = 300;
            }

            // 파워업 아이템 활성화 시간 관리
            if (powerUpActive) {
                powerUpDuration--;
                if (powerUpDuration <= 0) {
                    powerUpActive = false;
                }
            }

            // 파워업 아이템 재생성
            if (!powerUp.active && !powerUpActive) {
                if (GetRandomValue(0, 1000) < 1) {
                    powerUp.rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                    powerUp.rec.y = GetRandomValue(0, screenHeight - powerUp.rec.height);
                    powerUp.active = true;
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
        // 텍스처를 사용하여 맵 그리기
        DrawTexture(mapTexture, 0, 0, WHITE);

        // 텍스처를 사용하여 플레이어 그리기
        DrawTexture(playerTexture, player.rec.x, player.rec.y, player.color);

        for (int i = 0; i < 10; i++) {
            if (enemy[i].active) {
                // 텍스처를 사용하여 적 그리기
                DrawTexture(enemyTexture, enemy[i].rec.x, enemy[i].rec.y, enemy[i].color);

                // 적의 히트 박스에 빨간색 테두리 그리기
                DrawRectangleLinesEx(enemy[i].rec, 2, RED);
            }
        }

        for (int i = 0; i < 5; i++) {
            if (shoot[i].active) {
                // 총알 그리기
                DrawRectangleRec(shoot[i].rec, shoot[i].color);
            }
        }

        // 파워업 아이템 그리기
        if (powerUp.active && !powerUpActive) {
            DrawRectangleRec(powerUp.rec, powerUp.color);
        }

        // 플레이어의 히트 박스에 초록색 테두리 그리기
        DrawRectangleLinesEx(player.rec, 2, GREEN);

        DrawText(TextFormat("Score: %04i", score), 20, 20, 30, GRAY);
        DrawText(TextFormat("Wave: %02i", wave), 20, 60, 30, GRAY);

        if (pause) DrawText("GAME PAUSED", screenWidth / 2 - MeasureText("GAME PAUSED", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
    }
    else {
        DrawText("GAME OVER - PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("GAME OVER - PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 20 * 18 - 50, 20, GRAY);

        // 리더보드 화면 표시
        DrawText("Local Leaderboard", 280, 20, 20, GRAY);
        for (int i = 0; i < MAX_LEADERBOARD_ENTRIES; i++) {
            DrawText(TextFormat("%d. %s: %d", i + 1, leaderboard[i].name, leaderboard[i].score), 280, 60 + i * 30, 20, GRAY);
        }
    }

    EndDrawing();
}


int main(void) {
    InitWindow(screenWidth, screenHeight, "Simple Space Invaders");

    InitGame();

    ClearBackground((Color) { 0, 0, 0, 0 }); // 투명 배경

    // Load images
    Image mapImage = LoadImage("Map.png");
    Image enemyImage = LoadImage("Enemy.png");
    Image playerImage = LoadImage("Player.png");

    mapTexture = LoadTextureFromImage(mapImage);
    enemyTexture = LoadTextureFromImage(enemyImage);
    playerTexture = LoadTextureFromImage(playerImage);

    UnloadImage(mapImage);
    UnloadImage(enemyImage);
    UnloadImage(playerImage);

    if (mapImage.data == NULL || enemyImage.data == NULL || playerImage.data == NULL) {
        // Failed to load the image(s)
        CloseWindow();
        return -1;
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateGame();
        DrawGame();
    }

    SaveLocalLeaderboard(); // 게임 종료 시 리더보드 저장

    CloseWindow();

    return 0;
}
