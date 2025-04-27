#include <cmath>
#include "raylib.h"
#include <string>

// Oyun sabitleri
const int screenWidth = 1000;
const int screenHeight = 600;
const float gravity = 0.3f;
const float jumpForce = -12.0f;
const float moveSpeed = 7.0f;
const int attack1Damage = 10;
const int attack2Damage = 20;
const float attackRange = 50.0f;
const float attackWidth = 30.0f;
const float hurtDuration = 0.5f;
const float deathAnimDuration = 1.5f; // Ölüm animasyonu süresi
const float defenseDuration = 1.0f;   // Savunma süresi
const float defenseCooldown = 3.0f;   // Savunma bekleme süresi
const float attack1CooldownTime = 0.6f; // Saldırı bekleme süresi
const float attack2CooldownTime = 1.5f; // Saldırı bekleme süresi


// Oyun durumu
enum GameState { MENU,PLAYING, PLAYER1_KAZANDI, PLAYER2_KAZANDI, PLAYER1_OLDU, PLAYER2_OLDU };

// Oyuncu durumu
enum PlayerState { IDLE, RUNNING, JUMPING, ATTACKING1, ATTACKING2, HURT, DEAD, DEFENDING };

struct Player {
    Vector2 position;
    Vector2 velocity;
    float width;
    float height;
    PlayerState state;
    float hurtTimer;
    float deathTimer;
    int health;
    int facing;
    Texture2D idleTexture;
    Texture2D runTexture;
    Texture2D jumpTexture;
    Texture2D attack1Texture;
    Texture2D attack2Texture;
    Texture2D hurtTexture;
    Texture2D deadTexture;
    Texture2D defenseTexture;
    Rectangle frameRec;
    int currentFrame;
    int framesCounter;
    float attackCooldown;
    float attackTimer;
    float defenseTimer;
    float defenseCooldownTimer;
    bool canHit;
    bool isDefending;
};

bool CheckAttackHit(Player attacker, Player target) {
    if (!attacker.canHit || attacker.attackTimer < 0.1f || target.state == DEAD)
        return false;

    // Eğer hedef savunma yapıyorsa ve saldırıyı karşılıyorsa hasar verme
    if (target.isDefending &&
        ((attacker.facing == 1 && target.facing == -1) ||
         (attacker.facing == -1 && target.facing == 1))) {
        return false;
    }

    Rectangle attackArea;
    if (attacker.facing == 1) {
        attackArea = { attacker.position.x + attacker.width,
                      attacker.position.y + attacker.height/4,
                      attackRange,
                      attackWidth };
    } else {
        attackArea = { attacker.position.x - attackRange,
                      attacker.position.y + attacker.height/4,
                      attackRange,
                      attackWidth };
    }

    Rectangle targetBody = { target.position.x,
                            target.position.y,
                            target.width,
                            target.height };

    return CheckCollisionRecs(attackArea, targetBody);
}

void ResetPlayers(Player &player1, Player &player2) {
    player1.position = { 100, screenHeight - 100 };
    player1.velocity = { 0, 0 };
    player1.state = IDLE;
    player1.hurtTimer = 0;
    player1.deathTimer = 0;
    player1.health = 100;
    player1.facing = 1;
    player1.currentFrame = 0;
    player1.framesCounter = 0;
    player1.attackCooldown = 0;
    player1.attackTimer = 0;
    player1.defenseTimer = 0;
    player1.defenseCooldownTimer = 0;
    player1.canHit = false;
    player1.isDefending = false;

    player2.position = { 700, screenHeight - 100 };
    player2.velocity = { 0, 0 };
    player2.state = IDLE;
    player2.hurtTimer = 0;
    player2.deathTimer = 0;
    player2.health = 100;
    player2.facing = -1;
    player2.currentFrame = 0;
    player2.framesCounter = 0;
    player2.attackCooldown = 0;
    player2.attackTimer = 0;
    player2.defenseTimer = 0;
    player2.defenseCooldownTimer = 0;
    player2.canHit = false;
    player2.isDefending = false;
}

