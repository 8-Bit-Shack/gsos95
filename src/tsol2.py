# Solitaer: As auf leeren Zielstapel (Doppelklick und Ziehen), Ziehen von Stapeln ohne Reste
from twbase import *
import twbase
from emu import run
X3 = 0x200000
def x3(n, o=0): return cpu.rd16(X3 + lbl[n] + o)
def pile(p):
    n = x3('SPN', 2 * p); a = X3 + lbl['SPC'] + 52 * p
    return list(cpu.mem[a:a + n])
cpu = boot(); settle()
def key(c, mods=0): cpu.events.append((3, 300, 20, c, mods)); settle(); settle()
key(9); key(0x15); key(0x0A); key(0x0A); key(0x15); key(0x0A); key(0x0D)
check(w('WIN') == 25, 'Solitaer offen')
X = X3 + lbl['SPN']; C = X3 + lbl['SPC']
def setp(p, cards):
    for i, c in enumerate(cards): cpu.mem[C + 52 * p + i] = c
    cpu.wr16(X + 2 * p, len(cards))
def redraw(): key(ord('x'))   # unbekannte Taste: nichts
for p in range(13): setp(p, [])
setp(6, [5, 0 | 0x80])                       # Kreuz-As oben auf Spalte 1
setp(7, [13 | 0x80])                         # Karo-As in Spalte 2
setp(8, [30 | 0x80, 11 | 0x80])              # Herz-5, Kreuz-Dame
setp(9, [12 | 0x80, 37 | 0x80, 49 | 0x80])   # Kreuz-Koenig, Herz-Dame, Pik-Bube
setp(0, [20, 21]); setp(1, [26 | 0x80])      # Talon, Ablage: Herz-As
def cxy(p, i=None):
    if p < 6:
        col = p + 1 if p >= 2 else p
        return 6 + col * 44 + 20, 18 + 6
    n = x3('SPN', 2 * p); cs = pile(p); nd = sum(1 for c in cs if not c & 0x80)
    i = n - 1 if i is None else i
    y = 70 + (i * 4 if i < nd else nd * 4 + (i - nd) * 11)
    return 6 + (p - 6) * 44 + 20, y + 4
# Fenster neu zeichnen lassen (Klick auf leere Tischflaeche)
click(300, 150)
x, y = cxy(6)
cpu.events += [(1, x, y, 0), (2, x, y, 0), (1, x, y, 0), (2, x, y, 0)]; settle(); settle()
print([[hex(c) for c in pile(p)] for p in range(2, 6)])
check(any(pile(p) == [0x80] for p in range(2, 6)), 'Doppelklick: Kreuz-As auf leeren Zielstapel')
def drag(sx, sy, tx, ty):
    cpu.events += [(0, sx, sy, 0)]; settle()
    cpu.events += [(1, sx, sy, 0)] + [(0, sx + (tx - sx) * k // 6, sy + (ty - sy) * k // 6, 0) for k in range(1, 7)] + [(2, tx, ty, 0)]
    settle(); settle()
x, y = cxy(7); tx, ty = cxy(4)
drag(x, y, tx, ty + 10)
check(pile(4) == [13 | 0x80] or any(pile(p) == [13 | 0x80] for p in range(2, 6)), 'Ziehen: Karo-As auf leeren Zielstapel')
render(cpu, 'tsol2_a.png', 3)
# Stapel ziehen: Herz-Dame + Pik-Bube auf ... (kein Ziel) -> zurueck, Bild wie vorher
import hashlib
cpu.events += [(0, 300, 150, 0)]; settle()
scr0 = bytes(cpu.mem[0xE12000:0xE19D00])
x, y = cxy(9, 1)
drag(x, y, 300, 150)
scr1 = bytes(cpu.mem[0xE12000:0xE19D00])
d = sum(1 for a, b in zip(scr0, scr1) if a != b)
check(pile(9) == [12 | 0x80, 37 | 0x80, 49 | 0x80] and d == 0, 'Stapel ohne Ziel: zurueck, keine Reste (%d Bytes anders)' % d)
render(cpu, 'tsol2_b.png', 3)
# mitten im Ziehen ansehen
cpu.events += [(0, x, y, 0)]; settle()
cpu.events += [(1, x, y, 0)] + [(0, x + k * 10, y + k * 8, 0) for k in range(1, 7)]
run(cpu, 4_000_000)
render(cpu, 'tsol2_c.png', 3)
cpu.events += [(2, x + 60, y + 48, 0)]; settle(); settle()
print('ALLES OK' if twbase.ok else 'FAIL')
