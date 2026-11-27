import sys
sys.path.insert(0, __import__('os').path.dirname(__import__('os').path.abspath(__file__)))
from font import render, G

# ---------------- Palette (12 bit $0RGB) ----------------
PAL = [0x000, 0x888, 0x841, 0x72C, 0x00F, 0x080, 0xF80, 0xF00, 0xFA9, 0xFF0, 0x0E0, 0x6AF, 0x088, 0x008, 0xCCC, 0xFFF]
# 2,3,8,10 = Apple-Standard (Braun, Lila, Beige, Gruen); beim Start Ladebalken-Farbzyklus
# Belegung wie Apples Standardpalette (0 schwarz, 14 hellgrau, 15 weiss ...), damit NDAs passen
# 12-15: Farbzyklus fuer Ladebalken (Blauverlauf)

out = []
def emit(s): out.append(s)

def bytes_line(data):
    for i in range(0, len(data), 16):
        emit("  .byte " + ",".join("$%02X" % b for b in data[i:i+16]))

# 16 Paletten: 0 = Desktop, 1..15 = Splash-Verlauf (Index 1 = Hintergrundverlauf)
emit("palettes:")
import splashgrad as SG
for p in range(16):
    pal = list(PAL)
    if p > 0:
        pal[12] = SG.SHADES[p - 1]
        pal[13] = SG.SHADES[p]
    data = []
    for c in pal:
        data += [c & 0xFF, c >> 8]
    bytes_line(data)
# SCB und Fuellmuster fuer den Splash: 60 Verlaufsstufen ueber 200 Zeilen
emit("splash_scb:")
bytes_line([SG.scb(y) for y in range(200)])
emit("splash_fill:")
bytes_line([SG.fill(y) for y in range(200)])

