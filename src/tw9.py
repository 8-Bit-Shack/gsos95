# GIF-Bilder: normal, Zeilensprung, verkleinert, Transparenz, als Link aus HTML
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
from PIL import Image
import io
def mkgif(w, h, interlace=False, trans=None):
    im = Image.new('P', (w, h))
    pal = []
    for i in range(256): pal += [(i * 37) & 255, (i * 91) & 255, (i * 13) & 255]
    pal[0:3] = [255, 255, 255]; pal[3:6] = [0, 0, 0]; pal[6:9] = [255, 0, 0]; pal[9:12] = [0, 0, 255]
    im.putpalette(pal)
    px = im.load()
    for y in range(h):
        for x in range(w):
            px[x, y] = (x // 7 + y // 5) % 4 if (x + y) % 11 or w > 300 else (x * y) % 200 + 4
    b = io.BytesIO()
    kw = {'interlace': interlace}
    if trans is not None: kw['transparency'] = trans
    im.save(b, 'GIF', **kw)
    return b.getvalue(), im, pal
def expect(im, pal, s, sw, sh, trans=None):
    P = [cpu.rd16(0xE19E00 + 2 * i) for i in range(16)]
    comp = [((p >> 8) & 15, (p >> 4) & 15, p & 15) for p in P]
    m = []
    for i in range(256):
        r, g, b = pal[3 * i] >> 4, pal[3 * i + 1] >> 4, pal[3 * i + 2] >> 4
        best = min(range(16), key=lambda k: (3 * (comp[k][0] - r) ** 2 + 4 * (comp[k][1] - g) ** 2 + 2 * (comp[k][2] - b) ** 2, k))
        m.append(best)
    if trans is not None: m[trans] = 15   # WHITE
    px = im.load(); out = bytearray()
    for y in range(0, im.size[1], s):
        for x in range(0, im.size[0], s): out.append(m[px[x, y]])
    return bytes(out)
def got():
    b = w('IMBK') << 16; n = w('F_SW') * w('F_SH')
    return bytes(cpu.mem[b:b + n])
G1, im1, pal1 = mkgif(100, 60)
G2, im2, pal2 = mkgif(90, 45, interlace=True, trans=1)
G3, im3, pal3 = mkgif(640, 300)
for p, g in (('/a.gif', G1), ('/b.gif', G2), ('/c.gif', G3)):
    net.pages[('10.1.1.1', 80, p)] = b'HTTP/1.0 200 OK\r\nContent-Type: image/gif\r\n\r\n' + g
net.pages[('10.1.1.1', 80, '/img')] = b'HTTP/1.0 200 OK\r\n\r\n<p>Bild: <img src="a.gif"> <img src="x.png" alt="png"></p>'
print('GIF-Groessen', len(G1), len(G2), len(G3))
click(101, 193)
typ('frogfind.com/a.gif'); key(0x0D)
print(bstr(0xFC00), w('GIFON'), w('F_SW'), w('F_SH'), w('F_GS'))
check(w('GIFON') == 1 and (w('F_SW'), w('F_SH')) == (100, 60), 'GIF 100x60 entpackt')
check(got() == expect(im1, pal1, 1, 100, 60), 'Bildpunkte stimmen')
check(bstr(0xFC00) == b'GIF 100x60', 'Titel %r' % bstr(0xFC00))
render(cpu, 'tw9a.png', 3)
typ('frogfind.com/b.gif'); key(0x0D)
check(got() == expect(im2, pal2, 1, 90, 45, trans=1), 'Zeilensprung + Transparenz')
typ('frogfind.com/c.gif'); key(0x0D)
print(bstr(0xFC00), w('F_SW'), w('F_SH'), w('F_GS'), 'WRAPW', w('WRAPW'))
s = w('F_GS')
check(s >= 3 and got() == expect(im3, pal3, s, w('F_SW'), w('F_SH')), 'verkleinert 1:%d' % s)
render(cpu, 'tw9b.png', 3)
key(0x0A); key(0x0A)
render(cpu, 'tw9c.png', 3)
typ('frogfind.com/img'); key(0x0D)
L = lines(); print(L)
check(w('IMGN') == 1 and any('[png]' in l for l in L), 'GIF in die Seite, PNG nur Text')
print('ALLES OK' if ok else 'FEHLER')
