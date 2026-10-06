
#include "Game.h"
#include "Config.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

void Game::init(unsigned int playerTexture, unsigned int groundTexture, unsigned int obstacleTexture)
{
    player.textureID = playerTexture;
    player.size = glm::vec2(PLAYER_SIZE, PLAYER_SIZE);
    player.nFrames = PLAYER_FRAMES;
    player.nAnimations = PLAYER_ANIMATIONS;
    player.animFPS = PLAYER_ANIM_FPS;

    // Blocos suficientes para cobrir a largura da tela, mais 1 de folga
    // (quando a câmera está no meio de um bloco, aparece um pedaço a mais)
    int tileCount = static_cast<int>(std::ceil(WORLD_WIDTH / GROUND_TILE_SIZE)) + 1;
    Sprite tile;
    tile.textureID = groundTexture;
    tile.size = glm::vec2(GROUND_TILE_SIZE, GROUND_HEIGHT);
    groundTiles.assign(tileCount, tile);

    obstacleTextureID = obstacleTexture;

    srand(1234); // semente fixa (fases iguais em todo teste); troque por (unsigned)time(NULL) se quiser variar

    resetGame();
    setState(GameState::Menu);
}

// Troca de tela e monta as opções da tela nova
void Game::setState(GameState newState)
{
    state = newState;
    stateTime = 0.0f;
    selectedOption = 0;
    options.clear();

    const float centerX = WORLD_WIDTH * 0.5f;
    const glm::vec2 size(400.0f, 46.0f);

    if (state == GameState::Menu)
    {
        options.push_back({"Iniciar", glm::vec2(centerX, 290.0f), size});
        options.push_back({"Sair", glm::vec2(centerX, 230.0f), size});
    }
    else if (state == GameState::GameOver)
    {
        options.push_back({"Jogar de novo", glm::vec2(centerX, 235.0f), size});
        options.push_back({"Menu", glm::vec2(centerX, 175.0f), size});
    }
}

void Game::startGame()
{
    resetGame();
    setState(GameState::Playing);
}

// Executa a opção escolhida na tela atual
void Game::activateOption(GLFWwindow *window, int index)
{
    events.push_back(GameEvent::OptionConfirmed);

    if (state == GameState::Menu)
    {
        if (index == 0)
            startGame();
        else
            glfwSetWindowShouldClose(window, GLFW_TRUE); // "Sair": encerra o game loop
    }
    else if (state == GameState::GameOver)
    {
        if (index == 0)
            startGame();
        else
        {
            resetGame(); // limpa a fase, que fica de fundo no menu
            setState(GameState::Menu);
        }
    }
}

// Índice da opção que contém o ponto, ou -1. É o mesmo teste de hitbox (AABB)
// da colisão, só que entre um ponto e um retângulo.
int Game::optionAt(const glm::vec2 &point) const
{
    for (size_t i = 0; i < options.size(); i++)
    {
        glm::vec2 pMin = options[i].position - options[i].size * 0.5f;
        glm::vec2 pMax = options[i].position + options[i].size * 0.5f;
        if (point.x >= pMin.x && point.x <= pMax.x && point.y >= pMin.y && point.y <= pMax.y)
            return static_cast<int>(i);
    }
    return -1;
}

void Game::resetGame()
{
    cameraX = 0.0f;
    score = 0;
    timeAlive = 0.0f;

    velocityY = 0.0f;
    onGround = true;
    player.position = glm::vec2(PLAYER_SCREEN_X, GROUND_HEIGHT + player.size.y * 0.5f);
    player.rotation = 0.0f;
    player.iAnimation = ANIM_RUN;

    updateGround();

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
    obstacle.nFrames = OBSTACLE_FRAMES;
    obstacle.animFPS = OBSTACLE_ANIM_FPS;
    obstacles.push_back(obstacle);

    float gap = OBSTACLE_GAP_MIN + static_cast<float>(rand()) / RAND_MAX * (OBSTACLE_GAP_MAX - OBSTACLE_GAP_MIN);
    nextObstacleX += gap;
}

// Posição do cursor em coordenadas de TELA (as mesmas da HUD: 800x600, y para
// cima). A GLFW devolve em pixels da janela, com y para baixo; a conversão
// usa o tamanho atual da janela, então continua valendo se ela for redimensionada.
static glm::vec2 getMouseOnScreen(GLFWwindow *window)
{
    double x, y;
    int width, height;
    glfwGetCursorPos(window, &x, &y);
    glfwGetWindowSize(window, &width, &height);
    if (width <= 0 || height <= 0)
        return glm::vec2(-1.0f);

    return glm::vec2(static_cast<float>(x / width) * WORLD_WIDTH,
                     static_cast<float>(1.0 - y / height) * WORLD_HEIGHT);
}

