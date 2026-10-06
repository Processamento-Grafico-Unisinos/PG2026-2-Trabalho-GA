// Fonte para os textos da HUD. O arquivo .ttf é convertido UMA vez, na carga,
// em uma textura com todos os caracteres lado a lado (um "atlas"). Desenhar um
// texto vira desenhar um quad por caractere, recortando o atlas do mesmo jeito
// que se recorta um quadro de uma spritesheet.
#pragma once

#include "Texture.h"
#include <string>

// Onde está um caractere dentro do atlas e como posicioná-lo na linha
struct Glyph
{
    float s = 0.0f, t = 0.0f;          // canto inferior esquerdo no atlas (coord. de textura)
    float ds = 0.0f, dt = 0.0f;        // tamanho no atlas (coord. de textura)
    float width = 0.0f, height = 0.0f; // tamanho do desenho, em pixels
    float offsetX = 0.0f;              // da posição da "caneta" até a borda esquerda
    float offsetY = 0.0f;              // da linha de base até a borda de BAIXO (y para cima)
    float advance = 0.0f;              // quanto a caneta anda depois deste caractere
};

struct Font
{
    // Caracteres ASCII imprimíveis: do espaço (32) ao til (126). Sem acentos.
    static const int FIRST_CHAR = 32;
    static const int NUM_CHARS = 95;

    Texture atlas;
    float size = 0.0f;      // altura da fonte, em pixels
    float capHeight = 0.0f; // altura de uma letra maiúscula, para centralizar na vertical
    Glyph glyphs[NUM_CHARS];

    // Devolve nullptr se o caractere não existe na fonte
    const Glyph *getGlyph(char c) const;

    // Largura que o texto ocupa, em pixels
    float textWidth(const std::string &text) const;
};

// Carrega um arquivo .ttf e gera o atlas para a altura pedida (em pixels).
// Cada tamanho de texto usado no jogo é uma Font separada, para ficar nítido.
// Se o arquivo não for encontrado, avisa no console e os textos não aparecem.
Font loadFont(const std::string &path, float pixelHeight);
