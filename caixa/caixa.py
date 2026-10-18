"""Caixa da lava-louças: ESP32 DevKit V1 + módulo de 4 relés + step-down LM2596 + fan 70 mm.

Lado oeste (x pequeno) = baixa tensão: ESP32, LM2596, entrada dos sensores e dos 12 V.
Lado leste (x grande)  = 110 V: módulo de relés e os prensa-cabos das cargas.
Uma divisória separa os dois lados. O fan fica na tampa, sobre os relés, soprando para
dentro; o ar atravessa as fendas da divisória e sai pelas fendas da parede oeste.

Todas as medidas em mm. Ajuste os PARÂMETROS e rode:
    .venv/bin/python caixa.py
Gera em desenhos/: caixa_base.stl, caixa_tampa.stl, caixa_lava_loucas.3mf e prévias em PNG.

Medidas marcadas com "CONFERIR" foram estimadas e precisam do paquímetro.
"""
import os
import numpy as np
import trimesh
from manifold3d import Manifold, CrossSection

# ---------------- PARÂMETROS ----------------
WALL = 2.4                    # parede
FLOOR = 2.4                   # fundo
IN_X, IN_Y, IN_Z = 140.0, 120.0, 40.0   # espaço interno

# Tampa
LID_T = 3.0                   # espessura da tampa (mais grossa que antes: segura o fan)
LIP_H, LIP_T = 3.0, 1.6       # lábio de encaixe
SCREW_D, HEAD_D = 3.4, 6.2    # parafusos M3 da tampa (passagem e cabeça)
BOSS_D = 10.0                 # torres dos cantos

# Insertos de rosca (furo ~0,4 mm menor que o diâmetro externo do inserto)
M3_INSERT_D, M3_INSERT_DEPTH = 4.0, 6.5      # insertos M3 x 6 x 4.2 (furo 0,2 mm menor que o inserto)
M2_INSERT_D, M2_INSERT_DEPTH = 3.2, 5.5      # insertos M2 x 4 x 3.5 (placas: furo de 3 mm, a cabeça do M1.6 passaria)

# Divisória entre baixa tensão e 110 V
DIV_X = 58.0                  # posição (x interno) da face oeste da divisória
DIV_T = 2.0

# Módulo de 4 relés: 54 (x) x 73 (y), bornes virados para o leste
RELAY_W, RELAY_L = 54.0, 73.0
RELAY_X0 = 64.0
RELAY_Y0 = (IN_Y - RELAY_L) / 2
# Furos (x, y) a partir do canto sul-oeste da placa; não formam retângulo. Conferidos no gabarito
# em 2026-10-18. Oeste (x pequeno) = borda dos pinos GND/IN1-4/VCC; norte (y grande) = jumper JD-VCC.
RELAY_HOLES = [(2.0, 3.0), (RELAY_W - 3.0, 3.0), (4.0, RELAY_L - 4.0), (RELAY_W - 4.0, RELAY_L - 4.0)]
RELAY_PINS_Y = 57.0           # centro da barra de pinos de entrada, ao longo da borda oeste
RELAY_STANDOFF_H, RELAY_STANDOFF_D = 6.0, 7.0

# ESP32 DevKit V1: 28 (x) x 52 (y), USB virado para o sul (y = 0)
ESP_W, ESP_L = 28.0, 52.0
ESP_X0, ESP_Y0 = 4.0, 2.0
ESP_HOLE_DX, ESP_HOLE_DY = 23.0, 47.0        # conferido no gabarito em 2026-10-18
ESP_PILLAR_H, ESP_PILLAR_D = 12.0, 6.0       # pinos ficam soldados embaixo, sem nada ligado (~8,5 mm + folga); fios soldados por cima

# Step-down LM2596: 21 (x) x 43 (y), furos em cantos opostos
LM_W, LM_L = 21.0, 43.0
LM_X0, LM_Y0 = 35.0, 2.0
LM_HOLES = [(3.0, 3.5), (LM_W - 3.0, LM_L - 3.5)]  # CONFERIR: posição dos furos no módulo
LM_STANDOFF_H, LM_STANDOFF_D = 6.0, 7.0

