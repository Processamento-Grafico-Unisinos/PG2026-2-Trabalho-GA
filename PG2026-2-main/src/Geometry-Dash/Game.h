// Game.h - PESSOA 2 (lógica do jogo)
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

    // Monta a fase. Recebe os IDs das texturas já carregadas pelo main.
    void init(unsigned int playerTexture, unsigned int groundTexture, unsigned int obstacleTexture);

    // Lê teclado/mouse (pulo etc.)
    void processInput(GLFWwindow *window);

    // Atualiza física, colisões, animações e câmera. dt em segundos.
    void update(float dt);
};
