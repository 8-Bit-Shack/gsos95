# v2.18: Startmenue mit Gruppen (Werkzeuge, Internet, Spiele), Solitaer
from twbase import *
import twbase
X3 = 0x200000
def x3(n, o=0): return cpu.rd16(X3 + lbl[n] + o)
def pile(p):
    n = x3('SPN', 2 * p); a = X3 + lbl['SPC'] + 52 * p
    return list(cpu.mem[a:a + n])
cpu = boot(); settle()
def key(c, mods=0): cpu.events.append((3, 300, 20, c, mods)); settle(); settle()
key(9); key(0x15)                       # Start > Programme
check(w('SUBM') == 1, 'Programme offen')
render(cpu, 'tsol_m0.png', 3)
key(0x0A); key(0x0A)                     # Spiele
key(0x15)                                # Gruppe oeffnen
check(w('S2G') == 3 and w('S2F') == 1, 'Gruppe Spiele offen (S2G %d)' % w('S2G'))
render(cpu, 'tsol_m1.png', 3)
key(0x0A); key(0x0D)                     # Solitaer
check(w('WIN') == 25, 'Solitaer offen (WIN %d)' % w('WIN'))
counts = [x3('SPN', 2 * p) for p in range(13)]
print(counts)
check(counts == [24, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7], 'ausgeteilt')
cards = sorted(c & 0x3F for p in range(13) for c in pile(p))
check(cards == list(range(52)), 'alle 52 Karten genau einmal')
check(all((pile(6 + k)[-1] & 0x80) and not any(c & 0x80 for c in pile(6 + k)[:-1]) for k in range(7)), 'je Spalte nur die oberste offen')
render(cpu, 'tsol_1.png', 3)
# ---- Spielen: Talon, Ziehen, Doppelklick, Rueckgaengig
def cxy(p, i=None):                       # Bildschirm-Mitte oben einer Karte
    if p < 6:
        col = p + 1 if p >= 2 else p
        return 6 + col * 44 + 20, 18 + 6
    n = x3('SPN', 2 * p); cs = pile(p); nd = sum(1 for c in cs if not c & 0x80)
    i = n - 1 if i is None else i
    y = 70 + (i * 4 if i < nd else nd * 4 + (i - nd) * 11)
    return 6 + (p - 6) * 44 + 20, y + 4
def rank(c): return (c & 0x3F) % 13
def suit(c): return (c & 0x3F) // 13
def red(c): return suit(c) in (1, 2)
def legal(c, t):
    ts = pile(t)
    if t < 6:
        return (not ts and rank(c) == 0) or (ts and suit(ts[-1]) == suit(c) and rank(ts[-1]) + 1 == rank(c))
    if not ts: return rank(c) == 12
    return bool(ts[-1] & 0x80) and red(ts[-1]) != red(c) and rank(ts[-1]) == rank(c) + 1
def drag(p, i, t):
    sx, sy = cxy(p, i); tx, ty = cxy(t) if x3('SPN', 2 * t) else ((6 + ((t + 1 if 2 <= t < 6 else t) - (6 if t >= 6 else 0)) * 44 + 20), (70 if t >= 6 else 18) + 6)
    cpu.events += [(0, sx, sy, 0)]; settle()
    cpu.events += [(1, sx, sy, 0)] + [(0, sx + (tx - sx) * k // 6, sy + (ty - sy) * k // 6, 0) for k in range(1, 7)] + [(2, tx, ty, 0)]
    settle(); settle()
cs0, cw0 = x3('SPN', 0), x3('SPN', 2)
click(*cxy(0))
check(x3('SPN', 0) == cs0 - 1 and x3('SPN', 2) == 1 and pile(1)[-1] & 0x80, 'Talon: eine Karte offen auf die Ablage')
key(ord('u'))
check(x3('SPN', 0) == cs0 and x3('SPN', 2) == 0, 'Rueckgaengig')
# einen erlaubten Zug suchen (Spalte -> Spalte oder Ziel), sonst Talon ziehen
moved = False
for rnd in range(30):
    srcs = [(p, x3('SPN', 2 * p) - 1) for p in [1] + list(range(6, 13)) if x3('SPN', 2 * p)]
    for p in range(6, 13):                    # auch ganze Stapel ab der ersten offenen Karte
        cs = pile(p)
        for i, c in enumerate(cs):
            if c & 0x80: srcs.append((p, i)); break
    for p, i in srcs:
        c = pile(p)[i]
        if not c & 0x80: continue
        for t in list(range(2, 6)) + list(range(6, 13)):
            if t == p or (t < 6 and i != x3('SPN', 2 * p) - 1): continue
            if t >= 6 and not pile(t) and rank(c) == 12 and i == 0: continue
            if legal(c, t):
                before = (x3('SPN', 2 * p), x3('SPN', 2 * t), x3('SSC'))
                drag(p, i, t)
                after = (x3('SPN', 2 * p), x3('SPN', 2 * t), x3('SSC'))
                print('Zug', p, i, '->', t, before, after)
                check(after[0] == i and after[1] == before[1] + before[0] - i, 'Ziehen mit der Maus: Stapel %d -> %d' % (p, t))
                moved = True
                break
        if moved: break
    if moved: break
    click(*cxy(0))
check(moved, 'mindestens ein Zug gefunden und gezogen')
render(cpu, 'tsol_2.png', 3)
# verdeckte Karte umdrehen
for p in range(6, 13):
    cs = pile(p)
    if cs and not cs[-1] & 0x80:
        click(*cxy(p)); check(pile(p)[-1] & 0x80, 'verdeckte Karte umgedreht'); break
# 3 Karten
key(ord('d'))
check(x3('SD3') == 1 and x3('SPN', 0) == 24, 'D: drei Karten, neues Spiel')
click(*cxy(0)); check(x3('SPN', 2) == 3, 'drei Karten auf die Ablage')
render(cpu, 'tsol_3.png', 3)
# ---- Gewinn: fast fertiger Stand, letzter Zug per Doppelklick
X = X3 + lbl['SPN']; C = X3 + lbl['SPC']
for p in range(13): cpu.wr16(X + 2 * p, 0)
for s_ in range(4):
    n = 13 if s_ else 12
    for r in range(n): cpu.mem[C + 52 * (2 + s_) + r] = (s_ * 13 + r) | 0x80
    cpu.wr16(X + 2 * (2 + s_), n)
cpu.mem[C + 52 * 6] = 12 | 0x80; cpu.wr16(X + 12, 1)      # Kreuz-Koenig in Spalte 1
cpu.wr16(X3 + lbl['SWON'], 0); cpu.wr16(X3 + lbl['STS'], 100)
key(ord('n') ^ 0) if False else None
cpu.wr16(X + 12, 1)
key(0x1A) if False else None
x, y = cxy(6)
cpu.events += [(1, x, y, 0), (2, x, y, 0), (1, x, y, 0), (2, x, y, 0), (3, 300, 20, 32, 0)]
settle(); settle()
check(x3('SPN', 4) == 13 and x3('SWON') == 1, 'Doppelklick legt ab, gewonnen')
check(x3('SSC') >= 7000, 'Zeitbonus (%d)' % x3('SSC'))
render(cpu, 'tsol_4.png', 3)
key(0x1B)
check(w('WIN') != 25, 'Esc schliesst')
print('ALLES OK' if twbase.ok else 'FAIL Solitaer')
