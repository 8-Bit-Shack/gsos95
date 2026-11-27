# Startmenue: Programme (Ins Startmenue) und Dokumente (zuletzt geoeffnet)
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('BRIEF.TXT',4),('PONG.S16',0xB3),('SYSTEM',0x0F)]
t3.FILES[':SYSTEM.DISK:BRIEF.TXT']=b'hallo welt'
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
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
dclick(104,44)                     # BRIEF.TXT oeffnen
print('DOCLN', w('DOCLN'), t3.FILES.get(':SYSTEM.DISK:SYSTEM:GSOS95.DOC',b'')[:30])
key(0x1B)
click(176,44,0x0800)               # Kontextmenue PONG
yy = w('CMY') + dp(0x0A) + 2 + 7*12 + 6
xx = w('CMX') + dp(0x08) + 20
click(xx,yy)                       # Ins Startmenue
print('PRGLN', w('PRGLN'))
key(0x1B)
key(9); key(0x0A); key(0x15)       # Start, Dokumente, rechts
render(cpu,'t41a.png',3)
key(13)
print('Dokument geoeffnet: WIN', dp(0xB4))
render(cpu,'t41b.png',3)
key(0x1B)
key(9); key(0x15)                  # Programme
render(cpu,'t41c.png',3)
try: key(13)
except StopIteration: pass
print('quit', getattr(cpu,'quit',None))
