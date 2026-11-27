# Kleiner ProDOS-Volume-Generator (nur Wurzelverzeichnis, Seedling/Sapling)
import struct

def make_volume(volname, total, files, boot=b''):
    img = bytearray(total * 512)
    img[0:len(boot)] = boot
    date = (26 << 9) | (11 << 5) | 15; time = (12 << 8)
    dt = struct.pack('<HH', date, time)
    pname = lambda s: s.upper().encode().ljust(15, b'\0')
    nbitmap = (total + 4095) // 4096
    nxt = 6 + nbitmap
    entries = []
    for fn, ftype, aux, data in files:
        n = max(1, (len(data) + 511) // 512)
        assert n <= 256
        first = nxt
        for i in range(n):
            chunk = data[i * 512:(i + 1) * 512]
            img[(first + i) * 512:(first + i) * 512 + len(chunk)] = chunk
        if n == 1:
            key, st, used = first, 1, 1; nxt = first + 1
        else:
            key = first + n
            for i in range(n):
                img[key * 512 + i] = (first + i) & 0xFF
                img[key * 512 + 256 + i] = (first + i) >> 8
            st, used = 2, n + 1; nxt = key + 1
        e = bytes([(st << 4) | len(fn)]) + pname(fn) + bytes([ftype]) + struct.pack('<HH', key, used) \
            + struct.pack('<I', len(data))[:3] + dt + bytes([0, 0, 0xE3]) + struct.pack('<H', aux) + dt + struct.pack('<H', 2)
        entries.append(e)
    for b in range(2, 6):
        struct.pack_into('<HH', img, b * 512, b - 1 if b > 2 else 0, b + 1 if b < 5 else 0)
    hdr = bytes([0xF0 | len(volname)]) + pname(volname) + bytes(8) + dt + bytes([0, 0, 0xC3, 0x27, 0x0D]) \
        + struct.pack('<HHH', len(files), 6, total)
    img[2 * 512 + 4:2 * 512 + 4 + len(hdr)] = hdr
    assert len(entries) <= 12
    for i, e in enumerate(entries):
        off = 2 * 512 + 4 + 39 * (i + 1)
        img[off:off + 39] = e
    for b in range(nxt, total):
        img[6 * 512 + b // 8] |= 0x80 >> (b % 8)
    return img, 6 + nbitmap

def two_img(img):
    h = b'2IMG' + b'CLDE' + struct.pack('<HHIIIIIIIII', 64, 1, 1, 0, len(img) // 512, 64, len(img), 0, 0, 0, 0) + bytes(16)
    return h + img