# ---------------- 1bpp Texte (Deutsch / Englisch) ----------------
T = [  # name, deutsch, english, bold, scale
 ("t_start","Start","Start",True,1),
 ("t_arbeit","Arbeitsplatz","My Computer",False,1),
 ("t_papier","Papierkorb","Recycle Bin",False,1),
 ("t_lies","Liesmich","Readme",False,1),
 ("m_prog","Programme","Programs",False,1),
 ("m_finder","GS/OS-Finder","GS/OS Finder",False,1),
 ("m_einst","Einstellungen","Settings",False,1),
 ("m_hilfe","Hilfe","Help",False,1),
 ("m_ausf","Ausführen...","Run...",False,1),
 ("m_beenden","Beenden...","Shut Down...",False,1),
 ("m_arrow",">",">",False,1),
 ("ti_1","Arbeitsplatz","My Computer",True,1),
 ("ti_2","Papierkorb","Recycle Bin",True,1),
 ("ti_3","Liesmich - Editor","Readme - Notepad",True,1),
 ("ti_4","Programme","Programs",True,1),
 ("ti_5","Dokumente","Documents",True,1),
 ("ti_6","Systemsteuerung","Control Panel",True,1),
 ("ti_7","Info über GS/OS95","About GS/OS95",True,1),
 ("ti_8","Ausführen","Run",True,1),
 ("ti_9","GS/OS95","GS/OS95",True,1),
 ("ti_10","GS/OS95 beenden","Shut Down GS/OS95",True,1),
 ("t_menubar","Datei   Bearbeiten   Ansicht   ?","File   Edit   View   Help",False,1),
 ("t_obj0","0 Objekt(e)","0 object(s)",False,1),
 ("t_obj3","3 Objekt(e)","3 object(s)",False,1),
 ("t_objsuf"," Objekt(e)"," object(s)",False,1),
 ("t_leer","Der Papierkorb ist leer.","The Recycle Bin is empty.",False,1),
 ("t_keinedok","Keine Dokumente vorhanden.","There are no documents.",False,1),
 ("t_anzeige","Anzeige","Display",False,1),
 ("t_maus","Maus","Mouse",False,1),
 ("t_datum","Datum/Uhrzeit","Date/Time",False,1),
 ("t_ok","OK","OK",False,1),
 ("t_abbr","Abbrechen","Cancel",False,1),
 ("t_i1","GS/OS95  Version 2.18.0","GS/OS95  Version 2.18.0",True,1),
 ("t_i2","Eine Oberfläche im Stil von 1995","A desktop in the style of 1995",False,1),
 ("t_i3","für den Apple IIgs mit GS/OS.","for the Apple IIgs with GS/OS.",False,1),
 ("t_i4","Erstellt von Marc Golombeck.","Created by Marc Golombeck.",False,1),
 ("t_i5","Pixel, Nostalgie, 65816-Assembler.","Pixels, nostalgia, 65816 assembly.",False,1),
 ("t_r1","Geben Sie den Namen eines Programms ein.","Type the name of a program to open.",False,1),
 ("t_r2","Öffnen:","Open:",False,1),
 ("t_r3","GS.OS","GS.OS",False,1),
 ("t_s1","Sie können den Computer","It's now safe to turn off",False,2),
 ("t_s2","jetzt ausschalten.","your computer.",False,2),
 ("t_s3","(Taste oder Klick: Neustart)","(Press a key or click to restart)",False,1),
 ("t_logo","GS/OS95","GS/OS95",True,4),
 ("t_ed","Apple IIgs Edition","Apple IIgs Edition",True,1),
 ("t_by","Created by Marc Golombeck","Created by Marc Golombeck",False,1),
 ("t_build","Build v"+__import__('splashgrad').BUILD,"Build v"+__import__('splashgrad').BUILD,False,1),
 ("t_down","GS/OS95 wird heruntergefahren...","GS/OS95 is shutting down...",True,1),
 ("t_up","Aufwärts","Up",False,1),
 ("t_lt","<","<",False,1),
 ("t_gt",">",">",False,1),
 ("t_e1","Diese Datei kann nicht geöffnet werden.","This file cannot be opened.",False,1),
 ("t_e2","Es ist kein Programm damit verknüpft.","No program is associated with it.",False,1),
 ("t_e3","Ordner oder Laufwerk ist nicht lesbar.","The folder or drive cannot be read.",False,1),
 ("t_e4","Programm konnte nicht gestartet werden.","The program could not be started.",False,1),
 ("t_q1","Was möchten Sie tun?","What do you want to do?",False,1),
 ("t_bshut","Herunterfahren","Shut down",False,1),
 ("t_brest","Neu starten","Restart",False,1),
 ("t_bfind","Zum GS/OS-Modus wechseln","Switch to GS/OS mode",False,1),
 ("t_c1","Kontrollfelder (Zeit, Monitor, Maus ...)","Control panels (time, monitor, mouse ...)",False,1),
 ("t_c2","öffnest du über Start > Zubehör >","are opened via Start > Accessories >",False,1),
 ("t_c3","Control Panel.","Control Panel.",False,1),
 ("t_cbtn","Finder starten","Start Finder",False,1),
 ("t_lang","Sprache:","Language:",False,1),
 ("t_ldeu","Deutsch","Deutsch",False,1),
 ("t_leng","English","English",False,1),
 ("t_wait","Bitte warten...","Please wait...",False,1),
 ("mb_file","Datei","File",False,1),
 ("mb_edit","Bearbeiten","Edit",False,1),
 ("mb_view","Ansicht","View",False,1),
 ("mb_help","?","Help",False,1),
 ("fm_new","Neuer Ordner","New Folder",False,1),
 ("fm_ren","Umbenennen","Rename",False,1),
 ("fm_del","Löschen","Delete",False,1),
 ("fm_prop","Eigenschaften","Properties",False,1),
 ("fm_close","Schliessen","Close",False,1),
 ("fm_empty","Papierkorb leeren","Empty Recycle Bin",False,1),
 ("fm_restore","Wiederherstellen","Restore",False,1),
 ("fm_cut","Ausschneiden","Cut",False,1),
 ("fm_copy","Kopieren","Copy",False,1),
 ("fm_paste","Einfügen","Paste",False,1),
 ("fm_refr","Aktualisieren","Refresh",False,1),
 ("fm_open","Öffnen","Open",False,1),
 ("fm_up","Aufwärts","Up One Level",False,1),
 ("fm_about","Info über GS/OS95","About GS/OS95",False,1),
 ("sc_n","Strg+N","Ctrl+N",False,1),
 ("sc_e","Strg+E","Ctrl+E",False,1),
 ("sc_d","Strg+D","Ctrl+D",False,1),
 ("sc_i","Strg+P","Ctrl+P",False,1),
 ("sc_esc","Esc","Esc",False,1),
 ("sc_x","Strg+X","Ctrl+X",False,1),
 ("sc_c","Strg+C","Ctrl+C",False,1),
 ("sc_v","Strg+V","Ctrl+V",False,1),
 ("sc_r","Strg+R","Ctrl+R",False,1),
 ("sc_del","Entf","Del",False,1),
 ("ti_11","Umbenennen","Rename",True,1),
 ("ti_11b","Neuer Ordner","New Folder",True,1),
 ("ti_12","Löschen bestätigen","Confirm Delete",True,1),
 ("ti_13","Eigenschaften","Properties",True,1),
 ("t_newname","Neuer Name:","New name:",False,1),
 ("t_fname","Name des Ordners:","Folder name:",False,1),
 ("t_defdir","NEUER.ORDNER","NEW.FOLDER",False,1),
 ("t_cbin","In den Papierkorb verschieben?","Move this item to the Recycle Bin?",False,1),
 ("t_cdel","Endgültig löschen?","Delete this item permanently?",False,1),
 ("t_cempty","Papierkorb wirklich leeren?","Empty the Recycle Bin?",False,1),
 ("t_yes","Ja","Yes",False,1),
 ("t_no","Nein","No",False,1),
 ("p_name","Name:","Name:",False,1),
 ("p_type","Typ:","Type:",False,1),
 ("p_size","Grösse:","Size:",False,1),
 ("p_mod","Geändert:","Modified:",False,1),
 ("p_bytes"," Bytes"," bytes",False,1),
 ("p_folder","Ordner","Folder",False,1),
 ("p_app","Programm","Application",False,1),
 ("p_text","Textdatei","Text file",False,1),
 ("p_file","Datei","File",False,1),
 ("p_src","Quelltext","Source code",False,1),
 ("p_pic","Bild","Picture",False,1),
 ("p_aw","AppleWorks-Dokument","AppleWorks document",False,1),
 ("p_font","Zeichensatz","Font",False,1),
 ("p_snd","Klang","Sound",False,1),
 ("p_nda","Schreibtischzubehör","Desk accessory",False,1),
 ("p_cda","Klassisches Zubehör","Classic desk acc.",False,1),
 ("p_cdev","Kontrollfeld","Control panel",False,1),
 ("p_init","Systemerweiterung","System extension",False,1),
 ("p_tool","Werkzeug","Tool",False,1),
 ("p_drv","Treiber","Driver",False,1),
 ("p_bin","Binärdatei","Binary file",False,1),
 ("p_bas","BASIC-Programm","BASIC program",False,1),
 ("p_sys","Systemdatei","System file",False,1),
 ("p_vol","Laufwerk","Drive",False,1),
 ("t_e5","Vorgang fehlgeschlagen. Fehlercode:","The operation failed. Error code:",False,1),
 ("t_e6","Ordner kopieren ist noch nicht möglich.","Copying folders is not supported yet.",False,1),
 ("t_e7","Bitte zuerst einen Ordner öffnen.","Please open a folder first.",False,1),
 ("t_e8","Die Zwischenablage ist leer.","The clipboard is empty.",False,1),
 ("t_busy","Kopiere...","Copying...",False,1),
 ("fm_eject","Auswerfen","Eject",False,1),
 ("sc_j","Strg+L","Ctrl+L",False,1),
 ("t_e10","Dieses Bildformat wird nicht unterstützt.","This picture format is not supported.",False,1),
 ("t_e11","Das Startlaufwerk kann nicht gelöscht werden.","The startup disk cannot be erased.",False,1),
 ("ti_11c","Datenträger löschen","Erase Disk",True,1),
 ("t_volnm","Name des Datenträgers:","Disk name:",False,1),
 ("t_cerase","Alle Daten werden gelöscht!","All data will be erased!",False,1),
 ("t_cerase2","Datenträger jetzt löschen?","Erase this disk now?",False,1),
 ("fm_erase","Datenträger löschen...","Erase Disk...",False,1),
 ("p_unf","Nicht formatiert","Unformatted",False,1),
 ("p_525","5,25-Zoll-Laufwerk","5.25-inch drive",False,1),
 ("m_zub","Zubehör","Accessories",False,1),
 ("m_dok","Dokumente","Documents",False,1),
 ("s_saver","Schoner (Min.):","Saver (min):",False,1),
 ("s_muster","Muster (0-4):","Pattern (0-4):",False,1),
 ("s_dvon","Laufwerke zeigen:","Show drives:",False,1),
 ("s_chime","Startklang:","Startup sound:",False,1),
 ("t_leerm","(leer)","(empty)",False,1),
 ("fm_prgadd","Ins Startmenü","Add to Start menu",False,1),
 ("ti_14","Zubehör","Accessories",True,1),
 ("t_e17","BASIC.SYSTEM wurde nicht gefunden.","BASIC.SYSTEM was not found.",False,1),
 ("t_e18","Der Pfad ist für ProDOS 8 zu lang.","The path is too long for ProDOS 8.",False,1),
 ("t_e19","Die Datenträger sind verschieden groß.","The disks are not the same size.",False,1),
 ("t_e20","Ungültiger Dateityp (z.B. 04 oder C1/0002).","Invalid file type (e.g. 04 or C1/0002).",False,1),
 ("t_e21","Auf dem Desktop ist kein Platz mehr.","There is no more room on the desktop.",False,1),
 ("t_e22","Das Ziel der Verknüpfung fehlt.","The shortcut target is missing.",False,1),
 ("t_e23","Im Laufwerk liegt keine lesbare Diskette.","There is no readable disk in the drive.",False,1),
 ("fm_link","Auf den Desktop","Send to desktop",False,1),
 ("t_cdc1","Gesamten Datenträger kopieren:","Copy the whole disk:",False,1),
 ("t_cdc2","auf das markierte Laufwerk?","onto the selected drive?",False,1),
 ("t_cdc3","Alle Daten dort gehen verloren!","All data there will be lost!",False,1),
 ("t_chg","Ändern","Change",False,1),
 ("ti_12b","Datenträger kopieren","Copy Disk",True,1),
 ("ti_12c","Systemordner","System Folder",True,1),
 ("ti_19b","Löschen","Deleting",True,1),
 ("ti_11e","Dateityp ändern","Change File Type",True,1),
 ("t_ftype","Neuer Dateityp (hex), z.B. C1/0002:","New file type (hex), e.g. C1/0002:",False,1),
 ("ti_15","Datum/Uhrzeit","Date/Time",True,1),
 ("ti_16","Anzeige","Display",True,1),
 ("ti_17","Maus","Mouse",True,1),
 ("ti_18","Ton","Sound",True,1),
 ("ti_19","Kopieren","Copying",True,1),
 ("ti_20","Minesweeper","Minesweeper",True,1),
 ("ti_21","Internet","Internet",True,1),
 ("ti_22","Netzwerk","Network",True,1),
 ("ti_23","CD-Wiedergabe","CD Player",True,1),
 ("ti_24","Rechner","Calculator",True,1),
 ("ti_25","Solitär","Solitaire",True,1),
 ("ti_26","Paint","Paint",True,1),
 ("ti_27","Datum/Uhrzeit","Date/Time",True,1),
 ("ti_28","Editor","Notepad",True,1),
 ("t_netz","Netzwerk","Network",False,1),
 ("t_obj7","7 Objekt(e)","7 object(s)",False,1),
 ("t_mbest","Bestzeit","Best time",False,1),
 ("t_mwon","Gewonnen!","You win!",True,1),
 ("t_mlost","Verloren! Smiley = neues Spiel","Boom! Click the smiley",True,1),
 ("t_mhint","Wahltaste + Klick setzt eine Fahne","Option-click places a flag",False,1),
 ("t_pcopy","Kopiere:","Copying:",False,1),
 ("t_pmove","Verschiebe:","Moving:",False,1),
 ("t_pdel","Lösche:","Deleting:",False,1),
 ("t_csys1","In die passenden Unterordner","Sort these items into the",False,1),
 ("t_csys2","des Systemordners einsortieren?","matching System subfolders?",False,1),
 ("t_pof"," von "," of ",False,1),
 ("t_ton","Ton","Sound",False,1),
 ("t_sprache","Deutsch","English",False,1),
 ("t_weitere","Weitere...","More...",False,1),
 ("t_obj6","6 Objekt(e)","6 object(s)",False,1),
 ("s_tag","Tag:","Day:",False,1),
 ("s_mon","Monat:","Month:",False,1),
 ("s_jahr","Jahr:","Year:",False,1),
 ("s_std","Stunde:","Hour:",False,1),
 ("s_min","Minute:","Minute:",False,1),
 ("s_desk","Desktop:","Desktop:",False,1),
 ("s_rand","Rahmen:","Border:",False,1),
 ("s_dbl","Doppelklick:","Double-click:",False,1),
 ("s_fast","Schnelle Maus:","Fast mouse:",False,1),
 ("s_vol","Lautstärke:","Volume:",False,1),
 ("s_pitch","Tonhöhe:","Pitch:",False,1),
 ("s_slow","langsam","slow",False,1),
 ("s_quick","schnell","fast",False,1),
  ("t_acc1","Klicken Sie auf ein Zubehör:","Click an accessory to open it:",False,1),
 ("t_acc0","Kein Zubehör installiert.","No accessories installed.",False,1),
 ("t_e12","Dort gibt es schon ein Objekt mit diesem Namen.","An item with this name already exists there.",False,1),
 ("t_e13","Der Datenträger ist voll.","The disk is full.",False,1),
 ("t_e14","Der Datenträger oder die Datei ist schreibgeschützt.","The disk or file is write-protected.",False,1),
 ("p_icn","Symboldatei","Icon file",False,1),
 ("p_fdat","Finder-Daten","Finder data",False,1),
 ("t_e15","Das zugehörige Programm wurde nicht gefunden.","The associated program was not found.",False,1),
 ("fm_large","Große Symbole","Large Icons",False,1),
 ("fm_details","Details","Details",False,1),
 ("fm_tree","Ordnerbaum","Folder tree",False,1),
 ("fm_sname","Nach Name ordnen","Arrange by Name",False,1),
 ("fm_ssize","Nach Größe ordnen","Arrange by Size",False,1),
 ("fm_stype","Nach Typ ordnen","Arrange by Type",False,1),
 ("t_hname","Name","Name",False,1),
 ("t_hsize","Größe","Size",False,1),
 ("t_htype","Typ","Type",False,1),
 ("t_kb"," KB"," KB",False,1),
 ("t_nobj"," markierte Objekte"," selected items",False,1),
 ("fm_dup","Duplizieren","Duplicate",False,1),
 ("fm_print","Drucken","Print",False,1),
 ("fm_find","Suchen...","Find...",False,1),
 ("fm_findn","Weitersuchen","Find Next",False,1),
 ("t_lock","Schreibgeschützt","Locked",False,1),
 ("t_x","x","x",False,1),
 ("t_find","Name enthält:","Name contains:",False,1),
 ("ti_11d","Suchen","Find",True,1),
 ("t_e16","Es wurde nichts (mehr) gefunden.","Nothing (more) was found.",False,1),
 ("t_e9","Es sind schon zu viele Fenster offen.","Too many windows are open.",False,1),
 ("t_e24","Nicht genug Speicher (64 KB am Stück).","Not enough memory (64 KB in one piece).",False,1),
]
NOTE_DE = [
    "Willkommen bei GS/OS95!",
    "",
    "Eine Oberfläche im Stil von 1995 für GS/OS.",
    "Der Arbeitsplatz zeigt deine echten Laufwerke",
    "und Ordner. Programme (S16/SYS) startest du",
    "mit zwei Klicks - danach geht es hierher zurück.",
    "",
    "Bedienung:",
    "- Icons zweimal anklicken zum Öffnen",
    "- Tab = Startmenü, Pfeile + Return, Esc = zu",
    "- Datei/Bearbeiten: Ordner, Löschen, Kopieren",
    "- Sprache: Start > Einstellungen",
]
NOTE_EN = [
    "Welcome to GS/OS95!",
    "",
    "A 1995-style desktop for GS/OS.",
    "My Computer shows your real drives and",
    "folders. Start programs (S16/SYS) with a",
    "double click - afterwards you return here.",
    "",
    "How to use it:",
    "- Double-click icons to open them",
    "- Tab = Start menu, arrows + Return, Esc = close",
    "- File/Edit menus: folders, delete, copy",
    "- Language: Start > Settings",
]
assert len(NOTE_DE) == len(NOTE_EN)
for i, (d, e) in enumerate(zip(NOTE_DE, NOTE_EN)):
    if d or e: T.append(("t_n%d" % i, d or " ", e or " ", False, 1))

