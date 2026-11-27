# v2.18: Bluescreen mit Wahl-Apfel-C, Taste zurueck
from twbase import *
import twbase
cpu = boot(); settle()
cpu.events.append((0, 300, 20, 0)); settle()
desk = bytes(cpu.mem[0xE12000:0xE1A000])
cpu.events.append((3, 300, 20, ord('c'), 0x0900)); run(cpu, 6_000_000)
check(cpu.mem[0xE19E00:0xE19E02] == b'\x0a\x00' and cpu.mem[0xE12000 + 5 * 160] == 0, 'Wahl-Apfel-C: blauer Schirm')
txt = sum(1 for b in cpu.mem[0xE12000 + 64 * 160:0xE12000 + 180 * 160] if b & 0xF0 == 0x10 or b & 0x0F == 1)
check(txt > 300, 'Text zu sehen (%d)' % txt)
render(cpu, 'tbsod.png', 3)
cpu.events.append((3, 300, 20, ord(' '), 0)); settle(); settle()
check(bytes(cpu.mem[0xE19D00:0xE1A000]) == desk[0x7D00:] and bytes(cpu.mem[0xE12000:0xE19D00]) == desk[:0x7D00], 'Taste: Schreibtisch wie vorher')
cpu.events.append((3, 300, 20, 3, 0x0100 | 0x0200)); run(cpu, 6_000_000)
check(cpu.mem[0xE19E00:0xE19E02] == b'\x0a\x00', 'Strg-Apfel-C: ebenfalls Bluescreen')
cpu.events.append((1, 300, 20, 0)); settle(); settle()
check(bytes(cpu.mem[0xE19D00:0xE1A000]) == desk[0x7D00:], 'Klick: zurueck')
print('ALLES OK' if twbase.ok else 'FAIL')
