# Erzeugt webstr.inc: Texte fuer Netzwerk und Browser (Laengenbyte + Text)
# Umlaute -> eigene Zeichennummern $80..$86 (siehe font_ext in assets.py)
UML = {'ä': 0x80, 'ö': 0x81, 'ü': 0x82, 'Ä': 0x83, 'Ö': 0x84, 'Ü': 0x85, 'ß': 0x86}
S = [  # Name, Deutsch, Englisch
 ('slot',   'Slot der Netzwerkkarte:', 'Network card slot:'),
 ('dhcp',   'Adresse automatisch (DHCP)', 'Get address automatically (DHCP)'),
 ('ip',     'IP-Adresse:', 'IP address:'),
 ('mask',   'Subnetzmaske:', 'Subnet mask:'),
 ('gw',     'Gateway:', 'Gateway:'),
 ('dns',    'DNS-Server:', 'DNS server:'),
 ('test',   'Testen', 'Test'),
 ('testing','Teste die Verbindung...', 'Testing the connection...'),
 ('nocard', 'Keine Netzwerkkarte in Slot ', 'No network card in slot '),
 ('cardok', 'Karte bereit, IP ', 'Card ready, IP '),
 ('dhcpok', ' (DHCP)', ' (DHCP)'),
 ('dhcpno', ' (fest)', ' (fixed)'),
 ('dnsok',  'Namensdienst antwortet: ', 'DNS answers: '),
 ('dnsbad', 'Der Namensdienst antwortet nicht.', 'The DNS server does not answer.'),
 ('badip',  'Bitte eine gültige Adresse eingeben.', 'Please enter a valid address.'),
 ('saved',  'Gespeichert.', 'Saved.'),
 ('net',    'Netzwerk', 'Network'),
 ('inet',   'Internet', 'Internet'),
 # Browser
 ('ready',  'Fertig', 'Done'),
 ('start',  'Netzwerk wird gestartet...', 'Starting the network...'),
 ('look',   'Suche ', 'Looking up '),
 ('conn',   'Verbinde mit ', 'Connecting to '),
 ('load',   'Lade... ', 'Loading... '),
 ('bytes',  ' Bytes', ' bytes'),
 ('fmt',    'Seite wird aufbereitet...', 'Formatting the page...'),
 ('e_card', 'Keine Netzwerkkarte gefunden (Einstellungen > Netzwerk).', 'No network card found (Settings > Network).'),
 ('e_dns',  'Der Name wurde nicht gefunden: ', 'Name not found: '),
 ('e_conn', 'Der Server lehnt die Verbindung ab.', 'The server refused the connection.'),
 ('e_time', 'Der Server antwortet nicht (Zeitüberschreitung).', 'The server does not respond (time-out).'),
 ('e_abrt', 'Abgebrochen.', 'Cancelled.'),
 ('e_mem',  'Nicht genug Speicher.', 'Not enough memory.'),
 ('e_url',  'Diese Adresse kann ich nicht öffnen.', 'Cannot open this address.'),
 ('e_ssh',  'SSH geht nur mit der FujiNet-Karte.', 'SSH needs the FujiNet card.'),
 ('e_tls',  'HTTPS (verschlüsselt) geht mit CoGS oder FujiNet, sonst über einen Proxy (about:proxy=rechner:port).', 'HTTPS (encrypted) works with CoGS or FujiNet, otherwise through a proxy (about:proxy=host:port).'),
 ('e_mari', 'Marinetti ist nicht geladen oder nicht verbunden.', 'Marinetti is not loaded or not connected.'),
 ('e_mdm',  'Das WLAN-Modem antwortet nicht (Anschluss, Baudrate?).', 'The WiFi modem does not answer (port, baud rate?).'),
 ('card',   'Karte:', 'Card:'),
 ('proxy',  'HTTPS-Proxy:', 'HTTPS proxy:'),
 ('c_auto', 'Automatisch', 'Automatic'),
 ('c_w5',   'Uthernet II', 'Uthernet II'),
 ('c_cogs', 'CoGS', 'CoGS'),
 ('c_fuji', 'FujiNet', 'FujiNet'),
 ('c_mari', 'Marinetti', 'Marinetti'),
 ('c_mdm',  'WLAN-Modem', 'WiFi modem'),
 ('c_none', 'keine', 'none'),
 ('slot2',  'Slot', 'Slot'),
 ('port',   'Port', 'Port'),
 ('baud',   'Baudrate:', 'Baud rate:'),
 ('adrcard','Die Adresse stellt die Karte ein.', 'The card sets the address.'),
 ('adrmari','Die Adressen stellt Marinetti ein.', 'Marinetti sets the addresses.'),
 ('adrmdm', 'Port 1 = Drucker, 2 = Modem', 'Port 1 = printer, 2 = modem'),
 ('fujiok', 'FujiNet bereit (WLAN)', 'FujiNet ready (Wi-Fi)'),
 ('mariok', 'Marinetti verbunden, IP ', 'Marinetti connected, IP '),
 ('mdmok',  'WLAN-Modem antwortet', 'WiFi modem answers'),
 ('nofuji', 'Keine FujiNet am SmartPort gefunden.', 'No FujiNet found on the SmartPort.'),
 ('nomari', 'Marinetti: nicht geladen/verbunden.', 'Marinetti: not loaded/connected.'),
 ('nomdm',  'Das Modem antwortet nicht.', 'The modem does not answer.'),
 ('nocard2','Keine Netzwerkkarte gefunden.', 'No network card found.'),
 ('dnscard','Namen löst die Karte auf.', 'The card resolves names.'),
 ('nstitle','Netzwerkstatus', 'Network status'),
 ('nsoff',  'Nicht verbunden', 'Not connected'),
 ('nsip',   'IP-Adresse: ', 'IP address: '),
 ('nsdns',  'DNS-Server: ', 'DNS server: '),
 ('nscard', 'Verbindung über: ', 'Connected via: '),
 ('nstls',  'HTTPS: auf der Karte', 'HTTPS: on the card'),
 ('nsprox', 'HTTPS: über Proxy ', 'HTTPS: through proxy '),
 ('nsnotls','HTTPS: Proxy nötig (about:proxy=rechner:port)', 'HTTPS: needs a proxy (about:proxy=host:port)'),
  ('nstest', 'Verbindungstest frogfind.com: ', 'Connection test frogfind.com: '),
 ('nsok',   'erreichbar in ', 'reachable in '),
 ('nsms',   ' ms', ' ms'),
 ('nsfail', 'keine Verbindung', 'no connection'),
 ('nsset',  'Ändern: Einstellungen > Netzwerk oder Klick auf das Netzwerksymbol neben der Uhr.', 'Change: Settings > Network, or click the network icon next to the clock.'),
 ('e_link', 'Die CoGS-Karte hat keine Netzverbindung (WLAN/Kabel prüfen).', 'The CoGS card has no network link (check Wi-Fi/cable).'),
 ('e_tlsf', 'Die verschlüsselte Verbindung kam nicht zustande.', 'The encrypted connection could not be set up.'),
 ('e_cert', 'Das Zertifikat der Seite ist ungültig: ', 'The certificate of this site is not valid: '),
 ('cogsok', 'CoGS-Karte bereit, IP ', 'CoGS card ready, IP '),
 ('cwifi', ' (WLAN)', ' (Wi-Fi)'),
 ('cwire', ' (Kabel)', ' (wired)'),
 ('e_http', 'Fehler vom Server: ', 'Server error: '),
 ('e_type', 'Diese Art von Datei kann ich nicht anzeigen: ', 'Cannot show this kind of file: '),
 ('e_redir','Zu viele Weiterleitungen.', 'Too many redirects.'),
 ('e_empty','Der Server hat nichts geschickt.', 'The server sent nothing.'),
 ('cut',    '[Seite gekürzt - zu groß]', '[Page cut - too large]'),
 ('img',    '[Bild]', '[Image]'),
 ('send',   'Senden', 'Submit'),
 ('gsrch',  'Gopher-Suche', 'Gopher search'),
 ('imgld',  'Lade Bild ', 'Loading image '),
 ('tnon',   '[Verbunden. Eingabe in der Adresszeile, Return sendet.]', '[Connected. Type in the address bar, Return sends.]'),
 ('tnoff',  '[Verbindung beendet]', '[Connection closed]'),
 ('gifbad', 'Das GIF-Bild konnte nicht gelesen werden (zu gro\xdf oder besch\xe4digt).', 'The GIF image could not be read (too large or damaged).'),
 ('dl',     'Speichere... KB: ', 'Saving... KB: '),
 ('dlok',   'Datei gespeichert', 'File saved'),
 ('dlkb',   ' KB', ' KB'),
 ('dlerr',  'Die Datei konnte nicht gespeichert werden (Startvolume voll oder schreibgesch\xfctzt?).', 'The file could not be saved (startup volume full or locked?).'),
 ('dlhint', 'Der Browser speichert alles, was keine Textseite ist, im Ordner Downloads des Startvolumes.', 'The browser saves everything that is not a text page in the Downloads folder of the startup volume.'),
 ('favok',  'Zu den Lesezeichen hinzugef\xfcgt (Strg-B zeigt sie)', 'Added to bookmarks (Ctrl-B shows them)'),
 ('favno',  'Diese Seite kann nicht gemerkt werden', 'This page cannot be bookmarked'),
 ('fav',    'Favoriten', 'Favorites'),
 ('fhome',  'als Startseite', 'make home page'),
 ('fdel',   'entfernen', 'remove'),
 ('fnone',  '(noch keine - Stern oder Strg-D merkt die aktuelle Seite)', '(none yet - the star or Ctrl-D bookmarks the current page)'),
 ('fhelp',  'Stern/Strg-D merkt eine Seite, Liste/Strg-B zeigt diese Liste.', 'Star/Ctrl-D bookmarks a page, List/Ctrl-B shows this list.'),
 ('fhis',   'Startseite: ', 'Home page: '),
 ('fstd',   'Standard-Startseite verwenden', 'Use the standard start page'),
 ('fdef',   'Standard (about:start)', 'standard (about:start)'),
 ('fprox',  'HTTPS-Proxy ohne TLS-Karte (about:proxy=rechner:port): ', 'HTTPS proxy without a TLS card (about:proxy=host:port): '),
 ('fnopx',  'keiner', 'none'),
 ('ftzoff', 'aus', 'off'),
 ('term',   'Terminal', 'Terminal'),
 ('cdp',    'CD-Wiedergabe', 'CD Player'),
 ('arc',    'Software-Archiv', 'Software Archive'),
 ('news',   'Nachrichten', 'News'),
 ('calc',   'Rechner', 'Calculator'),
 ('dos',    'MS-DOS', 'MS-DOS'),
 ('paint',  'Paint', 'Paint'),
 ('edit',   'Editor', 'Notepad'),
 ('wx',     'Wetter', 'Weather'),
 ('gtools', 'Werkzeuge', 'Tools'),
 ('ginet',  'Internet', 'Internet'),
 ('ggames', 'Spiele', 'Games'),
 ('mine',   'Minesweeper', 'Minesweeper'),
 ('sol',    'Solitär', 'Solitaire'),
 ('bbtit',  'Mailbox-Telefonbuch', 'BBS phone book'),
 ('bbhelp', 'Ein Klick auf eine Mailbox verbindet (Telnet, Vollbild-Terminal). Apfel-F: Farbe/Grau, Apfel-Esc oder Strg-]: Ende.', 'Click a BBS to connect (Telnet, full-screen terminal). Apple-F: colour/gray, Apple-Esc or Ctrl-]: quit.'),
 ('bbchg',  'ändern', 'edit'),
 ('bbdel',  'löschen', 'delete'),
 ('bbnone', '(keine Einträge)', '(no entries)'),
 ('bbdir',  'Direkt verbinden', 'Connect directly'),
 ('bbhost', 'Rechner:', 'Host:'),
 ('bbport', 'Port:', 'Port:'),
 ('bbgo',   'Verbinden', 'Connect'),
 ('bbnew',  'Neue Mailbox', 'New BBS'),
 ('bbedit', 'Mailbox ändern', 'Edit BBS'),
 ('bbname', 'Name:', 'Name:'),
 ('bbsave', 'Speichern', 'Save'),
 ('fvt',    'Telnet-Terminal (Apfel-F, about:vt=farbe, about:vt=grau): ', 'Telnet terminal (Apple-F, about:vt=color, about:vt=gray): '),
 ('fvtc',   'Farbe', 'colour'),
 ('fvtg',   'Graustufen, scharf', 'grayscale, sharp'),
 ('vthelp', 'GS/OS95-Terminal ANSI 80x25 - Apfel-F: Farbe/Grau, Apfel-D/U: Datei empfangen/senden (ZMODEM kommt von selbst), Apfel-Esc oder Strg-]: Ende', 'GS/OS95 terminal ANSI 80x25 - Apple-F: colour/gray, Apple-D/U: receive/send a file (ZMODEM starts by itself), Apple-Esc or Ctrl-]: quit'),
 ('vtend',  '[Verbindung beendet - bitte eine Taste druecken]', '[Connection closed - press any key]'),
 ('ftz',    'Uhrzeit per Internet, Zeitzone (about:tz=+1eu, -5us, +0, off): ', 'Clock via Internet, time zone (about:tz=+1eu, -5us, +0, off): '),
 ('link',   '[Link]', '[Link]'),
 ('nolink', 'Diese Art von Link wird nicht unterstützt.', 'This kind of link is not supported.'),
 ('err',    'Die Seite konnte nicht geladen werden', 'The page could not be loaded'),
 ('uptitle','GS/OS95-Update', 'GS/OS95 update'),
 ('upinst', 'Installiert: ', 'Installed: '),
 ('upavail','Verfügbar: ', 'Available: '),
 ('upnew',  '<b>Eine neue Version ist da!</b>', '<b>A new version is available!</b>'),
 ('upcur',  'Sie haben die neueste Version.', 'You have the latest version.'),
 ('upbad',  'Die Versionsdatei ist unvollständig (Zeile 1: Build, z.B. 2.1.29).', 'The version file is incomplete (line 1: build, e.g. 2.1.29).'),
 ('upa1',   '</p><p><a href="', '</p><p><a href="'),
 ('upa2',   '">', '">'),
 ('upa3',   '</a></p><p>', '</a></p><p>'),
 ('uphow',  'Das neue Festplatten-Image am PC herunterladen und wie gewohnt auf das Laufwerk bzw. die Karte übertragen. Vorher *:System:GSOS95.PREFS, GSOS95.NET und GSOS95.LZ sichern (Einstellungen, Telefonbuch, Lesezeichen).', 'Download the new hard disk image on a PC and copy it to the drive or card as usual. Back up *:System:GSOS95.PREFS, GSOS95.NET and GSOS95.LZ first (settings, phone book, bookmarks).'),
 ('cdtit',  'CD-Titelnamen', 'CD track names'),
 ('cdsrv',  'Server: ', 'Server: '),
 ('cdml',   'E-Mail (an den Server): ', 'E-mail (sent to the server): '),
 ('cddef',  'freedb.dbpoweramp.com (Standard)', 'freedb.dbpoweramp.com (default)'),
 ('cdnone', '(keine)', '(none)'),
 ('cdhelp', 'Die CD-Wiedergabe holt Künstler, Album und Titelnamen über CDDB (HTTP): Taste I oder Klick auf die erste Zeile der Anzeige, automatisch wenn das Netzwerk schon läuft. ', 'The CD Player looks up artist, album and track names via CDDB (HTTP): key I or a click on the first line of the display, automatically when the network is already running. '),
 ('cdhelp2','Ändern: about:cddb=rechner (leer = Standard), about:cddbmail=name@rechner. gnudb.gnudb.org verlangt eine echte E-Mail-Adresse.', 'Change: about:cddb=host (empty = default), about:cddbmail=name@host. gnudb.gnudb.org requires a real e-mail address.'),
 ('upnourl','Noch keine Update-Adresse eingetragen. In die Adresszeile tippen: about:update=https://raw.githubusercontent.com/8-Bit-Shack/gsos95/main/GSOS95-VERSION.txt', 'No update address set yet. Type into the address bar: about:update=https://raw.githubusercontent.com/8-Bit-Shack/gsos95/main/GSOS95-VERSION.txt'),
 ('upsrc',  'Quelle: ', 'Source: '),
 ('nsupd',  '<br><a href="about:update">Nach Updates suchen</a> &middot; <a href="about:keys">Tastenkürzel</a>', '<br><a href="about:update">Check for updates</a> &middot; <a href="about:keys">Keyboard shortcuts</a>'),
]
out = []
def enc(t):
    b = []
    for ch in t:
        if ch in UML: b.append(UML[ch])
        else:
            assert 32 <= ord(ch) < 127, ch
            b.append(ord(ch))
    assert len(b) < 256
    return b
