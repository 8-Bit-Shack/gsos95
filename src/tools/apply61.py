# Uebernimmt die Fehlerkorrekturen von System 6.1 in ein GS/OS95-Image - ohne die
# 6.1-Erweiterungen (Looks, Sprachen, Phoenix, Video640, Japanisch usw.).
# Aufruf: apply61.py <GSOS61.2mg> <ziel.hdv> <START.GS.OS mit Bootbild>
# Die 6.1-Werkzeuge lesen das Aussehen aus dem Battery RAM ($5E Bits 5-7, $5F Flamme);
# hier wird an jeder dieser neuen Stellen fest "Standard" (wie 6.0.1) eingesetzt:
#   48 F4 5E 00 A2 03 0C 22 00 00 E1 68 (ReadBParam $5E) -> A9 60 00 + 9 x NOP
#   dasselbe mit $5F (Phoenix-Flamme)                    -> A9 00 00 + 9 x NOP
import sys, struct
sys.path.insert(0, sys.path[0])
import copy_files as CF
from pdtool import Vol, delete_tree

FILES = ['SYSTEM/GS.OS', 'SYSTEM/GS.OS.DEV', 'SYSTEM/P8', 'SYSTEM/ERROR.MSG', 'SYSTEM/START.GS.OS', 'SYSTEM/FINDER',
         'SYSTEM/SYSTEM.SETUP/TOOL.SETUP', 'SYSTEM/SYSTEM.SETUP/TS2', 'SYSTEM/SYSTEM.SETUP/TS3',
         'SYSTEM/SYSTEM.SETUP/RESOURCE.MGR', 'SYSTEM/SYSTEM.SETUP/SYS.RESOURCES',
         'SYSTEM/FSTS/PRO.FST'] + \
        ['SYSTEM/TOOLS/TOOL%03d' % n for n in (14, 15, 16, 18, 20, 23, 27, 28, 34)] + \
        ['SYSTEM/DRIVERS/' + n for n in ('APPLEDISK3.5', 'APPLEDISK5.25', 'SCSI.MANAGER', 'SCSIHD.DRIVER',
                                         'MODEM', 'PRINTER', 'IMAGEWRITER', 'IMAGEWRITER.LQ')]
LOOK = {   # Datei: [(Offset, Parameter)] - nur die in 6.1 neuen Stellen
    'SYSTEM/SYSTEM.SETUP/TS2': [(0x2E97, 0x5E), (0x2EC3, 0x5F)],
    'SYSTEM/SYSTEM.SETUP/TS3': [(0x77DC, 0x5E), (0x7808, 0x5F)],
    'SYSTEM/SYSTEM.SETUP/TOOL.SETUP': [(0x02F3, 0x5E)],
    'SYSTEM/TOOLS/TOOL014': [(0x70F1, 0x5E)],
    'SYSTEM/TOOLS/TOOL015': [(0x3A0D, 0x5E), (0x3A77, 0x5F)],
    'SYSTEM/TOOLS/TOOL016': [(0x36A7, 0x5E)],
    'SYSTEM/TOOLS/TOOL018': [(0x1AB2, 0x5E)],
}

def poke(v, path, offset, data):
    """Bytes in der Datei (Datenzweig) ueberschreiben, Groesse bleibt"""
    i = v.lookup(path)[2]
    st, key = i['st'], i['key']
    if st == 5:
        ext = v.blk(key); st, key = ext[0], struct.unpack_from('<H', ext, 1)[0]
    blocks = v.fork_blocks(st, key)
    for k, b in enumerate(data):
        o = offset + k
        v.d[blocks[o // 512] * 512 + o % 512] = b

def main(src61, dst, sgo):
    src, _ = CF.load(src61)
    for path, sites in LOOK.items():
        data = src.read(src.lookup(path)[2])[0]
        for o, par in sites:
            old = bytes([0x48, 0xF4, par, 0x00, 0xA2, 0x03, 0x0C, 0x22, 0x00, 0x00, 0xE1, 0x68])
            assert data[o:o + 12] == old, '%s %X: %s' % (path, o, data[o:o + 12].hex())
            poke(src, path, o, [0xA9, 0x60 if par == 0x5E else 0x00, 0x00] + [0xEA] * 9)
    from debrand61 import debrand          # keine Hinweise auf "System 6.1" / Phoenix
    debrand(src)
    s = open(sgo, 'rb').read()
    i = src.lookup('SYSTEM/START.GS.OS')[2]
    assert i['eof'] == len(s), (i['eof'], len(s))
    poke(src, 'SYSTEM/START.GS.OS', 0, s)
    d, wrap = CF.load(dst)
    for p in FILES:
        parent, name = p.rsplit('/', 1)
        if d.lookup(p): delete_tree(d, p)
        CF.copy_any(src, d, p, d.lookup(parent)[2]['key'])
        print('6.1:', p)
    open(dst, 'wb').write(bytes(d.d) if wrap is None else wrap[0] + bytes(d.d) + wrap[1])
    print('frei:', d.free_count())

if __name__ == '__main__':
    main(*sys.argv[1:4])
