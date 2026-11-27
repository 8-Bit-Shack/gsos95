# Ordnerbaum im Explorer
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
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def key(c): cpu.events.append((3,160,100,c)); settle(); settle()
def path():
    a=ADDR+lbl['pathbuf']; n=cpu.rd16(a); return bytes(cpu.mem[a+2:a+2+n]).decode('latin-1')
dpb=cpu.rd16(ADDR+lbl['MYDP'])
settle()
click(53,193)                       # Schnellstart: Arbeitsplatz
render(cpu,'tr1.png',3)
key(0x15); key(13)                  # erstes Laufwerk oeffnen
p1=path(); render(cpu,'tr2.png',3)
key(0x15); key(13)                  # ersten Ordner oeffnen
p2=path(); render(cpu,'tr3.png',3)
print('Pfade:', repr(p1), repr(p2))
# Baum: Zeile 1 = Laufwerk -> zurueck zur Laufwerkswurzel
click(62+30, 5+32+9+4)
p3=path(); render(cpu,'tr4.png',3)
# Zeile 2 = erster Unterordner
click(62+34, 5+32+18+4)
p4=path()
# Zeile 0 = Arbeitsplatz
click(62+30, 5+32+4)
mode=cpu.rd16(dpb+0x00) if False else None
render(cpu,'tr5.png',3)
print('nach Baumklicks:', repr(p3), repr(p4))
print('OK' if p3==p1 and p4==p2 and p2!=p1 else 'FAIL Baum')
# Menue Ansicht > Ordnerbaum schaltet die Spalte ab
click(160,27); render(cpu,'tr6.png',3)
tw0=cpu.rd16(ADDR+lbl['TW'])
click(190,88); render(cpu,'tr7.png',3)
print('TW vorher', tw0, 'TREEOFF', cpu.rd16(ADDR+lbl['TREEOFF']), 'TW', cpu.rd16(ADDR+lbl['TW']))
print('OK' if cpu.rd16(ADDR+lbl['TREEOFF'])==1 else 'FAIL Menue')
