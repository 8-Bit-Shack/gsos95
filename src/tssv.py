# v2.18: Bildschirmschoner abwechselnd Linienspiel (XCODE4) und Sternenfeld
import sys; sys.path.insert(0,'.')
from t3 import *
import t3
from emu import run, render
lbl = {l.split()[2].lstrip('.'): int(l.split()[1],16) for l in open('app.lbl')}
ADDR=0x030400; ML = ADDR + lbl['mainloop']
cpu = boot_app(ADDR)
ok = True
def check(c, m):
    global ok; print(('ok  ' if c else 'FEHLER ') + m); ok = ok and c
def settle(maxn=8_000_000):
    run(cpu, 30)
    while True:
        if run(cpu, maxn, stop_at=ML) and not cpu.events: return
settle()
desk = bytes(cpu.mem[0xE12000:0xE1A000])
cpu.mem[ADDR+lbl['SSMIN']] = 1
cpu.tickofs = 5000
run(cpu, 6_000_000)
pc = cpu.PBR << 16 | cpu.PC
check(0x300000 <= pc < 0x310000, 'erst das Linienspiel (fuenfte Codebank), PC %06X' % pc)
scr = cpu.mem[0xE12000:0xE19D00]
cols = set(); [cols.update((b >> 4, b & 15)) for b in scr]
check(len(cols - {0}) >= 2 and sum(1 for b in scr if b) > 300, 'Linien zu sehen (Farben %s)' % sorted(cols))
check(all(cpu.mem[0xE19D00 + y] == 0 for y in range(200)), 'alle Zeilen Palette 0')
render(cpu, 'tssv_a.png', 3)
run(cpu, 3_000_000)
render(cpu, 'tssv_b.png', 3)
cpu.events.append((3,160,100,0x20)); settle(); settle()
check(bytes(cpu.mem[0xE19D00:0xE1A000]) == desk[0x7D00:], 'Taste: Farben und SCBs wie vorher')
check(bytes(cpu.mem[0xE12000:0xE19D00]) == desk[:0x7D00], 'Schreibtisch wieder da')
cpu.tickofs += 5000
run(cpu, 3_000_000)
pc = cpu.PBR << 16 | cpu.PC
check(cpu.mem[ADDR+lbl['SSALT']] == 0 and not 0x300000 <= pc < 0x310000 and cpu.PC != ML & 0xFFFF, 'beim naechsten Mal: Sternenfeld (PC %06X)' % pc)
render(cpu, 'tssv_c.png', 3)
cpu.events.append((3,160,100,0x20)); settle(); settle()
print('ALLES OK' if ok else 'FEHLER')