void DrawPlayerTexture(Texture2D texture, Rectangle frameRec, Vector2 position, int facing, float textureWidth,int playerID) {
    if (facing == 1 ) {
        // Sağa bakan normal çizim
        DrawTextureRec(texture, frameRec, position, WHITE);
    }else if (facing == -1 && playerID == 1) {
        // Sola bakan karakter için özel çizim
        Rectangle flippedRec = {
            frameRec.x + frameRec.width,
            frameRec.y,
            -frameRec.width,              // Negatif genişlik = yansıtma
            frameRec.height
        };
        DrawTextureRec(texture, flippedRec, position, WHITE);
    }else {
        //Player 2 için özel dönme
        Rectangle flippedRec = {
            frameRec.x + frameRec.width+18,
            frameRec.y,
            -frameRec.width,              // Negatif genişlik = yansıtma
            frameRec.height
        };
        DrawTextureRec(texture, flippedRec, position, WHITE);
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "2 Player Samurai Battle");

    Texture2D menuBackground = LoadTexture("assets/menuBackground.jpg");
    Texture2D background = LoadTexture("assets/Background.png");

    // Ses sistemini başlat
    InitAudioDevice();

    // Arkaplan müziği
    Music bgMusic = LoadMusicStream("assets/battle_music.mp3");
    SetMusicVolume(bgMusic,0.05f);

    // Ses efektleri
    Sound swingSound = LoadSound("assets/swing.wav");
    Sound hitSound = LoadSound("assets/hit.mp3");
    Sound player1hitah1Sound = LoadSound("assets/player1_hit1.mp3");
    Sound player1hitah2Sound = LoadSound("assets/player1_hit2.mp3");
    Sound player1DeadSound = LoadSound("assets/player1_dead.mp3");
    Sound player2hitah1Sound = LoadSound("assets/player2_hit1.mp3");
    Sound player2hitah2Sound = LoadSound("assets/player2_hit2.mp3");
    Sound player2DeadSound = LoadSound("assets/player2_dead.mp3");
    Sound baslayalim = LoadSound("assets/baslayalim.mp3");
    SetSoundVolume(hitSound, 2.5f);

    // Müziği çalmaya başlat
    PlayMusicStream(bgMusic);

    float bgScale = fmax((float)screenWidth/background.width, (float)screenHeight/background.height);

    Player player1;
    player1.width = 50;
    player1.height = 100;
    player1.idleTexture = LoadTexture("assets/player1_Idle.png");
    player1.runTexture = LoadTexture("assets/player1_Run.png");
    player1.jumpTexture = LoadTexture("assets/player1_Jump.png");
    player1.attack1Texture = LoadTexture("assets/player1_Attack_1.png");
    player1.attack2Texture = LoadTexture("assets/player1_Attack_2.png");
    player1.hurtTexture = LoadTexture("assets/player1_Hurt.png");
    player1.deadTexture = LoadTexture("assets/player1_Dead.png");
    player1.defenseTexture = LoadTexture("assets/player1_Protection.png");
    player1.frameRec = { 0, 0, (float)player1.idleTexture.width/6, (float)player1.idleTexture.height };

    Player player2;
    player2.width = 50;
    player2.height = 100;
    player2.idleTexture = LoadTexture("assets/player2_Idle.png");
    player2.runTexture = LoadTexture("assets/player2_Run.png");
    player2.jumpTexture = LoadTexture("assets/player2_Jump.png");
    player2.attack1Texture = LoadTexture("assets/player2_Attack_1.png");
    player2.attack2Texture = LoadTexture("assets/player2_Attack_2.png");
    player2.hurtTexture = LoadTexture("assets/player2_Hurt.png");
    player2.deadTexture = LoadTexture("assets/player2_Dead.png");
    player2.defenseTexture = LoadTexture("assets/player2_Protect.png");
    player2.frameRec = { 0, 0, (float)player2.idleTexture.width/6, (float)player2.idleTexture.height };

    GameState gameState = MENU;
    ResetPlayers(player1, player2);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        bool dialog = false;
        UpdateMusicStream(bgMusic);
        float deltaTime = GetFrameTime();

        // Oyunu yeniden başlat
        if ((gameState == PLAYER1_KAZANDI || gameState == PLAYER2_KAZANDI ||
             gameState == PLAYER1_OLDU || gameState == PLAYER2_OLDU) &&
            IsKeyPressed(KEY_SPACE)) {
            gameState = PLAYING;
            PlaySound(baslayalim);
            ResetPlayers(player1, player2);
            StopMusicStream(bgMusic);    // Müziği durdur
            PlayMusicStream(bgMusic);    // Yeniden başlat
            }
        if (gameState == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                gameState = PLAYING;
                PlaySound(baslayalim);
            }
        }


        if (gameState == PLAYING && gameState != MENU ) {

            // Oyuncu 1 kontrolleri
            if (player1.state != DEAD) {
                if (IsKeyDown(KEY_A)) {
                    player1.velocity.x = -moveSpeed;
                    player1.facing = -1;
                    if (player1.state != JUMPING && player1.state != ATTACKING1 && player1.state != ATTACKING2 &&
                        player1.state != HURT && player1.state != DEFENDING) {
                        player1.state = RUNNING;
                        }
                }
                else if (IsKeyDown(KEY_D)) {
                    player1.velocity.x = moveSpeed;
                    player1.facing = 1;
                    if (player1.state != JUMPING && player1.state != ATTACKING1 && player1.state != ATTACKING2 &&
                        player1.state != HURT && player1.state != DEFENDING) {
                        player1.state = RUNNING;
                        }
                }
                else {
                    player1.velocity.x = 0;
                    if (player1.state == RUNNING && player1.state != DEFENDING) player1.state = IDLE;
                }

                if (IsKeyPressed(KEY_W) && player1.state != JUMPING && player1.state != ATTACKING1 &&
                    player1.state != ATTACKING2 && player1.state != HURT && player1.state != DEFENDING) {
                    player1.velocity.y = jumpForce;
                    player1.state = JUMPING;
                    }

                // Oyuncu 1 savunma
                if (player1.defenseCooldownTimer <= 0 && IsKeyPressed(KEY_H) &&
                    player1.state != JUMPING && player1.state != HURT &&
                    player1.state != ATTACKING1 && player1.state != ATTACKING2) {
                    player1.state = DEFENDING;
                    player1.isDefending = true;
                    player1.defenseTimer = defenseDuration;
                    player1.defenseCooldownTimer = defenseCooldown;
                    }

                // Oyuncu 1 saldırı 1
                if (player1.attackCooldown <= 0 && IsKeyPressed(KEY_G) &&
                    player1.state != JUMPING && player1.state != HURT &&
                    player1.state != ATTACKING2 && player1.state != DEFENDING) {
                    player1.state = ATTACKING1;
                    PlaySound(player1hitah2Sound);
                    PlaySound(swingSound);

                    player1.currentFrame = 0;
                    player1.attackCooldown = attack1CooldownTime;
                    player1.attackTimer = 0;
                    player1.canHit = true;
                    }

                // Oyuncu 1 saldırı 2
                if (player1.attackCooldown <= 0 && IsKeyPressed(KEY_F) &&
                   player1.state != JUMPING && player1.state != HURT &&
                   player1.state != ATTACKING1 && player1.state != DEFENDING) {
                    player1.state = ATTACKING2;
                    PlaySound(player1hitah1Sound);
                    PlaySound(swingSound);

                    player1.currentFrame = 0;
                    player1.attackCooldown = attack2CooldownTime;
                    player1.attackTimer = 0;
                    player1.canHit = true;
                   }
            }

            // Oyuncu 2 kontrolleri
            if (player2.state != DEAD) {


                if (IsKeyDown(KEY_LEFT)) {
                    player2.velocity.x = -moveSpeed;
                    player2.facing = -1;
                    if (player2.state != JUMPING && player2.state != ATTACKING1 && player2.state != ATTACKING2 &&
                        player2.state != HURT && player2.state != DEFENDING) {
                        player2.state = RUNNING;
                        }
                }
                else if (IsKeyDown(KEY_RIGHT)) {
                    player2.velocity.x = moveSpeed;
                    player2.facing = 1;
                    if (player2.state != JUMPING && player2.state != ATTACKING1 && player2.state != ATTACKING2 &&
                        player2.state != HURT && player2.state != DEFENDING) {
                        player2.state = RUNNING;
                        }
                }
                else {
                    player2.velocity.x = 0;
                    if (player2.state == RUNNING && player2.state != DEFENDING) player2.state = IDLE;
                }

                if (IsKeyPressed(KEY_UP) && player2.state != JUMPING && player2.state != ATTACKING1 &&
                    player2.state != ATTACKING2 && player2.state != HURT && player2.state != DEFENDING) {
                    player2.velocity.y = jumpForce;
                    player2.state = JUMPING;
                    }

                // Oyuncu 2 savunma
                if (player2.defenseCooldownTimer <= 0 && IsKeyPressed(KEY_O) &&
                    player2.state != JUMPING && player2.state != HURT &&
                    player2.state != ATTACKING1 && player2.state != ATTACKING2) {
                    player2.state = DEFENDING;
                    player2.isDefending = true;
                    player2.defenseTimer = defenseDuration;
                    player2.defenseCooldownTimer = defenseCooldown;
                    }

                // Oyuncu 2 saldırı 1
                if (player2.attackCooldown <= 0 && IsKeyPressed(KEY_K) &&
                    player2.state != JUMPING && player2.state != HURT &&
                    player2.state != ATTACKING2 && player2.state != DEFENDING) {
                    player2.state = ATTACKING1;
                    PlaySound(player2hitah1Sound);
                    PlaySound(swingSound);

                    player2.currentFrame = 0;
                    player2.attackCooldown = attack1CooldownTime;
                    player2.attackTimer = 0;
                    player2.canHit = true;
                    }

                // Oyuncu 2 saldırı 2
                if (player2.attackCooldown <= 0 && IsKeyPressed(KEY_L) &&
                   player2.state != JUMPING && player2.state != HURT &&
                   player2.state != ATTACKING1 && player2.state != DEFENDING) {
                    player2.state = ATTACKING2;
                    PlaySound(player2hitah2Sound);
                    PlaySound(swingSound);

                    player2.currentFrame = 0;
                    player2.attackCooldown = attack2CooldownTime;
                    player2.attackTimer = 0;
                    player2.canHit = true;
                   }
            }

            // Savunma zamanlayıcıları
            if (player1.isDefending) {
                player1.defenseTimer -= deltaTime;
                if (player1.defenseTimer <= 0) {
                    player1.isDefending = false;
                    player1.state = IDLE;
                }
            }

            if (player2.isDefending) {
                player2.defenseTimer -= deltaTime;
                if (player2.defenseTimer <= 0) {
                    player2.isDefending = false;
                    player2.state = IDLE;
                }
            }

            // Savunma bekleme süreleri
            if (player1.defenseCooldownTimer > 0) player1.defenseCooldownTimer -= deltaTime;
            if (player2.defenseCooldownTimer > 0) player2.defenseCooldownTimer -= deltaTime;

            // Saldırı zamanlayıcıları
            if (player1.state == ATTACKING1 ) {
                player1.attackTimer += deltaTime;
                if (player1.attackTimer > 0.5f) {
                    player1.canHit = false;
                }
                if (player1.attackTimer > attack1CooldownTime) {
                    player1.state = IDLE;
                }
            }
            // Saldırı zamanlayıcıları
            if (player1.state == ATTACKING2) {
                player1.attackTimer += deltaTime;
                if (player1.attackTimer > 0.5f) {
                    player1.canHit = false;
                }
                if (player1.attackTimer > attack2CooldownTime) {
                    player1.state = IDLE;
                }
            }

            if (player2.state == ATTACKING1 ) {
                player2.attackTimer += deltaTime;
                if (player2.attackTimer > 0.5f) {
                    player2.canHit = false;
                }
                if (player2.attackTimer > attack1CooldownTime) {
                    player2.state = IDLE;
                }
            }
            if (player2.state == ATTACKING2) {
                player2.attackTimer += deltaTime;
                if (player2.attackTimer > 0.5f) {
                    player2.canHit = false;
                }
                if (player2.attackTimer > attack2CooldownTime) {
                    player2.state = IDLE;
                }
            }

            // Saldırı bekleme süreleri
            if (player1.attackCooldown > 0) player1.attackCooldown -= deltaTime;
            if (player2.attackCooldown > 0) player2.attackCooldown -= deltaTime;

            // Vuruş kontrolü
            if (CheckAttackHit(player1, player2) && player2.state != HURT && player2.state != DEAD && player1.state != ATTACKING2) {
                player2.health -= attack1Damage;
                player2.state = HURT;
                PlaySound(hitSound);
                player2.hurtTimer = hurtDuration;
                player1.canHit = false;
                player2.velocity.x = player1.facing * 8.0f;
                player2.velocity.y = -4.0f;

                if (player2.health <= 0) {
                    PlaySound(player2DeadSound);

                    player2.health = 0;
                    player2.deathTimer = deathAnimDuration;
                    player2.state = DEAD;
                }
            }
            if (CheckAttackHit(player1, player2) && player2.state != HURT && player2.state != DEAD && player1.state != ATTACKING1) {
                player2.health -= attack2Damage;
                player2.state = HURT;
                PlaySound(hitSound);
                player2.hurtTimer = hurtDuration;
                player1.canHit = false;
                player2.velocity.x = player1.facing * 10.0f;
                player2.velocity.y = -4.0f;

                if (player2.health <= 0) {
                    PlaySound(player2DeadSound);

                    player2.health = 0;
                    player2.deathTimer = deathAnimDuration;
                    player2.state = DEAD;
                }
            }

            if (CheckAttackHit(player2, player1) && player1.state != HURT && player1.state != DEAD && player2.state != ATTACKING2) {
                player1.health -= attack1Damage;
                player1.state = HURT;
                PlaySound(hitSound);
                player1.hurtTimer = hurtDuration;
                player2.canHit = false;
                player1.velocity.x = player2.facing * 8.0f;
                player1.velocity.y = -4.0f;

                if (player1.health <= 0) {
                    PlaySound(player1DeadSound);
                    PlaySound(player1DeadSound);

                    player1.health = 0;
                    player1.deathTimer = deathAnimDuration;
                    player1.state = DEAD;
                }
            }
            if (CheckAttackHit(player2, player1) && player1.state != HURT && player1.state != DEAD && player2.state != ATTACKING1) {
                player1.health -= attack2Damage;
                player1.state = HURT;
                PlaySound(hitSound);
                player1.hurtTimer = hurtDuration;
                player2.canHit = false;
                player1.velocity.x = player2.facing * 8.0f;
                player1.velocity.y = -4.0f;

                if (player1.health <= 0) {
                    PlaySound(player1DeadSound);
                    player1.health = 0;
                    player1.deathTimer = deathAnimDuration;
                    player1.state = DEAD;
                }
            }

            // Hurt durumunu güncelle
            if (player1.state == HURT) {
                player1.hurtTimer -= deltaTime;
                if (player1.hurtTimer <= 0) {
                    player1.state = IDLE;
                }
            }

            if (player2.state == HURT) {
                player2.hurtTimer -= deltaTime;
                if (player2.hurtTimer <= 0) {
                    player2.state = IDLE;
                }
            }

            // Ölüm animasyonu
            if (player1.state == DEAD) {
                player1.deathTimer -= deltaTime;
                if (player1.deathTimer <= 0) {
                    gameState = PLAYER2_KAZANDI;
                }
            }

            if (player2.state == DEAD) {
                player2.deathTimer -= deltaTime;
                if (player2.deathTimer <= 0) {
                    gameState = PLAYER1_KAZANDI;
                }
            }

            // Fizik güncellemeleri
            player1.velocity.y += gravity;
            player2.velocity.y += gravity;

            player1.position.x += player1.velocity.x;
            player1.position.y += player1.velocity.y;

            player2.position.x += player2.velocity.x;
            player2.position.y += player2.velocity.y;

            // Zemin kontrolü
            if (player1.position.y > screenHeight - 350 ) {
                player1.position.y = screenHeight - 350;
                player1.velocity.y = 0;
                if (player1.state == JUMPING) player1.state = IDLE;
            }

            if (player2.position.y > screenHeight - 350 ) {
                player2.position.y = screenHeight - 350;
                player2.velocity.y = 0;
                if (player2.state == JUMPING) player2.state = IDLE;
            }

            // Sınır kontrolü
            if (player1.position.x < 0) player1.position.x = 0;
            if (player1.position.x > screenWidth - player1.width) player1.position.x = screenWidth - player1.width;

            if (player2.position.x < 0) player2.position.x = 0;
            if (player2.position.x > screenWidth - player2.width) player2.position.x = screenWidth - player2.width;

            // Animasyon güncellemeleri
            player1.framesCounter++;
            player2.framesCounter++;

            // Oyuncu 1 animasyon
            if (player1.framesCounter >= 10) {
                player1.framesCounter = 0;
                player1.currentFrame++;

                int maxFrame = 5;
                if (player1.state == ATTACKING1) maxFrame = 5;
                else if (player1.state == ATTACKING2) maxFrame = 5;
                else if (player1.state == DEFENDING) maxFrame = 5;
                else if (player1.state == DEAD) maxFrame = 5;

                if (player1.currentFrame > maxFrame) {
                    player1.currentFrame = 0;
                    if (player1.state == DEAD) player1.currentFrame = maxFrame; // Son karede kal
                    if (player1.state == DEFENDING && player1.defenseTimer <= 0) {
                        player1.state = IDLE;
                    }
                }

                player1.frameRec.x = (float)player1.currentFrame * (float)player1.idleTexture.width/6;
            }

            // Oyuncu 2 animasyon
            if (player2.framesCounter >= 10) {
                player2.framesCounter = 0;
                player2.currentFrame++;

                int maxFrame = 5;
                if (player2.state == ATTACKING1) maxFrame = 4;
                else if (player2.state == ATTACKING2) maxFrame = 5;
                else if (player2.state == DEFENDING) maxFrame = 5;
                else if (player2.state == DEAD) maxFrame = 6;

                if (player2.currentFrame > maxFrame) {
                    player2.currentFrame = 0;
                    if (player2.state == DEAD) player2.currentFrame = maxFrame;
                    if (player2.state == DEFENDING && player2.defenseTimer <= 0) {
                        player2.state = IDLE;
                    }
                }

                player2.frameRec.x = (float)player2.currentFrame * (float)player2.idleTexture.width/5;
            }
        }

        // Çizim
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (gameState == MENU) {
            SetWindowSize(menuBackground.width, menuBackground.height);
            DrawTextureEx(menuBackground, {0, 0}, 0, 1, WHITE);
        }

        // Oyuncuları çiz
        if (gameState == PLAYING || gameState == PLAYER1_KAZANDI || gameState == PLAYER2_KAZANDI) {

            SetWindowSize(1000, 600);
            DrawTextureEx(background, {0, 0}, 0, bgScale, WHITE);
            // Oyuncu 1 için offset hesapla
            float p1Offset = 0;
            Texture2D p1Texture = player1.idleTexture;

            if (player1.state == DEAD) {
                p1Texture = player1.deadTexture;
                p1Offset = (player1.facing == -1) ? player1.deadTexture.width/6 - player1.width : 0;
            }
            else if (player1.state == HURT) {
                p1Texture = player1.hurtTexture;
                p1Offset = (player1.facing == -1) ? player1.hurtTexture.width/6 - player1.width : 0;
            }
            else if (player1.state == ATTACKING1) {
                p1Texture = player1.attack1Texture;
                p1Offset = (player1.facing == -1) ? player1.attack1Texture.width/4 - player1.width : 0;
            }
            else if (player1.state == ATTACKING2) {
                p1Texture = player1.attack2Texture;
                p1Offset = (player1.facing == -1) ? player1.attack2Texture.width/5 - player1.width : 0;
            }
            else if (player1.state == JUMPING) {
                p1Texture = player1.jumpTexture;
                p1Offset = (player1.facing == -1) ? player1.jumpTexture.width/6 - player1.width : 0;
            }
            else if (player1.state == RUNNING) {
                p1Texture = player1.runTexture;
                p1Offset = (player1.facing == -1) ? player1.runTexture.width/6 - player1.width : 0;
            }
            else if (player1.state == DEFENDING) {
                p1Texture = player1.defenseTexture;
                p1Offset = (player1.facing == -1) ? player1.defenseTexture.width/6 - player1.width : 0;
            }
            else {
                p1Texture = player1.idleTexture;
                p1Offset = (player1.facing == -1) ? player1.idleTexture.width/6 - player1.width : 0;
            }

            // Oyuncu 1 çizimi
            DrawPlayerTexture(p1Texture, player1.frameRec,
                            {player1.position.x, player1.position.y},
                            player1.facing, p1Offset,1);

            // Oyuncu 2 için offset hesapla
            float p2Offset = 0;
            Texture2D p2Texture = player2.idleTexture;

            if (player2.state == DEAD) {
                p2Texture = player2.deadTexture;
                p2Offset = (player2.facing == -1) ? player2.deadTexture.width/6 - player2.width : 0;
            }
            else if (player2.state == HURT) {
                p2Texture = player2.hurtTexture;
                p2Offset = (player2.facing == -1) ? player2.hurtTexture.width/6 - player2.width : 0;
            }
            else if (player2.state == ATTACKING1) {
                p2Texture = player2.attack1Texture;
                p2Offset = (player2.facing == -1) ? player2.attack1Texture.width/4 - player2.width : 0;
            }
            else if (player2.state == ATTACKING2) {
                p2Texture = player2.attack2Texture;
                p2Offset = (player2.facing == -1) ? player2.attack2Texture.width/5 - player2.width : 0;
            }
            else if (player2.state == JUMPING) {
                p2Texture = player2.jumpTexture;
                p2Offset = (player2.facing == -1) ? player2.jumpTexture.width/6 - player2.width : 0;
            }
            else if (player2.state == RUNNING) {
                p2Texture = player2.runTexture;
                p2Offset = (player2.facing == -1) ? player2.runTexture.width/6 - player2.width : 0;
            }
            else if (player2.state == DEFENDING) {
                p2Texture = player2.defenseTexture;
                p2Offset = (player2.facing == -1) ? player2.defenseTexture.width/6 - player2.width : 0;
            }
            else {
                p2Texture = player2.idleTexture;
                p2Offset = (player2.facing == -1) ? player2.idleTexture.width/6 - player2.width : 0;
            }

            // Oyuncu 2 çizimi
            DrawPlayerTexture(p2Texture, player2.frameRec,
                            {player2.position.x, player2.position.y},
                            player2.facing, p2Offset,2);

            // Hasar göstergeleri
            if (player1.state == HURT) {
                DrawText(TextFormat("-%d",player2.state == ATTACKING1 ? attack1Damage : attack2Damage), player1.position.x, player1.position.y - 30, 20, RED);
            }
            if (player2.state == HURT) {
                DrawText(TextFormat("-%d", player1.state == ATTACKING1 ? attack1Damage : attack2Damage), player2.position.x, player2.position.y - 30, 20, RED);
            }
        }
        if (gameState == PLAYING){
        // Sağlık göstergeleri
        DrawRectangle(20, 20, player1.health * 2, 20, RED);
        DrawRectangle(screenWidth - 20 - player2.health * 2, 20, player2.health * 2, 20, BLUE);
    }
            // Savunma bekleme süreleri göster
            if (player1.defenseCooldownTimer > 0) {
                float cooldownPercent = player1.defenseCooldownTimer / defenseCooldown;
                DrawRectangle(20, 50, 200 * (1 - cooldownPercent), 10, GREEN);
                DrawRectangleLines(20, 50, 200, 10, DARKGREEN);
            }

            if (player2.defenseCooldownTimer > 0) {
                float cooldownPercent = player2.defenseCooldownTimer / defenseCooldown;
                DrawRectangle(screenWidth - 220, 50, 200 * (1 - cooldownPercent), 10, GREEN);
                DrawRectangleLines(screenWidth - 220, 50, 200, 10, DARKGREEN);
            }

            // Oyun sonu ekranı
            if (gameState == PLAYER1_KAZANDI || gameState == PLAYER2_KAZANDI) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 200});

                const char* winnerText = (gameState == PLAYER1_KAZANDI) ? "PLAYER 1 KAZANDI!" : "PLAYER 2 KAZANDI!";
                Color winnerColor = (gameState == PLAYER1_KAZANDI) ? RED : BLUE;

                DrawText(winnerText, screenWidth/2 - MeasureText(winnerText, 40)/2, screenHeight/2 - 50, 40, winnerColor);
                DrawText("Tekrar oynamak icin SPACE tusuna basin",
                        screenWidth/2 - MeasureText("Tekrar oynamak icin SPACE tusuna basin", 20)/2,
                        screenHeight/2 + 20, 20, WHITE);
            }
            if (gameState == MENU) {
                DrawText("Baslamak icin ENTER'e Basin",menuBackground.width-270, menuBackground.height  - 200, 18, BLACK);
            }
            // Kontrolleri göster
            if (gameState == PLAYING) {

                DrawText("Player 1: W-A-D  Hareket, G-F Saldiri, H  Savunma", 10, screenHeight - 30, 15, WHITE);
                DrawText("Player 2: Yön Tuslari Hareket, K-L Saldiri, O Savunma", screenWidth-400, screenHeight - 30, 15, WHITE);
            }

        EndDrawing();
    }

    // Kaynakları boşalt
    UnloadTexture(player1.idleTexture);
    UnloadTexture(player1.runTexture);
    UnloadTexture(player1.jumpTexture);
    UnloadTexture(player1.attack1Texture);
    UnloadTexture(player1.attack2Texture);
    UnloadTexture(player1.hurtTexture);
    UnloadTexture(player1.deadTexture);
    UnloadTexture(player1.defenseTexture);
    UnloadTexture(player2.idleTexture);
    UnloadTexture(player2.runTexture);
    UnloadTexture(player2.jumpTexture);
    UnloadTexture(player2.attack1Texture);
    UnloadTexture(player2.attack2Texture);
    UnloadTexture(player2.hurtTexture);
    UnloadTexture(player2.deadTexture);
    UnloadTexture(player2.defenseTexture);
    UnloadTexture(background);
    UnloadTexture(menuBackground);
    UnloadMusicStream(bgMusic);
    UnloadSound(swingSound);
    UnloadSound(hitSound);
    UnloadSound(player1hitah1Sound);
    UnloadSound(player1hitah2Sound);
    UnloadSound(player2hitah1Sound);
    UnloadSound(player2hitah2Sound);
    CloseAudioDevice();

    CloseWindow();

    return 0;
}