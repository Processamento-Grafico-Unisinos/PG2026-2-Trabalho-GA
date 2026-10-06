// Hud.cpp - AS DUAS PESSOAS
#include "Hud.h"
#include "Config.h"

#include <cmath>
#include <cstdio>
#include <string>

// Todas as posições deste arquivo estão em coordenadas de TELA:
// (0, 0) no canto inferior esquerdo, (800, 600) no superior direito.

static const glm::vec4 WHITE(1.0f, 1.0f, 1.0f, 1.0f);
static const glm::vec4 YELLOW(1.0f, 0.86f, 0.31f, 1.0f);
static const glm::vec4 CYAN(0.2f, 0.8f, 1.0f, 1.0f);
static const glm::vec4 RED(1.0f, 0.33f, 0.33f, 1.0f);
static const glm::vec4 GRAY(0.75f, 0.8f, 0.95f, 1.0f);
static const glm::vec4 SHADOW(0.03f, 0.03f, 0.12f, 1.0f);

static const glm::vec2 SCREEN_CENTER(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f);
static const glm::vec2 SCREEN_SIZE(WORLD_WIDTH, WORLD_HEIGHT);

// Texto com sombra: o mesmo texto desenhado duas vezes, a primeira em cor
// escura e deslocada para baixo e para a direita.
static void drawShadowText(Renderer &renderer, const Font &font, const std::string &text,
                           float x, float y, const glm::vec4 &color)
{
    float offset = font.size / 8.0f; // 1 "pixel" da fonte, que é desenhada numa grade de 8x8
    renderer.drawText(font, text, x + offset, y - offset, SHADOW);
    renderer.drawText(font, text, x, y, color);
}

// Igual, mas centralizado na horizontal em torno de centerX
static void drawCenteredText(Renderer &renderer, const Font &font, const std::string &text,
                             float centerX, float y, const glm::vec4 &color)
{
    drawShadowText(renderer, font, text, centerX - font.textWidth(text) * 0.5f, y, color);
}

// Segundos -> "MM:SS". O (int) descarta a fração, então o relógio só muda
// quando um segundo inteiro se completa.
static std::string formatTime(float seconds)
{
    int total = (int)seconds;
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", total / 60, total % 60);
    return buffer;
}

// Placar no canto superior esquerdo: rótulo em amarelo, valor em branco
static void drawScoreboard(Renderer &renderer, const Font &font, const Game &game)
{
    const float left = 16.0f;
    const float scoreY = WORLD_HEIGHT - 16.0f - font.size; // 16 px abaixo da borda de cima
    const float timeY = scoreY - font.size - 10.0f;        // linha seguinte
    const float valueX = left + font.textWidth("Score: ");

    drawShadowText(renderer, font, "Score:", left, scoreY, YELLOW);
    drawShadowText(renderer, font, std::to_string(game.score), valueX, scoreY, WHITE);

    drawShadowText(renderer, font, "Time:", left, timeY, YELLOW);
    drawShadowText(renderer, font, formatTime(game.timeAlive), valueX, timeY, WHITE);
}

// Lista de opções da tela atual. A selecionada ganha um fundo, cor de destaque
// e setas que piscam.
static void drawOptions(Renderer &renderer, const Font &font, const Game &game)
{
    bool blinkOn = std::fmod(game.stateTime, 0.8f) < 0.55f;

    for (size_t i = 0; i < game.options.size(); i++)
    {
        const MenuOption &option = game.options[i];
        bool selected = ((int)i == game.selectedOption);

        // As setas são trocadas por espaços (e não removidas) para o texto não
        // mudar de largura e ficar "pulando" de lugar
        std::string text = option.label;
        if (selected)
        {
            renderer.drawRect(option.position, option.size, glm::vec4(1.0f, 1.0f, 1.0f, 0.14f));
            text = (blinkOn ? "> " : "  ") + text + (blinkOn ? " <" : "  ");
        }

        // Linha de base que deixa as maiúsculas no meio do retângulo
        float baseline = option.position.y - font.capHeight * 0.5f;
        drawCenteredText(renderer, font, text, option.position.x, baseline, selected ? YELLOW : GRAY);
    }
}

static void drawMenu(Renderer &renderer, const HudFonts &fonts, const Game &game)
{
    renderer.drawRect(SCREEN_CENTER, SCREEN_SIZE, glm::vec4(0.0f, 0.0f, 0.1f, 0.5f)); // escurece a cena

    drawCenteredText(renderer, fonts.large, "GEOMETRY DASH", SCREEN_CENTER.x, 430.0f, CYAN);
    drawCenteredText(renderer, fonts.small, "Grau A", SCREEN_CENTER.x, 385.0f, WHITE);

    drawOptions(renderer, fonts.medium, game);

    drawCenteredText(renderer, fonts.small, "Setas: escolher   Enter: confirmar", SCREEN_CENTER.x, 42.0f, GRAY);
}

static void drawGameOver(Renderer &renderer, const HudFonts &fonts, const Game &game)
{
    renderer.drawRect(SCREEN_CENTER, SCREEN_SIZE, glm::vec4(0.1f, 0.0f, 0.0f, 0.6f)); // escurece a cena

    drawCenteredText(renderer, fonts.large, "GAME OVER", SCREEN_CENTER.x, 420.0f, RED);

    drawCenteredText(renderer, fonts.medium, "Score: " + std::to_string(game.score), SCREEN_CENTER.x, 345.0f, WHITE);
    drawCenteredText(renderer, fonts.medium, "Time: " + formatTime(game.timeAlive), SCREEN_CENTER.x, 300.0f, WHITE);

    drawOptions(renderer, fonts.medium, game);
}

void drawHud(Renderer &renderer, const HudFonts &fonts, const Game &game)
{
    switch (game.state)
    {
    case GameState::Menu:
        drawMenu(renderer, fonts, game);
        break;
    case GameState::Playing:
        drawScoreboard(renderer, fonts.small, game);
        break;
    case GameState::GameOver:
        drawScoreboard(renderer, fonts.small, game);
        drawGameOver(renderer, fonts, game);
        break;
    }
}
