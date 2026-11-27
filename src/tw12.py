# Lange Seiten (stueckweise), Tabellen (Tabstopps), Auswahllisten, Textarea
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
JUNK = b'<script>' + b''.join(b'var x%d = "<b>nicht zeigen</b>";\n' % i for i in range(1700)) + b'</script>'
PARAS = b''.join(b'<p>Absatz %d <b>fett</b> Ende</p>\n' % i for i in range(200))
BIG = b'<title>Lang</title>' + JUNK + PARAS + b'<p>SCHLUSS</p>'
print('Seite', len(BIG))
net.pages[('10.1.1.1', 80, '/big2')] = b'HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n' + BIG
net.pages[('10.1.1.1', 80, '/tab')] = (b'HTTP/1.0 200 OK\r\n\r\n<table><tr><td>A</td><td>B</td><td>C</td></tr>'
    b'<tr><td>Laenger</td><td>x</td><td>y</td></tr></table>')
net.pages[('10.1.1.1', 80, '/form')] = (b'HTTP/1.0 200 OK\r\n\r\n<form action="/s">'
    b'<select name=farbe><option>rot<option value=g>gr&uuml;n</option><option selected>blau</select><br>'
    b'<textarea name=t>Hallo\nWelt</textarea><br><input type=submit value=Los></form>')
net.pages[('10.1.1.1', 80, '/s?farbe=blau&t=Hallo+Welt')] = b'HTTP/1.0 200 OK\r\n\r\n<title>S1</title>ok'
net.pages[('10.1.1.1', 80, '/s?farbe=rot&t=Hallo+Welt')] = b'HTTP/1.0 200 OK\r\n\r\n<title>S2</title>ok'
click(101, 193)
typ('frogfind.com/big2'); key(0x0D)
L = lines()
print(len(L), L[:3], L[-3:])
T = '\n'.join(L)
check('Absatz 0 ' in T and 'Absatz 199' in T and 'SCHLUSS' in T, 'lange Seite vollstaendig')
check('nicht zeigen' not in T and 'gek' not in T, 'Skript ueber die Stueckgrenze uebersprungen')
check(bstr(0xFC00) == b'Lang', 'Titel')
typ('frogfind.com/tab'); key(0x0D)
L = lines(); print(L)
T2 = text(); print(T2)
check(T2.count(b'\x17') == 4, 'Spaltencodes')
render(cpu, 'tw12a.png', 3)
typ('frogfind.com/form'); key(0x0D)
print(lines())
check(w('HNFLD') == 3 and w('OPTN') == 3, 'Felder %d, Optionen %d' % (w('HNFLD'), w('OPTN')))
render(cpu, 'tw12b.png', 3)
click(30, 61)
print(net.requests[-1][1][:50])
check(net.requests[-1][1].startswith(b'GET /s?farbe=blau&t=Hallo+Welt '), 'Auswahl (selected) und Textarea gesendet')
key(0x08)
click(40, 41)
render(cpu, 'tw12c.png', 3)
click(30, 61)
print(net.requests[-1][1][:50])
check(net.requests[-1][1].startswith(b'GET /s?farbe=rot&t=Hallo+Welt '), 'Klick schaltet weiter (blau -> rot)')
print('ALLES OK' if ok else 'FEHLER')
