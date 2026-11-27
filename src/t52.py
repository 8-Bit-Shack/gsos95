# Hintergrundmuster: nur Schreibtischpunkte aendern sich, Fenster/Symbole bleiben
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
m=cpu.mem
px=lambda x,y: (m[0xE12000+y*160+x//2]>>(0 if x&1 else 4))&15
def redraw(): click(77,193); key(0x1B)
settle(); redraw()
base=[[px(x,y) for x in range(320)] for y in range(200)]
ok=True
want={1:lambda x,y: y%4==0 and x%4==0, 2:lambda x,y: ((x//2)+(y//2))%2==0,
      3:lambda x,y: (x-y)%8==0, 4:lambda x,y: y%8==0 or (x%16==0 if y%16<8 else x%16==8)}
for p in (1,2,3,4):
    cpu.wr16(ADDR+lbl['DESKPAT'],p) if hasattr(cpu,'wr16') else None
    m[ADDR+lbl['DESKPAT']]=p; m[ADDR+lbl['DESKPAT']+1]=0
    redraw()
    render(cpu,'pat%d.png'%p,3)
    bad=0
    for y in range(186):
        for x in range(250,320):
            v=px(x,y)
            if (v==0)!=want[p](x,y) or v not in (0,12): bad+=1
    key(0x1B) if False else None
    print('Muster',p,'falsche Punkte:',bad)
    ok = ok and bad == 0
print('OK' if ok else 'FAIL Muster')
