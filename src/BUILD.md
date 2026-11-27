# GS/OS95 – Quelltexte (Stand v2.18.0)

Autor: Marc Golombeck. 65816-Assembler fuer ca65/ld65 (cc65), Hilfsskripte in Python 3.

## Aufbau
- `app.s` (+ alle eingebundenen `*.s`): die Oberflaeche GS/OS95 (SYSTEM:GSOS95, Typ $B3)
  - drei Code-Segmente: Hauptbank, Grafik, XCODE2 (Fernaufrufe ueber `JSLF xf_*` / `tramp.s`)
  - `farcheck.py` prueft: im XCODE nur jsr/jmp zu Labels derselben Datei
- `init95/`: das Systemmodul GSOS95.INIT (Typ $B6, *:System:System.Setup)
  - `init95.s` (Fensterrahmen, Dispatcher-Haken), `sf95.s` (Oeffnen/Speichern), `ctl95.s` (Knoepfe, Ankreuzfelder)
  - `ti1.py`, `ti2.py`, `ti3.py`: Tests im 65816-Simulator (`emu.py`, `t3.py`)
- `net.s` (+ `netdlg.inc`, `cogs.inc`, `ntp.inc` Uhrzeit per SNTP/Zeitzone in NPREF+3): liegt in der vierten Codebank (XCODE3, FARMODE 3,
  Fernaufrufe FXD). Treiber nach NCARD (Sprungtabellen in tcp_open/write/readall/poll/close), Faehigkeiten in NCAPS
  (CAP_NAME, CAP_TLS, CAP_UDP, CAP_DNS), Kartenwahl NCSEL (NPREF+1536, Modem: +1537 Anschluss, +1538 Baudrate):
  - Uthernet II / W5100 (DHCP, DNS mit Zwischenspeicher, TCP) und CoGS
  - `mari.inc` Marinetti (Toolset $36), `fujinet.inc` FujiNet (SmartPort ueber FWEntry, N:TCP / N:HTTPS),
    `modem.inc` WLAN-Modem (SCC 8530 abgefragt, Hayes ATDT)
  - Texte (webstr.inc) liegen in XCODE2; net.s findet ihre Bank in STRBK (xf_strbk beim Start)
  - `ntp.inc`: Uthernet II per SNTP, andere Karten per Time-Dienst (TCP 37, time.nist.gov)
- `x3ext.inc` (in net.s): Explorer-Ergaenzungen v2.14 (Aufwaerts markiert Herkunft, Papierkorb leeren mit Fortschritt,
  Systemordner einsortieren, Klangdateien $D8 abspielen), `keyhtml.inc` (about:keys, aus mkwebstr.py)
- `cd.inc` (in net.s): CD-Wiedergabe, Fensterart 23 (Start > Programme, Diskettenabfrage bei neuer Audio-CD,
  Doppelklick auf unlesbares CD-Laufwerk). SCSI-Befehle per DStatus/DControl $80xx an SCSICD.DRIVER:
  SCSI-2 ($8043/$8042/$8047/$804B/$801B) oder Apple CD SC/150 ($80C1/$80CC/$80C2/$80C8/$80CA/$80CB), automatisch.
  Logik (cdl_*) laeuft mit Datenbank = XCODE3 (Variablen dort, Makro LG), Oberflaeche mit Hauptbank (Lesen ueber cdg).
  Zeichnen/Klicks/Tasten ueber die Einspruenge von Fensterart 22 (xf_wc_ndlg usw. verzweigen nach WIN).
  Treiber im Image: SCSICD.DRIVER aus BlueSCSI.2mg (Brutal Deluxe, 08/2026; Befehlstabelle: bei mehrfachen Eintraegen wird der naechste versucht),
  dazu `python3 tools/patch_scsicd.py drv/SCSICD.DRIVER.bd2026 drv/SCSICD.DRIVER` (ASC $3A = keine CD wie $B0: offline statt E/A-Fehler,
  keine Endlosschleife nach CD-Wechsel). Diskettenabfrage fragt CD-Laufwerke mit TEST UNIT READY (`xf_cd_tur`).
  Test: `python3 tcd.py` (cdsim.py: SCSI-2- und Apple-Laufwerk), `python3 tcd0.py` (ohne Laufwerk)
