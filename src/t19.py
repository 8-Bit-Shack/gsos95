import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FILES[':SYSTEM.DISK:DATEI.01'] = b'Hallo Welt\r'
t3.FILES[':SYSTEM.DISK:DATEI.03'] = b'Kopie\r'
t3.FS[':GSOS95'] = [('ALT',0x04)]
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
def drag(a,b,shot=None):
    (h0,v0),(h1,v1)=a,b
    cpu.events += [(0,h0,v0,0)]; settle()
    ev=[(1,h0,v0,0)]
    for i in range(1,9):
        ev.append((0,h0+(h1-h0)*i//8, v0+(v1-v0)*i//8,0))
    cpu.events += ev
    run(cpu, 3_000_000)
    if shot: render(cpu, shot, 3)
    cpu.events += [(2,h1,v1,0)]; settle(); settle()
def names(k): return [e[0] for e in t3.FS[k]]
settle()
dclick(28,14); key(0x15); key(13)
print(names(':SYSTEM.DISK'))
drag((260,76),(108,49),'g1.png')
print('SYSTEM:', names(':SYSTEM.DISK:SYSTEM'), 'DATEI.01' in names(':SYSTEM.DISK'))
render(cpu,'g2.png',3)
# DATEI.02 steht jetzt an Index 5 -> auf den Papierkorb
drag((260,76),(28,57))
print('RECYCLED:', t3.FS.get(':SYSTEM.DISK:RECYCLED'))
# zweites Fenster mit GSOS95
dclick(28,14)
render(cpu,'g3.png',3)
key(0x15); key(0x15); key(0x15); key(13)
render(cpu,'g4.png',3)
print('NWIN', cpu.mem[ADDR+lbl['NWIN']])
t3.FILES[':GSOS95:ALT'] = b'alte Datei\r'
drag((116,57),(65,100),'g6.png')
print('SYSTEM.DISK hat ALT:', 'ALT' in names(':SYSTEM.DISK'), ' GSOS95:', names(':GSOS95'), t3.FILES.get(':SYSTEM.DISK:ALT'))
render(cpu,'g7.png',3)
