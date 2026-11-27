# Vollbild-Terminal (ANSI/VT100) fuer Telnet
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
from PIL import Image
net.dns['bbs.example'] = '10.1.1.20'
ESC = b'\x1b'
TIN = []
SCREEN = (b'\xff\xfd\x18\xff\xfd\x1f\xff\xfb\x01\xff\xfb\x03\xff\xfa\x18\x01\xff\xf0'
          + ESC + b'[2J' + ESC + b'[H' + ESC + b'[1;33mGelb hell' + ESC + b'[0m normal\r\n'
          + b'\xc9' + b'\xcd' * 10 + b'\xbb\r\n'
          + ESC + b'[5;10H' + ESC + b'[7mINVERS' + ESC + b'[27m'
          + ESC + b'[10;1H' + ESC + b'[44;37mblau' + ESC + b'[0m'
          + ESC + b'[6n')
def bbs(d):
    if d is None: return SCREEN
    TIN.append(d)
    if d == b'x': return b'X'
    if d == b'\x1b[A': return b'hoch'
    if d == b'scroll': return b''.join(b'\r\nZeile %d' % i for i in range(30))
    return b''
net.pages[('10.1.1.20', 23, '*')] = bbs
def idle(n=12):
    for i in range(n): settle()
def imbk(): return w('IMBK') << 16
def cell(x, y):
    a = imbk() + (y * 80 + x) * 2; return cpu.mem[a], cpu.mem[a + 1]
def row(y): return bytes(cell(x, y)[0] for x in range(80)).decode('cp437').rstrip()
def render640(path):
    img = Image.new('RGB', (640, 200)); px = img.load(); mem = cpu.mem
    for y in range(200):
        scb = mem[0xE19D00 + y]; pal = 0xE19E00 + (scb & 15) * 32
        cols = []
        for i in range(16):
            v = mem[pal + i * 2] | mem[pal + i * 2 + 1] << 8
            cols.append(((v >> 8 & 15) * 17, (v >> 4 & 15) * 17, (v & 15) * 17))
        for xb in range(160):
            b = mem[0xE12000 + y * 160 + xb]
            if scb & 0x80:
                for i, g in enumerate((8, 12, 0, 4)):
                    px[xb * 4 + i, y] = cols[g + (b >> (6 - 2 * i) & 3)]
            else:
                for i in range(4): px[xb * 4 + i, y] = cols[b >> 4 if i < 2 else b & 15]
    img.resize((1280, 480), Image.NEAREST).save(path)
click(101, 193)
typ('telnet://bbs.example'); key(0x0D)
idle()
check(w('VTON') == 1, 'Terminal an')
check(w('VTCM') == 1 and all(cpu.mem[0xE19D00 + i] == 0x0F for i in range(200)), 'Vorgabe Farbmodus: SCBs 320er, Palette 15')
pal = [cpu.mem[0xE19E00 + 15 * 32 + 2 * i] | cpu.mem[0xE19E00 + 15 * 32 + 2 * i + 1] << 8 for i in range(16)]
check(pal[1] == 0x0A00 and pal[11] == 0x0FF5 and pal[15] == 0x0FFF, '16 ANSI-Farben')
A = b''.join(TIN); print(A)
check(b'\xff\xfb\x18' in A and b'\xff\xfb\x1f' in A, 'WILL TTYPE / WILL NAWS')
check(b'\xff\xfa\x1f\x00\x50\x00\x19\xff\xf0' in A, 'NAWS 80x25')
check(b'\xff\xfa\x18\x00ANSI\xff\xf0' in A, 'Terminaltyp ANSI')
check(b'\xff\xfd\x01' in A and b'\xff\xfd\x03' in A, 'DO ECHO / DO SGA')
for y in range(12): print(repr(row(y)))
check(row(0).startswith('Gelb hell normal'), 'Text Zeile 1')
check(cell(0, 0)[1] == 0x43 and cell(10, 0)[1] == 0x07, 'Attribut hell gelb / normal')
check(row(1) == '╔' + '═' * 10 + '╗', 'Rahmenzeichen CP437')
check(row(4)[9:15] == 'INVERS' and cell(9, 4)[1] == 0x38, 'Position 5;10 und invers')
check(row(9).startswith('blau') and cell(0, 9)[1] == 0x27, 'Hintergrund blau')
check(b'\x1b[10;5R' in A, 'Positionsmeldung ESC[6n')
render640('tw17a.png')
# Farbe der Zelle 'G' (hell gelb = 11) auf dem Bildschirm
so = 0
scr = [cpu.mem[0xE12000 + so + r * 160 + b] for r in range(8) for b in (0, 1)]
nib = set(v >> 4 for v in scr) | set(v & 15 for v in scr)
check(nib <= {0, 11} and 11 in nib, 'Farbmodus: hell gelb auf schwarz')
key(ord('f'), 0x0100); idle(4)
check(w('VTCM') == 0 and all(cpu.mem[0xE19D00 + i] == 0x8F for i in range(200)), 'Apfel-F: scharf/grau (640er)')
check(cpu.mem[ADDR + lbl['NPREF'] + 2] & 0x80 != 0, 'Wahl gemerkt (NPREF+2 Bit 7)')
check(row(0).startswith('Gelb hell normal'), 'Inhalt bleibt beim Umschalten')
render640('tw17g.png')
key(ord('f'), 0x0100); idle(4)
check(w('VTCM') == 1 and cpu.mem[0xE19D00] == 0x0F, 'Apfel-F: zurueck zu Farbe')
n0 = len(TIN)
key(ord('x'))
idle(4)
check(TIN[n0:n0 + 1] == [b'x'], 'Taste direkt gesendet')
check(row(9).startswith('blauX'), 'Antwort erscheint (Gegenstelle zeigt selbst)')
key(0x0B); idle(4)
check(b'\x1b[A' in TIN, 'Pfeil hoch als ESC [ A')
key(0x0D); idle(2)
check(TIN[-1] == b'\r\x00', 'Return als CR NUL')
# Rollen, Rollbereich, Zeilen einfuegen/loeschen, Umbruch in Spalte 80
def bbs2(d):
    TIN.append(d)
    if d == b'z': return ESC + b'[2J' + b''.join(b'Zeile %d\r\n' % i for i in range(30)) + b'Ende'
    if d == b'r': return (ESC + b'[2J' + ESC + b'[H' + b''.join(b'R%d\r\n' % i for i in range(25))[:-2]
                          + ESC + b'[5;10r' + ESC + b'[10;1H\r\nneu' + ESC + b'[r')
    if d == b'l': return ESC + b'[2J' + ESC + b'[H' + b'A\r\nB\r\nC\r\nD' + ESC + b'[2;1H' + ESC + b'[2L' + ESC + b'[5;1H' + ESC + b'[1M'
    if d == b'w': return ESC + b'[2J' + ESC + b'[H' + b'x' * 80 + b'\r\nzwei'
    if d == b'k': return ESC + b'[2J' + ESC + b'[H' + b'abcdefghij' + ESC + b'[1;4H' + ESC + b'[2P' + ESC + b'[1;2H' + ESC + b'[3@' + ESC + b'[1;8H' + ESC + b'[K'
    if d == b'q': return (b'\r\nTschuess', 'zu')
    return b''
