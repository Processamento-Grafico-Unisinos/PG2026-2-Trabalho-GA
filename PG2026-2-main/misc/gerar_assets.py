"""Gera as imagens do jogo (pasta assets/) usando só a biblioteca padrão do Python.

Uso, a partir da pasta PG2026-2-main:

    python misc/gerar_assets.py

As imagens ficam versionadas em assets/, então NÃO é preciso rodar este script
para compilar ou jogar. Ele existe para quem quiser mudar cores e formas, ou
para servir de referência dos tamanhos caso as imagens sejam trocadas por outras.

Convenções que o código do jogo espera (ver Game.h):
  player.png  spritesheet 8 colunas x 2 linhas, quadro de 80x80
              linha de BAIXO = correndo (iAnimation 0), linha de CIMA = pulando (1)
  spike.png   spritesheet 4 colunas x 1 linha, quadro de 72x100
  ground.png  bloco de 100x100, repetido lado a lado
  bg_*.png    camadas de fundo; a altura da imagem ocupa a altura da tela
"""
import math
import os
import random
import struct
import zlib

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")


def lerp(a, b, t):
    """Interpola duas cores (t = 0 devolve a, t = 1 devolve b)."""
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))


class Canvas:
    """Imagem RGBA em memória. Origem no canto superior esquerdo, como no PNG."""

    def __init__(self, w, h, matte=(0, 0, 0)):
        # "matte" é a cor RGB dos pixels transparentes. Ela não aparece, mas
        # evita uma borda escura quando a GPU interpola (GL_LINEAR) um pixel
        # opaco com o vizinho transparente.
        self.w, self.h = w, h
        self.px = bytearray(bytes((*matte, 0)) * (w * h))

    def rect(self, x0, y0, x1, y1, color):
        """Preenche [x0, x1) x [y0, y1) com uma cor opaca."""
        x0, y0 = max(0, x0), max(0, y0)
        x1, y1 = min(self.w, x1), min(self.h, y1)
        if x0 >= x1 or y0 >= y1:
            return
        row = bytes((*color, 255)) * (x1 - x0)
        for y in range(y0, y1):
            i = (y * self.w + x0) * 4
            self.px[i:i + len(row)] = row

    def put(self, x, y, color, alpha):
        """Grava um pixel (alpha de 0 a 255), sem misturar com o que havia."""
        i = (y * self.w + x) * 4
        self.px[i:i + 4] = bytes((*color, alpha))

    def blend(self, x, y, color, alpha):
        """Mistura uma cor sobre o pixel existente (alpha de 0.0 a 1.0)."""
        if not (0 <= x < self.w and 0 <= y < self.h) or alpha <= 0.0:
            return
        i = (y * self.w + x) * 4
        dst_a = self.px[i + 3] / 255.0
        out_a = alpha + dst_a * (1.0 - alpha)
        for c in range(3):
            self.px[i + c] = round((color[c] * alpha + self.px[i + c] * dst_a * (1.0 - alpha)) / out_a)
        self.px[i + 3] = round(out_a * 255)

    def save(self, name):
        stride = self.w * 4
        raw = bytearray()
        for y in range(self.h):
            raw.append(0)  # filtro 0 (nenhum) em cada linha
            raw += self.px[y * stride:(y + 1) * stride]

        def chunk(tag, data):
            return (struct.pack(">I", len(data)) + tag + data +
                    struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

        header = struct.pack(">IIBBBBB", self.w, self.h, 8, 6, 0, 0, 0)  # 8 bits, RGBA
        png = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) +
               chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b""))

        path = os.path.join(OUT_DIR, name)
        with open(path, "wb") as f:
            f.write(png)
        print(f"{name}: {self.w}x{self.h}, {len(png)} bytes")


