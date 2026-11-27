# Favoriten, Startseite, Bildlage beim Zurueck, DNS-Zwischenspeicher, Download
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
from t3 import FILES, FS
BIG = bytes((i * 7 + (i >> 8)) & 0xFF for i in range(70000))
net.pages[('10.1.1.1', 80, '/files/game.shk')] = (
    b'HTTP/1.0 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: 70000\r\n\r\n' + BIG)
click(101, 193)
typ('test.local/x?a=1&b=2'); key(0x0D)
for i in range(6): key(0x0A)
pg = w('PAGE'); check(pg == 6, 'geblaettert auf %d' % pg)
typ('frogfind.com'); key(0x0D)
check(w('PAGE') == 0, 'neue Seite oben')
key(0x08)
check(bstr(0xFD80).startswith(b'http://test.local/x') and w('PAGE') == 6, 'zurueck: Bildlage %d' % w('PAGE'))
# DNS-Zwischenspeicher: frogfind.com zweimal laden -> eine Anfrage
typ('frogfind.com/page2'); key(0x0D)
nd = sum(1 for x in net.log if x == ('dns', 'frogfind.com'))
check(nd == 1, 'DNS nur einmal gefragt (%d)' % nd)
# Favorit merken (Strg-D) und Liste (Strg-B)
key(0x04)
k = [x for x in FILES if x.endswith('GSOS95.NET')]
lzf = [x for x in FILES if x.endswith('GSOS95.LZ')]
check(lzf and b'http://frogfind.com/page2' in FILES[lzf[0]], 'Lesezeichen gespeichert')
key(0x02)
L = lines(); print(L)
check(bstr(0xFC00) == b'Lesezeichen' and any('Zwei' in l for l in L), 'Lesezeichenseite')
check(bstr(0xF800) == b'about:lz', 'Adresse about:lz (%r)' % bstr(0xF800))
render(cpu, 'tw7a.png', 3)
# als Startseite
typ('about:lzhome=1'); key(0x0D)
check(FILES[k[0]][26:26 + 26] == b'\x19http://frogfind.com/page2', 'Startseite gespeichert')
typ('about:start'); key(0x0D)
n0 = len(net.requests)
click(32, 33)                           # Start-Knopf
check(len(net.requests) > n0 and net.requests[-1][1].startswith(b'GET /page2 '), 'Start-Knopf laedt eigene Startseite')
# Download
typ('frogfind.com/files/game.shk'); key(0x0D)
L = lines(); print(L)
dk = [x for x in FILES if 'DOWNLOADS' in x.upper()]
print(dk, [len(FILES[x]) for x in dk])
check(any(x.upper().endswith(':DOWNLOADS:GAME.SHK') and FILES[x] == BIG for x in dk), 'Datei vollstaendig gespeichert')
ty = [e for kk, v in FS.items() if kk.upper().endswith('DOWNLOADS') for e in v]
print(ty)
check(ty and ty[0][1] == 0xE0, 'Dateityp $E0')
check(any('69 KB' in l for l in L), 'Ergebnisseite')
render(cpu, 'tw7b.png', 3)
# Favorit loeschen
typ('about:lzdel=1'); key(0x0D)
check(b'page2' not in FILES[lzf[0]][:600], 'Lesezeichen geloescht')

print('ALLES OK' if ok else 'FEHLER')
