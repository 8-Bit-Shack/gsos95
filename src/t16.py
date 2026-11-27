import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.DEVS[2] = ('.APPLEDISK3.5B', 'UNF')
t3.FS[':GSOS95'] = [('ALT',0x04)]
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
def click(h,v):
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
settle()
dclick(28,14)
render(cpu,'v1.png',3)
# GSOS95 (Index 2) umbenennen
key(0x15); key(0x15); key(0x15); key(0x05)
for c in b'DATEN': key(c)
key(13)
print('DEVS', t3.DEVS)
render(cpu,'v2.png',3)
# Startlaufwerk loeschen -> verweigert
key(0x15); key(0x15); key(0x15)   # Auswahl zuruecksetzen? 
