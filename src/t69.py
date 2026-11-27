# Startbild aus *:System:GSOS95.SPL: START.GS.OS laedt es, GSOS95 zeigt dasselbe Bild,
# Ladebalken laeuft weiter; ohne Datei bleibt der Farbverlauf (t46)
import sys; sys.path.insert(0,'.'); sys.path.insert(0,'boot95')
from t3 import *
import t3
from emu import run, render, CPU
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
SPL = open('sys/GSOS95.SPL','rb').read()
from bootcpu import BootCPU
def bootscreen(steps, files):
    d=open('sys/START.GS.OS.95','rb').read()
    c=BootCPU(None); c.files = files; c.mem[0x6800:0x6800+len(d)]=d
    drv=[0x20,0x19,0x79, 0x20,0xE6,0x77]
    for i in range(steps): drv+=[0x20,0x1C,0x79]
    drv+=[0x80,0xFE]
    c.mem[0x0300:0x0300+len(drv)]=bytes(drv)
    c.E=0; c.M=0; c.XF=0; c.D=0x0800; c.S=0x01FF; c.DBR=0; c.PBR=0; c.PC=0x0300
    c.mem[0x0802]=0xE1; c.mem[0x0806]=0xE1
    run(c, 30_000_000, stop_at=0x0300+len(drv)-2)
    return bytes(c.mem[0xE12000:0xE1A000]), getattr(c, 'calls', [])
ok = True
def check(c, m):
    global ok; print(('OK   ' if c else 'FAIL ') + m); ok = ok and c
