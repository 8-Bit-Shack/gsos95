import sys; sys.path.insert(0,'.')
from emu import *
labels = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('gs95.lbl')}
ML = labels['mainloop']
cpu = boot('GSOS95.2mg')
# Splash mitten in der Animation
run(cpu, 3_000_000, stop_at=labels['cycle_pal']); run(cpu,5000)
render(cpu, 'shot_splash.png', 3)
run(cpu, 5_000_000, stop_at=ML)
def settle():
    while cpu.mq: run(cpu, 2000)
    run(cpu, 20); run(cpu, 5_000_000, stop_at=ML)
def rw(a): return cpu.mem[a] | cpu.mem[a+1]<<8
def moveto(x,y):
    while True:
        dx = x - rw(0xAA); dy = y - rw(0xAC)
        if dx == 0 and dy == 0: break
        dx = max(-60,min(60,dx)); dy=max(-60,min(60,dy))
        cpu.mq.append((0x80 | (dx & 0x7F), 0x80 | (dy & 0x7F)))
        settle()
def click(x,y):
    moveto(x,y)
    cpu.mq.append((0x80, 0x00)); settle()
    cpu.mq.append((0x80, 0x80)); settle()
click(20,193); moveto(60,90); render(cpu,'shot_menu.png',3)
click(60,90)   # Programme
render(cpu,'shot_prog.png',3)
click(270,24)  # schliessen
click(28,14); click(28,14)   # Arbeitsplatz doppelt
render(cpu,'shot_arbeit.png',3)
click(270,24)
click(28,86); click(28,86)
render(cpu,'shot_lies.png',3)
click(270,24)
click(20,193); click(60,160)  # Ausfuehren
render(cpu,'shot_run.png',3)
click(100,140)
click(20,193); click(60,150)  # Hilfe
render(cpu,'shot_info.png',3)
click(160,160)
click(20,193); click(60,178)  # Beenden
render(cpu,'shot_shutdown.png',3)
cpu.kbd = 0xA0; settle()
render(cpu,'shot_restart.png',3)
print('ok', cpu.icount, 'WIN', rw(0xB4), 'SHUT', rw(0xD4))
