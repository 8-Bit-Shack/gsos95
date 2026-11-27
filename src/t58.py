# Bildvorschau: 32-KB-Bild ($C1) als 40x25-Miniatur
import sys; sys.path.insert(0,'.')
import t3
# Testbild: links Palettenfarbe 1 (rot), rechts Farbe 2 (blau); Zeilen ab 100: Farbe 3 (weiss)
pix=bytearray(32000)
for y in range(200):
    for b in range(160):
        c = 3 if y>=100 else (1 if b<80 else 2)
        pix[y*160+b]=c*16+c
scb=bytes(200)+bytes(56)
pal=bytearray(512)
for c,rgb in ((0,0x000),(1,0xF00),(2,0x00F),(3,0xFFF)): pal[c*2]=rgb&0xFF; pal[c*2+1]=rgb>>8
t3.FS[':SYSTEM.DISK'].append(('BILD', 0xC1))
t3.FILES[':SYSTEM.DISK:BILD']=bytes(pix)+scb+bytes(pal)
exec(open('t53.py').read().replace("import t3\n","").split("n=0\nfor k")[0])
n=0
for k in range(20):
    key(0x15); key(0x10)
    if cpu.rd16(ADDR+lbl['PVIMG']): break
    key(0x1B)
render(cpu,'pv2.png',3)
img=cpu.rd16(ADDR+lbl['PVIMG'])
th=ADDR  # Sprite liegt in der Grafikbank
ab=cpu.rd16(cpu.rd16(ADDR+lbl['MYDP'])+0x00) if False else None
import re
pal=[cpu.rd16(ADDR+lbl['palettes']+2*i) for i in range(16)]
def near(rgb):
    return min(range(16), key=lambda k: sum(abs(((pal[k]>>s)&15)-((rgb>>s)&15)) for s in (8,4,0)))
print('Bild erkannt', img, 'erwartet Farben', near(0xF00), near(0x00F), near(0xFFF))
m=cpu.mem
px=lambda x,y: (m[0xE12000+y*160+x//2]>>(0 if x&1 else 4))&15
# Miniatur suchen: erste Zeile mit rotem Pixel im Vorschaufeld
pts=[(x,y) for y in range(100,180) for x in range(60,280) if px(x,y)==near(0xF00)]
x0,y0=pts[0]
got=(px(x0+5,y0+3), px(x0+30,y0+3), px(x0+20,y0+20))
print('Miniatur', got)
print('OK' if img and got==(near(0xF00),near(0x00F),near(0xFFF)) else 'FAIL Bildvorschau')
