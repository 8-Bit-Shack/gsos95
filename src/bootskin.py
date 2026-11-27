# Erzeugt GS/OS95-Bootgrafik fuer START.GS.OS (System 6.0.4) - ersetzt nur Daten
import sys, struct
sys.path.insert(0, __import__('os').path.dirname(__import__('os').path.abspath(__file__)))
from font import render
BASE = 0x6800
def off(a): return a - BASE

ORIG = {  # Pruefbytes der Originaldatei
    0x7354: bytes([0xa9, 0xdd, 0xdd, 0x8f, 0x00, 0x20, 0xe1]),
    0x75F4: bytes([0xa9, 0xc5, 0x43]),
    0x760E: bytes([0xa9, 0xec, 0x46, 0x85, 0x00, 0xa9, 0x88, 0x77]),
    0x762D: bytes([0xa9, 0x89, 0x49, 0x85, 0x00, 0xa9, 0x48, 0x79]),
    0x764B: bytes([0xa9, 0x72, 0x50, 0x85, 0x00, 0xa9, 0x95, 0x7a]),
    0x7778: bytes.fromhex('0000000ff000ff0f00000f00f00fff0f'),
}

def pack640(pix_rows, nbytes):
    """Zeilen mit Werten 0..3 -> 640er-Bytes (4 Pixel/Byte, linkes Pixel = Bits 7-6)"""
    out = bytearray()
    for r in pix_rows:
        r = (r + [3] * (nbytes * 4))[:nbytes * 4]
        for i in range(0, nbytes * 4, 4):
            out.append((r[i] << 6) | (r[i + 1] << 4) | (r[i + 2] << 2) | r[i + 3])
    return bytes(out)

def text_rows(text, bold, width_px, height, color=0, hscale=2):
    rows = render(text, bold, 1)
    w = len(rows[0]) * hscale
    assert w <= width_px, (text, w, width_px)
    left = (width_px - w) // 2
    out = []
    top = (height - len(rows)) // 2
    for y in range(height):
        line = [3] * width_px
        ry = y - top
        if 0 <= ry < len(rows):
            for x, v in enumerate(rows[ry]):
                if v:
                    for k in range(hscale):
                        line[left + x * hscale + k] = color
        out.append(line)
    return out

ICON = """
................
.kkkkkkkkkkkkk..
.kwwwwwwwwwwwdk.
.kwkkkkkkkkkgdk.
.kwkcccccccbgdk.
.kwkcwccccbbgdk.
.kwkccccccbbgdk.
.kwkcccccbbbgdk.
.kwkbbbbbbbbgdk.
.kwgggggggGggdk.
.kkkkkkkkkkkkkk.
....kdddddk.....
..kkkkkkkkkkk...
.kwwwwwwwwwwgdk.
.kggggggggrggdk.
.kkkkkkkkkkkkkk.
"""
def icon_rows():
    m = {'.': 3, 'k': 0, 'w': 3, 'g': 3, 'd': 0, 'c': 2, 'b': 1, 'G': 2, 'r': 1}
    src = [r for r in ICON.strip('\n').split('\n')]
    rows = []
    for r in src:
        line = [3] * 4
        for c in r:
            line += [m[c]] * 3
        line += [3] * 4
        rows.append(line); rows.append(line)
    # helle Flaechen (w) als Raster, damit der Monitor sich vom Kasten abhebt
    return rows

def build(data):
    d = bytearray(data)
    for a, b in ORIG.items():
        assert d[off(a):off(a) + len(b)] == b, 'Unerwartete START.GS.OS-Version bei %X' % a
    # Hintergrund: Wert 2 ueberall (Petrol)
    d[off(0x7355):off(0x7355) + 2] = b'\xaa\xaa'
    # Palette (8 Farben, gilt fuer gerade und ungerade Spalten)
    pal = [0x000, 0x00A, 0x088, 0xCCC, 0x000, 0x00A, 0x088, 0xCCC]
    d[off(0x7778):off(0x7778) + 16] = b''.join(struct.pack('<H', c) for c in pal)
    # Symbol 56x32
    ic = pack640(icon_rows(), 14); assert len(ic) == 14 * 32
    d[off(0x7788):off(0x7788) + len(ic)] = ic
    # Text 1: 148 x 9
    t1 = pack640(text_rows('GS/OS95', True, 148, 9, color=1), 37); assert len(t1) == 37 * 9
    d[off(0x7948):off(0x7948) + len(t1)] = t1
    # Text 2: 88 x 10
    t2 = pack640(text_rows('Apple IIgs', False, 88, 10, color=0), 22); assert len(t2) == 22 * 10
    d[off(0x7A95):off(0x7A95) + len(t2)] = t2
    # Textmodus-Begruessung (gleiche Laenge)
    old = bytes(c | 0x80 for c in b'Welcome to the world of GS/OS      v4.02')
    k = d.find(old); assert k > 0
    new = 'Welcome to GS/OS95  (GS/OS v4.02)       '.encode()
    assert len(new) == len(old)
    d[k:k + len(new)] = bytes(c | 0x80 for c in new)
    return bytes(d)

def preview(data, path):
    """Simuliert die Zeichenroutinen fuer eine Vorschau (640x200)"""
    from PIL import Image
    d = data
    scr = bytearray(32000)
    fill = d[off(0x7355)]
    for i in range(32000): scr[i] = fill
    def put(addr, b):
        scr[addr - 0x2000] = b
    def line_box(o, kind):
        for yb in range(0x56):
            if kind == 'top': v = 0xFF
            elif kind == 'mid': v = 0x0F if yb == 0x56 - 1 else (0xF0 if yb == 0 else 0x00)
            else: v = 0xFF
            put(o + yb, v)
    # Kasten nachbilden (vereinfachte Logik: weisser Rand, Inneres Wert 3)
    o = 0x43C5
    for row in range(49):
        for yb in range(0x56):
            edge = row in (0, 48) or yb in (0, 0x55)
            inner_frame = row in (1, 47) or yb in (1, 0x54)
            put(o + yb, 0x00 if (row in (1, 47)) else 0xFF)
        if 2 <= row <= 46:
            put(o, 0xF0 & 0xFF | 0x0F if False else 0xF0 | 0x0F)  # Rand
            put(o, 0b11110000 | 0b1111); put(o, 0xF0 | 0x0F)
            put(o, (0xF0)); put(o + 0x55, 0x0F)
        o += 0xA0
    def blit(dst, src, nb, rows):
        for r in range(rows):
            for k in range(nb):
                put(dst + r * 0xA0 + k, d[off(src) + r * nb + k])
    blit(0x46EC, 0x7788, 14, 32); blit(0x4989, 0x7948, 37, 9); blit(0x5072, 0x7A95, 22, 10)
    pal = [struct.unpack_from('<H', d, off(0x7778) + 2 * i)[0] for i in range(8)] * 2
    img = Image.new('RGB', (640, 200)); px = img.load()
    grp = [8, 12, 0, 4]
    for y in range(200):
        for xb in range(160):
            b = scr[y * 160 + xb]
            for k in range(4):
                v = (b >> (6 - 2 * k)) & 3
                c = pal[grp[k] + v]
                px[xb * 4 + k, y] = (((c >> 8) & 15) * 17, ((c >> 4) & 15) * 17, (c & 15) * 17)
    img.resize((1280, 480 * 1)).save(path)

if __name__ == '__main__':
    src = open(sys.argv[1], 'rb').read()
    out = build(src)
    open(sys.argv[2], 'wb').write(out)
    if len(sys.argv) > 3: preview(out, sys.argv[3])
    print('ok')
