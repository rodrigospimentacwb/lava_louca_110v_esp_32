"""Gabaritos de teste: confere a furação das placas e do fan antes de imprimir a caixa inteira.

Cada gabarito é uma moldura fina do tamanho da placa, com as colunas (inserto M2) nas
mesmas posições usadas em caixa.py. A placa encaixada deve cobrir a moldura e os furos
devem cair no centro das colunas. No dos relés, a aba marca o lado do jumper JD-VCC. O do fan é um quadrado de 70 mm com os 4 furos M3.

Rode:
    .venv/bin/python gabarito.py
Gera em desenhos/: gabaritos.stl e gabaritos.3mf (tudo numa placa da A1).
"""
import os
import trimesh
from caixa import (box, cyl, to_trimesh, M2_INSERT_D, M2_INSERT_DEPTH, SCREW_D,
                   RELAY_W, RELAY_L, RELAY_HOLES, RELAY_STANDOFF_D,
                   ESP_W, ESP_L, ESP_HOLE_DX, ESP_HOLE_DY, ESP_PILLAR_D,
                   LM_W, LM_L, LM_HOLES, LM_STANDOFF_D,
                   FAN, FAN_HOLE_SPACING, FAN_OPEN_D)

PLATE_T = 1.2                 # espessura da moldura
FRAME_W = 7.0                 # largura da moldura (as colunas ficam em cima dela)
POST_H = 6.5                  # altura da coluna a partir da mesa
GAP = 10.0                    # espaço entre os gabaritos


def frame(w, l):
    """Moldura com o contorno da placa: a placa em cima deve cobrir exatamente isto."""
    return box(0, 0, 0, w, l, PLATE_T) - box(FRAME_W, FRAME_W, -1, w - 2 * FRAME_W, l - 2 * FRAME_W, PLATE_T + 2)


def board_template(w, l, holes, post_d):
    m = frame(w, l)
    for hx, hy in holes:
        m = m + cyl(hx, hy, 0, post_d, POST_H)
    for hx, hy in holes:
        m = m - cyl(hx, hy, POST_H - M2_INSERT_DEPTH, M2_INSERT_D, M2_INSERT_DEPTH + 1)
    return m


def centered_holes(w, l, dx, dy):
    return [(w / 2 + sx * dx / 2, l / 2 + sy * dy / 2) for sx in (-1, 1) for sy in (-1, 1)]


relay = board_template(RELAY_W, RELAY_L, RELAY_HOLES, RELAY_STANDOFF_D)
# Aba no meio da borda do lado do JD-VCC: mostra para que lado encaixar a placa
relay = relay + box(RELAY_W / 2 - 5, RELAY_L - 0.1, 0, 10, 6, PLATE_T)
esp = board_template(ESP_W, ESP_L, centered_holes(ESP_W, ESP_L, ESP_HOLE_DX, ESP_HOLE_DY), ESP_PILLAR_D)
lm = board_template(LM_W, LM_L, LM_HOLES, LM_STANDOFF_D)

# Fan: quadrado de 70 mm, abertura das pás e os 4 furos de passagem M3
fan = box(0, 0, 0, FAN, FAN, 2.4) - cyl(FAN / 2, FAN / 2, -1, FAN_OPEN_D, 4.4, 96)
for sx in (-1, 1):
    for sy in (-1, 1):
        fan = fan - cyl(FAN / 2 + sx * FAN_HOLE_SPACING / 2, FAN / 2 + sy * FAN_HOLE_SPACING / 2, -1, SCREW_D, 4.0)

# Lado a lado na mesa
parts, x = [], 0.0
for name, m, w in (("reles", relay, RELAY_W), ("esp32", esp, ESP_W), ("lm2596", lm, LM_W), ("fan", fan, FAN)):
    parts.append((name, m.translate([x, 0, 0])))
    x += w + GAP

if __name__ == "__main__":
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "desenhos")
    os.makedirs(out, exist_ok=True)
    meshes = {name: to_trimesh(m) for name, m in parts}
    for name, t in meshes.items():
        print(f"{name:7s} fechado: {t.is_watertight} tamanho {t.extents.round(1)}")
    trimesh.util.concatenate(list(meshes.values())).export(os.path.join(out, "gabaritos.stl"))
    trimesh.Scene(meshes).export(os.path.join(out, "gabaritos.3mf"))
