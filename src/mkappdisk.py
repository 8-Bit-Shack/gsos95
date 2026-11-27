import sys; sys.path.insert(0,'.')
from prodos import make_volume, two_img
app = open('GSOS95.S16','rb').read()
txt = ("GS/OS95 fuer GS/OS\r\r"
 "GSOS95 ist ein GS/OS-Programm (Typ S16). Starten: im Finder doppelklicken.\r"
 "Der Arbeitsplatz zeigt alle Laufwerke. Programme (S16/SYS/EXE) startet\r"
 "man mit zwei Klicks; nach dem Beenden geht es zurueck zu GS/OS95.\r"
 "Start > Beenden kehrt zum Finder zurueck.\r").encode()
img, _ = make_volume('GSOS95', 1600, [('GSOS95', 0xB3, 0x0000, app), ('LIESMICH', 0x04, 0x0000, txt)])
open('GSOS95_App_800K.po','wb').write(img)
open('GSOS95_App_800K.2mg','wb').write(two_img(img))
print('ok', len(img))