for name, de, en in S:
    for lab, t in (('ws_' + name, de), ('ws_' + name + '_en', en)):
        b = enc(t)
        out.append('%s: .byte %d,%s' % (lab, len(b), ','.join('$%02X' % x for x in b)))
BUILD = open('BUILD').read().strip()
VERNAME = 'v2.18.0'
# Standard-Adresse der Versionsdatei (leer = erst per about:update=... eintragen)
UPDATE_URL = 'https://raw.githubusercontent.com/8-Bit-Shack/gsos95/main/GSOS95-VERSION.txt'
bn = [int(x) for x in BUILD.split('.')]
out.append('UPD_B1 = %d' % bn[0])
out.append('UPD_B2 = %d' % bn[1])
out.append('UPD_B3 = %d' % bn[2])
for lab, t in (('ws_bld', BUILD + ' (' + VERNAME + ')'), ('ws_updef', UPDATE_URL)):
    b = enc(t)
    out.append('%s: .byte %d%s' % (lab, len(b), ''.join(',$%02X' % x for x in b)))
open('webstr.inc', 'w').write('\n'.join(out) + '\n')
open('GSOS95-VERSION.txt', 'w').write(BUILD + '\nGS/OS95 ' + VERNAME + '\nhttps://github.com/8-Bit-Shack/gsos95\n')
print(len(S), 'Texte')

