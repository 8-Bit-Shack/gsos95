# Zeiger ueber Zubehoer: QuickDraw-Zeiger; Apfel+C/V an das Zubehoer
exec(open('t21.py').read().split("click(250,50)")[0])
def move(h,v): cpu.events.append((0,h,v,0)); settle(); settle()
CV=lambda: cpu.rd16(ADDR+lbl['CURNDA'])
move(100,150); a=(CV(), getattr(cpu,'qdcur',None), cpu.mem[cpu.rd16(ADDR+lbl['MYDP'])+0xBC])
move(250,50);  b=(CV(), getattr(cpu,'qdcur',None), cpu.mem[cpu.rd16(ADDR+lbl['MYDP'])+0xBC])
move(150,150); c=(CV(), getattr(cpu,'qdcur',None), cpu.mem[cpu.rd16(ADDR+lbl['MYDP'])+0xBC])
print('daneben', a, 'ueber Zubehoer', b, 'wieder daneben', c)
ok = a[0]==0 and b[0]==1 and b[1] is True and b[2]==0 and c[0]==0 and c[1] is False and c[2]==1
click(250,50)
cpu.events.append((3,250,50,ord('c'),0x0100)); settle(); settle()
print('SystemEdit:', getattr(cpu,'sysedits',[]))
ok = ok and getattr(cpu,'sysedits',[])==[3]
print('OK' if ok else 'FAIL Zeiger/Edit')
