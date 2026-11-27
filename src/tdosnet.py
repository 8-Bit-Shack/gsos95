# v2.18: Netzwerkbefehle der MS-DOS-Eingabeaufforderung (Uthernet II nachgebildet)
from twbase import *
import twbase
from w5100sim import W5100
chip = W5100(net)
oldrd, oldwr = t3.AppCPU.io_rd, t3.AppCPU.io_wr
def io_rd(s, r):
    if 0xB4 <= r <= 0xB7: return chip.io_rd(r - 0xB4)
    return oldrd(s, r)
def io_wr(s, r, v):
    if 0xB4 <= r <= 0xB7: chip.io_wr(r - 0xB4, v); return
    oldwr(s, r, v & 0xFF)
t3.AppCPU.io_rd, t3.AppCPU.io_wr = io_rd, io_wr
net.dns['finger.test'] = '10.1.1.79'
net.pages[('10.1.1.79', 79, '*')] = lambda d: None if d is None else (b'Login: marc\r\nName: Marc G.\r\nPlan:\r\nGS/OS95 fertig machen.\r\n', 'zu')
def scr(): return [r.rstrip() for r in term_rows(25)]
def has(t): return any(t in r for r in scr())
def show(): print('\n'.join(r for r in scr() if r))
def cmd(t): typ(t); key(0x0D); settle()
cpu = boot(); settle()
key(9); key(0x15); key(0x15); key(0x0A); key(0x0A); key(0x0D)   # Werkzeuge > MS-DOS
check(w('DOSON') == 1, 'MS-DOS offen')
cmd('ipconfig')
show()
check(has('IP-Konfiguration') and has('Uthernet II') and has('IP-Adresse') and has('DHCP'), 'IPCONFIG')
cmd('cls'); cmd('ping frogfind.com')
show()
check(has('Ping wird ausgef') and sum(1 for r in scr() if r.startswith('Antwort von frogfind.com')) == 4, 'PING: vier Antworten')
cmd('cls'); cmd('nslookup frogfind.com')
check(has('Adresse:  10.1.1.1'), 'NSLOOKUP')
cmd('nslookup gibtsnicht.test')
check(has('nicht gefunden'), 'NSLOOKUP unbekannt')
cmd('cls'); cmd('finger marc@finger.test')
show()
check(has('[finger.test]') and has('Name: Marc G.') and has('GS/OS95 fertig machen.'), 'FINGER')
render(cpu, 'tdosnet.png', 3)
cmd('cls'); cmd('help')
check(has('IPCONFIG') and has('FINGER'), 'HELP nennt die Netzwerkbefehle')
print('ALLES OK' if twbase.ok else 'FAIL')
