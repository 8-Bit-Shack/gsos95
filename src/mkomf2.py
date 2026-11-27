# Baut GS/OS-Ladedatei: 1 Code/Daten, 2 Grafik+XCODE, 3..5 XCODE2..4, 6 Direct Page/Stack
import struct, re, subprocess, sys
CC = __import__('os').environ.get('CC65_BIN', '/home/claude/ext/cc65/bin').rstrip('/') + '/'
CFG = """MEMORY {
  RAM: start=$%X, size=$10000, file=%%O;
  AMEM: start=$%X, size=$10000, file="%s";
  CMEM: start=$%X, size=$10000, file="%s";
  DMEM: start=$%X, size=$10000, file="%s";
  EMEM: start=$%X, size=$10000, file="%s";
  FXAM: start=$0, size=$4000, file="%s";
  FXBM: start=$0, size=$4000, file="%s";
  FXCM: start=$0, size=$4000, file="%s";
  FXDM: start=$0, size=$4000, file="%s";
  FXEM: start=$0, size=$4000, file="%s";
}
SEGMENTS {
  CODE: load=RAM, type=ro;
  RODATA: load=RAM, type=ro;
  BSS: load=RAM, type=bss, define=yes;
  ASSETS: load=AMEM, type=ro;
  XCODE: load=AMEM, type=ro;
  XCODE2: load=CMEM, type=ro, optional=yes;
  XCODE3: load=DMEM, type=ro, optional=yes;
  XCODE4: load=EMEM, type=ro, optional=yes;
  FXA: load=FXAM, type=ro, optional=yes;
  FXB: load=FXBM, type=ro, optional=yes;
  FXC: load=FXCM, type=ro, optional=yes;
  FXD: load=FXDM, type=ro, optional=yes;
  FXE: load=FXEM, type=ro, optional=yes;
}
"""
NSEG = 5      # Code-Segmente: 1 Code, 2 Grafik+XCODE, 3 XCODE2, 4 XCODE3, 5 XCODE4
def link(b1, b2, b3, b4, b5, tag, extra=()):
    open('lk_%s.cfg' % tag, 'w').write(CFG % (b1, b2, 'a2_%s.bin' % tag, b3, 'a3_%s.bin' % tag,
                                              b4, 'a4_%s.bin' % tag, b5, 'a5_%s.bin' % tag,
                                              'fxa_%s.bin' % tag, 'fxb_%s.bin' % tag, 'fxc_%s.bin' % tag,
                                              'fxd_%s.bin' % tag, 'fxe_%s.bin' % tag))
    for f in ('a3_%s.bin', 'a4_%s.bin', 'a5_%s.bin', 'fxc_%s.bin', 'fxd_%s.bin', 'fxe_%s.bin'):
        open(f % tag, 'wb').close()
    subprocess.check_call([CC + 'ld65', '-C', 'lk_%s.cfg' % tag, 'app.o', '-o', 'a1_%s.bin' % tag] + list(extra))
    return [open(f % tag, 'rb').read() for f in ('a1_%s.bin', 'a2_%s.bin', 'a3_%s.bin', 'a4_%s.bin', 'a5_%s.bin')]

SHIFT = 0x0100   # Verschiebung der Probelinks (klein, damit fast die ganze Bank nutzbar ist)
BASE = link(0, 0, 0, 0, 0, 'A', ['-Ln', 'app.lbl', '-m', 'app.map'])
SH = {1: link(SHIFT, 0, 0, 0, 0, 'B'), 2: link(0, SHIFT, 0, 0, 0, 'C'), 3: link(0, 0, SHIFT, 0, 0, 'D'),
      4: link(0, 0, 0, SHIFT, 0, 'E'), 5: link(0, 0, 0, 0, SHIFT, 'F')}
A1, A2, A3, A4, A5 = BASE

def diffs(x, y):
    out = []
    i = 0
    while i < len(x):
        if x[i] != y[i]:
            j = i - 1
            v0 = x[j] | x[j + 1] << 8; v1 = y[j] | y[j + 1] << 8
            assert x[j] == y[j] and (v1 - v0) & 0xFFFF == SHIFT, 'Differenz bei %X' % i
            out.append((j, v0))
        i += 1
    return out

lbl = {l.split()[2].lstrip('.'): int(l.split()[1], 16) for l in open('app.lbl')}
abref = lbl['abank_ref']
def fxlist(fn):
    try: d = open(fn, 'rb').read()
    except FileNotFoundError: return set()
    return {struct.unpack_from('<H', d, i)[0] + 1 for i in range(0, len(d), 2)}
FX = {1: fxlist('fxa_A.bin'), 2: fxlist('fxb_A.bin'), 3: fxlist('fxc_A.bin'), 4: fxlist('fxd_A.bin'),
      5: fxlist('fxe_A.bin')}
m = open('app.map').read()
bss = int(re.search(r'^BSS\s+\S+\s+\S+\s+([0-9A-F]+)', m, re.M).group(1), 16)

def segment(data, bss, kind, segnum, name, recs):
    body = bytearray(bytes([0xF2]) + struct.pack('<I', len(data)) + data)
    body += recs + b'\x00'
    loadname = b'          '
    segname = bytes([len(name)]) + name
    dispname = 44
    dispdata = dispname + 10 + len(segname)
    total = dispdata + len(body)
    hdr = struct.pack('<IIIBBBBI', total, bss, len(data) + bss, 0, 0, 4, 2, 0x10000)
    hdr += struct.pack('<HHIIBBHIHH', kind, 0, 0, 0, 0, 0, segnum, 0, dispname, dispdata)
    return hdr + loadname + segname + bytes(body)

