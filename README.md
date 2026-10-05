# Grau A — Geometry Dash simplificado

Gabriel Gomes e Guilherme Paes · OpenGL 3.3 core, GLFW, GLAD, GLM, stb_image
Processamento Gráfico - 2026/2

Professora: Rossana

## Arquivos

| Arquivo | Dono | O que faz |
|---|---|---|
| `Config.h` | os dois | Constantes do mundo: janela do mundo 800x600, altura do chão |
| `Sprite.h` | os dois | A struct `Sprite`: o dado que liga a lógica ao desenho |
| `main.cpp` | os dois | Janela, game loop e ordem de desenho |
| `Shader.h/.cpp` | Pessoa 1 | Compila e linka os shaders, envia uniforms |
| `Texture.h/.cpp` | Pessoa 1 | Carrega imagens com stb_image e cria as texturas |
| `Renderer.h/.cpp` | Pessoa 1 | Quad (VAO/VBO/EBO), projeção, matriz de modelo, sprites, parallax |
| `Game.h/.cpp` | Pessoa 2 | Estado e regras do jogo (hoje é só um esboço) |

## O contrato entre as duas partes

1. **O `Sprite` é só dado.** A Pessoa 2 altera os campos em `Game::update()`;
   a Pessoa 1 lê os campos em `Renderer::drawSprite()`. Nenhum dos dois
   precisa conhecer o código do outro.
2. **`position` é o centro do sprite**, em coordenadas de mundo. Isso casa com
   `getPMin()` / `getPMax()` do slide de colisões.
3. **Sistema de coordenadas:** origem no canto inferior esquerdo, x para a
   direita, y para cima, 1 unidade = 1 pixel na janela 800x600.
4. **Câmera:** `Game::cameraX` é a borda esquerda do que aparece na tela.
5. **Tempo:** tudo que se move multiplica a velocidade por `dt` (segundos).
6. **Game loop** (em `main.cpp`):

   ```
   game.processInput(window);   // Pessoa 2
   game.update(dt);             // Pessoa 2
   renderer.beginFrame(game.cameraX);          // Pessoa 1
   renderer.drawLayer(...)  / drawSprite(...)  // Pessoa 1, do fundo para a frente
   ```

Mudou algum campo do `Sprite` ou alguma constante do `Config.h`? Avise o outro.

## Como compilar

Copie a pasta para `src/GrauA/` da base do projeto e acrescente ao
`CMakeLists.txt` algo como (ajuste os nomes aos que a base já usa):

```cmake
file(GLOB GRAUA_SOURCES ${CMAKE_SOURCE_DIR}/src/GrauA/*.cpp)
add_executable(GrauA ${GRAUA_SOURCES} ${GLAD_C_FILE})
target_link_libraries(GrauA glfw opengl32 glm::glm)
```

O código espera encontrar `<glad/glad.h>`, `<GLFW/glfw3.h>`, `<glm/glm.hpp>` e
`<stb_image.h>` nos diretórios de include. `Texture.cpp` define
`STB_IMAGE_IMPLEMENTATION`; se a base já compila a stb_image em outro arquivo,
remova essa linha para não duplicar símbolos.

## Trocando os provisórios por imagens

- Fundos: `loadTexture("caminho.png", true, true)` no lugar de
  `makeSkyTexture()` / `makeSkylineTexture()` em `main.cpp`.
- Sprites: `loadTexture("caminho.png")` e passe o `.id` para `game.init(...)`.
- Spritesheet: preencha `nFrames` e `nAnimations` do sprite e chame
  `sprite.updateAnimation(dt)` no update.

Caminhos relativos são resolvidos a partir da pasta de onde o executável é
chamado, não da pasta do código.
