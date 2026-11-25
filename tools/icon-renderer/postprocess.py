"""Post-process AH_IconRenderer screenshots -> web/static/assets/items/<KEY>.webp

Each job yields <KEY>__b.bmp: the engine's inventory item preview (centred rectangle sized per
category). We crop that rectangle, trim borders, re-centre on a #2E3031 canvas and save webp.
Usage: python postprocess.py <shots_dir> <out_dir> <jobs.txt>
"""
import sys, os
from PIL import Image

BG = (0x2E, 0x30, 0x31)
# preview rectangle (reference px, as in AH_IconStudio.Layout) and output size per category
RECT = {"weapon": (1200, 400), "item": (520, 520), "char": (380, 700), "veh": (1100, 680)}
SIZES = {"weapon": (512, 256), "item": (512, 384), "char": (512, 384), "veh": (512, 384)}


def crop_rect(img, cat):
    w, h = RECT.get(cat, RECT["item"])
    s = img.height / 1080.0  # workspace is laid out at 1080p reference units
    w, h = w * s, h * s
    cx, cy = img.width / 2, img.height / 2
    return img.crop((int(cx - w / 2) + 2, int(cy - h / 2) + 2, int(cx + w / 2) - 2, int(cy + h / 2) - 2))


def fit(crop, size):
    W, H = size
    s = min(W / crop.width, H / crop.height)
    crop = crop.resize((max(1, round(crop.width * s)), max(1, round(crop.height * s))), Image.LANCZOS)
    out = Image.new("RGB", size, BG)
    out.paste(crop, ((W - crop.width) // 2, (H - crop.height) // 2))
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
            f = os.path.join(shots, k + "__b.bmp")
        if not os.path.exists(f):
            bad.append(k); continue
        img = crop_rect(Image.open(f).convert("RGB"), cat)
        fit(img, SIZES.get(cat, SIZES["item"])).save(os.path.join(outdir, k + ".webp"), "WEBP", quality=86, method=6)
        ok.append(k)
    print("ok", len(ok), "failed", bad)


if __name__ == "__main__":
    main()
