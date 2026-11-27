# ProDOS-Volume lesen/aendern (Unterverzeichnisse, Seedling/Sapling/Tree, Extended-Dateien)
import struct

class Vol:
    def __init__(s, data):
        s.d = bytearray(data)
        h = s.blk(2)
        s.name = h[5:5 + (h[4] & 15)].decode()
        s.bitmap_ptr, s.total = struct.unpack_from('<HH', h, 0x27)

    def blk(s, n): return s.d[n * 512:(n + 1) * 512]
    def setblk(s, n, b):
        b = bytes(b).ljust(512, b'\0'); s.d[n * 512:(n + 1) * 512] = b

    # ---- Bitmap ----
    def is_free(s, b):
        return bool(s.d[s.bitmap_ptr * 512 + b // 8] & (0x80 >> (b % 8)))
    def set_used(s, b, used=True):
        o = s.bitmap_ptr * 512 + b // 8
        if used: s.d[o] &= ~(0x80 >> (b % 8)) & 0xFF
        else: s.d[o] |= 0x80 >> (b % 8)
    def free_count(s): return sum(1 for b in range(s.total) if s.is_free(b))
    def alloc(s):
        for b in range(s.total):
            if s.is_free(b):
                s.set_used(b); s.setblk(b, b''); return b
        raise Exception('Volume voll')

    # ---- Verzeichnisse ----
    def dir_entries(s, key):
        """liefert (block, offset, entrybytes) fuer aktive Eintraege"""
        out = []
        b = key; first = True
        while b:
            bd = s.blk(b)
            nxt = struct.unpack_from('<H', bd, 2)[0]
            for i in range(13):
                off = 4 + i * 39
                if first and i == 0: continue
                e = bd[off:off + 39]
                if e[0] >> 4: out.append((b, off, e))
            first = False; b = nxt
        return out

    @staticmethod
    def ent_info(e):
        st = e[0] >> 4; n = e[0] & 15
        name = e[1:1 + n].decode()
        key, used = struct.unpack_from('<HH', e, 0x11)
        eof = int.from_bytes(e[0x15:0x18], 'little')
        return dict(st=st, name=name, type=e[0x10], key=key, used=used, eof=eof,
                    aux=struct.unpack_from('<H', e, 0x1F)[0], access=e[0x1E])

    def lookup(s, path):
        key = 2; info = None
        for part in [p for p in path.split('/') if p]:
            found = None
            for b, off, e in s.dir_entries(key):
                i = s.ent_info(e)
                if i['name'].upper() == part.upper():
                    found = (b, off, i); break
            if not found: return None
            info = found
            key = found[2]['key']
        return info

    def walk(s, key=2, prefix=''):
        for b, off, e in s.dir_entries(key):
            i = s.ent_info(e)
            yield prefix + i['name'], i
            if i['st'] == 0xD:
                yield from s.walk(i['key'], prefix + i['name'] + '/')

    # ---- Dateiinhalt ----
    def fork_blocks(s, st, key):
        if st == 1: return [key]
        if st == 2:
            ib = s.blk(key); return [ib[i] | ib[256 + i] << 8 for i in range(256)]
        if st == 3:
            mb = s.blk(key); out = []
            for j in range(128):
                ix = mb[j] | mb[256 + j] << 8
                if ix:
                    ib = s.blk(ix); out += [ib[i] | ib[256 + i] << 8 for i in range(256)]
                else: out += [0] * 256
            return out
        raise Exception('st %d' % st)
    def read_fork(s, st, key, eof):
        out = bytearray()
        for b in s.fork_blocks(st, key):
            if len(out) >= eof: break
            out += s.blk(b) if b else bytes(512)
        return bytes(out[:eof])
    def read(s, info):
        if info['st'] == 5:
            ext = s.blk(info['key'])
            dst, dkey = ext[0], struct.unpack_from('<H', ext, 1)[0]
            deof = int.from_bytes(ext[5:8], 'little')
            rst, rkey = ext[256], struct.unpack_from('<H', ext, 257)[0]
            reof = int.from_bytes(ext[261:264], 'little')
            return s.read_fork(dst, dkey, deof), s.read_fork(rst, rkey, reof)
        return s.read_fork(info['st'], info['key'], info['eof']), None

    # ---- Datei schreiben (nur Datenfork, Seedling/Sapling/Tree) ----
    def write_fork(s, data):
        n = max(1, (len(data) + 511) // 512)
        blocks = []
        for i in range(n):
            b = s.alloc(); s.setblk(b, data[i * 512:(i + 1) * 512]); blocks.append(b)
        if n == 1: return 1, blocks[0], 1
        if n <= 256:
            ix = s.alloc(); ib = bytearray(512)
            for i, b in enumerate(blocks): ib[i] = b & 0xFF; ib[256 + i] = b >> 8
            s.setblk(ix, ib); return 2, ix, n + 1
        if n <= 128 * 256:               # Tree: Master-Index -> Index-Bloecke
            mi = s.alloc(); mb = bytearray(512); used = n + 1
            for j in range(0, n, 256):
                ix = s.alloc(); ib = bytearray(512); used += 1
                for i, b in enumerate(blocks[j:j + 256]): ib[i] = b & 0xFF; ib[256 + i] = b >> 8
                s.setblk(ix, ib)
                mb[j // 256] = ix & 0xFF; mb[256 + j // 256] = ix >> 8
            s.setblk(mi, mb); return 3, mi, used
        raise Exception('zu gross')

    def free_fork(s, st, key):
        for b in s.fork_blocks(st, key):
            if b: s.set_used(b, False)
        if st in (2, 3): s.set_used(key, False)
        if st == 3:
            mb = s.blk(key)
            for j in range(128):
                ix = mb[j] | mb[256 + j] << 8
                if ix: s.set_used(ix, False)

    def rename(s, path, newname):
        b, off, i = s.lookup(path)
        e = s.d[b * 512 + off: b * 512 + off + 39]
        e[0] = (e[0] & 0xF0) | len(newname)
        e[1:16] = newname.upper().encode().ljust(15, b'\0')
        s.d[b * 512 + off: b * 512 + off + 39] = e

    def add_file(s, dirpath, name, ftype, aux, data, template=None):
        dkey = s.lookup(dirpath)[2]['key'] if dirpath else 2
        st, key, used = s.write_fork(data)
        # freien Eintrag suchen
        b = dkey; first = True; slot = None
        while b and slot is None:
            bd = s.blk(b)
            for i in range(13):
                if first and i == 0: continue
                if not (bd[4 + i * 39] >> 4): slot = (b, 4 + i * 39); break
            first = False
            if slot is None: b = struct.unpack_from('<H', bd, 2)[0]
        assert slot, 'Verzeichnis voll'
        dt = template[0x18:0x1C] if template else bytes(4)
        e = bytes([(st << 4) | len(name)]) + name.upper().encode().ljust(15, b'\0') + bytes([ftype]) \
            + struct.pack('<HH', key, used) + struct.pack('<I', len(data))[:3] + dt + bytes([0, 0, 0xE3]) \
            + struct.pack('<H', aux) + dt + struct.pack('<H', dkey)
        s.d[slot[0] * 512 + slot[1]: slot[0] * 512 + slot[1] + 39] = e
        # Dateizaehler im Verzeichnis-Header erhoehen
        hb = dkey * 512
        fc = struct.unpack_from('<H', s.d, hb + 4 + 0x21)[0]
        struct.pack_into('<H', s.d, hb + 4 + 0x21, fc + 1)

    def delete(s, path):
        b, off, i = s.lookup(path)
        if i['st'] == 5:
            ext = s.blk(i['key'])
            s.free_fork(ext[0], struct.unpack_from('<H', ext, 1)[0])
            s.free_fork(ext[256], struct.unpack_from('<H', ext, 257)[0])
            s.set_used(i['key'], False)
        else:
            s.free_fork(i['st'], i['key'])
        s.d[b * 512 + off] = 0
        dk = struct.unpack_from('<H', s.d, b * 512 + off + 0x25)[0]
        hb = dk * 512
        fc = struct.unpack_from('<H', s.d, hb + 4 + 0x21)[0]
        struct.pack_into('<H', s.d, hb + 4 + 0x21, fc - 1)


def _free_dir_chain(v, key):
    b = key
    while b:
        nxt = struct.unpack_from('<H', v.d, b * 512 + 2)[0]
        v.set_used(b, False)
        b = nxt

def delete_tree(v, path):
    """Loescht Datei oder Ordner rekursiv"""
    b, off, i = v.lookup(path)
    if i['st'] == 0xD:
        for _, _, e in list(v.dir_entries(i['key'])):
            delete_tree(v, path + '/' + Vol.ent_info(e)['name'])
        _free_dir_chain(v, i['key'])
        v.d[b * 512 + off] = 0
        dk = struct.unpack_from('<H', v.d, b * 512 + off + 0x25)[0]
        fc = struct.unpack_from('<H', v.d, dk * 512 + 4 + 0x21)[0]
        struct.pack_into('<H', v.d, dk * 512 + 4 + 0x21, fc - 1)
    else:
        v.delete(path)

def set_volname(v, name):
    o = 2 * 512 + 4
    v.d[o] = 0xF0 | len(name)
    v.d[o + 1:o + 16] = name.upper().encode().ljust(15, b'\0')
