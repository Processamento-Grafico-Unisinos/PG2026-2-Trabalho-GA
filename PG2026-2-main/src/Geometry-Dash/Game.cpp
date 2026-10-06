
#include "Game.h"
#include "Config.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdlib>

void Game::init(unsigned int playerTexture, unsigned int groundTexture, unsigned int obstacleTexture)
{
    player.textureID = playerTexture;
    player.size = glm::vec2(PLAYER_SIZE, PLAYER_SIZE);

    ground.textureID = groundTexture;
    ground.size = glm::vec2(GROUND_WIDTH, GROUND_HEIGHT);

    obstacleTextureID = obstacleTexture;

    srand(1234); // semente fixa (fases iguais em todo teste); troque por (unsigned)time(NULL) se quiser variar

    resetGame();
}

void Game::resetGame()
{
    cameraX = 0.0f;
    gameOver = false;
    score = 0;

    velocityY = 0.0f;
    onGround = true;
    player.position = glm::vec2(PLAYER_SCREEN_X, GROUND_HEIGHT + player.size.y * 0.5f);

    obstacles.clear();
    nextObstacleX = 900.0f; // primeiro obstáculo um pouco além da borda direita inicial
    for (int i = 0; i < 5; i++)
        spawnObstacle();
}

void Game::spawnObstacle()
{
    Sprite obstacle;
    obstacle.textureID = obstacleTextureID;
    obstacle.size = glm::vec2(OBSTACLE_SIZE_W, OBSTACLE_SIZE_H);
    obstacle.position = glm::vec2(nextObstacleX, GROUND_HEIGHT + obstacle.size.y * 0.5f);
    obstacle.color = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f); // vermelho (ALTERADO)
    obstacles.push_back(obstacle);

    float gap = OBSTACLE_GAP_MIN + static_cast<float>(rand()) / RAND_MAX * (OBSTACLE_GAP_MAX - OBSTACLE_GAP_MIN);
    nextObstacleX += gap;
}

void Game::processInput(GLFWwindow *window)
{
    bool spacePressedNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    if (gameOver)
    {
        // Borda de subida: só reinicia no instante em que a tecla é pressionada
        if (spacePressedNow && !spacePressedLastFrame)
            resetGame();
    }
    else if (spacePressedNow && !spacePressedLastFrame && onGround)
    {
        velocityY = JUMP_VELOCITY;
        onGround = false;
    }

    spacePressedLastFrame = spacePressedNow;
}

void Game::update(float dt)
{
    if (gameOver)
        return;

    cameraX += SCROLL_SPEED * dt;
    score = static_cast<int>(cameraX / 10.0f);

    updatePlayerPhysics(dt);
    updateObstacles(dt);

    player.updateAnimation(dt);

    // O chão é um retângulo só, recentralizado sob a câmera a cada frame
    // (mais simples que controlar tiling de verdade, e já resolve o "infinito")
    ground.position = glm::vec2(cameraX + WORLD_WIDTH * 0.5f, GROUND_HEIGHT * 0.5f);

    for (const auto &obstacle : obstacles)
    {
        if (checkCollision(player, obstacle))
        {
            gameOver = true;
            break;
        }
    }
}

void Game::updatePlayerPhysics(float dt)
{
    // O personagem fica numa posição fixa na tela; quem "anda" é a câmera
    player.position.x = cameraX + PLAYER_SCREEN_X;

    velocityY += GRAVITY * dt;
    player.position.y += velocityY * dt;

    float floorY = GROUND_HEIGHT + player.size.y * 0.5f;
    if (player.position.y <= floorY)
    {
        player.position.y = floorY;
        velocityY = 0.0f;
        onGround = true;
    }
}

void Game::updateObstacles(float dt)
{
    // Gera obstáculos novos conforme a câmera se aproxima da borda direita da tela
    while (nextObstacleX < cameraX + WORLD_WIDTH + 200.0f)
        spawnObstacle();

    // Remove obstáculos que já ficaram totalmente pra trás da câmera (não aparecem mais)
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(),
                        [this](const Sprite &o) { return o.getPMax().x < cameraX; }),
        obstacles.end());
}

// Colisão AABB clássica: as duas caixas se sobrepõem se, em AMBOS os eixos,
// o intervalo [min, max] de uma toca o intervalo da outra.
bool Game::checkCollision(const Sprite &a, const Sprite &b) const
{
    glm::vec2 aMin = a.getPMin(), aMax = a.getPMax();
    glm::vec2 bMin = b.getPMin(), bMax = b.getPMax();

    return aMin.x <= bMax.x && aMax.x >= bMin.x &&
           aMin.y <= bMax.y && aMax.y >= bMin.y;
}