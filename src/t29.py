import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
from mkicons import *
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('BRIEF.TXT',0x04),('SPIEL.S16',0xB3),('TIEF',0x0F),('ICONS',0x0F)]
t3.FS[':SYSTEM.DISK:TIEF'] = [('NOCH.TIEFER',0x0F)]
t3.FS[':SYSTEM.DISK:TIEF:NOCH.TIEFER'] = [('SCHATZ.BRIEF',0x04)]
t3.FS[':SYSTEM.DISK:ICONS'] = [('T.ICONS',0xCA)]
t3.FILES[':SYSTEM.DISK:ICONS:T.ICONS'] = iconfile([rec('SPIEL.S16', '*.TXT', 4, 0, 16, 10, 9)])
for k in ('BRIEF.TXT','SPIEL.S16'): t3.FILES[':SYSTEM.DISK:'+k] = b'abc'
t3.FILES[':SYSTEM.DISK:TIEF:NOCH.TIEFER:SCHATZ.BRIEF'] = b'gold'
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
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
names=lambda k:[e[0] for e in t3.FS[k]]
def menu(item):   # Datei-Menue, Eintrag item
    click(77,26); click(90,40+12*item)
settle()
dclick(28,14); key(0x15); key(13)
key(0x15)                      # BRIEF.TXT
menu(5)                        # Duplizieren
print('Duplikat:', names(':SYSTEM.DISK'))
menu(7)                        # Suchen...
for c in b'SCHATZ': key(c)
key(13)
render(cpu,'ex1.png',3)
print('Ordner nach Suche:', bytes(cpu.mem[ADDR+lbl['pathbuf']+2:ADDR+lbl['pathbuf']+2+cpu.mem[ADDR+lbl['pathbuf']]]))
menu(8)                        # Weitersuchen -> nichts mehr
render(cpu,'ex2.png',3)
key(13)
# Schreibschutz: Eigenschaften (Strg+P) des Treffers, Kaestchen anklicken
key(0x10); render(cpu,'ex3.png',3)
click(84,112)
render(cpu,'ex4.png',3)
print('ACCESS', {k:hex(v) for k,v in t3.ACCESS.items()})

key(13)
key(0x7F); key(0x7F)                  # zurueck zur Wurzel
key(0x15)                             # BRIEF.TXT
try:
    menu(6)                           # Drucken
except StopIteration: pass
print('quit', getattr(cpu,'quit',None))
m=getattr(cpu,'message',None); print('Nachricht', m and m[2][:30])
