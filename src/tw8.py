# Gopher: Menue, Text, Suche, Datei
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
from t3 import FILES
net.dns['gopher.floodgap.com'] = '10.1.1.9'
MENU = (b'iWillkommen bei Floodgap\tfake\t(NULL)\t0\r\n'
        b'i\tfake\t(NULL)\t0\r\n'
        b'1Mehr Gopher\t/world\tgopher.floodgap.com\t70\r\n'
        b'0Ueber uns\t/about.txt\tgopher.floodgap.com\t70\r\n'
        b'7Veronica-2 Suche\t/v2/vs\tgopher.floodgap.com\t70\r\n'
        b'9Datei\t/f/prog.bxy\tgopher.floodgap.com\t70\r\n'
        b'hWebseite\tURL:http://frogfind.com/\tgopher.floodgap.com\t70\r\n'
        b'1Anderer Port\t/x y\tgopher.floodgap.com\t7070\r\n'
        b'.\r\n')
net.pages[('10.1.1.9', 70, '')] = MENU
net.pages[('10.1.1.9', 70, '/about.txt')] = b'Zeile eins\r\nZeile  zwei\r\n.\r\n'
net.pages[('10.1.1.9', 70, '/v2/vs\tapple iigs')] = b'1Treffer: Apple IIgs\t/a2\tgopher.floodgap.com\t70\r\n.\r\n'
BIN = bytes(range(256)) * 200
net.pages[('10.1.1.9', 70, '/f/prog.bxy')] = BIN
click(101, 193)
typ('gopher://gopher.floodgap.com'); key(0x0D)
L = lines(); print(L)
check(net.requests[-1][1] == b'\r\n', 'Anfrage Hauptmenue %r' % net.requests[-1][1])
check(L[0].startswith('Willkommen bei Floodgap'), 'Info-Zeile')
check(any('> <Mehr Gopher>' in l for l in L) and any('? <Veronica-2 Suche>' in l for l in L), 'Menue-Links')
check(bstr(0xFC00) == b'gopher.floodgap.com', 'Titel %r' % bstr(0xFC00))
render(cpu, 'tw8a.png', 3)
# Links pruefen: Ziele im Textstrom
T = text()
for h in (b'gopher://gopher.floodgap.com/1/world', b'gopher://gopher.floodgap.com/0/about.txt',
          b'http://frogfind.com/', b'gopher://gopher.floodgap.com:7070/1/x%20y'):
    check(h in T, 'Ziel %r' % h)
typ('gopher://gopher.floodgap.com/0/about.txt'); key(0x0D)
L = lines(); print(L)
check(net.requests[-1][1] == b'/about.txt\r\n' and 'Zeile  zwei' in ' '.join(L), 'Textdatei')
typ('gopher://gopher.floodgap.com/7/v2/vs'); key(0x0D)
L = lines(); print(L)
check(w('HNFLD') >= 1, 'Suchfeld')
click(30, 61)
print('FFOC', w('FFOC'))
typ('apple iigs'); key(0x0D)
print(net.requests[-1])
check(net.requests[-1][1] == b'/v2/vs\tapple iigs\r\n', 'Suchanfrage')
check(any('Treffer: Apple IIgs' in l for l in lines()), 'Suchergebnis')
typ('gopher://gopher.floodgap.com/9/f/prog.bxy'); key(0x0D)
dk = [x for x in FILES if x.upper().endswith('DOWNLOADS:PROG.BXY')]
check(dk and FILES[dk[0]] == BIN, 'Datei per Gopher gespeichert')
print('ALLES OK' if ok else 'FEHLER')
