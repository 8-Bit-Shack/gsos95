exec(open('t16.py').read().split("settle()\ndclick")[0])
settle()
dclick(28,14)
key(0x15); key(0x15); key(13)          # unformatierte Diskette oeffnen
render(cpu,'f1.png',3)
for c in b'SPIELE': key(c)
key(13); key(13)
print('erased', getattr(cpu,'erased',None), t3.DEVS[2])
render(cpu,'f2.png',3)
