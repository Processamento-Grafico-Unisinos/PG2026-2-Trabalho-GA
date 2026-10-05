// Sprite.h - CONTRATO (as duas pessoas usam)
// O Sprite é só DADO: a Pessoa 2 altera os campos em update(),
// a Pessoa 1 lê os campos em Renderer::drawSprite().
// Não inclui nada de OpenGL, então a lógica do jogo não depende do motor.
#pragma once

#include <glm/glm.hpp>

struct Sprite
{
    // --- Transformações (viram a matriz de modelo no Renderer) ---
    glm::vec2 position = glm::vec2(0.0f, 0.0f); // CENTRO do sprite, em coordenadas de mundo
    glm::vec2 size     = glm::vec2(50.0f, 50.0f); // largura e altura, em unidades de mundo
    float rotation     = 0.0f;                    // em graus, sentido anti-horário

    // --- Aparência ---
    unsigned int textureID = 0;                               // ID da textura na GPU (Texture::id)
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);      // multiplica a cor da textura (RGBA)

    // --- Spritesheet: 1 animação por linha, 1 quadro por coluna ---
    int nAnimations = 1; // número de linhas da spritesheet
    int nFrames     = 1; // número de colunas da spritesheet
    int iAnimation  = 0; // linha atual  (0 = linha de baixo da imagem)
    int iFrame      = 0; // coluna atual (0 = esquerda)
    float animFPS   = 12.0f; // quadros de animação por segundo
    float animTimer = 0.0f;  // tempo acumulado desde a última troca de quadro

    // --- Hitbox (AABB), como no slide de colisões ---
    // A caixa ignora a rotação: é sempre alinhada aos eixos.
    glm::vec2 getPMin() const { return position - size * 0.5f; }
    glm::vec2 getPMax() const { return position + size * 0.5f; }

    // Avança o quadro da animação conforme o tempo decorrido (dt em segundos).
    // Chamar uma vez por frame, no update(), para cada sprite animado.
    void updateAnimation(float dt)
    {
        if (nFrames <= 1 || animFPS <= 0.0f)
            return;

        animTimer += dt;
        float frameDuration = 1.0f / animFPS;
        while (animTimer >= frameDuration)
        {
            animTimer -= frameDuration;
            iFrame = (iFrame + 1) % nFrames; // volta ao primeiro quadro no fim
        }
    }
};
