import sys; sys.path.insert(0,'.')
from t3 import *
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x052340
ML = ADDR + lbl['mainloop']
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
settle()
print('Desktop ok, Tools:', {hex(k):v for k,v in cpu.toolcalls.items()})
render(cpu,'a_desk.png',3)
click(28,14); click(28,14)
render(cpu,'a_vol.png',3)
GX,GY=48,48
click(GX+38,GY+8); click(GX+38,GY+8)
render(cpu,'a_dir.png',3)
click(40+240-13, 16+160-10)
render(cpu,'a_page2.png',3)
click(40+240-28, 16+160-10)
click(GX+38,GY+8); click(GX+38,GY+8)
render(cpu,'a_sys.png',3)
click(60,38)
click(GX+38,GY+27+8); click(GX+38,GY+27+8)   # README -> Meldung
render(cpu,'a_msg.png',3)
click(160,16+160-18)
try:
    click(GX+38+76,GY+27+8); click(GX+38+76,GY+27+8)   # FINDER
except StopIteration:
    pass
print('Quit:', cpu.quit)
cpu = boot_app(ADDR); settle()
click(20,193); click(60,76)     # Start > Programme
render(cpu,'a_prog.png',3)
click(20,193); click(60,178)    # Beenden
render(cpu,'a_shut.png',3)
try:
    click(100,100)
except StopIteration: pass
print('Quit2:', cpu.quit)
