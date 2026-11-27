import sys, struct; sys.path.insert(0, '.')
from emu import CPU, OPS, render

# ---------- Fake-Dateisystem ----------
FS = {
    ':SYSTEM.DISK': [('SYSTEM', 0x0F), ('ICONS', 0x0F), ('BASIC.SYSTEM', 0xFF), ('README', 0x04),
                     ('FINDER', 0xB3)] + [('DATEI.%02d' % i, 0x04) for i in range(1, 11)],
    ':SYSTEM.DISK:SYSTEM': [('START', 0xB3), ('DESK.ACCS', 0x0F), ('FONTS', 0x0F), ('TOOLS', 0x0F)],
    ':GSOS95': [('GSOS95', 0xB3), ('LIESMICH', 0x04), ('SPIELE', 0x0F)],
    ':GSOS95:SPIELE': [('SHANGHAI', 0xB3), ('CRYSTAL.QUEST', 0xB3)],
}
FILES = {':SYSTEM.DISK:README': b'Hallo Welt' * 1000}
DEVINFO = {}
DISKS = {}
DEVS = {1: ('.APPLEDISK3.5A', ':SYSTEM.DISK'), 2: ('.APPLEDISK3.5B', None), 3: ('.SCSI1', ':GSOS95')}

class AppCPU(CPU):
    def __init__(s):
        super().__init__(b'')
        s.events = []
        s.mouse = [160, 100]
        s.opened = {}
        s.quit = None
        s.toolcalls = {}
        s.heap = 0x6000
        s.log = []

    def rdstr(s, p):  # Class-1-Eingabestring
        n = s.rd16(p); return bytes(s.mem[p + 2:p + 2 + n]).decode()
    def wrres(s, p, txt):  # Result-Buffer
        size = s.rd16(p); b = txt.encode()
        s.wr16(p + 2, len(b))
        if len(b) + 4 > size: return 0x4F
        s.mem[p + 4:p + 4 + len(b)] = b; return 0
    def ptr(s, a): return s.rd16(a) | (s.rd(a + 2) << 16)

    def norm(s, p):
        if p.startswith('*'): p = ':SYSTEM.DISK' + p[1:]
        p = p.replace('/', ':').rstrip(':')
        return p
    def find(s, p):
        p = s.norm(p)
        par, _, nm = p.rpartition(':')
        for k in FS:
            if k.upper() == par.upper():
                for e in FS[k]:
                    if e[0].upper() == nm.upper(): return k, e
        return None, None
    def dirkey(s, p):
        p = s.norm(p)
        for k in FS:
            if k.upper() == p.upper(): return k
        return None

    def gsos(s, num, pb):
        s.log.append(hex(num))
        if num == 0x2028:                                     # GetBootVol
            return s.wrres(s.ptr(pb + 2), ':SYSTEM.DISK:')
        if num == 0x202C:
            d = s.rd16(pb + 2)
            if d not in DEVS: return 0x11
            if s.rd16(pb) >= 8:
                ch, blk, did = DEVINFO.get(d, (0x0004, 1600, 4))
                s.wr16(pb + 8, ch); s.wr16(pb + 10, blk & 0xFFFF); s.wr16(pb + 12, blk >> 16); s.wr16(pb + 20, did)
            if s.rd16(pb) >= 9: s.wr16(pb + 22, 0xBEEF); s.wr16(pb + 24, 0xBEEF)
            if s.rd16(pb) >= 10: s.wr16(pb + 26, 0xBEEF); s.wr16(pb + 28, 0xBEEF)
            return s.wrres(s.ptr(pb + 4), DEVS[d][0])
        if num == 0x202D:
            d = s.rd16(pb + 2)
            if d not in DEVS: return 0x11
            st = 0x0010 if DEVS[d][1] is not None else 0
            s.wr16(s.ptr(pb + 6), st); return 0
        if num in (0x202F, 0x2030):
            d = s.rd16(pb + 2); buf = s.ptr(pb + 4); n = s.rd16(pb + 8); b = s.rd16(pb + 12)
            ch, blk, did = DEVINFO.get(d, (0x0004, 1600, 4))
            disk = DISKS.setdefault(d, bytearray(blk * 512))
            if num == 0x202F: s.mem[buf:buf + n] = disk[b * 512:b * 512 + n]
            else: disk[b * 512:b * 512 + n] = s.mem[buf:buf + n]
            s.dio = getattr(s, 'dio', 0) + 1
            return 0
        if num == 0x2016:
            s.opened[s.rd16(pb + 2)][1] = s.rd16(pb + 6); return 0
        if num == 0x2008:
            dev = s.rdstr(s.ptr(pb + 2))
            for d, (n, v) in DEVS.items():
                if n == dev:
                    if v is None: return 0x2F
                    if v == 'UNF': return 0x52
                    return s.wrres(s.ptr(pb + 6), v)
            return 0x10
        if num == 0x200E:
            return s.wrres(s.ptr(pb + 6), s.norm(s.rdstr(s.ptr(pb + 2))))
        if num == 0x2009:
            s.prefixes = getattr(s, 'prefixes', {}); s.prefixes[s.rd16(pb + 2)] = s.rdstr(s.ptr(pb + 4)); return 0
        if num == 0x2006:
            p = s.rdstr(s.ptr(pb + 2)); k, e = s.find(p)
            if e is None:
                if s.dirkey(p): e = (p, 0x0F)
                else: return 0x46
            ty = e[1]; data = FILES.get(s.norm(p).upper(), b'')
            s.wr16(pb + 8, ty); s.wr16(pb + 10, AUX.get(s.norm(p).upper().rsplit(':',1)[-1], 1 if ty == 0xC0 else 0)); s.wr16(pb + 12, 0)
            s.wr16(pb + 14, 0x0D if ty == 0x0F else 1)
            s.mem[pb + 24:pb + 32] = bytes([10, 42, 13, 126, 14, 10, 0, 1])
            s.wr16(pb + 36, len(data)); s.wr16(pb + 38, 0)
            s.wr16(pb + 6, ACCESS.get(s.norm(p).upper(), 0xC3))
            return 0
        if num == 0x2018:
            ref = s.rd16(pb + 2); o = s.opened[ref]; key = o[0].upper()
            FILES[key] = FILES.get(key, b'')[:s.rd16(pb + 6)]; return 0
        if num == 0x2005:
            p = s.rdstr(s.ptr(pb + 2)); ACCESS[s.norm(p).upper()] = s.rd16(pb + 6)
            if s.rd16(pb) >= 4:
                k, e = s.find(p); i = FS[k].index(e); FS[k][i] = (e[0], s.rd16(pb + 8))
                AUX[e[0].upper()] = s.rd16(pb + 10)
            return 0
        if num == 0x2001:
            p = s.norm(s.rdstr(s.ptr(pb + 2)))
            par, _, nm = p.rpartition(':')
            k = s.dirkey(par)
            if k is None: return 0x44
            if s.find(p)[1]: return 0x47
            st = s.rd16(pb + 14); ty = s.rd16(pb + 8)
            FS[k].append((nm, 0x0F if st == 0x0D else ty))
            if st == 0x0D: FS[k + ':' + nm] = []
            else: FILES[p.upper()] = b''
            return 0
        if num in (0x2024, 0x2025):
            dev = s.rdstr(s.ptr(pb + 2)); vn = s.rdstr(s.ptr(pb + 6))
            s.erased = (num, dev, vn)
            for d, (n, v) in list(DEVS.items()):
                if n == dev:
                    if v not in (None, 'UNF'):
                        for key in [k for k in FS if k.upper().startswith(v.upper())]: del FS[key]
                    if num == 0x2025 and v == 'UNF': return 0x27
                    DEVS[d] = (n, vn); FS[vn] = []
                    s.wr16(pb + 10, 1); return 0
            return 0x11
        if num == 0x2004:
            src = s.norm(s.rdstr(s.ptr(pb + 2))); dst = s.norm(s.rdstr(s.ptr(pb + 6)))
            if src.count(':') == 1 and dst.count(':') == 1:      # Volume umbenennen
                for d, (n, v) in DEVS.items():
                    if v and v.upper() == src.upper():
                        DEVS[d] = (n, dst)
                        for key in [k for k in FS if k.upper() == src.upper() or k.upper().startswith(src.upper() + ':')]:
                            FS[dst + key[len(src):]] = FS.pop(key)
                        return 0
                return 0x45
            k, e = s.find(src)
            if e is None: return 0x46
            par, _, nm = dst.rpartition(':'); dk = s.dirkey(par)
            if dk is None: return 0x44
            if s.find(dst)[1]: return 0x47
            FS[k].remove(e); FS[dk].append((nm, e[1]))
            for key in list(FS):
                if key.upper() == src.upper() or key.upper().startswith(src.upper() + ':'):
                    FS[dst + key[len(src):]] = FS.pop(key)
            if src.upper() in FILES: FILES[dst.upper()] = FILES.pop(src.upper())
            return 0
        if num == 0x2002:
            p = s.norm(s.rdstr(s.ptr(pb + 2))); k, e = s.find(p)
            if e is None: return 0x46
            dk = s.dirkey(p)
            if dk is not None:
                if FS[dk]: return 0x4E
                del FS[dk]
            FS[k].remove(e); FILES.pop(p.upper(), None)
            return 0
        if num in (0x2012, 0x2013):
            ref = s.rd16(pb + 2); o = s.opened[ref]
            buf = s.ptr(pb + 4); n = s.rd16(pb + 8)
            key = o[0].upper()
            if num == 0x2013:
                cur = FILES.get(key, b''); m0 = o[1]
                FILES[key] = cur[:m0] + bytes(s.mem[buf:buf + n]) + cur[m0 + n:]; o[1] += n
            else:
                d = FILES.get(key, b'')[o[1]:o[1] + n]
                if not d: return 0x4C
                s.mem[buf:buf + len(d)] = d; o[1] += len(d); n = len(d)
            s.wr16(pb + 12, n); return 0
        if num == 0x2010:
            p = s.norm(s.rdstr(s.ptr(pb + 4)))
            ref = len(s.opened) + 20 + s.icount % 1000
            if s.dirkey(p) is not None:
                s.opened[ref] = [s.dirkey(p), 0]
            elif p.upper() in FILES or s.find(p)[1]:
                if s.rd16(pb) >= 4 and s.rd16(pb + 10) == 1:
                    FILES.setdefault(p.upper() + '#R', b''); s.opened[ref] = [p + '#R', 0]
                else:
                    s.opened[ref] = [p, 0]
            else:
                return 0x46
            s.wr16(pb + 2, ref); return 0
        if num == 0x2019:                                     # GetEOF
            ln = len(FILES.get(s.opened[s.rd16(pb + 2)][0].upper(), b''))
            s.wr16(pb + 4, ln & 0xFFFF); s.wr16(pb + 6, ln >> 16); return 0
        if num == 0x201C:
            ref = s.rd16(pb + 2); path, i = s.opened[ref]
            if s.rd16(pb + 6) == 0 and s.rd16(pb + 8) == 0:     # Basis 0, Abstand 0: Anzahl
                s.wr16(pb + 14, len(FS.get(path, []))); return 0
            if s.rd16(pb + 6) == 0: i = s.rd16(pb + 8) - 1
            if i >= len(FS.get(path, [])): return 0x61
            nm, ty = FS[path][i]; s.opened[ref][1] += 1
            s.wr16(pb + 14, i + 1); s.wr16(pb + 16, ty)
            if s.rd16(pb) >= 8:
                full = (path + ':' + nm).upper()
                ln = next((len(v) for k, v in FILES.items() if k.upper() == full), 0)
                s.wr16(pb + 18, ln & 0xFFFF); s.wr16(pb + 20, ln >> 16)
            if s.rd16(pb) >= 11:
                s.mem[pb + 34:pb + 42] = bytes([0, 42, 13, 126, 14, 10, 0, 1])   # 15.11.2026 13:42
            if s.rd16(pb) >= 13:
                s.wr16(pb + 44, AUX.get(nm.upper(), 0)); s.wr16(pb + 46, 0)
            return s.wrres(s.ptr(pb + 10), nm)
        if num == 0x2014:
            s.opened.pop(s.rd16(pb + 2), None); return 0
        if num == 0x202E:
            s.ejected = s.rd16(pb + 2); return 0
        if num == 0x2003:
            s.quit = ('OSShutdown', s.rd16(pb + 2)); raise StopIteration
        if num == 0x2029:
            s.quit = (s.rdstr(s.ptr(pb + 2)) if s.rd16(pb) >= 1 and s.ptr(pb + 2) else None,
                      s.rd16(pb + 6) if s.rd16(pb) >= 2 else 0)
            raise StopIteration
        raise Exception('GS/OS %04X' % num)

    def tool(s, x):
        s.toolcalls[x] = s.toolcalls.get(x, 0) + 1
        s.toolerr = 0
        sp = s.S + 4  # erster Parameter (zuletzt gepusht)
        def res(nbytes_in, results):
            # Ergebnisse liegen ueber den Parametern
            for i, b in enumerate(results):
                s.mem[sp + nbytes_in + i] = b
            return nbytes_in
        if x == 0x9004: s.qdcur = False; n = 0
        elif x in (0x0201, 0x0203, 0x0306, 0x0304, 0x0303): n = 0
        elif x == 0x0202: n = res(0, [0x34, 0x12])
        elif x == 0x0902:
            h = s.heap; s.heap += 0x10
            n = (h - 0x6000) // 0x10
            size = s.rd16(sp + 8) | (s.rd16(sp + 10) << 16)
            attr = s.rd16(sp + 4)
            if (attr & 0x0001) and not getattr(s, 'dp0', 0): ptr = 0x000C00; s.dp0 = 1   # erster Block in Bank 0: Direct Page
            else:
                nxt = getattr(s, 'hnext', 0x064000)
                if (nxt & 0xFFFF) + size > 0x10000: nxt = (nxt & 0xFF0000) + 0x10000
                ptr = nxt; s.hnext = (nxt + max(size, 0x100) + 0xFF) & ~0xFF
            s.wr16(h, ptr & 0xFFFF); s.mem[h + 2] = ptr >> 16
            s.allocs = getattr(s, 'allocs', []) + [(ptr, size)]
            n = res(12, list(struct.pack('<I', h)))
        elif x == 0x1002: n = 4
        elif x in DESKSTUB:
            nin, nres = DESKSTUB[x]
            s.desklog = getattr(s, 'desklog', []) + [x]
            out = [0] * nres
            if x == 0x1B05: out = [len(NDAS), 0]
            elif x == 0x2D0F: out = [0x00, 0x10, 0x07, 0x00]
            elif x == 0x250F:
                i = s.rd16(sp)
                p = 0x070100 + i * 0x40
                nm = NDAS[i - 1].encode('latin-1') if 0 < i <= len(NDAS) else b''
                s.mem[p] = len(nm); s.mem[p + 1:p + 1 + len(nm)] = nm
                out = list(struct.pack('<I', p))
            elif x == 0x1505: s.opened_nda = s.rd16(sp); s.ndawin = True
            elif x == 0x150E: out = list(struct.pack('<I', 0x071000 if getattr(s, 'ndawin', False) else 0))
            elif x == 0x520E: out = list(struct.pack('<I', getattr(s, 'firstwin', 0x071000 if getattr(s, 'ndawin', False) else 0)))
            elif x == 0x0E0E:
                nm = b'  Calculator  '
                s.mem[0x070F00] = len(nm); s.mem[0x070F01:0x070F01 + len(nm)] = nm
                out = list(struct.pack('<I', 0x070F00))
            elif x == 0x1805:
                s.sysedits = getattr(s, 'sysedits', []) + [s.rd16(sp)]
                out = [1, 0]
            elif x == 0xCA04: s.qdcur = True
            elif x == 0x1A05:
                s.sysevents = getattr(s, 'sysevents', []) + [(s.rd16(sp+14), s.rd16(sp+10) & 0xFF)]
                out = [1, 0]
            elif x == 0x1C05: s.ndawin = False; s.ndaclosed = getattr(s, 'ndaclosed', 0) + 1
            elif x == 0x6704: out = [0, 0x20, 0x07, 0]
            elif x == 0x110E: s.selected = getattr(s, 'selected', 0) + 1
            elif x in (0x5404, 0x3C04, 0xA504, 0x3704, 0x3A04):
                s.qd = getattr(s, 'qd', []) + [(x, [s.rd16(sp + 2 * i) for i in range(DESKSTUB[x][0] // 2)])]
            elif x == 0x0608: out = [1, 0]
            elif x == 0x0E08:
                pb = s.rd16(sp) | s.mem[sp + 2] << 16
                s.ffsound = (s.rd16(sp + 4), bytes(s.mem[pb:pb + 18]))
            elif x == 0x2503:
                tk = s.icount // 40000 + getattr(s, 'tickofs', 0)
                out = list(struct.pack('<I', tk))
            elif x == 0x170E:
                h, v = s.rd16(sp + 2), s.rd16(sp)
                code = 0x8013 if getattr(s, 'ndawin', False) and 200 <= h < 300 and 20 <= v < 80 else 0x10
                fp = s.rd16(sp + 4) | s.mem[sp + 6] << 16
                s.mem[fp:fp + 4] = struct.pack('<I', 0x071000 if code & 0x8000 else 0)
                out = [code & 0xFF, code >> 8]
            elif x == 0x1705: s.sysclicks = getattr(s, 'sysclicks', 0) + 1
            elif x == 0x1501:
                h = s.rd16(sp) | s.mem[sp + 2] << 16
                p = s.rd16(h) | s.mem[h + 2] << 16
                s.message = (s.rd16(sp + 6), s.rd16(sp + 4), bytes(s.mem[p:p + 80]))
            n = res(nin, out)
        elif x == 0x2703:
            szp = s.rd16(sp) | s.mem[sp + 2] << 16
            hp = s.rd16(sp + 4) | s.mem[sp + 6] << 16
            bsz = s.rd16(sp + 8)
            src = s.rd16(sp + 10) | s.mem[sp + 12] << 16
            dst = s.rd16(hp) | s.mem[hp + 2] << 16
            want = s.rd16(szp)
            out = unpack_bytes(bytes(s.mem[src:src + bsz]))[:want]
            s.mem[dst:dst + len(out)] = out
            s.wr16(szp, want - len(out))
            n = res(14, [len(out) & 0xFF, len(out) >> 8])
        elif x == 0x0204:
            s.qd_dp = s.rd16(sp + 6)
            assert s.qd_dp == 0x0C00, 'QDStartUp: falsche Direct Page %04X' % s.qd_dp
            n = 8
        elif x == 0x0206:
            assert s.rd16(sp + 12) == 0x0F00, 'EMStartUp: falsche Direct Page %04X' % s.rd16(sp + 12)
            n = 14
        elif x == 0x0D03: n = res(0, [5, 42, 9, 126, 14, 10, 0, 1])
        elif x == 0x0C03:
            r = s.rd16(sp); v = BRAM.get(r, 0); n = res(2, [v & 0xFF, v >> 8])
        elif x == 0x0B03:
            r = s.rd16(sp); BRAM[r] = s.rd16(sp + 2); n = 4
        elif x == 0x0E03:
            s.timeset = (s.rd16(sp + 4), s.rd16(sp + 2), s.rd16(sp)); n = 6
        elif x == 0x1B02: n = res(0, list(struct.pack('<I', 1234567)))
        elif x == 0x2C03: s.beeps = getattr(s, 'beeps', 0) + 1; n = 0
        elif x == 0x0A06:
            evp = s.rd16(sp) | (s.mem[sp + 2] << 16)
            mask = s.rd16(sp + 4)
            mods = 0
            if s.events and (mask >> s.events[0][0]) & 1:
                ev = s.events.pop(0); what, h, v, msg = ev[:4]; s.mouse = [h, v]
                if len(ev) > 4: mods = ev[4]
            else:
                what, (h, v), msg = 0, s.mouse, 0
            rec = struct.pack('<HIIhhH', what, msg, s.icount // 40000, v, h, mods)
            s.mem[evp:evp + 16] = rec
            n = res(6, [1 if what else 0, 0])
        elif (x & 0xFF) == 0x36:           # Marinetti (TCP/IP-Toolset)
            if getattr(s, 'mari', None) is not None: n = s.mari.call(s, x, sp, res)
            else: n = 0; s.toolerr = 0x0002    # Toolset nicht geladen
        elif x == 0x2403:                   # FWEntry (SmartPort-Aufrufe der FujiNet)
            if getattr(s, 'fwentry', None) is None: raise Exception('FWEntry ohne SmartPort')
            n = s.fwentry(s, sp, res)
        else:
            raise Exception('Tool %04X' % x)
        # RTL + Parameter entfernen
        lo = s.pull(); hi = s.pull(); bank = s.pull()
        s.S += n
        s.PC = ((hi << 8 | lo) + 1) & 0xFFFF; s.PBR = bank; s.A = s.toolerr; s.C = 1 if s.toolerr else 0

    def step(s):
        full = (s.PBR << 16) | s.PC
        if full == 0xE10000:
            s.icount += 1; return s.tool(s.X)
        if full == 0xE100B0:
            s.icount += 1
            lo = s.pull(); hi = s.pull(); bank = s.pull()
            num = s.pull16(); p = s.pull16(); pb = p | (s.pull16() << 16)
            err = s.gsos(num, pb)
            s.PC = ((hi << 8 | lo) + 1) & 0xFFFF; s.PBR = bank
            s.A = err; s.C = 1 if err else 0
            return
        return super().step()

AUX = {}
ACCESS = {}
# Tests: Sortierung aus (Modus 3), damit feste Positionen stimmen
BRAM = {0x2E: 2, 0x1E: 8, 0x1F: 5, 0x1C: 6}
FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PREFS'] = bytes([0x0C, 0])
NDAS = ['Control Panel', 'Calculator', 'Alarm Clock', 'Notes']
DESKSTUB = {0x0E01: (4, 0), 0x0212: (0, 0), 0x020E: (2, 0), 0x0C0E: (6, 4), 0x020F: (4, 0), 0x0210: (4, 0),
    0x0214: (4, 0), 0x0215: (2, 0), 0x0216: (0, 0), 0x021B: (4, 0), 0x021C: (0, 0), 0x0205: (0, 0),
    0x2D0F: (4, 4), 0x0D0F: (6, 0), 0x1E05: (2, 0), 0x1B05: (0, 2), 0x250F: (2, 4), 0x1905: (0, 0),
    0x170E: (8, 2), 0x1705: (10, 0), 0x150E: (0, 4), 0x390E: (4, 0), 0x1505: (2, 2), 0x1D05: (0, 0),
    0x0305: (0, 0), 0x031C: (0, 0), 0x031B: (0, 0), 0x0316: (0, 0), 0x0315: (0, 0), 0x0314: (0, 0),
    0x0310: (0, 0), 0x030F: (0, 0), 0x030E: (0, 0), 0x0312: (0, 0),
    0x2A0E: (4, 4), 0x120E: (4, 0), 0x130E: (4, 0), 0x110E: (4, 0), 0xD604: (14, 0), 0x1501: (8, 0), 0x520E: (0, 4), 0x100E: (8, 0), 0x0F0E: (8, 0), 0x0E0E: (4, 4), 0x0608: (0, 2), 0x0F01: (4, 0), 0x0208: (2, 0), 0x0E08: (6, 0), 0x0F08: (2, 0), 0x2503: (0, 4), 0x0308: (0, 0), 0xA004: (2, 0), 0xA204: (2, 0), 0xA104: (0, 2), 0xA304: (0, 2), 0x3604: (0, 0), 0x3704: (2, 0), 0x3A04: (4, 0), 0x3C04: (4, 0), 0x5404: (4, 0), 0xA504: (4, 0), 0x1A05: (16, 2), 0x9104: (0, 0), 0xCA04: (0, 0), 0x1805: (2, 2), 0x1C05: (4, 0), 0x1C04: (0, 4), 0x200E: (0, 4), 0x1B04: (4, 0), 0x6704: (0, 4), 0x2504: (4, 0), 0x2404: (4, 0), 0x6904: (8, 0), 0x7304: (12, 0)}

def unpack_bytes(b):
    out = bytearray(); i = 0
    while i < len(b):
        f = b[i]; i += 1; k = (f & 0x3F) + 1; m = f >> 6
        if m == 0: out += b[i:i + k]; i += k
        elif m == 1: out += bytes([b[i]]) * k; i += 1
        elif m == 2: out += b[i:i + 4] * k; i += 4
        else: out += bytes([b[i]]) * (k * 4); i += 1
    return out

def pack_bytes(d):
    out = bytearray(); i = 0
    while i < len(d):
        j = i
        while j < len(d) and d[j] == d[i] and j - i < 64: j += 1
        if j - i >= 3: out += bytes([0x40 | (j - i - 1), d[i]]); i = j; continue
        j = i
        while j < len(d) and j - i < 64 and not (j + 2 < len(d) and d[j] == d[j+1] == d[j+2]): j += 1
        out += bytes([j - i - 1]) + d[i:j]; i = j
    return bytes(out)

def load_omf(cpu, data, addrs):
    """laedt alle Segmente; addrs = Ladeadresse je Segmentnummer"""
    segs = []
    p0 = 0
    while p0 < len(data):
        total = struct.unpack_from('<I', data, p0)[0]
        segs.append(data[p0:p0 + total]); p0 += total
    for n, seg in enumerate(segs, 1):
        if struct.unpack_from('<H', seg, 20)[0] == 0x12: continue     # DP/Stack
        addr = addrs[n]
        disp = struct.unpack_from('<H', seg, 42)[0]
        p = disp
        while True:
            op = seg[p]
            if op == 0xF2:
                k = struct.unpack_from('<I', seg, p + 1)[0]
                cpu.mem[addr:addr + k] = seg[p + 5:p + 5 + k]; p += 5 + k
            elif op == 0xF5:
                cnt, sh, off, val = struct.unpack_from('<BbHH', seg, p + 1)
                v = val + addr
                for k in range(cnt): cpu.mem[addr + off + k] = (v >> (8 * k)) & 0xFF
                p += 7
            elif op == 0xF6:
                cnt, sh, off, sn, val = struct.unpack_from('<BbHBH', seg, p + 1)
                v = val + addrs[sn]
                for k in range(cnt): cpu.mem[addr + off + k] = (v >> (8 * k)) & 0xFF
                p += 8
            elif op == 0xF7:                                        # SUPER
                ln = struct.unpack_from('<I', seg, p + 1)[0]; typ = seg[p + 5]
                q = p + 6; end = p + 5 + ln; page = 0
                while q < end:
                    c = seg[q]; q += 1
                    if c & 0x80: page += c & 0x7F; continue
                    for _ in range(c + 1):
                        off = page * 256 + seg[q]; q += 1; a = addr + off
                        rel = cpu.mem[a] | cpu.mem[a + 1] << 8
                        if typ == 0: w, v = 2, rel + addr
                        elif typ == 1: w, v = 3, (rel | cpu.mem[a + 2] << 16) + addr
                        elif 2 <= typ <= 13:
                            assert typ == 2; w, v = 3, rel + addrs[cpu.mem[a + 2]]
                        elif 14 <= typ <= 25: w, v = 2, rel + addrs[typ - 13]
                        else: w, v = 2, (rel + addrs[typ - 25]) >> 16
                        for k in range(w): cpu.mem[a + k] = (v >> (8 * k)) & 0xFF
                    page += 1
                assert q == end
                p = end
            elif op == 0x00: break
            else: raise Exception('OMF %02X' % op)
    return len(segs)

def boot_app(addr=0x052340):
    cpu = AppCPU()
    data = open('GSOS95.S16', 'rb').read()
    load_omf(cpu, data, {1: addr, 2: 0x040000, 3: 0x050000, 4: 0x200000, 5: 0x300000})
    cpu.E = 0; cpu.M = cpu.XF = 0
    cpu.PBR = addr >> 16; cpu.PC = addr & 0xFFFF; cpu.DBR = 0
    cpu.D = 0x0800; cpu.S = 0x0BFF; cpu.A = 0x1234
    return cpu
