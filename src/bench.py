import sys, os; sys.path.insert(0,'.')
import t3
from t3 import *
from emu import run
d=sys.argv[1]
os.chdir(d); sys.path.insert(0,d)
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x032340; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('ORDNER',0x0F)]+[('DATEI.%02d'%i,4) for i in range(20)]
t3.FS[':SYSTEM.DISK:ORDNER'] = [('X%d'%i,4) for i in range(8)]
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'] = bytes([0x0C, 0])
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def meas(f):
    i0=cpu.icount; f(); return cpu.icount-i0
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
settle()
dclick(28,14); key(0x15)
print('Laufwerk oeffnen', meas(lambda: key(13)))
print('Ordner oeffnen  ', meas(lambda: key(13)))
print('Aufwaerts       ', meas(lambda: key(0x7F)))
print('Seite weiter    ', meas(lambda: [key(0x0A) for _ in range(4)]))
