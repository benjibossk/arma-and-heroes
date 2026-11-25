"""Post-process AH_IconRenderer screenshots -> web/static/assets/items/<KEY>.webp (with alpha)

Each job yields <KEY>.bmp: the engine's inventory item preview (centred rectangle sized per
category) rendered over a flat chroma-green sky (Prefabs/AH/AH_ChromaPreviewWorld.et).
We crop that rectangle, key out the green (soft alpha from green excess, colour un-mixing and
green despill on the edges), trim to content with padding and save a transparent webp.
Usage: python postprocess.py <shots_dir> <out_dir> <jobs.txt>
"""
import sys, os
import numpy as np
from PIL import Image, ImageFilter

# preview rectangle (reference px, as in AH_IconStudio.Layout) and output size per category
RECT = {"weapon": (1200, 400), "item": (520, 520), "char": (380, 700), "veh": (1100, 680)}
SIZES = {"weapon": (512, 256), "item": (512, 384), "char": (512, 384), "veh": (512, 384)}
PAD = 0.05  # padding around trimmed content, fraction of output size
LO, HI = 0.22, 0.70  # green excess (relative to the key's) -> fully opaque below LO, fully clear above HI


def crop_rect(img, cat):
    w, h = RECT.get(cat, RECT["item"])
    s = img.height / 1080.0  # workspace is laid out at 1080p reference units
    w, h = w * s, h * s
    cx, cy = img.width / 2, img.height / 2
    return img.crop((int(cx - w / 2) + 2, int(cy - h / 2) + 2, int(cx + w / 2) - 2, int(cy + h / 2) - 2))


def key(img):
    """RGB chroma-green crop -> RGBA with soft alpha, unmixed colour and despilled edges."""
    a = np.asarray(img, dtype=np.float32)
    edge = np.concatenate([a[:4].reshape(-1, 3), a[-4:].reshape(-1, 3), a[:, :4].reshape(-1, 3), a[:, -4:].reshape(-1, 3)])
    k = np.median(edge, axis=0)  # key colour as rendered (~ 0,158,0)
    kex = max(k[1] - max(k[0], k[2]), 1.0)
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    ex = (g - np.maximum(r, b)) / kex
    alpha = 1.0 - np.clip((ex - LO) / (HI - LO), 0.0, 1.0)
    # un-mix the key from semi-transparent pixels: C = a*F + (1-a)*K
    am = np.maximum(alpha, 1e-3)[..., None]
    fg = np.clip((a - (1.0 - am) * k) / am, 0, 255)
    # despill: on the edge band (partial alpha + 2px into the opaque side) clamp G into
    # [(R+B)/2, max(R,B)] - removes green fringes without turning glass/over-unmixed pixels magenta
    band = Image.fromarray(((alpha < 0.98) * 255).astype(np.uint8)).filter(ImageFilter.MaxFilter(5))
    band = np.asarray(band) > 0
    gmax = np.maximum(fg[..., 0], fg[..., 2])
    fg[..., 1] = np.where(band, np.clip(fg[..., 1], (fg[..., 0] + fg[..., 2]) / 2, gmax), fg[..., 1])
    out = np.dstack([fg, alpha * 255]).round().clip(0, 255).astype(np.uint8)
    out[alpha <= 0.0] = 0
    return Image.fromarray(out, "RGBA")


def fit(rgba, size):
    W, H = size
    bbox = rgba.getchannel("A").point(lambda v: 255 if v > 10 else 0).getbbox()
    if not bbox:
        return None
    c = rgba.crop(bbox)
    iw, ih = W * (1 - 2 * PAD), H * (1 - 2 * PAD)
    s = min(iw / c.width, ih / c.height)
    c = c.resize((max(1, round(c.width * s)), max(1, round(c.height * s))), Image.LANCZOS)
    out = Image.new("RGBA", size, (0, 0, 0, 0))
    out.paste(c, ((W - c.width) // 2, (H - c.height) // 2))
    return out


def main():
    shots, outdir, jobs = sys.argv[1], sys.argv[2], sys.argv[3]
    ok, bad = [], []
    for line in open(jobs, encoding="utf-8"):
        p = line.strip().split("|")
        if len(p) < 5:
            continue
        k, cat = p[0], p[4]
        f = os.path.join(shots, k + ".bmp")
        if not os.path.exists(f):
            bad.append(k); continue
        out = fit(key(crop_rect(Image.open(f).convert("RGB"), cat)), SIZES.get(cat, SIZES["item"]))
        if out is None:
            bad.append(k); continue
        out.save(os.path.join(outdir, k + ".webp"), "WEBP", quality=88, method=6)
        ok.append(k)
    print("ok", len(ok), "failed", bad)


if __name__ == "__main__":
    main()
