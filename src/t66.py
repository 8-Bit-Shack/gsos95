# gmul / gdiv: Ergebnisse und keine fremden Variablen veraendert
import sys, random; sys.path.insert(0,'.')
from t3 import *
from emu import run
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML=ADDR+lbl['mainloop']
cpu=boot_app(ADDR)
run(cpu,30)
while not run(cpu,8_000_000,stop_at=ML): pass
STOP=ADDR+lbl['code_end']
def call(name,a,x):
    cpu.push16((STOP-1)&0xFFFF)
    cpu.PBR=ADDR>>16; cpu.PC=(ADDR+lbl[name])&0xFFFF
    cpu.A=a; cpu.X=x; cpu.M=cpu.XF=0
    assert run(cpu,100000,stop_at=STOP)
    return cpu.A&0xFFFF, cpu.X&0xFFFF
bad=0
cpu.wr16(ADDR+lbl['GMB'],0x1234)
random.seed(1)
for i in range(300):
    a=random.randrange(0,2000); x=random.randrange(0,400)
    r,xx=call('gmul',a,x)
    if r!=(a*x)&0xFFFF: bad+=1; print('gmul',a,x,r)
    if x:
        q,qx=call('gdiv',a,x)
        if q!=a//x or qx!=a//x or cpu.rd16(ADDR+lbl['GMR'])!=a%x: bad+=1; print('gdiv',a,x,q)
print('GMB unveraendert', hex(cpu.rd16(ADDR+lbl['GMB'])))
print('OK' if bad==0 and cpu.rd16(ADDR+lbl['GMB'])==0x1234 else 'FAIL Rechnen')
