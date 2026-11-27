# START.GS.OS im Emulator: GS/OS-Aufrufe mit Parametern hinter JSL $E100A8
from emu import CPU
class BootCPU(CPU):
    """GS/OS-Aufrufe mit Parametern hinter dem JSL ($E100A8): Open/Read/Close"""
    files = {}
    def step(s):
        if (s.PBR << 16 | s.PC) == 0xE100A8:
            lo = s.pull(); hi = s.pull(); bank = s.pull()
            ret = (bank << 16) | (hi << 8 | lo)
            num = s.rd16(ret + 1); pb = s.rd16(ret + 3) | s.rd(ret + 5) << 16
            err = 0
            if num == 0x2010:
                pp = s.rd16(pb + 4) | s.rd(pb + 6) << 16
                n = s.rd16(pp); name = bytes(s.mem[pp + 2:pp + 2 + n]).decode().upper()
                if name in s.files: s.wr16(pb + 2, 7); s.cur = s.files[name]; s.pos = 0
                else: err = 0x46
            elif num == 0x2012:
                buf = s.rd16(pb + 4) | s.rd16(pb + 6) << 16; n = s.rd16(pb + 8) | s.rd16(pb + 10) << 16
                d = s.cur[s.pos:s.pos + n]; s.mem[buf:buf + len(d)] = d; s.pos += len(d)
                s.wr16(pb + 12, len(d) & 0xFFFF); s.wr16(pb + 14, len(d) >> 16)
                err = 0 if d else 0x4C
            elif num == 0x2014: pass
            else: err = 1
            s.calls = getattr(s, 'calls', []) + [num]
            s.PC = (ret + 7) & 0xFFFF; s.PBR = bank; s.A = err; s.C = 1 if err else 0
            return
        return super().step()