- `cddb.inc` (in cd.inc): CD-Titelnamen per CDDB/HTTP (Disc-ID, query/read, Latin-1/UTF-8), Anfrage NWB:$7000,
  Antwort NWB:0; Server/E-Mail in GSOS95.NET +1700/+1764 (about:cddb in fav.inc). Test: `python3 tcddb.py`
- System 6.1 (nur Fehlerkorrekturen): `python3 tools/apply61.py <GSOS61_HardDisk_32MB-2.2mg> <image> sys/START.GS.OS.95`
  kopiert die 6.1-Systemdateien (Liste FILES) ins Image und stellt an den neuen Battery-RAM-Abfragen ($5E Look, $5F Flamme)
  fest "Standard" ein (Liste LOOK). Startbild: `python3 boot95/patch.py sys61/START.GS.OS.61 sys/START.GS.OS.95`
  (6.1-Original in sys61/; Bootbild ab $7919 statt gepacktem Logo, boot95b.bin ab $76B0, Variablen $7686;
  6.0.4-Fassung: boot95/patch_604.py.txt, boot95_604.s.txt)
- Startbild sofort: `boot95x/boot95x.s` -> *:System:Boot95 (Typ $06/$1000, Kopf "PHX1", JSL $01/1004 durch PRODOS 6.1,
  Bild = GSOS95.SPL, Marke "95" in $E1/9DFE). Alles neu bauen: `./mkboot61.sh` (auch PRODOS: $2509 BRA, Pfad SYSTEM:BOOT95). Texte von 6.1 entschaerfen: `tools/debrand61.py IMG` (auch in apply61.py).
  Rueckfall (2.1.36): `boot95/early.s` (ld65 -C early.cfg) haengt patch.py bei $9B79 an START.GS.OS; $6800 = JSR $9B79,
  laedt SYSTEM:GSOS95.SPL per PRODOS-Vektor ($2000,X=4) nach $E1/2000, wenn die Marke fehlt. Test: `python3 tearly.py`.
  PRODOS: `python3 tools/patch_prodos.py sys61/PRODOS.61 sys/PRODOS.95` (Phoenix immer starten), ins Image mit `tools/add_file.py <img> PRODOS sys/PRODOS.95 FF 0000`
- v2.18 (XCODE3): `pfilt.inc` Seitenfilter (Software-Archiv: Verzeichnisseiten unter ARCU = NPREF+1828 bzw. Asimov-Spiegel
  als Liste; RSS/Atom bei Inhaltstyp *xml* (xf_pf_ctype in hdr_parse) als Seite; Ausgabe ab WB_LIN, auch stueckweise in big_page),
  `xf_fname` Dateinamen fuer *:Downloads (auch dl_name), about:archiv / about:rss. `xfer.inc` ZMODEM-Empfang (automatisch),
  XMODEM/YMODEM (Apfel-D), XMODEM senden (Apfel-U); Haken in term.inc (tn_char, tn_poll), vt.inc (vt_event, vt_leave).
  Tests: `twa.py` (Archiv, Feeds), `twx.py` (Uebertragung gegen eine nachgebildete Mailbox). Zu weite Spruenge: `python3 tools/fixbr.py`.
- v2.18 Startmenue: `startsub.s` Programme = Gruppen (SGN=3) + Arbeitsplatz + eigene Programme (ab SPRG); Gruppen als
  zweites Untermenue (s2_*, Eintraege si_*), Treffer in der Gruppe als $4000+Zeile. Rechner `calc.inc` (Fensterart 24, BCD mit
  Dezimalmodus; Emulator kann jetzt SED/Dezimal-ADC/SBC), Solitaer `sol.inc` (Fensterart 25, Merkpuffer 4 KB fuer das Ziehen,
  Kartenfarben s_suit*/s_big* in assets.py). Tests: `tcalc.py`, `tsol.py`.
- `farify.py <datei> 3` verlegt eine Quelldatei in XCODE3 (v2.16: ext.s, prog.s, plist.s, multisel.s)
- Update-Pruefung (fav.inc): about:update laedt GSOS95-VERSION.txt (Zeile 1 Build, 2 Name, 3 Link; erzeugt von
  mkwebstr.py aus BUILD), Vergleich mit UPD_B1..3; Standard-Adresse UPDATE_URL in mkwebstr.py, sonst NPREF+1600
