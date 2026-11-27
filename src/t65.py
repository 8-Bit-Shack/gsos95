# Bilder anzeigen: $C1, $C0/0001 (PackBytes), $C0/0002 (APF)
import sys, struct; sys.path.insert(0,'.')
import t3
def packbytes(b):
    out=bytearray(); i=0
    while i<len(b):
        j=i
        while j<len(b) and j-i<64 and b[j]==b[i]: j+=1
        n=j-i
        if n>=3:
            out.append(0x40|(n-1)); out.append(b[i]); i=j; continue
        k=i; lit=bytearray()
        while k<len(b) and len(lit)<64:
            if k+2<len(b) and b[k]==b[k+1]==b[k+2]: break
            lit.append(b[k]); k+=1
        out.append(len(lit)-1); out+=lit; i=k
    return bytes(out)
# Testbild: Zeile y, Byte x -> Farbe abhaengig von Position
pix=bytearray(32000)
for y in range(200):
    for x in range(160):
        c=(x//20 + y//25)&15
        pix[y*160+x]=c*17
scb=bytes(200)+bytes(56)
pal=bytearray(512)
for i in range(16): struct.pack_into('<H',pal,2*i,(i*0x111)&0xFFF)
raw=bytes(pix)+scb+bytes(pal)
pk=packbytes(raw)
# APF: MAIN-Block
lines=[packbytes(bytes(pix[y*160:(y+1)*160])) for y in range(200)]
main=struct.pack('<HHH',0,320,1)+bytes(pal[:32])+struct.pack('<H',200)
main+=b''.join(struct.pack('<HH',len(l),0) for l in lines)+b''.join(lines)
blk=struct.pack('<I',4+5+len(main))+b'\x04MAIN'+main
other=struct.pack('<I',4+5+3)+b'\x04NOTE'+b'abc'
apf=other+blk
t3.FS[':SYSTEM.DISK']=[('A.RAW',0xC1),('B.PACK',0xC0),('C.APF',0xC0)]
t3.FILES[':SYSTEM.DISK:A.RAW']=raw
t3.FILES[':SYSTEM.DISK:B.PACK']=pk
t3.FILES[':SYSTEM.DISK:C.APF']=apf
t3.AUX['B.PACK']=1; t3.AUX['C.APF']=2
src=open('t49.py').read().split("p1=path()")[0]
exec(src)
print('in', path())
res=[]
for k in range(3):
    key(0x15)
    cpu.events.append((3,160,100,13))
    run(cpu, 60_000_000, stop_at=0x050000+lbl['pic_wait'])
    run(cpu, 200_000)
    sel=cpu.rd16(cpu.rd16(ADDR+lbl['MYDP'])+0x1E)
    scr=bytes(cpu.mem[0xE12000:0xE12000+32000])
    cpu.mem[0xE12000:0xE12000+32000]=bytes(32000)
    ok=scr==bytes(pix)
    res.append(ok); print(k, 'Auswahl', sel, ok)
    if k==2: render(cpu,'t65.png',3)
    cpu.events.append((3,160,100,32)); settle(); settle()
print('Bilder (roh, gepackt, APF):', res, 'Groessen', len(raw), len(pk), len(apf))
print('OK' if all(res) else 'FAIL Bilder')
# Miniatur in den Eigenschaften fuer das APF-Bild (Auswahl steht auf C.APF)
key(0x10)
pvimg=cpu.rd16(ADDR+lbl['PVIMG'])
render(cpu,'t65b.png',3)
pal_ok = bytes(cpu.mem[0xE19E00:0xE19E00+32])!=b'' 
print('Miniatur APF', pvimg)
print('OK' if pvimg==1 else 'FAIL Miniatur gepackt')
