import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('SPIELE',0x0F),('X',0x04)]
t3.FS[':SYSTEM.DISK:SPIELE'] = [('PONG.S16',0xB3),('TETRIS.S16',0xB3)]
t3.FILES[':SYSTEM.DISK:SPIELE:PONG.S16'] = b'p'
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
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
settle()
dclick(28,14); key(0x15); key(13); key(0x15); key(13)    # SYSTEM.DISK -> SPIELE
key(0x15)
try: key(13)                                            # PONG starten
except StopIteration: pass
print('quit', cpu.quit)
print('WIN-Datei', t3.FILES.get(':SYSTEM.DISK:SYSTEM:GSOS95.WIN'))
cpu, settle = boot(); settle()
render(cpu,'wr1.png',3)