# ---------------- webdata.inc: Tabellen fuer den HTML-Leser und die Startseite ----------------
A = dict(BLK1=1, P=2, H=3, BR=4, HR=5, PRE=6, A=7, B=8, UL=9, OL=10, LI=11, TR=12, TD=13,
         TITLE=14, SKIP=15, IMG=16, BQ=17, FORM=18, INPUT=19, BUTTON=20, SELECT=21, OPTION=22, TEXTAREA=23, TABLE=24)
TAGS = {
 'BLK1': 'div section article header footer nav main aside center address figure figcaption '
         'dl fieldset details summary caption legend dt frameset noscript hgroup',
 'P': 'p', 'H': 'h1 h2 h3 h4 h5 h6', 'BR': 'br', 'HR': 'hr', 'PRE': 'pre listing xmp plaintext',
 'A': 'a', 'B': 'b strong', 'UL': 'ul menu dir', 'OL': 'ol', 'LI': 'li', 'TR': 'tr', 'TD': 'td th',
 'TITLE': 'title', 'SKIP': 'script style svg template noembed object datalist math',
 'IMG': 'img', 'BQ': 'blockquote dd', 'FORM': 'form', 'INPUT': 'input', 'BUTTON': 'button', 'SELECT': 'select', 'OPTION': 'option', 'TEXTAREA': 'textarea', 'TABLE': 'table',
}
d = []
for act, names in TAGS.items():
    for n in names.split():
        assert len(n) <= 10
        d.append('  .byte %d,"%s",%d' % (len(n), n, A[act]))
