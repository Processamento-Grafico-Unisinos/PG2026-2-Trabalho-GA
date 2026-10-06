#include "Audio.h"
#include <iostream>

// A implementação da miniaudio deve ser compilada em UM único .cpp do projeto.
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

static ma_engine engine; // dispositivo de saída + mixer
static ma_sound music;   // a música carregada
static bool engineReady = false;
static bool musicReady = false;

static const int SFX_COUNT = 4; // um para cada valor do enum Sfx
static ma_sound sfxSounds[SFX_COUNT];
static bool sfxReady[SFX_COUNT] = {};

bool initAudio()
{
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
    {
        std::cerr << "ERRO: nao foi possivel iniciar o audio. O jogo vai rodar sem som." << std::endl;
        return false;
    }
    engineReady = true;
    return true;
}

bool loadMusic(const std::string &path)
{
    if (!engineReady)
        return false;

    // STREAM: decodifica o arquivo aos poucos enquanto toca, em vez de
    // descompactar a música inteira para a memória na carga.
    if (ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &music) != MA_SUCCESS)
    {
        std::cerr << "ERRO: nao foi possivel carregar a musica '" << path << "'." << std::endl;
        return false;
    }
    ma_sound_set_looping(&music, MA_TRUE);
    musicReady = true;
    return true;
}

void playMusic()
{
    if (!musicReady)
        return;
    ma_sound_seek_to_pcm_frame(&music, 0); // volta para o começo
    ma_sound_start(&music);
}

void stopMusic()
{
    if (!musicReady)
        return;
    ma_sound_stop(&music);
}

bool loadSfx(Sfx sfx, const std::string &path)
{
    int index = static_cast<int>(sfx);
    if (!engineReady || sfxReady[index])
        return false;

    // DECODE: ao contrário da música, o efeito é curto, então é descompactado
    // inteiro para a memória na carga e toca sem atraso.
    if (ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &sfxSounds[index]) != MA_SUCCESS)
    {
        std::cerr << "ERRO: nao foi possivel carregar o efeito sonoro '" << path << "'." << std::endl;
        return false;
    }
    sfxReady[index] = true;
    return true;
}

void playSfx(Sfx sfx)
{
    int index = static_cast<int>(sfx);
    if (!sfxReady[index])
        return;
    ma_sound_seek_to_pcm_frame(&sfxSounds[index], 0); // volta para o começo
    ma_sound_start(&sfxSounds[index]);
}

void shutdownAudio()
{
    for (int i = 0; i < SFX_COUNT; i++)
    {
        if (sfxReady[i])
            ma_sound_uninit(&sfxSounds[i]);
        sfxReady[i] = false;
    }
    if (musicReady)
        ma_sound_uninit(&music);
    if (engineReady)
        ma_engine_uninit(&engine);
    musicReady = false;
    engineReady = false;
}
