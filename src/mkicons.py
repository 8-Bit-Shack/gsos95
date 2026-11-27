# erzeugt eine Finder-Symboldatei ($CA) fuer Tests
import struct
def pstr(s, n):
    b = s.encode('latin1'); return bytes([len(b)]) + b + bytes(n - 1 - len(b))
def icon(w, h, col):
    bpr = (w + 1) // 2
    img = bytearray(); msk = bytearray()
    for y in range(h):
        for x in range(bpr):
            edge = y in (0, h - 1) or x in (0, bpr - 1)
            c = 0 if edge else col
            img.append(c << 4 | c); msk.append(0xFF)
    return struct.pack('<HHHH', 0x8000, len(img), h, w) + bytes(img) + bytes(msk)
def rec(boss, name, ft, aux, w, h, col):
    body = pstr(boss, 64) + pstr(name, 16) + struct.pack('<HH', ft, aux) + icon(w, h, col) + icon(8, 6, col)
    return struct.pack('<H', len(body) + 2) + body
def iconfile(recs):
    return struct.pack('<IHI', 0, 1, 0) + pstr('TEST.ICONS', 16) + b''.join(recs) + b'\0\0'
