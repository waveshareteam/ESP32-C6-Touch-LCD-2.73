"""Regenerate three 112px launcher PNGs and LVGL ARGB8888 assets with Pillow.

Pass --preview PATH to also write a review contact sheet.
"""
import argparse
from pathlib import Path
from PIL import Image, ImageColor, ImageDraw, ImageFilter, ImageFont

COMPONENTS = Path(__file__).resolve().parents[2]
SIZE = 112
S = 4
ASSETS = (
    ("watch", "Watch/assets", "watch-icon.png", "img_app_watch"),
    ("badge", "Badge/assets", "badge-icon.png", "img_app_badge"),
    ("pins", "app_pinout/assets", "pins-icon.png", "PinsTest"),
)


def box(values):
    return tuple(round(v * S) for v in values)


def rounded(im, bounds, radius, fill, outline=None, width=1):
    ImageDraw.Draw(im).rounded_rectangle(
        box(bounds), round(radius * S), fill, outline, round(width * S)
    )


def polygon(im, points, fill):
    ImageDraw.Draw(im).polygon([box(p) for p in points], fill=fill)


def line(im, points, fill, width=1):
    d = ImageDraw.Draw(im)
    points = [box(p) for p in points]
    w = round(width * S)
    d.line(points, fill=fill, width=w, joint="curve")
    for x, y in (points[0], points[-1]):
        d.ellipse((x-w/2, y-w/2, x+w/2, y+w/2), fill=fill)


def ellipse(im, bounds, fill):
    ImageDraw.Draw(im).ellipse(box(bounds), fill=fill)


def gradient(bounds, top, bottom):
    """Opaque vertical gradient; bounds use the 112px design grid."""
    x0, y0, x1, y1 = box(bounds)
    im = Image.new("RGBA", (x1-x0, y1-y0))
    d = ImageDraw.Draw(im)
    a, b = ImageColor.getrgb(top), ImageColor.getrgb(bottom)
    for y in range(im.height):
        t = y / max(1, im.height-1)
        color = tuple(round(v + (w-v)*t) for v, w in zip(a, b)) + (255,)
        d.line((0, y, im.width, y), fill=color)
    return im


def gradient_panel(im, bounds, radius, top, bottom):
    panel = gradient(bounds, top, bottom)
    mask = Image.new("L", panel.size)
    ImageDraw.Draw(mask).rounded_rectangle(
        (0, 0, panel.width-1, panel.height-1), round(radius*S), fill=255
    )
    im.paste(panel, box(bounds[:2]), mask)


def shadow(im, bounds, radius):
    layer = Image.new("RGBA", im.size)
    rounded(layer, bounds, radius, (3, 9, 25, 105))
    im.alpha_composite(layer.filter(ImageFilter.GaussianBlur(2*S)))


def device(im):
    """Reference LCD: square black bezel, square glass and bottom USB."""
    shadow(im, (14, 16, 99, 101), 11)
    rounded(im, (50, 96, 62, 100), 1.5, "#7f8993")
    rounded(im, (52, 98, 60, 99.5), .7, "#293440")
    gradient_panel(im, (13, 12, 99, 98), 10, "#40464e", "#12171e")
    rounded(im, (14, 13, 98, 97), 9, "#080b10")
    line(im, [(23, 13.5), (89, 13.5)], "#4c525a", .65)
    rounded(im, (22.5, 21.5, 89.5, 88.5), 2.5, "#242d36")
    gradient_panel(im, (24, 23, 88, 87), 1.5, "#101b23", "#050a0f")


