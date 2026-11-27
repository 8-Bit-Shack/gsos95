# Teilt das System-Image: Spiele auf eigene Platte "GAMES", System behaelt alles andere
import sys, struct; sys.path.insert(0, sys.path[0])
from pdtool import Vol, delete_tree, set_volname
src = sys.argv[1]
data = open(src, 'rb').read()
sysv = Vol(data); games = Vol(data)
root = [Vol.ent_info(e)['name'] for _, _, e in sysv.dir_entries(2)]
for n in root:
    if n.upper() != 'SHAREWARE.GAMES': delete_tree(games, n)
delete_tree(sysv, 'SHAREWARE.GAMES')
set_volname(games, 'GAMES')
# Namen in Gross/Kleinschreibung (GS/OS-Bits) fuer GAMES zuruecksetzen
struct.pack_into('<H', games.d, 2 * 512 + 4 + 0x1C, 0)
open(sys.argv[2], 'wb').write(sysv.d)
open(sys.argv[3], 'wb').write(games.d)
print('System frei:', sysv.free_count() // 2, 'KB  Games frei:', games.free_count() // 2, 'KB')
