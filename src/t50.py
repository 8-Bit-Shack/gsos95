# Minesweeper
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
g=lambda: bytes(cpu.mem[ADDR+lbl['mgrid']:ADDR+lbl['mgrid']+81])
v=lambda n: cpu.rd16(ADDR+lbl[n])
settle()
click(89,193); render(cpu,'m1.png',3)
MFX,MFY=40+70,16+48
click(MFX+4*11+5, MFY+4*11+5); render(cpu,'m2.png',3)
gr=g(); mines=[i for i in range(81) if gr[i]&0x80]; opened=[i for i in range(81) if gr[i]&0x40]
print('Zustand',v('MST'),'Minen',len(mines),'offen',len(opened),'uebrig',v('MLEFT'))
ok = len(mines)==10 and 40 not in mines and len(opened)>=1 and v('MLEFT')==71-len(opened) and not any(gr[i]&0x40 for i in mines)
# Nachbarzahlen pruefen
for i in range(81):
    r,c=divmod(i,9); n=sum(1 for dr in (-1,0,1) for dc in (-1,0,1) if (dr or dc) and 0<=r+dr<9 and 0<=c+dc<9 and (r+dr)*9+c+dc in mines)
    if gr[i]&15!=n: ok=False; print('Zahl falsch bei',i)
# alle sicheren Felder aufdecken -> gewonnen
for i in range(81):
    if i not in mines and not g()[i]&0x40:
        r,c=divmod(i,9); click(MFX+c*11+5, MFY+r*11+5)
render(cpu,'m3.png',3)
print('Zeit', v('MTIME'), 'Bestzeit', v('MBEST'))
ok_best = v('MBEST')>=1
print('nach allen sicheren Feldern: Zustand',v('MST'),'uebrig',v('MLEFT'))
ok = ok and v('MST')==3
# neues Spiel, dann auf eine Mine klicken -> verloren
click(40+70+42+8, 16+26+1+8); st0=v('MST')
click(MFX+5, MFY+5)
gr=g(); mines=[i for i in range(81) if gr[i]&0x80]
r,c=divmod(mines[0],9); click(MFX+c*11+5, MFY+r*11+5); render(cpu,'m4.png',3)
print('neu',st0,'nach Mine',v('MST'))
ok = ok and st0==0 and v('MST')==2
key(0x1B)
ok = ok and ok_best
print('OK' if ok else 'FAIL Minesweeper')
# Minimieren-Knopf
click(89,193)
nw=cpu.rd16(ADDR+lbl['NWIN']); a=ADDR+lbl['wrecs']+(nw-1)*298
wx=cpu.rd16(a+2); wy=cpu.rd16(a+4)
render(cpu,'m5.png',3)
click(wx+240-31+6, wy+5+5)
x2=cpu.rd16(a+2) if cpu.rd16(ADDR+lbl['NWIN'])==nw else None
rx=[cpu.rd16(ADDR+lbl['wrecs']+i*298+2) for i in range(cpu.rd16(ADDR+lbl['NWIN']))]
print('Minesweeper minimiert:', rx)
print('OK' if any(x>=600 for x in rx) else 'FAIL Minimieren Minesweeper')
render(cpu,'m6.png',3)
click(123+20, 193)                 # Taskleisten-Knopf holt es zurueck
rx2=[cpu.rd16(ADDR+lbl['wrecs']+i*298+2) for i in range(cpu.rd16(ADDR+lbl['NWIN']))]
render(cpu,'m7.png',3)
print('nach Taskleiste:', rx2, 'Knoepfe', v('TNOWN'))
print('OK' if all(x<600 for x in rx2) else 'FAIL Minesweeper-Knopf')
