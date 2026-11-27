# Ersetzt SYSTEM/GSOS95 in einem ProDOS-Image durch eine neue Version
import sys; sys.path.insert(0, sys.path[0])
from pdtool import Vol
img, app = sys.argv[1], sys.argv[2]
v = Vol(open(img, 'rb').read())
b, off, _ = v.lookup('SYSTEM/GSOS95')
tmpl = bytes(v.d[b*512+off:b*512+off+39])
v.delete('SYSTEM/GSOS95')
v.add_file('SYSTEM', 'GSOS95', 0xB3, 0xDB03, open(app, 'rb').read(), template=tmpl)
d, _ = v.read(v.lookup('SYSTEM/GSOS95')[2])
assert d == open(app, 'rb').read()
with open(img, 'r+b') as f:
    f.write(v.d)
print('ok, frei:', v.free_count())
