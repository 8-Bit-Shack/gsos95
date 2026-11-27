# v2.16: CD-Titelnamen per CDDB (HTTP) in der CD-Wiedergabe, about:cddb
from twbase import *
import twbase, cdsim
from marisim import Marinetti
X3 = 0x200000
cd2 = cdsim.CD('scsi2')
cdsim.install(t3, {4: cd2})
SRV = '10.1.1.30'
net.dns['freedb.dbpoweramp.com'] = SRV
net.dns['cddb.example.org'] = '10.1.1.31'

def discid(cd):
    secs = [st // 75 for st, c in cd.tracks]
    n = sum(sum(int(d) for d in str(x)) for x in secs)
    t = cd.leadout // 75 - cd.tracks[0][0] // 75
    return '%08x' % ((n % 255) << 24 | t << 8 | len(cd.tracks))
QUERY = []
READ = (b'210 rock %s CD database entry follows (until terminating `.\')\r\n# xmcd\r\n'
        b'DISCID=%s\r\nDTITLE=Die \xc4rzte / Le Frisur\r\nDYEAR=1996\r\n'
        b'TTITLE0=Mein Baby war beim Fris\xf6r\r\nTTITLE1=Mot\xc3\xb6rhead-Fan\r\n'
        b'TTITLE2=Ein ziemlich langer Titel, der nicht in die Zeile passt\r\nTTITLE3=Vier\r\n'
        b'TTITLE4=Daten\r\nEXTD=\r\nPLAYORDER=\r\n.\r\n')
class Pages(dict):
    def get(s, k, d=None):
        if k[2].startswith('/~cddb/cddb.cgi?'):
            def f(req):
                q = k[2]
                QUERY.append((k[0], q, req))
                H = b'HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n'
                i = discid(cdx[0])
                if 'cmd=cddb+query+' in q:
                    if i != '%08x' % 0x1234: pass
                    if cdx[0].known: return H + ('200 rock %s Die Aerzte / Le Frisur\r\n' % i).encode()
                    return H + b'202 No match for disc ID.\r\n'
                if 'cmd=cddb+read+rock+' + i in q:
                    return H + READ % (i.encode(), i.encode())
                return H + b'401 not found\r\n'
            return f
        return dict.get(s, k, d)
net.pages = Pages(net.pages)
net.pages[(SRV, 80, '/')] = b''
net.pages[('10.1.1.31', 80, '/')] = b''
cdx = [cd2]; cd2.known = True
def x3(n): return cpu.rd16(X3 + lbl[n])
def x3s(n, off=0):
    a = X3 + lbl[n] + off; return bytes(cpu.mem[a + 1:a + 1 + cpu.mem[a]])
def jsl(name, bank=X3):
    PBR, PC, DBR, S = cpu.PBR, cpu.PC, cpu.DBR, cpu.S
    ret = ADDR + lbl['code_end']
    cpu.push(ADDR >> 16); cpu.push16((ret - 1) & 0xFFFF)
    cpu.PBR = (bank + lbl[name]) >> 16; cpu.PC = (bank + lbl[name]) & 0xFFFF; cpu.DBR = ADDR >> 16
    run(cpu, 200_000_000, stop_at=ret)
    cpu.PBR, cpu.PC, cpu.DBR, cpu.S = PBR, PC, DBR, S
def tick(): jsl('xf_cd_tick'); settle()

cpu = boot(); cpu.mari = Marinetti(net)
settle()
key(9); key(0x15)
key(0x15); key(0x0A); key(0x0D)             # Werkzeuge > CD-Wiedergabe
check(w('WIN') == 23 and x3('CDOK') == 1, 'CD-Wiedergabe offen, CD gelesen')
check(x3('CDNF') == 0 and not QUERY, 'Netzwerk noch aus: keine automatische Suche')
render(cpu, 'tcddb_0.png', 3)
# ---- Taste I: Titelnamen holen
key(ord('i'))
check(len(QUERY) == 2, 'zwei Anfragen (query, read): %d' % len(QUERY))
q = QUERY[0][1]
exp = 'cmd=cddb+query+%s+5+%s+%d' % (discid(cd2), '+'.join(str(st) for st, c in cd2.tracks), cd2.leadout // 75)
check(QUERY[0][0] == SRV and q.startswith('/~cddb/cddb.cgi?' + exp), 'query: %s' % q)
check('&hello=gsos95+apple2gs+GSOS95+2.18&proto=5' in q, 'hello Standard')
check(b'Host: freedb.dbpoweramp.com\r\n' in QUERY[0][2], 'Host-Kopf')
check(QUERY[1][1].startswith('/~cddb/cddb.cgi?cmd=cddb+read+rock+' + discid(cd2) + '&hello='), 'read: %s' % QUERY[1][1])
check(x3('CDNF') == 2, 'Titelnamen da (CDNF %d)' % x3('CDNF'))
check(x3s('CDART') == b'Die \x83rzte / Le Frisur', 'Kuenstler/Album Latin-1: %r' % x3s('CDART'))
check(x3s('CDTT', 32) == b'Mein Baby war beim Fris\x81r', 'Titel 1: %r' % x3s('CDTT', 32))
check(x3s('CDTT', 64) == b'Mot\x81rhead-Fan', 'Titel 2 UTF-8: %r' % x3s('CDTT', 64))
check(len(x3s('CDTT', 96)) == 31, 'langer Titel auf 31 Zeichen gekuerzt')
check(x3s('CDL1') == b'1. Mein Baby war beim Fris\x81r'[:26], 'Anzeige Titel: %r' % x3s('CDL1'))
render(cpu, 'tcddb_1.png', 3)
key(0x15)
check(x3s('CDL1').startswith(b'2. Mot'), 'Titel 2 in der Anzeige')
# ---- about:cddb: Server und E-Mail einstellen
QUERY.clear()
key(9); key(0x15)
key(0x0A); key(0x15); key(0x0D)             # Internet > Internet
check(w('WIN') == 21, 'Browser offen')
typ('about:cddbmail=marc@example.org'); key(0x0D)
typ('about:cddb=cddb.example.org'); key(0x0D)
nb = ADDR + lbl['NPREF']
check(bytes(cpu.mem[nb + 1764:nb + 1764 + 17]) == b'\x10marc@example.org', 'E-Mail gespeichert')
check(bytes(cpu.mem[nb + 1700:nb + 1700 + 17]) == b'\x10cddb.example.org', 'Server gespeichert')
check(b'cddb.example.org' in txt() and b'marc@example.org' in txt(), 'about:cddb zeigt die Einstellungen')
f = t3.FILES.get(':SYSTEM.DISK:SYSTEM:GSOS95.NET', b'')
check(f[1764:1781] == b'\x10marc@example.org', 'in GSOS95.NET')
render(cpu, 'tcddb_page.png', 3)
typ('about:cddb='); key(0x0D)
check(cpu.mem[nb + 1700] == 0 and b'(Standard)' in txt(), 'Server zurueck auf Standard')
# ---- CD-Wiedergabe nach vorn, neue CD: automatisch (Netzwerk laeuft), keine Treffer
key(9); key(0x15)
key(0x15); key(0x0A); key(0x0D)             # Werkzeuge > CD-Wiedergabe
check(w('WIN') == 23, 'CD-Wiedergabe wieder vorn')
cd2.disc = False; tick()
cd3 = cdsim.CD('scsi2'); cd3.tracks = cd3.tracks[:3]; cd3.leadout = cdsim.fr(9, 0, 0); cd3.known = False
cd2.tracks = cd3.tracks; cd2.leadout = cd3.leadout; cd2.known = False
cd2.disc = True; tick(); tick()
check(x3('CDOK') == 1 and x3('CDT1') == 3, 'neue CD (3 Titel)')
check(len(QUERY) == 1 and 'hello=marc+example.org+GSOS95' in QUERY[0][1], 'automatisch gesucht, hello mit E-Mail')
check(x3('CDNF') == 3 and x3s('CDTT', 32) == b'', 'keine Treffer: Namen geloescht (CDNF %d)' % x3('CDNF'))
render(cpu, 'tcddb_2.png', 3)
# ---- alte CD wieder: wieder gesucht (andere Disc-ID)
print('ALLES OK' if twbase.ok else 'FAIL CD-Titelnamen')
