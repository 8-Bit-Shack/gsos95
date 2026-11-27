# GSOS95.INIT im Simulator: Dispatcher-Umleitung, defProc, Treffer, Zeichnen
import sys, struct; sys.path.insert(0,'.')
from emu import CPU, run
import t3
INIT=open('init95/GSOS95.INIT','rb').read()
BASE=0x060000
class C(CPU):
    def __init__(s):
        super().__init__(b''); s.log=[]; s.wins=[]; s.nextw=0x071000
    def step(s):
        full=(s.PBR<<16|s.PC)
        if full==0xE1F000: return s.tool(s.X)
        if full==0xE10000:          # JML im Speicher ausfuehren
            t=s.mem[0xE10001]|s.mem[0xE10002]<<8|s.mem[0xE10003]<<16
            s.PBR=t>>16; s.PC=t&0xFFFF; s.icount+=1; return
        return super().step()
    def tool(s,x):
        sp=s.S+4
        def ret(nin, res=b''):
            for i,b in enumerate(res): s.mem[sp+nin+i]=b
            lo=s.pull(); hi=s.pull(); bank=s.pull(); s.S+=nin
            s.PC=((hi<<8|lo)+1)&0xFFFF; s.PBR=bank; s.A=0; s.C=0
        s.log.append(x)
        if x==0x0902:
            s.mem[0x00F000:0x00F004]=struct.pack('<I',0x002000); return ret(12, struct.pack('<I',0x00F000))
        if x in (0x090E,0x610E):
            w=s.nextw; s.nextw+=0x400
            rec=w
            s.mem[rec:rec+300]=bytes(300)
            struct.pack_into('<I',s.mem,rec+174,ORIG)
            struct.pack_into('<H',s.mem,rec+210,s.frame)
            # Regionen: Handle -> Zeiger -> {size, top,left,bottom,right}
            for off,(h,p,r) in ((190,(rec+0x300,rec+0x310,s.srect)),(194,(rec+0x308,rec+0x320,s.crect))):
                struct.pack_into('<I',s.mem,h,p); struct.pack_into('<HHHHH',s.mem,p,10,*r)
                struct.pack_into('<I',s.mem,rec+off,h)
            s.wins.append(rec)
            return ret(4 if x==0x090E else 24, struct.pack('<I',rec+4))
        if x==0x1C04: return ret(0, struct.pack('<I', s.port))
        if x==0x200E: return ret(0, struct.pack('<I', 0x0C0000))
        if x==0x1B04: s.port=s.rd16(sp)|s.rd16(sp+2)<<16; s.ports.append(s.port); return ret(4)
        if x==0x6704: s.nrgn+=1; s.live+=1; return ret(0, struct.pack('<I', 0x0B0000+s.nrgn*16))
        if x==0x6804: s.live-=1; return ret(4)
        if x==0xC904: return ret(0, struct.pack('<I', 0x0B0F00))
        if x in (0x2504,0x2404): s.clips.append((x,s.rd16(sp)|s.rd16(sp+2)<<16)); return ret(4)
        if x in (0x6904,0x6C04): return ret(8)
        if x==0x7304: s.diffs+=1; return ret(12)
        if x==0x6F04: s.offs=(s.rd16(sp+2),s.rd16(sp)); return ret(8)
        if x==0x220E:
            s.hil.append((s.rd16(sp+4), s.rd16(sp)|s.rd16(sp+2)<<16)); return ret(6)
        if x==0x520E: return ret(0, struct.pack('<I', s.wins[-1]+4 if s.wins else 0))
        if x==0x2A0E: return ret(4, struct.pack('<I',0))
        if x==0x0E0E:
            t=0x07F000; s.mem[t]=7; s.mem[t+1:t+8]=b'Fenster'; return ret(4, struct.pack('<I',t))
        if x==0xA904: n=s.mem[(s.rd16(sp)|s.rd(sp+2)<<16)]; return ret(4, struct.pack('<H',n*8))
        if x in (0xA104,0xA304,0x9D04,0x9B04,0x2F04): return ret(0, b'\x00\x00')
        if x in (0x3104,0x2904,0x3004,0x5404,0x5304,0xA504): 
            if x==0x5404: assert s.port==0x0C0000, 'zeichnet nicht im WM-Port'
            if x==0x5404: p=s.rd16(sp)|s.rd(sp+2)<<16; s.rects.append(struct.unpack_from('<4h',s.mem,p)+(s.pen,))
            return ret(4)
        if x==0x3704: s.pen=s.rd16(sp); return ret(2)
        if x in (0xA004,0xA204,0x9C04,0x9A04,0x2E04): return ret(2)
        if x in (0x3A04,0x3C04): return ret(4)
        raise Exception('Tool %04X'%x)
