GS/OS95 v2.18.0 - a Windows-95-style desktop for the Apple IIgs
Created by Marc Golombeck
================================================================

GS/OS95 replaces the Finder of GS/OS (System 6.0.4) with a desktop in the
style of 1995: Start menu, taskbar, Explorer windows, Recycle Bin.
It has been tested in the GSSquared emulator (Apple IIgs ROM 3). The user
interface is available in German and English (switch it in Settings).

FILES
  GSOS95_System.hdv   32 MB ProDOS hard disk image: System 6.0.4 with
                      GS/OS95 as start program. Mount it as the boot
                      drive (slot 7, drive 1).
  LIESMICH.txt        full German manual and version history
  README_EN.txt       this file

FIRST STEPS
  Boot from the image. The GS/OS95 boot screen appears and the desktop
  follows. Tab opens the Start menu, the arrow keys and Return work
  everywhere, Esc closes dialogs, menus and desk accessories.

FEATURES
  - Explorer windows: icon and details view, folder tree on the left
    (View > Folder tree, also in details view when the window is wide),
    sorting, resize, maximize, minimize, up to six windows, drag and drop,
    copy/cut/paste with progress dialog.
  - Properties dialog with a preview of the first lines of text files
    and a colour thumbnail of pictures ($C1, packed $C0/0001, APF $C0/0002).
  - Picture viewer for the same formats (double-click or Return).
  - Recycle Bin on the boot volume with Restore (File menu, Ctrl+E).
  - Start menu with Programs and Documents submenus, Run, Settings, Help.
  - Taskbar with window buttons, clock (click it to set date and time)
    and a quick-launch bar: My Computer, Settings, Run, Minesweeper,
    Show Desktop.
  - Desktop patterns (Settings > Display > Pattern 0-4).
  - Desktop shortcuts; context menu with Option-click.
  - Drives on the desktop (Settings > Display > "Show drives", off by
    default): all drives appear in the right column like on a Mac;
    double-click opens them, a newly inserted 3.5" disk appears by itself.
    5.25" drives are shown without being read.
  - Desk accessories (NDAs) open in windows with a Win95 title bar; the
    X button or Esc closes them. Keys, including Apple-Z/X/C/V, go to the
    accessory you opened or clicked last (GS/OS95 windows then show a grey
    title bar); the system arrow is used over them.
  - Settings: display, mouse, date/time, sound, screen saver, language.
  - Minesweeper (own taskbar button, can be minimized): 9x9 cells,
    10 mines, smiley button, mine counter and
    timer, best time is saved. Option-click places a flag.
  - Launches programs and returns to the desktop afterwards.
  - NEW in 2.7.0: system extension GSOS95.INIT (in SYSTEM:SYSTEM.SETUP).
    It gives the windows of ALL programs a Win95 title bar - also in
    Apple's Finder and in other GS/OS applications: title on the left,
    maximize and X buttons on the right, grey (or white) when inactive.
    Works in 320 and 640 mode and picks the closest colours of the
    current palette (with Apple's 4-colour Finder palette the bar is
    black). To switch it off, move the file out of System.Setup and restart.
  - NEW in 2.8.0: Open and Save dialogs in Explorer style for all
    programs that use Apple's Standard File (SFGetFile2/SFPutFile2 and
    the older SFGetFile/SFPutFile): "Look in" with Up button, list with
    folders first, file name field, Open/Save and Cancel, New Folder
    (Save), "Replace file?" question. Click the "Look in" box to see all
    drives (My Computer). Keys: arrows, Return, Esc, Apple-Up, typing a
    letter jumps to the next matching entry. Language follows GS/OS95.
    Hold Option while the dialog is called to get Apple's original.
    Customized dialogs (e.g. in ShrinkIt) stay Apple's.
  - NEW in 2.9.0: push buttons of all programs in Win95 style (grey 3D
    face, black frame on the default button, sunken when pressed, grey
    text when inactive) - also in Apple's own dialogs such as Standard
    File. With only 4 colours they get plain black frames.
  - NEW in 2.9.2: check boxes of all programs in Win95 style (white sunken
    box with a black check mark, grey when inactive).
  - 2.9.3: stability fixes in GSOS95.INIT (checks that the Window Manager
    is running; clears its work memory at startup).
  - 2.9.4: ProDOS 8 programs launch again (GS/OS reported "system error
    $0201"): GSOS95.INIT no longer keeps a locked block in bank 0 and is
    no longer loaded into special memory.

  - NEW in 2.10.0: Internet with the Uthernet II card (WIZnet W5100),
    driven directly (no Marinetti needed). Settings > Network: card slot
    (default 3), DHCP or fixed IP address, subnet mask, gateway and DNS
    server; "Test" starts the card and asks the DNS server. Stored in
    *:System:GSOS95.NET. If DHCP fails, the fixed addresses are used.
  - NEW in 2.10.0: web browser "Internet" (globe in the quick-launch bar,
    or Start > Programs > Internet). Address bar (an address or a search
    term - FrogFind searches for you); Back and Home on the left; reload,
    bookmark (star), favorites (list) and save (disk) on the right.
    HTTP/1.0 with redirects and chunked transfer; pages of any size are
    read in pieces, up to 16 KB of text is shown per page. Headings,
    lists, quotes, tables with measured columns, <pre>, rules, UTF-8 /
    Latin-1 / Windows-1252 umlauts, entities. Blue links, one click
    follows them. GIF images in a page (up to 8) are fetched after the
    text and shown in place (up to 120 pixels high, mapped to the 16
    desktop colours); Esc skips them, a click shows an image full size.
    Forms: text fields, hidden fields, check boxes, buttons, <select>
    (a click switches to the next choice), <textarea>; GET and POST.
    Downloads of everything that is not a text page go to *:Downloads:
    (any size, file type from the extension); the save button (Ctrl-S)
    stores the current page unchanged. Favorites: star (Ctrl-D) adds,
    list (Ctrl-B or about:fav) shows them; one can become the home page.
    Gopher (menus, text, search, files). Telnet (telnet://host[:port])
    and IRC (irc://host[:port]/channel): received text scrolls in the
    window, the address bar is the input line, Return sends; IRC logs in
    by itself, answers PING, joins the channel; /msg, /join, /nick, /quit
    etc. are sent as IRC commands. HTTPS with the CoGS card (the card does
    the encryption); with the Uthernet II through a proxy on your network
    (e.g. WebOne, Macproxy): about:proxy=host:port.
    Keys: arrows and space scroll, left arrow = back, Ctrl-R reloads,
    Esc closes (or cancels loading). Back restores the scroll position.
    Start page with sites for vintage computers.
    Clock via the Internet: on the first page load after starting, the
    Uthernet II asks a time server (pool.ntp.org, SNTP) and sets the IIgs
    clock. Default is Central Europe (UTC+1 with daylight saving time);
    change it with about:tz=+1eu (EU DST), about:tz=-5us (US DST),
    about:tz=+0 (no DST), or turn it off with about:tz=off. Time zone and
    proxy are shown on the favorites page.
    Not yet: own TLS on the IIgs, JPEG/PNG,
    images beside text.
  - NEW in 2.11.0: full-screen terminal for Telnet (BBSes): 80 x 25
    characters with the PC character set (CP437, box and block
    graphics). Two looks, Apple-F switches (remembered; also
    about:vt=color / about:vt=gray): colour (default, 320 mode with the
    16 ANSI colours and a 4-pixel-wide font) or sharp grayscale (640
    mode, 8x8 font, colours as four brightness levels).
    ANSI/VT100 sequences: cursor positioning, erase, scroll region,
    insert/delete lines and characters, save/restore cursor, cursor
    position report, bold/underline/inverse, colours. Keys go straight
    to the remote side (arrows as ESC [ A..D); Apple-Esc or Ctrl-] ends
    the connection. Telnet negotiation reports terminal type "ANSI" and
    a size of 80 x 25. IRC stays in the browser window. Fonts: 8x8 PC
    font from luma.core (MIT licence), 4x8 font drawn for GS/OS95.
    Phone book (about:bbs): name, host and port, add/edit/delete up to
    8 entries (kept in GSOS95.NET, now 1.5 KB), "connect directly" with
    host and port. Start > Programs > Terminal and a new quick-launch
    icon open it. Boot and start screens: smoother blue gradient (16
    shades mixed in a 2x2 pattern, 60 steps) and the build number
    ("Build v2.1.26", file BUILD); image GSOS95_v2126_System.hdv.
  - NEW in 2.10.0: CoGS network card, driven directly (registers at
    $C080+n*16, ID $A2). GS/OS95 looks for a CoGS first, then for an
    Uthernet II. Marinetti is not needed for these two cards.
    In the GSSquared emulator (1.0 or newer) add a card with
    card = "uthernet2"; DHCP does not answer there, the fixed addresses
    (DNS 1.1.1.1) work, and the clock always follows the Mac. Tested in
    GSSquared 1.x: FrogFind, 68k.news, textfiles.com (repeatedly, with
    images), saving. GSSquared up to 0.11 drops receive packets when the
    card's buffer is full (fixed in 1.0.0, issue #172), so long pages get
    gaps there.

  - NEW in 2.12.0: every network card. Settings > Network has a card
    choice (- and + buttons): Automatic, Uthernet II, CoGS, FujiNet,
    Marinetti, WiFi modem. "Automatic" tries CoGS, FujiNet on the
    SmartPort, Marinetti (if already connected), Uthernet II directly,
    and finally Marinetti with its own connect. Stored in GSOS95.NET
    (now 2 KB; older files are still read).
  - Marinetti (TCP/IP tool set, TCPIP extension): GS/OS95 uses its
    connection, name lookup and TCP, so every card with a link layer
    works: Uthernet I (CS8900A), LANceGS, Uthernet II, SLIP/PPP, MacIP
    and the emulators. Marinetti 3.0b11 is installed on the image (TCPIP
    in System.Setup, TCP/IP control panel, TOOL054, link layers in
    System:TCPIP); pick the link layer once in the TCP/IP control panel.
  - FujiNet on the SmartPort: the "NETWORK" device is driven through the
    SmartPort firmware (FWEntry). http, Gopher, Telnet and IRC use
    N:TCP://; https is encrypted by the FujiNet itself (N:HTTPS://, also
    POST forms) - no proxy needed.
  - WiFi modem on the serial port (WiModem232, Zimodem, WiFi232, the
    GSplus/KEGS virtual modem): port 1 (printer) or 2 (modem), 2400 to
    57600 baud, "ATDThost:port", hang-up with "+++" / "ATH", end of data
    at "NO CARRIER". https through a proxy.
  - Network icon in the taskbar tray next to the clock (blue =
    connected, gray = not); a click opens Settings > Network.
  - New page about:net (network status: card, IP, DNS, how https works).
  - FujiNet and the WiFi modem were tested against simulators built from
    the firmware source and the Hayes behaviour, not yet on real hardware.

  - NEW in 2.13.0: clock via the Internet also with FujiNet, Marinetti
    and the WiFi modem (NIST Time service, TCP port 37); HTTPS proxy
    field in Settings > Network; session cookies in the browser (up to
    16, sent back to the same host or the Domain= domain); about:net
    shows a connection test to frogfind.com with the time in ms.

  - NEW in 2.14.0: text fields with a real cursor (arrows, Option-arrow
    by word, Apple-arrow / Ctrl-A / Ctrl-E start and end); Explorer
    Apple-Down opens, Apple-Up / Delete go up and keep the folder you came
    from selected; emptying the Recycle Bin shows progress and can be
    cancelled (Option skips the question); items dropped on the System
    folder can be sorted into CDevs, Desk.Accs, Fonts, Drivers, Tools,
    FSTs and System.Setup; double-clicking a sound file ($D8) plays it
    (system sounds with rSoundSample, 8-bit mono AIFF or raw at 11 kHz); Start > Help shows all keyboard
    shortcuts (about:keys); long Start submenus scroll, up to 12 programs;
    about:update checks a version file in the GitHub repository
    (set once with about:update=https://raw.githubusercontent.com/NAME/
    REPO/main/GSOS95-VERSION.txt; https needs CoGS/FujiNet or the proxy);
    new boot screen designed by Daniel (*:System:GSOS95.SPL, with credits,
    build number and progress bar; the small temporary init SPLASH95 adds
    the picture above the credits as soon as GS/OS is ready, colours first,
    then row by row without flicker).

  - NEW in 2.15.0: CD Player (Start > Programs > CD Player) for audio CDs
    in a SCSI CD-ROM drive: AppleCD SC / SC Plus / CD 150 (Apple's own
    audio commands) and AppleCD 300 / 300e / 300 Plus / 600 and other
    SCSI-2 drives (detected automatically). Display with track, time,
    track length, total time, state and drive name; buttons previous,
    play, pause, stop, next, eject and one button per track. Keys: space
    play/pause, Return play, left/right arrow, S stop, E eject, L next
    drive, Esc closes (the CD keeps playing). Inserting an audio CD opens
    the player and starts playing (like AutoPlay). Sound comes from the
    drive's headphone jack / audio outputs. SCSICD.DRIVER is now Brutal
    Deluxe's driver (Apple's driver plus SCSI-2 audio) in its latest
    build from August 2026 (from the BlueSCSI disk), whose command
    lookup tries the next table entry for commands listed more than once
    (e.g. $1B start/stop/eject, $A5).
    Build 2.1.31 also patches the driver for SCSI-2 drives: "no disc" is
    reported with the standard code $3A, which Apple's driver did not know
    (it only knew the Apple drives' $B0). Ejecting returned an I/O error
    instead of "offline", and after a disc change the driver could loop
    forever waiting for the drive (hang). GS/OS95 also checks CD drives
    itself once per second with TEST UNIT READY. Not yet tested on real
    hardware.

  - NEW in 2.16.0: the CD Player shows artist, album and track names from
    the Internet (CDDB over HTTP): key I or a click on the top line of the
    display; automatic for each new CD when the network is already
    running. Default server freedb.dbpoweramp.com; change it in the
    browser with about:cddb=host, set an e-mail address (needed by
    gnudb.gnudb.org) with about:cddbmail=name@host; about:cddb shows the
    settings. About 3.6 KB more room in the main code bank.

  - NEW in 2.17.0: the bug fixes of System 6.1 (Build 22): GS.OS,
    GS.OS.Dev, Start.GS.OS, P8, Error.Msg, Finder, Tool.Setup, TS2, TS3,
    Resource.Mgr, Sys.Resources, tools 14/15/16/18/20/23/27/28/34,
    Pro.FST and the 3.5", 5.25", SCSI, modem and printer drivers. Not
    taken: 6.1's looks (the 6.1 tools are fixed to the "Standard" look of
    6.0.1, Battery RAM is not touched), languages, Phoenix screen,
    Video640, Japanese. The SCSI CD driver stays the patched Brutal Deluxe
    one. Not yet started on real hardware or in GSSquared; the previous
    image GSOS95_v2132_System.hdv still has the 6.0.4 system files.
    Start-up picture at once: 6.1's PRODOS runs *:System:Phoenix before
    GS/OS; in GS/OS95 that file holds the start-up picture itself and
    shows it right after power-on. As a fallback (build 2.1.36),
    Start.GS.OS also loads the picture itself as its very first step,
    so it appears before the progress bar even where the Phoenix path
    does not run. The progress bar slows down as it fills instead of
    running full after 16 seconds.

  - NEW in 2.18.0: Software Archive (Start > Programs, about:archiv): the
    Asimov archive (mirror mirrors.apple2.org.za) as a plain list -
    folders in bold, files below, ".." on top; works with Apache, nginx
    and custom index pages. Clicking a file downloads it to *:Downloads
    (write disk images to a disk with DiskMaker). Other mirror:
    about:archiv=address. Downloads keep sensible names: long names are
    cut to 15 characters but keep their extension (and so their type).
    File transfer in the terminal: ZMODEM downloads start by themselves;
    Apple-D receives with XMODEM (128/1K, CRC or checksum) or YMODEM
    batch, Apple-U sends with XMODEM-CRC; Esc cancels; everything goes to
    *:Downloads. No ZMODEM upload yet.
    News (Start > Programs, about:rss): RSS and Atom feeds are shown as a
    page with the feed name and, per item, title, date, text (200
    characters, no pictures) and "Read more". Most feeds are https, which
    needs a CoGS or FujiNet card or a proxy.
    Tidier Start menu: Programs shows the groups Tools (Calculator, CD
    Player), Internet (Internet, Terminal, Software Archive, News) and
    Games (Minesweeper, Solitaire), then My Computer and your own
    programs, all with icons. New Calculator (16 digits, %, square root,
    square, 1/x, n!, pi, memory) and Solitaire (Klondike like Windows:
    drag cards, double-click, draw one or three, undo, score and time,
    bouncing cards when you win).
    No traces of System 6.1 left on the image: the start-up picture file
    is now called *:System:Boot95 instead of *:System:Phoenix (PRODOS
    still loads it right after power-on; Start.GS.OS loads the picture
    itself as a fallback), and Finder/Tool.Setup messages name GS/OS95.
    Build 2.1.39: in Solitaire an ace can be dragged or double-clicked
    onto an empty foundation, and dragging stacks leaves no traces. The
    browser start page lists more sites that work without https
    (wiby.me, TheOldNet, NeverSSL). News (about:rss) has a new
    "Without https" section: BBC News, Slashdot, OSnews, TechTinkering
    (not yet checked on real hardware).
    Build 2.1.40: an MS-DOS prompt (Start > Programs > Tools > MS-DOS, or
    about:msdos in the browser). Full screen like DOS, drive C: is the
    boot volume. Commands: DIR, CD (CD .., CD \), TYPE (paged, Esc
    stops), CLS, COLOR 1E, DATE, TIME, MEM, VER, ECHO, HELP, EXIT; up
    arrow recalls the last command. Read-only: DEL, COPY, FORMAT and
    friends change nothing.
    Build 2.1.41: start-up picture with clouds down to the bottom, text
    and progress bar in front of them; the very bottom fades to one colour
    so the system extension icons stay invisible. The progress bar
    continues seamlessly when GS/OS95 takes over.
    Build 2.1.42: a fifth code segment (64 KB of new room) with:
    Paint (Start > Programs > Tools > Paint; pen, brush, eraser, spray,
    line, rectangle, filled rectangle, fill, 16 colours; S saves a $C1
    picture, new ones as *:BILD1, *:BILD2 ...; Z undo, N new, arrows
    move the view; B in the picture viewer opens the picture in Paint),
    SSH through the FujiNet (ssh://user@host, then the password),
    a second screen saver (Mystify-style lines, alternating with the
    starfield), and a Windows 95 style blue screen on Option-Apple-C
    (boss key; any key returns, nothing actually crashes).
    Build 2.1.43: Paint gains ellipse, filled ellipse, colour picker
    (returns to the previous tool) and text (type at the click point;
    Return or a click places it, Esc cancels). Esc asks first when the
    picture is unsaved (a second Esc discards). The blue screen also
    opens with Ctrl-Apple-C (for emulators without an Option key).
    Build 2.1.44: internet pack. MS-DOS prompt gains PING, NSLOOKUP,
    IPCONFIG (also WINIPCFG) and FINGER user@host (PING times a TCP
    connect to port 80; the cards have no ICMP). FTP in the browser:
    ftp://host/path/ lists the directory as links ([folder], files with
    size, [..] up); clicking a file downloads it to *:Downloads. Anonymous
    login; works with the Uthernet II (second socket for data) and the
    FujiNet (N:FTP://), other cards show a hint. Weather: Start > Programs >
    Internet > Weather (about:wetter), town remembered, forecast from
    wttr.in. Bookmarks with folders (Ctrl-D, about:lz; [Home] sets the start page), history of the last
    20 pages (about:verlauf), selectable search engine (FrogFind, wiby.me,
    DuckDuckGo); old favourites are migrated (*:System:GSOS95.LZ).
    Ctrl-P in the browser opens the page's (first) picture in Paint.
    Build 2.1.45: faster start-up. SYSTEM:GSOS95 shrinks from 428 KB to
    278 KB: its ~23,000 relocation entries are now stored as compact SUPER
    records (about 1 byte instead of 7-8 per entry). No functional change.
    Build 2.1.46: fix. With 2.1.45 GS/OS95 no longer found the network card
    under real GS/OS (GSSquared showed 0.0.0.0 everywhere). The cause is the
    compact relocation format; the load file is back to the classic form.
    Build 2.1.47: Notepad (Start > Programs > Tools > Notepad, double-click
    on text files): up to 32,000 characters, word wrap, umlauts, Apple-S
    saves, Esc asks if changed. Clicking the clock opens Date/Time with a
    month calendar and a clock face. about:update now checks GitHub
    (8-Bit-Shack/gsos95).

KNOWN LIMITS
  - Radio buttons, scroll bars and menus of other programs
    keep their Apple look.
    Customized Standard File dialogs of a program stay Apple's.
  - 5.25" drives are not listed in the Open/Save dialogs.
  - The folder tree shows the path, sibling folders and other drives;
    clicking navigates (there is no separate expand/collapse).
  - 5.25" drives are only read when you open them (an empty drive takes
    about 20 seconds to report "no disk").
  - Versions 2.5.3 to 2.5.5 had a crash bug - please do not use them.

GS/OS95 itself is the file SYSTEM:GSOS95; SYSTEM:START.GS.OS carries the
new boot screen.
