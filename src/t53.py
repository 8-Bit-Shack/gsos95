# Eigenschaften mit Textvorschau
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
def key(c): cpu.events.append((3,160,100,c)); settle(); settle()
settle()
key(9); key(13); key(13); key(0x15); key(13)
n=0
for k in range(10):
    key(0x15)
    key(0x10)
    n=cpu.rd16(ADDR+lbl['PVN'])
    if n: break
    key(0x1B)
render(cpu,'pv1.png',3)
buf=bytes(cpu.mem[ADDR+lbl['pvbuf']:ADDR+lbl['pvbuf']+n])
print('Vorschau', n, repr(buf[:40]))
iop=ADDR+lbl['io_pb']; print('io_pb Zeiger wieder Kopierpuffer:', hex(cpu.rd16(iop+4)), hex(cpu.rd16(iop+6)), 'open_pb:', cpu.rd16(ADDR+lbl['open_pb']+4)==(lbl['pathbuf']+0x0400)&0xFFFF)
print('OK' if n>0 and cpu.rd16(ADDR+lbl['open_pb']+4)==(lbl['pathbuf']+0x0400)&0xFFFF else 'FAIL Vorschau')
