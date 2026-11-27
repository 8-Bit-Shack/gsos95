# Mailbox-Telefonbuch (about:bbs): Schnellstart, Liste, direkt verbinden, speichern, aendern, loeschen
exec(open('tw1.py').read().split("# 1) Schnellstart")[0])
net.dns['bbs.example'] = '10.1.1.20'
net.dns['cqbbs.ddns.net'] = '10.1.1.21'
TIN = []
def bbs(d):
    if d is None: return b'\x1b[2JHallo vom Test\r\n'
    TIN.append(d); return b''
net.pages[('10.1.1.20', 2323, '*')] = bbs
net.pages[('10.1.1.21', 6502, '*')] = bbs
def idle(n=8):
    for i in range(n): settle()
FLD = 0xC210
def fields():
    b = bank(); out = []
    for i in range(w('HNFLD')):
        a = b + FLD + 100 * i
        t, f = cpu.mem[a], cpu.mem[a + 1]
        nm = bytes(cpu.mem[a + 3:a + 3 + cpu.mem[a + 2]]).decode('latin-1')
        v = bytes(cpu.mem[a + 35:a + 35 + cpu.mem[a + 34]]).decode('latin-1')
        out.append((i, t, f, nm, v))
    return out
def setf(form, name, val):
    for i, t, f, nm, v in fields():
        if f == form and nm == name:
            a = bank() + FLD + 100 * i
            cpu.mem[a + 34] = len(val); cpu.mem[a + 35:a + 35 + len(val)] = val.encode('latin-1')
            return i
    raise Exception('Feld fehlt ' + name)
def submit(form, name):
    i = setf(form, name, dict((nm, v) for _, _, f, nm, v in fields() if f == form)[name])
    cpu.wr16(ADDR + lbl['FFOC'], i); cpu.wr16(ADDR + lbl['ADRFOC'], 0); cpu.wr16(ADDR + lbl['ADRSEL'], 0)
    key(0x0D); idle()
def plain(L): return [l.replace('<', '').replace('>', '').replace('|', '') for l in L]
def raw():
    return bytes(cpu.mem[bank():bank() + w('HO')]).decode('latin-1')
def entry(k):
    a = ADDR + lbl['NPREF'] + 1024 + 64 * k
    m = cpu.mem
    return (bytes(m[a + 1:a + 1 + m[a]]).decode('latin-1'), bytes(m[a + 25:a + 25 + m[a + 24]]).decode('latin-1'), m[a + 58] | m[a + 59] << 8)
# 1) Schnellstart-Symbol "Terminal"
click(113, 193); idle(4)
L = lines(); print(L[:12])
check(bstr(0xFD80) == b'about:bbs' and bstr(0xFC00).startswith(b'Mailbox-Telefonbuch'), 'Schnellstart oeffnet das Telefonbuch')
check(any("Captain's Quarters (cqbbs.ddns.net:6502)" in l for l in plain(L)) and any('Electronic Chicken' in l for l in L), 'Voreinstellung: zwei Mailboxen')
check('href="telnet://cqbbs.ddns.net:6502/"' in raw(), 'Link auf telnet://cqbbs.ddns.net:6502/')
print(fields())
render(cpu, 'tw18a.png', 3)
# 2) direkt verbinden
setf(0, 'h', 'bbs.example'); setf(0, 'p', '2323')
submit(0, 'h')
check(w('VTON') == 1 and ('connect', '10.1.1.20', 2323) in net.log, 'Direkt verbinden: Terminal mit bbs.example:2323')
key(0x1D); idle(4)
check(w('VTON') == 0, 'Strg-] zurueck')
# 3) Telefonbuch wieder (Fenster ist offen)
click(113, 193); idle(4)
check(bstr(0xFD80) == b'about:bbs', 'Schnellstart bei offenem Fenster: Telefonbuch')
# 4) neue Mailbox speichern (Name mit Grossbuchstaben und Leerzeichen)
setf(1, 'n', 'Test BBS "Eins"'); setf(1, 'h', 'bbs.example'); setf(1, 'p', '2323')
submit(1, 'n')
print(entry(2))
check(entry(2) == ('Test BBS "Eins"', 'bbs.example', 2323), 'gespeichert als Eintrag 3')
check(any('Test BBS "Eins" (bbs.example:2323)' in l for l in plain(lines())), 'in der Liste')
check(bstr(0xFD80) == b'about:bbs', 'Adresse wieder about:bbs')
# 5) aendern
click(101, 193); typ('about:bbse3'); key(0x0D); idle(4)
fl = dict((nm, v) for _, _, f, nm, v in fields() if f == 1)
print(fl)
check(fl.get('n') == 'Test BBS "Eins"' and fl.get('h') == 'bbs.example' and fl.get('p') == '2323' and fl.get('i') == '3', 'Formular mit Eintrag 3 gefuellt')
setf(1, 'n', 'Zweiter Name')
submit(1, 'n')
check(entry(2)[0] == 'Zweiter Name' and entry(3)[0] == '', 'Eintrag 3 geaendert, kein neuer')
# 6) Port leer -> 23, ohne Name -> Rechner
click(101, 193); typ('about:bbs'); key(0x0D); idle(4)
setf(1, 'n', ''); setf(1, 'h', 'bbs.example'); setf(1, 'p', '')
submit(1, 'h')
check(entry(3) == ('bbs.example', 'bbs.example', 23), 'ohne Name/Port: Rechner als Name, Port 23')
# 7) loeschen
click(101, 193); typ('about:bbsd1'); key(0x0D); idle(4)
check(entry(0)[0] == '' and not any("Captain's Quarters" in l for l in lines()), 'Eintrag 1 geloescht')
check(cpu.mem[ADDR + lbl['NPREF'] + 1534:ADDR + lbl['NPREF'] + 1536] == bytearray(b'B1'), 'Liste angelegt (B1)')
render(cpu, 'tw18b.png', 3)
# 8) Link einer Mailbox anklicken: Eintrag 2 (Electronic Chicken -> nicht im Simulator) daher Eintrag 3
TIN.clear()
click(101, 193); typ('telnet://bbs.example:2323/'); key(0x0D); idle()
check(w('VTON') == 1, 'telnet-Adresse aus dem Telefonbuch verbindet')
key(0x1D); idle(4)
print('ALLES OK' if ok else 'FEHLER')
