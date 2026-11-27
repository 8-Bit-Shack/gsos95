# 2.1.47: Editor (Notepad), Fensterart 28
from twbase import *
import twbase
from t3 import FILES, FS
cpu = boot(); settle()
def buf():
    p = cpu.rd16(ADDR + lbl['EDP']) | cpu.mem[ADDR + lbl['EDP'] + 2] << 16
    return bytes(cpu.mem[p:p + w('EDLEN')])
key(9); key(0x15); key(0x15); key(0x0A); key(0x0A); key(0x0A); key(0x0A); key(0x0D)   # Werkzeuge > Editor
check(w('WIN') == 28, 'Start > Programme > Werkzeuge > Editor (WIN %d)' % w('WIN'))
typ('Hallo Welt'); key(0x0D); typ('zweite Zeile')
check(buf() == b'Hallo Welt\rzweite Zeile', 'Tippen und Return: %r' % buf())
key(0x0B); check(w('EDCUR') == 10, 'Pfeil hoch (Spalte gemerkt, Zeilenende) %d' % w('EDCUR'))
key(0x08); key(0x08); key(0x7F); check(buf().startswith(b'Hallo Wlt\r'), 'Delete links: %r' % buf()[:12])
key(0x0A); check(w('EDCUR') == 10 + 7, 'Pfeil runter in Spalte 7: %d' % w('EDCUR'))
key(0x08, 0x0100); check(w('EDCUR') == 10, 'Apfel-links: Zeilenanfang')
key(0x15, 0x0800); check(w('EDCUR') == 17, 'Wahl-rechts: naechstes Wort %d' % w('EDCUR'))
render(cpu, 'ted_1.png', 3)
# langer Absatz: Umbruch an Leerzeichen
key(0x15, 0x0100); key(0x0D)
typ('Der Apple IIgs kam 1986 auf den Markt und hat einen 65C816 Prozessor mit 2,8 MHz Takt und Ensoniq Sound.')
check(w('EDCOLS') > 30, 'Spalten %d' % w('EDCOLS'))
render(cpu, 'ted_2.png', 3)
# Umlaut von der Tastatur (Mac Roman ae = $8A)
key(0x8A); check(buf()[-1] == 0x8A, 'Umlaut eingefuegt')
# Speichern: Name abfragen
key(ord('s'), 0x0100); check(w('EDPR') == 1, 'Apfel-S fragt nach dem Namen')
for k in range(6): key(0x7F)
typ('*:NOTIZ'); key(0x0D)
print([k for k in FILES if 'NOTIZ' in k])
f = [k for k in FILES if k.endswith(':NOTIZ')]
check(f and FILES[f[0]] == buf(), 'gespeichert als NOTIZ')
check(w('EDMOD') == 0 and w('EDNEW') == 0, 'nicht mehr geaendert')
render(cpu, 'ted_3.png', 3)
# Esc mit Aenderung fragt nach
typ('x'); key(0x1B); check(w('WIN') == 28 and w('EDESC') == 1, 'Esc: Rueckfrage')
key(0x1B); check(w('WIN') != 28, 'zweites Esc schliesst')
# Textdatei im Explorer oeffnen (LF und CR LF werden CR, Latin-1-Umlaute angezeigt)
FILES[':SYSTEM.DISK:README'] = b'Liesmich\r\nZeile zwei\nGr\xfc\xdfe aus K\xf6ln\n'
dclick(28, 14); dclick(176, 44); dclick(248, 71)
check(w('WIN') == 28 and w('EDNEW') == 0, 'Doppelklick auf README oeffnet den Editor')
check(buf() == b'Liesmich\rZeile zwei\rGr\xfc\xdfe aus K\xf6ln\r', 'Zeilenenden: %r' % buf())
render(cpu, 'ted_4.png', 3)
key(0x0A, 0x0100); typ('Neu'); key(ord('s'), 0x0100)
check(FILES[':SYSTEM.DISK:README'].endswith(b'\rNeu'), 'unter altem Namen gespeichert: %r' % FILES[':SYSTEM.DISK:README'][-8:])
key(0x1B)
# viele Zeilen: Rollen, Seite hoch, Mausklick
FILES[':SYSTEM.DISK:DATEI.01'] = b''.join(b'Zeile %d\r' % i for i in range(1, 41))
dclick(28, 14); dclick(176, 44); dclick(248, 98)
check(w('WIN') == 28 and w('EDLEN') > 300, 'DATEI.01 geladen (%d)' % w('EDLEN'))
rows = w('EDROWS')
for i in range(rows + 3): key(0x0A)
check(w('EDTOP') > 0, 'nach unten gerollt (TOP %d, Zeilen %d)' % (w('EDTOP'), rows))
key(0x0A, 0x0100); key(0x0A, 0x0100); key(0x0A, 0x0100)
check(w('EDCUR') == w('EDLEN'), 'Apfel-runter bis zum Ende')
render(cpu, 'ted_5.png', 3)
key(0x0B, 0x0100); key(0x0B, 0x0100); key(0x0B, 0x0100); key(0x0B, 0x0100)
check(w('EDTOP') == 0 and w('EDCUR') < 10, 'Apfel-hoch bis zum Anfang')
click(8 + 5 + 6 * 3, 12 + 18 + 10 * 2 + 3)
check(w('EDCUR') == 16 + 3, 'Mausklick setzt den Cursor (Zeile 3, Spalte 4): %d' % w('EDCUR'))
key(0x1B)
print('ALLES OK' if twbase.ok else 'FEHLER')
