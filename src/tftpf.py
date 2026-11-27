# 2.1.44: ftp:// ueber die FujiNet (N:FTP://, Verzeichnis Modus 6, Datei Modus 4)
from twbase import *
import twbase
from t3 import FILES
from fujisim import FujiNet
fn = FujiNet(net)
net.dns['ftp.apple2.org'] = '10.1.1.2'
BIG = bytes((i * 5 + 1) & 0xFF for i in range(40000))
net.ftp = {('ftp.apple2.org', '/', 6): b'pub/\r\nREADME.TXT\r\n',
           ('ftp.apple2.org', '/pub/', 6): b'disk.2mg\r\n',
           ('ftp.apple2.org', '/pub/disk.2mg', 4): BIG}
cpu = boot(); fn.install(cpu)
settle()
click(101, 193)
typ('ftp://ftp.apple2.org/'); key(0x0D); idle(4)
L = [l for l in txt().split(b'\n')]; print(txt()[:300], net.log[-3:])
check(('fopen', 6, 'N:FTP://ftp.apple2.org/') in net.log and fn.aux2 == 0x80, 'N:FTP:// als Verzeichnis geoeffnet')
check(b'[pub]' in txt() and b'README.TXT' in txt(), 'Liste gezeigt')
render(cpu, 'tftpf_1.png', 3)
click(26, 61); idle(4)
print(txt()[:200], net.log[-2:])
check(('fopen', 6, 'N:FTP://ftp.apple2.org/pub/') in net.log, 'Unterverzeichnis')
click(26, 71); idle(8)
print(net.log[-2:])
check(('fopen', 4, 'N:FTP://ftp.apple2.org/pub/disk.2mg') in net.log, 'Datei mit Modus 4')
dk = [x for x in FILES if 'DOWNLOADS' in x.upper()]
check(any(FILES[x] == BIG for x in dk), 'Datei gespeichert %r' % dk)
print('ALLES OK' if twbase.ok else 'FEHLER')