# Prensa-cabos (furo para a rosca)
PG7_D, PG9_D, PG11_D = 12.7, 15.4, 18.8
GLAND_Z = 20.0                # altura do centro, acima do fundo interno

# Fan 70 x 70 x 16 na tampa, sobre os relés
FAN = 70.0
FAN_CX = DIV_X + DIV_T + (IN_X - DIV_X - DIV_T) / 2
FAN_CY = IN_Y / 2
FAN_HOLE_SPACING = 62.0       # conferido no gabarito em 2026-10-18 (60 ficou 1 mm para dentro de cada lado)
FAN_OPEN_D = 66.0             # abertura circular sob as pás
# --------------------------------------------

ix, iy, iz = WALL, WALL, FLOOR               # origem do espaço interno
OX, OY = IN_X + 2 * WALL, IN_Y + 2 * WALL
H = FLOOR + IN_Z
corners = [(0, 0), (OX, 0), (0, OY), (OX, OY)]


def box(x, y, z, sx, sy, sz):
    return Manifold.cube([sx, sy, sz]).translate([x, y, z])


def cyl(x, y, z, d, h, n=48):
    return Manifold.cylinder(h, d / 2, d / 2, n).translate([x, y, z])


def hole_x(x, yc, zc, d):
    """Furo redondo atravessando uma parede perpendicular a X (paredes oeste/leste)."""
    return Manifold.cylinder(WALL + 2, d / 2, d / 2, 64).rotate([0, 90, 0]).translate([x - 1, yc, zc])


def hole_y(xc, y, zc, d):
    """Furo redondo atravessando uma parede perpendicular a Y (paredes sul/norte)."""
    return Manifold.cylinder(WALL + 2, d / 2, d / 2, 64).rotate([-90, 0, 0]).translate([xc, y - 1, zc])


def slot_y(xc, y, zc, length_x, height_z):
    """Furo oblongo (pontas redondas) numa parede perpendicular a Y."""
    r = height_z / 2
    s = (CrossSection.circle(r, 40) + CrossSection.circle(r, 40).translate([length_x - height_z, 0])).hull()
    m = s.extrude(WALL + 2).rotate([-90, 0, 0])
    b = m.bounding_box()
    return m.translate([xc - (b[0] + b[3]) / 2, y - 1 - b[1], zc - (b[2] + b[5]) / 2])


def standoff(x, y, h, d, insert_d, insert_depth):
    """Coluna no fundo com furo para inserto de rosca no topo."""
    post = cyl(x, y, iz - 0.1, d, h + 0.1)
    return post, cyl(x, y, iz + h - insert_depth, insert_d, insert_depth + 0.1)


# ---------- BASE ----------
outer = box(0, 0, 0, OX, OY, H)
for cx, cy in corners:
    outer = outer + cyl(cx, cy, 0, BOSS_D, H)
base = outer - box(ix, iy, iz, IN_X, IN_Y, IN_Z + 1)

# Divisória: quase até a tampa; nas pontas fica mais baixa para não bater no lábio da tampa
div = box(ix + DIV_X, iy, iz - 0.1, DIV_T, IN_Y, IN_Z - 0.3)
for y_end in (0, IN_Y - LIP_T - 1.0):
    div = div - box(ix + DIV_X - 1, iy + y_end, iz + IN_Z - LIP_H - 0.6, DIV_T + 2, LIP_T + 1.0, LIP_H + 2)
# Janela para os fios do módulo de relés (GND, IN1-4, VCC), em frente à barra de pinos
WIN_Y = RELAY_Y0 + RELAY_PINS_Y
div = div - box(ix + DIV_X - 1, iy + WIN_Y - 15, iz + 8, DIV_T + 2, 30, 10)
# Fendas finas no alto para o ar do fan passar (3 mm: fio não atravessa)
for i in range(9):
    yy = 12 + i * 11.0
    if abs(yy + 1.5 - WIN_Y) < 17:     # não abrir em cima da janela dos fios
        continue
    div = div - box(ix + DIV_X - 1, iy + yy, iz + 22, DIV_T + 2, 3.0, 12)
base = base + div

