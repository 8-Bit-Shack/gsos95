# Fenster verschieben / schliessen / Ordnerwechsel: Teil-Neuzeichnen == volles Neuzeichnen
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('ORDNER',0x0F)]+[('DATEI.%02d'%i,4) for i in range(5)]
t3.FS[':SYSTEM.DISK:ORDNER'] = [('X%d'%i,4) for i in range(3)]
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
def drag(h0,v0,h1,v1):
    cpu.events += [(0,h0,v0,0)]; settle()
    cpu.events += [(1,h0,v0,0)]+[(0,h0+(h1-h0)*i//8, v0+(v1-v0)*i//8,0) for i in range(1,9)]+[(2,h1,v1,0)]
    settle(); settle()
def screen(): return bytes(cpu.mem[0xE12000:0xE12000+32000])
def call(name):
    # Unterprogramm direkt aufrufen (Cursor vorher weg)
    S=cpu.S; PBR,PC=cpu.PBR,cpu.PC
    ret=ADDR+lbl['code_end']
    cpu.push(0); cpu.push16((ret-1)&0xFFFF)  # nicht benutzt
    cpu.S=S
    cpu.push16((ret-1)&0xFFFF)
    cpu.PC=(ADDR+lbl[name])&0xFFFF
    run(cpu, 50_000_000, stop_at=ret)
    cpu.PBR,cpu.PC=PBR,PC
def check(tag):
    call('cur_hide'); a=screen()
    call('redraw_all'); b=screen()
    call('cur_show')
    diff=sum(1 for x,y in zip(a,b) if x!=y)
    print(tag, 'abweichende Bytes:', diff)
    if diff: render(cpu,'t32_%s.png'%tag,3)
    return diff
settle()
dclick(28,14)                       # Arbeitsplatz
key(0x15); key(13)                  # Laufwerk oeffnen
r=check('ordner')
dclick(28,52)                       # Papierkorb (zweites Fenster)
r+=check('zweites')
drag(150,22,190,60)                 # Titelleiste ziehen
r+=check('ziehen')
drag(190,60,100,10)
r+=check('ziehen2')
key(0x1B)                            # schliessen
r+=check('schliessen')
key(0x15); key(13)
r+=check('unterordner')
key(0x7F)
r+=check('aufwaerts')
render(cpu,'t32.png',3)
assert r==0
