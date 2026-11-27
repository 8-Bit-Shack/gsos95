import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
txt = '\r'.join(['Zeile %02d: Dies ist ein Test des GS/OS95-Editors.' % i for i in range(1,41)]).encode()
t3.FILES[':SYSTEM.DISK:README'] = txt
# SHR-Bild: Farbverlauf
pic = bytearray(32768)
for y in range(200):
    for x in range(160): pic[y*160+x] = ((x//10)%16)*17
for i in range(200): pic[32000+i] = 0
for c in range(16):
    v = (c<<8)|(15-c)
    pic[32256+c*2] = v & 0xFF; pic[32256+c*2+1] = v >> 8
t3.FILES[':SYSTEM.DISK:BILD.PIC'] = bytes(pic)
t3.FS[':SYSTEM.DISK'].append(('BILD.PIC', 0xC1))
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
dclick(28,14)                          # Arbeitsplatz per Doppelklick
render(cpu,'d_mc.png',3)
key(0x15); key(13)                     # SYSTEM.DISK
for c in [0x15]*4: key(c)              # README (Index 3)
key(13); render(cpu,'d_txt.png',3)
key(0x15); key(0x0A); render(cpu,'d_txt2.png',3)   # Seite weiter, Zeile runter
key(0x1B)
for c in [0x15]*12: key(c)             # BILD.PIC ist letzter Eintrag (Index 15)
print('SEL2', cpu.mem[0x800+0x1E]|cpu.mem[0x801+0x1E]<<8)
cpu.events.append((3,160,100,13)); 
run(cpu, 3_000_000)
render(cpu,'d_pic.png',2)
key(0x20); render(cpu,'d_after.png',3)
key(0x7F); key(0x15); key(0x06); key(13)   # hoch, Laufwerk markieren, Datei-Menue, Auswerfen
print('ejected:', getattr(cpu,'ejected',None))
