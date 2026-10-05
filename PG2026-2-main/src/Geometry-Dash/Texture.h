// Texture.h - PESSOA 1 (motor de renderização)
// Carregamento de imagens (stb_image) e criação de texturas na GPU.
#pragma once

#include <string>

struct Texture
{
    unsigned int id = 0; // ID gerado por glGenTextures
    int width  = 1;      // dimensões da imagem, em pixels
    int height = 1;
};

// Carrega uma imagem do disco (png, jpg...) e cria a textura.
//   repeat: true  -> GL_REPEAT (fundos que rolam / parallax)
//           false -> GL_CLAMP_TO_EDGE (sprites e spritesheets)
//   smooth: true  -> GL_LINEAR (imagens "normais")
//           false -> GL_NEAREST (pixel art, mantém os pixels nítidos)
// Se o arquivo não for encontrado, avisa no console e devolve uma textura
// branca 1x1, para o jogo continuar rodando (o objeto aparece só com a cor).
Texture loadTexture(const std::string &path, bool repeat = false, bool smooth = false);

// Cria uma textura a partir de pixels RGBA já em memória (4 bytes por pixel).
// A primeira linha do array é a linha de BAIXO da imagem.
Texture createTextureFromPixels(const unsigned char *rgba, int width, int height,
                                bool repeat = false, bool smooth = false);

// Textura branca 1x1: serve para desenhar retângulos de cor sólida,
// usando o campo "color" do Sprite.
Texture createWhiteTexture();
