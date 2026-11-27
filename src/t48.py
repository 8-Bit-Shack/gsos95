# Schnellstart-Symbole in der Taskleiste
import sys; sys.path.insert(0,'.')
from t3 import *
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def key(c): cpu.events.append((3,160,100,c)); settle(); settle()
WINA=0xB4; dpb=cpu.rd16(ADDR+lbl['MYDP'])
def win(): return cpu.rd16(dpb+WINA) if WINA<0x100 else cpu.rd16(ADDR+WINA)
nw=lambda: cpu.rd16(ADDR+lbl['NWIN']) if lbl['NWIN']>0xFF else cpu.rd16(dpb+lbl['NWIN'])
settle(); n0=nw()
click(53,193); n1=nw(); render(cpu,'q1.png',3)
click(65,193); n2=nw(); w2=win()
key(0x1B)
click(77,193); n3=nw(); w3=win(); render(cpu,'q3.png',3)
print('Fenster:', n0,n1,n2,n3,'Art', w2, w3)
print('OK' if n1==n0+1 and n2==n1+1 and n3==n2 else 'FAIL Schnellstart')
key(0x1B); n4=nw()
click(300,193); n5=nw(); render(cpu,'q5.png',3)
print('Uhr-Klick:', n4, n5, 'OK' if n5==n4+1 else 'FAIL Uhr')
