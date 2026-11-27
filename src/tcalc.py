# v2.18: Rechner (Fensterart 24): Grundrechenarten, %, Wurzel, x2, 1/x, n!, Pi, Speicher, Anzeige
from twbase import *
import twbase
from decimal import Decimal, getcontext
X3 = 0x200000
def x3(n): return cpu.rd16(X3 + lbl[n])
def disp():
    a = X3 + lbl['CLDSP']; return bytes(cpu.mem[a + 1:a + 1 + cpu.mem[a]]).decode('latin-1')
cpu = boot(); settle()
key(9); key(0x15); key(0x15); key(0x0D)   # Programme > Werkzeuge > Rechner
check(w('WIN') == 24, 'Start > Programme > Rechner (WIN %d)' % w('WIN'))
render(cpu, 'tcalc_0.png', 3)
def calc(keys):
    for c in keys:
        key(0x1B if c == '\x1b' else ord(c) if isinstance(c, str) else c)
    return disp()
def fmt(v):          # 12 Stellen wie der Rechner
    if v == 0: return '0'
    getcontext().prec = 12
    d = +Decimal(v); s = '-' if d < 0 else ''; d = abs(d)
    t = d.as_tuple(); dig = ''.join(map(str, t.digits)).rstrip('0') or '0'
    e = len(t.digits) + t.exponent          # Wert = 0,dig * 10^e
    if 1 <= e <= 12: ip = dig[:e].ljust(e, '0'); fp = dig[e:]; return s + ip + (',' + fp if fp else '')
    if -5 <= e <= 0: return s + '0,' + '0' * (-e) + dig
    return s + dig[0] + (',' + dig[1:] if len(dig) > 1 else '') + 'e' + ('+' if e - 1 >= 0 else '-') + str(abs(e - 1))
getcontext().prec = 40
T = [('12+7=', '19'), ('c1/3=', fmt(Decimal(1) / 3)), ('c2w', fmt(Decimal(2).sqrt())), ('c10!', '3628800'),
     ('c70!', fmt(Decimal(__import__('math').factorial(70)))),
     ('c5/0=', 'Fehler'), ('c200+10%=', '220'), ('c0,1+0,2=', '0,3'), ('c3-5=', '-2'), ('cp', fmt(Decimal('3.141592653589793'))),
     ('c1/8=', '0,125'), ('c1/3000=', fmt(Decimal(1) / 3000)), ('c12q', '144'), ('c4r', '0,25'),
     ('c123\x7f', '12'), ('c5n+3=', '-2'), ('c99999999*99999999=', fmt(Decimal(99999999) ** 2)),
     ('c99999999*99999999*99999999=', fmt(Decimal(99999999) ** 3)), ('c5+=', '10'), ('c2*3+4*5=', '50'),
     ('c1000000*1000000=', '1e+12'), ('c123456789012*10=', '1,23456789012e+12'), ('c123456789012=', '123456789012'), ('c1000000*10000000=', '1e+13'), ('c0,5w', fmt(Decimal('0.5').sqrt())),
     ('c3,5!', 'Fehler'), ('c1e', '1'), ('c7-7=', '0'), ('c0,000001/3=', fmt(Decimal('0.000001') / 3)),
     ('c1-0,9999999999999=', fmt(Decimal(1) - Decimal('0.9999999999999')))]
for k, exp in T:
    k = k.replace('c', '\x1b', 1) if k.startswith('c') else k
    got = calc(k)
    check(got == exp, '%r -> %r (erwartet %r)' % (k.replace('\x1b', 'C'), got, exp))
# Speicher
calc('\x1b')
click_x = None
def btn(i):
    r, c = divmod(i, 6)
    rx = 10 + c * 37 + 17 + 40; ry = 52 + r * 22 + 9 + 16 - 8
    click(rx, ry)
calc('5'); btn(18); calc('3'); btn(18); btn(6 * 2)          # M+ M+ MR
check(disp() == '8' and x3('CLHM') == 1, 'Speicher: 5 M+ 3 M+ MR = 8 (%r)' % disp())
render(cpu, 'tcalc_1.png', 3)
btn(6); check(x3('CLHM') == 0, 'MC loescht den Speicher')
btn(1 * 6 + 1); btn(1 * 6 + 4); btn(2 * 6 + 3); btn(4 * 6 + 5)   # 7 / 6 =
check(disp() == fmt(Decimal(7) / 6), 'Klicks: 7 / 6 = %r' % disp())
calc('\x1b')
check(w('WIN') == 24 and disp() == '0', 'Esc loescht')
key(0x1B)
check(w('WIN') != 24, 'zweimal Esc schliesst das Fenster')
print('ALLES OK' if twbase.ok else 'FAIL Rechner')
