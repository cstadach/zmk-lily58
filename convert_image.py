#!/usr/bin/env python3
"""
Convert a PNG image to an LVGL 1-bit C array for the ZMK nice!view display.

Usage:
    python3 convert_image.py avatar.png

Expects a 64x64 image (the GitHub avatar — 4x upscale of a 16x16 sprite).
Outputs the C array for link_sprite_map[] to stdout.
Paste the output into config/custom_status_screen.c.

If Link appears inverted on the display, re-run with --invert.
"""

import sys
from PIL import Image


def convert(path: str, invert: bool = False) -> None:
    img = Image.open(path).convert("RGBA")

    # Flatten alpha onto white background
    bg = Image.new("RGB", img.size, (255, 255, 255))
    bg.paste(img, mask=img.split()[3])
    img = bg

    # The avatar is 64x64 = 16x16 original sprite scaled 4x.
    # We want 48x48 on the display (3x original), so:
    #   64x64 → 16x16 (recover originals) → 48x48 (3x for display)
    img = img.resize((16, 16), Image.NEAREST).resize((48, 48), Image.NEAREST)

    gray = img.convert("L")
    width, height = gray.size  # 48, 48
    pixels = list(gray.getdata())

    THRESHOLD = 128  # pixels darker than this → 1 (drawn as black)
    row_bytes = (width + 7) // 8  # 6 bytes per row for 48px

    data = []
    for y in range(height):
        for bx in range(row_bytes):
            byte = 0
            for bit in range(8):
                x = bx * 8 + bit
                if x < width:
                    dark = pixels[y * width + x] < THRESHOLD
                    if dark != invert:
                        byte |= 1 << (7 - bit)
            data.append(byte)

    inv_note = "  (inverted)" if invert else ""
    print(f"// {width}x{height} px · {len(data)} bytes{inv_note}")
    print("// If Link appears inverted: python3 convert_image.py avatar.png --invert")
    print("static const uint8_t link_sprite_map[] = {")
    for y in range(height):
        row = data[y * row_bytes : (y + 1) * row_bytes]
        print("    " + ", ".join(f"0x{b:02X}" for b in row) + f",  /* row {y:>2} */")
    print("};")


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("-")]
    flags = [a for a in sys.argv[1:] if a.startswith("-")]
    if not args:
        print(__doc__)
        sys.exit(1)
    convert(args[0], invert="--invert" in flags)