cpu=C(); cpu.rects=[]; cpu.pen=0; cpu.hil=[]; cpu.port=0x0A0000; cpu.ports=[]; cpu.nrgn=0; cpu.live=0; cpu.clips=[]; cpu.diffs=0
n=t3.load_omf(cpu, INIT, {1:BASE})
lbl={l.split()[2].lstrip('.'):int(l.split()[1],16) for l in open('init95/init95.lbl')}
# Standard-320-Palette, SCB 0
pal=[0x000,0x777,0x841,0x72C,0x00F,0x080,0xF70,0xD00,0xFA9,0xFF0,0x0E0,0x4DF,0xDAF,0x78F,0xCCC,0xFFF]
for i,c in enumerate(pal): struct.pack_into('<H',cpu.mem,0xE19E00+2*i,c)
# urspruenglicher Dispatcher
cpu.mem[0xE10000:0xE10004]=bytes([0x5C,0x00,0xF0,0xE1])
# WM-Port: Ursprung auf die Fensterecke verschoben (boundsRect oben/links = -20/-40)
struct.pack_into('<hh',cpu.mem,0x0C0008,-20,-40)
# Original-defProc: Merker, Parameter (14) abraeumen, Ergebnis 0, RTL
ORIG=0x0E1000
code=bytes([0xC2,0x30, 0xA9,0x01,0x00, 0x8F,0x00,0x00,0x0E,
            0xA3,0x02, 0x83,0x10, 0xA3,0x01, 0x83,0x0F,
            0x3B, 0x18, 0x69,0x0E,0x00, 0x1B, 0x18, 0x6B])
cpu.mem[ORIG:ORIG+len(code)]=code
STOP=0x0F0000
def call(addr, a=0):
    cpu.push(STOP>>16); cpu.push16((STOP-1)&0xFFFF)
    cpu.PBR=addr>>16; cpu.PC=addr&0xFFFF; cpu.A=a
    assert run(cpu, 2_000_000, stop_at=STOP), 'kein Ruecksprung'
cpu.E=0; cpu.M=cpu.XF=0; cpu.S=0x0BFF; cpu.D=0x0800; cpu.DBR=0
call(BASE, a=0x1234)
print('Dispatcher:', cpu.mem[0xE10000:0xE10004].hex(), 'Hook', hex(BASE+lbl['hook']))
ok = cpu.mem[0xE10001] | cpu.mem[0xE10002]<<8 | cpu.mem[0xE10003]<<16 == BASE+lbl['hook']
# NewWindow ueber den Dispatcher (wie ein Programm)
cpu.frame=0x8000|0x4000|0x0100|0x0001
cpu.srect=(20,40,120,240); cpu.crect=(33,41,119,239)
prog=bytes([0xF4,0,0, 0xF4,0,0,  0xF4,0,0, 0xF4,0x00,0x10,  0xA2,0x0E,0x09, 0x22,0x00,0x00,0xE1,
            0x68, 0x85,0x10, 0x68, 0x85,0x12, 0x6B])   # pea 0,0 (Ergebnis) pea 0,1000 ldx #090E jsl E10000 pla sta $10 pla sta $12 rtl