# Colunas do módulo de relés (M2)
holes = Manifold()
for hx, hy in RELAY_HOLES:
    p, h = standoff(ix + RELAY_X0 + hx, iy + RELAY_Y0 + hy,
                    RELAY_STANDOFF_H, RELAY_STANDOFF_D, M2_INSERT_D, M2_INSERT_DEPTH)
    base, holes = base + p, holes + h

# Colunas da ESP32 (M2)
ecx, ecy = ix + ESP_X0 + ESP_W / 2, iy + ESP_Y0 + ESP_L / 2
for sx in (-1, 1):
    for sy in (-1, 1):
        p, h = standoff(ecx + sx * ESP_HOLE_DX / 2, ecy + sy * ESP_HOLE_DY / 2,
                        ESP_PILLAR_H, ESP_PILLAR_D, M2_INSERT_D, M2_INSERT_DEPTH)
        base, holes = base + p, holes + h

# Colunas do LM2596 (M2)
for hx, hy in LM_HOLES:
    p, h = standoff(ix + LM_X0 + hx, iy + LM_Y0 + hy, LM_STANDOFF_H, LM_STANDOFF_D, M2_INSERT_D, M2_INSERT_DEPTH)
    base, holes = base + p, holes + h
base = base - holes

# Furo do USB da ESP32 (parede sul), para regravar sem abrir a caixa
usb_z = iz + ESP_PILLAR_H + 1.6 + 1.5
base = base - slot_y(ecx, 0, usb_z, 12.0, 8.0)

# Parede leste (110 V): entrada de força + resistência (PG11) e 3 cargas (PG9)
east_glands = [(16, PG11_D), (42, PG11_D), (66, PG9_D), (86, PG9_D), (106, PG9_D)]
for gy, gd in east_glands:
    base = base - hole_x(OX - WALL, iy + gy, iz + GLAND_Z, gd)

# Parede oeste (baixa tensão): 12 V e sensores (PG7), mais as fendas de saída de ar
for gy in (76, 100):
    base = base - hole_x(0, iy + gy, iz + GLAND_Z, PG7_D)
for i in range(7):
    base = base - box(-1, iy + 6 + i * 7.0, iz + 6, WALL + 2, 3.0, 22)

# Âncoras de abraçadeira no fundo, perto de cada furo de cabo: enquanto não houver
# prensa-cabo, uma abraçadeira (até 4 mm) prende o cabo e um puxão não chega no borne.
def tie_anchor(x, y):
    block = box(x - 3, y - 6, iz - 0.1, 6, 12, 5.1)
    return block - box(x - 2, y - 7, iz, 4.2, 14, 2.4)   # túnel por onde passa a abraçadeira

for gy, _ in east_glands:
    base = base + tie_anchor(ix + IN_X - 16, iy + gy)
for gy in (76, 100):
    base = base + tie_anchor(ix + 12, iy + gy)

# Insertos M3 nas torres dos cantos (tampa)
for cx, cy in corners:
    base = base - cyl(cx, cy, H - M3_INSERT_DEPTH, M3_INSERT_D, M3_INSERT_DEPTH + 1)

# Abas para parafusar atrás da máquina (lados sul e norte)
for y0, sgn in ((0, -1), (OY, 1)):
    tab = box(OX / 2 - 8, y0 if sgn > 0 else y0 - 9, 0, 16, 9, 3)
    base = (base + tab) - cyl(OX / 2, y0 + sgn * 5, -1, 4.5, 5)

# ---------- TAMPA (impressa com a face de fora na mesa) ----------
lid = box(0, 0, 0, OX, OY, LID_T)
for cx, cy in corners:
    lid = lid + cyl(cx, cy, 0, BOSS_D, LID_T)
lo = WALL + 0.3
lip = box(lo, lo, LID_T - 0.01, OX - 2 * lo, OY - 2 * lo, LIP_H) - \
      box(lo + LIP_T, lo + LIP_T, LID_T - 1, OX - 2 * (lo + LIP_T), OY - 2 * (lo + LIP_T), LIP_H + 2)
lid = lid + lip
for cx, cy in corners:
    lid = lid - cyl(cx, cy, -1, SCREW_D, LID_T + 2)
    lid = lid - cyl(cx, cy, -1, HEAD_D, 1.0 + 1.2)   # rebaixo da cabeça: 1,2 mm, sobram 1,8 mm sob ela

