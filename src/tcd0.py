# CD-Wiedergabe ohne CD-Laufwerk
from twbase import *
import twbase
X3 = 0x200000
def x3s(n):
    a = X3 + lbl[n]; return bytes(cpu.mem[a + 1:a + 1 + cpu.mem[a]]).decode('latin-1')
cpu = boot(); settle()
key(9); key(0x15)
key(0x15); key(0x0A); key(0x0D)             # Werkzeuge > CD-Wiedergabe
check(w('WIN') == 23, 'Fenster offen')
a = X3 + lbl['cds_nodrv']; p = cpu.rd16(X3 + lbl['CDL2P'])
check(p == lbl['cds_nodrv'], 'Meldung kein Laufwerk')
for k in (0x0D, 0x20, 0x15, 0x08, ord('s'), ord('e'), ord('l')): key(k)
check(w('WIN') == 23, 'Tasten ohne Laufwerk: kein Absturz')
render(cpu, 'tcd_none.png', 3)
key(0x1B)
check(w('WIN') != 23, 'Esc schliesst')
print('ALLES OK' if twbase.ok else 'FAIL')
