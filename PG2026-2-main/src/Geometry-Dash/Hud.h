// Hud.h - AS DUAS PESSOAS
// Tudo que é desenhado por cima do mundo e fica parado na tela: placar,
// menu inicial e tela de game over. Só lê o Game; não altera nada nele.
#pragma once

#include "Font.h"
#include "Game.h"
#include "Renderer.h"

// A mesma fonte em três tamanhos (cada tamanho tem o seu atlas)
struct HudFonts
{
    Font small;  // placar e dicas
    Font medium; // opções de menu
    Font large;  // títulos
};

// Chamar depois de renderer.beginHUD()
void drawHud(Renderer &renderer, const HudFonts &fonts, const Game &game);
