from twbase import *
import twbase
from marisim import Marinetti
mari = Marinetti(net)
cpu = boot(); cpu.mari = mari
settle()
click(101, 193)
typ('ftp://ftp.apple2.org/'); key(0x0D); idle(3)
print(txt())
check(b'FTP geht mit der Uthernet II oder der FujiNet' in txt(), 'Hinweis bei Marinetti')
print('ALLES OK' if twbase.ok else 'FEHLER')