d.append('  .byte 0')
lines = ['; erzeugt von mkwebstr.py', 'w_tags:'] + d
for k, v in A.items():
    lines.append('A_%s = %d' % (k, v))
ENT = {'amp': '&', 'lt': '<', 'gt': '>', 'quot': '"', 'apos': "'", 'nbsp': ' ',
       'auml': 0x80, 'ouml': 0x81, 'uuml': 0x82, 'Auml': 0x83, 'Ouml': 0x84, 'Uuml': 0x85, 'szlig': 0x86,
       'copy': 'c', 'reg': 'R', 'ndash': '-', 'mdash': '-', 'minus': '-', 'hellip': '.', 'laquo': '"', 'raquo': '"',
       'bull': 0x87, 'middot': 0x87, 'euro': 'E', 'eacute': 'e', 'egrave': 'e', 'ecirc': 'e', 'Eacute': 'E',
       'aacute': 'a', 'agrave': 'a', 'acirc': 'a', 'iacute': 'i', 'oacute': 'o', 'uacute': 'u', 'ccedil': 'c',
       'ntilde': 'n', 'oslash': 'o', 'aring': 'a', 'ldquo': '"', 'rdquo': '"', 'bdquo': '"', 'lsquo': "'",
       'rsquo': "'", 'sbquo': "'", 'trade': 'T', 'deg': 'o', 'times': 'x', 'shy': 0, 'zwnj': 0, 'zwj': 0,
       'lrm': 0, 'rlm': 0, 'thinsp': ' ', 'ensp': ' ', 'emsp': ' ', 'iexcl': '!', 'iquest': '?',
       'sect': 'S', 'para': 'P', 'plusmn': '+', 'larr': '<', 'rarr': '>', 'uarr': '^', 'darr': 'v',
       'pound': 'L', 'yen': 'Y', 'cent': 'c'}
lines.append('w_ents:')
for k, v in ENT.items():
    val = v if isinstance(v, int) else ord(v)
    lines.append('  .byte %d,"%s",$%02X' % (len(k), k, val))
lines.append('  .byte 0')
L1 = (" !cL?Y|S\"ca\"-\x00R-o+23'uP\x87,1o\"????"
      "AAAA\x83AACEEEEIIII" "DNOOOO\x84xOUUU\x85YP\x86"
      "aaaa\x80aaceeeeiiii" "dnoooo\x81/ouuu\x82ypy")
assert len(L1) == 96, len(L1)
lines.append('w_lat1:')
lines.append('  .byte ' + ','.join('$%02X' % ord(c) for c in L1))
CP = "E\x00'f\".++^%S<O\x00Z\x00\x00''\"\"\x87--~Ts>o\x00zY"
assert len(CP) == 32, len(CP)
lines.append('w_cp:')
lines.append('  .byte ' + ','.join('$%02X' % ord(c) for c in CP))

