import struct, sys, subprocess
CC = __import__('os').environ.get('CC65_BIN', '/home/claude/ext/cc65/bin').rstrip('/') + '/'
prog = open('gs95.bin','rb').read()
nblk = (len(prog)+511)//512
assert 0x2000 + nblk*512 <= 0xB000
TOTAL = 65535
FIRST = 6 + (TOTAL+4095)//4096
subprocess.check_call([CC+'ca65','--cpu','65C02','-D','FIRST=%d'%FIRST,'-D','COUNT=%d'%nblk,'boot.s','-o','boot.o'])
subprocess.check_call([CC+'ld65','-C','boot.cfg','boot.o','-o','boot.bin'])
boot = open('boot.bin','rb').read(); assert len(boot) <= 512
img = bytearray(TOTAL*512)
def blk(n): return n*512
img[0:len(boot)] = boot
# Datum 15.11.2026 12:00
date = (26<<9)|(11<<5)|15; time = (12<<8)|0
dt = struct.pack('<HH', date, time)
def pname(s): return s.upper().encode().ljust(15, b'\0')
nbitmap = (TOTAL+4095)//4096
BITMAP = 6
data_first = BITMAP + nbitmap
assert data_first == FIRST
FIRST_ACT = data_first
# Programmdatei: Datenbloecke + Indexblock
used = []
def alloc_file(data, first):
    n = max(1,(len(data)+511)//512)
    for i in range(n):
        chunk = data[i*512:(i+1)*512]
        img[blk(first+i):blk(first+i)+len(chunk)] = chunk
    if n == 1:
        return first, 1, 1, first+1
    idx = first+n
    for i in range(n):
        img[blk(idx)+i] = (first+i)&0xFF
        img[blk(idx)+256+i] = (first+i)>>8
    return idx, 2, n+1, idx+1
prog_pad = prog
key1, st1, used1, nxt = alloc_file(prog, FIRST_ACT)
liesmich = ("GS/OS95 - Desktop-Mockup fuer den Apple IIgs\r\r"
  "Dieses Volume bootet direkt in eine Oberflaeche im Stil von 1995.\r"
  "Maus: bewegen/klicken. Pfeiltasten bewegen den Zeiger,\r"
  "Leertaste/Return klickt, Esc schliesst Fenster.\r"
  "Icons zweimal anklicken zum Oeffnen.\r").encode()
key2, st2, used2, nxt = alloc_file(liesmich, nxt)
last_used = nxt-1
# Volume-Verzeichnis Bloecke 2-5
for b in range(2,6):
    prev = b-1 if b>2 else 0; nx = b+1 if b<5 else 0
    struct.pack_into('<HH', img, blk(b), prev, nx)
name = b'GSOS95'
hdr = bytes([0xF0|len(name)]) + pname('GSOS95') + bytes(8) + dt + bytes([0,0,0xC3,0x27,0x0D]) + struct.pack('<HHH', 2, BITMAP, TOTAL)
img[blk(2)+4:blk(2)+4+len(hdr)] = hdr
def entry(fn, st, ftype, key, used, eof, aux):
    e = bytes([(st<<4)|len(fn)]) + pname(fn) + bytes([ftype]) + struct.pack('<HH', key, used) + struct.pack('<I', eof)[:3] + dt + bytes([0,0,0xE3]) + struct.pack('<H', aux) + dt + struct.pack('<H', 2)
    assert len(e) == 39; return e
e1 = entry('GSOS95.SYSTEM', st1, 0xFF, key1, used1, len(prog), 0x2000)
e2 = entry('LIESMICH', st2, 0x04, key2, used2, len(liesmich), 0)
off = blk(2)+4+39
img[off:off+39] = e1; img[off+39:off+78] = e2
# Bitmap
for b in range(TOTAL):
    if b > last_used:
        img[blk(BITMAP) + b//8] |= 0x80 >> (b%8)
open('GSOS95.po','wb').write(img)
# 2IMG
h = b'2IMG' + b'CLDE' + struct.pack('<HHIIIIIIIII', 64, 1, 1, 0, TOTAL, 64, len(img), 0,0,0,0) + bytes(16)
assert len(h) == 64
open('GSOS95.2mg','wb').write(h+img)
print("prog blocks", nblk, "first", FIRST_ACT, "last used", last_used)