# Fan: a tampa é impressa de cabeça para baixo, então x é espelhado (x_tampa = OX - x_base)
fx, fy = OX - (ix + FAN_CX), iy + FAN_CY
grill = cyl(fx, fy, -1, FAN_OPEN_D, LID_T + 2, 96)
# Grade: barras cruzadas e um anel no meio protegem as pás
bars = Manifold()
for k in (-24, -12, 0, 12, 24):
    bars = bars + box(fx - FAN_OPEN_D / 2, fy + k - 1.2, 0, FAN_OPEN_D, 2.4, LID_T)
    bars = bars + box(fx + k - 1.2, fy - FAN_OPEN_D / 2, 0, 2.4, FAN_OPEN_D, LID_T)
ring = cyl(fx, fy, 0, 36, LID_T, 96) - cyl(fx, fy, -1, 31, LID_T + 2, 96)
lid = (lid - grill) + ((bars + ring) ^ cyl(fx, fy, 0, FAN_OPEN_D + 1.0, LID_T, 96))  # +1 mm: barras entram na tampa (sem faces coincidentes)
for sx in (-1, 1):
    for sy in (-1, 1):
        lid = lid - cyl(fx + sx * FAN_HOLE_SPACING / 2, fy + sy * FAN_HOLE_SPACING / 2, -1, SCREW_D, LID_T + 2)
# Passagem dos fios do fan: do lado da baixa tensão (os 12 V vêm de lá), perto da divisória
lid = lid - cyl(OX - (ix + DIV_X - 8), fy + FAN / 2 - 6, -1, 6.0, LID_T + 2)


def to_trimesh(m):
    mesh = m.to_mesh()
    return trimesh.Trimesh(vertices=np.array(mesh.vert_properties)[:, :3], faces=np.array(mesh.tri_verts))


def save_preview(path, parts):
    """Desenha cortes horizontais (vista de cima) para conferir o desenho sem abrir o fatiador."""
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, axes = plt.subplots(1, len(parts), figsize=(6 * len(parts), 5.5))
    from matplotlib.path import Path
    from matplotlib.patches import PathPatch
    for ax, (title, m, z) in zip(np.atleast_1d(axes), parts):
        verts, codes = [], []
        for poly in m.slice(z).to_polygons():   # contornos e furos juntos: o Path pinta pela regra par-ímpar
            verts += list(poly) + [poly[0]]
            codes += [Path.MOVETO] + [Path.LINETO] * (len(poly) - 1) + [Path.CLOSEPOLY]
        ax.add_patch(PathPatch(Path(verts, codes), facecolor="#2f6f6a", edgecolor="#123", linewidth=0.6))
        ax.autoscale_view()
        ax.set_title(title)
        ax.set_aspect("equal")
        ax.grid(alpha=0.2)
    fig.tight_layout()
    fig.savefig(path, dpi=110)
    plt.close(fig)


if __name__ == "__main__":
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "desenhos")
    os.makedirs(out, exist_ok=True)
    tb, tl = to_trimesh(base), to_trimesh(lid)
    print("base  fechada:", tb.is_watertight, "tamanho", tb.extents.round(1))
    print("tampa fechada:", tl.is_watertight, "tamanho", tl.extents.round(1))
    tb.export(os.path.join(out, "caixa_base.stl"))
    tl.export(os.path.join(out, "caixa_tampa.stl"))
    tl2 = tl.copy()
    tl2.apply_translation([0, OY + 20, 0])
    trimesh.Scene({"base": tb, "tampa": tl2}).export(os.path.join(out, "caixa_lava_loucas.3mf"))
    save_preview(os.path.join(out, "previa_base.png"), [
        ("Base: corte a 4 mm (colunas)", base, iz + 3),
        ("Base: corte a 22 mm (prensa-cabos)", base, iz + GLAND_Z),
        ("Base: corte a 30 mm (fendas da divisória)", base, iz + 28),
    ])
    save_preview(os.path.join(out, "previa_tampa.png"), [("Tampa (face de fora na mesa)", lid, LID_T / 2)])
