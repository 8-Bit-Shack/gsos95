# Uhrzeit per NTP: Zeitzonen und Sommerzeit
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
import datetime as dt
def at(y, mo, d, h, mi, s=0): return int(dt.datetime(y, mo, d, h, mi, s, tzinfo=dt.timezone.utc).timestamp())
def exp(t, off):
    x = dt.datetime.fromtimestamp(t, dt.timezone.utc) + dt.timedelta(hours=off)
    return ((x.month - 1) << 8 | (x.day - 1), (x.year - 1900) << 8 | x.hour, x.minute << 8 | x.second)
def go(tz, t, off, name):
    if tz:
        click(101, 193); typ('about:tz=' + tz); key(0x0D); settle()
    net.ntp = t; cpu.timeset = None
    click(101, 193); typ('frogfind.com'); key(0x0D); settle()
    got = cpu.timeset
    check(got == exp(t, off), '%s: %r erwartet %r' % (name, got and tuple(hex(v) for v in got), tuple(hex(v) for v in exp(t, off))))
go(None, at(2026, 11, 19, 15, 45, 30), 1, 'Voreinstellung Winter')
click(101, 193); typ('frogfind.com/page2'); key(0x0D); settle()
check(len([x for x in net.log if x[0] == 'ntp']) == 1, 'nur einmal je Sitzung')
go('+1eu', at(2026, 7, 1, 10, 0), 2, 'EU Sommer')
go('+1eu', at(2026, 3, 29, 0, 59), 1, 'EU vor Umstellung Maerz')
go('+1eu', at(2026, 3, 29, 1, 0), 2, 'EU nach Umstellung Maerz')
go('+1eu', at(2026, 10, 25, 0, 59), 2, 'EU vor Umstellung Oktober')
go('+1eu', at(2026, 10, 25, 1, 0), 1, 'EU nach Umstellung Oktober')
go('-5us', at(2026, 3, 8, 6, 59), -5, 'US vor Maerz')
go('-5us', at(2026, 3, 8, 7, 0), -4, 'US nach Maerz')
go('-5us', at(2026, 11, 1, 5, 59), -4, 'US vor November')
go('-5us', at(2026, 11, 1, 7, 0), -5, 'US nach November')
go('-5us', at(2026, 1, 1, 3, 0), -5, 'US Neujahr (Vortag)')
go('+0', at(2028, 2, 29, 23, 59, 59), 0, 'Schalttag ohne Sommerzeit')
go('+9', at(2030, 12, 31, 20, 0), 9, 'Jahreswechsel +9')
go('+14', at(2026, 6, 1, 12, 0), 14, '+14')
go('-12', at(2026, 6, 1, 12, 0), -12, '-12')
click(101, 193); typ('about:fav'); key(0x0D); settle()
L = lines(); print([l for l in L if 'UTC' in l])
check(any('UTC-12' in l for l in L), 'Anzeige der Zeitzone')
click(101, 193); typ('about:tz=off'); key(0x0D); settle()
n = len([x for x in net.log if x[0] == 'ntp'])
click(101, 193); typ('frogfind.com'); key(0x0D); settle()
check(len([x for x in net.log if x[0] == 'ntp']) == n, 'aus: keine Anfrage')
L = lines(); 
click(101, 193); typ('about:fav'); key(0x0D); settle()
check(any('Zeitzone' in l and 'aus' in l for l in lines()), 'Anzeige aus')
print('ALLES OK' if ok else 'FEHLER')
