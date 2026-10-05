// Renderer.cpp - PESSOA 1 (motor de renderização)
#include "Renderer.h"
#include "Config.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// ---------------------------------------------------------------------------
// SHADERS (GLSL 3.30, OpenGL 3.3 core)
// ---------------------------------------------------------------------------

// Vertex shader: executa uma vez por vértice.
// Leva o vértice do espaço local até o clip space e calcula a coordenada
// de textura final (recorte da spritesheet / deslocamento do parallax).
static const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

uniform mat4 projection; // janela do mundo -> clip space [-1, 1]
uniform mat4 model;      // local -> mundo (translação, rotação, escala)

uniform vec2 texScale;   // tamanho de 1 quadro: (1/nFrames, 1/nAnimations)
uniform vec2 texOffset;  // qual quadro: (iFrame * ds, iAnimation * dt)

out vec2 TexCoord;

void main()
{
    gl_Position = projection * model * vec4(position, 1.0);
    TexCoord = texCoord * texScale + texOffset;
}
)";

// Fragment shader: executa uma vez por fragmento e define a cor final.
static const char *fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 color;

uniform sampler2D tex1;  // pixels da textura
uniform vec4 tintColor;  // cor que multiplica a textura

void main()
{
    color = texture(tex1, TexCoord) * tintColor;
}
)";

// ---------------------------------------------------------------------------
// INICIALIZAÇÃO
// ---------------------------------------------------------------------------

bool Renderer::init()
{
    if (!shader.compile(vertexShaderSource, fragmentShaderSource))
        return false;

    setupQuad();

    // Composição das imagens (slide "Composição de imagens"):
    // com GL_ALWAYS, quem é desenhado por último fica na frente.
    // Por isso a ORDEM das chamadas de desenho define as camadas.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    // Transparência pelo canal alfa das texturas
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // O sampler "tex1" lê da unidade de textura 0
    shader.use();
    shader.setInt("tex1", 0);

    return true;
}

// Cria UM quadrado unitário, centrado na origem, que é reutilizado para
// desenhar todos os objetos. O que muda de um objeto para outro são os
// uniforms (matriz de modelo, textura, recorte e cor), não a geometria.
void Renderer::setupQuad()
{
    float vertices[] = {
        // posição (x, y, z)    // coord. de textura (s, t)
         0.5f,  0.5f, 0.0f,     1.0f, 1.0f, // 0: superior direito
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f, // 1: inferior direito
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, // 2: inferior esquerdo
        -0.5f,  0.5f, 0.0f,     0.0f, 1.0f  // 3: superior esquerdo
    };

    // 4 vértices e 6 índices: os 2 triângulos compartilham os vértices 1 e 3
    unsigned int indices[] = {
        0, 1, 3, // primeiro triângulo
        1, 2, 3  // segundo triângulo
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Primeiro o bind do VAO: ele guarda as configurações feitas a seguir
    glBindVertexArray(VAO);

    // VBO: dados dos vértices na memória da GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // EBO: índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Cada vértice ocupa 5 floats (20 bytes): x y z | s t
    // Atributo 0 (posição): 3 valores, deslocamento 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Atributo 1 (coord. de textura): 2 valores, deslocamento de 3 floats (12 bytes)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Desfaz os binds. O EBO NÃO pode ser desvinculado com o VAO ativo,
    // porque o VAO guarda essa ligação.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// FRAME
// ---------------------------------------------------------------------------

void Renderer::beginFrame(float camX, float camY)
{
    cameraX = camX;
    cameraY = camY;

    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // CÂMERA 2D: a projeção ortográfica define a janela do mundo.
    // Mover a câmera é deslocar os limites left/right/bottom/top,
    // por isso não precisamos de uma matriz de view separada.
    glm::mat4 projection = glm::ortho(cameraX, cameraX + WORLD_WIDTH,   // left, right
                                      cameraY, cameraY + WORLD_HEIGHT,  // bottom, top
                                      -1.0f, 1.0f);                     // near, far

    shader.use();
    shader.setMat4("projection", projection);
}

// Faz a chamada de desenho do quad com a textura indicada
void Renderer::drawQuad(unsigned int textureID)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------------
// SPRITES
// ---------------------------------------------------------------------------

void Renderer::drawSprite(const Sprite &sprite)
{
    // MATRIZ DE MODELO: leva o quad unitário para o lugar do objeto no mundo.
    // A multiplicação não é comutativa. Escrevendo T * R * S, o vértice é
    // primeiro escalado, depois rotacionado (em torno do próprio centro)
    // e só então transladado para a posição final.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(sprite.position, 0.0f));
    model = glm::rotate(model, glm::radians(sprite.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(sprite.size, 1.0f));

    // RECORTE DA SPRITESHEET: ds e dt são o tamanho de um quadro em
    // coordenadas de textura. O offset escolhe a coluna (quadro) e a linha
    // (animação). Sem spritesheet, nFrames = nAnimations = 1 e nada muda.
    float ds = 1.0f / (float)sprite.nFrames;
    float dt = 1.0f / (float)sprite.nAnimations;

    shader.setMat4("model", model);
    shader.setVec2("texScale", glm::vec2(ds, dt));
    shader.setVec2("texOffset", glm::vec2(sprite.iFrame * ds, sprite.iAnimation * dt));
    shader.setVec4("tintColor", sprite.color);

    drawQuad(sprite.textureID);
}

// ---------------------------------------------------------------------------
// CAMADAS DE FUNDO (PARALLAX)
// ---------------------------------------------------------------------------

void Renderer::drawLayer(const Layer &layer)
{
    // O quad da camada cobre exatamente a janela do mundo, então ele
    // acompanha a câmera e sempre preenche a tela inteira.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(cameraX + WORLD_WIDTH / 2.0f,
                                            cameraY + WORLD_HEIGHT / 2.0f, 0.0f));
    model = glm::scale(model, glm::vec3(WORLD_WIDTH, WORLD_HEIGHT, 1.0f));

    // Largura que a imagem ocupa no mundo quando sua altura é a da tela
    // (mantém o aspect ratio da imagem).
    float aspect = (float)layer.texture.width / (float)layer.texture.height;
    float layerWidth = WORLD_HEIGHT * aspect;

    // Quantas vezes a imagem cabe na largura da tela (GL_REPEAT cuida do resto)
    float repeatX = WORLD_WIDTH / layerWidth;

    // O scrolling é feito deslocando as coordenadas de textura, não o quad.
    // Cada camada anda uma fração (scrollRate) do que a câmera andou:
    // quanto mais ao fundo, menor a taxa, e mais devagar ela parece passar.
    float offsetS = (cameraX * layer.scrollRate) / layerWidth;
    offsetS = offsetS - std::floor(offsetS); // mantém em [0, 1) para não perder precisão

    shader.setMat4("model", model);
    shader.setVec2("texScale", glm::vec2(repeatX, 1.0f));
    shader.setVec2("texOffset", glm::vec2(offsetS, 0.0f));
    shader.setVec4("tintColor", glm::vec4(1.0f));

    drawQuad(layer.texture.id);
}

// ---------------------------------------------------------------------------

void Renderer::shutdown()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader.ID);
}
