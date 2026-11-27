<div align="center">

# GS/OS95

### A Windows 95 style desktop for the Apple IIgs

*Start menu · Taskbar · Explorer · Recycle Bin · Web browser · FTP · Telnet & SSH · MS-DOS prompt · Notepad · Paint · Solitaire · Minesweeper · CD Player*

![Platform](https://img.shields.io/badge/platform-Apple%20IIgs-6e5494)
![CPU](https://img.shields.io/badge/CPU-WDC%2065C816-blue)
![OS](https://img.shields.io/badge/runs%20on-GS%2FOS-informational)
![Build](https://img.shields.io/badge/build-2.1.47-brightgreen)
![Assembler](https://img.shields.io/badge/assembler-ca65-orange)
![UI](https://img.shields.io/badge/UI-English%20%7C%20Deutsch-lightgrey)
![Network](https://img.shields.io/badge/network-Uthernet%20II%20%7C%20CoGS%20%7C%20FujiNet%20%7C%20Marinetti%20%7C%20Wi--Fi%20modem-9cf)

<img src="docs/screenshots/boot.png" width="720" alt="GS/OS95 boot screen">

*What if Redmond had shipped Windows 95 for the Apple IIgs in 1995?*

**[📖 User Manual](docs/Manual.md)** · **[📖 Benutzerhandbuch](docs/Handbuch.md)** · **[📜 Version history](docs/README_EN.txt)** · **[💾 Disk image](image/)**

</div>

---

## Table of contents

- [What is GS/OS95?](#what-is-gsos95)
- [Highlights](#highlights)
- [Screenshots](#screenshots)
- [Getting started](#getting-started)
  - [What you need](#what-you-need)
  - [Running it in an emulator](#running-it-in-an-emulator)
  - [Running it on a real Apple IIgs](#running-it-on-a-real-apple-iigs)
  - [First steps](#first-steps)
- [Feature tour](#feature-tour)
  - [Desktop and taskbar](#desktop-and-taskbar)
  - [Explorer](#explorer)
  - [Control Panel](#control-panel)
  - [Win95 look for other applications: GSOS95.INIT](#win95-look-for-other-applications-gsos95init)
  - [Networking](#networking)
  - [Web browser](#web-browser)
  - [Terminal, Telnet, SSH and IRC](#terminal-telnet-ssh-and-irc)
  - [MS-DOS prompt](#ms-dos-prompt)
  - [Paint](#paint)
  - [Games and accessories](#games-and-accessories)
  - [CD Player](#cd-player)
  - [Screen savers and the boss key](#screen-savers-and-the-boss-key)
- [Keyboard reference](#keyboard-reference)
- [Files on the disk](#files-on-the-disk)
- [How it works](#how-it-works)
  - [Boot chain](#boot-chain)
  - [Program layout](#program-layout)
  - [Network stack](#network-stack)
  - [Rendering](#rendering)
- [Building from source](#building-from-source)
- [Test suite](#test-suite)
- [Source code map](#source-code-map)
- [Version history](#version-history)
- [Known limits](#known-limits)
- [Roadmap and ideas](#roadmap-and-ideas)
- [FAQ](#faq)
- [Contributing](#contributing)
- [Credits](#credits)
- [A note on Apple files](#a-note-on-apple-files)

---

## What is GS/OS95?

GS/OS95 replaces the Finder of GS/OS with a complete 1995-style desktop. You get a Start menu, a
taskbar with a clock and a quick-launch bar, and Explorer windows with a folder tree. Deleted files go
to a Recycle Bin that remembers where they came from, and you can put shortcuts on the desktop. It
feels like Windows 95 running on 16-bit Apple hardware.

It also brings the IIgs online. GS/OS95 drives five kinds of network hardware itself and includes a web
browser, FTP and Gopher clients, a full-screen ANSI terminal with ZMODEM, SSH, IRC, an RSS reader, a
weather forecast and access to the Asimov software archive. Your Apple IIgs can download software from
the Internet and write it straight to its own disks.

Everything is written in **65816 assembly** for a stock 2.8 MHz Apple IIgs. The program is about
250 KB of hand-written code and data spread across five 64 KB code banks. It sits on top of GS/OS and
the Apple IIgs Toolbox, so desk accessories, control panels and ordinary GS/OS applications keep
working. With the companion system extension **GSOS95.INIT**, those applications get the Win95 look as
well.

Development and testing happen in the [GSSquared](https://github.com/jawaidbazyar2/gssquared)
emulator. A purpose-built 65816 test emulator runs scripted sessions against every build. Tests on
real hardware are under way.

---

## Highlights

| | |
|---|---|
| 🪟 **Real Win95 desktop** | Start menu with cascading submenus, taskbar buttons, minimize/maximize/close, Apple-Tab window switching, context menus, desktop shortcuts |
| 🗂️ **Explorer** | Icon and detail views, folder tree, sorting, six resizable windows, drag & drop, multi-select, copy/move with progress, find, properties with previews |
| 🗑️ **Recycle Bin** | With restore to the original location |
| 🎨 **System-wide theme** | GSOS95.INIT restyles title bars, buttons, check boxes and Open/Save dialogs of *all* GS/OS applications |
| 🌐 **Web browser** | HTTP/HTTPS, inline GIFs, tables, forms, cookies, downloads, bookmarks with folders, history, choice of search engine |
| 📡 **Five network options** | Uthernet II, CoGS, FujiNet, Marinetti (any link layer) and Wi-Fi modems, with drivers written from scratch |
| 🖥️ **Terminal** | 80×25 ANSI/VT100 with the CP437 character set, BBS phone book, ZMODEM/XMODEM/YMODEM, SSH |
| 📂 **FTP & Gopher** | Directory listings as clickable pages, downloads to `*:Downloads` |
| ⌨️ **MS-DOS prompt** | `DIR`, `TYPE`, `PING`, `NSLOOKUP`, `IPCONFIG`, `FINGER` and friends |
| 📝 **Notepad** | Edit text files with word wrap, umlauts and fast scrolling |
| 📅 **Date/Time** | Click the clock for a month calendar and an analog clock face |
| 🖌️ **Paint** | 12 tools including ellipses, fill, colour picker and text; grab pictures from web pages |
| 🃏 **Games** | Solitaire (Klondike, with the famous bouncing cards) and Minesweeper |
| 💿 **CD Player** | Audio CDs on SCSI CD-ROM drives, track names from CDDB |
| 💙 **Boss key** | A pixel-perfect Windows 95 blue screen on Option-Apple-C |

---

## Screenshots

> The screenshots show the German user interface. English is one click away in the Control Panel.

### Desktop

| | | |
|:---:|:---:|:---:|
| <img src="docs/screenshots/startmenu.png" width="280"><br>**Start menu with submenus** | <img src="docs/screenshots/explorer.png" width="280"><br>**Explorer with folder tree** | <img src="docs/screenshots/explorer_tree.png" width="280"><br>**Properties dialog** |
| <img src="docs/screenshots/settings.png" width="280"><br>**Control Panel** | <img src="docs/screenshots/network.png" width="280"><br>**Network settings** | <img src="docs/screenshots/help.png" width="280"><br>**Built-in help** |

### Internet

| | | |
|:---:|:---:|:---:|
| <img src="docs/screenshots/browser.png" width="280"><br>**Web browser start page** | <img src="docs/screenshots/browser_images.png" width="280"><br>**GIF images inline** | <img src="docs/screenshots/bookmarks.png" width="280"><br>**Bookmarks with folders** |
| <img src="docs/screenshots/ftp.png" width="280"><br>**FTP directory listing** | <img src="docs/screenshots/archive.png" width="280"><br>**Asimov software archive** | <img src="docs/screenshots/news.png" width="280"><br>**RSS news** |
| <img src="docs/screenshots/terminal.png" width="280"><br>**ANSI terminal with ZMODEM** | <img src="docs/screenshots/ssh.png" width="280"><br>**SSH over FujiNet** | <img src="docs/screenshots/msdos.png" width="280"><br>**MS-DOS prompt with FINGER** |

### Programs

| | | |
|:---:|:---:|:---:|
| <img src="docs/screenshots/paint.png" width="280"><br>**Paint** | <img src="docs/screenshots/paint_from_web.png" width="280"><br>**A web picture in Paint** | <img src="docs/screenshots/solitaire.png" width="280"><br>**Solitaire** |
| <img src="docs/screenshots/notepad.png" width="280"><br>**Notepad** | <img src="docs/screenshots/calendar.png" width="280"><br>**Date/Time with calendar** | <img src="docs/screenshots/calculator.png" width="280"><br>**Calculator** |
| <img src="docs/screenshots/cdplayer.png" width="280"><br>**CD Player** | <img src="docs/screenshots/screensaver.png" width="280"><br>**Mystify-style screen saver** | |

<div align="center">
<img src="docs/screenshots/bluescreen.png" width="480"><br>
<b>The boss key. Nothing actually crashes. 😉</b>
</div>

---

## Getting started

### What you need

| | Minimum | Notes |
|---|---|---|
| Computer | Apple IIgs, or an emulator | Developed and tested with GSSquared (ROM 3) |
| Memory | a few MB recommended | the browser takes an extra 64 KB bank, Paint about 72 KB |
| Storage | a 32 MB hard disk or image | the image is a ProDOS volume with GS/OS already installed |
| Network (optional) | one of the cards below | see [Networking](#networking) |
| CD-ROM (optional) | Apple or SCSI-2 CD-ROM drive | for the CD Player |

The disk image ships ready to boot. It contains GS/OS with the System 6.1 bug fixes, Marinetti 3.0, the
GS/OS95 desktop, the system extension and the boot screen.

### Running it in an emulator

1. Download [`image/GSOS95_v2147_System.hdv`](image/).
2. Mount it as a hard disk in **slot 7, drive 1** and boot.

**GSSquared** (recommended). For networking, add an Uthernet II card to your configuration:

```toml
[[cards]]
card = "uthernet2"
```

GSSquared answers no DHCP requests, so set fixed addresses in *Settings > Network*. The defaults with
DNS `1.1.1.1` work. Use GSSquared 1.0 or newer: earlier versions dropped packets when the card's buffer
was full.

**KEGS / GSplus / MAME** boot the image as well. For networking there, use Marinetti with the emulator's
link layer, or the Wi-Fi modem emulation of GSplus/KEGS.

> ⚠️ Do not modify the image from outside while it is mounted in a running emulator. That can damage
> the file system.

### Running it on a real Apple IIgs

Copy the image to a mass-storage device such as BlueSCSI, CFFA3000 or MicroDrive/Turbo and boot from
it. For networking, install one of the supported cards.

The FujiNet and the Wi-Fi modem drivers were developed against simulators built from the firmware
sources. Reports from real hardware are very welcome.

### First steps

1. After power-on, the boot screen with clouds and a progress bar appears right away. GS/OS loads its
   drivers, then GS/OS95 takes over and the bar runs to the end.
2. The desktop appears. Press **Tab** or click **Start**.
3. Switch the language under *Start > Settings*, using the "Deutsch"/"English" icon.
4. Open *My Computer* to browse your drives, or click the globe in the quick-launch bar to go online.
5. *Start > Help* lists all keyboard shortcuts.

---

## Feature tour

### Desktop and taskbar

- **Start menu**: Programs (grouped into Tools, Internet and Games, plus My Computer and your own
  programs), Documents (last 8 files), Accessories (all NDAs), Settings, Help, Run… and Shut Down….
  It can be used fully from the keyboard, and long submenus scroll.
- **Taskbar**: one button per window, plus desk accessories. Clicking the active button minimizes its
  window.
- **Quick-launch bar**: My Computer, Settings, Run, Minesweeper, Internet, Terminal and Show Desktop.
- **System tray**: a network status icon (blue means connected) and a clock. Click the clock for a
  Date/Time window with a month calendar and an analog clock face.
- **Windows**: move, resize with the grip, maximize (button or double-click on the title bar),
  minimize, close. Apple-Tab brings the next window to the front, and Esc closes the front window.
- **Desktop**: My Computer, Recycle Bin, Readme, up to six shortcuts, optional drive icons (Mac style),
  five background patterns and a custom desktop colour.
- **Context menus**: Option-click an item for Open, Cut, Copy, Delete, Rename, Properties, Send to
  desktop and Add to Start menu. Option-click empty space for Paste, New Folder, Refresh and View.
- **Session memory**: open folder windows, with their size and position, come back after a restart.
- **Start-up chime**: a short chord after power-on, which you can switch off.

### Explorer

- **Views**: large icons, or details with name, size and type. Sort by name, size or type, with folders
  always first.
- **Folder tree** on the left: the path, sibling folders at every level and all drives. A click goes
  there.
- **Up to six windows**; folders with up to 400 entries.
- **Selection**: Shift-click, Apple-click, Ctrl-A.
- **Drag & drop** onto folders, windows, drives or the Recycle Bin. On the same drive this moves the
  items; onto another drive it copies them.
- **Cut, copy and paste** of up to 16 items at once, with a progress dialog and Cancel.
- **File operations**: new folder, rename, duplicate, delete, find (recursive), lock and unlock, and
  changing the file type and auxiliary type.
- **Properties** show the first lines of text files and colour thumbnails of pictures.
- **Recycle Bin** remembers where its last 16 items came from and restores them. Emptying it shows
  progress, and Option skips the question.
- **Opening files**:
  - Programs (S16, SYS, EXE) and BASIC programs run, and you return to GS/OS95 when they quit.
  - Documents open with their application via the Finder icon files (`*:Icons`), using the Message
    Center just like Apple's Finder.
  - Pictures (`$C1`, PackBytes `$C0/0001`, Apple Preferred `$C0/0002`) open in a full-screen viewer.
  - Sounds (`$D8`, 8-bit AIFF or raw) play.
- **Volumes**: rename, erase, copy a whole disk block by block, and format unformatted disks. 5.25"
  drives are only read on demand, because an empty drive takes 20 seconds to answer.
- **System folder helper**: items dropped onto *System* can be sorted into CDevs, Desk.Accs, Fonts,
  Drivers, Tools, FSTs and System.Setup.

### Control Panel

| Panel | Settings |
|---|---|
| Display | desktop and border colour, background pattern, screen saver delay, drives on the desktop |
| Mouse | double-click speed, fast mouse |
| Date/Time | sets the IIgs clock |
| Sound | volume, pitch, start-up chime |
| Language | English or German, switched instantly |
| Network | card, slot, DHCP or fixed addresses, DNS, HTTPS proxy, Test button |
| More… | Apple's control panels for third-party CDevs |

Settings live in Battery RAM and in `*:System:GSOS95.PREFS` / `GSOS95.NET`.

### Win95 look for other applications: GSOS95.INIT

The system extension `*:System:System.Setup:GSOS95.INIT` patches the Toolbox at boot. It stays active
in every application, including Apple's own Finder:

- **Title bars**: title on the left, maximize and X buttons on the right, gray when inactive. This works
  in 320 and 640 mode with the closest colours of the current palette.
- **Open and Save dialogs** (Standard File: `SFGetFile2`/`SFPutFile2` and the older calls) are
  replaced by Explorer-style dialogs with "Look in", Up, New Folder, "Replace file?" and type-ahead.
  Hold Option to get Apple's original. Programs with customized dialogs keep theirs.
- **Push buttons and check boxes** in Win95 style. Only the drawing is replaced; hit-testing and keyboard
  handling stay Apple's.

Remove the file from System.Setup to switch it off.

### Networking

GS/OS95 brings its **own drivers** and does not need Marinetti, though it can use it.

| Card | Connection | http | https | SSH | FTP | Notes |
|---|---|:---:|:---:|:---:|:---:|---|
| **Uthernet II** (WIZnet W5100) | slot card, Ethernet | ✅ | via proxy | – | ✅ | driven directly; DHCP, DNS, SNTP; FTP uses a second hardware socket |
| **CoGS** | slot card, Wi-Fi/Ethernet | ✅ | ✅ | – | – | the card does TLS itself |
| **FujiNet** | SmartPort | ✅ | ✅ | ✅ | ✅ | `N:TCP`, `N:HTTPS`, `N:SSH`, `N:FTP` devices |
| **Marinetti 3.0** | any link layer | ✅ | via proxy | – | – | Uthernet I, LANceGS, SLIP/PPP, MacIP, emulators |
| **Wi-Fi modem** | serial port | ✅ | via proxy | – | – | WiModem232, Zimodem, WiFi232, GSplus/KEGS modem; 2400–57600 baud |

- **Automatic** detection tries CoGS, FujiNet, Marinetti and Uthernet II in turn.
- **HTTPS without a TLS-capable card** works through a proxy on your LAN, such as
  [WebOne](https://github.com/atauenis/webone) or [Macproxy](https://github.com/rdmark/macproxy). Set
  it in *Settings > Network* or with `about:proxy=host:port`.
- **Clock sync**: the IIgs clock is set from the Internet on the first page load, using SNTP or the
  Time protocol, with time zones and DST rules (`about:tz=+1eu`, `about:tz=-5us`, `about:tz=off`).
- **Status**: `about:net` shows card, IP, DNS and a connection test with its round-trip time. The tray
  icon shows the link state.

### Web browser

**Rendering**
- HTTP/1.0 with redirects, chunked transfer and cookies.
- Pages of any size are streamed; up to 16 KB of text is shown per page.
- Headings, paragraphs, lists, block quotes, tables with measured columns, `<pre>` and rules.
- UTF-8, Latin-1 and Windows-1252 with HTML entities. Scripts and styles are skipped.
- **Inline GIF images** (up to 8 per page), mapped to the 16 desktop colours. Click one to view it full
  size.

**Forms:** text fields with a real cursor, hidden fields, check boxes, buttons, `<select>` and
`<textarea>`; GET and POST.

**Protocols:** `http`, `https`, `gopher`, `ftp`, `telnet`, `ssh`, `irc`.

**Bookmarks and history**
- Ctrl-D bookmarks the page into a folder, and `about:lz` manages bookmarks.
- Any bookmark can become the home page.
- The last 20 pages are listed at `about:verlauf`.
- The search engine is your choice: FrogFind (which makes modern sites readable), wiby.me or the
  DuckDuckGo HTML version.

**Downloads:** anything that is not a page goes to `*:Downloads` with a ProDOS file type derived from
its extension (`.shk`, `.bxy`, `.2mg`, `.gif`, `.txt`, `.s16` …). Ctrl-S saves the current page.

**FTP:** `ftp://host/path/` shows a directory as a page with folders, sizes and an up link. Click a file
to download it. It parses Unix, DOS/IIS and plain name listings.

**Picture to Paint:** Ctrl-P opens the page's first picture in Paint.

**Built-in pages**

| Page | What it does |
|---|---|
| `about:start` | start page with sites that work without https |
| `about:lz` | bookmarks, folders, search engine |
| `about:verlauf` | history |
| `about:wetter` | weather for a town (data from wttr.in) |
| `about:archiv` | the Asimov Apple II archive as a browsable list |
| `about:rss` | RSS/Atom news feeds, including a "without https" section |
| `about:bbs` | BBS phone book |
| `about:msdos` | MS-DOS prompt |
| `about:net` | network status |
| `about:keys` | keyboard shortcuts |
| `about:update` | checks GitHub for a new GS/OS95 version (needs https: CoGS, FujiNet or a proxy) |
| `about:cddb` | CD track name server settings |
| `about:proxy=…`, `about:tz=…` | proxy and time zone |

### Terminal, Telnet, SSH and IRC

- **Full-screen terminal** with 80 × 25 characters, the PC character set (CP437 box and block
  graphics) and 16 ANSI colours.
  - Colour mode (320 pixels, 4-pixel font) or sharp grayscale mode (640 pixels, 8×8 font); Apple-F
    switches.
  - ANSI/VT100: cursor addressing, erase, scroll regions, insert/delete, save/restore and cursor
    position reports.
  - Telnet negotiation (terminal type ANSI, window size 80×25, character mode).
- **BBS phone book** with up to 8 entries, plus "connect directly". It comes pre-filled with Captain's
  Quarters and Electronic Chicken.
- **File transfer**: ZMODEM downloads start automatically. XMODEM (128/1K, CRC/checksum) and YMODEM
  batch receive with Apple-D, XMODEM send with Apple-U.
- **SSH** through the FujiNet: `ssh://user@host`. The password prompt appears in the terminal.
- **IRC**: `irc://host/channel` in the browser window. It logs in, answers PING and joins the channel;
  `/msg`, `/join`, `/nick` and `/quit` work.

### MS-DOS prompt

A full-screen `C:\>` prompt where drive C: is the boot volume. It is just for fun: it never changes
anything on disk.

| Command | |
|---|---|
| `DIR`, `CD`, `CD ..`, `CD \` | browse the disk |
| `TYPE file` | page through a text file |
| `CLS`, `COLOR 1E`, `ECHO` | screen |
| `DATE`, `TIME`, `MEM`, `VER` | system info, DOS style |
| `PING host` | four TCP connects to port 80 with timings (the cards have no ICMP) |
| `NSLOOKUP host` | DNS lookup |
| `IPCONFIG`, `WINIPCFG` | card, IP, mask, gateway, DNS, DHCP, MAC |
| `FINGER user@host` | the classic finger protocol |
| `HELP`, `EXIT` | help, back to the desktop |

### Paint

320 × 200 pixels in the 16 desktop colours, saved as standard `$C1` super hi-res pictures.

| Tools | Keys |
|---|---|
| pen, brush, eraser, spray can | **S** save |
| line, rectangle, filled rectangle | **Z** undo |
| ellipse, filled ellipse | **N** new picture |
| flood fill | arrows scroll the view |
| colour picker | **Esc** close (asks if unsaved) |
| text | |

Open any picture from the picture viewer with **B**, or grab a web picture with **Ctrl-P** in the
browser.

### Notepad

A small text editor for TXT and SRC files (double-click in the Explorer, or *Start > Programs > Tools >
Notepad*):

- Files up to 32,000 characters, word wrap at the window edge, resizable window.
- LF and CR LF line endings are converted to ProDOS CR on load.
- Mac Roman and Latin-1 umlauts are displayed, and umlauts can be typed on the keyboard.
- Keys: arrows, Apple-arrows (line start/end, page up/down), Option-arrows (word), Delete, Ctrl-D,
  and a click to place the cursor.
- Scrolling moves the screen with the 65816 block-move instruction.
- Apple-S saves; new texts ask for a path in the status bar. Apple-N starts a new text. Esc closes and
  asks first if there are unsaved changes.

### Games and accessories

- **Solitaire**: Klondike as in Windows. Drag cards, double-click to a foundation, draw one or three,
  undo, score and timer, and the bouncing-card victory animation.
- **Minesweeper**: 9 × 9 with 10 mines and a safe first click. Smiley button, mine counter, timer and
  saved best time.
- **Calculator**: 16 digits, percent, square root, square, 1/x, factorial, pi and memory keys.
- **Desk accessories** (NDAs) run in windows with a Win95 frame and get keyboard focus correctly.

### CD Player

Plays audio CDs on SCSI CD-ROM drives. It supports Apple's AppleCD SC, SC Plus and CD 150 with their
own commands, and AppleCD 300/600 and other SCSI-2 drives through the standard audio commands. Features:

- Track buttons, time display, play/pause/stop, previous/next track and eject.
- An inserted CD starts playing by itself (AutoPlay).
- Artist, album and track names come from **CDDB** over HTTP.

The patched SCSI CD driver on the image adds SCSI-2 "no disc" handling and fixes a hang after a disc
change.

### Screen savers and the boss key

- Two screen savers, a **Starfield** and **Mystify-style lines**, take turns after 1–15 minutes
  (off by default).
- **Option-Apple-C** (or Ctrl-Apple-C in emulators without an Option key) shows a Windows 95 style
  "fatal exception 0E" blue screen. Any key brings the desktop back.

---

## Keyboard reference

| Context | Key | Action |
|---|---|---|
| Notepad | Apple-S / Apple-N / Esc | save / new / close |
| | Apple-arrows, Option-arrows, Ctrl-D | line & page, word, delete right |
| Everywhere | Tab | Start menu |
| | Apple-Tab | next window |
| | Esc | close window, menu, dialog or desk accessory |
| | Option-Apple-C, Ctrl-Apple-C | boss key |
| Explorer | Return, Apple-Down | open |
| | Delete, Apple-Up | parent folder |
| | Ctrl-N / Ctrl-E / Ctrl-D / Ctrl-P | new folder / rename / delete / properties |
| | Ctrl-X / Ctrl-C / Ctrl-V / Ctrl-A | cut / copy / paste / select all |
| | Ctrl-R / Ctrl-L | refresh / eject |
| Text fields | Option-arrow | word left/right |
| | Apple-arrow, Ctrl-A / Ctrl-E | start / end |
| Browser | Ctrl-R / Ctrl-D / Ctrl-B / Ctrl-S / Ctrl-P | reload / bookmark / bookmarks / save / picture to Paint |
| | Left arrow, space, arrows | back, page, scroll |
| Terminal | Apple-F | colour / grayscale |
| | Apple-D / Apple-U | receive / send file |
| | Apple-Esc, Ctrl-] | hang up |
| Solitaire | N, U, D, A | new, undo, draw 1/3, auto-move |
| CD Player | space, S, E, L, I | play/pause, stop, eject, next drive, track names |

---

## Files on the disk

| Path | Purpose |
|---|---|
| `*:System:GSOS95` | the desktop application (OMF load file, 428 KB) |
| `*:System:Start.GS.OS` | GS/OS loader with the GS/OS95 boot screen |
| `*:System:Boot95`, `*:System:GSOS95.SPL` | boot picture shown right after power-on, progress bar |
| `*:System:System.Setup:GSOS95.INIT` | Win95 look for all applications |
| `*:System:System.Setup:SPLASH95` | keeps the boot picture on screen while extensions load |
| `*:System:GSOS95.PREFS` | settings, windows, view options |
| `*:System:GSOS95.NET` | network settings, BBS phone book, home page |
| `*:System:GSOS95.LZ` | bookmarks and history |
| `*:System:GSOS95.LNK` | desktop shortcuts |
| `*:System:GSOS95.BIN` | Recycle Bin origins |
| `*:Downloads` | downloaded files |

---

## How it works

### Boot chain

```
power-on ─▶ PRODOS ─▶ *:System:Boot95          boot picture on screen immediately
                     ─▶ Start.GS.OS (patched)   picture kept, progress bar drawn
                         ─▶ GS/OS, drivers, System.Setup
                             ├─ SPLASH95        temporary init: keeps the picture during init loading
                             └─ GSOS95.INIT     permanent init: system-wide Win95 look
                         ─▶ *:System:GSOS95      the desktop; takes over the progress bar seamlessly
```

The progress bar is handed over without flicker. GS/OS95 detects the boot picture's palette and the
bar's frame and continues from the exact position.

### Program layout

GS/OS95 is a single GS/OS application (`S16`) made of six OMF segments:

| Segment | Size | Contents |
|---|---|---|
| 1 Main | 64 KB incl. BSS | event loop, windows, Explorer core, desktop, Toolbox glue |
| 2 GFX + XCODE | 64 KB | icons, fonts, sprites, Finder functions, documents |
| 3 XCODE2 | 64 KB | web browser, HTML engine, Start menu, UI strings |
| 4 XCODE3 | 64 KB | network drivers, terminal, calculator, Solitaire, CD Player |
| 5 XCODE4 | 64 KB | MS-DOS, SSH, Paint, FTP, bookmarks, screen saver, blue screen |
| 6 Stack | 16 KB | direct page and stack |

Code in different banks calls each other through generated far-call tables (`JSLF xf_…`) and
trampolines (`CALL`). The OMF file carries about 23,000 relocation records. Compact SUPER records
(`OMF_SUPER=1 python3 mkomf2.py`) would shrink it from 428 KB to 278 KB, but the build with them
(2.1.45) did not find the network card under real GS/OS, so the classic records are back for now.

### Network stack

The network layer exposes one small API (`net_start`, `resolve`, `tcp_open`, `tcp_write`,
`tcp_readall`, `tcp_close`) with one driver per card:

- **W5100**: talks to the WIZnet chip's registers directly, does DHCP and DNS over UDP, and uses
  socket 0 for TCP. Socket 1 serves UDP and the FTP data connection.
- **CoGS**: a command interface to the card's Raspberry Pi Pico 2 W, with TLS on the card.
- **FujiNet**: SmartPort `NETWORK` device calls (`N:TCP://`, `N:HTTPS://`, `N:SSH://`, `N:FTP://`).
- **Marinetti**: TCP/IP Toolset (tool `$36`) calls.
- **Modem**: Hayes `ATDT host:port`, with `NO CARRIER` as the end of data.

### Rendering

GS/OS95 draws its own UI directly into super hi-res memory. It uses fast routines for text, icons and
lines, and an off-screen buffer for window redraws so windows update without flicker. QuickDraw II
and the Window Manager are still started for desk accessories and the Toolbox.

---

## Building from source

**Requirements:** Python 3 (Pillow only for regenerating the boot picture) and [cc65](https://github.com/cc65/cc65)
(`ca65`, `ld65`).

```sh
git clone https://github.com/8-Bit-Shack/gsos95.git
cd gsos95/src
export CC65_BIN=/path/to/cc65/bin   # directory containing ca65 and ld65

python3 mkwebstr.py                 # browser strings (after changing src/BUILD)
sh buildapp.sh                      # -> GSOS95.S16 (the application SYSTEM:GSOS95)
sh mkboot61.sh                      # boot screen, BOOT95, START.GS.OS, GSOS95.SPL, SPLASH95
```

`buildapp.sh` checks far calls (`farcheck.py`), generates the assets, assembles `app.s` and links the
five code banks with `mkomf2.py`. The linker runs once per bank with shifted base addresses, and the
differences between those links yield the relocation table. If a branch is out of range, run
`python3 tools/fixbr.py`.

**Updating the disk image** (always start from a copy of the current image):

```sh
cp ../image/GSOS95_v2147_System.hdv new.hdv
python3 update_app.py new.hdv GSOS95.S16
python3 tools/update_file.py new.hdv SYSTEM/START.GS.OS sys/START.GS.OS.95
python3 tools/update_file.py new.hdv SYSTEM/GSOS95.SPL sys/GSOS95.SPL
python3 tools/update_file.py new.hdv SYSTEM/BOOT95 boot95x/BOOT95
python3 tools/update_file.py new.hdv SYSTEM/SYSTEM.SETUP/SPLASH95 init95/SPLASH95
python3 fsck.py new.hdv             # consistency check of the ProDOS volume
```

`tools/pdtool.py` is a small ProDOS library for reading and writing files, including tree files, inside
`.hdv`/`.po`/`.2mg` images.

---

## Test suite

`src/t3.py` and `src/emu.py` implement a 65816 CPU emulator with a high-level simulation of the
Toolbox and GS/OS calls GS/OS95 uses. The real build loads into it, and test scripts drive it with
mouse and keyboard events, inspect memory and render screenshots.

Simulated hardware:

| Simulator | Simulates |
|---|---|
| `w5100sim.py` | Uthernet II with scripted TCP/UDP servers (HTTP, DNS, DHCP, FTP …) |
| `cogssim.py` | CoGS card including TLS |
| `fujisim.py` | FujiNet SmartPort device incl. HTTPS, SSH and FTP |
| `marisim.py` | Marinetti TCP/IP Toolset |
| `modemsim.py` | Hayes Wi-Fi modem |
| `cdsim.py` | SCSI CD-ROM drive |

There are about 120 test scripts. Some examples:

```sh
cd src
python3 tw1.py      # browser basics over Uthernet II
python3 tftp.py     # FTP with a fake FTP server: login, LIST, CWD, RETR, errors
python3 tlz.py      # bookmarks, folders, history, search engine, weather
python3 tpaint.py   # Paint tools
python3 tsol.py     # Solitaire
python3 tdosnet.py  # MS-DOS network commands
GSOS95_IMG=../image/GSOS95_v2147_System.hdv python3 tearly.py   # boot path inside the image
```

Every test prints `ok …` lines and finishes with `ALLES OK` (German for "all OK") or `FEHLER` ("error").

---

## Source code map

<details>
<summary><b>Desktop and Explorer</b></summary>

| File | Contents |
|---|---|
| `app.s` | main program: start-up, event loop, includes everything |
| `windows.s` | window stack, moving, taskbar |
| `geom.s` | resizing, scroll bars, maximize |
| `finder.s`, `ext.s`, `x3ext.inc` | Explorer functions: menus, folders, rename, Recycle Bin |
| `tree.s` | folder tree |
| `listview.s` | details view and sorting |
| `dragdrop.s`, `multisel.s` | drag & drop, multi-select |
| `prog.s` | copy/move progress dialog |
| `preview.s` | text preview in Properties |
| `docs.s` | opening documents, picture viewer |
| `icons.s` | Finder icon files (`$CA`) |
| `links.s`, `plist.s` | desktop shortcuts, Start menu lists |
| `startsub.s` | Start menu submenus |
| `settings.s` | Control Panel |
| `desk.s`, `wdef.s` | desk accessories and their Win95 frame |
| `more.s` | BASIC programs, volume copy, formatting |
| `snd.s`, `ssv.inc` | start-up chime, screen savers |
| `deskpat.s` | desktop patterns |
| `mines.s` | Minesweeper |
| `bsod.inc` | blue screen |

</details>

<details>
<summary><b>Internet</b></summary>

| File | Contents |
|---|---|
| `web.s` | browser window, page loading, keys |
| `url.inc` | URL parsing, HTTP request and header |
| `html.inc` | HTML to formatted text |
| `img.inc`, `gif.inc` | inline images, GIF decoder |
| `forms.inc`, `lineed.inc` | forms, text field editor |
| `cookie.inc`, `proxy.inc` | cookies, HTTPS proxy |
| `dl.inc` | downloads |
| `fav.inc`, `lz.inc` | about: pages, bookmarks, history, search, weather |
| `gopher.inc`, `ftp.inc` | Gopher, FTP |
| `pfilt.inc` | Asimov archive and RSS filters |
| `term.inc`, `vt.inc`, `xfer.inc`, `bbs.inc` | Telnet/IRC, ANSI terminal, file transfer, phone book |
| `ssh.inc` | SSH |
| `net.s` | network API and Uthernet II driver |
| `cogs.inc`, `fujinet.inc`, `mari.inc`, `modem.inc` | other card drivers |
| `netdlg.inc`, `ntp.inc` | network dialog, clock sync |

</details>

<details>
<summary><b>Programs</b></summary>

| File | Contents |
|---|---|
| `dos.inc`, `dosnet.inc` | MS-DOS prompt |
| `paint.inc` | Paint |
| `calc.inc` | Calculator |
| `sol.inc` | Solitaire |
| `cd.inc`, `cddb.inc` | CD Player, CDDB |
| `ed.inc` | Notepad |
| `cal.inc` | Date/Time with calendar |

</details>

<details>
<summary><b>Boot, system extension, tools</b></summary>

| File | Contents |
|---|---|
| `boot95x/boot95x.s` | `*:System:Boot95`, the picture right after power-on |
| `boot95/boot95.s`, `boot95/early.s`, `boot95/mksplash.py` | boot screen inside Start.GS.OS, picture generator |
| `init95/init95.s`, `sf95.s`, `ctl95.s` | GSOS95.INIT: title bars, Open/Save dialogs, controls |
| `init95/spl95.s` | SPLASH95 temporary init |
| `mkomf2.py` | OMF linker with SUPER relocations |
| `mkboot61.sh`, `tools/*.py` | boot files, ProDOS image tools, System 6.1 patching |
| `assets.py`, `mkicons.py`, `font*.py` | icons, fonts and sprites |
| `mkwebstr.py` | browser and help strings (German/English) |

</details>

---

## Version history

| Version | Highlights |
|---|---|
| **2.0** | Desk accessories, drag & drop, volume rename/erase, custom file icons |
| **2.3–2.4** | Documents open with their application, details view, multi-select, find, Control Panel, BASIC launch, volume copy |
| **2.5** | Resizable windows, six windows, progress dialogs, context menus, desktop shortcuts, Start submenus, start-up chime, screen saver, new boot picture |
| **2.6** | Minimize, Show Desktop, folder tree, Win95 Minesweeper, picture viewer for APF |
| **2.7–2.9** | GSOS95.INIT: Win95 title bars, Open/Save dialogs, buttons and check boxes for all applications |
| **2.10** | Internet: Uthernet II and CoGS drivers, web browser, Gopher, Telnet, IRC, HTTPS, clock sync |
| **2.11** | Full-screen ANSI terminal, BBS phone book |
| **2.12–2.13** | FujiNet, Marinetti and Wi-Fi modem support, network tray icon, cookies |
| **2.14** | Text cursor editing, sound playback, help page, update check, new boot screen by Daniel |
| **2.15–2.16** | CD Player with CDDB track names |
| **2.17** | System 6.1 bug fixes integrated, instant boot picture |
| **2.18** | Software archive, ZMODEM/XMODEM/YMODEM, RSS news, Calculator, Solitaire; builds 2.1.40–2.1.45 added the MS-DOS prompt, Paint, SSH, Mystify, blue screen, FTP, bookmarks with folders, history, weather, DOS network commands and a SUPER relocations tried and reverted (2.1.46) |

The full history is in [docs/README_EN.txt](docs/README_EN.txt) (English) and
[docs/LIESMICH.txt](docs/LIESMICH.txt) (German).

---

## Known limits

- The browser has no JavaScript, no CSS, no JPEG/PNG images and no images beside text. Sites made for
  vintage computers work best: FrogFind, 68k.news, wiby.me, textfiles.com.
- HTTPS needs a CoGS or FujiNet card, or else a proxy. SSH needs a FujiNet. FTP needs an Uthernet II
  or a FujiNet.
- In other applications, radio buttons, scroll bars and menus keep Apple's look.
- 5.25" drives are not listed in the Open/Save dialogs, and an empty 5.25" drive takes about 20 seconds
  to answer.
- No ZMODEM upload yet.

---

## Roadmap and ideas

- FTP for CoGS and Marinetti (a second simultaneous connection)
- An optional, rare "simulated crash" for the full 1995 experience (off by default)
- FreeCell and a system monitor
- Faster text drawing in Notepad (typing in long lines takes about a quarter of a second at 2.8 MHz)
- E-mail (POP3/SMTP) and Usenet (NNTP)
- More testing on real hardware

---

## FAQ

**Does it replace GS/OS?**
No. GS/OS95 is a GS/OS application that replaces the *Finder*. GS/OS, the Toolbox, desk accessories,
control panels and applications all keep working. *Shut Down > GS/OS mode* switches to Apple's Finder.

**Will my applications still work?**
Yes. Applications start from the Explorer and you return to GS/OS95 when they quit. ProDOS 8 programs
(SYS) work too.

**Is it slow on a 2.8 MHz machine?**
GS/OS95 draws its UI with its own optimized routines instead of QuickDraw. The start-up time is mostly
GS/OS loading; GS/OS95's own initialization takes about half a second.

**Why German?**
GS/OS95 started as a German project. All UI text exists in both languages and the switch is instant.
The source comments are in German.

**Can I use only the Win95 look without the desktop?**
Probably. `GSOS95.INIT` is a standalone system extension: copy it into `*:System:System.Setup`. It was
developed on the GS/OS 6.0.4 system files with the 6.1 fixes on this image; other setups are untested.

---

## Contributing

Bug reports, test results from real hardware and pull requests are welcome. Please:

- Run the relevant `src/t*.py` tests before submitting.
- Keep new features in the code bank with free space (XCODE4 currently has the most room).
- Describe your emulator or hardware setup in bug reports (machine, ROM, RAM, cards).

---

## Credits

- **Marc Golombeck**: creator of GS/OS95
- **Daniel**: boot screen design
- **Brutal Deluxe**: SCSI CD driver with SCSI-2 audio support (patched for GS/OS95)
- **luma.core**: 8×8 PC font (MIT license)
- Thanks to the authors of GSSquared, Marinetti, FujiNet, CoGS and the Apple II community

---

## A note on Apple files

The disk image contains parts of Apple's GS/OS. `src/sys61`, `src/sys` and `src/drv` contain Apple
files (ProDOS, START.GS.OS, the SCSI CD driver), as originals or patched, that the build needs. These
files remain the property of their respective owners and are not covered by this project's license.

<div align="center">
<br>
<i>Made with ❤️ and 65816 assembly.</i>
</div>
