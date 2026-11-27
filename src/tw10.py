# Telnet und IRC
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
net.dns['bbs.example'] = '10.1.1.20'
net.dns['irc.example'] = '10.1.1.21'
TIN = []
def bbs(d):
    if d is None: return b'\xff\xfb\x01\xff\xfb\x03Willkommen in der Mailbox\r\nlogin: '
    TIN.append(d)
    if d.endswith(b'\r\n'): return b'Hallo ' + d[:-2] + b'!\r\n> '
    return b''
IIN = []
def irc(d):
    if d is None: return b''
    IIN.append(d); out = b''
    for ln in d.split(b'\r\n'):
        if ln.startswith(b'USER'): out += b':srv 001 nick :Willkommen im Netz\r\n'
        if ln.startswith(b'JOIN'): out += b':me!u@h JOIN :#apple2\r\n:bob!b@h PRIVMSG #apple2 :Hallo IIgs\r\nPING :abc\r\n'
    return out
net.pages[('10.1.1.20', 23, '*')] = bbs
net.pages[('10.1.1.21', 6667, '*')] = irc
def idle(n=12):
    for i in range(n): settle()
click(101, 193)
typ('telnet://bbs.example'); key(0x0D)
idle()
check(w('TNON') == 1 and w('VTON') == 1, 'Telnet verbunden (Vollbild-Terminal)')
imb = w('IMBK') << 16
row0 = bytes(cpu.mem[imb + 2 * i] for i in range(80)).decode('cp437')
row1 = ''.join(bytes(cpu.mem[imb + 160 * r + 2 * i] for i in range(80)).decode('cp437') for r in (1, 2, 3))
print(repr(row0), repr(row1))
check('Willkommen in der Mailbox' in row0 + row1, 'Begruessung im Terminal')
for ch in 'gast': key(ord(ch))
key(0x0D)
idle(4)
print(TIN)
check(b''.join(TIN).startswith(b'\xff\xfd\x01\xff\xfd\x03') or b'\xff\xfd\x01' in b''.join(TIN), 'Telnet-Antworten DO ECHO/SGA')
check(b''.join(TIN).endswith(b'gast\r\x00'), 'Tasten einzeln gesendet, Return als CR NUL')
key(0x1B, 0x0100); idle(4)
check(w('VTON') == 0 and w('TNON') == 0, 'Apfel-Esc beendet Telnet')
render(cpu, 'tw10a.png', 3)
click(101, 193)
typ('irc://irc.example/apple2'); key(0x0D)
idle(30)
print(IIN)
A = b''.join(IIN)
check(b'NICK GS95' in A and b'USER gsos95' in A, 'NICK/USER')
check(b'JOIN #apple2\r\n' in A, 'JOIN nach 001')
check(b'PONG :abc\r\n' in A, 'PONG')
L = lines(); print(L)
check(any('<bob> Hallo IIgs' in l for l in L), 'Nachricht angezeigt')
check(any(l.startswith('* me JOIN :#apple2') for l in L), 'JOIN-Zeile')
typ('hi there'); key(0x0D); idle(4)
typ('/msg bob yo'); key(0x0D); idle(4)
typ('/join #x'); key(0x0D); idle(12)
A = b''.join(IIN); print(A[-80:])
check(b'PRIVMSG #apple2 :hi there\r\n' in A, 'Text an den Kanal')
check(b'PRIVMSG bob :yo\r\n' in A, '/msg')
check(b'JOIN #x\r\n' in A, '/join')
L = lines(); print(L)
render(cpu, 'tw10b.png', 3)
print('ALLES OK' if ok else 'FEHLER')