cpu.mem[0x0D0000:0x0D0000+len(prog)]=prog
S0=cpu.S; cpu.Y=0x5555
call(0x0D0000)
wport=cpu.rd16(0x0810)|cpu.rd16(0x0812)<<16; rec=wport-4
dp=struct.unpack_from('<I',cpu.mem,rec+174)[0]
print('Fenster', hex(wport), 'defProc', hex(dp), 'erwartet', hex(BASE+lbl['xdef']), 'S', hex(S0), hex(cpu.S), 'Y', hex(cpu.Y))
ok = ok and dp==BASE+lbl['xdef'] and cpu.S==S0 and cpu.Y==0x5555
# Window Manager ruft die defProc: wDraw (alles)
def defproc(op, param):
    cpu.mem[0x0E0000]=0
    S1=cpu.S
    cpu.push16(0); cpu.push16(0)              # Ergebnis
    cpu.push16(0); cpu.push16(0)              # Globals
    cpu.push16(op)
    cpu.push16(rec>>16); cpu.push16(rec&0xFFFF)
    cpu.push16(param>>16); cpu.push16(param&0xFFFF)
    call(dp)
    r=cpu.rd16(cpu.S+1)|cpu.rd16(cpu.S+3)<<16
    rest=S1-cpu.S
    cpu.S=S1
    return r, rest, cpu.mem[0x0E0000]
cpu.rects=[]
r=defproc(0,0)
print('wDraw:', r, 'Rechtecke', cpu.rects[:4])
bar=[x for x in cpu.rects if x[0]==1 and x[1]==1]
ok = ok and r[1]==4 and r[2]==1 and bar and bar[0][4]==4
# Treffer: X-Knopf rechts, Maximieren daneben, Titel = Ziehen, Inhalt = Original
h1=defproc(1, (240-6)<<16 | 25)
h2=defproc(1, (240-21)<<16 | 25)
h3=defproc(1, 100<<16 | 25)
h4=defproc(1, 100<<16 | 60)
print('Treffer X/Max/Titel/Inhalt:', h1, h2, h3, h4)
ok = ok and h1[0]==22 and h2[0]==23 and h3[0]==20 and h4[2]==1 and all(h[1]==4 for h in (h1,h2,h3,h4))
# X-Knopf gedrueckt zeichnen (Parameter 1, Bit 15)
cpu.rects=[]
r5=defproc(0, 0x80000000 | 1)
print('X gedrueckt:', r5, len(cpu.rects))
ok = ok and r5[2]==0 and r5[1]==4 and len(cpu.rects)>=1
# Fenster ohne Titelleiste bleibt unveraendert
cpu.frame=0x2000
call(0x0D0000)
w2=(cpu.rd16(0x0810)|cpu.rd16(0x0812)<<16)-4
print('ohne Titel: defProc', hex(struct.unpack_from('<I',cpu.mem,w2+174)[0]))
ok = ok and struct.unpack_from('<I',cpu.mem,w2+174)[0]==ORIG
# sichtbares Fenster: Rahmen wird per HiliteWindow (umschalten, zurueck) neu gezeichnet
cpu.frame=0x8000|0x4000|0x0020|0x0001; cpu.hil=[]; cpu.Y=0x7777; S0=cpu.S
call(0x0D0000)
w3=cpu.rd16(0x0810)|cpu.rd16(0x0812)<<16
print('HiliteWindow:', [(f,hex(p)) for f,p in cpu.hil], 'S', cpu.S==S0, 'Y', hex(cpu.Y))
ok = ok and cpu.hil==[(0,w3),(1,w3)] and cpu.S==S0 and cpu.Y==0x7777
print('OffsetRgn dh,dv', [v-65536 if v>32767 else v for v in cpu.offs]); print('Port danach', hex(cpu.port), 'Regionen offen', cpu.live, 'Clips', cpu.clips[:2])
ok = ok and cpu.port==0x0A0000 and cpu.live==0
print('OK' if ok else 'FAIL INIT')
