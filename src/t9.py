import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x052340; ML = ADDR + lbl['mainloop']
def boot():
    global cpu
    cpu = boot_app(ADDR); settle()
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
boot()
key(9); key(0x0A); key(0x0A); key(13)        # Einstellungen
render(cpu,'l_set_de.png',3)
key(ord('l'))                                 # Englisch
render(cpu,'l_set_en.png',3)
print('Prefs:', t3.FILES)
key(0x1B); key(9); render(cpu,'l_menu_en.png',3)
key(13); key(0x15); key(13)                   # Programs -> erstes Laufwerk
n0=cpu.icount; key(0x15); key(0x0A); print('Instr. fuer 2 Auswahlschritte:', cpu.icount-n0)
render(cpu,'l_dir_en.png',3)
boot()                                        # Neustart: Sprache bleibt Englisch?
render(cpu,'l_boot2.png',3)
key(0x1B)
