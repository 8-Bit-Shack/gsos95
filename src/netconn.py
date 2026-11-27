# Gegenstelle einer TCP-Verbindung im kleinen Test-"Internet" (Net aus w5100sim):
# HTTP-Seiten, Gopher, Telnet/IRC-Funktionen. Fuer die Treiber-Simulatoren
# (Marinetti, FujiNet, WLAN-Modem), die keine W5100-Register nachbilden.
class Conn:
    def __init__(s, net, ip, port):
        s.net, s.ip, s.port = net, ip, port
        s.req = b''; s.out = bytearray(); s.closing = False
        f = net.pages.get((ip, port, '*'))
        if callable(f):
            rep = f(None)
            if isinstance(rep, tuple): s.closing = True; rep = rep[0]   # (Daten, 'zu')
            if rep: s.out += rep
    @staticmethod
    def exists(net, ip, port):
        return any(k[0] == ip and k[1] == port for k in net.pages)
    def send(s, data):
        f = s.net.pages.get((s.ip, s.port, '*'))
        s.req += data
        if callable(f):
            rep = f(data)
            if isinstance(rep, tuple): s.closing = True; rep = rep[0]
            if rep: s.out += rep
        elif s.port == 70:
            if s.req.endswith(b'\r\n'):
                sel = s.req[:-2].decode('latin-1')
                s.net.requests.append(((s.ip, s.port), s.req))
                s.out += s.net.pages.get((s.ip, 70, sel), b'3Nicht gefunden\t\terror.host\t1\r\n.\r\n')
                s.closing = True
        elif b'\r\n\r\n' in s.req and not s.closing:
            head, _, body = s.req.partition(b'\r\n\r\n')
            m = [l for l in head.split(b'\r\n') if l.lower().startswith(b'content-length:')]
            if m and len(body) < int(m[0].split(b':')[1]): return
            s.http()
    def http(s):
        req = s.req
        path = req.split(b'\r\n')[0].decode().split(' ')[1]
        s.net.requests.append(((s.ip, s.port), req))
        rep = s.net.pages.get((s.ip, s.port, path))
        if rep is None:
            rep = b'HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404</h1>'
        if callable(rep): rep = rep(req)
        if rep == 'hang': return
        s.closing = True
        s.out += rep
    def take(s, n):
        d = bytes(s.out[:n]); del s.out[:n]; return d
    def done(s):
        return s.closing and not s.out
