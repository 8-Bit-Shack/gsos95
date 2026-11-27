exec(open('tcd.py').read().split('# ---- Pause / weiter')[0])
print('vor', cd2.state, x3('CDST'))
key(0x20)
print('nach', cd2.state, x3('CDST'), [ (hex(c[1]), c[2][:9].hex()) for c in cd2.log[-4:]])
