import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('SYSTEM',0x0F),('BRIEF',0x04),('SPIEL.S16',0xB3),('FOTO',0xC0),('BRIEF.AWGS',0x50),
   ('HELVETICA',0xC8),('KLANG',0xD8),('UHR.NDA',0xB8),('MAUS.CDEV',0xC7),('START.INIT',0xB6),
   ('TREIBER',0xBB),('CODE.BIN',0x06),('HALLO.BAS',0xFC),('UNBEKANNT',0x2A)]
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
dclick(28,14)
key(0x15); key(13)
render(cpu,'ty1.png',3)
key(0x15); key(0x15); key(0x15); key(0x10)     # Eigenschaften von FOTO (Ctrl-P)
render(cpu,'ty2.png',3)