def emit_text(label, text, bold, scale):
    for ch in text:
        assert ch in G, (ch, text)
    if label == "t_logo":
        # Schriftzug mit Luft zwischen den Buchstaben: "GS/OS 95"
        rows = None
        for i, ch in enumerate(text):
            g = render(ch, bold, 1)
            if rows is None: rows = [list(r) for r in g]
            else:
                gap = 4 if ch == "9" else 2
                rows = [a + [0] * gap + list(b) for a, b in zip(rows, g)]
        rows = [[v for v in r for _ in range(scale)] for r in rows for _ in range(scale)]
    else:
        rows = render(text, bold, scale)
    w, h = len(rows[0]), len(rows)
    assert w < 256, (label, w)
    emit("%s:" % label)
    data = [w, h]
    for r in rows:
        for i in range(0, w, 8):
            b = 0
            for k in range(8):
                if i + k < w and r[i + k]:
                    b |= 0x80 >> k
            data.append(b)
    bytes_line(data)

cache = {}
for name, de, en, bold, scale in T:
    emit_text(name, de, bold, scale)
    if en == de:
        emit("%s_en = %s" % (name, name))
    else:
        emit_text(name + "_en", en, bold, scale)

for lang, lst in (("", NOTE_DE), ("_en", NOTE_EN)):
    emit("note_tab%s:" % lang)
    for i, (d, e) in enumerate(zip(NOTE_DE, NOTE_EN)):
        emit("  .word %s" % (("t_n%d%s" % (i, lang)) if (d or e) else "0"))
