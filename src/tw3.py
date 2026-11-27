# Browser: relative Adressen, Bildlauf, Groesse, grosse Seiten, Textdateien, Fehler, Esc beim Laden
import sys; sys.path.insert(0, '.')
from t3 import *
import t3
from emu import run, render
from w5100sim import W5100, Net
lbl = {l.split()[2].lstrip('.'): int(l.split()[1], 16) for l in open('app.lbl')}
ADDR = 0x030400; ML = ADDR + lbl['mainloop']
net = Net(); chip = W5100(net)
oldrd, oldwr = AppCPU.io_rd, AppCPU.io_wr
def io_rd(s, r):
    if 0xB4 <= r <= 0xB7: return chip.io_rd(r - 0xB4)
    return oldrd(s, r)
def io_wr(s, r, v):
    if 0xB4 <= r <= 0xB7: chip.io_wr(r - 0xB4, v); return
    oldwr(s, r, v & 0xFF)
AppCPU.io_rd, AppCPU.io_wr = io_rd, io_wr
H = b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n'
def links(*hs):
    return H + b'<title>L</title>' + b''.join(b'<p><a href="%s">L%d</a></p>' % (h.encode(), i) for i, h in enumerate(hs))
net.pages[('10.1.1.2', 80, '/a/b/c.html')] = links('d.html', '../e.html', './f/g.html', '/root.html', '?q=1',
                                                    '//info.cern.ch/x', 'http://test.local:8080/p', '../../../z', 'mailto:x@y', '#top')
for p in ('/a/b/d.html', '/a/e.html', '/a/b/f/g.html', '/root.html', '/a/b/c.html?q=1', '/z'):
    net.pages[('10.1.1.2', 80, p)] = H + b'<title>' + p.encode() + b'</title>ok'
net.pages[('10.1.1.3', 80, '/x')] = H + b'<title>cern</title>ok'
net.pages[('10.1.1.2', 8080, '/p')] = H + b'<title>port</title>ok'
net.pages[('10.1.1.2', 80, '/big')] = H + b'<title>big</title>' + b'<p>' + b'Wort ' * 9000 + b'</p>'
net.pages[('10.1.1.2', 80, '/t.txt')] = b'HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nZeile <1>\n  & Zeile 2\n'
net.pages[('10.1.1.2', 80, '/p.gif')] = b'HTTP/1.0 200 OK\r\nContent-Type: image/gif\r\n\r\nGIF89a'
net.pages[('10.1.1.2', 80, '/404')] = b'HTTP/1.0 404 Not Found\r\n\r\n'
net.pages[('10.1.1.2', 80, '/hang')] = 'hang'
net.pages[('10.1.1.2', 80, '/lang')] = H + b'<title>lang</title><p>' + b' '.join(b'<a href="/l%d">Ein ziemlich langer Linktext Nummer %d</a>' % (i, i) for i in range(4)) + b'</p>'
for i in range(4): net.pages[('10.1.1.2', 80, '/l%d' % i)] = H + b'<title>l%d</title>' % i
net.pages[('10.1.1.2', 80, '/long')] = H + b''.join(b'<p>Zeile %d</p>' % i for i in range(80))

