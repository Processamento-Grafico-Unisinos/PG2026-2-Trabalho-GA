// main.cpp - AS DUAS PESSOAS
// Trabalho do Grau A - Processamento Gráfico: Fundamentos
// Gabriel Gomes e Guilherme Paes
//
// Cria a janela e roda o game loop, que é o ponto de encontro das duas partes:
//   processInput() e update()  -> Game     (Pessoa 2)
//   render()                   -> Renderer (Pessoa 1)

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cstdio> // ALTERADO: sprintf, para montar o título com pontos/game over
#include <iostream>
#include <vector>

#include "Config.h"
#include "Game.h"
#include "Renderer.h"
#include "Texture.h"

// Quando a janela é redimensionada, a viewport passa a ocupar a nova área.
// A janela do mundo (ortho) continua 800x600, então a cena é só esticada.
static void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

// --- Texturas provisórias, geradas por código -------------------------------
// Servem só para testar o parallax enquanto não há imagens.
// Quando tiverem os arquivos, troquem por:
//     loadTexture("caminho/da/imagem.png", true, true);
// e apaguem estas duas funções.

// Céu: degradê vertical, opaco
static Texture makeSkyTexture()
{
    const int W = 4, H = 64;
    std::vector<unsigned char> pixels(W * H * 4);
    for (int y = 0; y < H; y++)
    {
        float t = (float)y / (float)(H - 1); // 0 embaixo, 1 em cima
        for (int x = 0; x < W; x++)
        {
            unsigned char *p = &pixels[(y * W + x) * 4];
            p[0] = (unsigned char)(90 - 60 * t);
            p[1] = (unsigned char)(60 - 40 * t);
            p[2] = (unsigned char)(150 - 70 * t);
            p[3] = 255;
        }
    }
    return createTextureFromPixels(pixels.data(), W, H, true, true);
}

// Silhueta de "prédios": blocos de alturas variadas, transparente acima deles
static Texture makeSkylineTexture(unsigned int seed, int maxHeight,
                                  unsigned char r, unsigned char g, unsigned char b)
{
    const int W = 256, H = 128, BLOCK = 16;
    std::vector<unsigned char> pixels(W * H * 4, 0);
    for (int x = 0; x < W; x++)
    {
        // altura pseudoaleatória, igual para todas as colunas do mesmo bloco
        unsigned int n = (x / BLOCK) * 2654435761u + seed * 40503u;
        int height = maxHeight / 3 + (int)((n >> 8) % (unsigned int)(maxHeight * 2 / 3));
        for (int y = 0; y < height && y < H; y++)
        {
            unsigned char *p = &pixels[(y * W + x) * 4];
            p[0] = r;
            p[1] = g;
            p[2] = b;
            p[3] = 255;
        }
    }
    return createTextureFromPixels(pixels.data(), W, H, true, false);
}
// -----------------------------------------------------------------------------

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

    // --- Texturas ---
    Texture white = createWhiteTexture(); // retângulos de cor sólida

    // --- Camadas de fundo, da mais distante para a mais próxima ---
    std::vector<Layer> layers;

    Layer sky; // céu: parado na tela
    sky.texture = makeSkyTexture();
    sky.scrollRate = 0.0f;
    layers.push_back(sky);

    Layer farBuildings; // prédios distantes: passam devagar
    farBuildings.texture = makeSkylineTexture(1, 80, 60, 50, 110);
    farBuildings.scrollRate = 0.25f;
    layers.push_back(farBuildings);

    Layer nearBuildings; // prédios próximos: passam mais rápido
    nearBuildings.texture = makeSkylineTexture(7, 55, 40, 35, 80);
    nearBuildings.scrollRate = 0.5f;
    layers.push_back(nearBuildings);

    // --- Lógica do jogo (Pessoa 2) ---
    Game game;
    game.init(white.id, white.id, white.id);

    game.player.color = glm::vec4(0.2f, 0.8f, 1.0f, 1.0f); // ciano
    game.ground.color  = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f); // cinza escuro

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

        // ALTERADO: HUD simples (pontos / game over) exibido no título da janela,
        // já que o trabalho ainda não tem texto na tela (FreeType é extra opcional)
        char titleBuf[256];
        if (game.gameOver)
            sprintf(titleBuf, "Grau A -- Gabriel Gomes e Guilherme Paes | GAME OVER (pontos: %d) - aperte ESPACO para reiniciar", game.score);
        else
            sprintf(titleBuf, "Grau A -- Gabriel Gomes e Guilherme Paes | Pontos: %d", game.score);
        glfwSetWindowTitle(window, titleBuf);

        // 3. RENDER - a ordem das chamadas define o que fica na frente
        renderer.beginFrame(game.cameraX);

        for (const Layer &layer : layers)
            renderer.drawLayer(layer);

        renderer.drawSprite(game.ground);

        for (const Sprite &obstacle : game.obstacles)
            renderer.drawSprite(obstacle);

        renderer.drawSprite(game.player);

        glfwSwapBuffers(window); // troca o back buffer pelo front buffer
    }

    renderer.shutdown();
    glfwTerminate();
    return 0;
}