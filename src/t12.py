import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK:SYSTEM:FONTS']=[('SHASTON.16',0xC8)]
t3.FS[':SYSTEM.DISK:SYSTEM:DESK.ACCS']=[]
t3.FS[':SYSTEM.DISK:SYSTEM:TOOLS']=[]
t3.FILES[':SYSTEM.DISK:SYSTEM:START']=b'S'*700
t3.FILES[':SYSTEM.DISK:SYSTEM:FONTS:SHASTON.16']=b'F'*9000
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
settle()
key(9); key(13); key(13); key(0x15); key(13)        # SYSTEM.DISK
key(0x15); key(0x03)                       # SYSTEM markieren, kopieren
key(13); key(0x16); render(cpu,'c_self.png',3)   # in SYSTEM einfuegen -> Fehler erwartet
key(13)                                    # Meldung schliessen
key(0x7F); key(0x7F)                       # hoch, hoch -> Laufwerke
key(0x15); key(0x15); key(13)              # GSOS95 oeffnen
key(0x16)                                  # einfuegen
render(cpu,'c_paste.png',3)
for k in sorted(t3.FS):
    if k.startswith(':GSOS95'): print(k, t3.FS[k])
print({k:len(v) for k,v in t3.FILES.items() if k.startswith(':GSOS95')})
