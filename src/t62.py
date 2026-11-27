# Gemerktes Fenster auf einem fehlenden Laufwerk: beim Start nicht danach suchen
import sys; sys.path.insert(0,'.')
import t3
p=b':SKULLHD:SKULL.ISLAND'
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.WIN']=bytes([1,0,1,0,len(p),0])+p
calls=[]
_g=t3.AppCPU.gsos
def g2(s,num,pb):
    r=_g(s,num,pb)
    try:
        if num in (0x2010,0x2006) and 'SKULL' in s.rdstr(s.ptr(pb+(2 if num==0x2006 else 4))).upper(): calls.append(num)
    except Exception: pass
    return r
t3.AppCPU.gsos=g2
src=open('t49.py').read().split("click(53,193)")[0]
exec(src)
for i in range(3): settle()
render(cpu,'t62.png',3)
nw=cpu.rd16(ADDR+lbl['NWIN'])
print('Zugriffe auf SKULLHD:', calls, 'Fenster', nw, 'Pfad', repr(path()))
# bekanntes Laufwerk wird weiter wiederhergestellt
ok1 = calls==[] and nw==1 and path()==''
print('OK' if ok1 else 'FAIL fehlendes Laufwerk')