net.pages[('10.1.1.20', 23, '*')] = bbs2
key(ord('z')); idle(6)
print([row(y) for y in (0, 1, 23, 24)])
check(row(0) == 'Zeile 6' and row(23) == 'Zeile 29' and row(24) == 'Ende', 'Rollen ueber 25 Zeilen')
render640('tw17c.png')
key(ord('r')); idle(6)
print([row(y) for y in range(3, 11)])
check(row(3) == 'R3' and row(4) == 'R5' and row(8) == 'R9' and row(9) == 'neu' and row(10) == 'R10', 'Rollbereich 5..10')
key(ord('l')); idle(6)
print([row(y) for y in range(6)])
check([row(y) for y in range(6)] == ['A', '', '', 'B', 'D', ''], 'Zeilen einfuegen (L) und loeschen (M)')
key(ord('w')); idle(6)
check(row(0) == 'x' * 80 and row(1) == 'zwei' and row(2) == '', 'Spalte 80: Umbruch erst beim naechsten Zeichen')
key(ord('k')); idle(6)
print(repr(row(0)))
check(row(0) == 'a   bcf', 'Zeichen loeschen (P), einfuegen (@), Zeilenrest loeschen (K)')
cpu.wr16(ADDR + lbl['TNECHO'], 0)          # Gegenstelle ohne ECHO-Ankuendigung
before = row(0)
key(ord('n')); idle(4)
check(row(0) == before, 'kein lokales Echo (sonst doppelte Zeichen)')
key(ord('q')); idle(6)
check(w('VTWAIT') == 1 and w('VTON') == 1, 'Gegenstelle beendet: Meldung, Terminal wartet')
print([row(y) for y in range(0, 3)])
check(row(1) == 'Tschuess' and row(2).startswith('[Verbindung beendet'), 'Text und Abschlussmeldung')
render640('tw17d.png')
key(ord(' ')); idle(4)
check(w('VTON') == 0 and all(cpu.mem[0xE19D00 + i] == 0 for i in range(200)), 'Taste: zurueck zum Schreibtisch')
render640('tw17e.png')
# erneut verbinden und mit Apfel-Esc beenden
net.pages[('10.1.1.20', 23, '*')] = bbs
click(101, 193)
typ('telnet://bbs.example'); key(0x0D); idle()
check(w('VTON') == 1, 'zweite Verbindung: Terminal wieder an')
key(0x1D); idle(4)
check(w('VTON') == 0, 'Strg-] beendet das Terminal')
check(all(cpu.mem[0xE19D00 + i] == 0 for i in range(200)), 'SCBs wieder 320er')
check(w('TNON') == 0, 'Verbindung zu')
render640('tw17b.png')
print('ALLES OK' if ok else 'FEHLER')
