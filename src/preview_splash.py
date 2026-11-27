import sys; sys.path.insert(0,'.')
from font import render
from PIL import Image
exec(open('assets.py').read().split("out = []")[0])  # PAL
def c12(v): return (((v>>8)&15)*17, ((v>>4)&15)*17, (v&15)*17)
img = Image.new('RGB',(320,200))
px = img.load()
for y in range(200):
    p = 1 + min(14, y*15//200); t=(p-1)/14
    bg = c12((int(t*6)<<8)|(int(2+t*9)<<4)|int(6+t*9))
    for x in range(320): px[x,y]=bg
def text(s,x,y,col,bold=False,scale=1):
    for r,row in enumerate(render(s,bold,scale)):
        for c,v in enumerate(row):
            if v: px[x+c,y+r]=c12(col)
lw=len(render("GS/OS95",True,4)[0])
text("GS/OS95",(320-lw)//2+3,63,0x000,True,4)
text("GS/OS95",(320-lw)//2,60,0xFFF,True,4)
ew=len(render("Apple IIgs Edition",True)[0])
text("Apple IIgs Edition",(320-ew)//2,98,0xFF0,True)
bw=len(render("Wird gestartet...")[0])
text("Wird gestartet...",(320-bw)//2,150,0xFFF)
# Balken
for x in range(60,260):
    for y in range(166,176):
        if x in(60,259) or y in(166,175): px[x,y]=c12(0x000)
        else: px[x,y]=c12([0x00A,0x03C,0x06E,0x09F][(x//8)%4])
img.resize((1280,960),Image.NEAREST).save('splash_preview.png')
