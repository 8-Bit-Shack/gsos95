# Startbild ohne GSOS95.SPL: Ladebalken laeuft vom Bootbild (START.GS.OS) aus weiter
# (Bild mit Datei: t69.py)
import sys; sys.path.insert(0,'.'); sys.path.insert(0,'boot95')
from t3 import *
from emu import run, render, CPU
from bootcpu import BootCPU
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
def bootscreen(steps):
    d=open('sys/START.GS.OS.95','rb').read()
    c=BootCPU(None); c.mem[0x6800:0x6800+len(d)]=d
    drv=[0x20,0x19,0x79, 0x20,0xE6,0x77]
    for i in range(steps): drv+=[0x20,0x1C,0x79]      # zwei Schritte je Datei
    drv+=[0x80,0xFE]
    c.mem[0x0300:0x0300+len(drv)]=bytes(drv)
    c.E=0; c.M=0; c.XF=0; c.D=0x0800; c.S=0x01FF; c.DBR=0; c.PBR=0; c.PC=0x0300
    c.mem[0x0802]=0xE1; c.mem[0x0806]=0xE1
    run(c, 30_000_000, stop_at=0x0300+len(drv)-2)
    return bytes(c.mem[0xE12000:0xE1A000])
def px(m,x,y): return (m[0xE12000+y*160+x//2]>>(0 if x&1 else 4))&15
for steps in (0, 30, 60, 200):
    scr=bootscreen(steps)
    cpu=boot_app(ADDR)
    cpu.mem[0xE12000:0xE1A000]=scr
    logo_boot=[[ (scr[y*160+x//2]>>(0 if x&1 else 4))&15 for x in range(320)] for y in range(56,125)]
    run(cpu, 30_000_000, stop_at=ADDR+lbl['spl_step'])
    m=cpu.mem
    bp=cpu.rd16(ADDR+lbl['BOOTP']); sp=cpu.rd16(ADDR+lbl['SPLP'])
    same=all(px(m,x,y)==logo_boot[y-56][x] for y in range(56,125) for x in range(320))
    scb=bytes(m[0xE19D00:0xE19DC8])
    print('Schritte %3d: Bootstand %3d, Balken danach %3d, Logo wie Bootbild: %s, SCB640 weg: %s' % (steps, bp, sp, same, 0x80 not in scb))
    if sp!=bp or (steps and bp<2): print('FAIL')
    if steps==60: render(cpu,'spl1.png',3)
    run(cpu, 60_000_000, stop_at=ML)
    ok = cpu.rd16(ADDR+lbl['SPLON'])==0 and cpu.rd16(ADDR+lbl['SPLEND'])==0 and not any(m[0xE19D00:0xE19DC8])
    print('   Schreibtisch:', 'OK' if ok else 'FAIL', 'Balken zuletzt', cpu.rd16(ADDR+lbl['SPLP']))
    if steps==60: render(cpu,'spl2.png',3)
print('fertig')
