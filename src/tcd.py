# v2.15: CD-Wiedergabe (Fensterart 23) mit nachgebildeten SCSI-CD-Laufwerken (cdsim.py)
from twbase import *
import twbase, cdsim
X3 = 0x200000
cd2 = cdsim.CD('scsi2')
cdr = cdsim.CD('ruby', 'SONY', 'CD-ROM CDU-8002')
cdsim.install(t3, {4: cd2, 5: cdr})
def x3(n): return cpu.rd16(X3 + lbl[n])
def x3s(n):
    a = X3 + lbl[n]; return bytes(cpu.mem[a + 1:a + 1 + cpu.mem[a]]).decode('latin-1')
def jsl(name, bank=X3, A=None):
    PBR, PC, DBR, S = cpu.PBR, cpu.PC, cpu.DBR, cpu.S
    ret = ADDR + lbl['code_end']
    cpu.push(ADDR >> 16); cpu.push16((ret - 1) & 0xFFFF)
    cpu.PBR = (bank + lbl[name]) >> 16; cpu.PC = (bank + lbl[name]) & 0xFFFF; cpu.DBR = ADDR >> 16
    if A is not None: cpu.A = A
    run(cpu, 80_000_000, stop_at=ret)
    cpu.PBR, cpu.PC, cpu.DBR, cpu.S = PBR, PC, DBR, S
def tick(): jsl('xf_cd_tick'); settle()
def lcd(): return (x3s('CDL1'), x3s('CDL1R'), x3s('CDL2R'))
def ops(cd): return [c[2][0] for c in cd.log]