START_DE = """<title>GS/OS95 Internet</title>
<h1>Willkommen im Internet</h1>
<p>Dieser Browser zeigt einfache Webseiten &uuml;ber HTTP. Geben Sie oben eine Adresse ein
(z.B. <i>frogfind.com</i>) oder einfach einen Suchbegriff - dann sucht FrogFind f&uuml;r Sie.</p>
<h2>Seiten f&uuml;r alte Rechner (ohne https)</h2>
<ul>
<li><a href="http://frogfind.com/">FrogFind</a> - Suchmaschine, die moderne Seiten lesbar macht
<li><a href="http://68k.news/">68k.news</a> - Nachrichten im Kleinformat
<li><a href="http://info.cern.ch/hypertext/WWW/TheProject.html">Die erste Webseite</a> (CERN, 1991)
<li><a href="http://textfiles.com/">textfiles.com</a> - Texte aus der Mailbox-Zeit
<li><a href="http://wiby.me/">wiby.me</a> - Suchmaschine f&uuml;r schlichte, klassische Seiten
<li><a href="http://theoldnet.com/">TheOldNet</a> - das Web der 90er f&uuml;r alte Browser
<li><a href="http://neverssl.com/">NeverSSL</a> - Testseite, garantiert unverschl&uuml;sselt
<li><a href="gopher://gopher.floodgap.com/">Floodgap</a> - der Einstieg ins Gopher-Netz
<li><a href="telnet://bbs.electronicchicken.com/">Electronic Chicken</a> - Mailbox mit ANSI-Grafik (Telnet)
<li><a href="telnet://cqbbs.ddns.net:6502/">Captain's Quarters</a> - Apple-II-Mailbox (Telnet)
<li><a href="about:lz">Meine Lesezeichen</a>
</ul>
<h2>Bedienung</h2>
<ul>
<li>Blauer Text ist ein Link: einmal anklicken.
<li>Pfeiltasten und Leertaste bl&auml;ttern, Pfeil links geht zur&uuml;ck.
<li>Tippen springt in die Adresszeile, Return l&auml;dt. Strg-R l&auml;dt neu.
<li>Auch gopher://, telnet://rechner (Vollbild-Terminal, Apfel-Esc oder Strg-] beendet) und irc://rechner/kanal (Eingabe in der Adresszeile).
<li>Kn&ouml;pfe oben rechts: neu laden, Lesezeichen setzen (Strg-D), Lesezeichen (Strg-B), speichern (Strg-S). Strg-P holt ein Bild der Seite in Paint.
<li>Formulare: Feld anklicken, tippen, Return oder Knopf schickt ab.
<li>GIF-Bilder stehen in der Seite (anklicken vergr&ouml;&szlig;ert), andere Dateien landen im Ordner Downloads.
<li>Esc schlie&szlig;t das Fenster, w&auml;hrend des Ladens bricht Esc ab.
<li>Slot und Adressen: Einstellungen &gt; Netzwerk.
</ul>
<hr>
<p>Mit der CoGS-Karte &ouml;ffnet der Browser auch verschl&uuml;sselte Seiten (https://), mit der Uthernet II &uuml;ber einen Proxy im Heimnetz (about:proxy=rechner:port).</p>
"""
START_EN = """<title>GS/OS95 Internet</title>
<h1>Welcome to the Internet</h1>
<p>This browser shows simple web pages over HTTP. Type an address above
(e.g. <i>frogfind.com</i>) or just a search term - FrogFind will search for you.</p>
<h2>Sites for vintage computers (no https)</h2>
<ul>
<li><a href="http://frogfind.com/">FrogFind</a> - a search engine that makes modern pages readable
<li><a href="http://68k.news/">68k.news</a> - news in a small format
<li><a href="http://info.cern.ch/hypertext/WWW/TheProject.html">The first web page</a> (CERN, 1991)
<li><a href="http://textfiles.com/">textfiles.com</a> - texts from the BBS era
<li><a href="http://wiby.me/">wiby.me</a> - a search engine for plain, classic pages
<li><a href="http://theoldnet.com/">TheOldNet</a> - the 90s web for old browsers
<li><a href="http://neverssl.com/">NeverSSL</a> - test page, guaranteed unencrypted
<li><a href="gopher://gopher.floodgap.com/">Floodgap</a> - the gateway to Gopherspace
<li><a href="telnet://bbs.electronicchicken.com/">Electronic Chicken</a> - a BBS with ANSI art (Telnet)
<li><a href="telnet://cqbbs.ddns.net:6502/">Captain's Quarters</a> - an Apple II BBS (Telnet)
<li><a href="about:lz">My bookmarks</a>
</ul>
<h2>How to use it</h2>
<ul>
<li>Blue text is a link: click it once.
<li>Arrow keys and space scroll, left arrow goes back.
<li>Typing jumps to the address bar, Return loads. Ctrl-R reloads.
<li>gopher://, telnet://host (full-screen terminal, Apple-Esc or Ctrl-] ends it) and irc://host/channel (type into the address bar) work too.
<li>Buttons at the top right: reload, bookmark (Ctrl-D), bookmarks (Ctrl-B), save (Ctrl-S). Ctrl-P opens a picture of the page in Paint.
<li>Forms: click a field, type, then Return or the button submits.
<li>GIF images appear in the page (click to enlarge); other files go to the Downloads folder.
<li>Esc closes the window; while loading, Esc cancels.
<li>Slot and addresses: Settings &gt; Network.
</ul>
<hr>
<p>With the CoGS card the browser also opens encrypted pages (https://); with the Uthernet II through a proxy on your network (about:proxy=host:port).</p>
"""
for lab, txt in (('w_start', START_DE), ('w_start_en', START_EN)):
    b = txt.encode('latin-1')
    assert 0 not in b
    lines.append(lab + ':')
    for i in range(0, len(b), 24):
        lines.append('  .byte ' + ','.join('$%02X' % x for x in b[i:i + 24]))
    lines.append('  .byte 0')
open('webdata.inc', 'w').write('\n'.join(lines) + '\n')
print('webdata.inc', len(lines))