- `cookie.inc` (in web.s): Cookies der Sitzung, eigener 2,25-KB-Block; hdr_parse -> ck_set, req_build -> ck_req
  (Rahmenprotokoll, TLS auf der Karte), Dialog Einstellungen > Netzwerk (Fensterart 22),
  Einstellungen, Startseite, Favoriten und Mailbox-Telefonbuch (`bbs.inc`) in *:System:GSOS95.NET (1,5 KB). Puffer: eine eigene 64-KB-Bank (Aufteilung oben in net.s)
- `web.s` (+ `url.inc`, `html.inc`, `webdata.inc`): Browser (Fensterart 21): Adressen, HTTP, HTML -> Text, Zeilenumbruch, Anzeige
  - `forms.inc` Formulare, `fav.inc` Favoriten/about:-Seiten, `dl.inc` Downloads, `gopher.inc` Gopher, `gif.inc` GIF-Anzeige,
    `term.inc` Telnet/IRC (Abfrage im Leerlauf der Hauptschleife: `xf_tn_poll`), `vt.inc` Vollbild-Terminal ANSI/VT100
    (640er-Modus, Hauptschleife leitet alle Ereignisse an `xf_vt_event`; Schriften `vtfont.inc` (8x8) und `vtfont4.inc` (4x8, Farbmodus; Zeichen in `font4x8.py`) aus `mkvtfont.py`, braucht `pip install luma.core`), `proxy.inc` HTTPS ueber Proxy,
    `table.inc` Spaltenbreiten (Textstrom $1C t / $1D t / $1E / $17 Spalte Breite), `img.inc` Bilder in der Seite ($1A n)
  - Bildbank (IMBK, 64 KB): $0000-$AFFF Bildpunkte, $B000 Bildadressen, $B800 Auswahl-Optionen, $C000-$FFFF LZW-Tabellen
- `mkwebstr.py` -> `webstr.inc` (Texte DE/EN) und `webdata.inc` (Tags, Entitaeten, Zeichentabellen, Startseite)
- `fixbr.py`: zu weite Verzweigungen (bxx) automatisch in Gegenbedingung + jmp umwandeln
- `boot95/`: Bootbild (START.GS.OS): hellblaue Flaeche mit "Created by", Build und Balken = unterer Teil von
  GSOS95.SPL (Farben/Positionen aus mksplash.py). Alter Farbverlauf: boot95_verlauf.s.txt (nur noch im Startbild
  von GSOS95, wenn GSOS95.SPL fehlt). `python3 boot95/gen.py`, ca65/ld65 mit `boot95.cfg`
  (zwei Teile: $7778 und $7638 im nicht mehr benutzten Originalcode), dann
  `python3 boot95/patch.py -r <START.GS.OS aus dem Image> sys/START.GS.OS.95` (setzt das Bild in eine schon
  gepatchte Datei ein; `patch.py <original> <ziel>` braucht Apples Original). Ansicht: `boot95/sim2.py <datei>`
- `boot95/mksplash.py`: Startbild nach Daniels Entwurf (`boot95/GS.OS.9..00.PNG`) -> `sys/GSOS95.SPL` ($C1/$0000,
  32 KB inkl. SCBs/Paletten; Text "Created by" und Build aus BUILD, Balkenrahmen per Emulation von START.GS.OS).
  Geladen wird sie beim Booten vom temporaeren Systemzusatz `init95/spl95.s` -> `init95/SPLASH95` ($B7, `python3 init95/mkspl.py`),
  im Verzeichnis System.Setup direkt hinter TOOL.SETUP (Reihenfolge = Ladereihenfolge). Er liest die Datei in einen Block, setzt
  Paletten 1..13 (vom Bootbild unbenutzt) und kopiert dann Zeile 0..140 (Punkte vor SCB) - kein Flackern. GSOS95 macht
  es beim Start genauso fuer alle Zeilen. START.GS.OS selbst darf GS/OS NICHT aufrufen
  (beim Zeichnen des Bootbilds ist GS/OS noch nicht geladen - Absturz in v2.14 vom 24.11.).
  Das Bild bis zum Balken benutzt nur EINE Palette (1): GSSquared liest Paletten je Zeile in Echtzeit, Bilder mit
  wechselnden Paletten von Zeile zu Zeile flackerten dort in Streifen.
- `assets.py`, `font.py`, `split_assets.py`: Grafik/Texte (DE/EN) -> `assets*.inc`
- `rel/LIESMICH.txt`, `rel/README_EN.txt`: Anleitung und Versionsgeschichte

