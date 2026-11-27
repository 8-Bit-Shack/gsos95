# Nachgebildetes SCSI-CD-ROM-Laufwerk fuer die Tests (DStatus/DControl $80xx des SCSICD.DRIVER)
#   kind='scsi2': AppleCD 300/600 u.a. (READ TOC $43, SUB-CHANNEL $42, PLAY MSF $47, PAUSE $4B, START/STOP $1B)
#   kind='ruby' : Apple CD SC / CD 150 ($C1 TOC, $CC Status, $C2 Q-Subcode, $C8 Search+Play, $CA Pause, $CB Stop)
def bcd(v): return (v // 10) << 4 | v % 10
def unbcd(b): return (b >> 4) * 10 + (b & 15)
def msf(fr): return fr // 4500, fr // 75 % 60, fr % 75
def fr(m, s, f): return (m * 60 + s) * 75 + f

class CD:
    def __init__(s, kind='scsi2', vendor='SONY', product='CD-ROM CDU-8003'):
        s.kind = kind; s.vendor = vendor; s.product = product
        # Titel: (Beginn in Frames, Steuerbits) - Titel 5 ist ein Datentitel
        s.tracks = [(fr(0, 2, 0), 0), (fr(4, 12, 30), 0), (fr(8, 0, 10), 0), (fr(12, 45, 0), 0), (fr(15, 10, 40), 4)]
        s.leadout = fr(20, 0, 0)
        s.disc = True
        s.state = 'none'            # none, play, pause, done
        s.pos = 0; s.end = 0
        s.stopped = False; s.log = []
        s.tick = 75                 # je Statusabfrage eine Sekunde weiter

    def track_at(s, p):
        t = 1
        for i, (st, c) in enumerate(s.tracks, 1):
            if p >= st: t = i
        return t

    def status_adv(s):
        if s.state == 'play':
            s.pos += s.tick
            if s.pos >= s.end: s.pos = s.end; s.state = 'done'

    def call(s, cpu, num, code, lst, req):
        cdb = bytes(cpu.mem[lst + 2:lst + 14])
        buf = cpu.mem[lst + 14] | cpu.mem[lst + 15] << 8 | cpu.mem[lst + 16] << 16
        op = code & 0xFF
        s.log.append((num, code, cdb))
        def out(data):
            data = bytes(data)[:req]
            cpu.mem[buf:buf + len(data)] = data
            return 0
        if num == 0x202E and code == 2:                       # Eject (Treiber)
            s.disc = False; s.state = 'none'; return 0
        if op == 0x00:                                        # TEST UNIT READY
            if getattr(s, 'attention', False): s.attention = False; return 0x2E
            return 0 if s.disc else 0x2F
        if op == 0x12:
            d = bytearray(36); d[0] = 5; d[2] = 2 if s.kind == 'scsi2' else 1
            d[8:16] = s.vendor.ljust(8).encode(); d[16:32] = s.product.ljust(16).encode(); d[32:36] = b'1.0a'
            return out(d)
        if s.kind == 'scsi2' and op >= 0xC0: return 0x27        # Check Condition
        if s.kind == 'ruby' and op in (0x42, 0x43, 0x47, 0x48, 0x4B): return 0x27
        if op == 0x1B:
            if cdb[4] & 2 and not cdb[4] & 1: s.disc = False; s.state = 'none'; return 0
            s.stopped = not (cdb[4] & 1)
            if s.stopped: s.state = 'none'
            return 0
        if not s.disc: return 0x2F if op in (0x43, 0xC1) else 0x27
        n = len(s.tracks)
        if op == 0x43:
            assert cdb[1] & 2, 'MSF-Bit fehlt'
            d = bytearray(4); d[2] = 1; d[3] = n
            for i, (st, c) in enumerate(s.tracks + [(s.leadout, 0)], 1):
                m, se, f = msf(st)
                d += bytes([0, 0x10 | c, i if i <= n else 0xAA, 0, 0, m, se, f])
            L = len(d) - 2; d[0] = L >> 8; d[1] = L & 255
            return out(d)
        if op == 0x42:
            s.status_adv()
            st = {'play': 0x11, 'pause': 0x12, 'done': 0x13}.get(s.state, 0x15)
            t = s.track_at(s.pos); rel = s.pos - s.tracks[t - 1][0]
            m, se, f = msf(s.pos); rm, rs, rf = msf(max(rel, 0))
            return out([0, st, 0, 12, 1, 0x10, t, 1, 0, m, se, f, 0, rm, rs, rf])
        if op == 0x47:
            a = fr(cdb[3], cdb[4], cdb[5]); e = fr(cdb[6], cdb[7], cdb[8])
            if a >= e: return 0x27
            if s.stopped: return 0x2F                             # nicht bereit ohne START UNIT
            if s.tracks[s.track_at(a) - 1][1] & 4: return 0x27
            s.pos = a; s.end = e; s.state = 'play'; return 0
        if op == 0x4B:
            if cdb[8] & 1:
                if s.state != 'pause': return 0x27
                s.state = 'play'
            else:
                if s.state not in ('play', 'pause'): return 0x27
                s.state = 'pause'
            return 0
        # ---- Ruby ----
        if op == 0xC1:
            ty = cdb[5] & 0xC0
            if ty == 0: return out([bcd(1), bcd(n)])
            if ty == 0x40: return out([bcd(v) for v in msf(s.leadout)])
            t0 = unbcd(cdb[2]); d = bytearray()
            for i in range(t0, n + 1):
                st, c = s.tracks[i - 1]; d += bytes([c] + [bcd(v) for v in msf(st)])
            return out(d)
        if op == 0xCC:
            s.status_adv()
            st = {'play': 0, 'pause': 1, 'done': 3}.get(s.state, 5)
            return out([st, 9, 0] + [bcd(v) for v in msf(s.pos)])
        if op == 0xC2:
            t = s.track_at(s.pos); rel = s.pos - s.tracks[t - 1][0]
            return out([0x10, bcd(t), 1] + [bcd(v) for v in msf(max(rel, 0))] + [bcd(v) for v in msf(s.pos)])
        if op == 0xC8:
            assert cdb[8] == 0x80 and cdb[2] == 0x10 and cdb[3] == 9, 'Track Search: %r' % cdb
            t = unbcd(cdb[7])
            if t < 1 or t > n or s.tracks[t - 1][1] & 4: return 0x27
            s.pos = s.tracks[t - 1][0]; s.end = s.leadout; s.state = 'play'; return 0
        if op == 0xCA:
            if cdb[2] & 0x10:
                if s.state != 'play': return 0x27
                s.state = 'pause'
            else:
                if s.state != 'pause': return 0x27
                s.state = 'play'
            return 0
        if op == 0xCB:
            s.state = 'none'; return 0
        return 0x27

def install(t3mod, cds):
    """cds: {Geraetenummer: CD}; haengt sich in AppCPU.gsos ein"""
    orig = t3mod.AppCPU.gsos
    for d, cd in cds.items():
        t3mod.DEVS[d] = ('.SCSI.CD%d' % d, None)
        t3mod.DEVINFO[d] = (0x0004 | 0x0080, 0, 7)
    def gsos(s, num, pb):
        if num in (0x202D, 0x202E):
            d = s.rd16(pb + 2)
            if d in cds:
                code = s.rd16(pb + 4)
                lst = s.ptr(pb + 6); req = s.rd16(pb + 10)
                if code >= 0x8000 or (num == 0x202E and code == 2):
                    return cds[d].call(s, num, code, lst, req)
                if num == 0x202D and code == 0:      # wie der echte Treiber: Auswerfen nicht bemerkt
                    cd = cds[d]
                    cd.seen = getattr(cd, 'seen', False) or cd.disc
                    s.wr16(lst, 0x0010 if cd.seen else 0)
                    return 0
                return 0x21
        return orig(s, num, pb)
    t3mod.AppCPU.gsos = gsos
