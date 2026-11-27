# Kontextmenue mit Wahltaste-Klick
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('ORDNER',0x0F),('BRIEF',4)]
t3.FS[':SYSTEM.DISK:ORDNER'] = [('X',4)]
t3.FILES[':SYSTEM.DISK:BRIEF']=b'hallo'
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v,mod=0):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0,mod),(2,h,v,0,mod)]; settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
w=lambda n: cpu.mem[ADDR+lbl[n]]|cpu.mem[ADDR+lbl[n]+1]<<8
def dp(o):
    v=cpu.mem[cpu.D+o]|cpu.mem[cpu.D+o+1]<<8
    return v-65536 if v>32767 else v
names=lambda k:[e[0] for e in t3.FS.get(k,[])]
settle()
dclick(28,14); key(0x15); key(13)
# Wahltaste-Klick auf BRIEF (2. Zelle: Mitte x=62+6+36+72=176, y=4+32+8)
click(176,44,0x0800)
print('DROP', dp(0x64), 'SEL2', dp(0x1E), 'CMX', w('CMX'), 'CMY', w('CMY'))
render(cpu,'t39a.png',3)
cpu_before=bytes(cpu.mem[0xE12000:0xE12000+32000])
# "Umbenennen" = 5. Eintrag: y = CMY+DYO+2+4*12+6
y = w('CMY') + (dp(0x0A)) + 2 + 4*12 + 6
x = w('CMX') + dp(0x08) + 20
click(x,y)
print('nach Klick: DROP', dp(0x64), 'WIN', dp(0xB4), 'TXTMODE', dp(0x6C))
render(cpu,'t39b.png',3)
key(0x1B)
click(200,120,0x0800)                       # freie Flaeche
print('DROP', dp(0x64))
render(cpu,'t39c.png',3)
key(0x1B)
print('DROP nach Esc', dp(0x64))
