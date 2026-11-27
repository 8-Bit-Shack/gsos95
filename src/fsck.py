# Prueft ein ProDOS-Image: Bloecke doppelt belegt / belegt aber als frei markiert
import sys, struct; sys.path.insert(0, sys.path[0] + '/tools'); sys.path.insert(0, sys.path[0])
from pdtool import Vol
v = Vol(open(sys.argv[1], 'rb').read())
own = {}
def claim(b, who):
    if b == 0: return
    own.setdefault(b, []).append(who)
def fork(st, key, eof, who):
    if st == 1: claim(key, who); return
    if st == 2:
        claim(key, who); ib = v.blk(key)
        for i in range(256):
            b = ib[i] | ib[256+i] << 8
            claim(b, who)
        return
    if st == 3:
        claim(key, who); mb = v.blk(key)
        for j in range(128):
            ix = mb[j] | mb[256+j] << 8
            if ix:
                claim(ix, who); ib = v.blk(ix)
                for i in range(256): claim(ib[i] | ib[256+i] << 8, who)
def dirblocks(key, who):
    b = key
    while b:
        claim(b, who); b = struct.unpack_from('<H', v.blk(b), 2)[0]
for b in range(0, 2): claim(b, 'boot')
dirblocks(2, '/')
nb = v.total // 4096 + (1 if v.total % 4096 else 0)
bm = struct.unpack_from('<H', v.blk(2), 4 + 0x23)[0]
for i in range(nb): claim(bm + i, 'bitmap')
for path, i in v.walk():
    if i['st'] == 0xD: dirblocks(i['key'], path)
    elif i['st'] == 5:
        claim(i['key'], path); ext = v.blk(i['key'])
        fork(ext[0], struct.unpack_from('<H', ext, 1)[0], 0, path)
        fork(ext[256], struct.unpack_from('<H', ext, 257)[0], 0, path + '(R)')
    elif i['st'] in (1, 2, 3): fork(i['st'], i['key'], i['eof'], path)
dup = {b: w for b, w in own.items() if len(set(w)) > 1}
free_used = [b for b in own if v.is_free(b)]
print('Bloecke belegt:', len(own), ' doppelt:', len(dup), ' belegt-aber-frei:', len(free_used))
for b, w in list(dup.items())[:15]: print('  Block', b, sorted(set(w)))
for b in free_used[:10]: print('  frei markiert, benutzt von', own[b][0], 'Block', b)
