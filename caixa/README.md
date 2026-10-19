# Caixa impressa

Caixa para a ESP32 DevKit V1, o módulo de 4 relés, o step-down LM2596 e um fan de 70 mm, desenhada em Python (paramétrica) para imprimir na Bambu Lab A1.

| Arquivo | O que é |
|---|---|
| `caixa.py` | desenho da base e da tampa; todas as medidas ficam no topo, em `PARÂMETROS` |
| `gabarito.py` | gabaritos finos para conferir a furação das placas e do fan antes de imprimir a caixa (usa as medidas do `caixa.py`); cada coluna tem um pino de 2,6 mm, então a placa só encaixa se os furos estiverem certos |
| `desenhos/caixa_lava_loucas.3mf` | base e tampa para o Bambu Studio |
| `desenhos/caixa_base.stl`, `caixa_tampa.stl` | as peças separadas |
| `desenhos/caixa_bambu.3mf` | projeto do Bambu Studio salvo (não é gerado pelo script). **Tem a base antiga, com colunas M2**: a tampa continua valendo, a base não |
| `desenhos/gabaritos.3mf`, `gabaritos.stl`, `gabarito_reles.stl` | os gabaritos (cerca de 10 g); o dos relés também separado |
| `desenhos/previa_*.png` | cortes vistos de cima, para conferir sem abrir o fatiador |
| `render.py`, `desenhos/render_*.png` | imagens 3D da caixa usadas no README principal |

## Leiaute

- **Lado oeste, baixa tensão:** ESP32 no meio (colunas de 12 mm; os pinos ficam soldados embaixo, sem nada ligado, e os fios são soldados por cima), LM2596 atrás dela, dois furos PG7 (sensores e entrada de 12 V), fendas de saída de ar e o furo da USB (13 × 9 mm, a ESP fica a 6 mm da parede).
- **Lado leste, 110 V:** módulo de relés com os bornes virados para a parede e cinco furos para prensa-cabos (2 × PG11, 3 × PG9).
- **Divisória** entre os dois lados, com uma janela em frente à barra de pinos do módulo (`GND IN1–IN4 VCC`) e fendas finas para o ar.
- **Âncoras de abraçadeira** no fundo, ao lado de cada furo de cabo: enquanto não houver prensa-cabo, o cabo fica preso por abraçadeira e um puxão não chega ao borne.
- **Tampa:** fan Delta AFB0712HB (12 V, 70 × 70 × 16 mm) sobre os relés, soprando para dentro, com grade. É impressa com a face de fora na mesa.

## Fixação

| Onde | Inserto | Parafuso |
|---|---|---|
| Tampa (4 cantos) | M3 × 6 × 4,2 (furo de 4,0 mm) | M3 × 8 |
| ESP32, relés, LM2596 (10 colunas de Ø 8 mm) | M3 × 4 × 4,2 (furo de 4,0 mm, 5,5 mm de fundo) | M3 × 6 |
| Fan | — | M3 × 25 com porca e arruela |

As placas têm furos de 3 mm: o M3 entra justo. Se travar, passar uma broca de 3,2 mm à mão no furo da placa (fica a 2 mm da borda, longe das trilhas).

**Colunas e torres longe das paredes:** uns 8–10 mm livres em volta de cada inserto, para o ferro de solda entrar reto sem derreter as laterais. Na primeira versão (colunas M2 coladas nas paredes) foi difícil colocar os insertos.

## Medidas

| Peça | Furos | Como foi conferido |
|---|---|---|
| ESP32 (28 × 52) | 23 × 47 entre centros | gabarito impresso |
| Relés (54 × 73) | Ø 3 mm, centro a 3,5 mm das bordas → 47 × 66 entre centros | paquímetro na placa |
| Fan 70 mm | 62 mm entre centros | gabarito impresso |
| LM2596 (21 × 43) | cantos opostos | **estimado** (`CONFERIR` no script) |

Lição dos relés: conferir pelo gabarito antigo, com colunas que passavam da moldura, levou a correções erradas. Para furação, medir direto na placa com paquímetro (distância da borda do furo até as bordas da placa + raio).

## Imprimir

- **Material: PETG** (ou ASA). PLA amolece perto de 55–60 °C e serve só para teste.
- Base e tampa não cabem juntas na mesa da A1: no Bambu Studio, abrir o 3MF e apertar `A` (Arrange) para separar em duas placas.
- Sem suporte.

## Regerar

```bash
cd caixa
python3 -m venv --without-pip .venv            # o Ubuntu pode não ter o python3-venv completo
curl -sSO https://bootstrap.pypa.io/get-pip.py && .venv/bin/python get-pip.py && rm get-pip.py
.venv/bin/pip install manifold3d trimesh numpy matplotlib
.venv/bin/python -B caixa.py
.venv/bin/python -B gabarito.py
.venv/bin/python -B render.py
```
