# Nachgebildete Uthernet II (W5100, indirekter Bus) mit kleinem "Internet":
# DHCP-Server, DNS-Server und HTTP-Server mit festen Seiten. Fuer die Tests im 65816-Simulator.
import struct

class Net:
    def __init__(s):
        s.dhcp = True                      # DHCP-Server antwortet
        s.dns = {'pool.ntp.org': '10.9.9.9', 'frogfind.com': '10.1.1.1', 'test.local': '10.1.1.2', 'info.cern.ch': '10.1.1.3'}
        s.pages = {}                       # (ip, port, pfad) -> Antwort (bytes) oder Funktion
        s.log = []
        s.requests = []
        s.ntp = 1795100000                 # Unix-Zeit fuer den NTP-Server
        s.udp_dns = True                   # False: DNS nur ueber TCP (wie GSSquared?)

class W5100:
    SREG = 0x0400
    def __init__(s, net):
        s.net = net
        s.mem = bytearray(0x8000)
        s.mode = 0
        s.addr = 0
        s.reset()
    def reset(s):
        s.mem[:0x0400] = bytes(0x400)
        s.mem[0x0017] = 0x07; s.mem[0x0018] = 0xD0
        s.sock = [dict(st=0, rxwr=0, pend=b'', srv=None, req=b'') for _ in range(4)]
        s.setsizes(0x55, 0x55)
    FIXED2K = False                # True: Aufteilung immer 2 KB je Socket (zum Pruefen)
    def setsizes(s, rm, tm):
        s.rmsr, s.tmsr = rm, tm
        if s.FIXED2K: rm = tm = 0x55
        s.rx, s.tx = [], []
        b = 0x6000
        for i in range(4):
            sz = 1024 << ((rm >> (2 * i)) & 3); s.rx.append((b, sz)); b += sz
        b = 0x4000
        for i in range(4):
            sz = 1024 << ((tm >> (2 * i)) & 3); s.tx.append((b, sz)); b += sz
    # ---- Busschnittstelle ($C0n4..7) ----
    def io_rd(s, r):
        r &= 3
        if r == 0: return s.mode
        if r == 1: return s.addr >> 8
        if r == 2: return s.addr & 0xFF
        s.trickle()
        v = s.rd(s.addr); s.inc(); return v
    def io_wr(s, r, v):
        r &= 3; v &= 0xFF
        if r == 0:
            if v & 0x80: s.reset(); s.mode = 0
            else: s.mode = v
        elif r == 1: s.addr = (v << 8) | (s.addr & 0xFF)
        elif r == 2: s.addr = (s.addr & 0xFF00) | v
        else: s.wr(s.addr, v); s.inc()
    def inc(s):
        if s.mode & 2:
            s.addr += 1
            if s.addr in (0x6000, 0x8000): s.addr -= 0x2000
    # ---- Register ----
    def be(s, a): return (s.mem[a] << 8) | s.mem[a + 1]
    def setbe(s, a, v): s.mem[a] = (v >> 8) & 0xFF; s.mem[a + 1] = v & 0xFF
    def rd(s, a):
        a &= 0x7FFF
        if 0x400 <= a < 0x800:
            i, reg = (a >> 8) - 4, a & 0xFF
            if reg == 3: return s.sock[i]['st']
            if reg in (0x20, 0x21):
                v = s.tx[i][1] - s.txused(i); return (v >> 8) if reg == 0x20 else v & 0xFF
            if reg in (0x26, 0x27):
                v = s.rsr(i); return (v >> 8) if reg == 0x26 else v & 0xFF
        return s.mem[a]
    def wr(s, a, v):
        a &= 0x7FFF
        if a == 0x1A: s.mem[a] = v; s.setsizes(v, s.tmsr); return
        if a == 0x1B: s.mem[a] = v; s.setsizes(s.rmsr, v); return
        if 0x400 <= a < 0x800 and (a & 0xFF) == 1:
            s.cmd((a >> 8) - 4, v); s.mem[a] = 0; return
        s.mem[a] = v
    def txused(s, i):
        b = s.SREG + i * 0x100; sz = s.tx[i][1]
        return (s.be(b + 0x24) - s.be(b + 0x22)) % sz
    def rsr(s, i):
        b = s.SREG + i * 0x100; sz = s.rx[i][1]
        return (s.sock[i]['rxwr'] - s.be(b + 0x28)) & 0xFFFF
    # ---- Befehle ----
    def cmd(s, i, c):
        b = s.SREG + i * 0x100; so = s.sock[i]
        s.net.log.append(('cmd', i, c))
        if c == 0x01:
            mr = s.mem[b] & 0x0F
            so.update(st={1: 0x13, 2: 0x22}.get(mr, 0), rxwr=0, pend=b'', srv=None, req=b'')
            for r in (0x22, 0x24, 0x28): s.setbe(b + r, 0)
        elif c == 0x04:
            ip = '.'.join(str(x) for x in s.mem[b + 0x0C:b + 0x10]); port = s.be(b + 0x10)
            s.net.log.append(('connect', ip, port))
            if port == 53: so['st'] = 0x17; so['srv'] = ('dns', 53)
            elif any(k[0] == ip and k[1] == port for k in s.net.pages):
                so['st'] = 0x17; so['srv'] = (ip, port)
                f = s.net.pages.get((ip, port, '*'))
                if callable(f):
                    rep = f(None)
                    if rep: s.put(i, rep)
            else: so['st'] = 0x00
        elif c in (0x08, 0x10):
            so.update(st=0, pend=b'')
        elif c == 0x20:
            data = s.txdata(i)
            if so['st'] == 0x22: s.udp(i, data)
            elif so['st'] in (0x17,) and so['srv'] == ('dns', 53):
                so['req'] += data
                q = so['req']
                if len(q) >= 2 and len(q) >= 2 + (q[0] << 8 | q[1]):
                    rep = s.dnsr(q[2:]); so['closing'] = True
                    s.net.log.append(('dnstcp',))
                    s.put(i, struct.pack('>H', len(rep)) + rep)
            elif so['st'] in (0x17,) and callable(s.net.pages.get((so['srv'][0], so['srv'][1], '*'))):
                so['req'] += data          # Telnet/IRC: Gegenstelle als Funktion (Eingang -> Antwort)
                rep = s.net.pages[(so['srv'][0], so['srv'][1], '*')](data)
                if isinstance(rep, tuple): so['closing'] = True; rep = rep[0]   # (Daten, 'zu')
                if rep: s.put(i, rep)
            elif so['st'] in (0x17,) and so['srv'][1] == 70:
                so['req'] += data
                if so['req'].endswith(b'\r\n'):
                    sel = so['req'][:-2].decode('latin-1')
                    s.net.requests.append((so['srv'], so['req']))
                    rep = s.net.pages.get((so['srv'][0], 70, sel), b'3Nicht gefunden\t\terror.host\t1\r\n.\r\n')
                    so['closing'] = True; s.put(i, rep)
            elif so['st'] in (0x17,):
                so['req'] += data
                if b'\r\n\r\n' in so['req']: s.http(i)
        elif c == 0x40:
            s.feed(i)
    def txdata(s, i):
        b = s.SREG + i * 0x100; base, sz = s.tx[i]
        rd, wr = s.be(b + 0x22), s.be(b + 0x24)
        out = bytes(s.mem[base + ((rd + k) % sz)] for k in range((wr - rd) % sz))
        s.setbe(b + 0x22, wr)
        return out
    def put(s, i, data):
        so = s.sock[i]; so['pend'] += data; s.feed(i)
    TRICKLE = 0                    # >0: wie GSSquared Daten in Happen waehrend des Lesens nachliefern
    def trickle(s):
        if not s.TRICKLE: return
        s.tick = getattr(s, 'tick', 0) + 1
        if s.tick % 7: return
        for i in range(4):
            if s.sock[i]['pend']: s.feed(i, s.TRICKLE)
    def feed(s, i, limit=None):
        so = s.sock[i]; base, sz = s.rx[i]
        if s.TRICKLE and limit is None and so['pend']: limit = s.TRICKLE
        free = sz - s.rsr(i) - (1 if s.TRICKLE else 0)
        n = min(free, len(so['pend']))
        if limit is not None: n = min(n, limit)
        n = max(n, 0)
        for k in range(n):
            s.mem[base + ((so['rxwr'] + k) % sz)] = so['pend'][k]
        so['rxwr'] = (so['rxwr'] + n) & 0xFFFF
        so['pend'] = so['pend'][n:]
        if so.get('closing') and not so['pend']: so['st'] = 0x00 if s.TRICKLE else 0x1C
    # ---- UDP: DHCP und DNS ----
    def udp(s, i, data):
        b = s.SREG + i * 0x100
        dip = '.'.join(str(x) for x in s.mem[b + 0x0C:b + 0x10]); dport = s.be(b + 0x10)
        sport = s.be(b + 4)
        s.net.log.append(('udp', dip, dport, len(data)))
        if dport == 67 and s.net.dhcp: rep, src = s.dhcp(data), '10.0.2.2'
        elif dport == 53 and s.net.udp_dns: rep, src = s.dnsr(data), dip
        elif dport == 123 and getattr(s.net, 'ntp', None) is not None: rep, src = s.ntpr(data), dip
        else: return
        if rep is None: return
        hdr = bytes(int(x) for x in src.split('.')) + struct.pack('>HH', dport if dport != 67 else 67, len(rep))
        s.put(i, hdr + rep)
    def dhcp(s, d):
        if d[0] != 1: return None
        xid = d[4:8]; mac = d[28:34]
        opts = d[240:]; typ = None; k = 0
        while k < len(opts) and opts[k] != 255:
            if opts[k] == 0: k += 1; continue
            if opts[k] == 53: typ = opts[k + 2]
            k += 2 + opts[k + 1]
        s.net.log.append(('dhcp', typ))
        rt = {1: 2, 3: 5}.get(typ)
        if rt is None: return None
        p = bytearray(240)
        p[0] = 2; p[1] = 1; p[2] = 6; p[4:8] = xid
        p[16:20] = bytes([10, 0, 2, 15]); p[28:34] = mac
        p[236:240] = b'\x63\x82\x53\x63'
        p += bytes([53, 1, rt, 54, 4, 10, 0, 2, 2, 1, 4, 255, 255, 255, 0, 3, 4, 10, 0, 2, 2,
                    6, 8, 10, 0, 2, 3, 8, 8, 8, 8, 255])
        return bytes(p)
    def ntpr(s, q):
        s.net.log.append(('ntp', len(q), q[0]))
        if len(q) != 48 or q[0] != 0x1B: return None
        r = bytearray(48); r[0] = 0x1C; r[1] = 2
        r[40:44] = struct.pack('>I', (s.net.ntp + 2208988800) & 0xFFFFFFFF)
        return bytes(r)
    def dnsr(s, q):
        qid = q[:2]; k = 12; labels = []
        while q[k]:
            labels.append(q[k + 1:k + 1 + q[k]].decode()); k += 1 + q[k]
        name = '.'.join(labels); k += 5
        s.net.log.append(('dns', name))
        ip = s.net.dns.get(name)
        if ip is None:
            return qid + b'\x81\x83' + q[4:6] + b'\x00\x00\x00\x00\x00\x00' + q[12:k]
        ans = b'\xc0\x0c' + struct.pack('>HHIH', 5, 1, 60, 2) + b'\xc0\x0c'   # CNAME zuerst (wird uebersprungen)
        ans += b'\xc0\x0c' + struct.pack('>HHIH', 1, 1, 60, 4) + bytes(int(x) for x in ip.split('.'))
        return qid + b'\x81\x80' + q[4:6] + b'\x00\x02\x00\x00\x00\x00' + q[12:k] + ans
    # ---- HTTP ----
    def http(s, i):
        so = s.sock[i]; req = so['req']
        line = req.split(b'\r\n')[0].decode()
        path = line.split(' ')[1]
        s.net.requests.append((so['srv'], req))
        rep = s.net.pages.get((so['srv'][0], so['srv'][1], path))
        if rep is None:
            rep = b'HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404</h1>'
        if callable(rep): rep = rep(req)
        if rep == 'hang': return
        so['closing'] = True
        s.put(i, rep)
