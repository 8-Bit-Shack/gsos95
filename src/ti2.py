# GSOS95.INIT: Explorer-Dialoge (SFGetFile2/SFPutFile2) im Simulator
import sys, struct; sys.path.insert(0, '.')
from emu import run
import t3
from t3 import AppCPU, FS, FILES

INIT = open('init95/GSOS95.INIT', 'rb').read()
BASE = 0x060000
lbl = {l.split()[2].lstrip('.'): int(l.split()[1], 16) for l in open('init95/init95.lbl')}

# Typen fuer die Testdateien
FS[':SYSTEM.DISK'] = [('SYSTEM', 0x0F), ('ICONS', 0x0F), ('BASIC.SYSTEM', 0xFF), ('README', 0x04),
                      ('FINDER', 0xB3), ('Brief', 0x50), ('apfel', 0x04)]
STRINGS = []
t3.DEVINFO.update({1: (0x0080, 1600, 4), 2: (0x0084, 1600, 4), 3: (0x0080, 65535, 5)})

class C(AppCPU):
    def __init__(s):
        super().__init__()
        s.wins = []; s.nextw = 0x071000; s.rects = []; s.texts = []; s.idle = 0
        s.port = 0x0A0000; s.pen = 0
        s.prefixes = {8: ':SYSTEM.DISK:'}
    def step(s):
        full = (s.PBR << 16) | s.PC
        if full == 0xE1F000:
            s.icount += 1; return s.tool2(s.X)
        if full == 0xE10000:
            t = s.mem[0xE10001] | s.mem[0xE10002] << 8 | s.mem[0xE10003] << 16
            s.PBR = t >> 16; s.PC = t & 0xFFFF; s.icount += 1; return
        return super().step()
    def gsos(s, num, pb):
        if num == 0x200A:
            n = s.rd16(pb + 2)
            return s.wrres(s.ptr(pb + 4), s.prefixes.get(n, ''))
        if num == 0x2009:
            s.prefixes[s.rd16(pb + 2)] = s.rdstr(s.ptr(pb + 4)); return 0
        return super().gsos(num, pb)
    def tool2(s, x):
        sp = s.S + 4
        def ret(nin, res=b''):
            for i, b in enumerate(res): s.mem[sp + nin + i] = b
            lo = s.pull(); hi = s.pull(); bank = s.pull(); s.S += nin
            s.PC = ((hi << 8 | lo) + 1) & 0xFFFF; s.PBR = bank; s.A = 0; s.C = 0
        if x in (0x090E,):
            w = s.nextw; s.nextw += 0x400; rec = w
            pl = s.ptr(sp)
            s.mem[rec:rec + 300] = bytes(300)
            struct.pack_into('<I', s.mem, rec + 174, 0x0E1000)
            struct.pack_into('<H', s.mem, rec + 210, s.rd16(pl + 2))
            t, l, b, r = struct.unpack_from('<4h', s.mem, pl + 62)
            s.winpos = (t, l, b, r)
            for off, (h, p, rr) in ((190, (rec + 0x300, rec + 0x310, (t - 12, l - 1, b + 1, r + 1))), (194, (rec + 0x308, rec + 0x320, (t, l, b, r)))):
                struct.pack_into('<I', s.mem, h, p); struct.pack_into('<HHHHH', s.mem, p, 10, *rr)
                struct.pack_into('<I', s.mem, rec + off, h)
            tp = s.ptr(pl + 4); s.wintitle = bytes(s.mem[tp + 1:tp + 1 + s.mem[tp]])
            s.wins.append(rec)
            return ret(4, struct.pack('<I', rec + 4))
        if x == 0x0B0E: s.closed = getattr(s, 'closed', 0) + 1; s.wins.pop(); return ret(4)
        if x == 0x0902:
            size = s.rd16(sp + 8) | s.rd16(sp + 10) << 16
            h = s.heap; s.heap += 0x10
            nxt = getattr(s, 'hn', 0x080000); s.hn = (nxt + max(size, 0x100) + 0xFF) & ~0xFF
            s.wr16(h, nxt & 0xFFFF); s.mem[h + 2] = nxt >> 16
            return ret(12, struct.pack('<I', h))
        if x == 0x1002: s.disposed = getattr(s, 'disposed', 0) + 1; return ret(4)
        if x == 0x1C04: return ret(0, struct.pack('<I', s.port))
        if x == 0x1B04: s.port = s.ptr(sp); return ret(4)
        if x == 0x200E: return ret(0, struct.pack('<I', 0x0C0000))
        if x == 0x1704: return ret(0, struct.pack('<H', getattr(s, 'mscb', 0)))
        if x == 0x1106: return ret(0, struct.pack('<I', 30))
        if x == 0x0E06: return ret(2, b'\0\0')
        if x == 0x2C03: s.beeps = getattr(s, 'beeps', 0) + 1; return ret(0)
        if x == 0x0A06:
            evp = s.ptr(sp); mods = 0
            if s.events:
                ev = s.events.pop(0); what, h, v, msg = ev[:4]
                if len(ev) > 4: mods = ev[4]
                s.idle = 0
            else:
                what, h, v, msg = 0, 0, 0, 0; s.idle += 1
                if s.idle > 50: raise Exception('keine Ereignisse mehr')
            rec = struct.pack('<HIIhhH', what, msg, s.icount // 2000, v, h, mods)
            s.mem[evp:evp + 16] = rec
            return ret(6, bytes([1 if what else 0, 0]))
        if x == 0x5404:
            p = s.ptr(sp); s.rects.append(struct.unpack_from('<4h', s.mem, p) + (s.pen,)); return ret(4)
        if x == 0x3704: s.pen = s.rd16(sp); return ret(2)
        if x == 0xA504:
            p = s.ptr(sp); s.texts.append(bytes(s.mem[p + 1:p + 1 + s.mem[p]])); return ret(4)
        if x == 0xA904: p = s.ptr(sp); return ret(4, struct.pack('<H', s.mem[p] * 6))
        if x in (0x220E,): return ret(6)
        if x in (0x2604, 0x5304, 0x2904, 0x3104, 0x3004, 0x1B04, 0x2504, 0x2404, 0x6804, 0x3A04, 0x3C04, 0x2C04): return ret(4)
        if x in (0x3004,): return ret(4)
        if x in (0xA004, 0xA204, 0x9C04, 0x9A04, 0x2E04): return ret(2)
        if x in (0xCA04,): return ret(0)
        if x in (0xA104, 0xA304, 0x9D04, 0x9B04, 0x2F04): return ret(0, b'\0\0')
        if x == 0x6704: s.nr = getattr(s, 'nr', 0) + 1; return ret(0, struct.pack('<I', 0x0B0000 + s.nr * 16))
        if x == 0xC904: return ret(0, struct.pack('<I', 0x0B0F00))
        if x in (0x6904, 0x6C04, 0x6F04): return ret(8)
        if x == 0x7304: return ret(12)
        if x == 0x060E: return ret(0, b'\x01\x00')
        if x in (0x520E, 0x150E): return ret(0, struct.pack('<I', s.wins[-1] + 4 if s.wins else 0))
        if x == 0x2A0E: return ret(4, struct.pack('<I', 0))
        if x == 0x0E0E:
            t = 0x07F000; s.mem[t] = 5; s.mem[t + 1:t + 6] = b'Titel'; return ret(4, struct.pack('<I', t))
        raise Exception('Tool %04X' % x)

cpu = C()
t3.load_omf(cpu, INIT, {1: BASE})
cpu.mem[0xE10000:0xE10004] = bytes([0x5C, 0x00, 0xF0, 0xE1])
pal = [0x000, 0x777, 0x841, 0x72C, 0x00F, 0x080, 0xF70, 0xD00, 0xFA9, 0xFF0, 0x0E0, 0x4DF, 0xDAF, 0x78F, 0xCCC, 0xFFF]
for i, c in enumerate(pal): struct.pack_into('<H', cpu.mem, 0xE19E00 + 2 * i, c)
code = bytes([0xC2, 0x30, 0x3B, 0x18, 0x69, 0x0E, 0x00, 0x1B, 0x18, 0x6B])   # Original-defProc: Parameter weg
cpu.mem[0x0E1000:0x0E1000 + len(code)] = code
STOP = 0x0F0000
def call(addr, a=0, n=30_000_000):
    cpu.push(STOP >> 16); cpu.push16((STOP - 1) & 0xFFFF)
    cpu.PBR = addr >> 16; cpu.PC = addr & 0xFFFF; cpu.A = a
    assert run(cpu, n, stop_at=STOP), 'kein Ruecksprung'
cpu.E = 0; cpu.M = cpu.XF = 0; cpu.S = 0x0BFF; cpu.D = 0x0800; cpu.DBR = 0
call(BASE, a=0x1234)
sfdp = cpu.rd16(BASE + lbl['sfdp'])
print('sfdp', hex(sfdp))

# Programm im Speicher: pusht die Parameter aus einer Tabelle und ruft das Tool
PROG = 0x0D0000
DATA = 0x0D8000
def sfcall(tool, words):
    """words: Liste 16-Bit-Werte in Push-Reihenfolge"""
    c = bytearray([0xC2, 0x30])
    for w in words: c += bytes([0xF4, w & 0xFF, w >> 8])
    c += bytes([0xA2, tool & 0xFF, tool >> 8, 0x22, 0x00, 0x00, 0xE1])
    c += bytes([0x8F, 0x00, 0x90, 0x0D, 0x6B])     # sta $0D9000 (Fehler), rtl
    cpu.mem[PROG:PROG + len(c)] = c
    S0 = cpu.S; cpu.DBR = 0x0D
    call(PROG)
    assert cpu.S == S0, 'Stapel %X statt %X' % (cpu.S, S0)
    return cpu.rd16(0x0D9000)

def pstr(addr, s):
    b = s.encode('latin-1'); cpu.mem[addr] = len(b); cpu.mem[addr + 1:addr + 1 + len(b)] = b; return addr
def reply_new(addr):
    cpu.mem[addr:addr + 20] = bytes(20)
    cpu.wr16(addr + 8, 3); cpu.wr16(addr + 14, 3)
    return addr
def outstr(addr, off):
    h = cpu.ptr(addr + off + 2); p = cpu.ptr(h)
    n = cpu.rd16(p + 2); return bytes(cpu.mem[p + 4:p + 4 + n]).decode('latin-1')
def lo(a): return a & 0xFFFF
def hi(a): return a >> 16

def entries():
    n = cpu.rd16(sfdp + 0x2E); lp = cpu.rd16(sfdp + 8) | cpu.mem[sfdp + 10] << 16
    idx = BASE + lbl['sfidx']; out = []
    for k in range(n):
        r = cpu.rd16(idx + 2 * k); e = lp + r * 72
        out.append((bytes(cpu.mem[e + 9:e + 9 + cpu.mem[e + 8]]).decode('latin-1'), cpu.rd16(e)))
    return out

CL, CT = 10, 36
def click(x, y): return [(1, CL + x, CT + y, 0), (2, CL + x, CT + y, 0)]
def row(r): return click(40, 26 + 10 * r)
def key(ch, mods=0): return [(3, 0, 0, ord(ch) if isinstance(ch, str) else ch, mods)]

ok = True
REP = reply_new(DATA)
PROMPT = pstr(DATA + 0x100, 'Welche Datei?')
# 1) Oeffnen: Buchstabe R springt zu README, Eingabe waehlt
cpu.events = key('r') + key(13)
err = sfcall(0x0E17, [0, 0, 0, hi(PROMPT), lo(PROMPT), 0, 0, 0, 0, hi(REP), lo(REP)])
ents = entries()
print('Liste:', ents)
print('Titel', cpu.wintitle, 'Fehler', err, 'good', cpu.rd16(REP), 'Typ', hex(cpu.rd16(REP + 2)),
      'Name', outstr(REP, 8), 'Pfad', outstr(REP, 14), 'Prefix8', cpu.prefixes.get(8))
ok &= [e[0] for e in ents] == ['ICONS', 'SYSTEM', 'apfel', 'BASIC.SYSTEM', 'Brief', 'FINDER', 'README']
ok &= err == 0 and cpu.rd16(REP) == 1 and cpu.rd16(REP + 2) == 4 and outstr(REP, 8) == 'README'
ok &= outstr(REP, 14) == ':SYSTEM.DISK:README' and b'Welche Datei?' in cpu.texts
ok &= getattr(cpu, 'closed', 0) == 1

# 2) Doppelklick auf SYSTEM (Zeile 1), dann Knopf "hoeher", dann Abbrechen
REP = reply_new(DATA + 0x200)
cpu.events = row(1) + row(1)
cpu.events += click(255, 9) + click(259, 130)
err = sfcall(0x0E17, [0, 0, 0, 0, 0, 0, 0, 0, 0, hi(REP), lo(REP)])
print('nach hoeher:', [e[0] for e in entries()][:3], 'good', cpu.rd16(REP), 'Fehler', err)
ok &= err == 0 and cpu.rd16(REP) == 0 and [e[0] for e in entries()][:2] == ['ICONS', 'SYSTEM']

# 3) Ordner betreten per Doppelklick, Datei per Doppelklick waehlen
REP = reply_new(DATA + 0x300)
cpu.events = row(1) + row(1) + row(3) + row(3)
err = sfcall(0x0E17, [0, 0, 0, 0, 0, 0, 0, 0, 0, hi(REP), lo(REP)])
print('SYSTEM-Liste:', [e[0] for e in entries()], 'Pfad', outstr(REP, 14))
ok &= cpu.rd16(REP) == 1 and outstr(REP, 14) == ':SYSTEM.DISK:SYSTEM:START' and cpu.prefixes[8] == ':SYSTEM.DISK:SYSTEM'
cpu.prefixes[8] = ':SYSTEM.DISK:'

# 4) Typliste: nur $B3 -> Ordner und FINDER
TL = DATA + 0x400
cpu.wr16(TL, 1); cpu.wr16(TL + 2, 0x8000); cpu.wr16(TL + 4, 0xB3)
REP = reply_new(DATA + 0x500)
cpu.events = key(27)
err = sfcall(0x0E17, [0, 0, 0, 0, 0, 0, 0, hi(TL), lo(TL), hi(REP), lo(REP)])
print('Typliste B3:', entries(), 'good', cpu.rd16(REP))
ok &= [e[0] for e in entries()] == ['ICONS', 'SYSTEM', 'FINDER'] and cpu.rd16(REP) == 0

# 5) Sichern: Vorgabename, Eingabe
ON = DATA + 0x600
cpu.wr16(ON, 7); cpu.mem[ON + 2:ON + 9] = b'NEU.TXT'
REP = reply_new(DATA + 0x700)
cpu.events = key(13)
err = sfcall(0x0F17, [0, 0, 0, 0, 0, 0, hi(ON), lo(ON), hi(REP), lo(REP)])
print('Sichern:', cpu.wintitle, 'good', cpu.rd16(REP), outstr(REP, 8), outstr(REP, 14), 'Fehler', err)
ok &= err == 0 and cpu.rd16(REP) == 1 and outstr(REP, 14) == ':SYSTEM.DISK:NEU.TXT'

# 6) Sichern: neuen Namen tippen (ersetzt die Vorgabe), Loeschen, ersetzen-Abfrage
REP = reply_new(DATA + 0x800)
cpu.events = key('R') + key('E') + key('A') + key('D') + key('M') + key('E') + key('X') + key(0x7F) + key(13) + key('n') + key(27)
err = sfcall(0x0F17, [0, 0, 0, 0, 0, 0, hi(ON), lo(ON), hi(REP), lo(REP)])
nf = BASE + lbl['namef']
print('Name im Feld:', bytes(cpu.mem[nf + 1:nf + 1 + cpu.mem[nf]]), 'good', cpu.rd16(REP), 'Ja/Nein-Texte' , b'Datei ersetzen?' in cpu.texts)
ok &= cpu.rd16(REP) == 0 and b'Datei ersetzen?' in cpu.texts and bytes(cpu.mem[nf + 1:nf + 7]) == b'README'
REP = reply_new(DATA + 0x900)
cpu.events = key('r') + key('e') + key('a') + key('d') + key('m') + key('e') + key(13) + key('j')
err = sfcall(0x0F17, [0, 0, 0, 0, 0, 0, hi(ON), lo(ON), hi(REP), lo(REP)])
print('Ersetzen ja:', cpu.rd16(REP), outstr(REP, 14))
ok &= cpu.rd16(REP) == 1 and outstr(REP, 14) == ':SYSTEM.DISK:readme'

# 7) Neuer Ordner (Sichern), dann Arbeitsplatz, Laufwerk GSOS95 oeffnen, Abbrechen
REP = reply_new(DATA + 0xA00)
cpu.events = click(276, 9) + click(150, 9) + row(0) + row(0) + key(27)
err = sfcall(0x0F17, [0, 0, 0, 0, 0, 0, hi(ON), lo(ON), hi(REP), lo(REP)])
print('Neuer Ordner:', [e[0] for e in FS[':SYSTEM.DISK']][-1], 'GSOS95:', entries())
ok &= FS[':SYSTEM.DISK'][-1] == ('NEUER.ORDNER', 0x0F) and [e[0] for e in entries()] == ['SPIELE', 'GSOS95', 'LIESMICH']

# 640er Modus: Fenster doppelt so breit, Klicks mit doppeltem x
cpu.mscb = 0x80
REP = reply_new(DATA + 0xB00)
cpu.events = [(1, 20 + 80, 36 + 26 + 60, 0), (2, 100, 122, 0), (1, 20 + 80, 36 + 26 + 60, 0)]
err = sfcall(0x0E17, [0, 0, 0, 0, 0, 0, 0, 0, 0, hi(REP), lo(REP)])
print('640:', cpu.winpos, 'good', cpu.rd16(REP), outstr(REP, 14))
ok &= cpu.winpos == (36, 20, 176, 620) and cpu.rd16(REP) == 1 and outstr(REP, 14).endswith(':FINDER')
cpu.mscb = 0
# 9) alter Aufruf SFGetFile: Typliste (1 Byte: $04), Pascal-Antwort
OTL = DATA + 0xC00
cpu.mem[OTL] = 1; cpu.mem[OTL + 1] = 0x04
OREP = DATA + 0xC10
cpu.mem[OREP:OREP + 160] = bytes(160)
cpu.events = key('r') + key(13)
err = sfcall(0x0917, [0, 0, hi(PROMPT), lo(PROMPT), 0, 0, hi(OTL), lo(OTL), hi(OREP), lo(OREP)])
nm = bytes(cpu.mem[OREP + 7:OREP + 7 + cpu.mem[OREP + 6]]); pth = bytes(cpu.mem[OREP + 23:OREP + 23 + cpu.mem[OREP + 22]])
print('SFGetFile alt:', [e[0] for e in entries()], cpu.rd16(OREP), hex(cpu.rd16(OREP + 2)), nm, pth, 'Fehler', err)
ok &= err == 0 and cpu.rd16(OREP) == 1 and nm == b'README' and pth == b':SYSTEM.DISK:README'
ok &= [e[0] for e in entries()] == ['ICONS', 'NEUER.ORDNER', 'SYSTEM', 'apfel', 'README']
# 10) alter Aufruf SFPutFile mit Pascal-Vorgabename
ONP = pstr(DATA + 0xD00, 'ALT.TXT')
cpu.mem[OREP:OREP + 160] = bytes(160)
cpu.events = key(13)
err = sfcall(0x0A17, [0, 0, hi(PROMPT), lo(PROMPT), hi(ONP), lo(ONP), 15, hi(OREP), lo(OREP)])
pth = bytes(cpu.mem[OREP + 23:OREP + 23 + cpu.mem[OREP + 22]])
print('SFPutFile alt:', cpu.rd16(OREP), pth, 'Fehler', err)
ok &= err == 0 and cpu.rd16(OREP) == 1 and pth == b':SYSTEM.DISK:ALT.TXT'
# 11) alter Aufruf mit eigener Filterprozedur: bleibt bei Apple
cpu.mem[0xC025] = 0
try:
    sfcall(0x0917, [0, 0, 0, 0, 0x0D, 0x1234, 0, 0, hi(OREP), lo(OREP)])
    ok = False
except Exception as e:
    print('Filter alt:', e); ok &= 'Tool 0917' in str(e)
# 8) Option gedrueckt: Aufruf geht an Apple (Original-Dispatcher)
cpu.mem[0xE0C025] = 0x40; cpu.mem[0xC025] = 0x40
try:
    sfcall(0x0E17, [0, 0, 0, 0, 0, 0, 0, 0, 0, hi(REP), lo(REP)])
    ok = False
except Exception as e:
    print('Option:', e); ok &= 'Tool 0E17' in str(e)
print('OK' if ok else 'FAIL Dialoge')
