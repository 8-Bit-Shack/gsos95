# Korrekturen fuer SCSICD.DRIVER (Brutal Deluxe, Fassung 08/2026 von der BlueSCSI-Diskette)
# Apples CD-Treiber kennt nur die Sense-Codes der Apple-Laufwerke ($B0 = keine CD, $B7 = CD wird geladen).
# SCSI-2-Laufwerke (AppleCD 300/600 u.a.) melden "keine CD" aber mit ASC $3A (MEDIUM NOT PRESENT):
#  1. unit_state: $3A ergab I/O-Fehler $27 statt "offline" $2F -> GS/OS merkt das Auswerfen nicht
#  2. unit_state/@do_switch und die Reset-Behandlung warten in einer Schleife, bis das Laufwerk
#     bereit ist oder $B0 meldet - mit $3A (Schublade offen/leer) endlos -> Rechner haengt
# Die Aenderungen bleiben in denselben Bytes (keine Adressen verschoben, keine Relocation betroffen).
import sys
PATCHES = [
  # 1. $3152: B7 -> Wechsel, B0 oder 3A -> offline, 28 -> Wechsel nochmal, sonst I/O-Fehler
  (0x3152,
   'c9b700 f01b c9b000 d003 82920029fe00 c92800 f003 826c00',
   'c9b700 f01b c9b000 f005 c93a00 d003 828d00 c92800 d06d ea'),
  # 2. $318A Schleife @do_switch: Ende bei A7/B0 und jetzt auch 3A (Maske $75: B0 und 3A -> $30)
  (0x318A, '29ff00 c9a700 f0f0 c9b000 d0db', '297500 c92500 f0f0 c93000 d0db'),
  # 3. $1A59 Schleife nach Reset/Wechsel: Ende bei B0 und jetzt auch 3A
  (0x1A59, '29ff00 c9b000 d0f0', '297500 c93000 d0f0'),
]
src, dst = sys.argv[1], sys.argv[2]
d = bytearray(open(src, 'rb').read())
for off, old, new in PATCHES:
    o = bytes.fromhex(old.replace(' ', '')); n = bytes.fromhex(new.replace(' ', ''))
    assert len(o) == len(n), hex(off)
    assert d[off:off + len(o)] == o, 'unerwartete Bytes bei $%X: %s' % (off, d[off:off + len(o)].hex())
    d[off:off + len(n)] = n
open(dst, 'wb').write(d)
print('ok', dst, len(d))
