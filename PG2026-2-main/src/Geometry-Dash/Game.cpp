// Game.cpp - PESSOA 2 (lógica do jogo)
// ATENÇÃO: o conteúdo abaixo é só um ESBOÇO para o projeto rodar e para
// testar o motor de renderização. A Pessoa 2 substitui pela lógica real
// (pulo, gravidade, colisão AABB, game over, geração de obstáculos).
#include "Game.h"
#include "Config.h"

#include <GLFW/glfw3.h>

static const float PLAYER_SPEED    = 300.0f; // unidades de mundo por segundo
static const float PLAYER_SIZE     = 50.0f;
static const float PLAYER_SCREEN_X = 200.0f; // distância do jogador até a borda esquerda da tela

void Game::init(unsigned int playerTexture, unsigned int groundTexture, unsigned int obstacleTexture)
{
    // Jogador: apoiado no chão (position é o CENTRO do sprite)
    player.textureID = playerTexture;
    player.size = glm::vec2(PLAYER_SIZE, PLAYER_SIZE);
    player.position = glm::vec2(PLAYER_SCREEN_X, GROUND_HEIGHT + PLAYER_SIZE / 2.0f);
    player.color = glm::vec4(1.0f, 0.85f, 0.1f, 1.0f);

    // Chão: uma faixa da largura da tela
    ground.textureID = groundTexture;
    ground.size = glm::vec2(WORLD_WIDTH, GROUND_HEIGHT);
    ground.position = glm::vec2(WORLD_WIDTH / 2.0f, GROUND_HEIGHT / 2.0f);
    ground.color = glm::vec4(0.15f, 0.2f, 0.45f, 1.0f);

    // Obstáculos de exemplo, espalhados pela fase
    obstacles.clear();
    for (int i = 0; i < 20; i++)
    {
        Sprite obstacle;
        obstacle.textureID = obstacleTexture;
        obstacle.size = glm::vec2(40.0f, 40.0f + 20.0f * (i % 3));
        obstacle.position = glm::vec2(700.0f + 450.0f * i, GROUND_HEIGHT + obstacle.size.y / 2.0f);
        obstacle.color = glm::vec4(0.9f, 0.2f, 0.3f, 1.0f);
        obstacles.push_back(obstacle);
    }

    cameraX = 0.0f;
}

void Game::processInput(GLFWwindow *window)
{
    // TODO (Pessoa 2): pulo
    // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) { ... }
    (void)window;
}

void Game::update(float dt)
{
    // O jogador avança sozinho, como no Geometry Dash
    player.position.x += PLAYER_SPEED * dt;

    // Só para demonstrar a rotação da matriz de modelo.
    // TODO (Pessoa 2): girar apenas durante o pulo.
    player.rotation -= 180.0f * dt;

    // TODO (Pessoa 2): gravidade e pulo (position.y)
    // TODO (Pessoa 2): colisão AABB jogador x obstáculos (getPMin / getPMax)
    // TODO (Pessoa 2): player.updateAnimation(dt) quando houver spritesheet

    // A câmera acompanha o jogador, mantendo-o sempre no mesmo ponto da tela
    cameraX = player.position.x - PLAYER_SCREEN_X;

    // O chão acompanha a câmera para parecer infinito
    ground.position.x = cameraX + WORLD_WIDTH / 2.0f;
}
