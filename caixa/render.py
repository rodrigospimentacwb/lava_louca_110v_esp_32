"""Imagens 3D da caixa para o README (rasterizador simples com z-buffer, sem placa de vídeo).

Rode depois do caixa.py:
    .venv/bin/python -B render.py
Gera em desenhos/: render_base.png, render_tampa.png e render_montada.png.
"""
import os
import numpy as np
from PIL import Image
from caixa import base, lid, to_trimesh, OX, H, LID_T

W, HGT, SS = 900, 640, 2           # tamanho final e supersampling (antisserrilhado)
BG = np.array([246, 248, 250]) / 255
COLOR_BASE = np.array([47, 111, 106]) / 255
COLOR_LID = np.array([88, 150, 170]) / 255


def lid_in_place(lift):
    """A tampa é desenhada de cabeça para baixo e espelhada em x: gira 180° no eixo y e põe em cima."""
    t = to_trimesh(lid)
    t.vertices = np.c_[OX - t.vertices[:, 0], t.vertices[:, 1], H + LID_T - t.vertices[:, 2] + lift]
    return t


def view_matrix(azim, elev):
    a, e = np.radians(azim), np.radians(elev)
    rz = np.array([[np.cos(a), -np.sin(a), 0], [np.sin(a), np.cos(a), 0], [0, 0, 1]])
    rx = np.array([[1, 0, 0], [0, np.cos(e), -np.sin(e)], [0, np.sin(e), np.cos(e)]])
    return rx @ rz


def render(parts, azim, elev, path):
    w, h = W * SS, HGT * SS
    verts_all = np.vstack([t.vertices for t, _ in parts])
    center = (verts_all.min(0) + verts_all.max(0)) / 2
    R = view_matrix(azim, elev)
    # câmera ortográfica: x, y na tela; z = profundidade (maior = mais perto)
    proj = [((t.vertices - center) @ R.T, t, c) for t, c in parts]
    allp = np.vstack([p for p, _, _ in proj])
    span = max(np.ptp(allp[:, 0]) / w, np.ptp(allp[:, 2]) / h) * 1.12
    img = np.ones((h, w, 3)) * BG
    zbuf = np.full((h, w), -np.inf)
    light = np.array([-0.4, -0.6, 0.7])
    light /= np.linalg.norm(light)
    for p, t, color in proj:
        sx = p[:, 0] / span + w / 2
        sy = h / 2 - p[:, 2] / span
        depth = -p[:, 1]
        shade = 0.38 + 0.62 * np.clip(t.face_normals @ light, 0, 1)
        for f, s in zip(t.faces, shade):
            xs, ys, zs = sx[f], sy[f], depth[f]
            x0, x1 = int(max(xs.min(), 0)), int(min(np.ceil(xs.max()), w - 1))
            y0, y1 = int(max(ys.min(), 0)), int(min(np.ceil(ys.max()), h - 1))
            if x1 < x0 or y1 < y0:
                continue
            den = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
            if abs(den) < 1e-12:
                continue
            gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
            l0 = ((ys[1] - ys[2]) * (gx - xs[2]) + (xs[2] - xs[1]) * (gy - ys[2])) / den
            l1 = ((ys[2] - ys[0]) * (gx - xs[2]) + (xs[0] - xs[2]) * (gy - ys[2])) / den
            l2 = 1 - l0 - l1
            inside = (l0 >= 0) & (l1 >= 0) & (l2 >= 0)
            if not inside.any():
                continue
            z = l0 * zs[0] + l1 * zs[1] + l2 * zs[2]
            zb = zbuf[y0:y1 + 1, x0:x1 + 1]
            upd = inside & (z > zb)
            zb[upd] = z[upd]
            img[y0:y1 + 1, x0:x1 + 1][upd] = color * s
    # contorno onde a profundidade salta (deixa as bordas legíveis)
    zf = np.where(np.isfinite(zbuf), zbuf, zbuf[np.isfinite(zbuf)].min() - 50)
    edge = np.zeros_like(zf, bool)
    edge[1:, :] |= np.abs(np.diff(zf, axis=0)) > 1.5
    edge[:, 1:] |= np.abs(np.diff(zf, axis=1)) > 1.5
    img[edge] *= 0.35
    out = Image.fromarray((np.clip(img, 0, 1) * 255).astype(np.uint8)).resize((W, HGT), Image.LANCZOS)
    out.save(path, optimize=True)


if __name__ == "__main__":
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "desenhos")
    tb = to_trimesh(base)
    render([(tb, COLOR_BASE)], -35, 35, os.path.join(out, "render_base.png"))
    render([(lid_in_place(0), COLOR_LID)], -35, 35, os.path.join(out, "render_tampa.png"))
    render([(tb, COLOR_BASE), (lid_in_place(45), COLOR_LID)], -35, 22, os.path.join(out, "render_montada.png"))
    print("ok")
