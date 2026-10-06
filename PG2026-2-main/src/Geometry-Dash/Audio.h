// Música de fundo e efeitos sonoros, tocados com a biblioteca miniaudio.
// Se o áudio falhar (sem placa de som, arquivo ausente...), as funções avisam
// no console e o jogo continua rodando sem som.
#pragma once

#include <string>

// Chamar uma única vez, no início do programa
bool initAudio();

// Carrega o arquivo de música (mp3, wav, flac). Ainda não toca.
bool loadMusic(const std::string &path);

// Toca a música desde o começo, repetindo ao chegar no fim
void playMusic();

// Para a música
void stopMusic();

// Efeitos sonoros (sons curtos). Tocam por cima da música, sem interrompê-la.
enum class Sfx
{
    Select,  // trocar de opção no menu
    Confirm, // confirmar a opção
    Jump,    
    Death    
};

// Carrega o arquivo de um efeito. Ainda não toca.
bool loadSfx(Sfx sfx, const std::string &path);

// Toca o efeito desde o começo. Se ele já estiver tocando, recomeça.
void playSfx(Sfx sfx);

// Libera a música, os efeitos e o dispositivo de áudio
void shutdownAudio();
