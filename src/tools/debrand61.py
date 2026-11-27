# Texte in den Systemdateien von 6.1, die an "System 6.1"/Phoenix erinnern, gleich lang ersetzen
# (Finder-Meldungen, Finder-Tipp zum Phoenix-Bild, Tool.Setup-Meldungen zu DOS 3.3/MFS).
# debrand61.py IMG  (oder aus apply61.py: debrand(vol))
import sys, os; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import struct
def poke(v, path, offset, data, fork=0):
    """Bytes in Datenzweig (0) oder Ressourcenzweig (1) ueberschreiben, Groesse bleibt"""
    i = v.lookup(path)[2]
    st, key = i['st'], i['key']
    if st == 5:
        ext = v.blk(key); o = 256 * fork; st, key = ext[o], struct.unpack_from('<H', ext, o + 1)[0]
    blocks = v.fork_blocks(st, key)
    for k, b in enumerate(data):
        a = offset + k
        v.d[blocks[a // 512] * 512 + a % 512] = b
R = {
 'SYSTEM/FINDER': [
  (b'Use the\rSystem 6.1 Installer to update this disk.', b'Use a GS/OS95\rstartup disk instead.'),
  (b'Reinstall System 6.1.', b'Reinstall GS/OS95.'),
  (b'Reinstall System 6.1 with the\rInstaller to replace it.', b'Reinstall GS/OS95 to replace it.'),
  (b'the Phoenix screen', b'the startup screen')],
 'SYSTEM/SYSTEM.SETUP/TOOL.SETUP': [
  (b'Translator in System 6.1 works', b'Translator in this GS/OS works'),
  (b'System 6.1 cannot read MFS', b'This GS/OS cannot read MFS')],
}
def debrand(v):
    for path, pairs in R.items():
        forks = v.read(v.lookup(path)[2])
        for old, new in pairs:
            new = new.ljust(len(old), b' ')
            assert len(new) == len(old)
            hit = [(f, d.find(old)) for f, d in enumerate(forks) if d and d.find(old) >= 0]
            if not hit:
                assert any(d and d.find(new) >= 0 for d in forks), (path, old); continue   # schon ersetzt
            f, o = hit[0]
            poke(v, path, o, new, f)
            print('ersetzt:', path, old[:30])
if __name__ == '__main__':
    from pdtool import Vol
    v = Vol(open(sys.argv[1], 'rb').read())
    debrand(v)
    open(sys.argv[1], 'r+b').write(bytes(v.d))
