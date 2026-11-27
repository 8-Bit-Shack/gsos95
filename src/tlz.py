# v2.18: Lesezeichen mit Ordnern, Verlauf, Suchmaschine, Wetter (Uthernet II nachgebildet)
from twbase import *
import twbase
from t3 import FILES
from w5100sim import W5100
chip = W5100(net)
oldrd, oldwr = t3.AppCPU.io_rd, t3.AppCPU.io_wr
def io_rd(s, r):
    if 0xB4 <= r <= 0xB7: return chip.io_rd(r - 0xB4)
    return oldrd(s, r)
def io_wr(s, r, v):
    if 0xB4 <= r <= 0xB7: chip.io_wr(r - 0xB4, v); return
    oldwr(s, r, v & 0xFF)
t3.AppCPU.io_rd, t3.AppCPU.io_wr = io_rd, io_wr
net.pages[('10.1.1.2', 80, '/a.html')] = H + b'<title>Seite A</title><p>A</p>'
net.pages[('10.1.1.2', 80, '/b.html')] = H + b'<title>Seite B</title><p>B</p>'
net.dns['wiby.me'] = '10.1.1.40'
net.pages[('10.1.1.40', 80, '/?q=apple+ii')] = H + b'<title>wiby</title><p>Treffer</p>'
net.dns['wttr.in'] = '10.1.1.41'
net.pages[('10.1.1.41', 80, '/Berlin?0T&lang=de')] = b'HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nWetterbericht: Berlin\n\n  Leichter Regen\n  +7 C\n'
def go(u): typ(u); key(0x0D); idle(2)
cpu = boot(); settle()
click(101, 193)
go('test.local/a.html')
check(bstr(0xFC00) == b'Seite A', 'Seite A')
key(0x04)                                          # Strg-D
check(':SYSTEM.DISK:SYSTEM:GSOS95.LZ' in FILES, 'Strg-D: Lesezeichen gespeichert')
go('test.local/b.html')
go('about:verlauf')
t = txt()
check(b'Verlauf' in t and b'Seite A' in t and b'Seite B' in t, 'Verlauf: A und B: %r' % t[:120])
go('about:lz')
t = txt()
check(b'Lesezeichen' in t and b'Favoriten' in t and b'Seite A' in t, 'Lesezeichen-Seite mit Ordner Favoriten')
check(b'Zuletzt besucht' in t and b'Seite B' in t, 'Formular: zuletzt besuchte Seite B')
render(cpu, 'tlz_0.png', 3)
go('about:lzadd?o=Retro+Rechner')
t = txt()
check(b'Retro Rechner' in t and t.index(b'Retro Rechner') < t.rindex(b'b.html'), 'in neuen Ordner "Retro Rechner" gemerkt: %r' % t[:300])
lz = FILES[':SYSTEM.DISK:SYSTEM:GSOS95.LZ']
check(lz[0] >= 2, 'Datei: %d Eintraege' % lz[0])
go('about:lzdel=1')
t = txt()
check(b'a.html' not in t.split(b'Verlauf')[0] and b'b.html' in t, 'Eintrag 1 geloescht: %r' % t[:260])
render(cpu, 'tlz_1.png', 3)
go('about:lzs=2')
check(w('NPREF') is not None and cpu.mem[ADDR + lbl['NPREF'] + 1960] == 2, 'Suchmaschine wiby.me gewaehlt')
go('apple ii')
check(bstr(0xFC00) == b'wiby', 'Suchbegriff geht an wiby.me: %r' % bstr(0xFC00))
go('about:lzs=1')
go('about:wetter')
check(b'Ort' in txt(), 'Wetter: ohne Ort ein Formular')
go('about:wetter?ort=Berlin')
check(b'Leichter Regen' in txt() and cpu.mem[ADDR + lbl['NPREF'] + 1961] == 6, 'Wetter fuer Berlin von wttr.in: %r' % txt()[:80])
render(cpu, 'tlz_2.png', 3)
go('about:wetter')
check(b'Leichter Regen' in txt(), 'Ort bleibt gespeichert')
print('ALLES OK' if twbase.ok else 'FAIL')
