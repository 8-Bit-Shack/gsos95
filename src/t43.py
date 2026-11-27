# Messung: Befehle fuer typische Neuzeichnungen
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('ORDNER',0x0F)]+[('DATEI.%02d'%i,4) for i in range(20)]
t3.FS[':SYSTEM.DISK:ORDNER'] = [('X%d'%i,4) for i in range(12)]
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
def drag(h0,v0,h1,v1):
    cpu.events += [(0,h0,v0,0)]; settle()
    cpu.events += [(1,h0,v0,0)]+[(0,h1,v1,0)]+[(2,h1,v1,0)]
    settle(); settle()
def call(name):
    PBR,PC=cpu.PBR,cpu.PC
    ret=ADDR+lbl['code_end']
    cpu.push16((ret-1)&0xFFFF)
    cpu.PC=(ADDR+lbl[name])&0xFFFF
    run(cpu, 80_000_000, stop_at=ret)
    cpu.PBR,cpu.PC=PBR,PC
def meas(tag, f):
    a=cpu.icount; f(); b=cpu.icount
    print('%-22s %8d Befehle  ~%.2f s' % (tag, b-a, (b-a)/800000))
settle()
dclick(28,14); key(0x15); key(13)
meas('redraw_all', lambda: call('redraw_all'))
meas('redraw_win', lambda: call('redraw_win'))
meas('Ordner oeffnen', lambda: key(13))
meas('Aufwaerts', lambda: key(0x7F))
meas('Auswahl +1', lambda: key(0x15))
meas('Fenster ziehen', lambda: drag(150,10,180,40))
meas('Startmenue', lambda: key(9))
meas('Startmenue zu', lambda: key(0x1B))
