# Minimieren
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
nw=lambda: cpu.rd16(ADDR+lbl['NWIN'])
def recs():
    out=[]
    for i in range(nw()):
        a=ADDR+lbl['wrecs']+i*298
        x=cpu.rd16(a+2); x=x-65536 if x>32767 else x
        out.append((cpu.rd16(a), x, cpu.rd16(a+20)))
    return out
settle()
click(53,193)                       # Arbeitsplatz
key(0x15); key(13)                  # Laufwerk oeffnen (gleiches Fenster)
click(53,193)                       # zweites Fenster
r0=recs(); print('offen:', r0); render(cpu,'mi0.png',3)
top=r0[-1]
# Minimieren-Knopf des obersten Fensters: rechts in der Titelleiste (Fenster bei x, Breite 240)
a=ADDR+lbl['wrecs']+(nw()-1)*298
wx=cpu.rd16(a+2); wy=cpu.rd16(a+4); ww=cpu.rd16(a+22)
click(wx+ww-45+6, wy+5+5)
r1=recs(); print('nach Minimieren:', r1); render(cpu,'mi1.png',3)
ok = r1[0][2]==top[2] and r1[0][1]>=600 and r1[-1][2]==r0[0][2] and r1[-1][1]<600
# Taskleiste: Knopf des minimierten Fensters holt es zurueck
tb=cpu.rd16(ADDR+lbl['TBWV'])
ids=sorted(r[2] for r in r1)       # Knoepfe in Reihenfolge der Fenster-IDs
pos=ids.index(top[2])
click(135+pos*tb+10,193)
r2=recs(); print('nach Taskleiste:', r2); render(cpu,'mi2.png',3)
ok = ok and r2[-1][2]==top[2] and r2[-1][1]==top[1]
# aktives Fenster ueber die Taskleiste minimieren, dann das andere auch
click(135+pos*tb+10,193); r3=recs()
ok = ok and r3[0][2]==top[2] and r3[0][1]>=600
other=ids.index(r3[-1][2]); click(135+other*tb+10,193); r4=recs()
print('beide minimiert:', r4); render(cpu,'mi3.png',3)
ok = ok and all(r[1]>=600 for r in r4)
key(0x15); key(0x0A)                # Tasten gehen an den Schreibtisch, nichts passiert an den Fenstern
ok = ok and recs()==r4
click(135+10,193); r5=recs(); render(cpu,'mi4.png',3)
ok = ok and r5[-1][1]<600
# Desktop anzeigen: alle weg, noch einmal: alle wieder da
click(125,193); r6=recs(); render(cpu,'mi5.png',3)
click(125,193); r7=recs(); render(cpu,'mi6.png',3)
print('Desktop anzeigen:', r6, r7)
a6=[r[1]>=600 for r in r6]; a7=[r[1]>=600 for r in r7]
ok = ok and len(set(a6))==1 and len(set(a7))==1 and a6[0]!=a7[0]
print('OK' if ok else 'FAIL Minimieren')
