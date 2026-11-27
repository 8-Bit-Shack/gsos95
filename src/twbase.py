# Gemeinsamer Rahmen fuer die Netzwerk-Tests ohne W5100 (Marinetti, FujiNet, WLAN-Modem)
import sys; sys.path.insert(0, '.')
from t3 import *
import t3
from emu import run, render
from w5100sim import Net

lbl = {l.split()[2].lstrip('.'): int(l.split()[1], 16) for l in open('app.lbl')}
ADDR = 0x030400; ML = ADDR + lbl['mainloop']
net = Net()
H = b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n'
net.pages[('10.1.1.1', 80, '/')] = H + b'<title>FrogFind!</title><p>Hallo ' + b'Welt ' * 400 + b'</p><a href="/zwei">zwei</a>'
net.pages[('10.1.1.1', 80, '/zwei')] = H + b'<title>Zwei</title><p>Seite zwei.</p>'
net.pages[('10.1.1.1', 80, '/redir')] = b'HTTP/1.0 301 Moved\r\nLocation: /zwei\r\n\r\n'
net.dns['secure.test'] = '10.1.1.5'
net.pages[('10.1.1.5', 443, '/')] = b'<title>Sicher</title><p>Verschluesselt geladen.</p><a href="/zwei">zwei</a>'
net.pages[('10.1.1.5', 443, '/zwei')] = b'<title>Zwei</title>ok'
net.dns['bbs.example'] = '10.1.1.20'
TIN = []
def bbs(d):
    if d is None: return b'Willkommen in der Mailbox\r\nlogin: '
    TIN.append(d)
    line = b''.join(TIN)
    if d.endswith(b'\r'): return b'Hallo ' + line.rstrip(b'\r\x00').split(b'\r')[-1].replace(b'\x00', b'') + b'!\r\n> '
    return b''
net.pages[('10.1.1.20', 23, '*')] = bbs
# Time-Dienst (RFC 868, TCP 37): 4 Bytes Sekunden seit 1900, dann zu
import struct as _st
net.dns['time.nist.gov'] = '10.9.9.37'
TIMEQ = []
def timesrv(d):
    TIMEQ.append(d)
    if d is None: return (_st.pack('>I', net.ntp + 2208988800), 'zu')
    return b''
net.pages[('10.9.9.37', 37, '*')] = timesrv
cpu = None
def boot():
    global cpu
    cpu = boot_app(ADDR)
    cpu.hnext = 0x0A0000
    return cpu
def settle(maxn=60_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
DPV = {'WIN': 0xB4, 'PAGE': 0x1C}
def w(n):
    if n in DPV: return cpu.rd16(cpu.D + DPV[n])
    return cpu.rd16(ADDR + lbl[n])
def setw(n, v): cpu.wr16(ADDR + lbl[n], v)
def key(c, mods=0): cpu.events.append((3, 300, 20, c, mods)); settle(); settle()
def typ(t):
    for ch in t: key(ord(ch))
def click(h, v):
    cpu.events += [(0, h, v, 0)]; settle()
    cpu.events += [(1, h, v, 0), (2, h, v, 0)]; settle(); settle()
def dclick(h, v):
    cpu.events += [(0, h, v, 0)]; settle()
    cpu.events += [(1, h, v, 0), (2, h, v, 0)]; settle()
    cpu.events += [(1, h, v, 0), (2, h, v, 0)]; settle(); settle()
def bstr(off):
    b = w('NWB') << 16; n = cpu.mem[b + off]; return bytes(cpu.mem[b + off + 1:b + off + 1 + n])
def txt():
    b = w('NWB') << 16; return bytes(cpu.mem[b + 0x8000:b + w('TXTEND')])
ok = True
def check(cond, msg):
    global ok
    print(('ok  ' if cond else 'FEHLER ') + msg); ok &= bool(cond)
def idle(n=12):
    for i in range(n): settle()
def term_rows(n=3):
    imb = w('IMBK') << 16
    return [bytes(cpu.mem[imb + 160 * r + 2 * i] for i in range(80)).decode('cp437') for r in range(n)]
