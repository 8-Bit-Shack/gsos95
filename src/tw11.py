# HTTPS ueber Proxy (Uthernet II)
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
from t3 import FILES
net.pages[('10.1.1.30', 8080, 'https://secure.example/x')] = b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<title>Sicher</title>ok'
click(101, 193)
typ('https://secure.example/x'); key(0x0D)
check(bstr(0xFC00) != b'Sicher', 'ohne Proxy: Fehlerseite (%r)' % bstr(0xFC00))
typ('about:proxy=10.1.1.30:8080'); key(0x0D)
L = lines(); print(L)
check(any('10.1.1.30:8080' in l for l in L), 'Proxy angezeigt')
k = [x for x in FILES if x.endswith('GSOS95.NET')]
check(k and FILES[k[0]][106:106 + 15] == b'\x0e10.1.1.30:8080', 'Proxy gespeichert')
typ('https://secure.example/x'); key(0x0D)
r = net.requests[-1]; print(r)
check(r[0] == ('10.1.1.30', 8080) and r[1].startswith(b'GET https://secure.example/x HTTP/1.0\r\nHost: secure.example\r\n'), 'Anfrage an den Proxy')
check(bstr(0xFC00) == b'Sicher' and bstr(0xFD80) == b'https://secure.example/x', 'Seite geladen')
print('ALLES OK' if ok else 'FEHLER')
