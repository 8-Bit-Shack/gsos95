# v2.18: Software-Archiv (Verzeichnisseiten als Liste, Downloads mit Namen) und Nachrichten (RSS/Atom)
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
from t3 import FILES, FS
ARC = '10.1.1.40'
net.dns['mirrors.apple2.org.za'] = ARC
net.dns['feeds.example'] = '10.1.1.41'
R = '/ftp.apple.asimov.net/'
H = b'HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n'
# eigene Listen-Seite (wie "Bootstrap Listr"): absolute https-Links, Brotkrumen, Sortier-Links, doppelte Links
def listr(path, dirs, files):
    base = 'https://mirrors.apple2.org.za' + path
    h = ('<!DOCTYPE html><html><head><meta charset="utf-8"><title>Index of %s</title>'
         '<link rel="stylesheet" href="/x.css"></head><body><nav><a href="https://apple2.org.za/@x">A2</a> '
         '<a href="https://mirrors.apple2.org.za/">mirrors</a> / <a href="%s">here</a></nav>'
         '<table><thead><tr><th><a href="?sort=name">Name</a></th><th><a href="#size">Size</a></th></tr></thead><tbody>'
         % (path, base))
    for d in dirs:
        h += '<tr><td class="icon"><a href="%s%s/"><i class="fa"></i></a></td><td><a href="%s%s/">%s</a></td><td>-</td></tr>' % (base, d, base, d, d)
    for f in files:
        h += ('<tr><td><a  class="x" HREF=\'%s%s\'><i></i></a></td><td><a href="%s%s">%s</a></td><td>389 bytes</td>'
              '<td><a href="#" data-href="%s%s">Download</a></td></tr>') % (base, f, base, f, f, base, f)
    return H + (h + '</tbody></table><footer><a href="https://apple2.org.za/safety.php">Safety</a></footer></body></html>').encode()
net.pages[(ARC, 80, R)] = listr(R, ['documentation', 'images', 'unsorted'], ['MIRROR.LIST.txt', 'site_index.txt'])
# nginx-Stil, relativ
NG = (b'<html>\r\n<head><title>Index of /ftp.apple.asimov.net/images/games/</title></head>\r\n<body>\r\n'
      b'<h1>Index of /ftp.apple.asimov.net/images/games/</h1><hr><pre><a href="../">../</a>\r\n'
      b'<a href="action/">action/</a>                                            01-Jan-2020 00:00       -\r\n'
      b'<a href="Lode%20Runner%20(1983).dsk">Lode Runner (1983).dsk</a>             01-Jan-2020 00:00  143360\r\n'
      b'<a href="ultima_iv_quest_of_the_avatar.zip">ultima_iv_quest_of_the_avatar.zip</a> 01-Jan-2020 00:00  100\r\n'
      b'</pre><hr></body>\r\n</html>\r\n')
net.pages[(ARC, 80, R + 'images/games/')] = H + NG
DSK = bytes((i * 13) & 0xFF for i in range(143360))
net.pages[(ARC, 80, R + 'images/games/Lode%20Runner%20(1983).dsk')] = (
    b'HTTP/1.0 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n' + DSK)
ZIP = b'PK\x03\x04' + bytes(96)
net.pages[(ARC, 80, R + 'images/games/ultima_iv_quest_of_the_avatar.zip')] = (
    b'HTTP/1.0 200 OK\r\nContent-Type: application/zip\r\n\r\n' + ZIP)
# grosse Liste (> 32 KB, kommt stueckweise)
BIGN = 700
net.pages[(ARC, 80, R + 'unsorted/')] = listr(R + 'unsorted/', [], ['file%04d.shk' % i for i in range(BIGN)])
# Feeds
RSS = (b'HTTP/1.0 200 OK\r\nContent-Type: application/rss+xml; charset=UTF-8\r\n\r\n'
       b'<?xml version="1.0" encoding="UTF-8"?><rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom" '
       b'xmlns:dc="http://purl.org/dc/elements/1.1/"><channel><title>A2Central.com</title>'
       b'<atom:link href="https://www.a2central.com/feed/" rel="self" type="application/rss+xml" />'
       b'<link>https://www.a2central.com</link><description>Apple II news</description>'
       b'<image><title>Logo</title><url>x.png</url></image>'
       b'<item><title>FishNDA recovered</title><link>https://www.a2central.com/fish/</link>'
       b'<dc:creator><![CDATA[Ryan]]></dc:creator><pubDate>Sat, 31 Oct 2026 12:00:00 +0000</pubDate>'
       b'<description><![CDATA[<p>Ryan Suenaga&#8217;s <b>FishNDA</b> is back.</p><img src="x.png">]]></description>'
       b'<content:encoded><![CDATA[<p>Lang lang lang</p>]]></content:encoded></item>'
       b'<item><title>a2pix &amp; mehr</title><link>https://www.a2central.com/a2pix/</link>'
       b'<description>&lt;p&gt;Version ' + b'x' * 300 + b'&lt;/p&gt;</description></item>'
       b'</channel></rss>')
