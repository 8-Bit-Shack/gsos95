# Titelleiste grau, solange ein Zubehoer den Fokus hat
src=open('t21.py').read().split("click(12,193)                      # Start")[0]
exec(src)
click(53,193)                       # Arbeitsplatz
a0=cpu.rd16(ADDR+lbl['ACTW'])
click(12,193); click(60,96); click(80,16+40+3+12+4)   # Zubehoer oeffnen
for i in range(3): settle()
a1=cpu.rd16(ADDR+lbl['ACTW']); f1=cpu.rd16(ADDR+lbl['NDAFOC'])
render(cpu,'t60a.png',3)
m=cpu.mem
px=lambda x,y: (m[0xE12000+y*160+x//2]>>(0 if x&1 else 4))&15
n0=cpu.rd16(ADDR+lbl['NWIN']); r0=ADDR+lbl['wrecs']+(n0-1)*298
t1=px(cpu.rd16(r0+2)+60, cpu.rd16(r0+4)+8)
n=cpu.rd16(ADDR+lbl['NWIN']); a=ADDR+lbl['wrecs']+(n-1)*298
wx=cpu.rd16(a+2); wy=cpu.rd16(a+4)
click(wx+60, wy+80)                 # in das eigene Fenster klicken
for i in range(3): settle()
a2=cpu.rd16(ADDR+lbl['ACTW']); f2=cpu.rd16(ADDR+lbl['NDAFOC'])
render(cpu,'t60b.png',3)
t2=px(wx+60, wy+8)
print('ACTW', a0, a1, a2, 'Fokus', f1, f2, 'Titelfarbe', t1, t2)
NAVY=lbl.get('NAVY')
rf=cpu.desklog.count(0x390E)
print('RefreshDesktop', rf)
print('OK' if f1==1 and f2==0 and t1==1 and t2!=1 else 'FAIL Fokuswechsel')
# Schreibtisch-Zeichenroutine wie der Window Manager aufrufen (RefreshDesktop)
def desk_cb():
    ret=ML-1
    cpu.push(ret>>16); cpu.push16(ret&0xFFFF)
    cpu.PBR=(ADDR+lbl['desk_cb'])>>16; cpu.PC=(ADDR+lbl['desk_cb'])&0xFFFF
    run(cpu, 30_000_000, stop_at=ML)
cpu.events += [(1,250,50,0),(2,250,50,0)]; settle(); settle()   # Klick ins Zubehoer
f3=cpu.rd16(ADDR+lbl['NDAFOC']); desk_cb()
a3=cpu.rd16(ADDR+lbl['ACTW']); render(cpu,'t60c.png',3)
print('nach Klick ins Zubehoer: Fokus', f3, 'ACTW', a3)
print('OK' if f3==1 and a3!=a0 else 'FAIL graue Titelleiste')
