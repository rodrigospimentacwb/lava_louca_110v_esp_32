# Caixa impressa

Caixa para a ESP32 DevKit V1, o módulo de 4 relés, o step-down LM2596 e um fan de 70 mm, desenhada em Python (paramétrica) para imprimir na Bambu Lab A1.

| Arquivo | O que é |
|---|---|
| `caixa.py` | desenho da base e da tampa; todas as medidas ficam no topo, em `PARÂMETROS` |
| `gabarito.py` | gabaritos finos para conferir a furação das placas e do fan antes de imprimir a caixa (usa as medidas do `caixa.py`) |
| `desenhos/caixa_lava_loucas.3mf` | base e tampa para o Bambu Studio |
| `desenhos/caixa_base.stl`, `caixa_tampa.stl` | as peças separadas |
| `desenhos/gabaritos.3mf`, `gabaritos.stl` | os gabaritos (cerca de 10 g) |
| `desenhos/previa_*.png` | cortes vistos de cima, para conferir sem abrir o fatiador |

## Leiaute

- **Lado oeste, baixa tensão:** ESP32 (colunas de 12 mm; os pinos ficam soldados embaixo, sem nada ligado, e os fios são soldados por cima), LM2596, dois furos PG7 (sensores e entrada de 12 V), fendas de saída de ar e o furo da USB.
- **Lado leste, 110 V:** módulo de relés com os bornes virados para a parede e cinco furos para prensa-cabos (2 × PG11, 3 × PG9).
- **Divisória** entre os dois lados, com uma janela em frente à barra de pinos do módulo (`GND IN1–IN4 VCC`) e fendas finas para o ar.
- **Âncoras de abraçadeira** no fundo, ao lado de cada furo de cabo: enquanto não houver prensa-cabo, o cabo fica preso por abraçadeira e um puxão não chega ao borne.
- **Tampa:** fan Delta AFB0712HB (12 V, 70 × 70 × 16 mm) sobre os relés, soprando para dentro, com grade. É impressa com a face de fora na mesa.

## Fixação

| Onde | Inserto | Parafuso |
|---|---|---|
| Tampa (4 cantos) | M3 × 6 × 4,2 (furo de 4,0 mm) | M3 × 8 |
| ESP32, relés, LM2596 | M2 × 4 × 3,5 (furo de 3,2 mm) | M2 × 5 |
| Fan | — | M3 × 25 com porca e arruela |

As placas têm furos de 3 mm: a cabeça de um M1.6 passaria por eles.

## Medidas

Conferidas com os gabaritos em 2026-10-18: ESP32, módulo de relés (os furos **não** formam um retângulo; ver `RELAY_HOLES`) e fan (62 mm entre furos). A posição dos furos do **LM2596** ainda é estimada (`CONFERIR` no script).

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
```