cpu = boot(); settle()
# ---- Start > Programme > CD-Wiedergabe (Zeile nach Terminal)
key(9); key(0x15)
key(0x15); key(0x0A)                    # Gruppe Werkzeuge > CD-Wiedergabe
check(w('S2G') == 1 and w('S2H') == 1, 'Werkzeuge > CD-Wiedergabe gewaehlt (%d/%d)' % (w('S2G'), w('S2H')))
render(cpu, 'tcd_menu.png', 3)
key(0x0D)
check(w('WIN') == 23, 'Fenster CD-Wiedergabe offen (WIN %d)' % w('WIN'))
check(x3('CDN') == 2, 'zwei CD-Laufwerke gefunden (%d)' % x3('CDN'))
check(x3('CDMD') == 2 and x3('CDOK') == 1, 'SCSI-2-Laufwerk, Inhaltsverzeichnis gelesen')
check(x3s('CDNM') == 'SONY CD-ROM CDU-8003', 'Name aus INQUIRY (%r)' % x3s('CDNM'))
check(x3('CDT0') == 1 and x3('CDT1') == 5, 'Titel 1..5')
print(lcd())
check(lcd() == ('Titel 1/5', '00:00 / 04:10', 'Gesamt 19:58'), 'Anzeige gestoppt')
render(cpu, 'tcd_stop.png', 3)
# ---- Wiedergabe (Eingabetaste)
key(0x0D)
check(cd2.state == 'play' and cd2.pos == cdsim.fr(0, 2, 0), 'spielt Titel 1')
c = cd2.log[-1][2]
check(c[0] == 0x47 and c[6:9] == bytes([15, 10, 40]), 'PLAY AUDIO MSF bis vor den Datentitel (%r)' % c[:10])
tick(); tick()
check(x3('CDST') == 1 and x3('CDRS') == 2, 'Zeit laeuft (%d s)' % x3('CDRS'))
check(lcd()[1] == '00:02 / 04:10', 'Anzeige Zeit %r' % lcd()[1])
render(cpu, 'tcd_play.png', 3)
# ---- weiter / zurueck
key(0x15)
check(cd2.state == 'play' and cd2.track_at(cd2.pos) == 2 and x3('CDTR') == 2, 'rechts: Titel 2')
key(0x08)
check(x3('CDTR') == 1 and cd2.track_at(cd2.pos) == 1, 'links (am Anfang): Titel 1')
tick(); tick(); tick(); tick()
key(0x08)
check(x3('CDTR') == 1 and cd2.pos == cdsim.fr(0, 2, 0), 'links nach 4 s: Titelanfang')
# ---- Pause / weiter
key(0x20)
check(cd2.state == 'pause' and x3('CDST') == 2, 'Leertaste: Pause')
tick()
check(x3('CDST') == 2 and lcd()[0] == 'Titel 1/5', 'Pause bleibt')
key(0x20)
check(cd2.state == 'play', 'Leertaste: weiter')
# ---- Titel im Raster anklicken (Titel 4: Zeile 0, Spalte 3)
ox, oy = 40, 8
click(ox + 12 + 3 * 27 + 10, oy + 110 + 5)
check(x3('CDTR') == 4 and cd2.track_at(cd2.pos) == 4 and cd2.state == 'play', 'Klick auf Titel 4')
render(cpu, 'tcd_t4.png', 3)
click(ox + 12 + 4 * 27 + 10, oy + 110 + 5)     # Datentitel 5: nichts
check(x3('CDTR') == 4, 'Datentitel wird nicht gespielt')
# ---- Stopp und wieder spielen (START UNIT davor)
key(ord('s'))
check(cd2.state == 'none' and cd2.stopped and x3('CDST') == 0, 'S: Stopp (STOP UNIT)')
tick()
check(x3('CDST') == 0 and lcd()[1].startswith('00:00'), 'gestoppt: Zeit 0')
click(ox + 12 + 1 * 36 + 16, oy + 84 + 9)     # Knopf Wiedergabe
check(cd2.state == 'play' and not cd2.stopped and x3('CDTR') == 4, 'Wiedergabe-Knopf: START UNIT + Titel 4')
# ---- Ende der CD
cd2.tick = 75 * 400
tick()
check(x3('CDST') == 0 and x3('CDTR') == 1, 'Ende: gestoppt, wieder Titel 1')
cd2.tick = 75
# ---- zweites Laufwerk (Apple CD 150): L
key(ord('l'))
check(x3('CDI') == 1 and x3('CDMD') == 1 and x3('CDOK') == 1, 'L: Apple-Laufwerk, Inhalt ueber $C1')
check(x3s('CDNM') == 'SONY CD-ROM CDU-8002' and x3('CDT1') == 5, 'Name/Titel Apple-Laufwerk')
print(lcd())
check(lcd()[2] == 'Gesamt 19:58', 'Gesamtzeit Apple %r' % lcd()[2])
key(0x0D)
check(cdr.state == 'play' and cdr.track_at(cdr.pos) == 1, 'Apple: spielt Titel 1 ($C8)')
tick(); tick(); tick()
check(x3('CDST') == 1 and x3('CDRS') == 3 and x3('CDTR') == 1, 'Apple: Zeit aus $CC/$C2 (%d)' % x3('CDRS'))
key(0x15)
check(cdr.track_at(cdr.pos) == 2 and x3('CDTR') == 2, 'Apple: weiter Titel 2')
key(0x20)
check(cdr.state == 'pause' and x3('CDST') == 2, 'Apple: Pause ($CA)')
key(0x20)
check(cdr.state == 'play', 'Apple: weiter')
render(cpu, 'tcd_ruby.png', 3)
key(ord('s'))
check(cdr.state == 'none' and x3('CDST') == 0, 'Apple: Stopp ($CB)')
# ---- Auswerfen (Apple: Eject des Treibers)
key(ord('e'))
check(not cdr.disc and x3('CDOK') == 0, 'E: ausgeworfen')
check(x3s('CDL1') == '', 'Anzeige ohne CD')
render(cpu, 'tcd_eject.png', 3)
# ---- CD wieder einlegen: Abfrage liest das Inhaltsverzeichnis
cdr.disc = True
tick(); tick()
check(x3('CDOK') == 1, 'wieder eingelegt: erkannt')
# ---- SCSI-2 auswerfen ueber START/STOP
key(ord('l'))
key(ord('e'))
check(not cd2.disc and any(c[2][0] == 0x1B and c[2][4] == 2 for c in cd2.log[-3:]), 'SCSI-2: Auswerfen mit LoEj')
# ---- Fenster schliessen, Musik laeuft weiter
key(ord('l')); key(0x0D)
key(0x1B)
check(w('WIN') != 23 and cdr.state == 'play', 'Esc schliesst, CD spielt weiter')
# ---- Audio-CD einlegen: Diskettenabfrage oeffnet die CD-Wiedergabe und spielt (wie AutoPlay)
key(ord('s')) if w('WIN') == 23 else None
cd2.disc = False; cd2.state = 'none'
jsl('xf_dpoll', 0x050000)                 # Anfangszustand
cd2.disc = True
jsl('xf_dpoll', 0x050000); settle()
check(w('WIN') == 23 and x3('CDI') == 0 and cd2.state == 'play', 'AutoPlay: Fenster offen, Titel 1 spielt')
render(cpu, 'tcd_auto.png', 3)
# ---- Auswerfen am Laufwerk (Treiber meldet weiter "online"): TEST UNIT READY merkt es
n0 = sum(1 for c in cd2.log if c[1] == 0x8000)
cd2.disc = False; cd2.state = 'none'
jsl('xf_dpoll', 0x050000); settle()
check(cpu.rd16(ADDR + lbl['dpst'] + 8) == 0, 'dpoll: CD weg erkannt (TEST UNIT READY)')
check(sum(1 for c in cd2.log if c[1] == 0x8000) > n0, 'TEST UNIT READY gesendet')
tick()
check(x3('CDOK') == 0, 'Wiedergabe: keine CD')
cd2.disc = True; cd2.attention = True
jsl('xf_dpoll', 0x050000); settle()
check(cpu.rd16(ADDR + lbl['dpst'] + 8) == 0x10 and cd2.state == 'play', 'neu eingelegt (nach Unit Attention): erkannt, spielt')
print('ALLES OK' if twbase.ok else 'FAIL CD-Wiedergabe')
