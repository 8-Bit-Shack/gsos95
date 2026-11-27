# Erzeugt vtfont.inc: 8x8-Schrift im PC-Zeichensatz (CP437) fuer das Terminal.
# Quelle: luma.core (MIT-Lizenz), luma/core/legacy/font.py, CP437_FONT
#   pip install luma.core
# Ausgabe: 256 Zeichen x 8 Zeilen, Bit 7 = linker Punkt.
from luma.core.legacy.font import CP437_FONT as F
out = ['; 8x8-Schrift CP437 (aus luma.core, MIT-Lizenz) - erzeugt von mkvtfont.py', 'vt_font:']
for ch in range(256):
    rows = []
    for r in range(8):
        b = 0
        for c in range(8):
            if F[ch][c] >> r & 1: b |= 0x80 >> c
        rows.append(b)
    out.append('        .byte ' + ','.join('$%02X' % b for b in rows) + '   ; %02X' % ch)
open('vtfont.inc', 'w').write('\n'.join(out) + '\n')
print('vtfont.inc: 2048 Bytes')

# ---- 4x8-Schrift fuer den Farbmodus (320er-Modus, 80 Spalten) ----
# Buchstaben/Ziffern/Umlaute: eigene Schrift font4x8.py (3x7 in 4 Spalten);
# Grafik ab $80: CP437-Zeichen auf 4 Spalten verkleinert (je 2 Spalten ODER),
# Schattierungen und deutsche Umlaute von Hand. Gepackt: je Byte 2 Zeilen
# (obere 4 Bit = gerade Zeile), Bit 3 bzw. 7 = linker Punkt.
from luma.core.legacy.font import TINY_FONT as T
def tiny(ch):
    return [sum(0x8 >> c for c in range(min(4, len(T[ch]))) if T[ch][c] >> r & 1) for r in range(8)]
def shrink(ch):
    out = []
    for r in range(8):
        b = 0
        for c in range(4):
            if (F[ch][2 * c] >> r & 1) or (F[ch][2 * c + 1] >> r & 1): b |= 0x8 >> c
        out.append(b)
    return out
def pic(*rows): return [int(x.replace('.', '0').replace('#', '1'), 2) for x in rows]
import font4x8
G4 = {}
for ch in range(256):
    G4[ch] = font4x8.rows(font4x8.G[chr(ch)]) if 32 <= ch < 127 else shrink(ch)
for ch, g in font4x8.X.items(): G4[ch] = font4x8.rows(g)
G4[0xB0] = pic('#...', '..#.', '#...', '..#.', '#...', '..#.', '#...', '..#.')
G4[0xB1] = pic('#.#.', '.#.#', '#.#.', '.#.#', '#.#.', '.#.#', '#.#.', '.#.#')
G4[0xB2] = pic('.###', '##.#', '.###', '##.#', '.###', '##.#', '.###', '##.#')
out = ['; 4x8-Schrift (gepackt, 2 Zeilen je Byte) - erzeugt von mkvtfont.py', 'vt_font4:']
for ch in range(256):
    g = G4[ch]
    out.append('        .byte ' + ','.join('$%02X' % (g[2 * i] << 4 | g[2 * i + 1]) for i in range(4)) + '   ; %02X' % ch)
open('vtfont4.inc', 'w').write('\n'.join(out) + '\n')
print('vtfont4.inc: 1024 Bytes')
