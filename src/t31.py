import sys, random; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
random.seed(1)
names = sorted(set('F%05d' % random.randint(0, 99999) for _ in range(320)))[:300]
random.shuffle(names)
t3.FS[':SYSTEM.DISK'] = [('VIELE',0x0F)]
t3.FS[':SYSTEM.DISK:VIELE'] = [(n,0x04) for n in names[:290]] + [('ORD%d' % i,0x0F) for i in range(10)]
for i in range(10): t3.FS[':SYSTEM.DISK:VIELE:ORD%d' % i] = []
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'] = bytes([0x00, 0])
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
settle()
dclick(28,14); key(0x15); key(13)          # Arbeitsplatz, Laufwerk
key(0x15); key(13)                          # VIELE
import time
D = cpu.rd16(ADDR + lbl['MYDP'])
cnt = cpu.rd16(D + 0x1A); base = cpu.rd16(D + 0x0C); bk = cpu.rd16(ADDR + lbl['ENTBK'])
got = []
for i in range(cnt):
    e = (bk << 16) + base + i * 48
    n = cpu.mem[e + 2]; got.append((cpu.mem[e], bytes(cpu.mem[e + 3:e + 3 + n]).decode()))
exp = [(0x0F, 'ORD%d' % i) for i in range(10)] + [(4, n) for n in sorted(names[:290])]
print('count', cnt, 'ok', got == exp)
assert cnt == 300 and got == exp, got[:12]
for _ in range(30): key(0x0A)
render(cpu, 'big1.png', 3)
