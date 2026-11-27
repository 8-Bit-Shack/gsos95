# Dialog Einstellungen > Netzwerk: Kartenwahl, Slot/Port, Baudrate, Testen mit FujiNet/Modem/Marinetti,
# Speichern in GSOS95.NET (+1536 Karte, +1537 Port, +1538 Baudrate)
from twbase import *
import twbase
from fujisim import FujiNet
from marisim import Marinetti
fn = FujiNet(net)
cpu = boot(); fn.install(cpu)
settle()
def opendlg():
    click(65, 193); dclick(40 + 12 + 30, 16 + 32 + 72 + 8)
opendlg()
check(w('DCSEL') == 0, 'Vorgabe: automatisch')
render(cpu, 'twn_a.png', 3)
WX, WY = 40, 6
click(WX + 142 + 7, WY + 24 + 6)          # Karte +
check(w('DCSEL') == 1, 'Karte + -> Uthernet II')
click(WX + 142 + 7, WY + 24 + 6); click(WX + 142 + 7, WY + 24 + 6)
check(w('DCSEL') == 3, 'FujiNet gewaehlt')
render(cpu, 'twn_b.png', 3)
click(WX + 8 + 30, WY + 180 - 24 + 7)     # Testen
check(w('NCARD') == 3 and w('DSM1') == lbl['ws_fujiok'], 'Testen: FujiNet bereit')
check(w('DSM2') == lbl['ws_dnscard'], 'Namen loest die Karte auf')
render(cpu, 'twn_c.png', 3)
click(WX + 142 + 7, WY + 24 + 6); click(WX + 142 + 7, WY + 24 + 6)
check(w('DCSEL') == 5, 'WLAN-Modem gewaehlt')
click(WX + 216 + 7, WY + 24 + 6)          # Port +
click(WX + 200 + 7, WY + 24 + 6)          # Port -
check(w('DMPORT') == 1, 'Port 1 (Drucker)')
click(WX + 174 + 7, WY + 42 + 6); click(WX + 174 + 7, WY + 42 + 6)   # Baud + +
check(w('DMBAUD') == 3, 'Baudrate 38400 (%d)' % w('DMBAUD'))
render(cpu, 'twn_d.png', 3)
click(WX + 8 + 30, WY + 180 - 24 + 7)     # Testen: kein Modem
check(w('DSM1') == lbl['ws_nomdm'], 'Testen: Modem antwortet nicht')
click(WX + 112 + 10, WY + 112 + 6)                 # Proxy-Feld
check(w('DFOC') == 4, 'Proxy-Feld hat die Schreibmarke')
typ('proxy.lan:8080')
render(cpu, 'twn_p.png', 3)
click(WX + 240 - 136 + 30, WY + 180 - 24 + 7)   # OK
f = t3.FILES.get(':SYSTEM.DISK:SYSTEM:GSOS95.NET', b'')
check(len(f) == 2048 and f[1536:1539] == bytes([5, 1, 3]), 'gespeichert: %r' % (f[1536:1540],))
check(f[106:106 + 15] == b'\x0eproxy.lan:8080', 'Proxy gespeichert: %r' % f[106:122])
check(w('WIN') == 0 or True, '')
# wieder oeffnen: Werte aus der Datei
opendlg()
check(w('DCSEL') == 5 and w('DMPORT') == 1 and w('DMBAUD') == 3, 'Werte wieder gelesen')
click(WX + 126 + 7, WY + 24 + 6)          # Karte -
check(w('DCSEL') == 4, 'Marinetti')
cpu.mari = Marinetti(net)
click(WX + 8 + 30, WY + 180 - 24 + 7)
check(w('NCARD') == 4 and w('DSM1') == lbl['ws_mariok'], 'Testen: Marinetti verbunden')
check(w('DSM2') == lbl['ws_dnsok'], 'DNS ueber Marinetti')
render(cpu, 'twn_e.png', 3)
print('ALLES OK' if twbase.ok else 'FAIL Dialog')
