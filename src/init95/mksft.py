# Baut GSOS95.INIT: eine OMF-Ladedatei (Code, verschiebbar), Typ $B6
import struct, subprocess, sys, os
CC = __import__('os').environ.get('CC65_BIN', '/home/claude/ext/cc65/bin').rstrip('/') + '/'
os.chdir(os.path.dirname(os.path.abspath(__file__)))
subprocess.check_call([CC + 'ca65', '-g', '-l', 'sftest.lst', 'sftest.s', '-o', 'sftest.o'])
CFG = """MEMORY { RAM: start=$%X, size=$10000, file=%%O; }
SEGMENTS { CODE: load=RAM, type=rw; }
"""
def link(base, tag):
    open('t_%s.cfg' % tag, 'w').write(CFG % base)
    extra = ['-Ln', 'sftest.lbl'] if base == 0 else []
    subprocess.check_call([CC + 'ld65', '-C', 't_%s.cfg' % tag, 'sftest.o', '-o', 't_%s.bin' % tag] + extra)
    return open('t_%s.bin' % tag, 'rb').read()
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
name = b'SFTEST'
segname = bytes([len(name)]) + name
dispname = 44
dispdata = dispname + 10 + len(segname)
total = dispdata + len(body)
hdr = struct.pack('<IIIBBBBI', total, 0, len(A), 0, 0, 4, 2, 0x10000)
hdr += struct.pack('<HHIIBBHIHH', 0x0000, 0, 0, 0, 0, 0, 1, 0, dispname, dispdata)
seg1 = hdr + b'          ' + segname + bytes(body)
name2 = b'STACK'; sn2 = bytes([len(name2)]) + name2
body2 = bytes([0xF1]) + struct.pack('<I', 0x2000) + b'\x00'
dd2 = dispname + 10 + len(sn2)
tot2 = dd2 + len(body2)
h2 = struct.pack('<IIIBBBBI', tot2, 0, 0x2000, 0, 0, 4, 2, 0x10000)
h2 += struct.pack('<HHIIBBHIHH', 0x0012, 0, 0, 0, 0, 0, 2, 0, dispname, dd2)
open('SFTEST', 'wb').write(seg1 + h2 + b'          ' + sn2 + body2)
print('SFTEST %d Bytes, Code %d, Reloc %d' % (total, len(A), len(rel)))
