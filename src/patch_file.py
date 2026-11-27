# Ersetzt den Inhalt einer Datei gleicher Groesse in einem ProDOS-Image
import sys; sys.path.insert(0, sys.path[0])
from pdtool import Vol
img, path, newf = sys.argv[1:4]
v = Vol(open(img, 'rb').read())
info = v.lookup(path)[2]
old, _ = v.read(info)
new = open(newf, 'rb').read()
assert len(new) == len(old), (len(new), len(old))
blocks = v.fork_blocks(info['st'], info['key'])[:(len(old) + 511) // 512]
for k, b in enumerate(blocks):
    chunk = new[k * 512:(k + 1) * 512]
    v.d[b * 512:b * 512 + len(chunk)] = chunk
assert v.read(info)[0] == new
with open(img, 'r+b') as f: f.write(v.d)
print('ok', path)
