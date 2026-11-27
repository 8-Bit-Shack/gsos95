# Fuegt eine neue Datei in ein ProDOS-Image ein: add_file.py IMG /VOL/DIR/NAME SRC TYP AUX
import sys; sys.path.insert(0, sys.path[0])
from pdtool import Vol
img, path, src, ty, aux = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4], 16), int(sys.argv[5], 16)
v = Vol(open(img, 'rb').read())
try:
    v.lookup(path); v.delete(path)
except Exception: pass
d, name = path.rsplit('/', 1) if '/' in path else ('', path)
v.add_file(d, name, ty, aux, open(src, 'rb').read())
assert v.read(v.lookup(path)[2])[0] == open(src, 'rb').read()
with open(img, 'r+b') as f:
    f.write(v.d)
print('ok', path, 'frei:', v.free_count())
