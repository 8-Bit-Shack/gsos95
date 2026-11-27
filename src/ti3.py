# GSOS95.INIT: Win95-Knoepfe (Standardknopf-Prozedur) im Simulator
import sys, struct; sys.path.insert(0, '.')
exec(open('ti2.py').read().split("cpu = C()")[0])     # Klasse C und Hilfen aus ti2
class D(C):
    def tool2(s, x):
        sp = s.S + 4
        def ret(nin, res=b''):
            for i, b in enumerate(res): s.mem[sp + nin + i] = b
            lo = s.pull(); hi = s.pull(); bank = s.pull(); s.S += nin
            s.PC = ((hi << 8 | lo) + 1) & 0xFFFF; s.PBR = bank; s.A = 0; s.C = 0
        if x in (0x0910, 0x3110): return ret(30 if x == 0x0910 else 10, struct.pack('<I', s.newctl))
        if x == 0x2C10:
            h = s.ptr(sp + 6); p = s.ptr(h)
            s.cdp.append((s.rd16(sp + 4), struct.unpack_from('<I', s.mem, p + 20)[0]))
            return ret(10, struct.pack('<I', 2))
        if x == 0x0E1E: s.res_loaded = (s.rd16(sp + 4), s.ptr(sp)); return ret(6, struct.pack('<I', RH))
        if x == 0x171E: s.res_released = True; return ret(8)
        if x in (0x2B04, 0x2A04): return ret(4)
        if x == 0x150E: return ret(0, struct.pack('<I', WREC + 4))
        if x == 0x0A02:
            h = s.ptr(sp); s.reallocs = getattr(s, 'reallocs', 0) + 1
            s.wr16(h, 0x4000); s.mem[h + 2] = 0; s.mem[h + 3] = 0
            s.mem[0x004000:0x004300] = bytes([0xAA]) * 0x300
            return ret(16)
        if x == 0x060E: return ret(0, struct.pack('<H', getattr(s, 'wmon', 1)))
        if x == 0x5504: s.erased = struct.unpack_from('<4h', s.mem, s.ptr(sp)); return ret(4)
        if x == 0x0F15: s.modal = struct.unpack_from('<I', s.mem, B1 + 20)[0]; return ret(4, b'\x01\x00')
        if x == 0x3604: return ret(0)
        if x == 0x1C04: return ret(0, struct.pack('<I', 0x0C2000))
        return super().tool2(x)
cpu = D(); cpu.cdp = []
t3.load_omf(cpu, INIT, {1: BASE})
cpu.mem[0xE10000:0xE10004] = bytes([0x5C, 0x00, 0xF0, 0xE1])
pal = [0x000, 0x777, 0x841, 0x72C, 0x00F, 0x080, 0xF70, 0xD00, 0xFA9, 0xFF0, 0x0E0, 0x4DF, 0xDAF, 0x78F, 0xCCC, 0xFFF]
for i, c in enumerate(pal): struct.pack_into('<H', cpu.mem, 0xE19E00 + 2 * i, c)
STOP = 0x0F0000
def call(addr, a=0, n=5_000_000):
    cpu.push(STOP >> 16); cpu.push16((STOP - 1) & 0xFFFF)
    cpu.PBR = addr >> 16; cpu.PC = addr & 0xFFFF; cpu.A = a
    assert run(cpu, n, stop_at=STOP), 'kein Ruecksprung'
cpu.E = 0; cpu.M = cpu.XF = 0; cpu.S = 0x0BFF; cpu.D = 0x0800; cpu.DBR = 0
call(BASE, a=0x1234)
# Fenster (Port 0x0C2000, Datensatz 0x0C1FFC) mit zwei Steuerelementen
WREC = 0x0C1FFC
def ctl(addr, h, proc, rect, title, more=0, nxt=0):
    cpu.mem[addr:addr + 64] = bytes(64)
    struct.pack_into('<I', cpu.mem, h, addr)
    struct.pack_into('<I', cpu.mem, addr, nxt)
    struct.pack_into('<I', cpu.mem, addr + 4, WREC + 4)
    struct.pack_into('<4h', cpu.mem, addr + 8, *rect)
    struct.pack_into('<I', cpu.mem, addr + 20, proc)
    struct.pack_into('<I', cpu.mem, addr + 28, title)
    struct.pack_into('<H', cpu.mem, addr + 60, more)
