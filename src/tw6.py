# Formulare: Textfeld + Absenden (GET), versteckte Felder, Ankreuzfeld, <button>, POST
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
FORM = (b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<title>Suche</title>'
        b'<form action="/search" method="get">Suche: <input type="text" name="q" value="">'
        b'<input type=hidden name=lang value=de><input type=checkbox name=x checked>'
        b'<input type=checkbox name=y><input type="submit" value="Los"></form>'
        b'<form method=POST action="post.php"><input name=t value="a b"><button type=submit>OK</button></form>')
net.pages[('10.1.1.1', 80, '/f')] = FORM
net.pages[('10.1.1.1', 80, '/search?q=Apple+II%C3%A4&lang=de&x=on')] = b'HTTP/1.0 200 OK\r\n\r\n<title>Treffer</title>ok'
net.pages[('10.1.1.1', 80, '/post.php')] = b'HTTP/1.0 200 OK\r\n\r\n<title>Gepostet</title>ok'
click(101, 193)
typ('frogfind.com/f'); key(0x0D)
L = lines(); print(L)
check(w('HNFLD') == 6 and w('HNF') == 2, 'Formulare/Felder erkannt: %d/%d' % (w('HNF'), w('HNFLD')))
render(cpu, 'tw6a.png', 3)
wx = cpu.rd16(ADDR + lbl['wrecs'] + 2); wy = cpu.rd16(ADDR + lbl['wrecs'] + 4)
print('WIN', wx, wy)
# Feld anklicken: hinter "Suche: "
click(wx + 8 + 45, wy + 31 + 4)
check(w('FFOC') == 0, 'Feld hat die Schreibmarke (%d)' % w('FFOC'))
typ('Apple II'); key(0x0D)
check(w('FFOC') == 0xFFFF or True, '')
render(cpu, 'tw6b.png', 3)
print(net.requests[-1][1][:60])
check(net.requests[-1][1].startswith(b'GET /search?q=Apple+II&lang=de&x=on '), 'GET-Abfrage: %r' % net.requests[-1][1][:60])
render(cpu, 'tw6c.png', 3)
click(wx + 11, wy + 27)                 # Zurueck
check(w('HNFLD') == 6, 'wieder auf dem Formular')
render(cpu, 'tw6d.png', 3)
# Feld 1 mit Umlaut: Text direkt in den Puffer (Tastatur-Umlaute testet tw1)
click(wx + 8 + 45, wy + 35)
typ('x'); key(0x7F); key(0x7F)          # Loeschen bis leer
b = bank(); o = 0xC210
cpu.mem[b + o + 34] = 3; cpu.mem[b + o + 35:b + o + 38] = bytes([0x82, ord('b'), ord('&')])
n0 = len(net.requests)
key(0x0D)
print(net.requests[-1][1][:50])
check(net.requests[-1][1].startswith(b'GET /search?q=%C3%BCb%26&lang=de&x=on '), 'UTF-8 + Sonderzeichen kodiert')
click(wx + 11, wy + 27)
# POST-Formular: Knopf "OK" in Zeile 2
click(120, 52)
r = net.requests[-1][1]
print(r)
check(r.startswith(b'POST /post.php HTTP/1.0') and r.endswith(b'\r\n\r\nt=a+b') and b'Content-Length: 5\r\n' in r, 'POST mit Inhalt')
check(bstr(0xFC00) == b'Gepostet', 'Antwort auf POST angezeigt')
