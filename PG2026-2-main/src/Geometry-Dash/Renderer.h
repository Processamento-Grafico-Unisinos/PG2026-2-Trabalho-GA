// Renderer.h - PESSOA 1 (motor de renderização)
// Tudo que envolve desenho passa por aqui: shader, quad (VAO/VBO/EBO),
// projeção ortográfica (câmera 2D), matriz de modelo e camadas de parallax.
#pragma once

#include "Shader.h"
#include "Sprite.h"
#include "Texture.h"

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

    // Libera os buffers e o programa de shader
    void shutdown();

private:
    Shader shader;
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    float cameraX = 0.0f, cameraY = 0.0f;

    void setupQuad();
    void drawQuad(unsigned int textureID);
};
