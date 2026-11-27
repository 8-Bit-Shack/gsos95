import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK:SYSTEM'] = [('FONTS',0x0F)]
t3.FILES[':SYSTEM.DISK:SYSTEM:BLA'] = b'x'
t3.FS[':SYSTEM.DISK:SYSTEM'] = [('FONTS',0x0F),('BLA',0x04)]
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
names=lambda k:[e[0] for e in t3.FS[k]]
settle()
dclick(28,14); key(0x15); key(13)      # SYSTEM.DISK
key(0x15); key(13)                     # Ordner SYSTEM
render(cpu,'h1.png',3)
key(0x15); key(0x15)                   # BLA auswaehlen
click(120,26); render(cpu,'h2.png',3)
click(128,40)
print('clipok', cpu.mem[ADDR+lbl['clipok']], 'clipcut', cpu.mem[ADDR+lbl['clipcut']])
click(268,26)                          # Aufwaerts
click(120,26); render(cpu,'h3.png',3)
click(128,64)
print('Wurzel:', names(':SYSTEM.DISK'), ' SYSTEM:', names(':SYSTEM.DISK:SYSTEM'))
render(cpu,'h4.png',3)