void Game::processInput(GLFWwindow *window)
{
    auto isDown = [window](int key) { return glfwGetKey(window, key) == GLFW_PRESS; };

    bool spaceNow = isDown(GLFW_KEY_SPACE);
    bool upNow = isDown(GLFW_KEY_UP) || isDown(GLFW_KEY_W);
    bool downNow = isDown(GLFW_KEY_DOWN) || isDown(GLFW_KEY_S);
    bool confirmNow = spaceNow || isDown(GLFW_KEY_ENTER) || isDown(GLFW_KEY_KP_ENTER);
    bool clickNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    glm::vec2 mouse = getMouseOnScreen(window);

    // Borda de subida: verdadeiro só no frame em que a tecla é pressionada
    bool spacePressed = spaceNow && !spaceLastFrame;
    bool upPressed = upNow && !upLastFrame;
    bool downPressed = downNow && !downLastFrame;
    bool confirmPressed = confirmNow && !confirmLastFrame;
    bool clickPressed = clickNow && !clickLastFrame;
    bool mouseMoved = hasMouseLastFrame && mouse != mouseLastFrame;

    if (state == GameState::Playing)
    {
        if (spacePressed && onGround)
        {
            velocityY = JUMP_VELOCITY;
            onGround = false;
            events.push_back(GameEvent::Jumped);
        }
    }
    else if (state == GameState::Menu || stateTime >= GAME_OVER_INPUT_DELAY)
    {
        // Setas (ou W/S) andam pela lista, voltando ao início depois do fim
        int count = static_cast<int>(options.size());
        int previousOption = selectedOption;
        if (upPressed)
            selectedOption = (selectedOption + count - 1) % count;
        if (downPressed)
            selectedOption = (selectedOption + 1) % count;

        // O mouse só muda a seleção quando se mexe, para não brigar com o teclado
        int hovered = optionAt(mouse);
        if (mouseMoved && hovered >= 0)
            selectedOption = hovered;

        if (selectedOption != previousOption)
            events.push_back(GameEvent::OptionChanged);

        if (confirmPressed)
            activateOption(window, selectedOption);
        else if (clickPressed && hovered >= 0)
            activateOption(window, hovered);
    }

    spaceLastFrame = spaceNow;
    upLastFrame = upNow;
    downLastFrame = downNow;
    confirmLastFrame = confirmNow;
    clickLastFrame = clickNow;
    mouseLastFrame = mouse;
    hasMouseLastFrame = true;
}

void Game::update(float dt)
{
    stateTime += dt;

    if (state == GameState::Menu)
        player.updateAnimation(dt); // o cubo fica piscando no fundo do menu

    if (state != GameState::Playing)
        return;

    timeAlive += dt;
    cameraX += SCROLL_SPEED * dt;
    score = static_cast<int>(cameraX / 10.0f);

    updatePlayerPhysics(dt);
    updateObstacles(dt);

    updateGround();

    player.updateAnimation(dt);

    for (const auto &obstacle : obstacles)
    {
        if (checkCollision(player, obstacle))
        {
            setState(GameState::GameOver);
            events.push_back(GameEvent::Died);
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

    // Troca a linha da spritesheet conforme o estado e gira o cubo no ar.
    // A hitbox não gira junto (ver Sprite::getPMin/getPMax).
    if (onGround)
    {
        player.iAnimation = ANIM_RUN;
        player.rotation = 0.0f;
    }
    else
    {
        player.iAnimation = ANIM_JUMP;
        player.rotation -= JUMP_SPIN * dt; // negativo = sentido horário
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

    for (auto &obstacle : obstacles)
        obstacle.updateAnimation(dt);
}

// O chão "infinito" usa sempre os mesmos blocos: a cada frame eles são
// reposicionados a partir do múltiplo de GROUND_TILE_SIZE imediatamente à
// esquerda da câmera. Como os blocos são iguais, parece que o chão rola.
void Game::updateGround()
{
    float firstX = std::floor(cameraX / GROUND_TILE_SIZE) * GROUND_TILE_SIZE;
    for (size_t i = 0; i < groundTiles.size(); i++)
        groundTiles[i].position = glm::vec2(firstX + (i + 0.5f) * GROUND_TILE_SIZE, GROUND_HEIGHT * 0.5f);
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