emit("NOTE_LINES = %d" % len(NOTE_DE))

# Zeichensatz fuer dynamischen Text (ASCII 32..126)
emit("font_tab:")
for c in range(32,127):
    ch = chr(c) if chr(c) in G else '?'
    emit("  .word fg_%d" % ord(ch))
done=set()
for c in range(32,127):
    ch = chr(c) if chr(c) in G else '?'
    if ord(ch) in done: continue
    done.add(ord(ch))
    rows = render(ch)
    while len(rows) < 9: rows.append([0]*len(rows[0]))
    w=len(rows[0]); data=[w,9]
    for r in rows:
        b=0
        for k in range(w):
            if r[k]: b |= 0x80>>k
        data.append(b)
    emit("fg_%d:" % ord(ch))
    bytes_line(data)

# Umlaute und Aufzaehlungspunkt fuer dynamischen Text (Codes $80..$87)
emit("font_ext:")
for i in range(8):
    emit("  .word fx_%d" % i)
for i, ch in enumerate("äöüÄÖÜß") :
    rows = render(ch)
    while len(rows) < 9: rows.append([0]*len(rows[0]))
    w=len(rows[0]); data=[w,9]
    for r in rows:
        b=0
        for k in range(w):
            if r[k]: b |= 0x80>>k
        data.append(b)
    emit("fx_%d:" % i)
    bytes_line(data)
