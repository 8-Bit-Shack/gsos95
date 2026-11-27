import sys; sys.path.insert(0,'.')
from emu import run
from bootcpu import BootCPU
SPL=open('sys/GSOS95.SPL','rb').read()
d=open('sys/START.GS.OS.95','rb').read()
class W(BootCPU):
    def wr(s,a,v):
        if 0xE12000<=a<0xE1A000 or (a&0xFFFF)==0xC029: s.w.append((s.n,a,v,s.PBR<<16|s.PC))
        return super().wr(a,v)
    def step(s):
        s.n+=1
        pc=s.PBR<<16|s.PC
        if pc in (0xE100A8,0xE10000) and not s.stopped: s.stopped=s.n; raise StopIteration('%06X X=%04X'%(pc,s.X))
        return super().step()
c=W(None); c.files={}; c.w=[]; c.n=0; c.stopped=0
c.mem[0x6800:0x6800+len(d)]=d
c.mem[0xE12000:0xE1A000]=SPL; c.mem[0xE19DFE]=0x39; c.mem[0xE19DFF]=0x35
c.mem[0xC029]=0xC1; c.mem[0x200C]=0x34; c.mem[0x200D]=0x12
c.E=0;c.M=c.XF=0;c.D=0;c.S=0x1FF;c.DBR=0;c.PBR=0;c.PC=0x6800
try: run(c, 3_000_000)
except StopIteration as e: print('stop',e)
print('first GS/OS call at step', c.stopped, 'PC %06X'%(c.PBR<<16|c.PC))
print('writes', len(c.w)); 
for x in c.w[:15]: print('%d %06X=%02X @%06X'%x)
scr=bytes(c.mem[0xE12000:0xE1A000])
print('pixels same', scr[:32000]==SPL[:32000], 'pal same', scr[32256:]==SPL[32256:], 'C029 %02X'%c.mem[0xC029])
