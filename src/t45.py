# Schreibt irgendeine Zeichenroutine ueber das Ende eines Puffers hinaus?
import sys; sys.path.insert(0,'.')
from t3 import *
from emu import run
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c): cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def dbl(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0),(1,h,v,0),(2,h,v,0)]; settle(); settle()
def check(tag):
    al = sorted(cpu.allocs)[1:]
    bad = 0
    for i,(p,sz) in enumerate(al):
        end = p+sz
        nxt = al[i+1][0] if i+1 < len(al) else end+0x100
        gap = bytes(cpu.mem[end:nxt])
        if any(gap):
            k = next(j for j,b in enumerate(gap) if b)
            print('FAIL', tag, 'Puffer %06X+%X: Byte hinter dem Ende bei +%d' % (p, sz, k), gap[k:k+8].hex()); bad += 1
    print(tag, 'ok' if not bad else 'FEHLER', [(hex(p),sz) for p,sz in al])
settle(); check('start')
dbl(20,20); check('arbeitsplatz')
dbl(120,60); check('volume')
click(12,193); check('startmenue')
key(0x1B); check('esc')
click(12,193); click(60,96); check('zubehoer')
click(80,16+40+3+12+4); check('nda offen')
for i in range(3):
    cpu.events.append((0,100+i*30,150,0)); settle()
check('nda maus')
print('fertig')