## Bauen
Pfade in den Skripten zeigen auf `/home/claude/gs95` bzw. `/home/claude/ext/cc65/bin` – ggf. anpassen.

    python3 mkwebstr.py         # -> webstr.inc, webdata.inc (nur nach Textaenderungen)
    python3 fixbr.py            # falls ca65 "Range error" meldet
    bash buildapp.sh            # -> GSOS95.S16
    python3 init95/mkinit.py    # -> init95/GSOS95.INIT
    python3 ti1.py; python3 ti2.py; python3 ti3.py   # Tests (OK)
    python3 tw1.py; python3 tw2.py; python3 tw3.py   # Internet-Tests mit nachgebildeter Uthernet II (w5100sim.py)
    python3 tw4.py              # kein DHCP, DNS nur ueber TCP (wie GSSquared)
    python3 tw5.py              # CoGS (cogssim.py): HTTP, HTTPS, Zertifikatfehler
    python3 tw6.py; python3 tw7.py; python3 tw8.py; python3 tw9.py   # Formulare; Favoriten/Download/DNS; Gopher; GIF
    python3 tw10.py; python3 tw11.py   # Telnet/IRC; HTTPS ueber Proxy
    python3 tw12.py; python3 tw13.py   # lange Seiten, Tabellen, Auswahllisten; Bilder in der Seite
    python3 tw14.py; python3 tw15.py; python3 tw16.py   # zwei Tabellen hintereinander; Symbolleiste; NTP/Zeitzonen
    python3 tw17.py             # Vollbild-Terminal (ANSI/VT100, Telnet-Aushandlung)
    python3 tw18.py             # Mailbox-Telefonbuch about:bbs (bbs.inc)
    python3 twm.py              # Marinetti (marisim.py): automatische Wahl, DNR, HTTP, Telnet, about:net
    python3 twf.py              # FujiNet (fujisim.py): SmartPort, N:TCP, N:HTTPS mit Inhaltstyp, POST, Telnet
    python3 twd.py              # WLAN-Modem (modemsim.py: SCC + Hayes): Baudrate, ATDT, NO CARRIER, +++/ATH
    python3 twn.py              # Dialog Netzwerk: Kartenwahl, Port, Baudrate, Proxy-Feld, Testen, GSOS95.NET (2 KB)
    python3 twc.py              # Cookies: Rechner, Domain, ersetzen, loeschen
    python3 twe.py              # Zeileneditor Adresszeile/Formulare
    python3 twu.py              # Start > Hilfe (about:keys), about:update, Blaettern im Startmenue
    python3 tcddb.py            # CD-Titelnamen (CDDB) mit Marinetti-Simulator, about:cddb
    python3 tcd.py; python3 tcd0.py   # CD-Wiedergabe mit nachgebildeten Laufwerken (cdsim.py) / ohne Laufwerk
    python3 t68.py              # Explorer v2.14: Apfel-Pfeile, Papierkorb mit Fortschritt, Systemordner, Klang
    python3 t46.py              # Bootbild und Startbild stimmen ueberein (braucht sys/START.GS.OS.95)
    python3 boot95/mksplash.py  # -> sys/GSOS95.SPL (nach jeder Aenderung von BUILD / START.GS.OS)
    python3 init95/mkspl.py     # -> init95/SPLASH95
    python3 t69.py              # Startbild: SPLASH95 laedt es ins Bootbild, GSOS95 zeigt dasselbe, Balken laeuft weiter
    GS2=40 python3 tw1.py       # dieselben Tests mit der Uthernet II wie in GSSquared 1.x (gs2sim.py:
                                # Netzwerk-Thread, verzoegerte Statusmeldungen, Empfangs-Credits; Zahl = Buszugriffe je Thread-Lauf)

Ins Disk-Image (ProDOS): `tools/update_file.py <image> SYSTEM/GSOS95 GSOS95.S16`, `tools/update_file.py <image> SYSTEM/DRIVERS/SCSICD.DRIVER drv/SCSICD.DRIVER`, `tools/update_file.py <image> SYSTEM/START.GS.OS sys/START.GS.OS.95` `tools/update_file.py <image> SYSTEM/GSOS95.SPL sys/GSOS95.SPL` und `tools/update_file.py <image> SYSTEM/SYSTEM.SETUP/SPLASH95 init95/SPLASH95`
Dateien samt Ressourcenzweig aus einem anderen Image: `tools/copy_files.py <quelle> <ziel> <pfade...>`
