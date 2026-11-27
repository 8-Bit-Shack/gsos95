import sys, struct; sys.path.insert(0,'.')
from pdtool import Vol
v = Vol(open('sys/orig.hdv','rb').read())
free0 = v.free_count()
app = open('GSOS95.S16','rb').read()
tmpl = v.lookup('SYSTEM/START')[2]
# 1) GSOS95 in den Systemordner
startent = None
b, off, _ = v.lookup('SYSTEM/START')
startent = bytes(v.d[b*512+off:b*512+off+39])
v.add_file('SYSTEM', 'GSOS95', 0xB3, 0xDB03, app, template=startent)
# 2) Starter umlenken: *:System:Finder -> *:System:GSOS95 (gleiche Laenge)
st = v.lookup('SYSTEM/START')[2]
blocks = v.fork_blocks(st['st'], st['key'])[:(st['eof']+511)//512]
data = bytearray(b''.join(bytes(v.blk(x)) for x in blocks))
old, new = b'\x0f\x00*:System:Finder', b'\x0f\x00*:System:GSOS95'
hits = data.count(old)
data = data.replace(old, new)
for k, x in enumerate(blocks): v.setblk(x, data[k*512:(k+1)*512])
assert hits == 1, hits
open('sys/GSOS95_System604.hdv','wb').write(v.d)
print('frei vorher', free0, 'nachher', v.free_count())
