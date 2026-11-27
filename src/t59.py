# Ordnerbaum mit zwei Fenstern: vorderes Fenster behaelt Laufwerke/Nachbarn
exec(open('t49.py').read().split("p1=path()")[0])
def drv():
    n=cpu.rd16(ADDR+lbl['TRCNT']); b=ADDR+lbl['treebuf']
    return sum(1 for i in range(n) if cpu.mem[b+i*18+1]&4), n
before=drv()
click(53,193)                       # zweites Arbeitsplatz-Fenster
key(0x15); key(13)                  # Laufwerk oeffnen
d1=drv()
key(9); key(0x1B)                   # Startmenue auf/zu: alles neu zeichnen
for i in range(3): settle()
d2=drv(); render(cpu,'t59.png',3)
print('vorher',before,'nach Oeffnen',d1,'nach Neuzeichnen',d2, path())
print('OK' if d2==d1 and d1[0]>=2 else 'FAIL Baum zwei Fenster')