# ---------------------------------------------------------------------------
# Personagem: cubo com rosto, 8 quadros x 2 animações
# ---------------------------------------------------------------------------
def make_player():
    FRAME, COLS, ROWS = 80, 8, 2
    OUTLINE = (12, 28, 48)
    BODY = (51, 204, 255)
    WHITE = (255, 255, 255)

    c = Canvas(FRAME * COLS, FRAME * ROWS, matte=OUTLINE)
    for row in range(ROWS):
        jumping = (row == 0)  # linha de cima da imagem = pulo
        for f in range(COLS):
            ox, oy = f * FRAME, row * FRAME

            def r(x0, y0, x1, y1, color):
                c.rect(ox + x0, oy + y0, ox + x1, oy + y1, color)

            # 2 pixels de margem transparente em volta de cada quadro, para um
            # quadro não "vazar" no vizinho com o filtro GL_LINEAR
            r(2, 2, 78, 78, OUTLINE)
            r(8, 8, 72, 72, BODY)

            # miolo que pulsa ao longo dos 8 quadros (vai e volta)
            pulse = 0.5 - 0.5 * math.cos(2.0 * math.pi * f / COLS)
            glow = (255, 250, 170) if jumping else (170, 240, 255)
            r(14, 14, 66, 66, lerp(BODY, glow, pulse))

            for ex in (20, 46):  # olho esquerdo e direito
                if jumping:  # olhos arregalados, olhando para cima
                    r(ex - 1, 18, ex + 15, 38, WHITE)
                    r(ex + 5, 20, ex + 11, 26, OUTLINE)
                elif f == 7:  # piscando: fechado
                    r(ex, 28, ex + 14, 32, OUTLINE)
                elif f == 6:  # piscando: meio fechado
                    r(ex, 25, ex + 14, 33, WHITE)
                    r(ex + 7, 27, ex + 13, 33, OUTLINE)
                else:  # aberto, olhando para a frente
                    r(ex, 22, ex + 14, 36, WHITE)
                    r(ex + 7, 26, ex + 13, 32, OUTLINE)

            if jumping:  # boca aberta
                r(30, 46, 50, 60, OUTLINE)
                r(34, 54, 46, 60, (230, 80, 90))
            else:
                r(24, 48, 56, 54, OUTLINE)

    c.save("player.png")


# ---------------------------------------------------------------------------
# Obstáculo: espinho que pulsa, 4 quadros
# ---------------------------------------------------------------------------
def make_spike():
    FW, FH, COLS = 72, 100, 4
    SS = 4  # amostras por pixel em cada eixo (suaviza as bordas inclinadas)
    EDGE = (250, 240, 240)
    BORDER = 5.0
    apex, left, right = (36.0, 2.0), (2.0, 100.0), (70.0, 100.0)

    def inside_dist(p, a, b, ref):
        """Distância de p até a reta a-b, positiva do lado em que está ref."""
        ex, ey = b[0] - a[0], b[1] - a[1]
        length = math.hypot(ex, ey)
        d = (ex * (p[1] - a[1]) - ey * (p[0] - a[0])) / length
        side = ex * (ref[1] - a[1]) - ey * (ref[0] - a[0])
        return d if side > 0 else -d

    c = Canvas(FW * COLS, FH, matte=EDGE)
    for f in range(COLS):
        pulse = 0.5 - 0.5 * math.cos(2.0 * math.pi * f / COLS)
        for y in range(FH):
            for x in range(FW):
                acc = [0, 0, 0]
                hits = 0
                for sy in range(SS):
                    for sx in range(SS):
                        p = (x + (sx + 0.5) / SS, y + (sy + 0.5) / SS)
                        d = min(inside_dist(p, left, apex, right),
                                inside_dist(p, apex, right, left),
                                inside_dist(p, right, left, apex))
                        if d < 0.0:
                            continue  # fora do triângulo
                        if d < BORDER:
                            color = EDGE
                        else:
                            fill = lerp((255, 90, 70), (140, 15, 35), (p[1] - apex[1]) / (FH - apex[1]))
                            color = lerp(fill, (255, 200, 120), 0.45 * pulse)
                        hits += 1
                        for i in range(3):
                            acc[i] += color[i]
                if hits:
                    c.put(f * FW + x, y, tuple(round(v / hits) for v in acc),
                          round(255 * hits / (SS * SS)))

    c.save("spike.png")


