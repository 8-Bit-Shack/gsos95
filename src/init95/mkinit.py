# Baut GSOS95.INIT: eine OMF-Ladedatei (Code, verschiebbar), Typ $B6
import struct, subprocess, sys, os
CC = __import__('os').environ.get('CC65_BIN', '/home/claude/ext/cc65/bin').rstrip('/') + '/'
os.chdir(os.path.dirname(os.path.abspath(__file__)))
subprocess.check_call([CC + 'ca65', '-g', '-l', 'init95.lst', 'init95.s', '-o', 'init95.o'])
CFG = """MEMORY { RAM: start=$%X, size=$10000, file=%%O; }
SEGMENTS { CODE: load=RAM, type=rw; }
"""
def link(base, tag):
    open('i_%s.cfg' % tag, 'w').write(CFG % base)
    extra = ['-Ln', 'init95.lbl'] if base == 0 else []
    subprocess.check_call([CC + 'ld65', '-C', 'i_%s.cfg' % tag, 'init95.o', '-o', 'i_%s.bin' % tag] + extra)
    return open('i_%s.bin' % tag, 'rb').read()
A = link(0, 'A'); B = link(0x100, 'B')
rel = []
i = 0
while i < len(A):
    if A[i] != B[i]:
        j = i - 1
        v0 = A[j] | A[j+1] << 8; v1 = B[j] | B[j+1] << 8
        assert A[j] == B[j] and (v1 - v0) & 0xFFFF == 0x100, 'Differenz bei %X' % i
        rel.append((j, v0)); i = j + 2; continue
    i += 1
body = bytearray(bytes([0xF2]) + struct.pack('<I', len(A)) + A)
for off, val in rel:
    body += bytes([0xF5, 2, 0]) + struct.pack('<HH', off, val)
body += b'\x00'
name = b'GSOS95INIT'
segname = bytes([len(name)]) + name
dispname = 44
dispdata = dispname + 10 + len(segname)
total = dispdata + len(body)
hdr = struct.pack('<IIIBBBBI', total, 0, len(A), 0, 0, 4, 2, 0x10000)
hdr += struct.pack('<HHIIBBHIHH', 0x1000, 0, 0, 0, 0, 0, 1, 0, dispname, dispdata)
open('GSOS95.INIT', 'wb').write(hdr + b'          ' + segname + bytes(body))
print('GSOS95.INIT %d Bytes, Code %d, Reloc %d' % (total, len(A), len(rel)))