# ---------------- keyhtml.inc: Hilfeseite Tastenkuerzel (about:keys, liegt in XCODE3) ----------------
KEYS_DE = """<title>Tastenk&uuml;rzel</title>
<h1>GS/OS95 - Tastenk&uuml;rzel</h1>
<h2>Schreibtisch und Fenster</h2>
<ul>
<li>Tab: Startmen&uuml; &ouml;ffnen, Pfeiltasten und Return w&auml;hlen, Esc schlie&szlig;t.
<li>Apfel-Tab: n&auml;chstes Fenster nach vorn.
<li>Esc: vorderstes Fenster schlie&szlig;en.
<li>Beenden-Dialog: H herunterfahren, N neu starten, F zum GS/OS-Finder.
</ul>
<h2>Explorer</h2>
<ul>
<li>Pfeiltasten: Auswahl bewegen. Return oder Apfel-Pfeil runter: &ouml;ffnen.
<li>Delete oder Apfel-Pfeil hoch: eine Ebene h&ouml;her, der Ordner, aus dem man kommt, bleibt markiert.
<li>Strg-N neuer Ordner, Strg-E umbenennen, Strg-D l&ouml;schen, Strg-P Eigenschaften.
<li>Strg-X ausschneiden, Strg-C kopieren, Strg-V einf&uuml;gen, Strg-A alles markieren.
<li>Strg-R aktualisieren, Strg-L Diskette auswerfen.
<li>Shift- oder Apfel-Klick: mehrere Objekte markieren.
<li>Wahltaste beim Klick auf &quot;Papierkorb leeren&quot;: ohne R&uuml;ckfrage leeren. Esc oder Apfel-Punkt bricht ab.
<li>Objekte auf den Ordner System ziehen: Kontrollfelder, Zubeh&ouml;r, Schriften, Treiber, Werkzeuge und Erweiterungen werden auf Wunsch in die passenden Unterordner einsortiert.
<li>Doppelklick auf eine Klangdatei spielt sie ab (AIFF 8 Bit oder Rohdaten).
</ul>
<h2>Eingabefelder</h2>
<ul>
<li>Pfeil links/rechts: Cursor. Wahltaste-Pfeil: wortweise. Apfel-Pfeil oder Strg-A/Strg-E: Anfang/Ende.
<li>Delete l&ouml;scht vor dem Cursor, Strg-D (in Formularen) dahinter.
</ul>
<h2>Internet</h2>
<ul>
<li>Pfeiltasten und Leertaste bl&auml;ttern, Pfeil links geht zur&uuml;ck.
<li>Strg-R neu laden, Strg-D Lesezeichen setzen, Strg-B Lesezeichen, Strg-S speichern, Strg-P Bild nach Paint.
<li>Esc schlie&szlig;t das Fenster, w&auml;hrend des Ladens bricht Esc ab.
<li>Terminal: Apfel-F Farbe/Grau, Apfel-Esc oder Strg-] beendet.
<li>Dateien im Terminal: ZMODEM-Downloads starten von selbst; Apfel-D empf&auml;ngt mit XMODEM/YMODEM, Apfel-U sendet mit XMODEM. Alles landet in *:Downloads.
<li><a href="about:archiv">Software-Archiv</a> (Asimov, anderer Spiegel: about:archiv=Adresse) &middot; <a href="about:rss">Nachrichten</a> (RSS/Atom-Feeds werden als Seite gezeigt)
<li><a href="about:net">Netzwerkstatus</a> &middot; <a href="about:update">Nach Updates suchen</a>
</ul>
<h2>Minesweeper</h2>
<ul><li>Wahltaste + Klick setzt eine Fahne.</ul>
<h2>Rechner</h2>
<ul><li>Ziffern, Komma/Punkt, + - * / (auch :), Return oder = rechnet, % Prozent, Delete l&ouml;scht die letzte Ziffer.
<li>W Wurzel, Q x hoch 2, R 1/x, ! Fakult&auml;t, P Pi, N Vorzeichen. Esc l&ouml;scht, zweimal Esc schlie&szlig;t.</ul>
<h2>Solit&auml;r</h2>
<ul><li>Karten mit der Maus ziehen, Doppelklick legt eine Karte auf den Zielstapel, Klick auf den Talon zieht.
<li>N neues Spiel, U oder Strg-Z r&uuml;ckg&auml;ngig, D eine/drei Karten ziehen, A alles M&ouml;gliche ablegen, Leertaste Talon, Esc schlie&szlig;t.</ul>
<h2>CD-Wiedergabe</h2>
<ul><li>Leertaste Wiedergabe/Pause, Eingabetaste Wiedergabe, Pfeil links/rechts Titel zur&uuml;ck/weiter.
<li>S Stopp, E Auswerfen, L n&auml;chstes Laufwerk, I Titelnamen aus dem Internet (<a href="about:cddb">about:cddb</a>), Esc schlie&szlig;t (die CD spielt weiter).</ul>
"""
KEYS_EN = """<title>Keyboard shortcuts</title>
<h1>GS/OS95 - Keyboard shortcuts</h1>
<h2>Desktop and windows</h2>
<ul>
<li>Tab: open the Start menu, choose with the arrow keys and Return, Esc closes it.
<li>Apple-Tab: bring the next window to the front.
<li>Esc: close the front window.
<li>Shut Down dialog: H shut down, N restart, F switch to the GS/OS Finder.
</ul>
<h2>Explorer</h2>
<ul>
<li>Arrow keys: move the selection. Return or Apple-Down: open.
<li>Delete or Apple-Up: go up one level; the folder you came from stays selected.
<li>Ctrl-N new folder, Ctrl-E rename, Ctrl-D delete, Ctrl-P properties.
<li>Ctrl-X cut, Ctrl-C copy, Ctrl-V paste, Ctrl-A select all.
<li>Ctrl-R refresh, Ctrl-L eject disk.
<li>Shift- or Apple-click: select several items.
<li>Option while choosing &quot;Empty Recycle Bin&quot;: empty without asking. Esc or Apple-period cancels.
<li>Drag items onto the System folder: control panels, desk accessories, fonts, drivers, tools and extensions are sorted into the matching subfolders if you agree.
<li>Double-click a sound file to play it (8-bit AIFF or raw samples).
</ul>
<h2>Text fields</h2>
<ul>
<li>Left/right arrow: cursor. Option-arrow: word by word. Apple-arrow or Ctrl-A/Ctrl-E: start/end.
<li>Delete removes the character before the cursor, Ctrl-D (in forms) the one after it.
</ul>
<h2>Internet</h2>
<ul>
<li>Arrow keys and space scroll, left arrow goes back.
<li>Ctrl-R reload, Ctrl-D bookmark, Ctrl-B bookmarks, Ctrl-S save, Ctrl-P picture to Paint.
<li>Esc closes the window; while loading, Esc cancels.
<li>Terminal: Apple-F colour/gray, Apple-Esc or Ctrl-] quits.
<li>Files in the terminal: ZMODEM downloads start by themselves; Apple-D receives with XMODEM/YMODEM, Apple-U sends with XMODEM. Everything goes to *:Downloads.
<li><a href="about:archiv">Software Archive</a> (Asimov; other mirror: about:archiv=address) &middot; <a href="about:rss">News</a> (RSS/Atom feeds are shown as a page)
<li><a href="about:net">Network status</a> &middot; <a href="about:update">Check for updates</a>
</ul>
<h2>Minesweeper</h2>
<ul><li>Option-click places a flag.</ul>
<h2>Calculator</h2>
<ul><li>Digits, comma/period, + - * / (also :), Return or = calculates, % percent, Delete removes the last digit.
<li>W square root, Q square, R 1/x, ! factorial, P pi, N sign. Esc clears, Esc twice closes.</ul>
<h2>Solitaire</h2>
<ul><li>Drag cards with the mouse, double-click sends a card to a foundation, click the stock to draw.
<li>N new game, U or Ctrl-Z undo, D draw one/three, A move everything possible to the foundations, space draws, Esc closes.</ul>
<h2>CD Player</h2>
<ul><li>Space play/pause, Return play, left/right arrow previous/next track.
<li>S stop, E eject, L next drive, I track names from the Internet (<a href="about:cddb">about:cddb</a>), Esc closes (the CD keeps playing).</ul>
"""
kl = ['; erzeugt von mkwebstr.py: about:keys']
for lab, txt in (('w_keys', KEYS_DE), ('w_keys_en', KEYS_EN)):
    b = txt.encode('latin-1')
    assert 0 not in b
    kl.append(lab + ':')
    for i in range(0, len(b), 24):
        kl.append('  .byte ' + ','.join('$%02X' % x for x in b[i:i + 24]))
    kl.append('  .byte 0')
