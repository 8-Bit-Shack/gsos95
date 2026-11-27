# Tasten: frisch geoeffnetes Zubehoer bekommt sie; Klick auf GS/OS95 holt sie zurueck
exec(open('t21.py').read().split("click(250,50)")[0])
key(ord('a')); ev0=list(getattr(cpu,'sysevents',[]))
foc0=cpu.rd16(ADDR+lbl['NDAFOC'])
click(300,150)                     # Klick auf den Schreibtisch -> Tasten fuer uns
foc1=cpu.rd16(ADDR+lbl['NDAFOC'])
key(ord('b')); ev1=list(getattr(cpu,'sysevents',[]))
click(250,50)                      # Klick ins NDA-Fenster -> Fokus
key(ord('7')); key(ord('+')); ev2=list(getattr(cpu,'sysevents',[]))
key(0x1B); closed=getattr(cpu,'ndaclosed',0)
print('neu', ev0, foc0, 'nach Desktop', ev1, foc1, 'nach Klick', ev2, 'Esc schliesst', closed)
ok = ev0==[(3,97)] and foc0==1 and foc1==0 and ev1==ev0 and ev2==ev0+[(3,ord('7')),(3,ord('+'))] and closed==1
print('OK' if ok else 'FAIL Tasten')
