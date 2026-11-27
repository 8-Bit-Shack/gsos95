# v2.18: Dateiuebertragung im Terminal - ZMODEM (automatisch, CRC-16 und CRC-32), XMODEM/YMODEM
# empfangen (Apfel-D), XMODEM senden (Apfel-U); Telnet-255 verdoppelt
from twbase import *
import twbase, binascii
from marisim import Marinetti
from t3 import FILES, FS
net.dns['bbs.example'] = '10.1.1.20'

def crc16(d):
    c = 0
    for b in d:
        c ^= b << 8
        for i in range(8): c = ((c << 1) ^ 0x1021) & 0xFFFF if c & 0x8000 else (c << 1) & 0xFFFF
    return c
ESC = {0x18, 0x10, 0x11, 0x13, 0x90, 0x91, 0x93}
def zesc(d):
    o = bytearray()
    for b in d:
        if b in ESC: o += bytes([0x18, b ^ 0x40])
        elif b == 0x7F: o += b'\x18l'
        elif b == 0xFF and len(o) % 3 == 0: o += b'\x18m'       # mal ZDLE m, mal roh
        else: o.append(b)
    return bytes(o)
def zhex(t, p=b'\0\0\0\0'):
    h = bytes([t]) + p
    return b'**\x18B' + (h + crc16(h).to_bytes(2, 'big')).hex().encode() + b'\r\x8a' + (b'' if t in (3, 8) else b'\x11')
def zbin(t, p=b'\0\0\0\0', c32=False):
    h = bytes([t]) + p
    if c32: return b'*\x18C' + zesc(h + (binascii.crc32(h) ^ 0).to_bytes(4, 'little'))
    return b'*\x18A' + zesc(h + crc16(h).to_bytes(2, 'big'))
def sub(d, end, c32=False):
    if c32: cr = binascii.crc32(d + bytes([end])).to_bytes(4, 'little')
    else: cr = crc16(d + bytes([end])).to_bytes(2, 'big')
    return zesc(d) + bytes([0x18, end]) + zesc(cr)
def le4(n): return n.to_bytes(4, 'little')
def telnet_out(d): return d.replace(b'\xff', b'\xff\xff')

