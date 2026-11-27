import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('ZEBRA.TXT',0x04),('ALPHA',0x0F),('Mittel.Bild',0xC1),('beta.s16',0xB3),('GROSS.DATEI',0x06),('KLEIN',0x04)]
t3.FILES[':SYSTEM.DISK:ZEBRA.TXT'] = b'z'*3000
t3.FILES[':SYSTEM.DISK:MITTEL.BILD'] = b'm'*32768
t3.FILES[':SYSTEM.DISK:BETA.S16'] = b'b'*70000
t3.FILES[':SYSTEM.DISK:GROSS.DATEI'] = b'g'*150000
t3.FILES[':SYSTEM.DISK:KLEIN'] = b'k'*10
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
settle()
dclick(28,14); key(0x15); key(13)
render(cpu,'lv1.png',3)
click(152,26); render(cpu,'lv2.png',3)
click(160,76)                       # Details
key(0x15); key(0x0A); key(0x0A)     # Auswahl, 2x runter
render(cpu,'lv3.png',3)
click(152,26); click(160,100)       # nach Groesse
render(cpu,'lv4.png',3)
print('prefs', t3.FILES.get(':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'))