ATOM = (b'HTTP/1.0 200 OK\r\nContent-Type: application/atom+xml\r\n\r\n'
        b'<?xml version="1.0" encoding="utf-8"?><feed xmlns="http://www.w3.org/2005/Atom"><title>heise online</title>'
        b'<link href="https://www.heise.de/" rel="alternate"/><updated>2026-11-25T10:00:00Z</updated>'
        b'<entry><title type="html">Neuer IIgs-Klon</title><link rel="replies" href="https://h/c"/>'
        b'<link href="https://www.heise.de/news/1.html" rel="alternate"/><id>1</id>'
        b'<updated>2026-11-25T09:00:00Z</updated><summary type="html">Kurz &amp;amp; knapp</summary></entry>'
        b'<entry><title>Zwei</title><link href="https://www.heise.de/news/2.html"/>'
        b'<content type="xhtml"><div xmlns="http://www.w3.org/1999/xhtml"><p>Inhalt zwei</p></div></content></entry></feed>')
net.pages[('10.1.1.41', 80, '/rss')] = RSS
net.pages[('10.1.1.41', 80, '/atom')] = ATOM

def links():
    b = w('NWB') << 16; t = bytes(cpu.mem[b + 0x8000:b + w('TXTEND')]); out = []; k = 0; cur = None
    while k < len(t):
        c = t[k]
        if c == 1:
            n = t[k + 1]; cur = [t[k + 2:k + 2 + n].decode('latin-1'), '']; k += 2 + n; continue
        if c == 2 and cur: out.append(tuple(cur)); cur = None
        elif c in (0x14, 0x18, 0x19, 0x1A, 0x1C, 0x1D): k += 2; continue
        elif c == 0x17: k += 3; continue
        elif cur and c >= 0x20: cur[1] += chr(c) if c < 0x80 else chr(c)
        k += 1
    return out

click(101, 193)
typ('about:archiv'); key(0x0D)
L = lines(); print(L[:12])
check(bstr(0xFC00) == b'Software-Archiv', 'Titel Software-Archiv: %r' % bstr(0xFC00))
check(bstr(0xFD80) == b'http://mirrors.apple2.org.za/ftp.apple.asimov.net/', 'Adresse: Asimov-Spiegel')
lk = links(); print(lk)
check(lk == [('documentation/', 'documentation/'), ('images/', 'images/'), ('unsorted/', 'unsorted/'),
             ('MIRROR.LIST.txt', 'MIRROR.LIST.txt'), ('site_index.txt', 'site_index.txt')],
      'nur Ordner und Dateien, je einmal, relativ')
check(not any('..' in l for l in L), 'Wurzel: kein ".."')
render(cpu, 'twa_1.png', 3)
typ('mirrors.apple2.org.za/ftp.apple.asimov.net/images/games/'); key(0x0D)
lk = links(); print(lines()[:10], lk)
check(lk[0] == ('../', '.. (übergeordneter Ordner)'.replace('ü', '\x82')), 'Unterordner: ".." oben')
check(lk[1:] == [('action/', 'action/'), ('Lode%20Runner%20(1983).dsk', 'Lode Runner (1983).dsk'),
                 ('ultima_iv_quest_of_the_avatar.zip', 'ultima_iv_quest_of_the_avatar.zip')], 'nginx-Liste: %r' % lk[1:])
