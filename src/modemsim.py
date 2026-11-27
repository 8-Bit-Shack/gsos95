# Nachgebildeter SCC 8530 (abgefragt, ohne Interrupts) mit Hayes-WLAN-Modem (wie WiModem232/Zimodem
# bzw. das virtuelle Modem von GSplus): AT, ATDTrechner:port -> CONNECT, Daten, NO CARRIER, +++ / ATH.
from netconn import Conn

class Channel:
    def __init__(s): s.ptr = 0; s.wr = [0] * 16; s.ptrset = False
class Modem:
    def __init__(s, net, port=2, answer=True):
        s.net = net; s.port = port; s.answer = answer
        s.ch = {0: Channel(), 1: Channel()}         # 0 = Kanal B (Modem), 1 = Kanal A (Drucker)
        s.rx = bytearray(); s.line = b''; s.online = None; s.plus = 0
        s.log = []; s.sent = bytearray()
    def mine(s, idx): return (idx == 0) == (s.port == 2)
    def io_rd(s, r):                                 # r: 0x38..0x3B
        idx = r & 1
        if r < 0x3A:
            c = s.ch[idx]; c.ptrset = False
            reg = c.ptr; c.ptr = 0
            if reg != 0: return 0
            if not s.mine(idx): return 0x04
            s.pump()
            v = 0x04
            if s.rx: v |= 0x01
            if s.online is not None: v |= 0x08
            return v
        if not s.mine(idx) or not s.rx: return 0
        b = s.rx[0]; del s.rx[0]; return b
    def io_wr(s, r, v):
        idx = r & 1
        if r < 0x3A:
            c = s.ch[idx]
            if not c.ptrset: c.ptr = v & 0x0F; c.ptrset = True; return
            c.wr[c.ptr] = v; c.ptrset = False; c.ptr = 0
            return
        if not s.mine(idx): return
        ch = s.ch[idx]
        assert ch.wr[5] & 0x08 and ch.wr[3] & 0x01, 'Kanal nicht eingeschaltet'
        s.tx(v)
    def baud(s):
        tc = s.ch[0 if s.port == 2 else 1].wr[12]
        return round(3686400 / (32 * (tc + 2)))
    def say(s, t): s.rx += b'\r\n' + t + b'\r\n'
    def tx(s, v):
        s.sent.append(v)
        if s.online is not None:
            if v == ord('+'):
                s.plus += 1
                if s.plus == 3: s.online = None; s.plus = 0; s.say(b'OK'); s.log.append('escape')
                return
            if s.plus: s.online.send(b'+' * s.plus); s.plus = 0
            s.online.send(bytes([v])); return
        if not s.answer: return
        s.rx.append(v)                                  # Echo
        if v != 13: s.line += bytes([v]); return
        cmd = s.line.strip().upper(); s.line = b''
        s.log.append(cmd)
        if cmd.startswith(b'ATDT') or cmd.startswith(b'ATD'):
            dest = cmd[4:] if cmd.startswith(b'ATDT') else cmd[3:]
            host, _, port = dest.decode().partition(':')
            ip = s.net.dns.get(host.lower())
            s.net.log.append(('mdial', host.lower(), int(port or 23)))
            if ip and Conn.exists(s.net, ip, int(port or 23)):
                s.online = Conn(s.net, ip, int(port or 23)); s.say(b'CONNECT %d' % s.baud())
            else: s.say(b'NO CARRIER')
        elif cmd.startswith(b'AT'): s.say(b'OK')
    def pump(s):
        if s.online is None: return
        if len(s.rx) < 3:
            s.rx += s.online.take(3)
        if s.online.done() and not s.rx:
            s.online = None; s.say(b'NO CARRIER')
