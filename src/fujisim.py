# Nachgebildete FujiNet am SmartPort (Slot 5) fuer die Tests im 65816-Simulator.
# Bildet die Firmware-Seite nach (fujinet-firmware lib/device/iwm, NDevice):
# SmartPort STATUS/CONTROL/READ/WRITE ueber FWEntry, Geraet "NETWORK" (Typ $11),
# Befehle 'O' (Modus, Uebersetzung, Adresse), 'C', 'S' (verfuegbar, verbunden, Fehler),
# 'M' (HTTP-Kanal: 0 Inhalt, 1 Kopfzeilen sammeln, 2 Kopfzeilen lesen, 4 POST-Daten).
import struct
from netconn import Conn

DEVS = [('FUJINET_DISK_0', 0x02), ('FUJINET', 0x10), ('NETWORK', 0x11), ('FN_CLOCK', 0x13)]

class FujiNet:
    def __init__(s, net, slot=5):
        s.net = net; s.slot = slot
        s.log = []
        s.ch = None
    def install(s, cpu):
        b = 0xC000 + s.slot * 0x100
        for o, v in ((1, 0x20), (3, 0x00), (5, 0x03), (7, 0x00), (0xFF, 0x0A)):
            cpu.mem[b + o] = v
        cpu.fwentry = s.fwentry
    def fwentry(s, cpu, sp, res):
        addr = cpu.rd16(sp)
        st = cpu.mem[addr:addr + 7]
        assert st[0] == 0x20 and st[6] == 0x60, 'Aufrufblock %s' % st.hex()
        disp = st[1] | st[2] << 8
        assert disp == 0xC000 + s.slot * 0x100 + 0x0A + 3, 'Einsprung %04X' % disp
        cmd = st[3]; cl = st[4] | st[5] << 8
        err, cnt = s.sp(cpu, cmd, cl)
        return res(8, [cnt >> 8, 0, cnt & 0xFF, 0, err, 0, 0x01 if err else 0, 0])
    def sp(s, cpu, cmd, cl):
        pc, unit = cpu.mem[cl], cpu.mem[cl + 1]
        buf = cpu.mem[cl + 2] | cpu.mem[cl + 3] << 8
        m = cpu.mem
        if cmd == 0:                                    # STATUS
            code = m[cl + 4]
            if unit == 0 and code == 0:
                m[buf] = len(DEVS); return 0, 8
            if code == 3:
                if not 1 <= unit <= len(DEVS): return 0x28, 0
                name, typ = DEVS[unit - 1]
                dib = bytes([0xF8, 0, 0, 0, len(name)]) + name.encode().ljust(16, b' ') + bytes([typ, 0, 0, 1])
                m[buf:buf + 25] = dib; return 0, 25
            assert DEVS[unit - 1][1] == 0x11 and pc == 4, 'Status %r' % (unit, code, pc)
            assert m[cl + 5] == 1, 'Netzwerk-Einheit'
            if code == ord('S'):
                av, con, e = s.status()
                m[buf:buf + 4] = struct.pack('<HBB', av, con, e); return 0, 4
            return 0x21, 0
        if cmd == 4:                                    # CONTROL
            assert DEVS[unit - 1][1] == 0x11 and pc == 4 and m[cl + 5] == 1
            code = m[cl + 4]; n = m[buf] | m[buf + 1] << 8; d = bytes(m[buf + 2:buf + 2 + n])
            s.log.append(('ctl', chr(code), d))
            if code == ord('O'): s.lastaux2 = d[1]; return s.open(d[0], d[2:].split(b'\0')[0].decode()), 0
            if code == ord('C'): s.ch = None; return 0, 0
            if code in (0xFD, 0xFE):                    # Benutzer / Passwort (fuer SSH, SMB ...)
                v = d.split(b'\0')[0].decode()
                if code == 0xFD: s.login = v
                else: s.password = v
                return 0, 0
            if code == ord('M'):
                if not s.ch or s.ch['kind'] != 'http': return 0x27, 0
                s.ch['mode'] = d[1]; return 0, 0
            return 0x21, 0
        if cmd in (8, 9):                               # READ / WRITE
            assert pc == 5 and m[cl + 6] == 1
            n = m[cl + 4] | m[cl + 5] << 8
            assert n <= 512
            if cmd == 9: return s.write(bytes(m[buf:buf + n])), n
            d = s.read(n)
            if d is None: return 0x27, 0
            m[buf:buf + len(d)] = d; return 0, len(d)
        return 0x01, 0
    # ---- Netzwerkkanal ----
    def open(s, mode, spec):
        s.net.log.append(('fopen', mode, spec))
        if not spec.startswith('N:'): return 0x27
        proto, _, rest = spec[2:].partition('://')
        hostport, _, path = rest.partition('/')
        host, _, port = hostport.partition(':')
        ip = s.net.dns.get(host)
        if proto == 'TCP':
            port = int(port)
            if ip is None or not Conn.exists(s.net, ip, port): s.ch = None; return 0x27
            s.ch = dict(kind='tcp', c=Conn(s.net, ip, port)); return 0
        if proto == 'SSH':                              # Karte verschluesselt; Anmeldung mit $FD/$FE
            port = int(port) if port else 22
            ok = s.net.ssh.get(host) == (getattr(s, 'login', ''), getattr(s, 'password', ''))
            if ip is None or not ok or not Conn.exists(s.net, ip, port): s.ch = None; return 0x27
            s.ch = dict(kind='tcp', c=Conn(s.net, ip, port)); return 0
        if proto == 'FTP':                              # Karte meldet sich selbst an (anonymous)
            s.aux2 = getattr(s, 'lastaux2', 0)
            d = getattr(s.net, 'ftp', {}).get((host, '/' + path, mode))
            if ip is None or d is None: s.ch = None; return 0x27
            s.ch = dict(kind='ftp', body=bytearray(d)); return 0
        if proto in ('HTTP', 'HTTPS'):
            port = int(port) if port else (443 if proto == 'HTTPS' else 80)
            s.ch = dict(kind='http', ip=ip, port=port, path='/' + path, method='POST' if mode == 13 else 'GET',
                        mode=0, collect=[], post=b'', body=None, err=1, hdr=b'')
            return 0x27 if ip is None else 0
        return 0x27
    def trans(s):
        c = s.ch
        if c['body'] is not None: return
        rep = s.net.pages.get((c['ip'], c['port'], c['path']))
        s.net.requests.append(((c['ip'], c['port']), (c['method'], c['path'], c['post'], list(c['collect']))))
        if callable(rep): rep = rep(c['post'])
        if rep is None: c['body'] = b''; c['err'] = 170; return
        c['body'] = bytearray(rep); c['err'] = 1
        c['hdr'] = b''.join(b'text/html\x9b' for h in c['collect'] if h.lower() == b'content-type')
    def status(s):
        c = s.ch
        if c is None: return 0, 0, 207
        if c['kind'] == 'ftp':
            av = len(c['body']); return av, 1 if av else 0, 1 if av else 136
        if c['kind'] == 'tcp':
            done = c['c'].done()
            return len(c['c'].out), 0 if done else 1, 136 if done else 1
        if c['mode'] in (1, 4): return 0, 1, 1
        s.trans()
        if c['mode'] == 2: return len(c['hdr']), 0, 136 if not c['hdr'] else 1
        av = len(c['body'])
        if c['err'] != 1: return 0, 0, c['err']
        return av, 1 if av else 0, 1 if av else 136
    def write(s, d):
        c = s.ch
        if c is None: return 0x27
        if c['kind'] == 'tcp': c['c'].send(d); return 0
        if c['mode'] == 1:
            assert c['method'] == 'GET', 'Kopfzeilen nur bei GET (Firmware)'
            c['collect'].append(d.rstrip(b'\r\n\x9b')); return 0
        if c['mode'] == 4: c['post'] += d; return 0
        return 0x27
    def read(s, n):
        c = s.ch
        if c is None: return None
        if c['kind'] == 'tcp': return c['c'].take(n)
        if c['kind'] == 'ftp': d = bytes(c['body'][:n]); del c['body'][:n]; return d
        s.trans()
        if c['mode'] == 2:
            d = c['hdr'][:n]; return d + bytes(n - len(d))      # Firmware kopiert immer n Bytes
        d = bytes(c['body'][:n]); del c['body'][:n]; return d