check(any('/images/games/' in l for l in lines()), 'Pfad angezeigt')
render(cpu, 'twa_2.png', 3)
# Download mit langem Namen
typ('mirrors.apple2.org.za/ftp.apple.asimov.net/images/games/Lode%20Runner%20(1983).dsk'); key(0x0D)
dk = [x for x in FILES if 'DOWNLOADS' in x.upper()]; print(dk)
check(any(x.upper().endswith(':DOWNLOADS:LODE.RUNNER.DSK') and FILES[x] == DSK for x in dk), 'Disk-Image als Lode.Runner.dsk')
typ('mirrors.apple2.org.za/ftp.apple.asimov.net/images/games/ultima_iv_quest_of_the_avatar.zip'); key(0x0D)
dk = [x for x in FILES if 'DOWNLOADS' in x.upper()]; print(dk)
check(any(x.upper().endswith(':DOWNLOADS:ULTIMA.IV.Q.ZIP') and FILES[x] == ZIP for x in dk), 'ultima.iv.q.zip')
ty = [e for kk, v in FS.items() if kk.upper().endswith('DOWNLOADS') for e in v]; print(ty)
check(any(n.upper() == 'ULTIMA.IV.Q.ZIP' and t == 0xE0 for n, t in ty), 'Typ $E0 (Endung .zip erkannt)')
# grosse Liste
typ('mirrors.apple2.org.za/ftp.apple.asimov.net/unsorted/'); key(0x0D)
lk = links(); print(len(lk), lk[:3], lk[-2:])
check(len(lk) > 500 and lk[1][0] == 'file0000.shk' and all(lk[i][0] == 'file%04d.shk' % (i - 1) for i in range(1, len(lk))), 'grosse Liste stueckweise, lueckenlos (%d)' % len(lk))
check(len(lk) == BIGN + 1 or any('gek' in l for l in lines()[-4:]), 'vollstaendig oder "gekuerzt" am Ende')
# andere Seiten bleiben unveraendert
typ('frogfind.com'); key(0x0D)
check(bstr(0xFC00) == b'FrogFind!' and any('Search Engine' in l for l in lines()), 'normale Seite unveraendert')
# about:archiv=URL
typ('about:archiv=http://feeds.example/arc/'); key(0x0D)
f = [x for x in FILES if x.endswith('GSOS95.NET')]
check(f and FILES[f[0]][1828:1828 + 26] == b'\x19http://feeds.example/arc/', 'Archiv-Adresse gespeichert')
typ('about:archiv='); key(0x0D)
check(FILES[f[0]][1828] == 0, 'zurueck auf Vorgabe')
# ---- Nachrichten
typ('about:rss'); key(0x0D)
lk = links(); print(lk)
check(bstr(0xFC00) == b'Nachrichten' and ('https://www.a2central.com/feed/', 'A2Central') in lk, 'Nachrichten-Seite')
typ('feeds.example/rss'); key(0x0D)
L = lines(); print(L); lk = links(); print(lk)
check(bstr(0xFC00) == b'A2Central.com', 'Feed-Titel: %r' % bstr(0xFC00))
check(L[0] == 'A2Central.com', 'Ueberschrift')
check(('https://www.a2central.com/fish/', 'Weiterlesen') in lk and ('https://www.a2central.com/a2pix/', 'Weiterlesen') in lk, 'Links je Meldung')
t = ' '.join(L)
check('FishNDA recovered' in t and 'a2pix & mehr' in t, 'Titel der Meldungen')
check('Ryan Suenaga' in t and 'is back.' in t and '<p>' not in t and 'Lang lang' not in t, 'Text ohne Tags, nur description')
check('+0000 Ryan' in t and 'Sat, 31 Oct 2026' in t and 'Ryan]' not in t and 'Logo' not in t, 'Datum, sonst nichts')
check('Version' in t and t.count('x') < 210 and 'x...' in t, 'langer Text gekuerzt (%d)' % t.count('x'))
render(cpu, 'twa_3.png', 3)
typ('feeds.example/atom'); key(0x0D)
L = lines(); print(L); lk = links(); print(lk)
t = ' '.join(L)
check(bstr(0xFC00) == b'heise online', 'Atom: Titel')
check(lk == [('https://www.heise.de/news/1.html', 'Weiterlesen'), ('https://www.heise.de/news/2.html', 'Weiterlesen')], 'Atom: Links (rel=alternate)')
check('Neuer IIgs-Klon' in t and 'Kurz & knapp' in t and 'Inhalt zwei' in t and '2026-11-25T09:00:00Z' in t, 'Atom: Titel, Text, Datum')
render(cpu, 'twa_4.png', 3)
print('ALLES OK' if ok else 'FAIL Archiv/Nachrichten')
