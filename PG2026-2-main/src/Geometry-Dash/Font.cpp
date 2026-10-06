// Font.cpp - PESSOA 1 (motor de renderização)
#include "Font.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

// A implementação da stb_truetype deve ser compilada em UM único .cpp do projeto.
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

const Glyph *Font::getGlyph(char c) const
{
    int index = (int)(unsigned char)c - FIRST_CHAR;
    if (index < 0 || index >= NUM_CHARS)
        return nullptr;
    return &glyphs[index];
}

float Font::textWidth(const std::string &text) const
{
    float width = 0.0f;
    for (char c : text)
    {
        const Glyph *glyph = getGlyph(c);
        if (glyph)
            width += glyph->advance;
    }
    return width;
}

Font loadFont(const std::string &path, float pixelHeight)
{
    Font font;
    font.size = pixelHeight;

    // 1. Lê o arquivo .ttf inteiro para a memória
    std::ifstream file(path, std::ios::binary);
    std::vector<unsigned char> ttf((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
    if (ttf.empty())
    {
        std::cerr << "ERRO: nao foi possivel carregar a fonte '" << path << "'." << std::endl;
        return font;
    }

    // 2. A stb_truetype desenha todos os caracteres em uma imagem de 1 canal
    //    (0 = vazio, 255 = cheio) e devolve a posição de cada um nela.
    const int W = 512, H = 512;
    std::vector<unsigned char> coverage(W * H);
    stbtt_bakedchar baked[Font::NUM_CHARS];
    int result = stbtt_BakeFontBitmap(ttf.data(), 0, pixelHeight, coverage.data(), W, H,
                                      Font::FIRST_CHAR, Font::NUM_CHARS, baked);
    if (result <= 0)
    {
        std::cerr << "ERRO: a fonte '" << path << "' e invalida ou nao coube no atlas de "
                  << W << "x" << H << "." << std::endl;
        return font;
    }

    // 3. Converte para RGBA: letra branca, com a cobertura no canal alfa.
    //    Assim o "tintColor" do shader define a cor do texto.
    //    A stb entrega a linha de cima primeiro; invertemos para a linha [0]
    //    ser a de baixo, como nas outras texturas.
    std::vector<unsigned char> rgba(W * H * 4);
    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            unsigned char *p = &rgba[((H - 1 - y) * W + x) * 4];
            p[0] = p[1] = p[2] = 255;
            p[3] = coverage[y * W + x];
        }
    }
    font.atlas = createTextureFromPixels(rgba.data(), W, H, false, false);

    // 4. Guarda o recorte e o posicionamento de cada caractere.
    //    Na stb o y cresce para baixo; aqui convertemos para y para cima.
    for (int i = 0; i < Font::NUM_CHARS; i++)
    {
        const stbtt_bakedchar &b = baked[i];
        Glyph &glyph = font.glyphs[i];
        glyph.width = (float)(b.x1 - b.x0);
        glyph.height = (float)(b.y1 - b.y0);
        glyph.s = (float)b.x0 / W;
        glyph.t = 1.0f - (float)b.y1 / H;
        glyph.ds = glyph.width / W;
        glyph.dt = glyph.height / H;
        glyph.offsetX = b.xoff;
        glyph.offsetY = -(b.yoff + glyph.height);
        glyph.advance = b.xadvance;
    }
    font.capHeight = font.getGlyph('H')->height;

    return font;
}
