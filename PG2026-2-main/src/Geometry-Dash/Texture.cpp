// Texture.cpp - PESSOA 1 (motor de renderização)
#include "Texture.h"

#include <glad/glad.h>
#include <iostream>

// A implementação da stb_image deve ser compilada em UM único .cpp do projeto.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture createTextureFromPixels(const unsigned char *rgba, int width, int height,
                                bool repeat, bool smooth)
{
    Texture tex;
    tex.width = width;
    tex.height = height;

    // 1. Gera o identificador e faz o bind (mesma ideia dos buffers)
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    // 2. Wrapping: o que acontece com coordenadas de textura fora de [0, 1]
    GLint wrap = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    // 3. Filtering: como amostrar quando a textura é ampliada ou reduzida
    GLint filter = smooth ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    // 4. Envia os pixels para a memória da GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

Texture createWhiteTexture()
{
    const unsigned char white[4] = {255, 255, 255, 255};
    return createTextureFromPixels(white, 1, 1, false, false);
}

Texture loadTexture(const std::string &path, bool repeat, bool smooth)
{
    // A stb_image lê a imagem de cima para baixo, mas na OpenGL a coordenada
    // de textura t = 0 fica embaixo. Invertendo na carga, a linha [0] da
    // spritesheet passa a ser a de baixo, como nos slides.
    stbi_set_flip_vertically_on_load(true);

    // O último parâmetro (4) força a saída em RGBA, mesmo que o arquivo
    // seja RGB ou tons de cinza. Assim o formato enviado é sempre o mesmo.
    int width, height, channels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!data)
    {
        std::cerr << "ERRO: nao foi possivel carregar a textura '" << path
                  << "' (" << stbi_failure_reason() << "). Usando textura branca."
                  << std::endl;
        return createWhiteTexture();
    }

    Texture tex = createTextureFromPixels(data, width, height, repeat, smooth);
    stbi_image_free(data); // os pixels já estão na GPU, libera a cópia da CPU
    return tex;
}
