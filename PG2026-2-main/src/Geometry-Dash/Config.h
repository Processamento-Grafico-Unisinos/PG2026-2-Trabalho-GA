// Constantes compartilhadas entre o motor de renderização e a lógica do jogo.
#pragma once

// Janela do mundo ("window" dos slides): a porção do mundo que a câmera enxerga.
// Usamos proporção 1:1 com a tela, como em glm::ortho(0, 800, 0, 600, -1, 1):
// origem no canto inferior esquerdo, x para a direita, y para cima.
const float WORLD_WIDTH  = 800.0f;
const float WORLD_HEIGHT = 600.0f;

// Tamanho inicial da janela da aplicação (pixels)
const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;

// Altura do chão no mundo (o topo do chão fica em y = GROUND_HEIGHT)
const float GROUND_HEIGHT = 100.0f;
