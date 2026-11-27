# sechs Ordnerfenster + Taskleiste
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
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
def w(name): a=ADDR+lbl[name]; return cpu.mem[a]|cpu.mem[a+1]<<8
settle()
for i in range(7):
    dclick(28,14)
    print(i, 'NWIN', w('NWIN'), 'TBWV', w('TBWV'), 'TNOWN', w('TNOWN'))
render(cpu,'t37.png',3)
