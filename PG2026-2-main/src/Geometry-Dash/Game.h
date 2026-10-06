
// Guarda o estado do jogo e o atualiza a cada frame.
// Não desenha nada: só altera os Sprites, que o Renderer depois lê.
#pragma once

#include "Sprite.h"
#include <vector>

struct GLFWwindow;

class Game
{
public:
    // --- Estado lido pelo main na hora de desenhar ---
    Sprite player;
    Sprite ground;
    std::vector<Sprite> obstacles;
    float cameraX = 0.0f; // canto esquerdo da janela do mundo

    // --- Estado lido pelo main para HUD / título da janela (extras) ---
    bool gameOver = false;
    int score = 0; // distância percorrida (cameraX / 10, arredondado)

    // Monta a fase. Recebe os IDs das texturas já carregadas pelo main.
    void init(unsigned int playerTexture, unsigned int groundTexture, unsigned int obstacleTexture);

    // Lê teclado/mouse (pulo, reiniciar após game over).
    void processInput(GLFWwindow *window);

    // Atualiza física, colisões, animações e câmera. dt em segundos.
    void update(float dt);

private:
    // --- Física do pulo (ajuste os valores à vontade pra calibrar a sensação) ---
    static constexpr float GRAVITY = -2500.0f;     // px/s^2
    static constexpr float JUMP_VELOCITY = 900.0f; // px/s, impulso inicial do pulo

    // --- Layout do personagem na tela ---
    static constexpr float PLAYER_SCREEN_X = 150.0f; // distância fixa da borda esquerda da câmera
    static constexpr float PLAYER_SIZE = 40.0f;

    // --- Movimento automático (o "correr" do runner) ---
    static constexpr float SCROLL_SPEED = 320.0f; // px/s

    // --- Geração de obstáculos ---
    static constexpr float OBSTACLE_SIZE_W = 36.0f;
    static constexpr float OBSTACLE_SIZE_H = 50.0f;
    static constexpr float OBSTACLE_GAP_MIN = 260.0f;
    static constexpr float OBSTACLE_GAP_MAX = 480.0f;
    static constexpr float GROUND_WIDTH = 4000.0f; // bem maior que WORLD_WIDTH; reposicionado por frame

    unsigned int obstacleTextureID = 0;
    float nextObstacleX = 0.0f;
    float velocityY = 0.0f;
    bool onGround = true;
    bool spacePressedLastFrame = false; // para detectar a BORDA de subida da tecla (não repetir pulo ao segurar)

    void resetGame();
    void spawnObstacle();
    void updatePlayerPhysics(float dt);
    void updateObstacles(float dt);
    bool checkCollision(const Sprite &a, const Sprite &b) const;
};