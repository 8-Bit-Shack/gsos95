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
settle()
key(9); key(0x0A); key(0x0A); key(13)
render(cpu,'t35a.png',3)
key(0x0A); key(0x0A)
render(cpu,'t35b.png',3)
key(13)
print('OpenNDA', getattr(cpu,'opened_nda',None))
