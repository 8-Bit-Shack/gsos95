# Ordnerbaum: Klick auf Nachbarordner und anderes Laufwerk
exec(open('t49.py').read().split("print('Pfade:'")[0])
a=ADDR+lbl['wrecs']+(cpu.rd16(ADDR+lbl['NWIN'])-1)*298
wx=cpu.rd16(a+2); wy=cpu.rd16(a+4)
def row(r): click(wx+8+24, wy+32+r*9+4)
row(6); pa=path(); render(cpu,'ts1.png',3)
a=ADDR+lbl['wrecs']+(cpu.rd16(ADDR+lbl['NWIN'])-1)*298
wx=cpu.rd16(a+2); wy=cpu.rd16(a+4)
n=cpu.rd16(ADDR+lbl['TRCNT']); print('Zeilen', n)
# letzte Zeile = anderes Laufwerk
row(n-1); pb=path(); render(cpu,'ts2.png',3)
print('Nachbar:', repr(pa), 'Laufwerk:', repr(pb))
print('OK' if pa.endswith(':ICONS') and pb==':GSOS95' else 'FAIL Nachbarordner')