import os
# Standard: einzelne cRELOC/cINTERSEG. SUPER-Datensaetze (OMF_SUPER=1) waren in 2.1.45 aktiv;
# damit fand GS/OS95 unter echtem GS/OS (GSSquared) die Netzwerkkarte nicht mehr -> wieder aus.
OLDREL = not os.environ.get('OMF_SUPER')
def super_rec(typ, offs):
    """SUPER-Datensatz ($F7): je 256-Byte-Seite Anzahl-1 und die Lagen; $80|n ueberspringt n Seiten"""
    pages = {}
    for o in sorted(offs): pages.setdefault(o >> 8, []).append(o & 0xFF)
    body = bytearray(); cur = 0
    for pg in sorted(pages):
        d = pg - cur
        while d > 0:
            k = min(d, 0x7F); body.append(0x80 | k); d -= k
        lst = pages[pg]
        for i in range(0, len(lst), 256):
            part = lst[i:i + 256]
            assert i == 0, 'mehr als 256 Lagen in einer Seite'
            body.append(len(part) - 1); body += bytes(part)
        cur = pg + 1
    return bytes([0xF7]) + struct.pack('<I', len(body) + 1) + bytes([typ]) + bytes(body)

nrel = {}
def relocs(s, data=None):
    """Relocations des Segments s (1..4) gegen alle Segmente"""
    out = bytearray(); seen = set(); sup = {}
    for t in (1, 2, 3, 4, 5):
        for off, val in diffs(BASE[s - 1], SH[t][s - 1]):
            assert off not in seen, 'doppelte Relocation %d:%X' % (s, off)
            seen.add(off)
            n = 3 if (off in FX[s] or (s == 1 and off == abref)) else 2
            if not OLDREL:
                assert data[off] | data[off + 1] << 8 == val, 'Wert %d:%X' % (s, off)
                if n == 3:
                    assert data[off + 2] == 0, 'Bankbyte %d:%X' % (s, off)
                    typ = 1 if t == s else 2          # RELOC3 / INTERSEG1 (Segment im 3. Byte)
                    if t != s: data[off + 2] = t
                else:
                    typ = 0 if t == s else 13 + t     # RELOC2 / INTERSEG13.. (Segment 1..12)
                sup.setdefault(typ, []).append(off)
                nrel[(s, t)] = nrel.get((s, t), 0) + 1
                continue
            if t == s:
                out += bytes([0xF5, n, 0]) + struct.pack('<HH', off, val)          # cRELOC (3: mit Bank)
            else:
                out += bytes([0xF6, n, 0]) + struct.pack('<HBH', off, t, val)       # cINTERSEG
            nrel[(s, t)] = nrel.get((s, t), 0) + 1
    for typ in sorted(sup): out += super_rec(typ, sup[typ])
    return out
A1, A2, A3, A4, A5 = [bytearray(x) for x in (A1, A2, A3, A4, A5)]
r1 = relocs(1, A1); r2 = relocs(2, A2); r3 = relocs(3, A3) if A3 else b''; r4 = relocs(4, A4) if A4 else b''; r5 = relocs(5, A5) if A5 else b''
seg1 = segment(bytes(A1), bss, 0x0000, 1, b'GSOS95', r1)
seg2 = segment(bytes(A2), 0, 0x0001, 2, b'GFX', r2)
seg3 = segment(bytes(A3) if A3 else b'\x00', 0, 0x0000, 3, b'CODE2', r3)
seg3b = segment(bytes(A4) if A4 else b'\x00', 0, 0x0000, 4, b'CODE3', r4)
seg3c = segment(bytes(A5) if A5 else b'\x00', 0, 0x0000, 5, b'CODE4', r5)
# Segment 3: Direct Page / Stack, 16 KB (statt der 4 KB, die GS/OS sonst vergibt)
STK = 0x4000
def dpstack(segnum, size):
    body = bytes([0xF1]) + struct.pack('<I', size) + b'\x00'      # DS: Nullen
    loadname = b'          '
    name = b'STACK'
    segname = bytes([len(name)]) + name
    dispname = 44
    dispdata = dispname + 10 + len(segname)
    total = dispdata + len(body)
    hdr = struct.pack('<IIIBBBBI', total, 0, size, 0, 0, 4, 2, 0x10000)
    hdr += struct.pack('<HHIIBBHIHH', 0x0012, 0, 0, 0, 0, 0, segnum, 0, dispname, dispdata)
    return hdr + loadname + segname + body
seg4 = dpstack(6, STK)
open('GSOS95.S16', 'wb').write(seg1 + seg2 + seg3 + seg3b + seg3c + seg4)
print('OMF %d Bytes: Code %d (+BSS %d), Grafik %d, Code2 %d, Code3 %d, Code4 %d; Reloc %s, Fern %d/%d/%d/%d/%d' % (
    len(seg1) + len(seg2) + len(seg3) + len(seg3b) + len(seg3c), len(A1), bss, len(A2), len(A3), len(A4), len(A5),
    ' '.join('%d>%d:%d' % (s, t, n) for (s, t), n in sorted(nrel.items())), len(FX[1]), len(FX[2]), len(FX[3]), len(FX[4]), len(FX[5])))
if len(A1) + bss > 0x10000 - 0x400:
    raise SystemExit('FEHLER: Hauptbank voll (Code+BSS %d > %d)' % (len(A1) + bss, 0x10000 - 0x400))
if max(len(A2), len(A3), len(A4), len(A5)) > 0x10000:
    raise SystemExit('FEHLER: Grafikbank voll')