open('keyhtml.inc', 'w').write('\n'.join(kl) + '\n')
print('keyhtml.inc', len(KEYS_DE) + len(KEYS_EN))

# ---------------- dosstr.inc: Texte der MS-DOS-Eingabeaufforderung (XCODE3, Zeichensatz CP437) ----------------
DOS = [
 ('BANNER', '\r\nGS/OS95 MS-DOS-Eingabeaufforderung\r\n(C) GS/OS95-Projekt 1995-2026. HELP zeigt die Befehle.\r\n\r\n',
            '\r\nGS/OS95 MS-DOS Prompt\r\n(C) GS/OS95 project 1995-2026. HELP lists the commands.\r\n\r\n'),
 ('VER',    '\r\nGS/OS95 [Version %s]\r\n' % VERNAME[1:], '\r\nGS/OS95 [Version %s]\r\n' % VERNAME[1:]),
 ('BAD',    'Befehl oder Dateiname nicht gefunden.\r\n', 'Bad command or file name\r\n'),
 ('BADDIR', 'Ungültiges Verzeichnis\r\n', 'Invalid directory\r\n'),
 ('NOFILE', 'Datei nicht gefunden\r\n', 'File not found\r\n'),
 ('NEEDP',  'Erforderlicher Parameter fehlt\r\n', 'Required parameter missing\r\n'),
 ('DENY',   'Zugriff verweigert - hier werden keine Dateien verändert.\r\n', 'Access denied - this prompt never changes files.\r\n'),
 ('DRV',    'Nicht bereit beim Lesen von Laufwerk ', 'Not ready reading drive '),
 ('WIN',    'GS/OS95 läuft bereits.\r\n', 'GS/OS95 is already running.\r\n'),
 ('FMT',    'Lieber nicht - Ihre Daten bleiben, wo sie sind.\r\n', 'Better not - your data stays where it is.\r\n'),
 ('MORE',   '-- Fortsetzung (Esc = Abbruch) --', '-- More (Esc = stop) --'),
 ('VOL',    ' Datenträger in Laufwerk C: ist ', ' Volume in drive C is '),
 ('DIROF',  ' Verzeichnis von C:', ' Directory of C:'),
 ('DIRTAG', '<DIR>        ', '<DIR>        '),
 ('FILES',  ' Datei(en)', ' file(s)'),
 ('BYTES',  ' Bytes', ' bytes'),
 ('DIRS',   ' Verzeichnis(se)', ' dir(s)'),
 ('DATE',   'Aktuelles Datum: ', 'Current date is '),
 ('TIME',   'Aktuelle Uhrzeit: ', 'Current time is '),
 ('MEM1',   ' Bytes konventioneller Speicher (wie immer)\r\n', ' bytes conventional memory (as always)\r\n'),
 ('MEM2',   ' Bytes frei\r\n', ' bytes free\r\n'),
 ('WDAY',   'SoMoDiMiDoFrSa', 'SuMoTuWeThFrSa'),
 ('HELP1',  'CD      Verzeichnis wechseln (CD .. / CD \\)\r\nCLS     Bildschirm löschen\r\nCOLOR   Farben, z.B. COLOR 1E\r\nDATE    Datum anzeigen\r\nDIR     Dateien auflisten\r\nECHO    Text ausgeben\r\nEXIT    Eingabeaufforderung beenden\r\n',
            'CD      change directory (CD .. / CD \\)\r\nCLS     clear the screen\r\nCOLOR   colours, e.g. COLOR 1E\r\nDATE    show the date\r\nDIR     list files\r\nECHO    print text\r\nEXIT    quit the prompt\r\n'),
 ('HELP2',  'MEM     freien Speicher anzeigen\r\nTIME    Uhrzeit anzeigen\r\nTYPE    Textdatei anzeigen\r\nVER     Version anzeigen\r\n',
            'MEM     show free memory\r\nTIME    show the time\r\nTYPE    show a text file\r\nVER     show the version\r\n'),
 ('HELP3',  'IPCONFIG  Netzwerkkarte und Adressen\r\nPING      Rechner erreichbar? (TCP-Port 80)\r\nNSLOOKUP  Adresse eines Namens\r\nFINGER    benutzer@rechner abfragen\r\nPfeil hoch holt den letzten Befehl zurück.\r\n',
            'IPCONFIG  network card and addresses\r\nPING      is a host reachable? (TCP port 80)\r\nNSLOOKUP  address of a name\r\nFINGER    query user@host\r\nUp arrow recalls the last command.\r\n'),
 ('NONET',  'Keine Netzwerkkarte gefunden.\r\n', 'No network card found.\r\n'),
 ('NDNS',   'Rechner nicht gefunden.\r\n', 'Host not found.\r\n'),
 ('NCONN',  'Verbindung abgelehnt.\r\n', 'Connection refused.\r\n'),
 ('NTIME',  'Zeitüberschreitung.\r\n', 'Request timed out.\r\n'),
 ('NABRT',  'Abgebrochen.\r\n', 'Cancelled.\r\n'),
 ('NOTH',   'Netzwerkfehler ', 'Network error '),
 ('PING1',  'Ping wird ausgeführt für ', 'Pinging '),
 ('PING2',  ' (TCP-Port 80):\r\n\r\n', ' (TCP port 80):\r\n\r\n'),
 ('PREP',   'Antwort von ', 'Reply from '),
 ('PTIM',   ': Zeit=', ': time='),
 ('PMS',    'ms\r\n', 'ms\r\n'),
 ('NSL1',   'Name:     ', 'Name:     '),
 ('NSL2',   'Adresse:  ', 'Address:  '),
 ('NSLC',   'Die Karte löst Namen selbst auf - keine Adresse abrufbar.\r\n', 'The card resolves names itself - no address available.\r\n'),
 ('IPC0',   '\r\nGS/OS95-IP-Konfiguration\r\n\r\n', '\r\nGS/OS95 IP Configuration\r\n\r\n'),
 ('IPC1',   '   Netzwerkkarte . . . : ', '   Network card. . . . : '),
 ('IPC2',   '   IP-Adresse. . . . . : ', '   IP Address. . . . . : '),
 ('IPC3',   '   Subnetzmaske. . . . : ', '   Subnet Mask . . . . : '),
 ('IPC4',   '   Standardgateway . . : ', '   Default Gateway . . : '),
 ('IPC5',   '   DNS-Server. . . . . : ', '   DNS Server. . . . . : '),
 ('IPC6',   '   DHCP. . . . . . . . : ', '   DHCP. . . . . . . . : '),
 ('IPC7',   '   Physische Adresse . : ', '   Physical Address. . : '),
 ('IPCK',   '(von der Karte verwaltet)', '(managed by the card)'),
 ('JA',     'Ja\r\n', 'Yes\r\n'),
 ('NEIN',   'Nein\r\n', 'No\r\n'),
 ('CARDS',  '?|Uthernet II|CoGS|FujiNet|Marinetti|WLAN-Modem', '?|Uthernet II|CoGS|FujiNet|Marinetti|WiFi modem'),
]
X2N = ()                                  # (frueher: lange Texte in XCODE2)
DOS = [d for d in DOS if d[0] not in X2N] + [d for d in DOS if d[0] in X2N]
dl = ['DS_X2 = %d' % (len(DOS) - len(X2N))]
d2 = []
pg = []
for i, (k, de, en) in enumerate(DOS):
    dl.append('DS_%s = %d' % (k, i))
