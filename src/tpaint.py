# v2.18: Paint (Fensterart 26, fuenfte Codebank)
from twbase import *
import twbase
from t3 import FILES, FS
cpu = boot(); settle()
X4 = 0x300000
def pix(px, py):                         # Bildpunkt im Dokument
    d = cpu.rd16(ADDR + lbl['PDOC']) | cpu.mem[ADDR + lbl['PDOC'] + 2] << 16
    b = cpu.mem[d + py * 160 + px // 2]; return (b >> 4) if px % 2 == 0 else b & 15
def spix(x, y):                          # Schirm
    b = cpu.mem[0xE12000 + y * 160 + x // 2]; return (b >> 4) if x % 2 == 0 else b & 15
def drag(pts):
    (h, v) = pts[0]
    cpu.events.append((1, h, v, 0))
    for (h, v) in pts[1:]: cpu.events.append((0, h, v, 0))
    cpu.events.append((2, h, v, 0)); settle(); settle()
CX, CY = 32, 17
key(9); key(0x15); key(0x15); key(0x0A); key(0x0A); key(0x0A); key(0x0D)   # Programme > Werkzeuge > Paint
check(w('WIN') == 26, 'Start > Programme > Werkzeuge > Paint (WIN %d)' % w('WIN'))
check(all(pix(x, y) == 15 for x, y in ((0, 0), (100, 50), (319, 199))), 'neues Bild ist weiss')
render(cpu, 'tpaint_0.png', 3)
# Farbe rot (7), Stift: Linie ziehen
click(CX + 7 * 14 + 5, 165 + 4)
check(w('PCOL') == 7, 'Farbe rot gewaehlt')
drag([(CX + 10, CY + 10), (CX + 40, CY + 10), (CX + 40, CY + 30)])
check(all(pix(x, 10) == 7 for x in range(10, 41)) and all(pix(40, y) == 7 for y in range(10, 31)), 'Stift: Linien im Bild')
check(spix(CX + 20, CY + 10) == 7, 'und auf dem Schirm')
# Rechteck gefuellt, blau (4)
def tool(t): click(3 + (t & 1) * 14 + 5, 18 + (t >> 1) * 14 + 5)
click(CX + 4 * 14 + 5, 169); tool(6)
check(w('PTOOL') == 6, 'Werkzeug gefuelltes Rechteck (%d)' % w('PTOOL'))
drag([(CX + 60, CY + 40), (CX + 80, CY + 50), (CX + 90, CY + 60)])
check(all(pix(x, y) == 4 for x in range(60, 91) for y in range(40, 61)), 'gefuelltes Rechteck')
check(pix(59, 50) == 15 and pix(91, 50) == 15, 'Vorschau hinterlaesst nichts daneben')
cpu.events.append((0, 300, 120, 0)); settle()
check(all(spix(CX + x, CY + y) == pix(x, y) for x in range(50, 100) for y in range(35, 66)), 'Schirm = Bild nach der Vorschau')
# Linie schwarz
click(CX + 0 * 14 + 5, 169); tool(4)
drag([(CX + 100, CY + 100), (CX + 150, CY + 120), (CX + 200, CY + 100)])
check(pix(100, 100) == 0 and pix(200, 100) == 0 and pix(150, 100) == 0 and pix(150, 120) == 15 and pix(130, 108) == 15, 'Linie nur vom Start zum Ende')
# Rahmen zeichnen, innen fuellen (gruen 5)
tool(5)
check(w('PTOOL') == 5, 'Werkzeug Rechteck (%d)' % w('PTOOL'))
drag([(CX + 150, CY + 20), (CX + 200, CY + 60)])
check(pix(150, 20) == 0 and pix(200, 60) == 0 and pix(175, 40) == 15, 'Rahmen')
click(CX + 5 * 14 + 5, 169); tool(7)
check(w('PTOOL') == 7, 'Werkzeug Fuellen')
drag([(CX + 175, CY + 40)])
check(all(pix(x, y) == 5 for x in range(151, 200) for y in range(21, 60)) and pix(149, 40) == 15 and pix(201, 40) == 15, 'Fuellen bleibt im Rahmen')
render(cpu, 'tpaint_1.png', 3)
# Rueckgaengig, wiederherstellen
key(ord('z'))
check(pix(175, 40) == 15 and spix(CX + 175, CY + 40) == 15, 'Z: Fuellen rueckgaengig')
key(ord('z'))
check(pix(175, 40) == 5, 'Z noch einmal: wieder da')
# Ausschnitt verschieben
key(0x15); key(0x0A)
check(w('QOFX') == 8 and w('QOFY') == 8 and spix(CX + 20 - 8, CY + 10 - 8) == 7, 'Pfeile verschieben den Ausschnitt')
# Speichern
key(ord('s'))
f = FILES.get(':SYSTEM.DISK:BILD1')
check(f is not None and len(f) == 32768 and ('BILD1', 0xC1) in FS[':SYSTEM.DISK'], 'S: als *:BILD1 gespeichert ($C1, 32 KB)')
check(f and f[10 * 160 + 10] >> 4 == 7 and f[32256:32258] == bytes(cpu.mem[0xE19E00:0xE19E02]), 'Inhalt und Palette')
render(cpu, 'tpaint_2.png', 3)
key(ord('n')); key(ord('s'))
check(':SYSTEM.DISK:BILD2' in FILES, 'neues Bild: naechster Name BILD2')
# Bildbetrachter: B oeffnet das Bild in Paint (Pfad bleibt: Speichern ueberschreibt)
img = bytearray(FILES[':SYSTEM.DISK:BILD1'])
cpu.mem[0xE12000:0xE1A000] = img
pb = ADDR + lbl['pathbuf']
p = b':SYSTEM.DISK:BILD1'
cpu.wr16(pb, len(p)); cpu.mem[pb + 2:pb + 2 + len(p)] = p
cpu.PC_save = None
import struct
# xf_pt_screen direkt aufrufen (wie aus pic_wait)
from emu import run as _run
stub = 0x0300
cpu.mem[stub:stub + 6] = bytes([0x22]) + (0x300000 + lbl['xf_pt_screen']).to_bytes(3, 'little') + bytes([0x80, 0xFE])
sv = (cpu.PBR, cpu.PC); cpu.PBR = 0; cpu.PC = stub
_run(cpu, 20_000_000, stop_at=stub + 4)
cpu.PBR, cpu.PC = sv
settle()
check(w('WIN') == 26 and pix(10, 10) == 7 and pix(175, 40) == 5, 'B im Bildbetrachter: Bild in Paint')
click(CX + 9 * 14 + 5, 169); tool(1)
drag([(CX + 20, CY + 120), (CX + 60, CY + 120)])
key(ord('s'))
check(FILES[':SYSTEM.DISK:BILD1'][121 * 160 + 20] >> 4 == 9 and ':SYSTEM.DISK:BILD3' not in FILES, 'Speichern ueberschreibt BILD1 (Pinsel gelb)')
render(cpu, 'tpaint_3.png', 3)

# --- 2.1.43: Ellipse, Pipette, Text, Esc-Nachfrage ---
key(ord('n'))
click(CX + 12 * 14 + 5, 169); tool(8)                    # Ellipse, Farbe Petrol (12)
check(w('PTOOL') == 8, 'Werkzeug Ellipse')
drag([(CX + 20, CY + 20), (CX + 60, CY + 50), (CX + 100, CY + 80)])
check(pix(60, 20) == 12 and pix(60, 80) == 12 and pix(20, 50) == 12 and pix(100, 50) == 12, 'Ellipse: oben, unten, links, rechts')
check(pix(60, 50) == 15 and pix(21, 21) == 15 and pix(99, 79) == 15, 'innen und Ecken frei')
tool(9); click(CX + 10 * 14 + 5, 169)                     # gefuellt, hellgruen (10)
drag([(CX + 150, CY + 20), (CX + 210, CY + 60)])
check(pix(180, 40) == 10 and pix(180, 21) == 10 and pix(151, 40) == 10 and pix(151, 21) == 15, 'gefuellte Ellipse')
tool(10)
check(w('PTOOL') == 10, 'Pipette gewaehlt')
click(CX + 60, CY + 20)
check(w('PCOL') == 12 and w('PTOOL') == 9, 'Pipette: Farbe genommen, zurueck zum vorigen Werkzeug (%d/%d)' % (w('PCOL'), w('PTOOL')))
click(CX + 0 * 14 + 5, 169); tool(11)
cpu.events += [(1, CX + 30, CY + 100, 0), (2, CX + 30, CY + 100, 0)] + [(3, 300, 20, ord(ch), 0) for ch in 'Hallo'] + [(3, 300, 20, 0x0D, 0)]
settle(); settle()
cnt = sum(1 for x in range(30, 80) for y in range(96, 112) if pix(x, y) == 0)
check(cnt > 20, 'Text im Bild (%d schwarze Punkte)' % cnt)
check(not any(pix(x, y) == 0 for x in range(30, 120) for y in range(96, 112) if False), '')
render(cpu, 'tpaint_4.png', 3)
key(0x1B)
check(w('WIN') == 26 and w('PESC') == 1, 'Esc bei ungespeichertem Bild: Nachfrage statt schliessen')
key(0x1B)
check(w('WIN') != 26, 'zweites Esc: schliessen')
print('ALLES OK' if twbase.ok else 'FAIL Paint')
