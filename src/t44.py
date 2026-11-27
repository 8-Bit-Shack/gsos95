# Win95-Rahmen: eigene defProc fuer Zubehoer-Fenster (mit nachgebautem Fenster)
import sys, struct; sys.path.insert(0,'.')
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
settle()
W=0x0C1000; m=cpu.mem
def w16(a,v): m[a]=v&255; m[a+1]=(v>>8)&255
def w32(a,v): w16(a,v&0xFFFF); w16(a+2,v>>16)
# Original-defProc: Merker setzen, Parameter abraeumen, RTL
ORIG=0x0E1000
code=bytes([0xC2,0x30, 0xA9,0x01,0x00, 0x8F,0x00,0x00,0x0E,   # rep; lda #1; sta $0E0000
            0xA3,0x02, 0x83,0x10, 0xA3,0x01, 0x83,0x0F,        # lda 2,s; sta 16,s; lda 1,s; sta 15,s
            0x3B, 0x18, 0x69,0x0E,0x00, 0x1B, 0x18, 0x6B])     # tsc clc adc #14 tcs clc rtl
m[ORIG:ORIG+len(code)]=code
w32(W+174, ORIG)
w16(W+210, 0x8000|0x4000|0x0001)
# Regionen: Handle -> Zeiger -> {size, top,left,bottom,right}
def rgn(hnd, ptr, r):
    w32(hnd, ptr); w16(ptr, 10)
    for i,v in enumerate(r): w16(ptr+2+2*i, v)
rgn(0x0C2000, 0x0C2100, (40, 100, 140, 260))   # Struktur
rgn(0x0C2010, 0x0C2200, (53, 102, 138, 258))   # Inhalt
w32(W+190, 0x0C2000); w32(W+194, 0x0C2010)
# Installation wie aus nda_style
cpu.ndawin=True
def call_far(label, a=None):
    S=cpu.S
    ret=ADDR+lbl['code_end']
    cpu.push(ret>>16); cpu.push16((ret-1)&0xFFFF)
    cpu.PBR=0x04; cpu.PC=lbl[label]
    run(cpu, 5_000_000, stop_at=ret)
NSW=ADDR+lbl['NSW']; w32(NSW, W+4)    # Fensterzeiger = Port = Record+4
call_far('xf_wdef_install')
dp=struct.unpack_from('<I', m, W+174)[0]
print('defProc jetzt', hex(dp), 'erwartet', hex(0x040000+lbl['x_wdef']), 'Tabelle', m[ADDR+lbl['wdtab']:ADDR+lbl['wdtab']+8].hex())
def defproc(op, param):
    S0=cpu.S
    cpu.push16(0); cpu.push16(0)                 # Ergebnis
    cpu.push16(0); cpu.push16(0x1234)            # windGlobals
    cpu.push16(op)
    cpu.push16(W>>16); cpu.push16(W&0xFFFF)
    cpu.push16(param>>16); cpu.push16(param&0xFFFF)
    ret=0x0E2000; m[ret]=0xEA; m[ret+1]=0x80; m[ret+2]=0xFE
    cpu.push(0x0E); cpu.push16(ret-1)
    m[0x0E0000]=0
    cpu.qd=[]
    cpu.PBR=dp>>16; cpu.PC=dp&0xFFFF
    run(cpu, 5_000_000, stop_at=ret)
    res=cpu.rd16(cpu.S+1)|cpu.rd16(cpu.S+3)<<16
    left=S0-cpu.S
    cpu.S=S0
    return res, left, m[0x0E0000], cpu.C
for name,(op,p) in {'hit X':(1,(250<<16)|45),'hit Titel':(1,(150<<16)|45),'hit Inhalt':(1,(150<<16)|90),
                     'draw alles':(0,0),'draw X gedrueckt':(0,0x80000001),'draw Info':(0,3),'calc':(2,0)}.items():
    r=defproc(op,p)
    print('%-16s Ergebnis %-6s Stapelrest %d  Original gerufen %d  C=%d  QD %d' % (name, r[0], r[1], r[2], r[3], len(cpu.qd)))
before=bytes(m[0x0C2300:0x0CA000])
r=defproc(0,0)
print('OK' if bytes(m[0x0C2300:0x0CA000])==before else 'FAIL: Schreibzugriff in die Bank des Fensterrecords')
render(cpu,'wd1.png',3)
px=lambda x,y: (m[0xE12000+y*160+x//2]>>(0 if x&1 else 4))&15
print('X-Knopf Pixel (245..256,42..51):', [px(x,46) for x in range(242,258)])
print('links uebermalt:', [px(x,46) for x in (101,105,119)])
r=defproc(0,0x80000001)
render(cpu,'wd2.png',3)
print('gedrueckt:', [px(x,42) for x in range(242,258)])
ok = px(243,42)==0 and px(200,46)==13
print('OK' if ok else 'FAIL Rahmen')
# Klick auf den X-Knopf: selbst verfolgt, Zubehoer wird geschlossen
dpb=cpu.rd16(ADDR+lbl['MYDP']); EV=dpb+0x30
def clickx(x,y,rx,ry):
    w32(ADDR+lbl['fwptr'], W+4); w16(EV+12,x); w16(EV+10,y)
    cpu.events.append((2,rx,ry,0)); cpu.ndaclosed=0
    cpu.D=dpb; cpu.DBR=ADDR>>16
    call_far('xf_wdef_click')
    return cpu.C, cpu.ndaclosed
r1=clickx(250,45,250,45); r2=clickx(250,45,150,120); r3=clickx(150,45,150,45)
print('Klick X:', r1, ' weggezogen:', r2, ' Titel:', r3)
print('OK' if r1==(1,1) and r2==(1,1) and r3==(0,0) else 'FAIL Klick')
render(cpu,'wd3.png',3)

# Fenster teilweise ausserhalb des Bildschirms: nichts selbst zeichnen
for rg in ((40,200,140,360),(-6,100,94,260),(180,100,280,260),(40,-30,140,130)):
    rgn(0x0C2000, 0x0C2100, [v & 0xFFFF for v in rg])
    rgn(0x0C2010, 0x0C2200, [(rg[0]+13)&0xFFFF, (rg[1]+2)&0xFFFF, (rg[2]-2)&0xFFFF, (rg[3]-2)&0xFFFF])
    scr=bytes(m[0xE12000:0xE19D00]); oth=bytes(m[0x0C2300:0x0CA000])
    r=defproc(0,0); h=defproc(1,((rg[3]-10)&0xFFFF)<<16|((rg[0]+5)&0xFFFF))
    ok = bytes(m[0xE12000:0xE19D00])==scr and bytes(m[0x0C2300:0x0CA000])==oth and r[1]==4 and h[1]==4 and h[2]==1
    print(rg, 'OK' if ok else 'FAIL ausserhalb', r, h)
# inaktives Fenster, vorn im Window Manager: graue Titelleiste mit X
rgn(0x0C2000, 0x0C2100, (40, 100, 140, 260)); rgn(0x0C2010, 0x0C2200, (53, 102, 138, 258))
w16(W+210, 0x8000|0x4000)            # Titel + Schliessfeld, nicht aktiv
cpu.firstwin = W+4
r=defproc(0,0); g1=px(200,46)
cpu.firstwin = 0x0C5004              # anderes Zubehoer liegt davor
m[0xE12000+46*160+100]=0x77
r=defproc(0,0); g2=px(200,46)
print('inaktiv vorn:', g1, ' verdeckt:', g2)
DK=1
print('OK' if g1==DK and g2!=DK else 'FAIL inaktiv')
