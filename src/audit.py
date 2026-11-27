import sys, struct; sys.path.insert(0,'.')
from pdtool import Vol
def audit(path):
    v = Vol(open(path,'rb').read())
    used = {}
    def mark(b, who):
        if b == 0: return
        if b in used: raise Exception('Block %d doppelt: %s / %s' % (b, used[b], who))
        used[b] = who
    for b in range(0, 6): mark(b, 'sys')
    for b in range(v.bitmap_ptr, v.bitmap_ptr + (v.total+4095)//4096):
        if b not in used: mark(b, 'bitmap')
    def dirblocks(key, who):
        b = key
        while b:
            if b not in (2,3,4,5): mark(b, who)
            b = struct.unpack_from('<H', v.blk(b), 2)[0]
    def fork(st, key, eof, who):
        if st == 1: mark(key, who); return
        n = (eof + 511)//512
        if st in (2,3): mark(key, who)
        if st == 3:
            mb = v.blk(key)
            for j in range(128):
                ix = mb[j] | mb[256+j]<<8
                if ix: mark(ix, who)
        for b in v.fork_blocks(st, key): 
            if b: mark(b, who)
    def walk(key, pre):
        for b, off, e in v.dir_entries(key):
            i = v.ent_info(e); p = pre + i['name']
            if i['st'] == 0xD:
                dirblocks(i['key'], p); walk(i['key'], p + '/')
            elif i['st'] == 5:
                mark(i['key'], p); ext = v.blk(i['key'])
                fork(ext[0], struct.unpack_from('<H', ext, 1)[0], int.from_bytes(ext[5:8],'little'), p+'#d')
                fork(ext[256], struct.unpack_from('<H', ext, 257)[0], int.from_bytes(ext[261:264],'little'), p+'#r')
            else:
                fork(i['st'], i['key'], i['eof'], p)
    walk(2, '')
    bad_free = [b for b in used if v.is_free(b)]
    leaked = [b for b in range(v.total) if not v.is_free(b) and b not in used]
    return v, bad_free, leaked
for p in ['sys/orig.hdv', 'sys/GSOS95_System604.hdv']:
    v, bf, lk = audit(p)
    print(p, 'belegt-aber-frei-markiert:', len(bf), 'unbenutzt-belegt:', len(lk))
v = Vol(open('sys/GSOS95_System604.hdv','rb').read())
i = v.lookup('SYSTEM/GSOS95')[2]; d,_ = v.read(i)
print('GSOS95', hex(i['type']), hex(i['aux']), i['eof'], 'identisch' if d == open('GSOS95.S16','rb').read() else 'FEHLER')
d,_ = v.read(v.lookup('SYSTEM/START')[2]); print('START enthaelt:', d[d.find(b'*:'):d.find(b'*:')+15])
o = open('sys/orig.hdv','rb').read(); n = v.d
diff = [b for b in range(len(o)//512) if o[b*512:(b+1)*512] != n[b*512:(b+1)*512]]
print('geaenderte Bloecke:', len(diff), diff[:8], '...')
# Dateizaehler im SYSTEM-Verzeichnis
k = v.lookup('SYSTEM')[2]['key']; fc = struct.unpack_from('<H', v.d, k*512+4+0x21)[0]
print('Dateizaehler SYSTEM:', fc, 'tatsaechlich:', len(v.dir_entries(k)))
