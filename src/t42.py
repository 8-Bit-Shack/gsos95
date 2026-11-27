# Startklang, Bildschirmschoner, Apfel+Tab, Einstellungen
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
def key(c,mod=0):
    cpu.events.append((3,160,100,c,mod)); settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
w=lambda n: cpu.mem[ADDR+lbl[n]]|cpu.mem[ADDR+lbl[n]+1]<<8
def dp(o): return cpu.mem[cpu.D+o]|cpu.mem[cpu.D+o+1]<<8
settle()
fs=getattr(cpu,'ffsound',None)
print('Startklang', hex(fs[0]) if fs else None, fs[1].hex() if fs else '')
# Apfel+Tab
dclick(28,14); dclick(28,14)
ids=lambda: [cpu.mem[ADDR+lbl['wrecs']+i*298+20] for i in range(w('NWIN'))]
print('IDs vorher', ids())
key(9,0x0100)
print('IDs nach Apfel+Tab', ids(), 'MENU', dp(0xB2))
key(0x1B); key(0x1B)
# Schoner: 1 Minute einstellen und Zeit vorspulen
cpu.mem[ADDR+lbl['SSMIN']]=1
cpu.tickofs = 5000
run(cpu, 3_000_000)
render(cpu,'t42a.png',3)
pc=(cpu.PBR<<16|cpu.PC)-ADDR
print('im Schoner:', lbl['saver'] <= pc < lbl['star_new']+400 or 0x300000 <= (cpu.PBR<<16|cpu.PC) < 0x310000)
cpu.events.append((3,160,100,0x20)); settle(); settle()
render(cpu,'t42b.png',3)
