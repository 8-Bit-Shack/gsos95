# Baut eine GS/OS-Ladedatei (OMF v2, ein statisches Codesegment mit Relokationen)
import struct, re, sys

def relocs(b0, b1, delta=0x1000):
    assert len(b0) == len(b1)
    out = []
    i = 0
    while i < len(b0):
        if b0[i] != b1[i]:
            j = i - 1           # Delta $1000 aendert nur das High-Byte
            v0 = b0[j] | (b0[j+1] << 8); v1 = b1[j] | (b1[j+1] << 8)
            assert b0[j] == b1[j] and (v1 - v0) & 0xFFFF == delta, "unerwartete Differenz bei %X" % i
            assert not out or out[-1][0] + 2 <= j
            out.append((j, v0))
            i += 1
        else:
            i += 1
    return out

def build(b0, b1, bss, name=b'GSOS95'):
    rl = relocs(b0, b1)
    body = bytearray()
    body += bytes([0xF2]) + struct.pack('<I', len(b0)) + b0          # LCONST
    for off, val in rl:                                               # RELOC
        body += bytes([0xF5, 2, 0]) + struct.pack('<HH', off, val)   # cRELOC
    body += b'\x00'                                                   # END
    loadname = b'          '
    segname = bytes([len(name)]) + name
    dispname = 44
    dispdata = dispname + 10 + len(segname)
    total = dispdata + len(body)
    hdr = struct.pack('<IIIBBBBI', total, bss, len(b0) + bss, 0, 0, 4, 2, 0x10000)
    hdr += struct.pack('<HHIIBBHIHH', 0x0000, 0, 0, 0, 0, 0, 1, 0, dispname, dispdata)
    assert len(hdr) == 44
    seg = hdr + loadname + segname + body
    assert len(seg) == total
    return bytes(seg), rl

if __name__ == '__main__':
    b0 = open('app0.bin', 'rb').read(); b1 = open('app1.bin', 'rb').read()
    m = open('app.map').read()
    bss = int(re.search(r'^BSS\s+\S+\s+\S+\s+([0-9A-F]+)', m, re.M).group(1), 16)
    seg, rl = build(b0, b1, bss)
    open('GSOS95.S16', 'wb').write(seg)
    print('OMF', len(seg), 'Bytes,', len(rl), 'Relokationen, BSS', bss)
