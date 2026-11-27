# Cookies: Set-Cookie merken, an denselben Rechner/Domain zurueckschicken, fremde Rechner nicht,
# Ersetzen, Loeschen mit leerem Wert (ueber Marinetti, nachgebildet)
from twbase import *
import twbase
from marisim import Marinetti
net.dns['www.frogfind.com'] = '10.1.1.1'
net.dns['test.local'] = '10.1.1.2'
H = b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n'
net.pages[('10.1.1.1', 80, '/login')] = H + b'Set-Cookie: sid=abc123; Path=/; HttpOnly\r\nSet-Cookie: lang=de; Domain=.frogfind.com; Path=/\r\n\r\n<title>Login</title>ok'
def echo(req):
    c = [l for l in req.split(b'\r\n') if l.lower().startswith(b'cookie:')]
    return H + b'\r\n<title>' + (c[0][8:] if c else b'keine') + b'</title>x'
net.pages[('10.1.1.1', 80, '/check')] = echo
net.pages[('10.1.1.2', 80, '/check')] = echo
net.pages[('10.1.1.1', 80, '/neu')] = H + b'Set-Cookie: sid=xyz\r\n\r\n<title>Neu</title>ok'
net.pages[('10.1.1.1', 80, '/aus')] = H + b'Set-Cookie: sid=; Max-Age=0\r\n\r\n<title>Aus</title>ok'
cpu = boot(); cpu.mari = Marinetti(net)
settle()
click(101, 193)
typ('frogfind.com/check'); key(0x0D)
check(bstr(0xFC00) == b'keine', 'ohne Cookies kein Cookie-Kopf')
typ('frogfind.com/login'); key(0x0D)
typ('frogfind.com/check'); key(0x0D)
check(bstr(0xFC00) == b'sid=abc123; lang=de', 'Cookies zurueck: %r' % bstr(0xFC00))
typ('www.frogfind.com/check'); key(0x0D)
check(bstr(0xFC00) == b'lang=de', 'Unterdomain: nur Domain-Cookie: %r' % bstr(0xFC00))
typ('test.local/check'); key(0x0D)
check(bstr(0xFC00) == b'keine', 'fremder Rechner bekommt nichts: %r' % bstr(0xFC00))
typ('frogfind.com/neu'); key(0x0D)
typ('frogfind.com/check'); key(0x0D)
check(bstr(0xFC00) == b'sid=xyz; lang=de', 'ersetzt: %r' % bstr(0xFC00))
typ('frogfind.com/aus'); key(0x0D)
typ('frogfind.com/check'); key(0x0D)
check(bstr(0xFC00) == b'lang=de', 'geloescht: %r' % bstr(0xFC00))
print('ALLES OK' if twbase.ok else 'FAIL Cookies')