emit("fx_7:")
bytes_line([2,9,0,0,0,0xC0,0xC0,0,0,0,0])

# Ziffern fuer die Uhr
emit("digits:")
for d in "0123456789:":
    emit("  .word dg_%s" % ("c" if d == ':' else d))
for d in "0123456789:":
    rows = render(d)
    w, h = len(rows[0]), len(rows)
    data = [w, h]
    for r in rows:
        b = 0
        for k in range(w):
            if r[k]: b |= 0x80 >> k
        data.append(b)
    emit("dg_%s:" % ("c" if d == ':' else d))
    bytes_line(data)

# ---------------- Kartenfarben (1 Bit, Solitaer) ----------------
SUITS = {
 's_suit0': """
..##...
.####..
..##...
######.
######.
..##...
.####..""",
 's_suit1': """
...#...
..###..
.#####.
#######
.#####.
..###..
...#...""",
 's_suit2': """
.##.##.
#######
#######
.#####.
..###..
...#...
.......""",
 's_suit3': """
...#...
..###..
.#####.
#######
#######
..#.#..
.#####.""",
 's_big0': """
....###....
...#####...
...#####...
....###....
.##.###.##.
###########
###########
.##..#..##.
.....#.....
....###....
...#####...""",
 's_big1': """
.....#.....
....###....
...#####...
..#######..
.#########.
###########
.#########.
..#######..
...#####...
....###....
.....#.....""",
 's_big2': """
..##...##..
.####.####.
###########
###########
###########
.#########.
..#######..
...#####...
....###....
.....#.....
...........""",
 's_big3': """
.....#.....
....###....
...#####...
..#######..
.#########.
###########
###########
.##.###.##.
.....#.....
....###....
...#####...""",
}
for name, art in SUITS.items():
    rows = [r for r in art.strip("\n").split("\n")]
    w, h = len(rows[0]), len(rows)
    data = [w, h]
    for r in rows:
        for i in range(0, w, 8):
            b = 0
            for k in range(8):
                if i + k < w and r[i + k] == '#': b |= 0x80 >> k
            data.append(b)
    emit("%s:" % name)
    bytes_line(data)

