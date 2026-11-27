# v2.18: ssh://benutzer@rechner ueber die FujiNet (Karte verschluesselt, Anmeldung $FD/$FE)
from twbase import *
import twbase
from fujisim import FujiNet
fn = FujiNet(net)
net.ssh = {'ssh.example': ('marc', 'geheim')}
net.dns['ssh.example'] = '10.1.1.22'
SIN = []
def shell(d):
    if d is None: return b'Welcome to Ubuntu\r\nmarc@server:~$ '
    SIN.append(d)
    if d.endswith(b'\r'): return b'\r\n' + b''.join(SIN).rstrip(b'\r').split(b'\r')[-1] + b': ok\r\nmarc@server:~$ '
    return d                      # Gegenstelle zeigt Eingaben selbst
net.pages[('10.1.1.22', 22, '*')] = shell
cpu = boot(); fn.install(cpu)
settle()
click(101, 193)
def rows(): return [r.rstrip() for r in term_rows(25)]
def has(t): return any(t in r for r in rows())
# 1) mit Benutzer in der Adresse
typ('ssh://marc@ssh.example'); key(0x0D)
check(w('VTON') == 1 and w('SSHPW') == 2, 'Terminal offen, Passwort wird abgefragt')
check(has('ssh marc@ssh.example') and has('marc@ssh.example - Passwort:'), 'Kopf und Passwortfrage')
render(cpu, 'tssh_0.png', 3)
for ch in 'geheim': key(ord(ch))
check(not has('geheim'), 'Passwort bleibt unsichtbar')
key(0x0D); idle(4)
check(('ctl', '\xfd', b'marc\x00') in fn.log and ('ctl', '\xfe', b'geheim\x00') in fn.log, 'Benutzer und Passwort an die Karte ($FD/$FE)')
check(('fopen', 12, 'N:SSH://ssh.example:22/') in net.log, 'geoeffnet als N:SSH://ssh.example:22/')
check(w('TNON') == 1 and has('Welcome to Ubuntu'), 'verbunden, Begruessung der Gegenstelle')
check(not any(cpu.mem[ADDR + lbl['SSHP']:ADDR + lbl['SSHP'] + 66]), 'Passwort danach aus dem Speicher geloescht')
for ch in 'ls': key(ord(ch))
key(0x0D); idle(4)
check(has('ls: ok'), 'Eingabe und Antwort')
render(cpu, 'tssh_1.png', 3)
key(0x1B, 0x0100); idle(4)
check(w('TNON') == 0 and w('VTON') == 0, 'Apfel-Esc beendet')
# 2) ohne Benutzer, falsches Passwort
typ('ssh://ssh.example'); key(0x0D)
check(w('SSHPW') == 1 and has('Benutzer:'), 'ohne Benutzer: erst "Benutzer:"')
for ch in 'marc': key(ord(ch))
key(0x0D)
check(w('SSHPW') == 2 and has('marc@ssh.example - Passwort:'), 'dann das Passwort')
for ch in 'falsch': key(ord(ch))
key(0x0D); idle(4)
check(has('Anmeldung fehlgeschlagen') and w('VTWAIT') == 1 and w('TNON') == 0, 'falsches Passwort: Meldung, Taste zurueck')
key(ord(' ')); idle(2)
check(w('VTON') == 0, 'Taste: zurueck zum Browser')
# 3) Esc waehrend der Eingabe
typ('ssh://marc@ssh.example'); key(0x0D)
key(ord('x')); key(0x1B); idle(2)
check(w('VTON') == 0 and w('SSHPW') == 0 and w('FNSSH') == 0, 'Esc bei der Passwortfrage: abgebrochen')
typ('frogfind.com'); key(0x0D)
check(('fopen', 12, 'N:TCP://frogfind.com:80/') in net.log[-3:] , 'danach normale Seiten wieder ueber N:TCP')
print('ALLES OK' if twbase.ok else 'FAIL SSH')
