# Ersetzt eine Datei in einem ProDOS-Image (Typ, Zusatztyp und Datum bleiben)
import sys; sys.path.insert(0, sys.path[0])
from pdtool import Vol
img, path, src = sys.argv[1], sys.argv[2], sys.argv[3]
v = Vol(open(img, 'rb').read())
b, off, info = v.lookup(path)
tmpl = bytes(v.d[b*512+off:b*512+off+39])
ftype, aux = tmpl[16], tmpl[31] | tmpl[32] << 8
v.delete(path)
d, name = path.rsplit('/', 1)
v.add_file(d, name, ftype, aux, open(src, 'rb').read(), template=tmpl)
b2, off2, _ = v.lookup(path)
v.d[b2*512+off2+28:b2*512+off2+30] = tmpl[28:30]   # Version/Mindestversion wie vorher
assert v.read(v.lookup(path)[2])[0] == open(src, 'rb').read()
with open(img, 'r+b') as f:
    f.write(v.d)
print('ok', path, 'Typ $%02X/$%04X' % (ftype, aux), 'frei:', v.free_count())
