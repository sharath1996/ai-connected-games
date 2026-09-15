"""Shared helpers for building the OLED's 128x64 raw pixel-string payload."""
WIDTH, HEIGHT = 128, 64
# This SH1106 panel renders content shifted 8px left of where it's drawn;
# shifting the source pixels right by this amount compensates for it.
X_SHIFT = 8


def shift_pixel_string(pixels: str, dx: int = X_SHIFT, width: int = WIDTH, height: int = HEIGHT) -> str:
    """Shift a row-major '0'/'1' pixel string horizontally by dx columns (blank fill)."""
    rows = (pixels[y * width:(y + 1) * width] for y in range(height))
    if dx >= 0:
        shifted = (('0' * dx + row)[:width] for row in rows)
    else:
        shifted = ((row + '0' * (-dx))[-width:] for row in rows)
    return ''.join(shifted)


def image_to_pixel_string(img, threshold: int = 128, dx: int = X_SHIFT) -> str:
    """Convert a PIL grayscale image to the device's pixel-string format, applying the panel offset."""
    pixels = []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            pixels.append('1' if img.getpixel((x, y)) < threshold else '0')
    return shift_pixel_string(''.join(pixels), dx)
