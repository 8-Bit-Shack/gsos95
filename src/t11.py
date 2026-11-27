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
def drag(h,v,h2,v2):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(0,(h+h2)//2,(v+v2)//2,0),(0,h2,v2,0),(2,h2,v2,0)]; settle(); settle()
rw=lambda a: cpu.mem[a]|cpu.mem[a+1]<<8
settle()
click(28,14); click(28,14)                 # Arbeitsplatz
click(28,50); click(28,50)                 # Papierkorb
click(28,86); click(28,86)                 # Liesmich
render(cpu,'w_three.png',3)
# oberstes Fenster (Liesmich, 3. Fenster) an Titelleiste ziehen
nw=rw(ADDR+lbl['NWIN']); print('Fenster:', nw)
x0=ADDR+lbl['wrecs']; RS=282
pos=[(rw(x0+i*RS+2), rw(x0+i*RS+4), rw(x0+i*RS)) for i in range(nw)]; print('Positionen:', pos)
tx,ty,_=pos[-1]
drag(tx+60,ty+8, tx+60+70, ty+8+30)
render(cpu,'w_drag.png',3)
pos=[(rw(x0+i*RS+2), rw(x0+i*RS+4), rw(x0+i*RS)) for i in range(nw)]; print('nach Ziehen:', pos)
# Taskleiste: erste Schaltflaeche (Arbeitsplatz) nach vorn
click(115,193)
render(cpu,'w_task.png',3)
print('oben:', rw(ADDR+lbl['wrecs']+(nw-1)*RS), 'WIN', rw(0x0800+0xB4))
# Im Arbeitsplatz: Laufwerk oeffnen per Tastatur, Dialog Neuer Ordner, Esc
key(0x15); key(13); key(0x0E); render(cpu,'w_dialog.png',3); key(0x1B)
key(0x1B); key(0x1B); key(0x1B)
print('Fenster am Ende:', rw(ADDR+lbl['NWIN']))
render(cpu,'w_end.png',3)
