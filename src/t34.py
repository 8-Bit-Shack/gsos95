# BASIC starten, Dateityp aendern, Datentraeger kopieren, Diskette einlegen
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
part = sys.argv[1] if len(sys.argv) > 1 else 'all'
t3.FS[':SYSTEM.DISK'] = [('BASIC.SYSTEM',0xFF),('HALLO',0xFC),('BILD',0x06)]
bs = bytearray(10240); bs[0:6] = bytes([0x4C,0x00,0x21,0xEE,0xEE,0x41])
t3.FILES[':SYSTEM.DISK:BASIC.SYSTEM'] = bytes(bs)
t3.FILES[':SYSTEM.DISK:BILD'] = b'x'*100
cpu = boot_app(ADDR)
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
def key(c, mods=0):
    cpu.events.append((3,160,100,c,mods) if mods else (3,160,100,c)); settle(); settle()
def click(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def dclick(h,v):
    cpu.events += [(0,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle()
    cpu.events += [(1,h,v,0),(2,h,v,0)]; settle(); settle()
def org():
    D = cpu.rd16(ADDR + lbl['MYDP']); s=lambda v: v-65536 if v>32767 else v
    return 40 + s(cpu.rd16(D + 8)), 16 + s(cpu.rd16(D + 0x0A))
def dp(o): return cpu.rd16(cpu.rd16(ADDR + lbl['MYDP']) + o)
settle()
dclick(28,14)                               # Arbeitsplatz
if part in ('all','type'):
    key(0x15); key(13)                      # SYSTEM.DISK
    key(0x15); key(0x15); key(0x15)         # BILD (3.)
    print('SEL2', dp(0x1E))
    key(6); key(0x0A); key(0x0A); key(0x0A); key(13)   # Datei > Eigenschaften
    x,y = org(); render(cpu,'ty1.png',3)
    click(x+240-62+20, y+44+5)              # Aendern
    render(cpu,'ty2.png',3)
    for ch in 'C1/0002': key(ord(ch))
    render(cpu,'ty3.png',3)
    key(13)
    print('BILD', [e for e in t3.FS[':SYSTEM.DISK'] if e[0]=='BILD'], 'aux', t3.AUX.get('BILD'))
    assert ('BILD',0xC1) in t3.FS[':SYSTEM.DISK'] and t3.AUX.get('BILD') == 2
    key(0x1B)
    render(cpu,'ty4.png',3)
if part in ('all','bas'):
    if part=='bas': key(0x15); key(13)
    while dp(0x1E) != 1: key(0x15)          # HALLO
    try:
        key(13)
    except StopIteration: pass
    print('Quit', cpu.quit)
    f = t3.FILES.get(':SYSTEM.DISK:SYSTEM:BASIC95.SYSTEM')
    print('Kopie', f[:30] if f else None, len(f or b''))
    assert cpu.quit[0].upper().endswith('SYSTEM:BASIC95.SYSTEM')
    assert f[6:6+19] == b'\x12/SYSTEM.DISK/HALLO', f[6:30]
def call(name):
    PBR,PC=cpu.PBR,cpu.PC
    ret=ADDR+lbl['code_end']
    cpu.push16((ret-1)&0xFFFF)
    cpu.PC=(ADDR+lbl[name])&0xFFFF
    run(cpu, 80_000_000, stop_at=ret)
    cpu.PBR,cpu.PC=PBR,PC
def jsl(name, bank=0x050000):
    PBR, PC, DBR = cpu.PBR, cpu.PC, cpu.DBR
    ret = ADDR + lbl['code_end']
    cpu.push(ADDR >> 16); cpu.push16((ret - 1) & 0xFFFF)
    cpu.PBR = (bank + lbl[name]) >> 16; cpu.PC = (bank + lbl[name]) & 0xFFFF; cpu.DBR = ADDR >> 16
    run(cpu, 80_000_000, stop_at=ret)
    cpu.PBR, cpu.PC, cpu.DBR = PBR, PC, DBR
if part == 'dcopy':
    t3.DEVS[2] = ('.APPLEDISK3.5B', ':ZIEL'); t3.FS[':ZIEL'] = []
    t3.DISKS[1] = bytearray((i * 7 + i // 512) & 0xFF for i in range(1600 * 512))
    key(0x12)                                # Strg+R neu einlesen
    key(0x15)                                # SYSTEM.DISK
    print('SEL2', dp(0x1E)); key(3)          # Strg+C
    key(0x15)                                # naechstes: ZIEL
    print('SEL2', dp(0x1E)); key(0x16)       # Strg+V
    render(cpu,'dc1.png',3)
    key(13)
    render(cpu,'dc2.png',3)
    print('gleich', t3.DISKS.get(2) == t3.DISKS[1], 'dio', getattr(cpu,'dio',0))
    assert t3.DISKS.get(2) == t3.DISKS[1]
if part == 'insert':
    jsl('xf_dpoll')                            # Anfangszustand merken
    t3.DEVS[2] = ('.APPLEDISK3.5B', 'UNF')   # unformatierte Diskette eingelegt
    jsl('xf_dpoll'); settle()
    render(cpu,'in1.png',3)
    win = dp(0xB4); print('WIN', win, 'TXTMODE', dp(0x6C))
    assert win == 11 and dp(0x6C) == 2
