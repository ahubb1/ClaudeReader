"""Write a small test comic (CBZ): pages of framed panels in a grid, for the panel detection checks.

Usage: python3 make_test_comic.py output.cbz
"""
import io
import sys
import zipfile

from PIL import Image, ImageDraw


def page(rows, cols, seed):
    """A white page with rows x cols black framed panels, each with a few shapes inside."""
    width, height, margin, gap = 1200, 1800, 60, 40
    im = Image.new('RGB', (width, height), 'white')
    d = ImageDraw.Draw(im)
    pw = (width - 2 * margin - (cols - 1) * gap) // cols
    ph = (height - 2 * margin - (rows - 1) * gap) // rows
    for r in range(rows):
        for c in range(cols):
            x, y = margin + c * (pw + gap), margin + r * (ph + gap)
            shade = 200 - 15 * ((r * cols + c + seed) % 6)
            d.rectangle((x, y, x + pw, y + ph), fill=(shade, shade + 20, 230), outline='black', width=6)
            d.ellipse((x + pw // 4, y + ph // 4, x + pw // 2, y + ph // 2), fill=(90, 60, 40))
    return im


def main():
    layouts = [(1, 1), (2, 2), (3, 2), (2, 3), (3, 3), (1, 1)]  # Cover and last page: whole pages
    with zipfile.ZipFile(sys.argv[1], 'w') as z:
        for i, (rows, cols) in enumerate(layouts):
            data = io.BytesIO()
            page(rows, cols, i).save(data, 'JPEG', quality=90)
            z.writestr('%03d.jpg' % (i + 1), data.getvalue())


if __name__ == '__main__':
    main()
