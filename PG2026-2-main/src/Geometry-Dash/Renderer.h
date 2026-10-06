// Renderer.h - PESSOA 1 (motor de renderização)
// Tudo que envolve desenho passa por aqui: shader, quad (VAO/VBO/EBO),
// projeção ortográfica (câmera 2D), matriz de modelo, camadas de parallax
// e os textos e retângulos da HUD.
#pragma once

#include "Font.h"
#include "Shader.h"
#include "Sprite.h"
#include "Texture.h"

#include <string>

// Uma camada de fundo. Ocupa a tela toda e se repete na horizontal.
struct Layer
{
    Texture texture;         // deve ser carregada com repeat = true
    float scrollRate = 1.0f; // 0 = parada na tela (céu distante), 1 = anda junto com o mundo
};

class Renderer
{
public:
    // Chamar UMA vez, depois de criar a janela e carregar a GLAD
    bool init();

    // Chamar no início de cada frame: limpa a tela e posiciona a câmera.
    // (cameraX, cameraY) é o canto inferior esquerdo da janela do mundo.
    void beginFrame(float cameraX, float cameraY = 0.0f);

    // Desenha uma camada de fundo com parallax. Desenhar do fundo para a frente.
    void drawLayer(const Layer &layer);

    // Desenha um sprite (personagem, chão, obstáculo...)
    void drawSprite(const Sprite &sprite);

    // Chamar depois de desenhar o mundo e antes de desenhar a HUD.
    // Troca a projeção por uma SEM câmera: (0, 0) passa a ser o canto inferior
    // esquerdo da tela, então o que for desenhado fica parado enquanto o mundo rola.
    void beginHUD();

    // Desenha um retângulo de cor sólida (painéis e escurecimento da tela).
    // center e size nas coordenadas da projeção atual.
    void drawRect(const glm::vec2 &center, const glm::vec2 &size, const glm::vec4 &color);

    // Desenha um texto. x é a borda esquerda e y é a linha de base das letras.
    void drawText(const Font &font, const std::string &text, float x, float y, const glm::vec4 &color);

    // Libera os buffers e o programa de shader
    void shutdown();

private:
    Shader shader;
    Texture whiteTexture; // 1x1 branca, para o drawRect
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    float cameraX = 0.0f, cameraY = 0.0f;

    void setupQuad();
    void drawQuad(unsigned int textureID);
};
