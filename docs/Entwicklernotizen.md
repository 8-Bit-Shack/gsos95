# GS/OS95 Netzwerk & Oberfläche – Stand v2.18.0 (26.11.2026, Build 2.1.45, Image GSOS95_v2145_System.hdv)

## Arbeitsordner (seit 26.11.2026)
- **Nur noch** im Mac-Ordner `…/CrossAssembler/GSOS95/claude/gsos95` arbeiten. Das ist der Git-Klon von https://github.com/8-Bit-Shack/gsos95 (origin). Inhalt: `README.md` (englisch, ausführlich), `.gitignore`, `src/` (Quellen, Skripte, Tests), `image/` (aktuelles Image als Grundlage für den nächsten Build), `docs/` (Handbuch.md, Manual.md, Screenshots, LIESMICH, README_EN, diese Notizen).
- Gebaut wird in der Cloud-Arbeitsumgebung (cc65 fehlt auf dem Mac-Teil): Spiegel in /home/claude/repo, Arbeitskopie /home/claude/gs95. Sitzungsbeginn: fehlt die Cloud-Kopie, `src/` aus dem Ordner holen. Nach jeder Änderung: geänderte Dateien in `src/`, `docs/`, neues Image nach `image/` zurückschreiben (altes Image nach `_to_delete/`, nichts löschen). Kein git commit/push ohne Auftrag. Pfade: `CC65_BIN`, `GSOS95_IMG`.
- Update-Adresse für about:update wäre https://raw.githubusercontent.com/8-Bit-Shack/gsos95/main/… (noch nicht eingetragen).

## Aufbau
- **Fünf Codesegmente** (`mkomf2.py`): 1 Hauptbank (CODE/RODATA/BSS), 2 Grafik + XCODE, 3 XCODE2 (Browser, Startmenü, Texte), 4 XCODE3 (Netzwerk, Terminal, Rechner, Solitär), **5 XCODE4 (`x4.s`)**, Segment 6 = Direct Page/Stack 16 KB.
- Fernaufrufe: `JSLF xf_...` (Ziel in der Liste FXA..FXE je nach FARMODE 0..4), Hauptbank-Routinen mit `CALL` (Sprungstellen `tp_` in tramp.s). `farcheck.py` prüft alle XCODE-Bänke. Test-Lader t3.boot_app: Segment 5 bei $30:0000.
- **Platz (Build 2.1.44):** Hauptbank ~1,1 KB frei (38761 + BSS 24676 von 64512), Grafik ~0,4 KB (deskpat.s liegt jetzt in X4), XCODE2 ~1,2 KB, XCODE3 ~3,8 KB, **XCODE4 ~42 KB**. Neue Programme gehören nach XCODE4; große Variablen dort nicht ins BSS (Hauptbank!), sondern per NewHandle.
- Treiberwahl über `NCARD` (1 Uthernet II, 2 CoGS, 3 FujiNet, 4 Marinetti, 5 WLAN-Modem). TLS: CoGS für jede TCP-Verbindung; FujiNet `N:HTTPS`, **SSH nur FujiNet** (`N:SSH://`, Anmeldung CONTROL $FD Benutzer / $FE Passwort).
- **Zweiter TCP-Socket (2.1.44, nur Uthernet II):** `NSOCK`=1 → tcp_sel nimmt Socket 1 (S1/TX1/RX1, 2 KB; sonst UDP für DNS/DHCP/NTP). tcp_close mit NSOCK=1 schließt Socket 1 und danach Socket 0, setzt NSOCK=0. Basen über WTXB/WRXB.
- Fallen: `lda f:label` der eigenen Bank wird nicht verschoben → `TBL`-Makro; `jmp (tab,x)` liest aus der Programmbank. Keine Mnemonics als Labels, kurze Namen (DI, PT, PX0 …) oft vergeben. ca65 `.smart` folgt sep/rep nur geradeaus → an Sprungzielen im 8-Bit-Teil `.a8`. `python3 tools/fixbr.py` für zu weite Sprünge. Modale GetNextEvent-Schleifen mit Maske $FFFF und Typprüfung.
- **Zeichnen im Fenster-Neuzeichnen geht in einen Puffer** (windows.s, OFFON): Ziel immer über `linetab` + Bank in `PTR+2`. Im Klick-Handler nicht `orgr` rufen, nur KL/KR/KT/KB setzen.
- Per-Fensterart-Tabellen in app.s: `titles`, `titles_en`, `wincont` (bei neuer Art alle verlängern!), Verteiler in net.s (xf_wc_ndlg / xf_ndlg_click / xf_ndlg_key, Art ≥ 22).
- Schrift der Oberfläche: Umlaute ä $80, ö $81, ü $82, Ä $83, Ö $84, Ü $85, ß $86 (in HTML-Seiten Entities nehmen). Vollbild-Terminal: CP437.

## Startmenü (`startsub.s`)
- Programme = Gruppen Werkzeuge (Rechner, CD-Wiedergabe, MS-DOS, Paint), Internet (Internet, Terminal, Software-Archiv, Nachrichten, **Wetter**), Spiele (Minesweeper, Solitär) + Arbeitsplatz + eigene Programme. `sg_n 4,5,2`, `sg_first 0,4,9`, Sprungtabelle in s2_action in derselben Reihenfolge.