def draw_time(im):
    """A borderless clock face, simplified for launcher-size readability."""
    rounded(im, (0, 0, SIZE, SIZE), 25, "#f6f5ef")
    ink, accent = "#343d3d", "#56786a"
    masks = (0x06, 0x3f, 0x66, 0x6d)  # 10:45
    shapes = ((3, 0, 13, 3), (15, 3, 3, 16), (15, 24, 3, 16),
              (3, 40, 13, 3), (0, 24, 3, 16), (0, 3, 3, 16),
              (3, 20, 13, 3))
    for origin, mask in zip((9, 31, 63, 85), masks):
        for bit, (dx, dy, w, h) in enumerate(shapes):
            if not mask & (1 << bit):
                continue
            x, y = origin + dx, 36 + dy
            b = min(w, h) / 2
            if w > h:
                points = ((x+b,y), (x+w-b,y), (x+w,y+b),
                          (x+w-b,y+h), (x+b,y+h), (x,y+b))
            else:
                points = ((x+b,y), (x+w,y+b), (x+w,y+h-b),
                          (x+b,y+h), (x,y+h-b), (x,y+b))
            polygon(im, points, ink)
    for y in (48, 64):
        ellipse(im, (54, y, 58, y+4), ink)
    draw = ImageDraw.Draw(im)
    font_path = "C:/Windows/Fonts/segoeui.ttf"
    for text, y, size in (("CLOCK", 20, 10), ("41", 94, 11)):
        font = ImageFont.truetype(font_path, size*S)
        draw.text(box((56, y)), text, font=font, anchor="mm", fill=accent)


def draw_landscape(im):
    """An edge-to-edge landscape without a device case or screen bezel."""
    picture = Image.new("RGBA", im.size)
    gradient_panel(picture, (24, 23, 88, 87), 0, "#708fba", "#f8c5ae")
    ellipse(picture, (65, 30, 76, 41), "#ffe5b5")
    polygon(picture, [(24, 57), (36, 43), (47, 54), (61, 39),
                      (77, 56), (88, 46), (88, 69), (24, 69)], "#8b9db7")
    polygon(picture, [(24, 63), (39, 38), (57, 64)], "#436480")
    polygon(picture, [(39, 38), (34, 47), (39, 45), (42, 49), (45, 48)], "#f0f3e9")
    polygon(picture, [(39, 38), (43, 55), (57, 64)], "#63839a")
    polygon(picture, [(55, 64), (73, 44), (88, 60), (88, 70)], "#365a76")
    polygon(picture, [(73, 44), (65, 53), (72, 51), (75, 54), (79, 51)], "#e1e8e3")
    polygon(picture, [(73, 44), (79, 62), (88, 60)], "#54788f")
    gradient_panel(picture, (24, 63, 88, 87), 0, "#82b8c0", "#386e8b")
    polygon(picture, [(24, 63), (43, 65), (52, 68), (34, 72), (24, 77)], "#254e61")
    polygon(picture, [(88, 61), (70, 65), (63, 69), (79, 73), (88, 78)], "#25465b")
    for y, x0, x1 in ((69, 53, 61), (73, 48, 63), (78, 53, 72), (83, 39, 58)):
        line(picture, [(x0, y), (x1, y)], "#b0d2d0", .65)
    polygon(picture, [(24, 78), (33, 81), (44, 87), (24, 87)], "#183a49")
    polygon(picture, [(88, 77), (78, 81), (71, 87), (88, 87)], "#173549")
    for x, y, height in ((28, 73, 11), (32, 77, 8), (83, 75, 10)):
        line(picture, [(x, y), (x, y+height)], "#163b46", .9)
        polygon(picture, [(x, y), (x-3, y+height*.7), (x+3, y+height*.7)], "#163b46")
    face = picture.crop(box((24, 23, 88, 87)))
    im.alpha_composite(face.resize(im.size, Image.Resampling.LANCZOS))


