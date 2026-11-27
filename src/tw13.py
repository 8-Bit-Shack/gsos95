# Bilder in der Seite
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
exec(open('tw9.py').read().split("G1, im1")[0].split("exec(open('tw1.py')")[1].split('\n',1)[1])
GA, ima, pala = mkgif(80, 40)
GB, imb, palb = mkgif(200, 300)
GS, ims, pals = mkgif(1, 1)
for p, g in (('/a.gif', GA), ('/b.gif', GB), ('/spacer.gif', GS)):
    net.pages[('10.1.1.1', 80, p)] = b'HTTP/1.0 200 OK\r\nContent-Type: image/gif\r\n\r\n' + g
net.pages[('10.1.1.1', 80, '/p')] = (b'HTTP/1.0 200 OK\r\n\r\n<title>Bilder</title><p>Oben</p><img src="a.gif">'
    b'<p>Mitte <img src="/spacer.gif"> Text</p><a href="/x"><img src="b.gif" alt="&nbsp; Hallo &amp; Tsch&uuml;ss &nbsp"></a><p>Unten</p><img src="missing.gif">')
click(101, 193)
typ('frogfind.com/p'); key(0x0D)
settle()
print('IMGN', w('IMGN'), [cpu.rd16(ADDR + lbl['IMGH'] + 2 * i) for i in range(4)], [cpu.rd16(ADDR + lbl['IMGW'] + 2 * i) for i in range(4)])
check(w('IMGN') == 4, 'vier Bilder eingetragen')
H = [cpu.rd16(ADDR + lbl['IMGH'] + 2 * i) for i in range(4)]
W = [cpu.rd16(ADDR + lbl['IMGW'] + 2 * i) for i in range(4)]
check(H[0] == 40 and W[0] == 80, 'Bild 1 80x40')
check(H[1] == 0 and H[3] == 0, 'Platzhalter und fehlendes Bild unsichtbar')
check(0 < H[1 + 1] <= 120 and W[2] <= 120, 'grosses Bild verkleinert (%dx%d)' % (W[2], H[2]))
b = w('IMBK') << 16; o = cpu.rd16(ADDR + lbl['IMGO'])
check(bytes(cpu.mem[b + o:b + o + 80 * 40]) == expect(ima, pala, 1, 80, 40), 'Bildpunkte Bild 1')
L = lines(); print(L, w('NLINES'))
check(w('NLINES') >= 4 + 4 + 2, 'Bildzeilen angelegt (%d)' % w('NLINES'))
check(any('Mitte' in l for l in L) and any('Unten' in l for l in L), 'Text drumherum')
check(w('STMSG') == lbl['ws_ready'], 'Statuszeile Fertig')
check(any('[  Hallo & Tsch' in l for l in L) and not any('nbsp' in l or 'uuml' in l for l in L), 'Entitaeten im alt-Text')
check(cpu.rd16(ADDR + lbl['IMGLK'] + 4) != 0 and cpu.rd16(ADDR + lbl['IMGLK']) == 0, 'Bild 3 steht im Link, Bild 1 nicht')
render(cpu, 'tw13a.png', 3)
for i in range(5): key(0x0A)
render(cpu, 'tw13b.png', 3)
for i in range(5): key(0x0B)
click(40, 90)                           # auf Bild 1 klicken
print(bstr(0xFC00), bstr(0xFD80))
check(bstr(0xFD80) == b'http://frogfind.com/a.gif' and w('GIFON') == 1, 'Klick zeigt das Bild gross')
print('ALLES OK' if ok else 'FEHLER')
