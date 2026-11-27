import sys; sys.path.insert(0,'/home/claude/gs95'); sys.path.insert(0,'/home/claude/gs95/boot95')
import bootskin, patch
from emu import CPU, run
from PIL import Image
orig=open('/home/claude/gs95/sys/START.GS.OS','rb').read()
d=patch.patch(bootskin.build(orig))
open('START.GS.OS.95','wb').write(d)
cpu=CPU(None)
cpu.mem[0x6800:0x6800+len(d)]=d
# Treiber: JSR $75F4, JSR $76BB, 60x JSR $74F3, dann Endlosschleife
drv=[0x20,0xF4,0x75, 0x20,0xBB,0x76]
for i in range(int(sys.argv[1]) if len(sys.argv)>1 else 60): drv+=[0x20,0xF3,0x74]
drv+=[0x80,0xFE]
cpu.mem[0x0300:0x0300+len(drv)]=bytes(drv)
cpu.E=0; cpu.M=0; cpu.XF=0; cpu.D=0x0800; cpu.S=0x01FF; cpu.DBR=0; cpu.PBR=0; cpu.PC=0x0300
cpu.mem[0x0802]=0xE1
end=0x0300+len(drv)-2
run(cpu, 20_000_000, stop_at=end)
print('fertig', hex(cpu.PC), cpu.icount)
m=cpu.mem
img=Image.new('RGB',(640,200)); px=img.load()
for y in range(200):
    scb=m[0xE19D00+y]; pal=0xE19E00+(scb&15)*32
    cols=[]
    for i in range(16):
        w=m[pal+2*i]|m[pal+2*i+1]<<8
        cols.append((((w>>8)&15)*17,((w>>4)&15)*17,(w&15)*17))
    for xb in range(160):
        b=m[0xE12000+y*160+xb]
        if scb&0x80:
            grp=[8,12,0,4]
            for k in range(4):
                v=(b>>(6-2*k))&3; px[xb*4+k,y]=cols[grp[k]+v]
        else:
            for k,v in enumerate((b>>4,b&15)):
                px[xb*4+2*k,y]=cols[v]; px[xb*4+2*k+1,y]=cols[v]
img.resize((1280,800)).save('boot95.png')
