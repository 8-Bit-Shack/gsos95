# Netzwerk-Dialog (Systemsteuerung > Netzwerk), feste Adressen, Testen, keine Karte
import sys; sys.path.insert(0, '.')
from t3 import *
import t3
from emu import run, render
from w5100sim import W5100, Net

lbl = {l.split()[2].lstrip('.'): int(l.split()[1], 16) for l in open('app.lbl')}
ADDR = 0x030400; ML = ADDR + lbl['mainloop']
net = Net(); net.dhcp = False
chip = W5100(net)
oldrd, oldwr = AppCPU.io_rd, AppCPU.io_wr
def io_rd(s, r):
    if 0xB4 <= r <= 0xB7: return chip.io_rd(r - 0xB4)
    return oldrd(s, r)
def io_wr(s, r, v):
    if 0xB4 <= r <= 0xB7: chip.io_wr(r - 0xB4, v); return
    oldwr(s, r, v & 0xFF)
AppCPU.io_rd, AppCPU.io_wr = io_rd, io_wr
net.pages[('10.1.1.1', 80, '/')] = b'HTTP/1.0 200 OK\r\n\r\n<title>T</title>Hallo'
NETF = ':SYSTEM.DISK:SYSTEM:GSOS95.NET'

def boot():
    cpu = boot_app(ADDR)
    cpu.hnext = 0x0A0000
    def settle(maxn=60_000_000):
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
def dclick(h, v):
    cpu.events += [(0, h, v, 0)]; settle()
    cpu.events += [(1, h, v, 0), (2, h, v, 0)]; settle()
    cpu.events += [(1, h, v, 0), (2, h, v, 0)]; settle(); settle()
def fld(i):
    a = ADDR + lbl['DFLD'] + 16 * i; return bytes(cpu.mem[a + 1:a + 1 + cpu.mem[a]]).decode()
ok = True
def check(cond, msg):
    global ok
    print(('ok  ' if cond else 'FEHLER ') + msg); ok &= bool(cond)

# Systemsteuerung: 7 Eintraege, Netzwerk unten links
click(65, 193)
check(w('WIN') == 6, 'Systemsteuerung (WIN=%d)' % w('WIN'))
render(cpu, 'tw2a.png', 3)
dclick(40 + 12 + 30, 16 + 32 + 72 + 8)
check(w('WIN') == 22, 'Netzwerk-Dialog (WIN=%d)' % w('WIN'))
check(w('DSLOT') == 3 and w('DDHCP') == 1, 'Vorgaben Slot 3, DHCP')
check([fld(i) for i in range(4)] == ['192.168.1.50', '255.255.255.0', '192.168.1.1', '1.1.1.1'], 'Felder %r' % [fld(i) for i in range(4)])
render(cpu, 'tw2b.png', 3)
# DHCP aus, IP aendern
click(40 + 14, 6 + 46)
check(w('DDHCP') == 0, 'DHCP aus')
click(40 + 112 + 10, 6 + 56 + 6)
for _ in range(2): key(0x7F)
typ('99')
check(fld(0) == '192.168.1.99', 'IP getippt: %s' % fld(0))
key(0x0A)                               # naechstes Feld
check(w('DFOC') == 1, 'Pfeil runter: Feld 2')
# Gateway falsch -> Meldung
click(40 + 112 + 10, 6 + 56 + 28 + 6)
key(0x7F); key(0x7F); typ('.')
check(fld(2) == '192.168.1.', 'Gateway kaputt: %s' % fld(2))
click(40 + 240 - 136 + 30, 6 + 180 - 24 + 7)   # OK
check(w('WIN') == 22 and w('DFOC') == 2, 'ungueltige Adresse bemerkt')
render(cpu, 'tw2c.png', 3)
typ('1')
# Testen: Karte, feste Adresse, DNS (Ersatz 1.1.1.1)
click(40 + 8 + 30, 6 + 180 - 24 + 7)
check(('dns', 'frogfind.com') in net.log, 'DNS-Probe')
check(not any(x[0] == 'dhcp' for x in net.log), 'kein DHCP')
check(bytes(cpu.mem[ADDR + lbl['NCUR']:ADDR + lbl['NCUR'] + 4]) == bytes([192, 168, 1, 99]), 'feste IP aktiv')
check(w('DSM1') == lbl['ws_cardok'] and w('DSM2') == lbl['ws_dnsok'], 'Meldungen Karte/DNS')
render(cpu, 'tw2d.png', 3)
# OK speichert GSOS95.NET
click(40 + 240 - 136 + 30, 6 + 180 - 24 + 7)
f = t3.FILES.get(NETF, b'')
print('Datei:', f[:28].hex())
check(f[:4] == b'N1\x03\x00' and f[4:8] == bytes([192, 168, 1, 99]) and f[12:16] == bytes([192, 168, 1, 1]), 'GSOS95.NET gespeichert')
check(f[20:23] == b'\x00\x08\xdc', 'MAC 00:08:DC:..')
check(w('WIN') == 6, 'zurueck in der Systemsteuerung')
key(0x1B)
# Slot ohne Karte
dclick(40 + 12 + 30, 16 + 32 + 72 + 8) if w('WIN') == 6 else None
click(65, 193); dclick(40 + 12 + 30, 16 + 32 + 72 + 8)
click(40 + 142 + 7, 6 + 24 + 6)                                        # Karte: Uthernet II
click(40 + 216 + 5, 6 + 24 + 5); click(40 + 216 + 5, 6 + 24 + 5)    # Slot 5
check(w('DSLOT') == 5, 'Slot 5')
click(40 + 8 + 30, 6 + 180 - 24 + 7)
check(w('DSM1') == lbl['ws_nocard'], 'keine Karte in Slot 5')
render(cpu, 'tw2e.png', 3)
click(40 + 240 - 136 + 30, 6 + 180 - 24 + 7)
key(0x1B)
# Browser ohne Karte -> Fehlerseite
click(101, 193)
typ('frogfind.com'); key(0x0D)
b = w('NWB') << 16
txt = bytes(cpu.mem[b + 0x8000:b + w('TXTEND')])
print(txt)
check(b'Keine Netzwerkkarte' in txt, 'Fehlerseite ohne Karte')
print('OK' if ok else 'FAIL Netzwerk')
