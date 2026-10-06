
// Guarda o estado do jogo e o atualiza a cada frame.
// Não desenha nada: só altera os Sprites, que o Renderer depois lê.
#pragma once

#include "Sprite.h"
#include <vector>

struct GLFWwindow;

// As telas do jogo
enum class GameState
{
    Menu,     // tela inicial: "Iniciar" / "Sair"
    Playing,  // partida em andamento
    GameOver  // o jogador bateu: "Jogar de novo" / "Menu"
};

// Coisas que aconteceram no frame e que interessam a quem está de fora
// (hoje, o main, para tocar os efeitos sonoros). O Game só avisa O QUE
// aconteceu; ele não conhece o áudio.
enum class GameEvent
{
    OptionChanged,   // a opção selecionada do menu mudou
    OptionConfirmed, // uma opção do menu foi confirmada
    Jumped,          // o jogador pulou
    Died             // o jogador bateu em um obstáculo
};

// Uma opção de menu. O retângulo (em coordenadas de TELA, 800x600) serve tanto
// para desenhar a opção quanto para testar se o mouse está em cima dela.
struct MenuOption
{
    const char *label;
    glm::vec2 position; // centro
    glm::vec2 size;
};

class Game
{
public:
    // --- Estado lido pelo main na hora de desenhar ---
    Sprite player;
    std::vector<Sprite> groundTiles; // blocos do chão, lado a lado
    std::vector<Sprite> obstacles;
    float cameraX = 0.0f; // canto esquerdo da janela do mundo

    // --- Estado lido pela HUD ---
    GameState state = GameState::Menu;
    float stateTime = 0.0f; // segundos desde que entrou na tela atual
    int score = 0;          // distância percorrida (cameraX / 10, arredondado)
    float timeAlive = 0.0f; // segundos vivo na partida atual

    std::vector<MenuOption> options; // opções da tela atual (vazio durante a partida)
    int selectedOption = 0;

    // --- Eventos do frame ---
    // Preenchido por processInput() e update(). Quem lê deve esvaziar depois.
    std::vector<GameEvent> events;

    // Monta a fase. Recebe os IDs das texturas já carregadas pelo main.
    void init(unsigned int playerTexture, unsigned int groundTexture, unsigned int obstacleTexture);

    // Lê teclado/mouse (pulo durante a partida, navegação nos menus).
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

    // --- Spritesheet do personagem (player.png): 8 quadros x 2 animações ---
    static constexpr int PLAYER_FRAMES = 8;
    static constexpr int PLAYER_ANIMATIONS = 2;
    static constexpr int ANIM_RUN = 0;  // linha de baixo da imagem
    static constexpr int ANIM_JUMP = 1; // linha de cima da imagem
    static constexpr float PLAYER_ANIM_FPS = 10.0f;

    // Giro do cubo no ar, em graus/s: exatamente 1 volta durante o tempo de
    // um pulo (2 * v0 / g), para ele pousar "de pé".
    static constexpr float JUMP_SPIN = 360.0f * -GRAVITY / (2.0f * JUMP_VELOCITY);

    // --- Movimento automático (o "correr" do runner) ---
    static constexpr float SCROLL_SPEED = 320.0f; // px/s

    // --- Geração de obstáculos ---
    static constexpr float OBSTACLE_SIZE_W = 36.0f;
    static constexpr float OBSTACLE_SIZE_H = 50.0f;
    static constexpr float OBSTACLE_GAP_MIN = 260.0f;
    static constexpr float OBSTACLE_GAP_MAX = 480.0f;
    static constexpr int OBSTACLE_FRAMES = 4; // spritesheet do espinho (spike.png): 4 quadros x 1 animação
    static constexpr float OBSTACLE_ANIM_FPS = 8.0f;

    // --- Chão: largura de cada bloco (ground.png é quadrada, 100x100) ---
    static constexpr float GROUND_TILE_SIZE = 100.0f;

    // --- Menus ---
    // Logo depois de morrer, as teclas são ignoradas por este tempo. Sem isso,
    // quem estava apertando espaço para pular reiniciaria sem ver a tela.
    static constexpr float GAME_OVER_INPUT_DELAY = 0.5f; // segundos

    unsigned int obstacleTextureID = 0;
    float nextObstacleX = 0.0f;
    float velocityY = 0.0f;
    bool onGround = true;

    // Estado das teclas no frame anterior, para detectar a BORDA de subida
    // (agir só no instante em que a tecla é pressionada, não enquanto é segurada)
    bool spaceLastFrame = false;
    bool upLastFrame = false;
    bool downLastFrame = false;
    bool confirmLastFrame = false;
    bool clickLastFrame = false;
    glm::vec2 mouseLastFrame = glm::vec2(0.0f);
    bool hasMouseLastFrame = false;

    void setState(GameState newState);
    void startGame();
    void activateOption(GLFWwindow *window, int index);
    int optionAt(const glm::vec2 &point) const;

    void resetGame();
    void spawnObstacle();
    void updatePlayerPhysics(float dt);
    void updateObstacles(float dt);
    void updateGround();
    bool checkCollision(const Sprite &a, const Sprite &b) const;
};
