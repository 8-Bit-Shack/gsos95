# Ganzer Startweg ab PRODOS: START.GS.OS laden, (PHOENIX ja/nein), START bis zum ersten Toolaufruf.
# Erwartet: Startbild im SHR-Speicher, SHR an, Marke gesetzt - bevor der Balken anlaeuft.
import sys; sys.path.insert(0, '.')
sys.path.insert(0, 'tools')
from emu import CPU, run
from pdtool import Vol
IMG = sys.argv[1] if len(sys.argv) > 1 else __import__('os').environ.get('GSOS95_IMG', '/home/claude/out/GSOS95_v2147_System.hdv')
img = open(IMG, 'rb').read()
SPL = open('sys/GSOS95.SPL', 'rb').read()
v = Vol(img); pd = v.read(v.lookup('PRODOS')[2])[0]
class Stop(Exception): pass
class P(CPU):
    def step(s):
        pc = s.PBR << 16 | s.PC
        if pc == 0x0022BE:
            blk = s.rd16(0x46); buf = s.rd16(0x44)
            s.mem[buf:buf + 512] = img[blk * 512:blk * 512 + 512]
            ret = s.pull16(); s.PC = (ret + 1) & 0xFFFF; s.C = 0; s.A = 0; return
        if pc == 0x006800: s.started = True
        if pc in (0xE10000, 0xE100A8): raise Stop('%06X X=%04X' % (pc, s.X))
        if s.started and 0x7570 <= s.PC <= 0x7600 and s.PBR == 0: raise Stop('Balken %04X' % s.PC)
        return super().step()
ok = True
for phx in (True, False):
    c = P(None); c.started = False
    c.mem[0x2000:0x2000 + len(pd)] = pd
    if not phx: c.mem[0x2509] = 0x60       # PHOENIX-Weg faellt aus
    c.mem[0xE12000:0xE1A000] = bytes([0x11]) * 0x8000; c.mem[0xC029] = 0x01
    c.mem[0x0300:0x0303] = bytes([0x4C, 0x34, 0x24])
    c.E = 0; c.M = c.XF = 0; c.PBR = 0; c.DBR = 0; c.D = 0; c.S = 0x01FF; c.PC = 0x0300
    try: run(c, 80_000_000)
    except Stop as e: where = str(e)
    scr = bytes(c.mem[0xE12000:0xE1A000])
    good = c.started and scr[:0x7DFE] == SPL[:0x7DFE] and c.mem[0xC029] & 0x80 and (c.mem[0xE19DFE] | c.mem[0xE19DFF] << 8) == 0x3539
    print(('OK   ' if good else 'FAIL ') + 'Startweg (%s): Halt bei %s, Bild %s, C029 %02X' % ('Boot95' if phx else 'ohne Boot95', where, scr[:32000] == SPL[:32000], c.mem[0xC029]))
    ok = ok and good
sys.exit(0 if ok else 1)
