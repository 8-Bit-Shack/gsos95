import sys; sys.path.insert(0,'.')
from t3 import *
from emu import run
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x032340; ML = ADDR + lbl['mainloop']
cpu = boot_app(ADDR)
run(cpu, 30)
while not run(cpu, 8_000_000, stop_at=ML): pass
ti=ADDR+lbl['type_icon']; tt=ADDR+lbl['typtab']
print('typtab words', [cpu.mem[tt+i]|cpu.mem[tt+i+1]<<8 for i in range(0,32,2)])
print({k:hex(v) for k,v in lbl.items() if k in ('i_app','i_doc','i_hdd','i_folder','p_app','p_aw')})
for ty in (0x0F,0xB3,0x04,0xC0,0x2A):
    cpu.A=ty; cpu.PC=ti&0xFFFF; cpu.PBR=ti>>16
    sp=cpu.S; cpu.push16(0xBEEF-1)  # Rueckkehr
    for i in range(500):
        cpu.step()
        if cpu.S==sp: break
    tn=cpu.mem[ADDR+lbl['TYPN']]|cpu.mem[ADDR+lbl['TYPN']+1]<<8
    print(hex(ty),'->',hex(cpu.A),'TYPN',hex(tn),'X',cpu.X,'M',cpu.M,cpu.XF)
cpu.A=0xB3; cpu.PC=ti&0xFFFF; cpu.PBR=ti>>16; cpu.push16(0xBEEF-1)
print('DBR',hex(cpu.DBR),'D',hex(cpu.D),'S',hex(cpu.S))
for i in range(30):
    pc=cpu.PC; cpu.step(); print(hex(pc-(ADDR&0xFFFF)),hex(cpu.A),cpu.X,'Z',cpu.Z)
