# Minimaler 65816-Emulator + IIgs-Hardware-Stubs zum Testen von GS/OS95
import struct
from PIL import Image

class CPU:
    def __init__(s, img):
        s.mem = bytearray(0x1000000)
        s.img = img
        s.A = s.X = s.Y = 0
        s.S = 0x1FF; s.D = 0; s.DBR = 0; s.PBR = 0; s.PC = 0
        s.N = s.V = s.Z = s.C = 0
        s.M = s.XF = 1; s.Dm = 0; s.I = 1; s.E = 1
        s.icount = 0
        s.kbd = 0; s.mq = []; s.mphase = 0
        s.blockreads = 0

    # ---------------- Speicher ----------------
    def rd(s, a):
        a &= 0xFFFFFF
        if (a & 0xFFFF00) in (0x00C000, 0xE0C000):
            return s.io_rd(a & 0xFF)
        if a == 0x00C7FF: return 0xE0
        return s.mem[a]
    def wr(s, a, v):
        a &= 0xFFFFFF
        if (a & 0xFFFF00) in (0x00C000, 0xE0C000):
            s.io_wr(a & 0xFF, v); return
        s.mem[a] = v & 0xFF
    def io_rd(s, r):
        if r == 0x00: return s.kbd
        if r == 0x10: s.kbd &= 0x7F; return 0
        if r == 0x19: return 0x80 if (s.icount // 3000) & 1 else 0
        if r == 0x27:
            v = 0
            if s.mq: v |= 0x80
            if s.mphase: v |= 0x02
            return v
        if r == 0x24:
            if not s.mq: return 0
            if s.mphase == 0:
                s.mphase = 1; return s.mq[0][0]
            s.mphase = 0; return s.mq.pop(0)[1]
        return s.mem[0xC000 + r]
    def io_wr(s, r, v):
        if r == 0x10: s.kbd &= 0x7F
        s.mem[0xC000 + r] = v & 0xFF
    def rd16(s, a): return s.rd(a) | (s.rd(a + 1) << 8)
    def wr16(s, a, v): s.wr(a, v); s.wr(a + 1, v >> 8)

    def fetch(s):
        v = s.rd((s.PBR << 16) | s.PC); s.PC = (s.PC + 1) & 0xFFFF; return v
    def fetch16(s): lo = s.fetch(); return lo | (s.fetch() << 8)
    def fetch24(s): lo = s.fetch16(); return lo | (s.fetch() << 16)

    def push(s, v):
        s.wr(s.S, v)
        s.S = (s.S - 1) & 0xFFFF
        if s.E: s.S = 0x100 | (s.S & 0xFF)
    def pull(s):
        s.S = (s.S + 1) & 0xFFFF
        if s.E: s.S = 0x100 | (s.S & 0xFF)
        return s.rd(s.S)
    def push16(s, v): s.push(v >> 8); s.push(v)
    def pull16(s): lo = s.pull(); return lo | (s.pull() << 8)

    def getP(s):
        return (s.N << 7) | (s.V << 6) | (s.M << 5) | (s.XF << 4) | (s.Dm << 3) | (s.I << 2) | (s.Z << 1) | s.C
    def setP(s, p):
        s.N = (p >> 7) & 1; s.V = (p >> 6) & 1; s.M = (p >> 5) & 1; s.XF = (p >> 4) & 1
        s.Dm = (p >> 3) & 1; s.I = (p >> 2) & 1; s.Z = (p >> 1) & 1; s.C = p & 1
        if s.E: s.M = s.XF = 1
        if s.XF: s.X &= 0xFF; s.Y &= 0xFF

    # ---------------- Adressierung ----------------
    def ea(s, mode, wide_idx=None):
        db = s.DBR << 16
        if mode == 'dp': return (s.D + s.fetch()) & 0xFFFF
        if mode == 'dpx': return (s.D + s.fetch() + s.X) & 0xFFFF
        if mode == 'dpy': return (s.D + s.fetch() + s.Y) & 0xFFFF
        if mode == 'abs': return db | s.fetch16()
        if mode == 'absx': return (db | s.fetch16()) + s.X
        if mode == 'absy': return (db | s.fetch16()) + s.Y
        if mode == 'long': return s.fetch24()
        if mode == 'longx': return s.fetch24() + s.X
        if mode == 'ind':
            p = (s.D + s.fetch()) & 0xFFFF; return db | s.rd16(p)
        if mode == 'indy':
            p = (s.D + s.fetch()) & 0xFFFF; return (db | s.rd16(p)) + s.Y
        if mode == 'indx':
            p = (s.D + s.fetch() + s.X) & 0xFFFF; return db | s.rd16(p)
        if mode == 'indl':
            p = (s.D + s.fetch()) & 0xFFFF; return s.rd16(p) | (s.rd(p + 2) << 16)
        if mode == 'indly':
            p = (s.D + s.fetch()) & 0xFFFF; return (s.rd16(p) | (s.rd(p + 2) << 16)) + s.Y
        if mode == 'sr': return (s.S + s.fetch()) & 0xFFFF
        if mode == 'sry':
            p = (s.S + s.fetch()) & 0xFFFF; return (db | s.rd16(p)) + s.Y
        raise Exception('mode ' + mode)

    def ldm(s, mode, wide):
        if mode == 'imm':
            return s.fetch16() if wide else s.fetch()
        a = s.ea(mode)
        return s.rd16(a) if wide else s.rd(a)

    def nz(s, v, wide):
        if wide: s.N = (v >> 15) & 1; s.Z = int((v & 0xFFFF) == 0)
        else: s.N = (v >> 7) & 1; s.Z = int((v & 0xFF) == 0)

    # ---------------- Ausfuehrung ----------------
    def step(s):
        s.icount += 1
        full = (s.PBR << 16) | s.PC
        if full == 0x00C7E0:   # ProDOS-Blocktreiber
            cmd = s.mem[0x42]; buf = s.mem[0x44] | (s.mem[0x45] << 8)
            b = s.mem[0x46] | (s.mem[0x47] << 8)
            assert cmd == 1
            s.mem[buf:buf + 512] = s.img[b * 512:(b + 1) * 512]
            s.blockreads += 1
            s.C = 0; s.A = 0
            ret = s.pull16(); s.PC = (ret + 1) & 0xFFFF
            return
        if full == 0xE10000:   # Toolbox-Dispatcher
            assert s.X == 0x0D03, hex(s.X)
            base = s.S + 4
            vals = [5, 42, 9, 126, 14, 10, 0, 1]  # sec,min,hour,year,day,month,0,weekday
            for i, v in enumerate(vals): s.mem[base + i] = v
            lo = s.pull(); hi = s.pull(); bank = s.pull()
            s.PC = ((hi << 8 | lo) + 1) & 0xFFFF; s.PBR = bank; s.A = 0; s.C = 0
            return
        op = s.fetch()
        m16 = not s.M; x16 = not s.XF
        o = OPS.get(op)
        if o is None:
            raise Exception('Opcode %02X bei %06X' % (op, full))
        o(s, m16, x16)

def branch(cond):
    def f(s, m, x):
        off = s.fetch()
        if off & 0x80: off -= 256
        if cond(s): s.PC = (s.PC + off) & 0xFFFF
    return f

OPS = {}
MODES = {
    'imm': None, 'dp': 1, 'dpx': 1, 'dpy': 1, 'abs': 1, 'absx': 1, 'absy': 1, 'long': 1, 'longx': 1,
    'ind': 1, 'indy': 1, 'indx': 1, 'indl': 1, 'indly': 1, 'sr': 1, 'sry': 1}
GROUP = {  # ALU-Gruppen: opcode-basis je mode
    'ora': 0x00, 'and': 0x20, 'eor': 0x40, 'adc': 0x60, 'sta': 0x80, 'lda': 0xA0, 'cmp': 0xC0, 'sbc': 0xE0}
GOFF = {'indx': 0x01, 'sr': 0x03, 'dp': 0x05, 'indl': 0x07, 'imm': 0x09, 'abs': 0x0D, 'long': 0x0F,
        'indy': 0x11, 'ind': 0x12, 'sry': 0x13, 'dpx': 0x15, 'indly': 0x17, 'absy': 0x19, 'absx': 0x1D, 'longx': 0x1F}

def mk_alu(name, mode):
    def f(s, m, x):
        if name == 'sta':
            a = s.ea(mode)
            if m: s.wr16(a, s.A)
            else: s.wr(a, s.A)
            return
        v = s.ldm(mode, m)
        mask = 0xFFFF if m else 0xFF
        a = s.A & mask
        if name == 'lda': r = v
        elif name == 'ora': r = a | v
        elif name == 'and': r = a & v
        elif name == 'eor': r = a ^ v
        elif name == 'cmp':
            r = a - v; s.C = int(r >= 0); s.nz(r & mask, m); return
        elif name in ('adc', 'sbc') and getattr(s, 'Dm', 0):
            nd = 4 if m else 2
            da = int('%0*x' % (nd, a)); dv = int('%0*x' % (nd, v)); lim = 10 ** nd
            if name == 'adc':
                rr = da + dv + s.C; s.C = int(rr >= lim); rr %= lim
            else:
                rr = da - dv - (1 - s.C); s.C = int(rr >= 0); rr %= lim
            r = int(str(rr), 16)
        elif name == 'adc':
            r = a + v + s.C
            sign = 0x8000 if m else 0x80
            s.V = int(((a ^ r) & (v ^ r) & sign) != 0)
            s.C = int(r > mask); r &= mask
        elif name == 'sbc':
            r = a - v - (1 - s.C)
            sign = 0x8000 if m else 0x80
            s.V = int(((a ^ v) & (a ^ r) & sign) != 0)
            s.C = int(r >= 0); r &= mask
        s.nz(r, m)
        if m: s.A = r & 0xFFFF
        else: s.A = (s.A & 0xFF00) | (r & 0xFF)
    return f
for nm, base in GROUP.items():
    for md, off in GOFF.items():
        if nm == 'sta' and md == 'imm': continue
        OPS[base + off] = mk_alu(nm, md)

def mk_ldx(reg, mode):
    def f(s, m, x):
        v = s.ldm(mode, x); setattr(s, reg, v); s.nz(v, x)
    return f
for op, md in [(0xA2, 'imm'), (0xA6, 'dp'), (0xAE, 'abs'), (0xB6, 'dpy'), (0xBE, 'absy')]: OPS[op] = mk_ldx('X', md)
for op, md in [(0xA0, 'imm'), (0xA4, 'dp'), (0xAC, 'abs'), (0xB4, 'dpx'), (0xBC, 'absx')]: OPS[op] = mk_ldx('Y', md)
def mk_st(reg, mode, idx):
    def f(s, m, x):
        a = s.ea(mode)
        wide = x if idx else m
        v = 0 if reg == 'Z' else getattr(s, reg)
        if wide: s.wr16(a, v)
        else: s.wr(a, v)
    return f
for op, md in [(0x86, 'dp'), (0x8E, 'abs'), (0x96, 'dpy')]: OPS[op] = mk_st('X', md, True)
for op, md in [(0x84, 'dp'), (0x8C, 'abs'), (0x94, 'dpx')]: OPS[op] = mk_st('Y', md, True)
for op, md in [(0x64, 'dp'), (0x9C, 'abs'), (0x74, 'dpx'), (0x9E, 'absx')]: OPS[op] = mk_st('Z', md, False)
def mk_cpi(reg, mode):
    def f(s, m, x):
        v = s.ldm(mode, x); mask = 0xFFFF if x else 0xFF
        r = (getattr(s, reg) & mask) - v; s.C = int(r >= 0); s.nz(r & mask, x)
    return f
for op, md in [(0xE0, 'imm'), (0xE4, 'dp'), (0xEC, 'abs')]: OPS[op] = mk_cpi('X', md)
for op, md in [(0xC0, 'imm'), (0xC4, 'dp'), (0xCC, 'abs')]: OPS[op] = mk_cpi('Y', md)

def mk_rmw(kind, mode):
    def f(s, m, x):
        mask = 0xFFFF if m else 0xFF
        if mode == 'acc':
            v = s.A & mask
        else:
            a = s.ea(mode); v = s.rd16(a) if m else s.rd(a)
        top = 0x8000 if m else 0x80
        if kind == 'inc': r = (v + 1) & mask
        elif kind == 'dec': r = (v - 1) & mask
        elif kind == 'asl': s.C = int(bool(v & top)); r = (v << 1) & mask
        elif kind == 'lsr': s.C = v & 1; r = v >> 1
        elif kind == 'rol': r = ((v << 1) | s.C) & mask; s.C = int(bool(v & top))
        elif kind == 'ror': r = (v >> 1) | (top if s.C else 0); s.C = v & 1
        s.nz(r, m)
        if mode == 'acc':
            s.A = r if m else (s.A & 0xFF00) | r
        else:
            if m: s.wr16(a, r)
            else: s.wr(a, r)
    return f
for kind, ops in {'asl': (0x0A, 0x06, 0x0E, 0x16, 0x1E), 'rol': (0x2A, 0x26, 0x2E, 0x36, 0x3E),
                  'lsr': (0x4A, 0x46, 0x4E, 0x56, 0x5E), 'ror': (0x6A, 0x66, 0x6E, 0x76, 0x7E)}.items():
    for op, md in zip(ops, ('acc', 'dp', 'abs', 'dpx', 'absx')): OPS[op] = mk_rmw(kind, md)
for op, md in [(0x1A, 'acc'), (0xE6, 'dp'), (0xEE, 'abs'), (0xF6, 'dpx'), (0xFE, 'absx')]: OPS[op] = mk_rmw('inc', md)
for op, md in [(0x3A, 'acc'), (0xC6, 'dp'), (0xCE, 'abs'), (0xD6, 'dpx'), (0xDE, 'absx')]: OPS[op] = mk_rmw('dec', md)

def mk_bit(mode):
    def f(s, m, x):
        v = s.ldm(mode, m); mask = 0xFFFF if m else 0xFF
        s.Z = int((s.A & v & mask) == 0)
        if mode != 'imm':
            top = 15 if m else 7
            s.N = (v >> top) & 1; s.V = (v >> (top - 1)) & 1
    return f
for op, md in [(0x89, 'imm'), (0x24, 'dp'), (0x2C, 'abs'), (0x34, 'dpx'), (0x3C, 'absx')]: OPS[op] = mk_bit(md)

def idx_incdec(reg, d):
    def f(s, m, x):
        mask = 0xFFFF if x else 0xFF
        v = (getattr(s, reg) + d) & mask; setattr(s, reg, v); s.nz(v, x)
    return f
OPS[0xE8] = idx_incdec('X', 1); OPS[0xCA] = idx_incdec('X', -1)
OPS[0xC8] = idx_incdec('Y', 1); OPS[0x88] = idx_incdec('Y', -1)

def xfer(src, dst, kind):
    def f(s, m, x):
        v = getattr(s, src)
        if kind == 'x':   # Ziel ist Indexregister
            if x: v &= 0xFFFF
            else: v &= 0xFF
            setattr(s, dst, v); s.nz(v, x)
        elif kind == 'a':
            if m: s.A = v & 0xFFFF
            else: s.A = (s.A & 0xFF00) | (v & 0xFF)
            s.nz(s.A, m)
    return f
OPS[0xAA] = xfer('A', 'X', 'x'); OPS[0xA8] = xfer('A', 'Y', 'x')
OPS[0x8A] = xfer('X', 'A', 'a'); OPS[0x98] = xfer('Y', 'A', 'a')
OPS[0x9B] = xfer('X', 'Y', 'x'); OPS[0xBB] = xfer('Y', 'X', 'x')
def tcd(s, m, x): s.D = s.A; s.nz(s.D, True)
def tdc(s, m, x): s.A = s.D; s.nz(s.A, True)
def txs(s, m, x): s.S = s.X if not s.E else 0x100 | (s.X & 0xFF)
def tsx(s, m, x): s.X = s.S & (0xFFFF if x else 0xFF); s.nz(s.X, x)
def xba(s, m, x): s.A = ((s.A >> 8) | (s.A << 8)) & 0xFFFF; s.nz(s.A & 0xFF, False)
OPS[0x5B] = tcd; OPS[0x7B] = tdc; OPS[0x9A] = txs; OPS[0xBA] = tsx; OPS[0xEB] = xba

for op, cond in [(0x10, lambda s: not s.N), (0x30, lambda s: s.N), (0x50, lambda s: not s.V), (0x70, lambda s: s.V),
                 (0x90, lambda s: not s.C), (0xB0, lambda s: s.C), (0xD0, lambda s: not s.Z), (0xF0, lambda s: s.Z),
                 (0x80, lambda s: True)]:
    OPS[op] = branch(cond)
def brl(s, m, x):
    off = s.fetch16()
    if off & 0x8000: off -= 0x10000
    s.PC = (s.PC + off) & 0xFFFF
OPS[0x82] = brl

def jmp_abs(s, m, x): s.PC = s.fetch16()
def jmp_ind(s, m, x): a = s.fetch16(); s.PC = s.rd16(a)
def jmp_indx(s, m, x): a = s.fetch16(); s.PC = s.rd16((s.PBR << 16) | ((a + s.X) & 0xFFFF))
def jml(s, m, x): a = s.fetch24(); s.PC = a & 0xFFFF; s.PBR = a >> 16
def jsr(s, m, x): a = s.fetch16(); s.push16((s.PC - 1) & 0xFFFF); s.PC = a
def jsr_indx(s, m, x):
    a = s.fetch16(); s.push16((s.PC - 1) & 0xFFFF); s.PC = s.rd16((s.PBR << 16) | ((a + s.X) & 0xFFFF))
def jsl(s, m, x):
    a = s.fetch24(); s.push(s.PBR); s.push16((s.PC - 1) & 0xFFFF); s.PC = a & 0xFFFF; s.PBR = a >> 16
def rts(s, m, x): s.PC = (s.pull16() + 1) & 0xFFFF
def rtl(s, m, x): s.PC = (s.pull16() + 1) & 0xFFFF; s.PBR = s.pull()
OPS.update({0x4C: jmp_abs, 0x6C: jmp_ind, 0x7C: jmp_indx, 0x5C: jml, 0x20: jsr, 0xFC: jsr_indx, 0x22: jsl,
            0x60: rts, 0x6B: rtl})

def pha(s, m, x):
    if m: s.push16(s.A)
    else: s.push(s.A)
def pla(s, m, x):
    if m: s.A = s.pull16()
    else: s.A = (s.A & 0xFF00) | s.pull()
    s.nz(s.A, m)
def phr(reg):
    def f(s, m, x):
        if x: s.push16(getattr(s, reg))
        else: s.push(getattr(s, reg))
    return f
def plr(reg):
    def f(s, m, x):
        v = s.pull16() if x else s.pull(); setattr(s, reg, v); s.nz(v, x)
    return f
def php(s, m, x): s.push(s.getP())
def plp(s, m, x): s.setP(s.pull())
def phb(s, m, x): s.push(s.DBR)
def plb(s, m, x): s.DBR = s.pull(); s.nz(s.DBR, False)
def phk(s, m, x): s.push(s.PBR)
def phd(s, m, x): s.push16(s.D)
def pld(s, m, x): s.D = s.pull16(); s.nz(s.D, True)
OPS.update({0x48: pha, 0x68: pla, 0xDA: phr('X'), 0xFA: plr('X'), 0x5A: phr('Y'), 0x7A: plr('Y'),
            0x08: php, 0x28: plp, 0x8B: phb, 0xAB: plb, 0x4B: phk, 0x0B: phd, 0x2B: pld})

def rep(s, m, x): s.setP(s.getP() & ~s.fetch())
def sep(s, m, x): s.setP(s.getP() | s.fetch())
def xce(s, m, x):
    c = s.C; s.C = s.E; s.E = c
    if s.E: s.M = s.XF = 1; s.X &= 0xFF; s.Y &= 0xFF; s.S = 0x100 | (s.S & 0xFF)
    else: s.M = s.XF = 1
def flag(attr, v):
    def f(s, m, x): setattr(s, attr, v)
    return f
OPS.update({0xC2: rep, 0xE2: sep, 0xFB: xce, 0x18: flag('C', 0), 0x38: flag('C', 1), 0x78: flag('I', 1),
            0x58: flag('I', 0), 0xD8: flag('Dm', 0), 0xF8: flag('Dm', 1), 0xB8: flag('V', 0), 0xEA: lambda s, m, x: None})


def render(cpu, path, scale=4):
    mem = cpu.mem
    img = Image.new('RGB', (320, 200))
    px = img.load()
    shr = bool(mem[0xC029] & 0x80)
    for y in range(200):
        scb = mem[0xE19D00 + y]
        pal = 0xE19E00 + (scb & 15) * 32
        cols = []
        for i in range(16):
            w = mem[pal + i * 2] | (mem[pal + i * 2 + 1] << 8)
            cols.append((((w >> 8) & 15) * 17, ((w >> 4) & 15) * 17, (w & 15) * 17))
        base = 0xE12000 + y * 160
        for xb in range(160):
            b = mem[base + xb]
            px[xb * 2, y] = cols[b >> 4]
            px[xb * 2 + 1, y] = cols[b & 15]
    img = img.resize((320 * scale, int(200 * scale * 1.2)), Image.NEAREST)
    img.save(path)
    return shr


def boot(imgpath):
    data = open(imgpath, 'rb').read()
    if data[:4] == b'2IMG': data = data[64:]
    cpu = CPU(data)
    cpu.mem[0x800:0xA00] = data[0:512]
    cpu.X = 0x70; cpu.PC = 0x801; cpu.S = 0x1FF
    return cpu

def run(cpu, n, stop_at=None):
    for _ in range(n):
        cpu.step()
        if stop_at is not None and ((cpu.PBR << 16) | cpu.PC) == stop_at:
            return True
    return False

def pea(s, m, x): s.push16(s.fetch16())
def pei(s, m, x): a = (s.D + s.fetch()) & 0xFFFF; s.push16(s.rd16(a))
OPS.update({0xF4: pea, 0xD4: pei})

def mvn(s, m, x):
    dst = s.fetch(); src = s.fetch()
    n = (s.A & 0xFFFF) + 1
    for i in range(n):
        s.mem[(dst << 16) | ((s.Y + i) & 0xFFFF)] = s.mem[(src << 16) | ((s.X + i) & 0xFFFF)]
    s.X = (s.X + n) & 0xFFFF; s.Y = (s.Y + n) & 0xFFFF; s.A = 0xFFFF; s.DBR = dst
OPS[0x54] = mvn

def mvp(s, m, x):
    dst = s.fetch(); src = s.fetch()
    n = (s.A & 0xFFFF) + 1
    for i in range(n):
        s.mem[(dst << 16) | ((s.Y - i) & 0xFFFF)] = s.mem[(src << 16) | ((s.X - i) & 0xFFFF)]
    s.X = (s.X - n) & 0xFFFF; s.Y = (s.Y - n) & 0xFFFF; s.A = 0xFFFF; s.DBR = dst
OPS[0x44] = mvp

def tcs(s, m, x): s.S = s.A & 0xFFFF
def tsc(s, m, x): s.A = s.S & 0xFFFF; s.nz(s.A, False)
OPS.update({0x1B: tcs, 0x3B: tsc})

def mk_tsb(setb, mode):
    def f(s, m, x):
        mask = 0xFFFF if m else 0xFF
        a = s.ea(mode); v = s.rd16(a) if m else s.rd(a)
        acc = s.A & mask; s.Z = int((v & acc) == 0)
        r = (v | acc) if setb else (v & ~acc & mask)
        if m: s.wr16(a, r)
        else: s.wr(a, r)
    return f
for op, md, sb in [(0x04, 'dp', 1), (0x0C, 'abs', 1), (0x14, 'dp', 0), (0x1C, 'abs', 0)]:
    OPS.setdefault(op, mk_tsb(sb, md))
