# Kopiert Dateien und Ordner (auch Extended-Dateien mit Ressourcenzweig) von einem
# ProDOS-Image in ein anderes. Typ, Zusatztyp, Datum, Zugriff und Finder-Infos bleiben.
# Aufruf: copy_files.py <quelle.img> <ziel.img> <pfad> [<pfad> ...]   (Pfad ohne Volumenamen)
import sys, struct
sys.path.insert(0, sys.path[0])
from pdtool import Vol

def load(fn):
    d = open(fn, 'rb').read()
    if d[:4] == b'2IMG':
        off, ln = struct.unpack_from('<II', d, 0x18)
        return Vol(d[off:off + ln]), (d[:off], d[off + ln:])
    return Vol(d), None

def raw_entry(v, path):
    b, off, i = v.lookup(path)
    return bytes(v.d[b * 512 + off:b * 512 + off + 39]), i

def free_slot(v, dkey):
    """freier Eintrag im Verzeichnis dkey; haengt notfalls einen Block an"""
    b = dkey; first = True; last = None
    while b:
        bd = v.blk(b)
        for i in range(13):
            if first and i == 0: continue
            if not (bd[4 + i * 39] >> 4): return b, 4 + i * 39, i + 1
        first = False; last = b
        b = struct.unpack_from('<H', bd, 2)[0]
    nb = v.alloc()
    struct.pack_into('<HH', v.d, nb * 512, last, 0)
    struct.pack_into('<H', v.d, last * 512 + 2, nb)
    # Verzeichnis ist gewachsen: Blockzahl im Eintrag beim Elternverzeichnis anpassen
    hdr = v.blk(dkey)
    if dkey != 2:
        pb, pn = struct.unpack_from('<HB', hdr, 4 + 0x23)
        po = pb * 512 + 4 + (pn - 1) * 39
        used, = struct.unpack_from('<H', v.d, po + 0x13)
        struct.pack_into('<H', v.d, po + 0x13, used + 1)
        eof = int.from_bytes(v.d[po + 0x15:po + 0x18], 'little') + 512
        v.d[po + 0x15:po + 0x18] = eof.to_bytes(3, 'little')
    return nb, 4, 1

def put_entry(v, dkey, e):
    b, off, _ = free_slot(v, dkey)
    e = bytearray(e)
    struct.pack_into('<H', e, 0x25, dkey)
    v.d[b * 512 + off:b * 512 + off + 39] = e
    hb = dkey * 512
    fc = struct.unpack_from('<H', v.d, hb + 4 + 0x21)[0]
    struct.pack_into('<H', v.d, hb + 4 + 0x21, fc + 1)
    return b, off

def copy_file(src, dst, path, dkey):
    e, i = raw_entry(src, path)
    e = bytearray(e)
    if i['st'] == 5:
        data, rsrc = src.read(i)
        ext_src = src.blk(i['key'])
        ext = bytearray(ext_src)           # Finder-Infos (+8.., +264..) bleiben
        dst_, dkey_, dused = dst.write_fork(data)
        rst, rkey, rused = dst.write_fork(rsrc)
        ek = dst.alloc()
        ext[0] = dst_; struct.pack_into('<HH', ext, 1, dkey_, dused); ext[5:8] = len(data).to_bytes(3, 'little')
        ext[256] = rst; struct.pack_into('<HH', ext, 257, rkey, rused); ext[261:264] = len(rsrc).to_bytes(3, 'little')
        dst.setblk(ek, ext)
        struct.pack_into('<HH', e, 0x11, ek, dused + rused + 1)
        e[0x15:0x18] = (512).to_bytes(3, 'little')
    else:
        data, _ = src.read(i)
        st, key, used = dst.write_fork(data)
        e[0] = (st << 4) | (e[0] & 15)
        struct.pack_into('<HH', e, 0x11, key, used)
        e[0x15:0x18] = len(data).to_bytes(3, 'little')
    put_entry(dst, dkey, e)

def copy_dir(src, dst, path, dkey):
    e, i = raw_entry(src, path)
    nk = dst.alloc()
    hdr = bytearray(src.blk(i['key'])[:4 + 39])          # Kopfeintrag uebernehmen
    struct.pack_into('<HH', hdr, 0, 0, 0)
    e = bytearray(e)
    struct.pack_into('<HH', e, 0x11, nk, 1)
    e[0x15:0x18] = (512).to_bytes(3, 'little')
    b, off = put_entry(dst, dkey, e)
    struct.pack_into('<H', hdr, 4 + 0x21, 0)            # noch keine Dateien
    struct.pack_into('<HB', hdr, 4 + 0x23, b, (off - 4) // 39 + 1)
    dst.d[nk * 512:nk * 512 + len(hdr)] = hdr
    for name in [Vol.ent_info(x)['name'] for _, _, x in src.dir_entries(i['key'])]:
        copy_any(src, dst, path + '/' + name, nk)

def copy_any(src, dst, path, dkey):
    i = src.lookup(path)[2]
    if i['st'] == 0xD: copy_dir(src, dst, path, dkey)
    else: copy_file(src, dst, path, dkey)

if __name__ == '__main__':
    src, _ = load(sys.argv[1])
    dst, wrap = load(sys.argv[2])
    for p in sys.argv[3:]:
        parent, name = p.rsplit('/', 1) if '/' in p else ('', p)
        if dst.lookup(p):
            print('schon da, ersetze:', p)
            from pdtool import delete_tree
            delete_tree(dst, p)
        pk = dst.lookup(parent)[2]['key'] if parent else 2
        copy_any(src, dst, p, pk)
        print('kopiert', p)
    out = bytes(dst.d) if wrap is None else wrap[0] + bytes(dst.d) + wrap[1]
    open(sys.argv[2], 'wb').write(out)
    print('frei:', dst.free_count())