scr0, calls = bootscreen(30, {})
check(calls == [], 'START.GS.OS ruft GS/OS nicht auf')
B0 = 141
def col(m, x, y):
    s = m[32000 + y]; p = s & 15
    v = (m[y*160 + x//2] >> (0 if x & 1 else 4)) & 15
    return m[32256 + p*32 + 2*v] | m[32257 + p*32 + 2*v] << 8
diff = [y for y in list(range(B0, 166)) + list(range(172, 200)) if any(col(scr0, x, y) != col(SPL, x, y) for x in range(320))]
# seit 2.1.41: Startbild mit Wolken bis unten, das Ersatz-Bootbild (ohne Datei) bleibt flach
print('info: Zeilen anders als Ersatzbild', len(diff))
pb, ps = scr0[32256:], SPL[32256:]

check(all(scr0[32000+y] == 14 for y in range(166)), 'Bootbild: Palette 14 bis zum Balken')
check(set(SPL[32000:32166]) == {1}, 'Startbild: nur Palette 1 bis zum Balken (keine Palettenwechsel)')
render_scr = scr0
# SPLASH95 (temporaerer Systemzusatz) setzt das Bild ein
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.SPL'] = SPL
t3.FS[':SYSTEM.DISK:SYSTEM'].append(('GSOS95.SPL', 0xC1))
class Watch(t3.AppCPU):
    def wr(s, a, v):
        if 0xE12000 <= a < 0xE1A000: s.wlog.append(a)
        return super().wr(a, v)
ic = Watch(); ic.wlog = []
load_omf(ic, open('init95/SPLASH95', 'rb').read(), {1: 0x070000})
ic.mem[0xE12000:0xE1A000] = scr0
ic.mem[0x0300:0x0306] = bytes([0x22, 0x00, 0x00, 0x07, 0x80, 0xFE])
ic.E = 0; ic.M = ic.XF = 0; ic.PBR = 0; ic.PC = 0x0300; ic.DBR = 0; ic.D = 0x0800; ic.S = 0x0BFF; ic.A = 0x1234
run(ic, 5_000_000, stop_at=0x0304)
scr = bytes(ic.mem[0xE12000:0xE1A000])
check(ic.PC == 0x0304 and ic.C == 0, 'SPLASH95 laeuft durch')
check(scr[:166*160] == SPL[:166*160] and scr[32000:32166] == SPL[32000:32166] and scr[32256+32:32256+14*32] == SPL[32256+32:32256+14*32], 'danach: Bild, SCBs und Paletten 1..13 wie die Datei')
check(scr[166*160:172*160] == scr0[166*160:172*160], 'Balkenzeilen mit Fortschritt unveraendert')
w = ic.wlog
pal = [i for i, a in enumerate(w) if a >= 0xE19E00]
scbw = [(i, a - 0xE19D00) for i, a in enumerate(w) if 0xE19D00 <= a < 0xE19DC8]
pix = [(i, (a - 0xE12000) // 160) for i, a in enumerate(w) if a < 0xE19D00]
check(pal and pix and max(pal) < min(i for i, _ in pix), 'Paletten zuerst')
check(all(0xE19E20 <= w[i] < 0xE19E20 + 13*32 for i in pal), 'nur Paletten 1..13 geschrieben (Bootbild nutzt 14)')
lastpix = {}
for i, r in pix: lastpix[r] = i
check(all(lastpix[r] < i for i, r in scbw), 'je Zeile: Punkte vor SCB')
check(max(r for _, r in pix) < 166 and max(r for _, r in scbw) < 166, 'Balken und darunter nicht angefasst')
check(ic.toolcalls.get(0x1002, 0) == 1 and ic.toolcalls.get(0x0902, 0) == 1, 'Speicher geholt und freigegeben')
# ohne Datei: nichts passiert
del t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.SPL']
ic2 = t3.AppCPU(); load_omf(ic2, open('init95/SPLASH95', 'rb').read(), {1: 0x070000})
ic2.mem[0xE12000:0xE1A000] = scr0
ic2.mem[0x0300:0x0306] = bytes([0x22, 0x00, 0x00, 0x07, 0x80, 0xFE])
ic2.E = 0; ic2.M = ic2.XF = 0; ic2.PBR = 0; ic2.PC = 0x0300; ic2.DBR = 0; ic2.D = 0x0800; ic2.S = 0x0BFF
run(ic2, 2_000_000, stop_at=0x0304)
check(bytes(ic2.mem[0xE12000:0xE1A000]) == scr0, 'ohne Datei: Bootbild bleibt')
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.SPL'] = SPL
# ohne Bootbild (z.B. Textbildschirm): nichts laden
ic3 = t3.AppCPU(); load_omf(ic3, open('init95/SPLASH95', 'rb').read(), {1: 0x070000})
ic3.mem[0x0300:0x0306] = bytes([0x22, 0x00, 0x00, 0x07, 0x80, 0xFE])
ic3.E = 0; ic3.M = ic3.XF = 0; ic3.PBR = 0; ic3.PC = 0x0300; ic3.DBR = 0; ic3.D = 0x0800; ic3.S = 0x0BFF
run(ic3, 2_000_000, stop_at=0x0304)
check(not any(ic3.mem[0xE12000:0xE12100]), 'ohne Bootbild: nichts geladen')
# GSOS95: gleiches Bild, Balken setzt fort
_og = t3.AppCPU.gsos
OPENED = []
def _g(s, num, pb):
    if num == 0x2010: OPENED.append(s.rdstr(s.ptr(pb + 4)))
    return _og(s, num, pb)
t3.AppCPU.gsos = _g
_ot = t3.AppCPU.tool
QDSEEN = []
def _t(s, x):
    if x == 0x0204: QDSEEN.append(not any(s.mem[0xE12000:0xE1A000]))
    return _ot(s, x)
t3.AppCPU.tool = _t
cpu = boot_app(ADDR)
cpu.mem[0xE12000:0xE1A000] = scr
run(cpu, 30_000_000, stop_at=ADDR+lbl['spl_step'])
m = cpu.mem
bp = cpu.rd16(ADDR+lbl['BOOTP']); sp = cpu.rd16(ADDR+lbl['SPLP'])
check(not any('GSOS95.SPL' in p for p in OPENED), 'GSOS95 liest die Datei nicht neu (Bild vor QDStartUp gesichert)')
check(QDSEEN == [True], 'vor QDStartUp: Schirm, SCBs und Farben schwarz (kein Farbsalat)')
check(bp > 2 and sp == bp, 'Balkenstand uebernommen (Boot %d, Start %d)' % (bp, sp))
app = bytes(m[0xE12000:0xE1A000])
check(app[:166*160] == SPL[:166*160] and app[32000:32166] == SPL[32000:32166] and app[32172:32200] == SPL[32172:32200], 'GSOS95 zeigt dasselbe Bild')
check(app[167*160:171*160] == scr[167*160:171*160], 'Balken gleich wie beim Booten')
render(cpu, 'spl_pic.png', 3)
run(cpu, 60_000_000, stop_at=ML)
pal = bytes(m[0xE19E00:0xE1A000])
check(not any(m[0xE19D00:0xE19DC8]) and cpu.rd16(ADDR+lbl['SPLON']) == 0, 'Schreibtisch: SCBs zurueck')
check(pal == bytes(m[ADDR+lbl['palettes']:ADDR+lbl['palettes']+512]), 'Schreibtisch: Paletten zurueck')
render(cpu, 'spl_desk.png', 3)
# Hauptweg (BOOT95/START.GS.OS laden das ganze Wolkenbild): Balken bei etwa der Haelfte
from emu import CPU as _C
d = open('sys/START.GS.OS.95', 'rb').read()
c = _C(None); c.mem[0x6800:0x6800 + len(d)] = d; c.mem[0xE12000:0xE1A000] = SPL
drv = [0x20, 0x72, 0x75] * 100 + [0x80, 0xFE]
c.mem[0x0300:0x0300 + len(drv)] = bytes(drv)
c.E = 0; c.M = 0; c.XF = 0; c.D = 0x0800; c.S = 0x01FF; c.DBR = 0; c.PBR = 0; c.PC = 0x0300; c.mem[0x0802] = 0xE1
run(c, 80_000_000, stop_at=0x0300 + len(drv) - 2)
half = bytes(c.mem[0xE12000:0xE1A000])
cpu = boot_app(ADDR)
cpu.mem[0xE12000:0xE1A000] = half
run(cpu, 30_000_000, stop_at=ADDR + lbl['spl_step'])
bp = cpu.rd16(ADDR + lbl['BOOTP']); sp = cpu.rd16(ADDR + lbl['SPLP'])
check(bp > 10 and sp == bp, 'Wolkenbild: Balken laeuft nahtlos weiter (Boot %d, Start %d)' % (bp, sp))
app = bytes(cpu.mem[0xE12000:0xE1A000])
check(app[:32000] == half[:32000] and app[32000:32200] == half[32000:32200], 'Wolkenbild: GSOS95 zeigt genau das Bootbild')
render(cpu, 'spl_half.png', 3)
print('ALLES OK' if ok else 'FEHLER')
