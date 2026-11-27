# Nachgebildete CoGS-Karte (Register + Rahmenprotokoll laut CoGS Technical Reference)
# Nutzt dieselbe Net-Klasse wie w5100sim (dns, pages, requests, log).
import struct

class CoGS:
    def __init__(s, net, link=2, wifi=True):
        s.net = net; s.link = link; s.wifi = wifi
        s.rx = bytearray(); s.inb = bytearray()
        s.socks = {}
        s.certbad = set()          # Rechner mit ungueltigem Zertifikat
    def frame(s, op, payload=b''):
        s.rx += bytes([op, len(payload) & 0xFF, len(payload) >> 8]) + payload
    # ---- Register ($C0n0..F) ----
    def io_rd(s, r):
        r &= 15
        if r == 0:
            if not s.rx: return 0
            b = s.rx[0]; del s.rx[0]; return b
        if r == 1: return (0x80 if s.rx else 0) | 0x40 | (0x10 if s.link == 2 else 0)
        if r == 5: return 0xA2
        if r == 6: return 0x04
        return 0
    def io_wr(s, r, v):
        r &= 15; v &= 0xFF
        if r == 4:
            if v & 4: s.rx = bytearray()
            if v & 8: s.inb = bytearray()
            return
        if r == 0:
            s.inb.append(v); s.parse()
    def parse(s):
        while len(s.inb) >= 3:
            op = s.inb[0]; n = s.inb[1] | s.inb[2] << 8
            if len(s.inb) < 3 + n: return
            p = bytes(s.inb[3:3 + n]); del s.inb[:3 + n]
            s.cmd(op, p)
    def cmd(s, op, p):
        s.net.log.append(('cogs', op))
        if op == 0x00: s.frame(0x80, bytes([1, 31, 4, 0xFF]))
        elif op == 0x08:
            ip = bytes([192, 168, 2, 77]); sub = bytes([255, 255, 255, 0]); gw = bytes([192, 168, 2, 1])
            ssid = b'Labor' if s.wifi else b''
            s.frame(0x88, bytes([s.link]) + ip + sub + gw + gw + bytes(4) + bytes(6) + bytes([0xC8, len(ssid)]) + ssid + bytes(6))
        elif op == 0x0B:
            host = p.decode(); ip = s.net.dns.get(host)
            s.net.log.append(('cresolve', host))
            if ip: s.frame(0x8B, bytes([0, 1]) + bytes(int(x) for x in ip.split('.')))
            else: s.frame(0x8B, bytes([2, 0]))
        elif op == 0x10:
            sock, flags, port = p[0], p[1], p[2] | p[3] << 8
            host = p[5:5 + p[4]].decode()
            s.net.log.append(('cconnect', host, port, flags))
            ip = s.net.dns.get(host)
            if ip is None: res = 2
            elif flags & 1 and host in s.certbad: res = 6
            elif any(k[0] == ip and k[1] == port for k in s.net.pages): res = 0
            else: res = 4
            if res == 0: s.socks[sock] = dict(ip=ip, port=port, req=b'', tls=flags & 1)
            s.frame(0x90, bytes([sock, res]))
        elif op == 0x11:
            sock = p[0]; so = s.socks.get(sock)
            if so is None: s.frame(0x93, bytes([0xFF, 8, 0])); return
            s.frame(0x9A, bytes([sock, 0]))
            so['req'] += p[1:]
            if b'\r\n\r\n' in so['req']:
                req = so['req']; path = req.split(b'\r\n')[0].decode().split(' ')[1]
                s.net.requests.append(((so['ip'], so['port']), req))
                rep = s.net.pages.get((so['ip'], so['port'], path),
                                      b'HTTP/1.0 404 Not Found\r\n\r\n<h1>404</h1>')
                if callable(rep): rep = rep(req)
                for i in range(0, len(rep), 1000):
                    s.frame(0x91, bytes([sock]) + rep[i:i + 1000])
                s.frame(0x92, bytes([sock, 0]))
                del s.socks[sock]
        elif op == 0x12:
            s.socks.pop(p[0], None)
            s.frame(0x92, bytes([p[0], 0]))
        else:
            s.frame(0x93, bytes([0xFF, 11, 0]))
