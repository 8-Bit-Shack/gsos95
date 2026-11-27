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
def ls(p): return [n for n,_ in t3.FS.get(p,[])]
settle()
key(9); key(13); key(13); key(0x15); key(13)            # Programme -> SYSTEM.DISK
click(77,22);         # Datei-Menue
cpu.events += [(0,82,56,0)]; settle(); 
key(0x1B)
# README (Index 3) waehlen und kopieren
for c in [0x15,0x15,0x15,0x15]: key(c)
key(0x03)                                       # Ctrl-C
key(0x15); key(0x15); key(0x15)                 # -> nichts; zurueck zu SYSTEM (0)
for c in [0x08]*6: key(c)
key(13)                                         # SYSTEM oeffnen
key(0x16)                                       # Ctrl-V
print('SYSTEM:', ls(':SYSTEM.DISK:SYSTEM'), 'Datei:', len(t3.FILES.get(':SYSTEM.DISK:SYSTEM:README','') ))

key(0x0E);             # Ctrl-N
for ch in b'Spiele': key(ch)
key(13)
print('nach neuer Ordner:', ls(':SYSTEM.DISK:SYSTEM'))
# README (jetzt?) markieren und loeschen
names = ls(':SYSTEM.DISK:SYSTEM'); i = names.index('README')
key(0x15)
for _ in range(i): key(0x15)
key(0x04);           # Ctrl-D
key(ord('j'))
print('SYSTEM:', ls(':SYSTEM.DISK:SYSTEM'), 'BIN:', ls(':SYSTEM.DISK:RECYCLED'))

# Papierkorb oeffnen, Eintrag waehlen, Wiederherstellen (Strg+E)
print('Liste:', [k for k in t3.FILES if 'GSOS95.BIN' in k])
click(28,50); click(28,50)
key(0x15); render(cpu,'r_bin.png',3)
key(0x06); render(cpu,'r_menu.png',3); key(0x1B)
key(0x05)
render(cpu,'r_after.png',3)
a=ls(':SYSTEM.DISK:SYSTEM'); b=ls(':SYSTEM.DISK:RECYCLED')
print('nach Wiederherstellen SYSTEM:', a, 'BIN:', b)
print('OK' if 'README' in a and 'README' not in b else 'FAIL Wiederherstellen')
