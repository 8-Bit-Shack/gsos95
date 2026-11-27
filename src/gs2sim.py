# Uthernet II wie in GSSquared 1.x nachgebildet (src/devices/uthernet2/w5100_chip.cpp + net_worker.cpp):
# Befehle setzen den Status sofort lokal und schicken eine Anfrage an einen Arbeitsthread; der
# meldet Statuswechsel und Empfangsdaten spaeter als Ereignisse, die bei jedem Buszugriff
# eingesammelt werden. TCP-Empfang mit "Credits", Gegenstelle schliesst -> CLOSED (kein CLOSE_WAIT),
# DHCP bekommt (ohne Root-Rechte auf dem Mac) keine Antwort.
import struct
from w5100sim import W5100, Net

class GS2(W5100):
    PERIOD = 40          # Buszugriffe zwischen zwei Laeufen des Arbeitsthreads
    CONN_DELAY = 3       # Laeufe bis ESTABLISHED
    UDP_DELAY = 2
    DHCP_OK = False
    def reset(s):
        W5100.reset(s)
        s.reqq, s.evq = [], []
        s.host = [s._h() for _ in range(4)]
        s.pending = [b''] * 4
        s.rsrv = [0] * 4
        s.cred = [0] * 4
        s.cnt = 0
        s.udpout = []
    def _h(s):
        return dict(kind=None, st=0, buf=b'', closing=False, srv=None, req=b'', wait=0, port=0)
    # ---- Bus ----
    def tick(s):
        s.cnt += 1
        if s.cnt % s.PERIOD == 0: s.worker()
        s.drain()
    def io_rd(s, r):
        s.tick(); return W5100.io_rd(s, r)
    def io_wr(s, r, v):
        if (r & 3) == 0 and (v & 0x80):
            for i in range(4): s.reqq.append(('closehost', i))
        s.tick(); W5100.io_wr(s, r, v)
    def rsr(s, i): return s.rsrv[i]
    def hdr(s, i): return 8 if s.sock[i]['st'] == 0x22 else 0
    def rx_free(s, i):
        used = s.rsrv[i] + s.hdr(i)
        sz = s.rx[i][1]
        return 0 if used + 1 >= sz else sz - used - 1
    def wbyte(s, i, v):
        base, sz = s.rx[i]; so = s.sock[i]
        s.mem[base + so['rxwr']] = v
        so['rxwr'] = (so['rxwr'] + 1) % sz
        s.rsrv[i] += 1
    def flush(s, i):
        while s.pending[i]:
            n = min(s.rx_free(i), len(s.pending[i]))
            if n == 0: return
            for c in s.pending[i][:n]: s.wbyte(i, c)
            s.pending[i] = s.pending[i][n:]
    def drain(s):
        for i in range(4):
            if s.pending[i]: s.flush(i)
        while s.evq:
            e = s.evq.pop(0); i = e[1]
            if e[0] == 'st':
                s.sock[i]['st'] = e[2]
                if e[2] == 0: s.cred[i] = 0; s.pending[i] = b''
            elif e[0] == 'tcp':
                s.flush(i)
                d = e[2]; n = min(s.rx_free(i), len(d))
                for c in d[:n]: s.wbyte(i, c)
                s.pending[i] += d[n:]
            elif e[0] == 'udp':
                ip, port, d = e[2], e[3], e[4]
                if s.rsrv[i] + len(d) + s.hdr(i) < s.rx[i][1]:
                    for c in bytes(int(x) for x in ip.split('.')) + struct.pack('>HH', port, len(d)) + d:
                        s.wbyte(i, c)
                else:
                    s.net.log.append(('udpdrop', i))
    # ---- Befehle (Gastseite) ----
    def cmd(s, i, c):
        b = s.SREG + i * 0x100; so = s.sock[i]
        s.net.log.append(('cmd', i, c))
        if c == 0x01:
            mr = s.mem[b] & 0x0F
            s.reqq.append(('closehost', i))
            if mr == 1: s.reqq.append(('opentcp', i)); so['st'] = 0x13
            elif mr == 2: s.reqq.append(('openudp', i, s.be(b + 4))); so['st'] = 0x22
            else: so['st'] = 0
            so['rxwr'] = 0; s.rsrv[i] = 0
            for r in (0x22, 0x24, 0x28): s.setbe(b + r, 0)
            s.pending[i] = b''
            s.cred[i] = s.rx[i][1] - 1 if mr == 1 else 0
        elif c == 0x04:
            ip = '.'.join(str(x) for x in s.mem[b + 0x0C:b + 0x10]); port = s.be(b + 0x10)
            s.net.log.append(('connect', ip, port))
            s.reqq.append(('connect', i, ip, port)); so['st'] = 0x15
        elif c in (0x08, 0x10):
            s.reqq.append(('close', i)); s.cred[i] = 0; s.pending[i] = b''; so['st'] = 0
        elif c == 0x20:
            data = s.txdata(i)
            if so['st'] == 0x17: s.reqq.append(('sendtcp', i, data))
            elif so['st'] == 0x22:
                dip = '.'.join(str(x) for x in s.mem[b + 0x0C:b + 0x10])
                s.reqq.append(('sendudp', i, dip, s.be(b + 0x10), data))
        elif c == 0x40:
            base, sz = s.rx[i]
            old = s.rsrv[i]
            rd = s.be(b + 0x28) & (sz - 1); wr = so['rxwr'] & (sz - 1)
            s.rsrv[i] = (wr - rd) % sz
            if old > s.rsrv[i]: s.cred[i] += old - s.rsrv[i]
            s.flush(i)
    # ---- Arbeitsthread (Wirtsseite) ----
    def worker(s):
        while s.reqq:
            s.handle(s.reqq.pop(0))
        for i in range(4):
            h = s.host[i]
            if h['kind'] == 'tcp' and h['st'] == 0x15:
                h['wait'] -= 1
                if h['wait'] <= 0:
                    if h['srv'] is None:
                        h['kind'] = None; h['st'] = 0
                    else:
                        h['st'] = 0x17
                        f = s.net.pages.get((h['srv'][0], h['srv'][1], '*'))
                        if callable(f):
                            rep = f(None)
                            if rep: h['buf'] += rep
                    s.evq.append(('st', i, h['st']))
            if h['kind'] == 'tcp' and h['st'] == 0x17:
                if s.cred[i] > 0 and h['buf']:
                    n = min(s.cred[i], 2048, len(h['buf']))
                    s.cred[i] -= n
                    s.evq.append(('tcp', i, h['buf'][:n])); h['buf'] = h['buf'][n:]
                elif not h['buf'] and h['closing']:
                    h['kind'] = None; h['st'] = 0
                    s.evq.append(('st', i, 0))
        for u in list(s.udpout):
            u[0] -= 1
            if u[0] <= 0:
                s.udpout.remove(u)
                if s.host[u[1]]['kind'] == 'udp': s.evq.append(('udp', u[1], u[2], u[3], u[4]))
    def handle(s, r):
        k, i = r[0], r[1]; h = s.host[i]
        if k in ('close', 'closehost'):
            s.host[i] = s._h()
            if k == 'close': s.evq.append(('st', i, 0))
        elif k == 'opentcp':
            s.host[i] = s._h(); s.host[i].update(kind='tcp', st=0x13); s.evq.append(('st', i, 0x13))
        elif k == 'openudp':
            s.host[i] = s._h(); s.host[i].update(kind='udp', st=0x22, port=r[2]); s.evq.append(('st', i, 0x22))
        elif k == 'connect':
            if h['kind'] != 'tcp': return
            ip, port = r[2], r[3]
            if port == 53: h['srv'] = ('dns', 53)
            elif any(q[0] == ip and q[1] == port for q in s.net.pages): h['srv'] = (ip, port)
            h['st'] = 0x15; h['wait'] = s.CONN_DELAY
            s.evq.append(('st', i, 0x15))
        elif k == 'sendtcp':
            if h['kind'] == 'tcp' and h['st'] == 0x17: s.serve(i, r[2])
        elif k == 'sendudp':
            if h['kind'] != 'udp': return
            dip, dport, data = r[2], r[3], r[4]
            s.net.log.append(('udp', dip, dport, len(data)))
            if dport == 67 and s.DHCP_OK: rep, src = s.dhcp(data), '10.0.2.2'
            elif dport == 53 and s.net.udp_dns: rep, src = s.dnsr(data), dip
            elif dport == 123 and getattr(s.net, 'ntp', None) is not None: rep, src = s.ntpr(data), dip
            else: return
            if rep: s.udpout.append([s.UDP_DELAY, i, src, dport, rep])
    def serve(s, i, data):
        h = s.host[i]; srv = h['srv']
        if srv == ('dns', 53):
            h['req'] += data; q = h['req']
            if len(q) >= 2 and len(q) >= 2 + (q[0] << 8 | q[1]):
                rep = s.dnsr(q[2:]); s.net.log.append(('dnstcp',))
                h['buf'] += struct.pack('>H', len(rep)) + rep; h['closing'] = True
            return
        f = s.net.pages.get((srv[0], srv[1], '*'))
        if callable(f):
            rep = f(data)
            if isinstance(rep, tuple): h['closing'] = True; rep = rep[0]
            if rep: h['buf'] += rep
            return
        h['req'] += data
        if srv[1] == 70:
            if h['req'].endswith(b'\r\n'):
                sel = h['req'][:-2].decode('latin-1')
                s.net.requests.append((srv, h['req']))
                h['buf'] += s.net.pages.get((srv[0], 70, sel), b'3Nicht gefunden\t\terror.host\t1\r\n.\r\n')
                h['closing'] = True
            return
        if b'\r\n\r\n' in h['req']:
            req = h['req']
            path = req.split(b'\r\n')[0].decode().split(' ')[1]
            s.net.requests.append((srv, req))
            rep = s.net.pages.get((srv[0], srv[1], path))
            if rep is None: rep = b'HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404</h1>'
            if callable(rep): rep = rep(req)
            if rep == 'hang': return
            h['buf'] += rep; h['closing'] = True
