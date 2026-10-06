// main.cpp - AS DUAS PESSOAS
// Trabalho do Grau A - Processamento Gráfico: Fundamentos
// Gabriel Gomes e Guilherme Paes
//
// Cria a janela e roda o game loop, que é o ponto de encontro das duas partes:
//   processInput() e update()  -> Game     (Pessoa 2)
//   render()                   -> Renderer (Pessoa 1) + Hud (textos e menus)

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "Audio.h"
#include "Config.h"
#include "Game.h"
#include "Hud.h"
#include "Renderer.h"
#include "Texture.h"

// Quando a janela é redimensionada, a viewport passa a ocupar a nova área.
// A janela do mundo (ortho) continua 800x600, então a cena é só esticada.
static void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

// Pasta das imagens. O CMakeLists.txt define ASSETS_DIR com o caminho absoluto
// de PG2026-2-main/assets/, então o jogo acha as imagens independente da pasta
// de onde o executável é chamado. Sem o CMake, vale o caminho relativo abaixo.
#ifndef ASSETS_DIR
#define ASSETS_DIR "assets/"
#endif

static std::string assetPath(const std::string &file)
{
    return std::string(ASSETS_DIR) + file;
}

// Toca o som de cada evento do frame e esvazia a lista, para o mesmo evento
// não tocar de novo no frame seguinte.
static void playEventSounds(Game &game)
{
    for (GameEvent event : game.events)
    {
        switch (event)
        {
        case GameEvent::OptionChanged:
            playSfx(Sfx::Select);
            break;
        case GameEvent::OptionConfirmed:
            playSfx(Sfx::Confirm);
            break;
        case GameEvent::Jumped:
            playSfx(Sfx::Jump);
            break;
        case GameEvent::Died:
            playSfx(Sfx::Death);
            break;
        }
    }
    game.events.clear();
}

int main()
{
    // --- Janela e contexto OpenGL 3.3 core ---
    if (!glfwInit())
    {
        std::cerr << "ERRO: falha ao inicializar a GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          "Grau A - Gabriel Gomes e Guilherme Paes", NULL, NULL);
    if (!window)
    {
        std::cerr << "ERRO: falha ao criar a janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSwapInterval(1); // vsync: limita o FPS à taxa do monitor

    // GLAD: carrega os ponteiros das funções OpenGL em tempo de execução
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "ERRO: falha ao inicializar a GLAD" << std::endl;
        return -1;
    }

    // Viewport inicial: a área da janela onde a cena é desenhada
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // --- Motor de renderização (Pessoa 1) ---
    Renderer renderer;
    if (!renderer.init())
        return -1;

    // --- Texturas dos sprites (sem repetição, filtro linear) ---
    // player.png e spike.png são spritesheets; o número de quadros e de
    // animações de cada uma está em Game.h.
    Texture playerTexture = loadTexture(assetPath("player.png"), false, true);
    Texture groundTexture = loadTexture(assetPath("ground.png"), false, true);
    Texture obstacleTexture = loadTexture(assetPath("spike.png"), false, true);

    // --- Camadas de fundo, da mais distante para a mais próxima ---
    // repeat = true: a imagem se repete na horizontal enquanto a câmera anda
    std::vector<Layer> layers;

    Layer sky; // céu: parado na tela
    sky.texture = loadTexture(assetPath("bg_sky.png"), true, true);
    sky.scrollRate = 0.0f;
    layers.push_back(sky);

    Layer farBuildings; // prédios distantes: passam devagar
    farBuildings.texture = loadTexture(assetPath("bg_far.png"), true, true);
    farBuildings.scrollRate = 0.25f;
    layers.push_back(farBuildings);

    Layer nearBuildings; // prédios próximos: passam mais rápido
    nearBuildings.texture = loadTexture(assetPath("bg_near.png"), true, true);
    nearBuildings.scrollRate = 0.5f;
    layers.push_back(nearBuildings);

    // --- Fonte da HUD, em três tamanhos ---
    // A Press Start 2P é desenhada numa grade de 8x8, então fica nítida em
    // tamanhos múltiplos de 8.
    const std::string fontFile = assetPath("fonts/PressStart2P-Regular.ttf");
    HudFonts fonts;
    fonts.small = loadFont(fontFile, 16.0f);
    fonts.medium = loadFont(fontFile, 24.0f);
    fonts.large = loadFont(fontFile, 48.0f);

    // --- Música ---
    if (initAudio())
    {
        loadMusic(assetPath("Arcade_Rush.mp3"));
        loadSfx(Sfx::Select, assetPath("sfx/select.wav"));
        loadSfx(Sfx::Confirm, assetPath("sfx/confirm.wav"));
        loadSfx(Sfx::Jump, assetPath("sfx/jump.wav"));
        loadSfx(Sfx::Death, assetPath("sfx/death.wav"));
    }

    // --- Lógica do jogo (Pessoa 2) ---
    Game game;
    game.init(playerTexture.id, groundTexture.id, obstacleTexture.id);
    GameState lastState = game.state; // para perceber a troca de tela e ligar/desligar a música

    // --- GAME LOOP ---
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        // Tempo decorrido desde o frame anterior (delta time), em segundos.
        // Multiplicar as velocidades por dt deixa o jogo igual em qualquer FPS.
        double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        lastTime = now;
        if (dt > 0.05f)
            dt = 0.05f; // evita "saltos" se a janela travar (ex.: ao arrastá-la)

        // 1. PROCESS INPUT
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        game.processInput(window);

        // 2. UPDATE
        game.update(dt);

        // Efeitos sonoros: um para cada evento que o Game registrou neste frame
        playEventSounds(game);

        // Música: começa do início quando a partida começa e para quando ela
        // termina (game over ou volta ao menu). No menu não toca.
        if (game.state != lastState)
        {
            if (game.state == GameState::Playing)
                playMusic();
            else
                stopMusic();
            lastState = game.state;
        }

        // 3. RENDER - a ordem das chamadas define o que fica na frente
        renderer.beginFrame(game.cameraX);

        for (const Layer &layer : layers)
            renderer.drawLayer(layer);

        for (const Sprite &tile : game.groundTiles)
            renderer.drawSprite(tile);

        for (const Sprite &obstacle : game.obstacles)
            renderer.drawSprite(obstacle);

        renderer.drawSprite(game.player);

        // HUD por último (fica na frente de tudo), com a projeção sem câmera
        renderer.beginHUD();
        drawHud(renderer, fonts, game);

        glfwSwapBuffers(window); // troca o back buffer pelo front buffer
    }

    shutdownAudio();
    renderer.shutdown();
    glfwTerminate();
    return 0;
}