# ---------------- 4bpp Icons ----------------
CM = {'.': 16, 'k': 0, 't': 12, 'n': 13, 'd': 1, 'g': 14, 'w': 15, 'y': 9, 'r': 7, 'G': 5, 'b': 4, 'c': 11, 'o': 6, '1': 2, '2': 3, '3': 8, '4': 10}
ICONS = {}
ICONS['i_comp'] = """
................
.kkkkkkkkkkkkk..
.kwwwwwwwwwwwdk.
.kwkkkkkkkkkgdk.
.kwkcccccccbgdk.
.kwkcwccccbbgdk.
.kwkccccccbbgdk.
.kwkcccccbbbgdk.
.kwkbbbbbbbbgdk.
.kwgggggggGggdk.
.kkkkkkkkkkkkkk.
....kdddddk.....
..kkkkkkkkkkk...
.kwwwwwwwwwwgdk.
.kggggggggrggdk.
.kkkkkkkkkkkkkk.
"""
ICONS['i_bin'] = """
.....kkkkkk.....
..kkkwwwwwwkkk..
.kwwwwggggggddk.
.kkkkkkkkkkkkkk.
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
..kwgwdgwdgwdk..
...kwgdgwdgdk...
...kkkkkkkkkk...
................
"""
ICONS['i_doc'] = """
..kkkkkkkkk.....
..kwwwwwwwkk....
..kwkkkkwwkwk...
..kwwwwwwwkkkk..
..kwkkkkkkwwwk..
..kwwwwwwwwwwk..
..kwkkkkkkkkwk..
..kwwwwwwwwwwk..
..kwkkkkkkkwwk..
..kwwwwwwwwwwk..
..kwkkkkkkkkwk..
..kwwwwwwwwwwk..
..kwkkkkkwwwwk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_folder'] = """
................
................
..kkkkkk........
.kwyyyyyk.......
.kyyyyyyykkkkkk.
.kwwwwwwwwwwwwyk
.kwyyyyyyyyyyyok
.kwyyyyyyyyyyyok
.kwyyyyyyyyyyyok
.kwyyyyyyyyyyyok
.kwyyyyyyyyyyyok
.kwyyyyyyyyyyyok
.kyooooooooooook
.kkkkkkkkkkkkkkk
................
................
"""
ICONS['i_floppy'] = """
................
.kkkkkkkkkkkkk..
.kdkgggggggkdk..
.kdkgggkkggkdk..
.kdkgggkkggkdk..
.kdkgggggggkdk..
.kddkkkkkkkddk..
.kdddddddddddk..
.kddwwwwwwwwdk..
.kddwkkkkkkwdk..
.kddwwwwwwwwdk..
.kddwkkkkkwwdk..
.kddwwwwwwwwdk..
.kkkkkkkkkkkkk..
................
................
"""
ICONS['i_hdd'] = """
................
................
................
................
................
.kkkkkkkkkkkkkk.
.kwwwwwwwwwwwwdk
.kwggggggggggddk
.kwgdddddddgGddk
.kwggggggggggddk
.kdddddddddddddk
.kkkkkkkkkkkkkkk
................
................
................
................
"""
ICONS['i_printer'] = """
................
....kkkkkkkk....
....kwwwwwwk....
....kwkkkkwk....
....kwwwwwwk....
..kkkkkkkkkkkk..
.kwwwwwwwwwwwdk.
.kwggggggggGgdk.
.kwggggggggggdk.
.kdddddddddddk..
..kwwwwwwwwwk...
..kwkkkkkkkwk...
..kwwwwwwwwwk...
..kkkkkkkkkkk...
................
................
"""
ICONS['i_mouse'] = """
.......kk.......
........k.......
.......kk.......
.....kkkkkk.....
....kwwkwwwk....
....kwwkwwwk....
....kwwkwwwk....
....kkkkkkkk....
....kwwwwwwk....
....kwwwwwgk....
....kwwwwwgk....
....kwwwwwgk....
.....kwwwgk.....
......kkkk......
................
................
"""
ICONS['i_clock'] = """
................
.....kkkkkk.....
...kkwwwwwwkk...
..kwwwwkwwwwwk..
..kwwwwkwwwwwk..
.kwwwwwkwwwwwwk.
.kwwwwwkwwwwwwk.
.kwkwwwkkkkwkwk.
.kwwwwwwwwwwwwk.
.kwwwwwwwwwwwwk.
..kwwwwwwwwwwk..
..kwwwwwkwwwwk..
...kkwwwwwwkk...
.....kkkkkk.....
................
................
"""
ICONS['i_help'] = """
................
...kkkkkkkkkk...
..kbbbbbbbbbbk..
..kbbbwwwbbbbk..
..kbbwbbbwbbbk..
..kbbbbbbwbbbk..
..kbbbbbwbbbbk..
..kbbbbwbbbbbk..
..kbbbbwbbbbbk..
..kbbbbbbbbbbk..
..kbbbbwbbbbbk..
..kbbbbbbbbbbk..
..kkkkkkkkkkkk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_run'] = """
................
................
.kkkkkkkkkkkkk..
.knnnnnnnnnnnk..
.kkkkkkkkkkkkk..
.kwwwwwwwwwwgk..
.kwwwwwwwwwwgk..
.kwwwwkwwwwwgk..
.kwwwwkkwwwwgk..
.kwkkkkkkwwwgk..
.kwwwwkkwwwwgk..
.kwwwwkwwwwwgk..
.kwwwwwwwwwwgk..
.kgggggggggggk..
.kkkkkkkkkkkkk..
................
"""
ICONS['i_ctrl'] = """
................
.kkkkkkkkkkkkk..
.kwwwwwwwwwwwk..
.kwrrkwyykwGGk..
.kwrrkwyykwGGk..
.kwkkkwkkkwkkk..
.kwwwwwwwwwwwk..
.kwbbkwcckwook..
.kwbbkwcckwook..
.kwkkkwkkkwkkk..
.kwwwwwwwwwwwk..
.kkkkkkkkkkkkk..
................
................
................
................
"""
ICONS['i_startlogo'] = """
.kkkkkkk.
krrrrrrrk
kooooooo k
kyyyyyyyk
kGGGGGGGk
kbbbbbbbk
.kkkkkkk.
"""
ICONS['i_tfold'] = """
.kkk.....
kyyykkkk.
kwwwwwwwk
kwyyyyyok
kwyyyyyok
kyooooook
kkkkkkkkk
"""
ICONS['i_tdisk'] = """
.........
kkkkkkkkk
kgggggggk
kgggggGgk
kddddddnk
kkkkkkkkk
.........
"""
ICONS['i_qdesk'] = """
kkkkkkkkk
kccccccck
kccccccck
kccccccck
kccccccck
kccccccck
kggggggnk
kkkkkkkkk
.........
"""
ICONS['i_qmine'] = """
....k....
.k.kkk.k.
..kkkkk..
.kkwkkkk.
kkkkkkkkk
.kkkkkkk.
..kkkkk..
.k.kkk.k.
....k....
"""
ICONS['i_smile'] = """
...kkkkk...
..kyyyyyk..
.kyyyyyyyk.
kyykyyykyyk
kyykyyykyyk
kyyyyyyyyyk
kykyyyyykyk
kyykyyykyyk
.kyykkkyyk.
..kyyyyyk..
...kkkkk...
"""
ICONS['i_smdead'] = """
...kkkkk...
..kyyyyyk..
.kyyyyyyyk.
kykykykykyk
kyykyyykyyk
kykykykykyk
kyyyyyyyyyk
kyyykkkyyyk
.kykyyykyk.
..kyyyyyk..
...kkkkk...
"""
ICONS['i_smcool'] = """
...kkkkk...
..kyyyyyk..
.kyyyyyyyk.
kkkkkkkkkkk
kykkkykkkyk
kyykyyykyyk
kykyyyyykyk
kyykyyykyyk
.kyykkkyyk.
..kyyyyyk..
...kkkkk...
"""
ICONS['i_qterm'] = """
kkkkkkkkk
kgggggggk
kgkkkkkgk
kgk4kkkgk
kgkk4kkgk
kgk4k44gk
kgkkkkkgk
kgggggggk
kkkkkkkkk
"""
ICONS['i_mprg'] = """
kkkkkkkkk
knnnnnnwk
kkkkkkkkk
kwwwwwwwk
kwgggggwk
kwwwwwwwk
kwgggwwwk
kwwwwwwwk
kkkkkkkkk
"""
ICONS['i_mcalc'] = """
kkkkkkkk.
kccccccdk
kddddddk.
kwkwkwk..
kkkkkkk..
kwkwkrk..
kkkkkkk..
kwkwkrk..
kkkkkkk..
"""
ICONS['i_mcd'] = """
..kkkkk..
.kgwgggk.
kgwgggggk
kgggkkggk
kggkwkggk
kgggkkggk
kgggggwgk
.kgggwgk.
..kkkkk..
"""
ICONS['i_marc'] = """
.kkkkkkk.
kyyyyyyyk
kkkkkkkkk
kwwwwwwwk
kwwbbbwwk
kwwwbwwwk
kwwwwwwwk
kddddddnk
kkkkkkkkk
"""
ICONS['i_mnews'] = """
kkkkkkkk.
kwwwwwwkk
kwkkkkwkk
kwwwwwwkk
kwkkwkkkk
kwwwwwwkk
kwkkwkkkk
kwwwwwwk.
kkkkkkkk.
"""
ICONS['i_mdos'] = """
kkkkkkkkk
knnnnnnnk
kkkkkkkkk
kwkkkkkkk
kkwkkkkkk
kwkkkkkkk
kkkkwwwkk
kkkkkkkkk
kkkkkkkkk
"""
ICONS['i_mpaint'] = """
.......kk
......kyk
.....kyk.
....kyk..
.rrkyk...
rrrkk....
rrbb.....
.bbgg....
..gg.....
"""
ICONS['i_ppen'] = """
.......kk
......kok
.....kok.
....kok..
...kok...
..kok....
..kk.....
.k.......
.........
"""
ICONS['i_pbrush'] = """
.......d.
......dd.
.....dd..
....dd...
...kk....
..kkk....
.kkk.....
.kk......
.........
"""
ICONS['i_perase'] = """
.........
....kkkkk
...krrrrk
..krrrrkk
.kwwwwk.k
.kwwwk.k.
.kkkkkk..
.........
.........
"""
ICONS['i_pspray'] = """
.k.k.....
k.k.k....
.k.k..kk.
.....kddk
.....kddk
....kdddk
....kdddk
....kdddk
....kkkkk
"""
ICONS['i_pline'] = """
k........
.k.......
..k......
...k.....
....k....
.....k...
......k..
.......k.
........k
"""
ICONS['i_prect'] = """
.........
kkkkkkkkk
k.......k
k.......k
k.......k
k.......k
k.......k
kkkkkkkkk
.........
"""
ICONS['i_pfill'] = """
.........
kkkkkkkkk
kbbbbbbbk
kbbbbbbbk
kbbbbbbbk
kbbbbbbbk
kbbbbbbbk
kkkkkkkkk
.........
"""
ICONS['i_pbuck'] = """
...k.....
..k.k....
.k...k...
k.....k..
.k...kbb.
..k.k.bbb
...k...bb
........b
.........
"""
ICONS['i_pell'] = """
..kkkkk..
.k.....k.
k.......k
k.......k
k.......k
k.......k
k.......k
.k.....k.
..kkkkk..
"""
ICONS['i_pellf'] = """
..kkkkk..
.kbbbbbk.
kbbbbbbbk
kbbbbbbbk
kbbbbbbbk
kbbbbbbbk
kbbbbbbbk
.kbbbbbk.
..kkkkk..
"""
ICONS['i_ppick'] = """
......kk.
.....kkkk
......kk.
.....k.k.
....k.k..
...k.k...
..k.k....
.kkk.....
k........
"""
ICONS['i_ptext'] = """
...kkk...
...kkk...
..kk.kk..
..kk.kk..
.kk...kk.
.kkkkkkk.
kk.....kk
kk.....kk
.........
"""
ICONS['i_medit'] = """
.kkkkkkk.
kwkwkwkwk
kwwwwwwwk
kwkkkkkwk
kwwwwwwwk
kwkkkkwwk
kwwwwwwwk
kwkkkkkwk
kkkkkkkkk
"""
ICONS['i_mwx'] = """
..y......
y.y.y....
.yyy.....
yyyyyggg.
.yyygwwwg
y.ygwwwwg
..gwwwwwg
..ggggggg
.........
"""
ICONS['i_msol'] = """
kkkkkk...
kwwwwwk..
kwrwkkkkk
kwrrwwwwk
kwwrkwkwk
kkkkwkkwk
...kwwkwk
...kwwwwk
...kkkkkk
"""
ICONS['i_qweb'] = """
..kkkkk..
.kbbGbbk.
kbGGGbbbk
kbbGGbbbk
kbbbbGGbk
kbbbGGGbk
kbbbbGbbk
.kbbbbbk.
..kkkkk..
"""
ICONS['i_net'] = """
................
.kkkkkk.........
.kcccck.........
.kcccck.........
.kkkkkk.........
...kk...........
..kkkk..kkkkkk..
....k...kcccck..
....kkkkkcccck..
........kkkkkk..
..........kk....
.........kkkk...
................
................
................
................
"""
ICONS['i_qcomp'] = """
kkkkkkkkk
kcccccbbk
kccccbbbk
kcccbbbbk
kkkkkkkkk
...kkk...
.kkkkkkk.
.kgggggk.
.kkkkkkk.
"""
ICONS['i_qset'] = """
kkkkkkkkk
kwwwwwwwk
kwrwwbwwk
kwrwwbwwk
kwwwwbwwk
kwGwwwwwk
kwGwyywwk
kwwwwwwwk
kkkkkkkkk
"""
ICONS['i_qrun'] = """
kkkkkkkkk
knnnnnnnk
kwwwwwwwk
kwkwwwwwk
kwkkwwwwk
kwkkkwwwk
kwkkwwwwk
kwkwwwwwk
kkkkkkkkk
"""
ICONS['i_app'] = """
................
.kkkkkkkkkkkkkk.
.knnnnnnnnnngwk.
.kkkkkkkkkkkkkk.
.kwwwwwwwwwwwgk.
.kwyywwrrwwwwgk.
.kwyywwrrwwGGgk.
.kwwwwwwwwwGGgk.
.kwbbwwwwwwwwgk.
.kwbbwwccccwwgk.
.kwwwwwccccwwgk.
.kwwwwwwwwwwwgk.
.kggggggggggggk.
.kkkkkkkkkkkkkk.
................
................
"""
ICONS['i_cursor'] = """
k..........
kk.........
kwk........
kwwk.......
kwwwk......
kwwwwk.....
kwwwwwk....
kwwwwwwk...
kwwwwwwwk..
kwwwwwwwwk.
kwwwwwkkkkk
kwwkwwk....
kwk.kwwk...
kk..kwwk...
k....kwwk..
.....kwwk..
......kk...
"""
ICONS['i_close'] = """
kk....kk
.kk..kk.
..kkkk..
...kk...
..kkkk..
.kk..kk.
kk....kk
"""

ICONS['i_blank'] = """
..kkkkkkkkk.....
..kwwwwwwwkk....
..kwwwwwwwkwk...
..kwwwwwwwkkkk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_pic'] = """
................
.kkkkkkkkkkkkkk.
.kwwwwwwwwwwwwk.
.kwccccccccyywk.
.kwcccccccyyywk.
.kwccccccccyywk.
.kwccccccccccwk.
.kwcccGccccccwk.
.kwccGGGcccGcwk.
.kwcGGGGGcGGGwk.
.kwGGGGGGGGGGwk.
.kwGGGGGGGGGGwk.
.kwwwwwwwwwwwwk.
.kkkkkkkkkkkkkk.
................
................
"""
ICONS['i_sys'] = """
..kkkkkkkkk.....
..kwwwwwwwkk....
..kwwwwwwwkwk...
..kwwwwwwwkkkk..
..kwwwkkwwwwwk..
..kwkwkddkwkwk..
..kwwkddddkwwk..
..kwkddwwddkwk..
..kwkddwwddkwk..
..kwwkddddkwwk..
..kwkwkddkwkwk..
..kwwwkkwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_font'] = """
..kkkkkkkkk.....
..kwwwwwwwkk....
..kwwwwwwwkwk...
..kwwwwwwwkkkk..
..kwwwwnnwwwwk..
..kwwwwnnwwwwk..
..kwwwnwwnwwwk..
..kwwwnwwnwwwk..
..kwwnnnnnnwwk..
..kwwnwwwwnwwk..
..kwnnwwwwnnwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_snd'] = """
..kkkkkkkkk.....
..kwwwwwwwkk....
..kwwwwwwwkwk...
..kwwwwwwwkkkk..
..kwwwwwkkkwwk..
..kwwwkkkwkwwk..
..kwwwkwwwkwwk..
..kwwwkwwwkwwk..
..kwwwkwwwkwwk..
..kwwwkwwkkwwk..
..kwwkkwkkkwwk..
..kwkkkwkkwwwk..
..kwkkwwwwwwwk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_aw'] = """
..kkkkkkkkk.....
..knnnnnnnkk....
..knwwnwwnkwk...
..knnnnnnnkkkk..
..kwwwwwwwwwwk..
..kwkkkkkkkkwk..
..kwwwwwwwwwwk..
..kwkkkkkkkwwk..
..kwwwwwwwwwwk..
..kwkkkkkkkkwk..
..kwwwwwwwwwwk..
..kwkkkkkwwwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_binf'] = """
..kkkkkkkkk.....
..kwwwwwwwkk....
..kwwwwwwwkwk...
..kwkwwkwkkkkk..
..kwkwkwkwkwwk..
..kwkwkwkwkwwk..
..kwkwwkwkwwwk..
..kwwwwwwwwwwk..
..kwwkwkwwkwwk..
..kwkkwkwkwkwk..
..kwwkwkwkwkwk..
..kwwkwkwwkwwk..
..kwwwwwwwwwwk..
..kwwwwwwwwwwk..
..kkkkkkkkkkkk..
................
"""
ICONS['i_bas'] = """
................
.kkkkkkkkkkkkkk.
.knnnnnnnnnnnnk.
.kkkkkkkkkkkkkk.
.kwwwwwwwwwwwwk.
.kwkkwwkwwkkwwk.
.kwkwkkwkwkwwwk.
.kwkkwkkkwkkwwk.
.kwkwkkwkwwkwwk.
.kwkkwkwkwkkwwk.
.kwwwwwwwwwwwwk.
.kwGGGGwwwwwwwk.
.kwwwwwwwwwwwwk.
.kkkkkkkkkkkkkk.
................
................
"""
for name, s in ICONS.items():
    rows = [r.replace(' ', '') for r in s.strip('\n').split('\n')]
    w = max(len(r) for r in rows)
    rows = [r.ljust(w, '.') for r in rows]
    h = len(rows)
    emit("%s_W = %d" % (name.upper(), w))
    emit("%s_H = %d" % (name.upper(), h))
    emit("%s:" % name)
    data = [w, h] + [CM[c] for r in rows for c in r]
    bytes_line(data)

open('assets.inc', 'w').write("\n".join(out) + "\n")
print("ok", len(out))

# vertikaler Banner-Text fuer das Startmenue (90 Grad gegen Uhrzeigersinn)
rows = render("GS/OS95", True, 1)
h0, w0 = len(rows), len(rows[0])
rot = [[rows[r][w0 - 1 - c] for r in range(h0)] for c in range(w0)]
w, h = len(rot[0]), len(rot)
out2 = ["T_VERT_W = %d" % w, "T_VERT_H = %d" % h, "t_vert_en = t_vert", "t_vert:"]
data = [w, h]
for r in rot:
    b = 0
    for k in range(w):
        if r[k]: b |= 0x80 >> k
    data.append(b)
for i in range(0, len(data), 16):
    out2.append("  .byte " + ",".join("$%02X" % b for b in data[i:i+16]))
open('assets.inc', 'a').write("\n".join(out2) + "\n")

# Startklang: aufsteigender Dreiklang (C5-E5-G5-C6), 11025 Hz, 8 Bit ohne Nullbytes
import math
SR = 11025
notes = [(523.25, 0.00), (659.25, 0.12), (783.99, 0.24), (1046.5, 0.36)]
n = int(SR * 0.95)
n = (n + 255) // 256 * 256
wave = []
for i in range(n):
    t = i / SR
    v = 0.0
    for f, t0 in notes:
        if t >= t0:
            d = t - t0
            env = math.exp(-d * 4.0) * min(1.0, d * 200)
            v += env * (math.sin(2 * math.pi * f * d) + 0.3 * math.sin(4 * math.pi * f * d))
    fade = min(1.0, (n - i) / 400.0)
    s = int(128 + 40 * v * fade)
    wave.append(max(1, min(255, s)))
out3 = ["SNDPAGES = %d" % (n // 256), "snd_wave:"]
for i in range(0, len(wave), 16):
    out3.append("  .byte " + ",".join("$%02X" % b for b in wave[i:i+16]))
open('assets.inc', 'a').write("\n".join(out3) + "\n")

# Maskentabelle fuer schnelle 1-Bit-Glyphen: Byte v (Bit 7 = Pixel 0) -> 4 Bildschirmbytes
out4 = ["gmask:"]
data = []
for v in range(256):
    for j in range(4):
        m = (0xF0 if v & (0x80 >> (2 * j)) else 0) | (0x0F if v & (0x40 >> (2 * j)) else 0)
        data.append(m)
for i in range(0, len(data), 16):
    out4.append("  .byte " + ",".join("$%02X" % b for b in data[i:i+16]))
open('assets.inc', 'a').write("\n".join(out4) + "\n")
