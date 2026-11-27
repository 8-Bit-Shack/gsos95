# v2.14: Start > Hilfe (about:keys), about:update (Versionsdatei), Blaettern im Startmenue
from twbase import *
import twbase, struct
from marisim import Marinetti
net.dns['upd.test'] = '10.1.1.9'
T = b'HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n'
net.pages[('10.1.1.9', 80, '/neu.txt')] = T + b'2.9.1\r\nGS/OS95 v9.0.0\r\nhttps://example.org/rel\r\n'
B = open('BUILD').read().strip().encode()
net.pages[('10.1.1.9', 80, '/gleich.txt')] = T + B + b'\nGS/OS95 gleich\n'
# 12 Programme im Startmenue (16 Zeilen mit den Gruppen und Arbeitsplatz)
ents = b''
for i in range(12):
    p = (':SYSTEM.DISK:PROG%02d' % i).encode()
    ents += struct.pack('<HHH', 0xB3, 0, len(p)) + p + bytes(66 - len(p))
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.PRG'] = struct.pack('<H', 12) + ents
cpu = boot(); cpu.mari = Marinetti(net)
settle()
check(w('PRGLN') == 12, '12 Programme geladen (%d)' % w('PRGLN'))
# ---- Startmenue: Programme blaettern
key(9)                                  # Tab: Startmenue
key(0x15)                               # rechts: Untermenue Programme
check(w('SUBVIS') == 14 and w('SUBTOP') == 0, 'Untermenue 14 von 16 Zeilen sichtbar (Gruppen, Arbeitsplatz, 12 Programme)')
render(cpu, 'twu_m0.png', 3)
for i in range(14): key(0x0A)           # runter bis Zeile 14
check(w('SUBH') == 14 and w('SUBTOP') == 1, 'Tastatur blaettert (SUBH %d TOP %d)' % (w('SUBH'), w('SUBTOP')))
render(cpu, 'twu_m1.png', 3)
key(0x0A)                               # letztes Programm (15)
check(w('SUBH') == 15 and w('SUBTOP') == 2, 'letzte Zeile (SUBH %d TOP %d)' % (w('SUBH'), w('SUBTOP')))
key(0x0A)                               # wieder oben
check(w('SUBH') == 0 and w('SUBTOP') == 0, 'Umlauf nach oben')
key(0x1B); key(0x1B)
# Maus am unteren Rand blaettert
key(9); key(0x15)
sy = w('SUBY'); y = sy + 2 + 13 * 12 + 4
cpu.events += [(0, 170, y, 0)]; settle()
check(w('SUBTOP') == 1, 'Maus am Rand blaettert (TOP %d)' % w('SUBTOP'))
key(0x1B); key(0x1B)
# ---- Start > Hilfe -> about:keys
key(9)
for i in range(4): key(0x0A)
key(0x0D)
check(w('WIN') == 21 and bstr(0xFC00) == b'Tastenk\x82rzel', 'Hilfe zeigt about:keys (%r)' % bstr(0xFC00))
check(b'Apfel-Pfeil hoch' in txt(), 'Text der Hilfeseite')
render(cpu, 'twu_keys.png', 3)
# ---- about:update ohne Adresse
typ('about:update'); key(0x0D)
check(b'raw.githubusercontent.com/8-Bit-Shack/gsos95' in bstr(0xF800), 'ohne eigene Adresse: GitHub (%r)' % bstr(0xF800)[:60])
# ---- Adresse setzen, neuere Version
typ('about:update=http://upd.test/neu.txt'); key(0x0D)
check(bstr(0xFC00) == b'GS/OS95-Update', 'Titel Update (%r)' % bstr(0xFC00))
t = txt()
check(b'neue Version ist da' in t and b'2.9.1' in t and b'v9.0.0' in t, 'neuere Version erkannt')
check(b'https://example.org/rel' in t, 'Link auf das Release')
render(cpu, 'twu_upd.png', 3)
f = t3.FILES.get(':SYSTEM.DISK:SYSTEM:GSOS95.NET', b'')
check(f[1600:1601 + 23] == b'\x17http://upd.test/neu.txt', 'Adresse gespeichert: %r' % f[1600:1630])
# gleiche Version
typ('about:update=http://upd.test/gleich.txt'); key(0x0D)
check(b'neueste Version' in txt(), 'gleiche Version: aktuell')
# danach normale Seite unveraendert
typ('frogfind.com'); key(0x0D)
check(bstr(0xFC00) == b'FrogFind!', 'normale Seite danach')
typ('about:update'); key(0x0D)
check(b'neueste Version' in txt(), 'gespeicherte Adresse benutzt')
typ('about:net'); key(0x0D)
check(b'Nach Updates suchen' in txt(), 'about:net verlinkt Update und Tastenkuerzel')
print('ALLES OK' if twbase.ok else 'FAIL Update/Hilfe')
