#include <cmath>
#include "raylib.h"


const int screenWidth = 1000;
const int screenHeight = 600;
const float gravity = 0.5f;
const float jumpForce = -12.0f;
const float moveSpeed = 5.0f;

// Oyuncu DurumLarı
enum PlayerState { IDLE, RUNNING, JUMPING };

struct Player {
    Vector2 position;
    Vector2 velocity;
    float width;
    float height;
    PlayerState state;
    int facing;
    Texture2D idleTexture;
    Texture2D runTexture;
    Texture2D jumpTexture;
    Rectangle frameRec;
    int currentFrame;
    int framesCounter;
};

void DrawPlayerTexture(Texture2D texture, Rectangle frameRec, Vector2 position, int facing) {
    if (facing == 1) {
        // Sağa bakan normal çizim
        DrawTextureRec(texture, frameRec, position, WHITE);
    } else {
        // Sola bakan karakter için özel çizim
        Rectangle flippedRec = {
            frameRec.x + frameRec.width,
            frameRec.y,
            -frameRec.width,
            frameRec.height
        };
        DrawTextureRec(texture, flippedRec, position, WHITE);
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "2Player Samurai Battle");

    Texture2D background = LoadTexture("C:/Users/canavarPC/CLionProjects/2-player-samurai-battle/assets/background.png");
    float bgScale = fmax((float)screenWidth/background.width, (float)screenHeight/background.height);

    Player player;
    player.width = 50;
    player.height = 100;
    player.idleTexture = LoadTexture("C:/Users/canavarPC/CLionProjects/2-player-samurai-battle/assets/player1_Idle.png");
    player.runTexture = LoadTexture("C:/Users/canavarPC/CLionProjects/2-player-samurai-battle/assets/player1_Run.png");
    player.jumpTexture = LoadTexture("C:/Users/canavarPC/CLionProjects/2-player-samurai-battle/assets/player1_Jump.png");
    player.frameRec = { 0, 0, (float)player.idleTexture.width/6, (float)player.idleTexture.height };

    // Oyuncu başlangıç pozisyonu
    player.position = { 100, screenHeight - 100 };
    player.velocity = { 0, 0 };
    player.state = IDLE;
    player.facing = 1;
    player.currentFrame = 0;
    player.framesCounter = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {


        // Kontrolleri
        if (IsKeyDown(KEY_A)) {
            player.velocity.x = -moveSpeed;
            player.facing = -1;
            if (player.state != JUMPING) {
                player.state = RUNNING;
            }
        }
        else if (IsKeyDown(KEY_D)) {
            player.velocity.x = moveSpeed;
            player.facing = 1;
            if (player.state != JUMPING) {
                player.state = RUNNING;
            }
        }
        else {
            player.velocity.x = 0;
            if (player.state == RUNNING) player.state = IDLE;
        }

        if (IsKeyPressed(KEY_W) && player.state != JUMPING) {
            player.velocity.y = jumpForce;
            player.state = JUMPING;
        }

        // Fizik güncellemeleri
        player.velocity.y += gravity;
        player.position.x += player.velocity.x;
        player.position.y += player.velocity.y;

        // Zemin kontrolü
        if (player.position.y > screenHeight - 350) {
            player.position.y = screenHeight - 350;
            player.velocity.y = 0;
            if (player.state == JUMPING) player.state = IDLE;
        }

        // Sınır kontrolü
        if (player.position.x < 0) player.position.x = 0;
        if (player.position.x > screenWidth - player.width*2) player.position.x = screenWidth - player.width*2;

        // Animasyon güncellemeleri
        player.framesCounter++;
        if (player.framesCounter >= 10) {
            player.framesCounter = 0;
            player.currentFrame++;

            int maxFrame = 5;
            if (player.currentFrame > maxFrame) {
                player.currentFrame = 0;
            }

            player.frameRec.x = (float)player.currentFrame * (float)player.idleTexture.width/6;
        }

        // Çizim
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Arkaplan çiz
            DrawTextureEx(background, {0, 0}, 0, bgScale, WHITE);

            // Oyuncu Texture Seçimi
            Texture2D currentTexture = player.idleTexture;
            if (player.state == RUNNING) currentTexture = player.runTexture;
            else if (player.state == JUMPING) currentTexture = player.jumpTexture;

            // Oyuncuyu çiz
            DrawPlayerTexture(currentTexture, player.frameRec,
                            {player.position.x, player.position.y},
                            player.facing);

            // Kontrolleri göster
            DrawText("W: Ziplama, A: Sola git, D: Saga git", 10, screenHeight - 30, 20, BLACK);

        EndDrawing();
    }

    // Kaynakları boşalt
    UnloadTexture(player.idleTexture);
    UnloadTexture(player.runTexture);
    UnloadTexture(player.jumpTexture);
    UnloadTexture(background);

    CloseWindow();

    return 0;
}