B1, H1, B2, H2 = 0x0C3000, 0x0C3800, 0x0C3100, 0x0C3810
T1 = 0x0C3400; cpu.mem[T1] = 2; cpu.mem[T1 + 1:T1 + 3] = b'OK'
ctl(B1, H1, 0x80000000, (100, 20, 114, 80), T1, nxt=H2)
B3, H3 = 0x0C3200, 0x0C3840
ctl(B2, H2, 0x82000000, (120, 20, 130, 80), T1, nxt=H3)
B4, H4 = 0x0C3280, 0x0C3850
ctl(B3, H3, 0x00000000, (140, 20, 154, 80), T1, nxt=H4)
ctl(B4, H4, 0x02000000, (160, 20, 172, 120), T1)
cpu.mem[WREC:WREC + 300] = bytes(300)
struct.pack_into('<I', cpu.mem, WREC + 202, H1)
cpu.newctl = H2
prog = bytes([0xC2, 0x30] + [0xF4, 0, 0] * 7 + [0xA2, 0x10, 0x31, 0x22, 0, 0, 0xE1, 0x68, 0x68, 0x6B])
cpu.mem[0x0D0000:0x0D0000 + len(prog)] = prog
S0 = cpu.S
call(0x0D0000)
p1 = struct.unpack_from('<I', cpu.mem, B1 + 20)[0]; p2 = struct.unpack_from('<I', cpu.mem, B2 + 20)[0]
p4 = struct.unpack_from('<I', cpu.mem, B4 + 20)[0]
print('Prozeduren:', hex(p1), hex(p2), hex(p4), 'Stapel', cpu.S == S0)
ok = p1 == BASE + lbl['btnproc8'] and p2 == BASE + lbl['chkproc8'] and p4 == BASE + lbl['chkproc0'] and cpu.S == S0
def defproc(msg, param, h=H1):
    S1 = cpu.S
    cpu.push16(0); cpu.push16(0)
    cpu.push16(msg)
    cpu.push16(param >> 16); cpu.push16(param & 0xFFFF)
    cpu.push16(h >> 16); cpu.push16(h & 0xFFFF)
    call(p1)
    r = cpu.rd16(cpu.S + 1) | cpu.rd16(cpu.S + 3) << 16
    rest = S1 - cpu.S; cpu.S = S1
    return r, rest
