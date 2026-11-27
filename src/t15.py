import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
pic = bytearray(32768)
for y in range(200):
    for x in range(160): pic[y*160+x] = (((x+y)//12)%16)*17
for c in range(16):
    v = (c<<4)|(15-c)<<8 | 0x5
    pic[32256+c*2] = v & 0xFF; pic[32256+c*2+1] = v >> 8
pk = t3.pack_bytes(bytes(pic))
assert t3.unpack_bytes(pk) == bytes(pic)
print('gepackt', len(pk))
t3.FILES[':SYSTEM.DISK:FOTO'] = pk
t3.FS[':SYSTEM.DISK'] = [('FOTO',0xC0),('X',0x04)]
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
key(0x15)
cpu.events.append((3,160,100,13)); run(cpu,30)
for _ in range(40):
    run(cpu, 400_000)
render(cpu,'pk1.png',3)
print('Tools', {hex(k):v for k,v in cpu.toolcalls.items() if k in (0x2703,0x1002,0x0902)})
cpu.events.append((3,160,100,32)); settle(); settle()
render(cpu,'pk2.png',3)
