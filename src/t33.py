# Systemsteuerung: Datum/Uhrzeit, Anzeige, Maus, Ton
import sys; sys.path.insert(0,'.')
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
def key(c):
    cpu.events.append((3,160,100,c)); settle(); settle()
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
D = None
def org():
    D = cpu.rd16(ADDR + lbl['MYDP'])
    s=lambda v: v-65536 if v>32767 else v
    return 40 + s(cpu.rd16(D + 8)), 16 + s(cpu.rd16(D + 0x0A))
def var(n): return cpu.rd16(ADDR + lbl[n])
settle()
key(9); key(0x0A); key(0x0A); key(0x0A); key(13)  # Start > Einstellungen
x,y = org(); print("org",x,y); render(cpu,"st1.png",3)
assert cpu.rd16(cpu.rd16(ADDR+lbl['MYDP'])+0xB4) == 6
# Datum/Uhrzeit (Zelle 2) per Doppelklick
dclick(x+12+2*68+30, y+34+10)
x,y = org(); render(cpu,'st2.png',3)
assert cpu.rd16(cpu.rd16(ADDR+lbl['MYDP'])+0xB4) == 15, 'kein Datumsdialog'
sv = [cpu.rd16(ADDR+lbl['sval']+2*i) for i in range(5)]
print('Zeit gelesen', sv)
click(x+96+58+5, y+30+3*22+5)                  # Stunde +
click(x+96+40+5, y+30+4*22+5)                  # Minute -
render(cpu,'st3.png',3)
click(x+240-136+20, y+160-24+5)                # OK
print('WriteTimeHex', [hex(v) for v in cpu.timeset])
assert cpu.timeset == (0x0A0E, 0x7E0A, 0x2900), cpu.timeset
# Anzeige (Zelle 0): Tastatur
x,y = org()
key(0x15); key(0x08); key(0x08); key(0x08)      # Auswahl auf 0 (Rechts, dann 3x links -> 0? )
cpu_sel = var('CPSEL'); print('CPSEL', cpu_sel)
while var('CPSEL') != 0: key(0x08)
key(13)
x,y = org(); render(cpu,'st4.png',3)
click(x+96-12+4*8+3, y+30+5)                   # Desktopfarbe 4
click(x+96-12+2*8+3, y+30+22+5)                # Rahmen 2
render(cpu,'st5.png',3)
key(13)                                        # Return = OK
print('DESKC', var('DESKC'), 'BRAM', t3.BRAM, 'prefs', t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'])
assert var('DESKC') == 4 and t3.BRAM[0x1C] == 2
assert t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'][1] == 0x84
render(cpu,'st6.png',3)
# Maus (Zelle 1)
key(0x15); key(13)
x,y = org()
click(x+96+4*16+5, y+30+5)                     # Doppelklick 4 (kurz)
click(x+96+5, y+30+22+5)                       # schnelle Maus an
render(cpu,'st7.png',3)
key(13)
print('BRAM', t3.BRAM, 'DBLT', var('DBLT'))
assert t3.BRAM[0x2E] == 4 and t3.BRAM[0x33] == 1 and var('DBLT') == 14
# Ton (Zelle 3)
key(0x15); key(0x15); key(13)
x,y = org()
click(x+96+58+5, y+30+5); click(x+96+58+5, y+30+5)   # Lautstaerke +2
render(cpu,'st8.png',3)
key(13)
print('BRAM', t3.BRAM, 'GLU', cpu.mem[0xC03C], 'beeps', getattr(cpu,'beeps',0))
assert t3.BRAM[0x1E] == 10 and cpu.mem[0xC03C] & 15 == 10
# Sprache (Zelle 4)
key(0x15); key(13)
print('LANG', cpu.rd16(cpu.rd16(ADDR+lbl['MYDP'])+0x54))
render(cpu,'st9.png',3)
