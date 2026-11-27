# Internet: Uthernet II (nachgebildet), DHCP, DNS, HTTP, HTML-Darstellung, Links, Zurueck
import sys; sys.path.insert(0, '.')
from t3 import *
import t3
from emu import run, render
from w5100sim import W5100, Net

lbl = {l.split()[2].lstrip('.'): int(l.split()[1], 16) for l in open('app.lbl')}
ADDR = 0x030400; ML = ADDR + lbl['mainloop']
net = Net()
import os
if os.environ.get('GS2'):
    from gs2sim import GS2
    GS2.PERIOD = int(os.environ.get('GS2'))
    GS2.DHCP_OK = bool(os.environ.get('DHCP'))
    chip = GS2(net)
else:
    chip = W5100(net)
oldrd, oldwr = AppCPU.io_rd, AppCPU.io_wr
def io_rd(s, r):
    if 0xB4 <= r <= 0xB7: return chip.io_rd(r - 0xB4)
    return oldrd(s, r)
def io_wr(s, r, v):
    if 0xB4 <= r <= 0xB7: chip.io_wr(r - 0xB4, v); return
    oldwr(s, r, v & 0xFF)
AppCPU.io_rd, AppCPU.io_wr = io_rd, io_wr

PAGE1 = (b'HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n'
         b'<html><head><title>FrogFind!</title><style>body{color:red}</style>'
         b'<script>if (a<b) document.write("<p>nein</p>");</script></head><body>'
         b'<h1>FrogFind!</h1><p>The Search Engine for Vintage Computers. Gr\xc3\xbc\xc3\x9fe aus K&ouml;ln &amp; '
         b'&#8222;Anf\xc3\xbchrung&#8220; &mdash; fertig.</p>'
         b'<ul><li><a href="/page2">Seite zwei</a></li><li><a href="http://test.local/x?a=1&amp;b=2">Andere</a>'
         b'<li>Punkt <b>fett</b></ul><!-- Kommentar <a href="x"> -->'
         b'<pre>  a  b\n  c</pre><hr><p>Ende</body></html>')
PAGE2 = (b'HTTP/1.0 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: text/html\r\n\r\n'
         b'1a\r\n<title>Zwei</title><p>Seit\r\n'
         b'14\r\ne zwei, chunked.</p>\r\n'
         b'0\r\n\r\n')
REDIR = b'HTTP/1.1 301 Moved\r\nLocation: /page2\r\nContent-Length: 0\r\n\r\n'
LONG = b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n' + b''.join(
    b'<p>Absatz %d mit etwas Text, damit die Zeilen umbrechen muessen und die Seite lang wird.</p>' % i
    for i in range(60))
net.pages[('10.1.1.1', 80, '/')] = PAGE1
net.pages[('10.1.1.1', 80, '/page2')] = PAGE2
net.pages[('10.1.1.1', 80, '/alt')] = REDIR
net.pages[('10.1.1.2', 80, '/x?a=1&b=2')] = LONG

def boot():
    cpu = boot_app(ADDR)
    cpu.hnext = 0x0A0000
    def settle(maxn=40_000_000):
        run(cpu, 30)
        while True:
            if run(cpu, maxn, stop_at=ML) and not cpu.events: return
    return cpu, settle
cpu, settle = boot()
settle()
DPV = {'WIN': 0xB4, 'PAGE': 0x1C}
def w(n):
    if n in DPV: return cpu.rd16(cpu.D + DPV[n])
    return cpu.rd16(ADDR + lbl[n])
def key(c, mods=0):
    cpu.events.append((3, 160, 100, c, mods)); settle(); settle()
def typ(t):
    for ch in t: key(ord(ch))
def click(h, v):
    cpu.events += [(0, h, v, 0)]; settle()
    cpu.events += [(1, h, v, 0), (2, h, v, 0)]; settle(); settle()
def bank(): return w('NWB') << 16
def bstr(off):
    b = bank(); n = cpu.mem[b + off]; return bytes(cpu.mem[b + off + 1:b + off + 1 + n])
WB_TXT, WB_LIN = 0x8000, 0xC800
def text():
    b = bank(); return bytes(cpu.mem[b + WB_TXT:b + w('TXTEND')])
def lines():
    b = bank(); out = []
    n = w('NLINES')
    for i in range(n):
        o = cpu.rd16(b + WB_LIN + 6 * i); e = cpu.rd16(b + WB_LIN + 6 * i + 6) if i + 1 < n else w('TXTEND')
        raw = bytes(cpu.mem[b + o:b + e])
        s = ''; k = 0
        while k < len(raw):
            c = raw[k]
            if c == 1: s += '<'; k += 2 + raw[k + 1]; continue
            if c in (0x18, 0x19): s += '[F%d]' % raw[k + 1]; k += 2; continue
            if c == 0x17: k += 3; continue
            if c in (0x1A, 0x1C, 0x1D): k += 2; continue
            if c == 2: s += '>'
            elif c == 0x14: s += '|'; k += 1
            elif c >= 0x20 and c < 0x7F: s += chr(c)
            elif 0x80 <= c <= 0x86: s += 'äöüÄÖÜß'[c - 0x80]
            elif c == 0x87: s += '•'
            elif c == 0x16: s += '----'
            k += 1
        out.append(s)
    return out
