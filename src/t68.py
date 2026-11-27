# v2.14: Explorer - Apfel-Pfeile, Papierkorb mit Fortschritt/Wahltaste, Systemordner einsortieren, Klang
import sys, struct; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
B = ADDR & 0xFF0000
# Testdaten: Schrift und Treiber zum Einsortieren, AIFF-Klang, Rohklang
t3.FS[':SYSTEM.DISK'][5] = ('MYFONT', 0xC8)
t3.FS[':SYSTEM.DISK'][6] = ('MYDRV', 0xBB)
t3.FS[':SYSTEM.DISK:SYSTEM:FONTS'] = []
t3.FILES[':SYSTEM.DISK:MYFONT'] = b'FONT' * 10
t3.FILES[':SYSTEM.DISK:MYDRV'] = b'DRV' * 10
def ext80(rate):
    import math
    e = int(math.floor(math.log2(rate))); m = int(rate * 2 ** (63 - e))
    return struct.pack('>HQ', 16383 + e, m)
samples = bytes([0x00, 0x7F, 0x80, 0xFF, 0x10] * 60)          # mit Vorzeichen
comm = b'COMM' + struct.pack('>IhIh', 18, 1, len(samples), 8) + ext80(22050)
ssnd = b'SSND' + struct.pack('>III', len(samples) + 8, 0, 0) + samples
aiff = b'FORM' + struct.pack('>I', 4 + len(comm) + len(ssnd)) + b'AIFF' + comm + ssnd
t3.FS[':SYSTEM.DISK'] += [('KLANG', 0xD8), ('ROH', 0xD8), ('QUACK', 0xD8)]
t3.FILES[':SYSTEM.DISK:QUACK'] = b''
t3.FILES[':SYSTEM.DISK:QUACK#R'] = open('/tmp/quack.rsrc','rb').read()
t3.FILES[':SYSTEM.DISK:KLANG'] = aiff
t3.FILES[':SYSTEM.DISK:ROH'] = bytes([0x80, 0x00, 0x40] * 100)
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c, mods=0):
    cpu.events.append((3,160,100,c,mods)); settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def drag(a,b,shot=None):
    (h0,v0),(h1,v1)=a,b
    cpu.events += [(0,h0,v0,0)]; settle()
    ev=[(1,h0,v0,0)]
    for i in range(1,9):
        ev.append((0,h0+(h1-h0)*i//8, v0+(v1-v0)*i//8,0))
    cpu.events += ev
    run(cpu, 3_000_000)
    cpu.events += [(2,h1,v1,0)]; settle(); settle()
    if shot: render(cpu, shot, 3)
def names(k): return [e[0] for e in t3.FS.get(k, [])]
def rw(a):
    a = a if isinstance(a, int) else a
    return cpu.mem[a] | cpu.mem[a+1] << 8
DPV = {'SEL2': 0x1E, 'WIN': 0xB4, 'CONFACT': 0x7C, 'MODE': 0x18}
class L(dict):
    def __getitem__(s, k): return 0x800 + DPV[k] if k in DPV else ADDR + lbl[k]
LB = L()
ok = True
def check(c, msg):
    global ok
    print(('OK   ' if c else 'FAIL ') + msg)
    ok = ok and c

settle()
dclick(28,14); key(0x15); key(13)                     # SYSTEM.DISK
# ---- Apfel-Pfeil runter oeffnet SYSTEM, Apfel-Pfeil hoch markiert SYSTEM wieder
def path(): return bytes(cpu.mem[LB['pathbuf']+2:LB['pathbuf']+2+rw(LB['pathbuf'])]).decode()
key(0x15)
key(0x0A, 0x0100)
check(path() == ':SYSTEM.DISK:SYSTEM', 'Apfel-Runter oeffnet SYSTEM (%s)' % path())
key(0x0B, 0x0100)
check(path() == ':SYSTEM.DISK' and rw(LB['SEL2']) == 0, 'Apfel-Hoch zurueck, SYSTEM markiert (SEL2=%d)' % rw(LB['SEL2']))
t3.FS[':SYSTEM.DISK:ICONS'] = [('X', 4)]
key(0x15); key(13)
check(path() == ':SYSTEM.DISK:ICONS', 'ICONS offen')
key(0x7F)
check(rw(LB['SEL2']) == 1, 'Delete: ICONS markiert (SEL2=%d)' % rw(LB['SEL2']))
key(0x0B, 0x0100)
check(rw(LB['MODE']) == 0 and rw(LB['SEL2']) == 0, 'Arbeitsplatz: SYSTEM.DISK markiert')
render(cpu, 'x_up.png', 3)
key(13)
render(cpu, 'x_pre.png', 3)
# ---- Systemordner: MYFONT (Index 5) auf SYSTEM ziehen -> Rueckfrage -> Ja
drag((248,100),(176,44),'x_sys.png')
check(rw(LB['WIN']) == 12 and rw(LB['CONFACT']) == 6, 'Rueckfrage einsortieren')
key(ord('j'))
check('MYFONT' in names(':SYSTEM.DISK:SYSTEM:FONTS'), 'MYFONT in SYSTEM:FONTS ' + str(names(':SYSTEM.DISK:SYSTEM:FONTS')))
check('MYFONT' not in names(':SYSTEM.DISK'), 'MYFONT nicht mehr im Hauptordner')
# MYDRV ist nachgerutscht (Index 5): Nein -> direkt in SYSTEM
drag((248,100),(176,44))
key(ord('n'))
check('MYDRV' in names(':SYSTEM.DISK:SYSTEM'), 'Nein: MYDRV direkt in SYSTEM ' + str(names(':SYSTEM.DISK:SYSTEM')))
check(not any(k.upper().endswith(':DRIVERS') for k in t3.FS), 'kein DRIVERS angelegt')
# ---- Klang: KLANG oeffnen (Return)
nm = names(':SYSTEM.DISK'); i = nm.index('KLANG')
key(0x15)
for _ in range(rw(LB['SEL2']), i): key(0x15)
check(rw(LB['SEL2']) == i, 'KLANG markiert')
key(13)
fs = getattr(cpu, 'ffsound', None)
check(fs is not None, 'FFStartSound aufgerufen')
if fs:
    gen, pb = fs
    start, pages, freq = struct.unpack('<IHH', pb[:8])
    data = bytes(cpu.mem[start:start + len(samples) + 4])
    exp = bytes(((b ^ 0x80) or 1) for b in samples)
    check(gen == 0x0101 and pages == (len(samples) + 255) // 256, 'Generator %04X, Seiten %d' % (gen, pages))
    check(freq == 22050 // 51, 'Frequenz %d' % freq)
    check(data[:len(samples)] == exp and data[len(samples)] == 0x80, 'AIFF-Daten umgesetzt, Nullen ersetzt, Stille danach')
# Rohklang: 11 kHz, Nullen ersetzt
key(0x15); key(13)
gen, pb = cpu.ffsound
start, pages, freq = struct.unpack('<IHH', pb[:8])
raw = bytes(cpu.mem[start:start + 300])
check(freq == 11025 // 51 and raw == bytes([0x80, 0x01, 0x40] * 100), 'Rohdaten 11 kHz (%d)' % freq)
# Klang im Ressourcenzweig (rSoundSample wie in *:System:Sounds)
key(0x15); key(13)
gen, pb = cpu.ffsound
start, pages, freq = struct.unpack('<IHH', pb[:8])
q = open('/tmp/quack.rsrc','rb').read()[498+10:498+2082]
qe = bytes((b or 1) for b in q)
check(freq == 11127 // 51 and pages == (len(q) + 255) // 256 and bytes(cpu.mem[start:start+len(q)]) == qe, 'rSoundSample: Rate %d Seiten %d' % (freq, pages))
check(cpu.toolcalls.get(0x0F08, 0) >= 1 and cpu.toolcalls.get(0x1002, 0) >= 1, 'alter Klang angehalten und freigegeben')
# ---- Papierkorb: drei Dateien loeschen, dann mit Wahltaste leeren
for n in ('README', 'DATEI.03', 'DATEI.04'):
    nm = names(':SYSTEM.DISK'); i = nm.index(n)
    key(0x15)
    for _ in range(rw(LB['SEL2']), i): key(0x15)
    for _ in range(i, rw(LB['SEL2'])): key(0x08)
    key(0x04); key(ord('j'))
check(len(names(':SYSTEM.DISK:RECYCLED')) == 3, 'drei Dateien im Papierkorb')
key(0x1B)
dclick(28,50)                                          # Papierkorb
key(0x06); render(cpu, 'x_binmenu.png', 3)
key(13, 0x0800)                                        # "Papierkorb leeren" mit Wahltaste
check(':SYSTEM.DISK:RECYCLED' not in t3.FS, 'Papierkorb ohne Rueckfrage geleert')
check(rw(LB['PRGM']) == 2 and rw(LB['PRGN']) == 3 and rw(LB['PRGI']) == 3, 'Fortschritt Loeschen 3/3 (M%d N%d I%d)' % (rw(LB['PRGM']), rw(LB['PRGN']), rw(LB['PRGI'])))
render(cpu, 'x_binempty.png', 3)
print('ALLES OK' if ok else 'FEHLER')
