import sys; sys.path.insert(0,'.')
from t3 import *
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x052340; ML = ADDR + lbl['mainloop']
def new():
    global cpu
    cpu = boot_app(ADDR); settle()
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def tryclick(h,v):
    try: click(h,v)
    except StopIteration: return cpu.quit
WX,WY,WW,WH=40,16,240,160
new()
click(20,193); print('Finder:', tryclick(60,94))
new()
click(20,193); click(60,178); render(cpu,'b_quit.png',3)
print('Neustart:', tryclick(WX+120,WY+77))
new()
click(20,193); click(60,178); click(WX+120,WY+57); render(cpu,'b_shut.png',3)
print('nach Aus-Bildschirm:', tryclick(100,100))
new()
click(20,193); click(60,112); render(cpu,'b_ctrl.png',3)
print('Ctrl-Button:', tryclick(WX+WW//2,WY+121))
new()
click(20,193); moveto=None
cpu.events += [(0,60,94,0)]; settle(); render(cpu,'b_menu.png',3)
