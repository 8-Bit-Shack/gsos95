# Gemeinsamer Farbverlauf fuer Bootbild (boot95/gen.py) und Startbild (assets.py)
# 16 Blautoene, im Lab-Farbraum moeglichst gleichmaessig gestuft (alle Kanaele
# steigen monoton, kein Farbstich). Palette p (1..15): Farbe 12 = Ton p-1,
# Farbe 13 = Ton p. Zwischen beiden wird je Zeile im 2x2-Raster gemischt
# (0/25/50/75 %), so entstehen 60 Stufen statt 15 sichtbarer Baender.
SHADES = [0x003, 0x014, 0x025, 0x136, 0x247, 0x258, 0x269, 0x26A,
          0x37A, 0x37B, 0x48B, 0x48C, 0x49D, 0x5AD, 0x5AE, 0x5BF]
STEP = 77                      # Stufe(y) = y*77/256 -> 0..59
# Fuellbyte je (Mischstufe, Zeile gerade/ungerade); $C = Farbe 12, $D = Farbe 13
FILLB = [0xCC, 0xCC, 0xCD, 0xCC, 0xCD, 0xDC, 0xDD, 0xDC]
BUILD = open(__import__('os').path.join(__import__('os').path.dirname(__file__), 'BUILD')).read().strip()
def level(y): return (y * STEP) >> 8
def scb(y): return 1 + level(y) // 4
def fill(y): return FILLB[(level(y) & 3) * 2 + (y & 1)]
