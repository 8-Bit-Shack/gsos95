# Ordnerbaum: lange Namen gekuerzt, Detailansicht im breiten Fenster
import sys; sys.path.insert(0,'.')
exec(open('t49.py').read().split("dpb=cpu.rd16")[0])
settle()
click(53,193); key(0x15); key(13)
render(cpu,'tb1.png',3)
# maximieren (Knopf in der Mitte rechts), dann Details
a=ADDR+lbl['wrecs']+(cpu.rd16(ADDR+lbl['NWIN'])-1)*298
wx=cpu.rd16(a+2); wy=cpu.rd16(a+4); ww=cpu.rd16(a+22)
click(wx+ww-31+6, wy+5+5)
tw0=cpu.rd16(ADDR+lbl['TW'])
click(103,26); render(cpu,'tb_menu.png',3)
click(110,72); render(cpu,'tb2.png',3)
print('Details, TW =', cpu.rd16(ADDR+lbl['TW']), 'VIEWM', cpu.rd16(ADDR+lbl['VIEWM']))
# Klick auf die zweite Zeile der Liste (Eintrag markieren)
click(110, 52); render(cpu,'tb3.png',3)
print('ok')
print('OK' if cpu.rd16(ADDR+lbl['TW'])==72 and cpu.rd16(ADDR+lbl['VIEWM'])==1 else 'FAIL Details+Baum')