for k, de, en in DOS:
    for suf, t in (('de', de), ('en', en)):
        b = t.encode('cp437')
        assert len(b) < 256, k
        (d2 if k in X2N else dl).append('ds_%s_%s: .byte %d%s' % (k.lower(), suf, len(b), ''.join(',$%02X' % x for x in b)))
dl.append('ds_de: .addr ' + ', '.join('ds_%s_de' % k.lower() for k, _, _ in DOS))
dl.append('ds_en: .addr ' + ', '.join('ds_%s_en' % k.lower() for k, _, _ in DOS))
for lab, t in (('dos_pg', '<title>MS-DOS-Eingabeaufforderung</title><h1>MS-DOS-Eingabeaufforderung</h1><p>Die Eingabeaufforderung wurde beendet. <a href="about:msdos">Neu starten</a></p>'),
               ('dos_pg_en', '<title>MS-DOS Prompt</title><h1>MS-DOS Prompt</h1><p>The prompt has ended. <a href="about:msdos">Start again</a></p>')):
    pg.append('%s: .byte %s,0' % (lab, ','.join('$%02X' % x for x in t.encode('latin-1'))))
open('dosstr.inc', 'w').write('\n'.join(dl) + '\n')
open('dospg.inc', 'w').write('\n'.join(pg) + '\n')
open('dosstr2.inc', 'w').write('\n'.join(d2) + '\n')
print('dosstr.inc', sum(len(d) + len(e) for _, d, e in DOS))