# Zeichnen, normal
cpu.rects = []; cpu.texts = []
r = defproc(0, 0)
print('Zeichnen:', r, cpu.rects[:6], cpu.texts)
face = [x for x in cpu.rects if x[:4] == (100, 20, 114, 80)]
ok &= r == (0, 4) and face and face[0][4] == 14 and cpu.texts == [b'OK']
ok &= any(x[4] == 15 for x in cpu.rects) and any(x[4] == 0 for x in cpu.rects)
# gedrueckt
cpu.mem[B1 + 17] = 2; cpu.rects = []
defproc(0, 0)
print('gedrueckt:', cpu.rects[:4])
ok &= not any(x[4] == 15 for x in cpu.rects)          # kein weisses Licht oben/links
cpu.mem[B1 + 17] = 0
# Standardknopf: zusaetzlicher Rahmen
cpu.mem[B1 + 16] = 1; cpu.rects = []
defproc(0, 0)
ok &= any(x[:4] == (100, 20, 101, 80) and x[4] == 0 for x in cpu.rects)
cpu.mem[B1 + 16] = 0
# Treffertest geht an das Original
r = defproc(2, 0x00300040)
pp = struct.unpack_from('<I', cpu.mem, B1 + 20)[0]
print('Treffer:', r, cpu.cdp, hex(pp))
ok &= r == (2, 4) and cpu.cdp == [(2, 0x80000000)] and pp == p1
# Titel als Ressource
RH = 0x0C3820; RP = 0x0C3500
struct.pack_into('<I', cpu.mem, RH, RP); cpu.mem[RP] = 6; cpu.mem[RP + 1:RP + 7] = b'Cancel'
struct.pack_into('<I', cpu.mem, B1 + 28, 0x00000123); struct.pack_into('<H', cpu.mem, B1 + 60, 2)
cpu.texts = []
defproc(0, 0)
print('Ressource:', cpu.texts, cpu.res_loaded)
ok &= cpu.texts == [b'Cancel'] and cpu.res_loaded == (0x8006, 0x123)
# unsichtbar: nichts zeichnen
cpu.mem[B1 + 16] = 0x80; cpu.rects = []
defproc(0, 0)
ok &= cpu.rects == []
# intern angelegter Knopf: ModalDialog stellt ihn vorher um
cpu.mem[B1 + 16] = 0
struct.pack_into('<I', cpu.mem, B1 + 20, 0x80000000)
prog2 = bytes([0xC2, 0x30, 0xF4, 0, 0, 0xF4, 0, 0, 0xF4, 0, 0, 0xA2, 0x15, 0x0F, 0x22, 0, 0, 0xE1, 0x68, 0x6B])
cpu.mem[0x0D0100:0x0D0100 + len(prog2)] = prog2
S0 = cpu.S; call(0x0D0100)
print('ModalDialog sieht:', hex(cpu.modal), 'Stapel', cpu.S == S0)
ok &= cpu.modal == BASE + lbl['btnproc8'] and cpu.S == S0
# alter NewControl-Knopf (ctlProc 0): eigene Prozedur, Original 0
p3 = struct.unpack_from('<I', cpu.mem, B3 + 20)[0]
cpu.cdp = []
r = defproc(2, 0, H3) if False else None
S1 = cpu.S
for v in (0, 0, 2, 0, 0, H3 >> 16, H3 & 0xFFFF): cpu.push16(v)
call(p3); cpu.S = S1
print('NewControl-Knopf:', hex(p3), cpu.cdp, hex(struct.unpack_from('<I', cpu.mem, B3 + 20)[0]))
ok &= p3 == BASE + lbl['btnproc0'] and cpu.cdp == [(2, 0)] and struct.unpack_from('<I', cpu.mem, B3 + 20)[0] == p3
# alter Knopf mit Zufallswert in moreFlags: Titel trotzdem als Zeiger
struct.pack_into('<H', cpu.mem, B3 + 60, 2); cpu.res_loaded = None; cpu.texts = []
S1 = cpu.S
for v in (0, 0, 0, 0, 0, H3 >> 16, H3 & 0xFFFF): cpu.push16(v)
call(p3); cpu.S = S1
print('alter Knopf, moreFlags=2:', cpu.texts, cpu.res_loaded)
ok &= cpu.texts == [b'OK'] and cpu.res_loaded is None
# Ankreuzfeld zeichnen: geloescht, Kaestchen weiss, Haekchen schwarz, Titel
cpu.mem[B4 + 18] = 1; cpu.rects = []; cpu.texts = []; cpu.erased = None
S1 = cpu.S
for v in (0, 0, 0, 0, 0, H4 >> 16, H4 & 0xFFFF): cpu.push16(v)
call(p4); cpu.S = S1
blk = [r for r in cpu.rects if r[4] == 0]
print('Ankreuzfeld:', cpu.erased, cpu.rects[0], len(blk), 'schwarze Stuecke', cpu.texts)
ok &= cpu.erased == (160, 20, 172, 120) and cpu.rects[0][:4] == (161, 20, 172, 31) and cpu.rects[0][4] == 15
ok &= len(blk) >= 10 and cpu.texts == [b'OK']
# nicht angekreuzt: keine Haekchen-Stuecke ausser den Kanten
cpu.mem[B4 + 18] = 0; cpu.rects = []
S1 = cpu.S
for v in (0, 0, 0, 0, 0, H4 >> 16, H4 & 0xFFFF): cpu.push16(v)
call(p4); cpu.S = S1
print('leer:', len([r for r in cpu.rects if r[4] == 0]))
ok &= len([r for r in cpu.rects if r[4] == 0]) == 2
# Treffertest des Ankreuzfelds geht an das Original $02000000
cpu.cdp = []
S1 = cpu.S
for v in (0, 0, 2, 0, 0, H4 >> 16, H4 & 0xFFFF): cpu.push16(v)
call(p4); cpu.S = S1
print('Treffer Ankreuzfeld:', cpu.cdp, hex(struct.unpack_from('<I', cpu.mem, B4 + 20)[0]))
ok &= cpu.cdp == [(2, 0x02000000)] and struct.unpack_from('<I', cpu.mem, B4 + 20)[0] == p4
# neuer Wert (Nachricht 9): Original, danach selbst neu zeichnen
cpu.cdp = []; cpu.rects = []; cpu.erased = None
S1 = cpu.S
for v in (0, 0, 9, 0, 1, H4 >> 16, H4 & 0xFFFF): cpu.push16(v)
call(p4); cpu.S = S1
print('newValue:', cpu.cdp, cpu.erased is not None, len(cpu.rects))
ok &= cpu.cdp == [(9, 0x02000000)] and cpu.erased == (160, 20, 172, 120)
# DP-Block wurde von GS/OS entfernt (ProDOS-8-Start): neu anlegen und nullen
dh = cpu.ptr(BASE + lbl['dphnd'])
cpu.mem[dh:dh + 4] = bytes(4)
cpu.mem[B1 + 17] = 0; cpu.mem[B1 + 16] = 0; struct.pack_into('<H', cpu.mem, B1 + 60, 0)
struct.pack_into('<I', cpu.mem, B1 + 28, T1); cpu.rects = []; cpu.texts = []
r = defproc(0, 0)
print('AA:', cpu.mem[0x004000:0x004300].count(0xAA), 'ok bisher', ok)
print('nach Entfernen:', getattr(cpu, 'reallocs', 0), hex(cpu.rd16(BASE + lbl['ctdp'])), cpu.texts, len(cpu.rects))
ok &= getattr(cpu, 'reallocs', 0) == 1 and cpu.rd16(BASE + lbl['ctdp']) == 0x4200 and cpu.texts == [b'OK']
ok &= cpu.mem[0x004000:0x004300].count(0xAA) < 0x300 - 0x100
cpu.reallocs = 0; defproc(0, 0); ok &= cpu.reallocs == 0
print('OK' if ok else 'FAIL Knoepfe')