F1 = bytes(range(256)) * 9 + b'\x18\x18\xff\xff\x11\x13ENDE'          # 2310 Bytes
F2 = bytes((i * 31 + 7) & 0xFF for i in range(5000))
X1 = bytes((i * 5) & 0xFF for i in range(1000))                     # XMODEM: 8 Bloecke, letzter aufgefuellt
Y1 = b'Hallo YMODEM\r' * 100
Y2 = bytes(range(256)) * 5
class BBS:
    def __init__(s):
        s.mode = None; s.buf = b''; s.hdrs = []; s.up = bytearray(); s.upblk = []; s.iac = b''; s.log = []
    def strip(s, d):                       # Telnet: IAC-Folgen weg, 255 255 -> 255
        o = bytearray(); i = 0
        while i < len(d):
            b = d[i]
            if b == 0xFF:
                if i + 1 < len(d) and d[i + 1] == 0xFF: o.append(0xFF); i += 2; continue
                i += 3; continue
            o.append(b); i += 1
        return bytes(o)
    def __call__(s, d):
        if d is None: return b'\xff\xfb\x01\xff\xfb\x03\xff\xfb\x00\xff\xfd\x00Willkommen\r\n> '
        d = s.strip(d); s.log.append(d)
        if s.mode is None:
            s.buf += d
            if s.buf.endswith(b'\r\x00') or s.buf.endswith(b'\r'):
                cmd = s.buf.strip(b'\r\x00').decode(); s.buf = b''
                return telnet_out(s.cmd(cmd))
            return b''
        return telnet_out(getattr(s, 'm_' + s.mode)(d))
    def cmd(s, c):
        if c == 'z':
            s.mode = 'z'; s.zfiles = [(b'sub/dir/Test Datei_1.bin', F1, False), (b'zweite.datei.txt', F2, True)]; s.zi = 0
            s.zbuf = b''; return b'rz\r' + zhex(0)
        if c == 'x':
            s.mode = 'x'; s.data = X1; s.blk = 1; s.k = 128; return b'Starte XMODEM-Download.\r\n'
        if c == 'k':
            s.mode = 'x'; s.data = Y2; s.blk = 1; s.k = 1024; return b'1K\r\n'
        if c == 'y':
            s.mode = 'y'; s.files = [(b'hallo.txt', Y1), (b'bytes.bin', Y2)]; s.fi = 0; s.state = 'h'; return b'YMODEM\r\n'
        if c == 'u':
            s.mode = 'u'; s.up = bytearray(); s.exp = 1; return b'Bereit zum Empfang.\r\n'
        return b'?\r\n> '
    # ---- ZMODEM senden
    def m_z(s, d):
        s.zbuf += d; out = b''
        while True:
            i = s.zbuf.find(b'**\x18B')
            if i < 0 or len(s.zbuf) < i + 18: break
            hx = s.zbuf[i + 4:i + 18]; s.zbuf = s.zbuf[i + 18:]
            h = bytes.fromhex(hx.decode())
            assert crc16(h[:5]) == int.from_bytes(h[5:7], 'big'), 'CRC im Hex-Kopf falsch: %r' % hx
            t, p = h[0], int.from_bytes(h[1:5], 'little'); s.hdrs.append((t, p, h[1:5]))
            if t == 1:                     # ZRINIT
                if s.zi < len(s.zfiles):
                    n, data, c32 = s.zfiles[s.zi]
                    out += zbin(4, c32=c32) + sub(n + b'\0' + str(len(data)).encode() + b' 0 0\0', ord('k'), c32)
                else:
                    out += zhex(8)
            elif t == 9:                   # ZRPOS
                n, data, c32 = s.zfiles[s.zi]
                out += zbin(10, le4(p), c32)
                for k in range(p, len(data), 1024):
                    blk = data[k:k + 1024]
                    end = 'h' if k + 1024 >= len(data) else ('j' if (k // 1024) % 2 else 'i')
                    out += sub(blk, ord(end), c32)
                out += zbin(11, le4(len(data)), c32)
                s.zi += 1
            elif t == 8:                   # ZFIN
                out += b'OO'; s.mode = None; s.done = True
        return out
    # ---- XMODEM senden (Bloecke 128 oder 1K, CRC)
    def m_x(s, d):
        out = b''
        for b in d:
            if b in (ord('C'), 0x15) and s.blk == 1 or b == 6:
                if b == 6: s.blk += 1
                k = (s.blk - 1) * s.k
                if k >= len(s.data):
                    if getattr(s, 'eot', 0): s.mode = None; s.eot = 0; return b'\r\n> '
                    s.eot = 1; return b'\x04'
                blk = s.data[k:k + s.k].ljust(s.k, b'\x1a')
                out = bytes([1 if s.k == 128 else 2, s.blk & 255, 255 - (s.blk & 255)]) + blk + crc16(blk).to_bytes(2, 'big')
        return out
    # ---- YMODEM-Stapel
    def m_y(s, d):
        out = b''
        for b in d:
            if s.state == 'h' and b == ord('C'):
                if s.fi < len(s.files): n, data = s.files[s.fi]; h = n + b'\0' + str(len(data)).encode() + b' 0\0'
                else: h = b''
                blk = h.ljust(128, b'\0'); s.state = 'ha' if s.fi < len(s.files) else 'end'
                out = b'\x01\x00\xff' + blk + crc16(blk).to_bytes(2, 'big')
            elif s.state == 'end' and b == 6: s.mode = None; return b'\r\n> '
            elif s.state == 'ha' and b == 6: s.state = 'hc'
            elif s.state in ('hc', 'd') and (b == ord('C') and s.state == 'hc' or b == 6 and s.state == 'd'):
                if s.state == 'hc': s.blk = 1; s.state = 'd'
                else: s.blk += 1
                n, data = s.files[s.fi]; k = (s.blk - 1) * 1024
                if k >= len(data): s.state = 'eot'; return b'\x04'
                blk = data[k:k + 1024].ljust(1024, b'\x1a')
                out = bytes([2, s.blk & 255, 255 - (s.blk & 255)]) + blk + crc16(blk).to_bytes(2, 'big')
            elif s.state == 'eot' and b == 6: s.fi += 1; s.state = 'h'
        return out
    # ---- XMODEM empfangen (Upload vom IIgs)
    def m_u(s, d):
        s.buf += d; out = b''
        while s.buf:
            if s.buf[0] == 4: s.buf = s.buf[1:]; s.mode = None; s.upok = True; return b'\x06Danke.\r\n> '
            if s.buf[0] != 1: s.buf = s.buf[1:]; continue
            if len(s.buf) < 133: break
            p = s.buf[:133]; s.buf = s.buf[133:]
            ok = p[1] == (s.exp & 255) and p[2] == 255 - p[1] and crc16(p[3:131]) == int.from_bytes(p[131:133], 'big')
            s.upblk.append((p[1], ok))
            if ok: s.up += p[3:131]; s.exp += 1; out += b'\x06'
            else: out += b'\x15'
        return out
bbs = BBS()
net.pages[('10.1.1.20', 23, '*')] = bbs
mari = Marinetti(net)
cpu = boot(); cpu.mari = mari
settle()
click(101, 193)
typ('telnet://bbs.example'); key(0x0D); idle()
check(w('VTON') == 1 and w('TNON') == 1, 'Terminal verbunden')
def screen():
    return '\n'.join(term_rows(25))
def bbscmd(c):
    for ch in c: key(ord(ch))
    key(0x0D); idle(6)
# ---- ZMODEM: startet von selbst
bbscmd('z')
idle(30)
dk = {x.split(':')[-1]: FILES[x] for x in FILES if 'DOWNLOADS' in x.upper()}; print(list(dk))
print([h[:2] for h in bbs.hdrs])
check([h[0] for h in bbs.hdrs][:2] == [1, 9] and bbs.hdrs[0][2] == b'\0\0\0\x03', 'ZRINIT (CANFDX|CANOVIO), ZRPOS 0')
check(dk.get('TEST.DATEI.BIN') == F1, 'Datei 1 (CRC-16, Name ohne Ordner, gekuerzt): %r' % (len(dk.get('TEST.DATEI.BIN', b'')),))
check(dk.get('ZWEITE.DATE.TXT') == F2, 'Datei 2 (CRC-32)')
check(any(h[0] == 3 for h in bbs.hdrs), 'ZACK auf ZCRCQ')
check([h[0] for h in bbs.hdrs][-1] == 8 and getattr(bbs, 'done', False) and w('XMON') == 0, 'ZFIN, Ende')
sc = screen(); print(sc)
check('ZMODEM' in sc and 'Gespeichert' in sc and 'OO' not in sc and '**' in sc, 'Meldungen, "OO" verschluckt')
check(any(n.upper() == 'ZWEITE.DATE.TXT' and t == 4 for kk, v in FS.items() if kk.upper().endswith('DOWNLOADS') for n, t in v), 'Typ TXT nach Endung')
render(cpu, 'twx_1.png', 3)
# ---- XMODEM empfangen (128er-Bloecke)
bbscmd('x')
key(ord('d'), 0x0100); idle(2)
check(w('XMON') == 1 and 'Datei empfangen' in screen(), 'Apfel-D: Name erfragt')
typ('mein file'); key(0x0D); idle(30)
dk = {x.split(':')[-1]: FILES[x] for x in FILES if 'DOWNLOADS' in x.upper()}
check(dk.get('MEIN.FILE') == X1, 'XMODEM: Fuellbytes entfernt (%d)' % len(dk.get('MEIN.FILE', b'')))
check(w('XMON') == 0, 'XMODEM fertig')
# ---- XMODEM-1K ohne Namen
bbscmd('k')
key(ord('d'), 0x0100); key(0x0D); idle(30)
dk = {x.split(':')[-1]: FILES[x] for x in FILES if 'DOWNLOADS' in x.upper()}
check(dk.get('DOWNLOAD') == Y2, 'XMODEM-1K ohne Namen -> DOWNLOAD (%d)' % len(dk.get('DOWNLOAD', b'')))
# ---- YMODEM-Stapel
bbscmd('y')
key(ord('d'), 0x0100); key(0x0D); idle(40)
dk = {x.split(':')[-1]: FILES[x] for x in FILES if 'DOWNLOADS' in x.upper()}
check(dk.get('HALLO.TXT') == Y1 and dk.get('BYTES.BIN') == Y2, 'YMODEM: zwei Dateien, Groesse genau (%d, %d)' % (len(dk.get('HALLO.TXT', b'')), len(dk.get('BYTES.BIN', b''))))
check(w('XMON') == 0, 'YMODEM: Stapel beendet')
render(cpu, 'twx_2.png', 3)
# ---- XMODEM senden
FILES[':SYSTEM.DISK:DOWNLOADS:HOCH.TXT'] = UPL = bytes((i * 3 + 1) & 0xFF for i in range(700))
FS.setdefault(':SYSTEM.DISK:DOWNLOADS', []).append(('HOCH.TXT', 4))
bbscmd('u')
key(ord('u'), 0x0100); typ('hoch.txt'); key(0x0D); idle(2)
check(w('XMON') == 4, 'Senden wartet auf den Empfaenger')
c = [x['c'] for x in mari.conns.values() if x['c']][0]
c.out += b'C'; idle(30)
check(bytes(bbs.up[:700]) == UPL and all(ok for _, ok in bbs.upblk) and len(bbs.upblk) == 6, 'XMODEM gesendet: 6 Bloecke, CRC ok')
check(bytes(bbs.up[700:]) == b'\x1a' * (768 - 700) and getattr(bbs, 'upok', False) and w('XMON') == 0, 'aufgefuellt, EOT quittiert')
check('gesendet' in screen(), 'Meldung')
# ---- Abbruch mit Esc
bbscmd('x')
key(ord('d'), 0x0100); key(0x1B); idle(2)
check(w('XMON') == 0 and 'Abgebrochen' in screen(), 'Esc bricht die Eingabe ab')
render(cpu, 'twx_3.png', 3)
print('ALLES OK' if twbase.ok else 'FAIL Dateiuebertragung')
