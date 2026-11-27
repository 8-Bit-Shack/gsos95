# Erzeugt Daten fuer das GS/OS95-Bootbild in START.GS.OS: einfarbige Flaeche
# mit "Created by ..." und Build, genau wie der untere Teil von GSOS95.SPL
# (boot95/mksplash.py); SPLASH95 setzt beim Booten das Bild darueber ein.
import re, struct, sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE); sys.path.insert(0, os.path.join(HERE, '..'))
os.chdir(HERE)
import mksplash as MS
txt=open('../assets_spr.inc').read()
def spr(name):
    m=re.search(r'^%s:\n((?:\s*\.byte .*\n)+)'%re.escape(name), txt, re.M)
    vals=[]
    for l in m.group(1).strip().split('\n'):
        vals += [int(x.strip()[1:],16) for x in l.split('.byte')[1].split(',')]
    return vals
bandc = MS.band_color()
out=[]
def bl(name,data):
    out.append(name+':')
    for i in range(0,len(data),16): out.append('  .byte '+','.join('$%02X'%b for b in data[i:i+16]))
by=spr('t_by'); bu=spr('t_build')
out.append('BANDC = $%03X' % bandc)
out.append('BAR0 = %d' % MS.BAR0)
out.append('BAR1 = %d' % MS.BAR1)
out.append('BY_X = %d' % (160 - by[0]//2)); out.append('BY_Y = %d' % MS.BY_Y)
out.append('BUILD_X = %d' % (160 - bu[0]//2)); out.append('BUILD_Y = %d' % MS.BUILD_Y)
# Pixel-Schriftzug "GS/OS 95" (Viertelgroesse, beim Zeichnen 4fach) bis SPLASH95 das Bild bringt
lg=spr('t_logo'); w,h=lg[0],lg[1]; bpr=(w+7)//8
sw,sh=w//4,h//4; sb=(sw+7)//8
small=[sw,sh]
for r in range(sh):
    row=[0]*sb
    for c in range(sw):
        if (lg[2+(r*4)*bpr+(c*4)//8]>>(7-(c*4)%8))&1: row[c//8]|=0x80>>(c%8)
    small+=row
bl('d_logo', small)
out.append('LOGO_X = %d' % (160-(sw*4)//2))
out.append('LOGO_Y = %d' % 52)
bl('d_p14', b''.join(struct.pack('<H',c) for c in MS.pal14(bandc)))
open('data.inc','w').write('\n'.join(out)+'\n')
out=[]
bl('d_by', by)
bl('d_build', bu)
bl('d_pal0', b''.join(struct.pack('<H',c) for c in MS.pal0(bandc)))
open('data2.inc','w').write('\n'.join(out)+'\n')
print('build', len(bu), 'Flaeche $%03X' % bandc)
