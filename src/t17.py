exec(open('t16.py').read().split("settle()\ndclick")[0])
settle()
dclick(28,14)
key(0x15); key(0x04)                      # SYSTEM.DISK, Strg-D
render(cpu,'e1.png',3); key(13)
key(0x15); key(0x15); key(0x04)           # GSOS95 -> loeschen
render(cpu,'e2.png',3)
for c in b'LEER': key(c)
key(13); render(cpu,'e3.png',3)
key(13)
print('erased', getattr(cpu,'erased',None), t3.FS.get(':LEER'))
# unformatierte Diskette: Doppelklick (2. Symbol)
dclick(240-40+40-160+120+40, 52+10) if False else None
key(0x14 if False else 0x08)   # Pfeil links
key(13)
render(cpu,'e4.png',3)
key(13); key(13)
print('erased', getattr(cpu,'erased',None), t3.DEVS)
render(cpu,'e5.png',3)
