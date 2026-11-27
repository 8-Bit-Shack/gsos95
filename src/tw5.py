# CoGS-Karte in Slot 3: Erkennung, HTTP, HTTPS ueber die Karte, Zertifikatsfehler, Dialog
import sys; sys.path.insert(0, '.')
from t3 import *
import t3
from emu import run, render
from w5100sim import Net
from cogssim import CoGS
lbl = {l.split()[2].lstrip('.'): int(l.split()[1], 16) for l in open('app.lbl')}
ADDR = 0x030400; ML = ADDR + lbl['mainloop']
net = Net(); card = CoGS(net)
net.dns['secure.test'] = '10.1.1.5'; net.dns['bad.test'] = '10.1.1.6'
oldrd, oldwr = AppCPU.io_rd, AppCPU.io_wr
def io_rd(s, r):
    if 0xB0 <= r <= 0xBF: return card.io_rd(r - 0xB0)
    return oldrd(s, r)
def io_wr(s, r, v):
    if 0xB0 <= r <= 0xBF: card.io_wr(r - 0xB0, v); return
    oldwr(s, r, v & 0xFF)
AppCPU.io_rd, AppCPU.io_wr = io_rd, io_wr
H = b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n'
net.pages[('10.1.1.1', 80, '/')] = H + b'<title>FrogFind!</title><p>Hallo ' + b'Welt ' * 400 + b'</p>'
net.pages[('10.1.1.5', 443, '/')] = H + b'<title>Sicher</title><p>Verschluesselt geladen.</p><a href="/zwei">zwei</a>'
net.pages[('10.1.1.5', 443, '/zwei')] = H + b'<title>Zwei</title>ok'
net.pages[('10.1.1.6', 443, '/')] = H + b'<title>x</title>'
net.pages[('10.1.1.1', 80, '/redir')] = b'HTTP/1.0 301 Moved\r\nLocation: https://secure.test/\r\n\r\n'
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
click(101, 193)
typ('frogfind.com'); key(0x0D)
check(w('NCARD') == 2, 'CoGS erkannt (NCARD=%d)' % w('NCARD'))
check(bstr(0xFC00) == b'FrogFind!' and txt().count(b'Welt') == 400, 'HTTP ueber CoGS (mehrere DATA-Rahmen)')
check(('cconnect', 'frogfind.com', 80, 0) in net.log and not any(x[0] == 'udp' for x in net.log), 'Karte loest den Namen selbst auf')
typ('https://secure.test'); key(0x0D)
check(bstr(0xFC00) == b'Sicher' and ('cconnect', 'secure.test', 443, 1) in net.log, 'HTTPS ueber die Karte')
render(cpu, 'tw5a.png', 3)
b = w('NWB') << 16
L = txt()
# Link auf der HTTPS-Seite (relativ, bleibt https)
key(0x0A); key(0x0B)
idx = 2
click(cpu.rd16(ADDR + lbl['wrecs'] + 2) + 14, cpu.rd16(ADDR + lbl['wrecs'] + 4) + 31 + 10 * idx + 4)
check(bstr(0xFD80) == b'https://secure.test/zwei', 'relativer Link bleibt https: %r' % bstr(0xFD80))
typ('frogfind.com/redir'); key(0x0D)
check(bstr(0xFC00) == b'Sicher', 'Weiterleitung http -> https')
card.certbad.add('bad.test')
typ('https://bad.test'); key(0x0D)
check(b'Zertifikat' in txt() and b'bad.test' in txt(), 'Zertifikatsfehler: %r' % txt()[:80])
typ('gibtsnicht.test'); key(0x0D)
check(b'nicht gefunden' in txt(), 'DNS-Fehler ueber die Karte')
key(0x1B)
# Dialog: Testen zeigt CoGS
click(65, 193); dclick(40 + 12 + 30, 16 + 32 + 72 + 8)
click(40 + 8 + 30, 6 + 180 - 24 + 7)
check(w('DSM1') == lbl['ws_cogsok'] and w('DSM1D') == 3, 'Dialog: CoGS bereit (WLAN)')
check(w('DSM2') == lbl['ws_dnsok'], 'Dialog: DNS-Probe ueber die Karte')
render(cpu, 'tw5b.png', 3)
print('OK' if ok else 'FAIL CoGS')