## Programme
- Rechner (`calc.inc`, Art 24), Solitär (`sol.inc`, Art 25) in XCODE3.
- **MS-DOS-Eingabeaufforderung** (`dos.inc` + `dosnet.inc`, XCODE4): DIR/CD/TYPE/CLS/COLOR/DATE/TIME/MEM/VER/ECHO/HELP/EXIT, **PING** (4× TCP-Connect Port 80, Zeit per GetTick), **NSLOOKUP**, **IPCONFIG/WINIPCFG** (Karte, IP/Maske/Gateway/DNS, DHCP, MAC bei Uthernet II), **FINGER** (Port 79). Texte in mkwebstr.py (DS_*). Tests tdos.py, tdosnet.py.
- **SSH** (`ssh.inc`, XCODE4) über FujiNet. Test tssh.py.
- **Paint** (`paint.inc`, Art 26, XCODE4): Werkzeuge 0–11 (Stift, Pinsel, Radierer, Sprühdose, Linie, Rechteck, gefüllt, Füllen, Ellipse, gefüllte Ellipse, Pipette, Text), S/Z/N/Pfeile/Esc-Rückfrage. Bildbetrachter B → xf_pt_screen. **Browser Strg-P → xf_pt_img**: erstes Bild der Seite (bzw. GIF-Seite) aus IMBK (1 Byte/Punkt, Schreibtischfarben) ins Paint-Dokument (PDONLY: pt_pset nur ins Bild). Tests tpaint.py, timg.py.
- **Bildschirmschoner**: Linienspiel (`ssv.inc`) und Sternenfeld; Standard aus, Systemsteuerung > Anzeige „Schoner (Min.)“ 1–15.
- **Bluescreen** (`bsod.inc`): Wahl-Apfel-C oder Strg-Apfel-C. Test tbsod.py.

## Browser
- **Lesezeichen/Verlauf/Suche** (`lz.inc`, XCODE4): Datei *:System:GSOS95.LZ (Handle, 48 Einträge à 192 Bytes: Ordner 1+23, Titel 1+47, URL 1+119; Verlauf 20 Einträge ab $2500; alte 8 Favoriten werden übernommen). about:lz (Ordner, merken, [x] löschen = about:lzdel=N, Suchmaschine about:lzs=N), about:verlauf, Strg-D = xf_lz_add, his_push → xf_hist_add. Suchmaschine `SRCHE` (NPREF+1960): 1 FrogFind, 2 wiby, 3 DuckDuckGo (url.inc u_stab). Test tlz.py.
- **Wetter**: about:wetter[?ort=X] (xf_wx_cmd), Ort in WXORT (NPREF+1961), Weiterleitung auf http(s)://wttr.in/<Ort>?0T&lang=de.
- **FTP** (`ftp.inc`, XCODE4; USCH=7, Port 21): web_load → xf_ftp_pre (Kartenprüfung; FujiNet: FNFTP 1 Datei/2 Verzeichnis) → normales Öffnen → ftp_go (web.s) → xf_ftp_run (Uthernet II: USER anonymous/PASS gs95@/TYPE I/CWD/PASV, Daten auf Socket 1, LIST oder RETR; Antworten in WB_REQ+$100). Verzeichnis → xf_ftp_list (Unix-, DOS/IIS- und reine Namenslisten → HTML ab $4000 mit absoluten ftp://-Links), Datei → dl_run (*:Downloads). FujiNet: `N:FTP://host/pfad`, Modus 6 + aux2 $80 für Verzeichnisse, Modus 4 für Dateien. PASV-Adresse wird ignoriert (NAT), nur der Port zählt. Andere Karten: Hinweisseite. Tests tftp.py (W5100-Sim mit FTP-Server), tftpf.py, tftpm.py.
- `pfilt.inc`: about:archiv, about:rss. `xfer.inc`: ZMODEM/XMODEM/YMODEM.

## Ladedatei (seit 2.1.45)
- `mkomf2.py` schreibt die Verschiebe-Einträge als SUPER-Datensätze ($F7: RELOC2/RELOC3, INTERSEG1 mit Segmentnummer im 3. Byte der Stelle, INTERSEG13+n für 2-Byte-Verweise). Datei 278 statt 428 KB. Alte Form: `OMF_OLD=1 python3 mkomf2.py`. t3.load_omf kann beide; Vergleich alt/neu bei mehreren Ladeadressen ergab identischen Speicher.

## Startbild (seit 2.1.41)
- Wolken bis zum Balken, Balkenzeilen 320er-Modus Palette 0, ab Zeile 172 eine Farbe. boot_probe und SPLASH95 erkennen Palette 0 + linken Balkenrahmen. Nach BUILD-Wechsel `./mkboot61.sh`, Image: update_app.py + tools/update_file.py für SYSTEM/START.GS.OS, GSOS95.SPL, BOOT95, SYSTEM.SETUP/SPLASH95.

## Tests
- tdos, tdosnet, tlz, timg, tftp, tftpf, tftpm, tssh, tpaint, tssv, tbsod, twa, twx, twf, tcalc, tsol, tsol2, tearly (Image-Pfad im Skript), t69, t42, tw-Serie, tcd*, twu. Bildschirm-Koordinaten in Tests: x = Pixel/3, y = Pixel/3,6 der gerenderten PNGs.

## Offen
- Update-Adresse im Programm eintragen (Repo 8-Bit-Shack/gsos95). Hardware-Tests: FTP mit echter Uthernet II/FujiNet (Format der FujiNet-Verzeichnisliste ungeprüft), SSH, Paint, CD, Feeds, ZMODEM, GetBootVol.
- FTP für CoGS/Marinetti (zweite Verbindung) fehlt noch.
- Ideen: gelegentliche Fake-Abstürze (abschaltbar, Standard aus), ProDOS-Eintrag im Startmenü, FreeCell, Kalender, E-Mail (CoGS), NNTP.
