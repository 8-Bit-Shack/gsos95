# WLAN-Modem am Modemanschluss (nachgebildeter SCC + Hayes-Modem): feste Wahl in GSOS95.NET,
# Baudrate, ATDTrechner:port, HTTP, NO CARRIER am Ende, https -> Proxy-Hinweis, Telnet
from twbase import *
import twbase
from modemsim import Modem
mdm = Modem(net, port=2)
t3.FILES[':SYSTEM.DISK:SYSTEM:GSOS95.NET'] = bytes(b'N1' + bytes([3, 1]) + bytes(22)).ljust(1536, b'\0') + bytes([5, 2, 2, 0]).ljust(512, b'\0')
oldrd, oldwr = AppCPU.io_rd, AppCPU.io_wr
def io_rd(s, r):
    if 0x38 <= r <= 0x3B: return mdm.io_rd(r)
    return oldrd(s, r)
def io_wr(s, r, v):
    if 0x38 <= r <= 0x3B: mdm.io_wr(r, v & 0xFF); return
    oldwr(s, r, v & 0xFF)
AppCPU.io_rd, AppCPU.io_wr = io_rd, io_wr
cpu = boot()
settle()
click(101, 193)
typ('frogfind.com'); key(0x0D)
check(w('NCARD') == 5 and w('NCSEL') == 5, 'WLAN-Modem gewaehlt (NCARD=%d)' % w('NCARD'))
check(mdm.baud() == 19200, 'Baudrate 19200 (Zeitkonstante %d)' % mdm.ch[0].wr[12])
check(b'ATDTFROGFIND.COM:80' in mdm.log and b'ATDTTIME.NIST.GOV:37' in mdm.log, 'gewaehlt: %r' % mdm.log[:6])
check(bstr(0xFC00) == b'FrogFind!' and txt().count(b'Welt') == 400, 'HTTP ueber das Modem')
check(getattr(cpu, 'timeset', None) is not None and len(twbase.TIMEQ) >= 1, 'Uhr per Time-Dienst (TCP 37) gestellt: %r' % (getattr(cpu, 'timeset', None),))
check(b'NO CARRIER' not in txt(), '"NO CARRIER" nicht in der Seite')
typ('frogfind.com/zwei'); key(0x0D)
check(bstr(0xFC00) == b'Zwei', 'zweite Seite (neu gewaehlt)')
typ('https://secure.test'); key(0x0D)
check(b'Proxy' in txt(), 'https ohne TLS: Proxy-Hinweis')
typ('telnet://bbs.example'); key(0x0D)
idle()
r = ''.join(term_rows())
check(w('TNON') == 1 and 'Willkommen in der Mailbox' in r, 'Telnet ueber das Modem')
for ch in 'gast': key(ord(ch))
key(0x0D); idle(6)
check(b''.join(twbase.TIN).startswith(b'gast'), 'Tasten gesendet: %r' % b''.join(twbase.TIN))
r = ''.join(term_rows(4))
check('Hallo gast' in r, 'Antwort im Terminal')
key(0x1B, 0x0100); idle(6)
check(w('TNON') == 0 and 'escape' in mdm.log and b'ATH' in mdm.log, 'Telnet beendet: +++ und ATH')
render(cpu, 'twd_a.png', 3)
# Modem antwortet nicht
mdm2 = Modem(net, port=2, answer=False); mdm = mdm2
cpu = boot(); settle()
click(101, 193)
typ('frogfind.com'); key(0x0D)
check(b'Modem' in txt(), 'Modem antwortet nicht: Meldung %r' % txt()[:50])
print('ALLES OK' if twbase.ok else 'FAIL Modem')
