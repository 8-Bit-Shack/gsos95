import sys; sys.path.insert(0,'.')
from t3 import *
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x052340; ML = ADDR + lbl['mainloop']
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
settle()
for c in [9, 13]: key(c)            # Tab, Return -> Programme/Arbeitsplatz
key(13)                             # erstes Laufwerk oeffnen
for c in [0x0A, 0x15]: key(c)       # runter, rechts
render(cpu,'k_dir.png',3)
key(0x7F); render(cpu,'k_up.png',3)
key(0x1B)
key(9); key(0x0B); render(cpu,'k_menu.png',3)
key(13); render(cpu,'k_quit.png',3)
try: key(ord('n'))
except StopIteration: pass
print('Ergebnis:', cpu.quit)
