import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('A1',0x04),('A2',0x04),('A3',0x04),('ZIEL',0x0F)]
t3.FS[':SYSTEM.DISK:ZIEL'] = []
for n in ('A1','A2','A3'): t3.FILES[':SYSTEM.DISK:'+n] = n.encode()*10
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v,m=0):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0,m),(2,h,v,0,m)]; settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def drag(a,b):
    (h0,v0),(h1,v1)=a,b
    cpu.events += [(0,h0,v0,0)]; settle()
    cpu.events += [(1,h0,v0,0)]+[(0,h0+(h1-h0)*i//8, v0+(v1-v0)*i//8,0) for i in range(1,9)]
    run(cpu,3_000_000); cpu.events += [(2,h1,v1,0)]; settle(); settle()
names=lambda k:[e[0] for e in t3.FS[k]]
settle()
dclick(28,14); key(0x15); key(13)
# Zellen: (108+76c, 49+27r); A1=(108,49) A2=(184,49) A3=(260,49) ZIEL=(108,76)
click(108,49); click(260,49,0x0200)          # A1 + Shift A3
render(cpu,'ms1.png',3)
print('MSELN', cpu.mem[ADDR+lbl['MSELN']])
for _ in range(30): run(cpu, 400_000)
drag((260,49),(108,76))                      # beide auf ZIEL ziehen
print('Wurzel', names(':SYSTEM.DISK'), 'ZIEL', names(':SYSTEM.DISK:ZIEL'))
key(0x01)                                    # Strg+A
render(cpu,'ms2.png',3)
print('Strg+A MSELN', cpu.mem[ADDR+lbl['MSELN']])
# ZIEL oeffnen, beide kopieren, hoch, einfuegen
key(0x1B) if False else None
dclick(184,49)                      # ZIEL
for _ in range(30): run(cpu, 400_000)
key(0x01); key(0x03)                # Strg+A, Strg+C
key(0x7F)                           # Aufwaerts
key(0x16)                           # Strg+V
print('nach Kopieren: Wurzel', names(':SYSTEM.DISK'), 'ZIEL', names(':SYSTEM.DISK:ZIEL'))
key(0x01); key(0x04)                # alles markieren, loeschen
render(cpu,'ms3.png',3)
key(13)
print('nach Loeschen: Wurzel', names(':SYSTEM.DISK'), 'Papierkorb', t3.FS.get(':SYSTEM.DISK:RECYCLED'))
