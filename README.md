# Grau A — Geometry Dash simplificado

Gabriel Gomes e Guilherme Paes · OpenGL 3.3 core, GLFW, GLAD, GLM, stb_image, stb_truetype, miniaudio
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
| `Font.h/.cpp` | Pessoa 1 | Converte a fonte `.ttf` em uma textura com todos os caracteres (stb_truetype) |
| `Renderer.h/.cpp` | Pessoa 1 | Quad (VAO/VBO/EBO), projeção, matriz de modelo, sprites, spritesheets, parallax, textos |
| `Game.h/.cpp` | Pessoa 2 | Estado e regras do jogo: telas (menu, partida, game over), pulo, obstáculos, colisão, animações, pontos e tempo |
| `Hud.h/.cpp` | os dois | Desenha o placar, o menu inicial e a tela de game over |
| `Audio.h/.cpp` | os dois | Música de fundo e efeitos sonoros (miniaudio) |

Os arquivos de código ficam em `PG2026-2-main/src/Geometry-Dash/`. As imagens,
a fonte e a música ficam em `PG2026-2-main/assets/`.

## Como jogar

O jogo abre em um menu com as opções **Iniciar** e **Sair**.

- **Setas ou W/S:** escolhem a opção. **Enter ou Espaço:** confirma.
- **Mouse:** passar por cima escolhe, clicar confirma.
- **Espaço** (durante a partida): pula.
- **Esc:** fecha o jogo a qualquer momento.

O cubo corre sozinho. Encostar em um espinho encerra a partida e abre a tela
de game over, com as opções **Jogar de novo** e **Menu**.

Durante a partida, o canto superior esquerdo mostra o **Score** (distância
percorrida) e o **Time** (tempo vivo, em minutos e segundos). A música começa
junto com a partida, repete ao chegar no fim e para quando o jogador morre.

Há quatro efeitos sonoros: trocar de opção no menu, confirmar a opção, pular
e morrer.

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
   renderer.beginHUD();                        // troca para a projeção sem câmera
   drawHud(renderer, fonts, game);             // textos e menus, por cima de tudo
   ```
7. **HUD:** as posições da HUD são em coordenadas de tela (800x600, origem no
   canto inferior esquerdo), não de mundo. Por isso ela não rola com a câmera.

Mudou algum campo do `Sprite` ou alguma constante do `Config.h`? Avise o outro.

## Como compilar

Precisa de CMake, de um compilador C++17 e de internet na primeira vez (o
CMake baixa a GLFW, a GLM, a stb e a miniaudio). A partir da raiz do repositório:

```
cmake -S PG2026-2-main -B build -G "MinGW Makefiles"
cmake --build build
build/Geometry-Dash.exe
```

No Linux ou no macOS, tire o `-G "MinGW Makefiles"` e rode `build/Geometry-Dash`.

No VS Code com a extensão CMake Tools basta abrir a raiz do repositório: o
`.vscode/settings.json` já aponta para `PG2026-2-main`.

## Imagens

| Arquivo | Uso | Formato |
|---|---|---|
| `player.png` | Personagem | Spritesheet 8 colunas x 2 linhas, quadro de 80x80. Linha de baixo: correndo. Linha de cima: pulando |
| `spike.png` | Obstáculo | Spritesheet 4 colunas x 1 linha, quadro de 72x100 |
| `ground.png` | Chão | Bloco de 100x100, repetido lado a lado |
| `bg_sky.png` | Fundo, camada 1 | Céu, parado na tela (`scrollRate` 0) |
| `bg_far.png` | Fundo, camada 2 | Prédios distantes (`scrollRate` 0.25) |
| `bg_near.png` | Fundo, camada 3 | Prédios próximos (`scrollRate` 0.5) |

As imagens foram geradas pelo script `PG2026-2-main/misc/gerar_assets.py`
(só Python, sem bibliotecas extras). Não é preciso rodá-lo para compilar.

Para trocar uma imagem por outra:

- Mantenha o nome do arquivo, ou mude o nome em `main.cpp`.
- Se a spritesheet tiver outro número de quadros ou de animações, ajuste
  `PLAYER_FRAMES`, `PLAYER_ANIMATIONS` e `OBSTACLE_FRAMES` em `Game.h`.
- Os fundos precisam emendar consigo mesmos na horizontal, porque se repetem.

## Fonte, música e efeitos sonoros

| Arquivo | Uso |
|---|---|
| `fonts/PressStart2P-Regular.ttf` | Fonte de todos os textos. Licença SIL Open Font License 1.1 (`fonts/OFL.txt`) |
| `Arcade_Rush.mp3` | Música da partida |
| `sfx/select.wav` | Trocar de opção no menu |
| `sfx/confirm.wav` | Confirmar a opção |
| `sfx/jump.wav` | Pular |
| `sfx/death.wav` | Morrer |

Os efeitos foram sintetizados pelo script `PG2026-2-main/misc/gerar_sfx.py`
(só Python, sem bibliotecas extras). Para ajustar um som, mude a frequência, a
duração ou o volume dele no script e rode de novo; ou troque o `.wav` por
outro, mantendo o nome.

O `Game` não toca som nenhum: ele só registra em `events` o que aconteceu no
frame (trocou de opção, confirmou, pulou, morreu). O `main.cpp` lê essa lista
e toca o efeito de cada evento.

A fonte é carregada em três tamanhos (16, 24 e 48 pixels) em `main.cpp`. Ela
é desenhada numa grade de 8x8, então fica nítida em múltiplos de 8. Os textos
aceitam só caracteres sem acento.

Para trocar a música, mantenha o nome do arquivo ou mude o nome em `main.cpp`.
Funciona com mp3, wav e flac.

## Onde o jogo procura os arquivos

O jogo acha a pasta `assets/` pelo caminho absoluto que o CMake grava na
compilação (`ASSETS_DIR`), então funciona de qualquer pasta. Se a pasta do
repositório for movida, recompile.
