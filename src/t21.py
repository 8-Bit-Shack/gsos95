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
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
settle()
click(12,193)                      # Start
render(cpu,'z1.png',3)
click(60,96)
render(cpu,'z2.png',3)
print('Tools:', [hex(x) for x in getattr(cpu,'desklog',[])][:40])
click(80,16+40+3+12+4)             # zweites Zubehoer (Calculator)
print('OpenNDA', getattr(cpu,'opened_nda',None))
click(250,50)                      # Klick ins NDA-Fenster
print('SystemClick', getattr(cpu,'sysclicks',0), 'RefreshDesktop', cpu.desklog.count(0x390E))
render(cpu,'z3.png',3)
click(12,193); click(12,193)
render(cpu,'z4.png',3)
click(115,193)                     # Zubehoer-Knopf in der Taskleiste
print('SelectWindow', getattr(cpu,'selected',0))
