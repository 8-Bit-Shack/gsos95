# Baut START.GS.OS (System 6.1) mit GS/OS95-Bootbild aus der Original-Datei
# (frueher 6.0.4: siehe patch_604.py.txt)
#  $740E JSR $7743 (Startlogo entpacken) -> JSR $7919 (Bootbild, boot95.bin)
#  $7411/$7414/$7417 Kasten, Symbol, Text -> NOP; $741A Balkenrahmen bleibt
#  Platz: $7919-$7B8A (gepackte Logo-Daten, 626 Bytes), $7686-$7794 (Aufbau, Symbol,
#  Text, Entpacker; $7686-$76AF Variablen, ab $76B0 boot95b.bin), $78BC-$7904 (boot95c.bin)
#  Ist das Bild schon da (SYSTEM:PHOENIX, Marke "95" in $E1/9DFE), wird nichts geloescht/gezeichnet.
import sys, struct, os
BASE = 0x6800
HERE = os.path.dirname(os.path.abspath(__file__))
def off(a): return a - BASE
def load(n): return open(os.path.join(HERE, n), 'rb').read()
def patch(orig):
    d = bytearray(orig)
    def chk(a, b):
        assert d[off(a):off(a)+len(b)] == bytes(b), 'unerwartet bei %X: %s' % (a, d[off(a):off(a)+len(b)].hex())
    def put(a, b): d[off(a):off(a)+len(b)] = bytes(b)
    chk(0x740E, [0x20, 0x43, 0x77, 0x20, 0x86, 0x76, 0x20, 0xDA, 0x76, 0x20, 0xFA, 0x76, 0x20, 0xE6, 0x77])
    chk(0x7503, [0x20, 0x72, 0x75])
    chk(0x77E6, [0xA9, 0x49, 0x57]); chk(0x77F7, [0xA0, 0x23, 0x00]); chk(0x780F, [0xA0, 0x23, 0x00])
    chk(0x7584, [0xA9, 0xE9, 0x57]); chk(0x7594, [0xC0, 0x24, 0x00])
    chk(0x755E, [0xAF, 0x65, 0x44, 0xE1, 0xCF, 0x0D, 0x79, 0x00])
    chk(0x7B8B - 1, [d[off(0x7B8A)]])
    # Aussehen (Battery RAM $5E): immer "Standard" wie 6.0.1, keine 6.1-Looks
    chk(0x7355, [0x48, 0xF4, 0x5E, 0x00, 0xA2, 0x03, 0x0C, 0x22, 0x00, 0x00, 0xE1, 0x68])
    put(0x7355, [0xA9, 0x60, 0x00] + [0xEA] * 9)
    put(0x7905, d[off(0x790F):off(0x7919)])         # Werte des Standard-Looks schon in der Datei
    code = load('boot95.bin'); c2 = load('boot95b.bin'); c3 = load('boot95c.bin')
    assert 0x7919 + len(code) <= 0x7B8B, len(code)
    assert 0x76B0 + len(c2) <= 0x7795, len(c2)
    assert 0x78BC + len(c3) <= 0x7905, len(c3)
    assert code[3:6] == bytes([0x20, 0x72, 0x75])
    put(0x7919, code)
    put(0x76B0, c2)
    put(0x78BC, c3)
    # Bildschirm loeschen ($73C4..): nur wenn SYSTEM:PHOENIX das Bild nicht schon gezeigt hat
    chk(0x73C4, [0xC2, 0x20, 0xAF, 0xBA, 0x78, 0x00, 0x4A])
    put(0x73C6, [0x4C, 0xBC, 0x78, 0xEA])
    put(0x740E, [0x20, 0x19, 0x79] + [0xEA] * 9)   # Bootbild statt Logo/Kasten/Symbol/Text
    put(0x7503, [0x20, 0x1C, 0x79])                 # zwei Schritte je Datei
    # Balkenmuster: Bloecke aus 6 weissen Punkten + 2 Punkten Luecke
    chk(0x7600, bytes.fromhex('7fff4ffff7fff4ffff7fff4ffff7fff4'))
    put(0x7600, bytes.fromhex('7fff5ffff7fff5ffff7fff5fffffffff'))
    # Fortschrittsbalken: Zeile 166, ab Byte 30, 101 Bytes breit
    fb = 0x2000 + 166 * 160 + 30
    put(0x77E7, struct.pack('<H', fb))
    put(0x7585, struct.pack('<H', fb + 0xA0))
    put(0x77F8, [0x63]); put(0x7810, [0x63]); put(0x7595, [0x64])
    # Kontrollwort "Bootbild noch sichtbar": unterer Balkenrahmen (schwarz = $0000)
    sent = 0x2000 + 171 * 160 + 40
    put(0x755F, struct.pack('<H', sent)); put(0x7562, [0xC9, 0x00, 0x00, 0xEA])
    return early(d)
def early(d):
    """Startbild gleich am Einsprung laden (early.bin hinter der Datei bei $9B79)"""
    d = bytearray(d)
    if len(d) > 0x9B79 - 0x6800: del d[0x9B79 - 0x6800:]
    assert len(d) == 0x9B79 - 0x6800
    e = load('early.bin'); assert 0x9B79 + len(e) <= 0x9C00
    if d[0:6] == bytes.fromhex('a900008df27c'): d[0:6] = bytes([0x20, 0x79, 0x9B, 0xEA, 0xEA, 0xEA])
    assert d[0:3] == bytes([0x20, 0x79, 0x9B])
    return bytes(d + e)
def repatch(done):
    """Neues Bootbild in eine bereits gepatchte START.GS.OS einsetzen."""
    d = bytearray(done)
    assert d[off(0x740E):off(0x7411)] == bytes([0x20, 0x19, 0x79])
    code = load('boot95.bin'); c2 = load('boot95b.bin'); c3 = load('boot95c.bin')
    assert 0x7919 + len(code) <= 0x7B8B and 0x76B0 + len(c2) <= 0x7795 and 0x78BC + len(c3) <= 0x7905
    d[off(0x7919):off(0x7919) + len(code)] = code
    d[off(0x76B0):off(0x76B0) + len(c2)] = c2
    d[off(0x78BC):off(0x78BC) + len(c3)] = c3
    return early(d)
if __name__ == '__main__' and len(sys.argv) > 1 and sys.argv[1] == '-r':
    open(sys.argv[3], 'wb').write(repatch(open(sys.argv[2], 'rb').read())); print('ok'); sys.exit()
if __name__ == '__main__':
    open(sys.argv[2], 'wb').write(patch(open(sys.argv[1], 'rb').read()))
    print('ok')
