import sys; sys.path.insert(0,'.')
from t3 import *
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
inv = sorted((v,k) for k,v in lbl.items())
def where(pc):
    off = pc - 0x052340; best=None
    for v,k in inv:
        if v<=off: best=(k,off-v)
    return best
ADDR=0x052340
cpu = boot_app(ADDR)
for i in range(6):
    r = run(cpu, 500000, stop_at=ADDR+lbl['mainloop'])
    print(i, r, hex(cpu.PBR<<16|cpu.PC), where(cpu.PC|cpu.PBR<<16), hex(cpu.S), cpu.toolcalls)
    if r: break