cpu = boot_app(ADDR); cpu.hnext = 0x0A0000
def settle(maxn=60_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
settle()
DPV = {'WIN': 0xB4, 'PAGE': 0x1C}
def w(n):
    if n in DPV: return cpu.rd16(cpu.D + DPV[n])
    return cpu.rd16(ADDR + lbl[n])
def key(c, mods=0): cpu.events.append((3, 160, 100, c, mods)); settle(); settle()
def typ(t):
    for ch in t: key(ord(ch))
def click(h, v):
    cpu.events += [(0, h, v, 0)]; settle()
    cpu.events += [(1, h, v, 0), (2, h, v, 0)]; settle(); settle()
def bstr(off):
    b = w('NWB') << 16; n = cpu.mem[b + off]; return bytes(cpu.mem[b + off + 1:b + off + 1 + n])
def go(u): typ(u); key(0x0D)
def rec():
    r = cpu.mem[ADDR + lbl['NREC']:ADDR + lbl['NREC']]
ok = True
def check(cond, msg):
    global ok
    print(('ok  ' if cond else 'FEHLER ') + msg); ok &= bool(cond)
def wx(): return cpu.rd16(ADDR + lbl['wrecs'] + 2)      # Lage des Fensters (einziges Fenster)
def wy(): return cpu.rd16(ADDR + lbl['wrecs'] + 4)
def clink(i):   # i-te Zeile (0 = oberste sichtbare) anklicken
    click(wx() + 8 + 6, wy() + 31 + 10 * i + 4)

# Start ueber das Startmenue: Programme > Internet
click(10, 193)
click(20 + 40, 186 - (7 * 18 + 12) + 3 + 8)     # Programme
sy = cpu.rd16(ADDR + lbl['SUBY'])
click(114 + 30, sy + 2 + 12 * 1 + 5)            # Gruppe Internet
s2y = cpu.rd16(ADDR + lbl['S2Y'])
click(222 + 30, s2y + 2 + 5)                    # Internet
check(w('WIN') == 21, 'Startmenue > Programme > Internet')
go('test.local/a/b/c.html')
check(bstr(0xFC00) == b'L', 'Linkseite')
exp = ['/a/b/d.html', '/a/e.html', '/a/b/f/g.html', '/root.html', '/a/b/c.html?q=1', 'cern', 'port', '/z']
# Zeilen: Leerzeile zwischen Absaetzen -> Link i in Zeile 2*i
for i, e in enumerate(exp):
    if i: key(0x08)                     # zurueck zur Linkseite
    key(0x0B); key(0x0B)
    while w('PAGE') > 0: key(0x0B)
    tgt = 2 * i
    while tgt - w('PAGE') > 10: key(0x0A)
    clink(tgt - w('PAGE'))
    check(bstr(0xFC00) == e.encode(), 'Link %d -> %r (Adresse %r)' % (i, bstr(0xFC00), bstr(0xFD80)))
key(0x08)
while w('PAGE') > 0: key(0x0B)
while 16 - w('PAGE') > 10: key(0x0A)
clink(16 - w('PAGE'))
check(bstr(0xFC00) == b'L' and w('STMSG') == lbl['ws_nolink'], 'mailto: nicht unterstuetzt')
# grosse Seite: gekuerzt
go('test.local/big')
b = w('NWB') << 16
txt = bytes(cpu.mem[b + 0x8000:b + w('TXTEND')])
check(w('TXTEND') > 0xBF00 and txt.endswith(bytes([0x5B]) + b'Seite gek' + bytes([0x82]) + b'rzt - zu gro' + bytes([0x86]) + b']'), 'grosse Seite gekuerzt (%d Zeilen)' % w('NLINES'))
# Bildlaufleiste: unten in die Spur klicken
p0 = w('PAGE')
click(wx() + cpu.rd16(ADDR + lbl['wrecs'] + 22) - 12, wy() + 120)
check(w('PAGE') > p0, 'Bildlaufleiste blaettert (%d)' % w('PAGE'))
# Text-Datei
go('test.local/t.txt')
txt = bytes(cpu.mem[b + 0x8000:b + w('TXTEND')])
check(txt == b'Zeile <1>\n  & Zeile 2\n', 'text/plain: %r' % txt)
go('test.local/p.gif')
txt = bytes(cpu.mem[b + 0x8000:b + w('TXTEND')])
check(w('GIFON') == 1 or b'GIF' in txt, 'GIF: angezeigt oder Hinweis (seit 2.10 Bildanzeige)')
go('test.local/404')
txt = bytes(cpu.mem[b + 0x8000:b + w('TXTEND')])
check(b'404' in txt, 'HTTP 404: %r' % txt[-60:])
# Maximieren: neuer Umbruch, lange Links ueber zwei Zeilen
go('test.local/lang')
n1 = w('NLINES')
click(wx() + cpu.rd16(ADDR + lbl['wrecs'] + 22) - 31 + 6, wy() + 5 + 5)    # Maximieren
check(cpu.rd16(ADDR + lbl['wrecs'] + 22) == 320 and w('WRAPW') == 290, 'maximiert, Breite %d' % w('WRAPW'))
n2 = w('NLINES')
check(n2 <= n1, 'nicht mehr Zeilen nach Maximieren: %d -> %d' % (n1, n2))
render(cpu, 'tw3a.png', 3)
clink(1)                                 # zweite Zeile: Fortsetzung eines Links
check(bstr(0xFC00).startswith(b'l'), 'Link in Folgezeile: %r' % bstr(0xFC00))
# Esc waehrend des Ladens
typ('test.local/hang')
cpu.events += [(3, 160, 100, 0x0D, 0), (3, 160, 100, 0x1B, 0)]
settle(); settle()
txt = bytes(cpu.mem[b + 0x8000:b + w('TXTEND')])
check(b'Abgebrochen' in txt, 'Esc bricht ab')
# Englisch
cpu.mem[cpu.D + 0x54] = 1
go('about:start')
check(bstr(0xFC00) == b'GS/OS95 Internet' and b'Welcome' in bytes(cpu.mem[b + 0x8000:b + w('TXTEND')]), 'Startseite englisch')
render(cpu, 'tw3b.png', 3)
print('OK' if ok else 'FAIL Browser 2')
