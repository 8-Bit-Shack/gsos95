# Laufwerke auf dem Desktop (Einstellung)
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'] = bytes([0x0C, 0, 0, 2, 0, 0, 0, 0])
t3.DEVINFO.update({1: (0x0080, 1600, 4), 2: (0x0084, 1600, 4), 3: (0x0080, 65535, 5), 4: (0x0084, 280, 0)})
t3.DEVS[4] = ('.APPLEDISK5.25A', None)
def boot():
    cpu = boot_app(ADDR)
    def settle(maxn=8_000_000):
        run(cpu, 30)
        while True:
            if run(cpu, maxn, stop_at=ML) and not cpu.events: return
    return cpu, settle
cpu, settle = boot()
w=lambda n: cpu.mem[ADDR+lbl[n]]|cpu.mem[ADDR+lbl[n]+1]<<8
def dp(o):
    v=cpu.mem[cpu.D+o]|cpu.mem[cpu.D+o+1]<<8
    return v-65536 if v>32767 else v
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
settle()
def names():
    out=[]
    for i in range(w('DVCNT')):
        a=ADDR+lbl['dvtab']+20*i
        out.append((hex(cpu.rd16(a)), bytes(cpu.mem[a+5:a+5+cpu.mem[a+4]]).decode()))
    return out
print('DVON', w('DVON'), 'Laufwerke', names())
ok = w('DVON') == 1 and names() == [('0x100','SYSTEM.DISK'), ('0x100','GSOS95'), ('0x102','5,25\" A')]
log=[x for x in cpu.log if x=='0x2008']
print('Volume-Aufrufe:', len(log))
render(cpu,'t67a.png',3)
click(292, 6+36+8)                      # GSOS95 waehlen
print('SEL', dp(0xB6))
ok &= dp(0xB6) == 17
dclick(292, 6+36+8)                     # oeffnen
print('nach Doppelklick: NWIN', w('NWIN'), 'Pfad', bytes(cpu.mem[ADDR+lbl['pathbuf']+2:ADDR+lbl['pathbuf']+2+cpu.rd16(ADDR+lbl['pathbuf'])]))
ok &= w('NWIN') == 1 and bytes(cpu.mem[ADDR+lbl['pathbuf']+2:ADDR+lbl['pathbuf']+9]) == b':GSOS95'
render(cpu,'t67b.png',3)
key(0x1B)
# Diskette eingelegt: Ereignis 7 -> neu abfragen
t3.DEVS[2] = ('.APPLEDISK3.5B', ':NEUE.DISK')
cpu.events.append((7,160,100,0)); settle(); settle()
print('nach Diskette:', names())
ok &= ('0x100','NEUE.DISK') in names()
# Einstellungen > Anzeige: neue Zeile, ausschalten, OK
click(65,193); dclick(104,55)
render(cpu,'t67c.png',3)
click(40+96+5, 16+30+4*22+6)
click(40+240-136+30, 16+160-24+7)
print('nach Abschalten im Dialog:', w('DVON'), w('DVCNT'), 'Datei', t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'][:4])
ok &= w('DVON') == 0 and w('DVCNT') == 0 and t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'][3] & 2 == 0
render(cpu,'t67d.png',3)
# ausschalten: Einstellungen schreiben ohne Bit
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'] = bytes([0x0C, 0, 0, 0, 0, 0, 0, 0])
cpu, settle = boot(); settle()
print('aus:', w('DVON'), w('DVCNT'))
ok &= w('DVON') == 0 and w('DVCNT') == 0
print('OK' if ok else 'FAIL Laufwerke')