def draw_pins(im):
    shadow(im, (23, 25, 89, 92), 10)
    # Four generous gold contacts per side, without tiny pin labels.
    for y in (34, 48, 62, 76):
        for x in (18, 80):
            gradient_panel(im, (x, y, x+14, y+6), 1.5, "#ffe5ae", "#b9895a")
            line(im, [(x+2, y+1), (x+11, y+1)], "#fff1c6", .7)
    gradient_panel(im, (27, 21, 85, 89), 9, "#b6c8ff", "#616ec2")
    gradient_panel(im, (28.5, 22.5, 83.5, 87.5), 7.5, "#576cae", "#293561")
    for y, chip_y in ((37, 46), (51, 52), (65, 59), (79, 65)):
        line(im, [(29, y), (35, y), (39, chip_y), (43, chip_y)], "#99b7ef", 1.5)
        line(im, [(83, y), (77, y), (73, chip_y), (69, chip_y)], "#99b7ef", 1.5)
    for x in (35, 77):
        for y in (29, 81):
            ellipse(im, (x-2.1, y-2.1, x+2.1, y+2.1), "#bdcff4")
            ellipse(im, (x-.9, y-.9, x+.9, y+.9), "#28395d")
    rounded(im, (42, 39, 70, 71), 4, "#a4b9ea")
    gradient_panel(im, (43, 40, 69, 70), 3, "#18263f", "#101a31")
    line(im, [(48, 59), (53, 59), (53, 50), (59, 50), (59, 59), (64, 59)],
         "#b7f4ff", 2)
    rounded(im, (49, 28, 63, 33), 2, "#24385e")
    ellipse(im, (51, 29, 54, 32), "#9aede9")
    line(im, [(57, 30.5), (61, 30.5)], "#889eca", 1)


def icon(kind):
    im = Image.new("RGBA", (SIZE*S, SIZE*S))
    colors = {
        "watch": ("#678bb5", "#293f68"),
        "badge": ("#80b3ab", "#335f69"),
        "pins": ("#9c8cdb", "#424e98"),
    }
    gradient_panel(im, (0, 0, SIZE, SIZE), 25, *colors[kind])
    if kind == "watch":
        draw_time(im)
    elif kind == "pins":
        draw_pins(im)
    else:
        draw_landscape(im)
    # Keep shadows inside the common rounded silhouette.
    mask = Image.new("L", im.size)
    rounded(mask, (0, 0, SIZE-.25, SIZE-.25), 25, 255)
    im.putalpha(mask)
    return im.resize((SIZE, SIZE), Image.Resampling.LANCZOS)


def write_asset(im, path, symbol):
    # On this little-endian target LVGL ARGB8888 bytes are B, G, R, A.
    pixels = im.tobytes("raw", "BGRA")
    source = ('/* SPDX-License-Identifier: Apache-2.0 */\n'
              '/* Generated by components/Watch/assets/generate_icons.py. */\n'
              '#include "lvgl.h"\n'
              'static const LV_ATTRIBUTE_MEM_ALIGN uint8_t pixels[] = {\n')
    source += ''.join('    '+','.join(f'0x{x:02x}' for x in pixels[i:i+32])+',\n'
                      for i in range(0, len(pixels), 32))
    source += ('};\n'
               f'const lv_image_dsc_t {symbol} = {{\n'
               '    .header = {.magic=LV_IMAGE_HEADER_MAGIC, '
               '.cf=LV_COLOR_FORMAT_ARGB8888, .w=112, .h=112, .stride=448},\n'
               '    .data_size=sizeof(pixels), .data=pixels,\n'
               '};\n')
    path.write_text(source, encoding="utf-8")


def contact_sheet(icons, path):
    sheet = Image.new("RGB", (840, 440), "#172232")
    d = ImageDraw.Draw(sheet)
    font = ImageFont.load_default(size=18)
    for index, (kind, im) in enumerate(icons):
        x = index*280+28
        large = im.resize((224, 224), Image.Resampling.LANCZOS)
        sheet.paste(large, (x, 24), large)
        label = {"watch": "Watch / 10:45", "badge": "Badge / picture", "pins": "PinsTest / GPIO"}[kind]
        d.text((x, 263), label, font=font, fill="#e2eaf4")
        # Native size on a light field reveals transparent corners.
        d.rounded_rectangle((x, 299, x+224, 427), 12, fill="#e4eaf0")
        sheet.paste(im, (x+56, 307), im)
    path.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preview", type=Path)
    args = parser.parse_args()
    icons = []
    for kind, directory, png, symbol in ASSETS:
        out = COMPONENTS / directory
        im = icon(kind)
        im.save(out / png)
        write_asset(im, out / (symbol+".c"), symbol)
        icons.append((kind, im))
    if args.preview:
        contact_sheet(icons, args.preview)


if __name__ == "__main__":
    main()
