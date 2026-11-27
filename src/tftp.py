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


from t3 import FILES
net.dns['ftp.apple2.org'] = '10.1.1.2'
BIG = bytes((i * 7 + 3) & 0xFF for i in range(50000))
LIST_ROOT = (b'total 3\r\n'
    b'drwxr-xr-x   2 ftp  ftp      4096 Jan  1  2020 pub\r\n'
    b'lrwxrwxrwx   1 ftp  ftp         3 Jan  1  2020 mirror -> pub\r\n'
    b'-rw-r--r--   1 ftp  ftp      1234 Mar  3 12:00 README.TXT\r\n')
LIST_PUB = (b'01-27-20  10:30AM       <DIR>          Games\r\n'
    b'01-27-20  10:31AM                50000 Total Replay.shk\r\n')
CMDS = []; CTL = {'buf': b''}
def data_sock():
    for i, so in enumerate(chip.sock):
        if so['srv'] == ('10.1.1.2', 2000) and so['st'] == 0x17: return i
def ftp(d):
    if d is None: CTL['buf'] = b''; CTL['cwd'] = '/'; return b'220-Willkommen\r\n220 apple2 FTP\r\n'
    CTL['buf'] += d; out = b''
    while b'\r\n' in CTL['buf']:
        line, CTL['buf'] = CTL['buf'].split(b'\r\n', 1)
        c = line.decode('latin-1'); CMDS.append(c)
        v = c.split(' ', 1); arg = v[1] if len(v) > 1 else ''
        if v[0] == 'USER': out += b'331 Passwort bitte\r\n'
        elif v[0] == 'PASS': out += b'230 Drin\r\n'
        elif v[0] == 'TYPE': out += b'200 ok\r\n'
        elif v[0] == 'CWD':
            if arg.rstrip('/') in ('/pub', '/mirror'): CTL['cwd'] = '/pub'; out += b'250 ok\r\n'
            else: out += b'550 Kein Verzeichnis\r\n'
        elif v[0] == 'PASV': out += b'227 Entering Passive Mode (192,168,0,9,7,208)\r\n'
        elif v[0] in ('LIST', 'RETR'):
            i = data_sock()
            if i is None: out += b'425 keine Daten\r\n'; continue
            if v[0] == 'LIST': data = LIST_ROOT if CTL['cwd'] == '/' else LIST_PUB
            elif arg == '/pub/Total Replay.shk': data = BIG
            else: out += b'550 Datei fehlt\r\n'; chip.sock[i]['closing'] = True; continue
            out += b'150 Daten\r\n'
            chip.sock[i]['closing'] = True; chip.put(i, data)
    return out
net.pages[('10.1.1.2', 21, '*')] = ftp
net.pages[('10.1.1.2', 2000, '*')] = lambda d: b''
click(101, 193)
typ('ftp://ftp.apple2.org/'); key(0x0D)
for k in range(6): settle()
L = lines(); print(L); print(CMDS)
check(CMDS[:5] == ['USER anonymous', 'PASS gs95@', 'TYPE I', 'PASV', 'LIST'], 'Anmeldung und LIST')
check(any('[pub]' in l for l in L) and any('[mirror]' in l for l in L), 'Verzeichnisse')
check(any('README.TXT' in l and '1234 Bytes' in l for l in L), 'Datei mit Groesse')
check(not any('total' in l for l in L), 'total-Zeile weg')
check(w('NSOCK') == 0, 'Sockets zu')
render(cpu, 'tftp_1.png', 3)
# Link in ein Verzeichnis (ohne / getippt)
del CMDS[:]
click(26, 61)                           # [pub]
for k in range(4): settle()
L = lines(); print(L); print(CMDS, bstr(0xFD80))
check('CWD /pub/' in CMDS and CMDS[-1] == 'LIST', 'CWD und LIST im Unterverzeichnis')
check(any('[Games]' in l for l in L) and any('Total Replay.shk' in l and '50000 Bytes' in l for l in L), 'DOS-Liste gelesen')
check(any('[..]' in l for l in L), 'Link eine Ebene hoeher')
render(cpu, 'tftp_2.png', 3)
del CMDS[:]
y = [i for i, l in enumerate(L) if 'Total Replay' in l][0]
click(40, 41 + y * 10)
for k in range(8): settle()
print(CMDS, lines())
check('RETR /pub/Total Replay.shk' in CMDS, 'RETR mit entschluesseltem Namen')
dk = [x for x in FILES if 'DOWNLOADS' in x.upper()]
print(dk, [len(FILES[x]) for x in dk])
check(any(FILES[x] == BIG for x in dk), 'Datei vollstaendig gespeichert')
render(cpu, 'tftp_3.png', 3)
# Verzeichnis ohne Schraegstrich getippt, und ein fehlendes
del CMDS[:]
click(250, 150); click(150, 28); typ('ftp://ftp.apple2.org/mirror'); key(0x0D)
for k in range(4): settle()
print(CMDS, lines()[:3])
check(any('[Games]' in l for l in lines()), 'mirror ohne / als Verzeichnis erkannt')
click(250, 150); click(150, 28); typ('ftp://ftp.apple2.org/fehlt/'); key(0x0D)
for k in range(4): settle()
print(lines())
check(any('550' in l for l in lines()), 'Fehlerseite mit Serverantwort')
check(w('NSOCK') == 0, 'Sockets zu')
render(cpu, 'tftp_4.png', 3)
print('ALLES OK' if ok else 'FEHLER')
