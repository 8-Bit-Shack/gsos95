# Verknuepfungen auf dem Desktop
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('ORDNER',0x0F),('BRIEF.TXT',4),('SYSTEM',0x0F)]
t3.FS[':SYSTEM.DISK:ORDNER'] = [('X',4),('Y',4)]
t3.FILES[':SYSTEM.DISK:BRIEF.TXT']=b'hallo welt'
def boot():
    cpu = boot_app(ADDR)
    def settle(maxn=8_000_000):
        run(cpu, 30)
        while True:
            if run(cpu, maxn, stop_at=ML) and not cpu.events: return
    return cpu, settle
cpu, settle = boot()
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v,mod=0):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0,mod),(2,h,v,0,mod)]; settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
w=lambda n: cpu.mem[ADDR+lbl[n]]|cpu.mem[ADDR+lbl[n]+1]<<8
def dp(o):
    v=cpu.mem[cpu.D+o]|cpu.mem[cpu.D+o+1]<<8
    return v-65536 if v>32767 else v
settle()
dclick(28,14); key(0x15); key(13)
def ctx_link(x,y):
    click(x,y,0x0800)
    yy = w('CMY') + dp(0x0A) + 2 + 6*12 + 6
    xx = w('CMX') + dp(0x08) + 20
    click(xx,yy)
ctx_link(104,44)            # ORDNER
ctx_link(176,44)            # BRIEF.TXT
print('LNKN', w('LNKN'), 'Datei', t3.FILES.get(':SYSTEM.DISK:SYSTEM:GSOS95.LNK',b'')[:40])
key(0x1B)
render(cpu,'t40a.png',3)
dclick(20+8,114+8)           # Verknuepfung ORDNER
print('nach Oeffnen: NWIN', w('NWIN'), 'COUNT', dp(0x1A), 'MODE', dp(0x18))
render(cpu,'t40b.png',3)
key(0x1B)
dclick(28,150+8)             # Verknuepfung BRIEF.TXT
print('nach Datei: WIN', dp(0xB4), 'NWIN', w('NWIN'))
render(cpu,'t40c.png',3)
key(0x1B); key(0x1B)
click(28,114+8); key(0x7F)  # ORDNER-Verknuepfung entfernen
print('nach Entfernen LNKN', w('LNKN'))
cpu, settle = boot(); settle()
print('nach Neustart LNKN', w('LNKN'))
render(cpu,'t40d.png',3)