ok = True
def check(cond, msg):
    global ok
    print(('ok  ' if cond else 'FEHLER ') + msg)
    ok &= bool(cond)

# 1) Schnellstart: Internet -> Startseite
click(101, 193)
check(w('WIN') == 21, 'Browserfenster offen (WIN=%d)' % w('WIN'))
check(bstr(0xFC00) == b'GS/OS95 Internet', 'Titel der Startseite: %r' % bstr(0xFC00))
L = lines()
check(any('FrogFind' in l for l in L) and w('NLINES') > 10, 'Startseite hat %d Zeilen' % w('NLINES'))
render(cpu, 'tw1a.png', 3)
print('\n'.join(L[:14]))

# 2) Adresse tippen -> DHCP, DNS, HTTP
typ('frogfind.com'); key(0x0D)
print('Log:', [x for x in net.log if x[0] in ('dhcp', 'dns', 'connect', 'udp')])
check(('dhcp', 1) in net.log and ('dhcp', 3) in net.log, 'DHCP DISCOVER/REQUEST')
check(bytes(cpu.mem[ADDR + lbl['NCUR']:ADDR + lbl['NCUR'] + 16]) == bytes([10,0,2,15,255,255,255,0,10,0,2,2,10,0,2,3]), 'Adressen per DHCP')
check(('dns', 'frogfind.com') in net.log, 'DNS-Anfrage')
req = net.requests[-1][1] if net.requests else b''
print(req.decode())
check(req.startswith(b'GET / HTTP/1.0\r\nHost: frogfind.com\r\n'), 'HTTP-Anfrage')
check(bstr(0xFC00) == b'FrogFind!', 'Titel %r' % bstr(0xFC00))
check(bstr(0xFD80) == b'http://frogfind.com/', 'Adresse %r' % bstr(0xFD80))
L = lines()
print('\n'.join(L))
T = '\n'.join(L)
check('nein' not in T and 'color' not in T, 'script/style ausgeblendet')
check('Grüße aus Köln & "Anführung" - fertig.' in ' '.join(x.strip() for x in L), 'UTF-8, Entitaeten')
check('<Seite zwei>' in T and '<Andere>' in T, 'Links')
check('Kommentar' not in T, 'Kommentar')
check('  a  b' in T and '  c' in T, 'pre')
check('----' in T, 'Trennlinie')
render(cpu, 'tw1b.png', 3)

# 3) Link anklicken (Seite zwei, chunked)
idx = next(i for i, l in enumerate(L) if 'Seite zwei' in l)
y = 6 + 31 + (idx - w('PAGE')) * 10 + 4
x = 8 + 8 + 30
click(x, y)
print('nach Klick:', bstr(0xFD80), lines())
check(bstr(0xFD80) == b'http://frogfind.com/page2', 'Link gefolgt')
check(any('Seite zwei, chunked.' in l for l in lines()), 'chunked')
check(w('HISN') == 2, 'Verlauf %d' % w('HISN'))
# 4) Zurueck mit Pfeil links
key(0x08)
check(bstr(0xFD80) == b'http://frogfind.com/', 'zurueck: %r' % bstr(0xFD80))
check(w('HISN') == 1, 'Verlauf nach Zurueck %d' % w('HISN'))
# 5) Weiterleitung
typ('frogfind.com/alt'); key(0x0D)
check(bstr(0xFD80) == b'http://frogfind.com/page2', 'Weiterleitung: %r' % bstr(0xFD80))
# 6) andere Seite, Entitaet im Link, lange Seite + Blaettern
typ('http://test.local/x?a=1&b=2'); key(0x0D)
check(net.requests[-1][1].startswith(b'GET /x?a=1&b=2 HTTP/1.0\r\nHost: test.local\r\n'), 'Abfrage im Pfad')
n = w('NLINES')
check(n > 60, 'lange Seite: %d Zeilen' % n)
key(ord(' '))
check(w('PAGE') > 0, 'Leertaste blaettert (PAGE=%d)' % w('PAGE'))
render(cpu, 'tw1c.png', 3)
# 7) unbekannter Name -> Fehlerseite
typ('gibtsnicht.example'); key(0x0D)
T = '\n'.join(lines())
print(T)
check('gibtsnicht.example' in T and 'nicht gefunden' in T, 'DNS-Fehlerseite')
render(cpu, 'tw1d.png', 3)
# 8) Suchbegriff -> FrogFind-Suche
net.pages[('10.1.1.1', 80, '/?q=apple+iigs%21')] = PAGE2
typ('apple iigs!'); key(0x0D)
check(net.requests[-1][1].startswith(b'GET /?q=apple+iigs%21 HTTP/1.0'), 'Suche: %r' % net.requests[-1][1][:40])
# 9) HTTPS -> Hinweis
typ('https://example.com'); key(0x0D)
check('HTTPS' in '\n'.join(lines()), 'HTTPS-Hinweis')
# 10) Esc schliesst
key(0x1B)
check(w('NWIN') == 0, 'Fenster zu')
print('OK' if ok else 'FAIL Internet')
