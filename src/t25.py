import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
from mkicons import *
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK:ICONS'] = [('TEST.ICONS',0xCA)]
t3.FILES[':SYSTEM.DISK:ICONS:TEST.ICONS'] = iconfile([
    rec('', 'SPIEL.S16', 0xB3, 0, 24, 14, 7),          # rotes Programmsymbol
    rec('SPIEL.S16', '*.XYZ', 0, 0, 20, 12, 10)])       # gruenes Dokumentsymbol
t3.FS[':SYSTEM.DISK:GAMES'] = [('SPIEL.S16',0xB3)]
t3.FS[':SYSTEM.DISK'] = [('GAMES',0x0F),('BRIEF.XYZ',0x06),('NOTIZ',0x04),('ICONS',0x0F)]
t3.FILES[':SYSTEM.DISK:GAMES:SPIEL.S16'] = b'x'*10
t3.FILES[':SYSTEM.DISK:BRIEF.XYZ'] = b'y'*5
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
settle()
print('ICN', cpu.mem[ADDR+lbl['ICN']])
dclick(28,14); key(0x15); key(13)
render(cpu,'ic1.png',3)
key(0x15); key(0x15)       # BRIEF.XYZ
try:
    key(13)
except StopIteration: pass
print('quit', getattr(cpu,'quit',None))
print('message', getattr(cpu,'message',None))
