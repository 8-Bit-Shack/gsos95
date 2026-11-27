# Einfuegen mit Fortschritt; Abbrechen mit Escape
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
def setup():
    t3.FS[':SYSTEM.DISK'] = [('QUELLE',0x0F),('ZIEL',0x0F)]
    t3.FS[':SYSTEM.DISK:QUELLE'] = [('A%d'%i,4) for i in range(3)]
    t3.FS[':SYSTEM.DISK:ZIEL'] = []
    for i in range(3): t3.FILES[':SYSTEM.DISK:QUELLE:A%d'%i] = bytes(20000)
setup()
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
w=lambda n: cpu.mem[ADDR+lbl[n]]|cpu.mem[ADDR+lbl[n]+1]<<8
names=lambda k:[e[0] for e in t3.FS.get(k,[])]
settle()
dclick(28,14); key(0x15); key(13)          # SYSTEM.DISK
key(0x15); key(13)                         # QUELLE
key(0x01); key(0x03)                       # alles, kopieren
key(0x7F); key(0x15); key(0x15); key(13)   # hoch, ZIEL
# Einfuegen + sofort Escape
cpu.events += [(3,160,100,0x16),(3,160,100,0x1B)]; settle(); settle()
print('abgebrochen: ZIEL', names(':SYSTEM.DISK:ZIEL'), 'PRGI', w('PRGI'), 'XABORT', w('XABORT'), 'NWIN', w('NWIN'))
render(cpu,'t38a.png',3)
key(0x16)                                  # Einfuegen ohne Abbruch
print('fertig: ZIEL', names(':SYSTEM.DISK:ZIEL'), 'PRGI', w('PRGI'), 'PRGN', w('PRGN'), 'NWIN', w('NWIN'))
render(cpu,'t38b.png',3)
t3.FS[':SYSTEM.DISK:ZIEL'] = []
for k in list(t3.FILES):
    if k.startswith(':SYSTEM.DISK:ZIEL:'): del t3.FILES[k]
key(0x7F)
cpu.events += [(3,160,100,0x16)]
run(cpu, 30)
run(cpu, 80_000_000, stop_at=ADDR+lbl['prog_step']); run(cpu,50)
run(cpu, 80_000_000, stop_at=ADDR+lbl['prog_step']); run(cpu,20000)
render(cpu,'t38c.png',3)
run(cpu, 80_000_000, stop_at=ADDR+lbl['prog_step'])
render(cpu,'t38d.png',3)
a=ADDR+lbl['prgname']; print('prgname', bytes(cpu.mem[a:a+8]), 'NL', w('PRGNL'), 'src', bytes(cpu.mem[ADDR+lbl['srcpath']:ADDR+lbl['srcpath']+30]))
