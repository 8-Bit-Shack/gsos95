# Bildlaufleiste, Maximieren, Groesse ziehen, Teil-Neuzeichnen == volles
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FS[':SYSTEM.DISK'] = [('ORDNER',0x0F)]+[('DATEI.%02d'%i,4) for i in range(40)]
t3.FS[':SYSTEM.DISK:ORDNER'] = [('X%d'%i,4) for i in range(3)]
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
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
    PBR,PC=cpu.PBR,cpu.PC
    ret=ADDR+lbl['code_end']
    cpu.push16((ret-1)&0xFFFF)
    cpu.PC=(ADDR+lbl[name])&0xFFFF
    run(cpu, 50_000_000, stop_at=ret)
    cpu.PBR,cpu.PC=PBR,PC
def w(name): a=ADDR+lbl[name]; return cpu.mem[a]|cpu.mem[a+1]<<8
def dp(off): return cpu.mem[cpu.D+off]|cpu.mem[cpu.D+off+1]<<8
bad=0
def check(tag):
    global bad
    call('cur_hide'); a=screen()
    import emu
    cpu.mem[0xE12000:0xE12000+32000]=a
    call('redraw_all'); b=screen()
    call('cur_show')
    diff=sum(1 for x,y in zip(a,b) if x!=y)
    if diff:
        idx=[i for i,(x,y) in enumerate(zip(a,b)) if x!=y]
        ys=[i//160 for i in idx]; xs=[(i%160)*2 for i in idx]
        print('  bbox x',min(xs),max(xs),'y',min(ys),max(ys))
        render(cpu,'t36_%s_full.png'%tag,3)
    print(tag, 'PAGE',dp(0x1C),'CW',w('CW'),'CH',w('CH'),'NCOL',w('NCOL'),'NROW',w('NROW'),'MAXPG',w('MAXPG'),'PPG',w('PPG'),'NCV',w('NCV'),'NRV',w('NRV'),'SLOT',dp(0x2A),'diff',diff)
    render(cpu,'t36_%s.png'%tag,3)
    bad+=diff
settle()
dclick(28,14); key(0x15); key(13)          # Arbeitsplatz -> Laufwerk
check('start')
click(290,139)                              # Pfeil runter
click(290,139)
check('runter2')
click(290,120)                              # Spur unterhalb Daumen
check('seite')
drag(290,95,290,140)                        # Daumen nach unten ziehen
check('daumen')
click(277,10)                               # maximieren
check('max')
click(308,160)                             # Pfeil runter (maximiert)
check('max_runter')
key(0x0A); key(0x0A); key(0x0A)
check('max_tasten')
click(295,10)                               # wiederherstellen
check('zurueck')
drag(295,157,250,120)                       # Groesse verkleinern
check('klein')
key(0x0A); key(0x0A); key(0x0A)
check('tasten')
key(0x16 if False else 0x15)
dclick(150,10)                              # Doppelklick Titel = maximieren
check('titel')
print('SUMME diff', bad)