# ---------------------------------------------------------------------------
# Chão: um bloco que se repete lado a lado
# ---------------------------------------------------------------------------
def make_ground():
    S = 100
    c = Canvas(S, S)
    for y in range(S):
        c.rect(0, y, S, y + 1, lerp((44, 88, 170), (14, 28, 72), y / (S - 1)))

    c.rect(0, 0, S, 5, (210, 235, 255))  # linha clara no topo, onde o personagem pisa
    c.rect(0, 5, 2, S, (20, 42, 100))    # emenda entre um bloco e o próximo

    # moldura decorativa no meio do bloco
    frame = (78, 128, 205)
    c.rect(18, 22, 84, 24, frame)
    c.rect(18, 84, 84, 86, frame)
    c.rect(18, 22, 20, 86, frame)
    c.rect(82, 22, 84, 86, frame)

    c.save("ground.png")


# ---------------------------------------------------------------------------
# Fundos
# ---------------------------------------------------------------------------
def make_sky():
    W, H = 800, 600
    c = Canvas(W, H)
    for y in range(H):
        c.rect(0, y, W, y + 1, lerp((16, 12, 58), (112, 72, 160), (y / (H - 1)) ** 1.3))

    rnd = random.Random(7)
    star = (255, 255, 240)
    for _ in range(110):
        x, y = rnd.randrange(W), rnd.randrange(400)
        brightness = rnd.uniform(0.25, 0.9) * (1.0 - y / 520.0)  # mais fracas perto do horizonte
        c.blend(x, y, star, brightness)
        if rnd.random() < 0.2:  # algumas estrelas maiores, em forma de cruz
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                c.blend(x + dx, y + dy, star, brightness * 0.4)

    # lua crescente: um disco menos outro disco deslocado
    cx, cy, radius = 640, 120, 44
    for y in range(cy - radius - 1, cy + radius + 2):
        for x in range(cx - radius - 1, cx + radius + 2):
            disc = min(1.0, max(0.0, radius - math.hypot(x - cx, y - cy) + 0.5))
            cut = min(1.0, max(0.0, 40 - math.hypot(x - (cx - 18), y - (cy - 10)) + 0.5))
            c.blend(x, y, (245, 238, 205), disc * (1.0 - cut))

    c.save("bg_sky.png")


def make_skyline(name, width, seed, min_height, max_height, body, window_off, window_on, lit_chance):
    """Fileira de prédios, transparente acima deles. Nenhum prédio cruza a borda
    da imagem, então ela emenda consigo mesma ao se repetir (GL_REPEAT)."""
    H = 600
    MIN_W, MAX_W = 60, 130
    rnd = random.Random(seed)
    c = Canvas(width, H, matte=body)

    x = 0
    while x < width:
        bw = rnd.randint(MIN_W, MAX_W)
        if width - (x + bw) < MIN_W:
            bw = width - x  # o último prédio vai até a borda
        top = H - rnd.randint(min_height, max_height)
        shade = rnd.randint(-6, 6)
        color = tuple(max(0, min(255, v + shade)) for v in body)

        c.rect(x, top, x + bw, H, color)
        if rnd.random() < 0.4:  # antena
            c.rect(x + bw // 2 - 3, top - 18, x + bw // 2 + 3, top, color)

        for wy in range(top + 12, H - 8, 20):
            for wx in range(x + 8, x + bw - 12, 16):
                c.rect(wx, wy, wx + 8, wy + 11, window_on if rnd.random() < lit_chance else window_off)
        x += bw

    c.save(name)


if __name__ == "__main__":
    os.makedirs(OUT_DIR, exist_ok=True)
    make_player()
    make_spike()
    make_ground()
    make_sky()
    make_skyline("bg_far.png", 960, 1, 220, 400, (60, 50, 110), (70, 60, 124), (118, 104, 172), 0.25)
    make_skyline("bg_near.png", 720, 7, 140, 280, (40, 35, 80), (52, 46, 98), (235, 200, 110), 0.30)
