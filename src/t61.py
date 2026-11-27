# 5,25"-Laufwerke werden beim Start nicht abgefragt, erst beim Oeffnen
import sys; sys.path.insert(0,'.')
import t3
t3.DEVS[4]=('.APPLEDISK5.25A', ':DISK525')
t3.DEVS[5]=('.APPLEDISK5.25B', None)
t3.DEVINFO[4]=(0x0084,280,0); t3.DEVINFO[5]=(0x0084,280,0)
t3.FS[':DISK525']=[('HALLO',0x04)]
vq=[]
_g=t3.AppCPU.gsos
def g2(s,num,pb):
    if num==0x2008: vq.append(s.rdstr(s.ptr(pb+2)))
    return _g(s,num,pb)
t3.AppCPU.gsos=g2
src=open('t49.py').read().split("p1=path()")[0].replace("key(0x15); key(13)                  # erstes Laufwerk oeffnen\n","")
exec(src)
render(cpu,'t61a.png',3)
q0=[v for v in vq if '5.25' in v]
key(0x15); key(0x15); key(0x15); key(13)    # dritter Eintrag (5.25A) oeffnen
p=path(); render(cpu,'t61b.png',3)
print('nach Oeffnen 5.25A:', repr(p))
q1=[v for v in vq if '5.25' in v]
key(0x1B); click(53,193)            # Arbeitsplatz erneut: Name bleibt
key(0x15); key(0x15); key(0x15); key(0x15); key(13)   # vierter: 5.25B leer -> Meldung
err=cpu.rd16(cpu.rd16(ADDR+lbl['MYDP'])+0x4A) if 'MYDP' in lbl else None
render(cpu,'t61c.png',3)
print('Abfragen beim Start', q0, 'beim Oeffnen', q1, 'Meldung', err)
print('OK' if q0==[] and q1==['.APPLEDISK5.25A'] and p==':DISK525' else 'FAIL 5,25-Laufwerke')
