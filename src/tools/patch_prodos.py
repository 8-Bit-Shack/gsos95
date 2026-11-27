# PRODOS (System 6.1) fuer GS/OS95: *:System:Boot95 (das Startbild) immer starten.
# Das Original fragt vorher das Battery RAM ($5F) und die Tastatur ab und laesst das Bild
# dann weg.  $2509: BRA $2528 (hinter die Abfragen). Dateiname: SYSTEM:BOOT95.
import sys
B = 0x2000
d = bytearray(open(sys.argv[1], 'rb').read())
o = 0x2509 - B
old = bytes.fromhex('48f45f00a2030c220000e168b00929d000c99000d00160ad00c0298000d0f7')
assert d[o:o + len(old)] == old, d[o:o + len(old)].hex()
d[o:o + 2] = bytes([0x80, 0x1D])
assert o + 2 + 0x1D == 0x2528 - B
p = 0x268F - B                          # Pfad der Startbild-Datei (16 Bytes Platz)
assert d[p:p + 16] == b'\x0e\x00SYSTEM:PHOENIX'
d[p:p + 16] = b'\x0d\x00SYSTEM:BOOT95\x00'
open(sys.argv[2], 'wb').write(d)
print('ok', sys.argv[2])
