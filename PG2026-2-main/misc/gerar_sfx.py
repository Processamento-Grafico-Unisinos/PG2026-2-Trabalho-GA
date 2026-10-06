"""Gera os efeitos sonoros do jogo (pasta assets/sfx/) usando só a biblioteca
padrão do Python. Os sons são sintetizados: ondas quadradas e ruído, no estilo
dos videogames de 8 bits.

Uso, a partir da pasta PG2026-2-main:

    python misc/gerar_sfx.py

Os arquivos .wav ficam versionados em assets/sfx/, então NÃO é preciso rodar
este script para compilar ou jogar. Ele existe para quem quiser ajustar os sons:
mude as frequências (Hz), as durações (s) ou os volumes abaixo e rode de novo.

  select.wav   trocar de opção no menu
  confirm.wav  confirmar a opção
  jump.wav     pular
  death.wav    morrer
"""
import math
import os
import random
import struct
import wave

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "sfx")
RATE = 44100  # amostras por segundo


def tone(duration, freq_start, freq_end, volume, duty=0.5, noise=0.0, decay=4.0):
    """Um trecho de onda quadrada cuja frequência vai de freq_start a freq_end.

    duty   fração do ciclo em que a onda fica "em cima" (0.5 = quadrada pura;
           valores menores dão um timbre mais fino)
    noise  quanto de ruído misturar (0 = nenhum, 1 = só ruído)
    decay  quão rápido o volume cai ao longo do trecho (maior = mais seco)
    """
    total = int(duration * RATE)
    rnd = random.Random(1)
    samples = []
    phase = 0.0
    for i in range(total):
        t = i / total  # 0 no começo do trecho, 1 no fim
        phase += (freq_start + (freq_end - freq_start) * t) / RATE
        square = 1.0 if (phase % 1.0) < duty else -1.0
        value = square * (1.0 - noise) + rnd.uniform(-1.0, 1.0) * noise

        # Envelope: sobe em 2 ms (evita estalo no início), cai exponencialmente
        # e chega a zero no fim (evita estalo no final)
        attack = min(1.0, i / (0.002 * RATE))
        envelope = attack * math.exp(-decay * t) * min(1.0, (1.0 - t) * 10.0)
        samples.append(value * volume * envelope)
    return samples


def save(name, samples):
    path = os.path.join(OUT_DIR, name)
    with wave.open(path, "wb") as f:
        f.setnchannels(1)   # mono
        f.setsampwidth(2)   # 16 bits
        f.setframerate(RATE)
        f.writeframes(b"".join(struct.pack("<h", round(max(-1.0, min(1.0, s)) * 32767)) for s in samples))
    print(f"{name}: {len(samples) / RATE * 1000:.0f} ms, pico {max(abs(s) for s in samples):.2f}")


if __name__ == "__main__":
    os.makedirs(OUT_DIR, exist_ok=True)

    # Trocar de opção: um "blip" curto e agudo
    save("select.wav", tone(0.07, 880, 880, 0.28, duty=0.25, decay=5.0))

    # Confirmar: duas notas subindo (a segunda uma quinta acima)
    save("confirm.wav", tone(0.07, 660, 660, 0.30, decay=2.0) + tone(0.16, 990, 990, 0.30, decay=4.0))

    # Pular: a frequência sobe rápido, dando a sensação de impulso
    save("jump.wav", tone(0.16, 300, 760, 0.28, duty=0.25, decay=3.0))

    # Morrer: tom descendo, com ruído misturado
    save("death.wav", tone(0.5, 440, 55, 0.40, noise=0.35, decay=3.5))
