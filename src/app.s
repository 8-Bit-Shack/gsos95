; GS/OS95 - Oberflaeche im Stil von 1995 als GS/OS-Programm (S16)
; Nutzt Toolbox (Memory/Misc/QuickDraw II/Event Manager) und GS/OS-Dateiaufrufe.
        .p816
        .smart +
        .a16
        .i16

; Farbnummern wie in Apples Standardpalette (NDAs zeichnen damit)
BLACK  = 0
TEAL   = 12
NAVY   = 13
DKGRAY = 1
GRAY   = 14
WHITE  = 15
YELLOW = 9
ORANGE = 6

; ---------- Direct Page (vom Loader zugeteilt) ----------
FX      = $00
FY      = $02
FW      = $04
FH      = $06
DXO     = $08
DYO     = $0A
ENTBASE = $0C
TMPK    = $0E
USERID  = $10
MYBANK  = $12
GSNUM   = $14
GSPB    = $16
MODE    = $18   ; 0 = Laufwerke, 1 = Ordner
COUNT   = $1A
PAGE    = $1C
SEL2    = $1E
STRP    = $20
STRI    = $22
STRN    = $24
CX      = $26
IDX     = $28
SLOT    = $2A
HND     = $2C   ; 4 Bytes
EVREC   = $30   ; 16 Bytes
QDDP    = $40
EVPTR   = $42
EP      = $44   ; 3 Bytes: Long-Zeiger auf Eintrag
TCNT    = $48
ERRMSG  = $4A
DIGS    = $4C
LASTWHAT= $4E
PREVWIN = $50
KEYC    = $52
LANG    = $54
VOLOK   = $56
VOLN    = $58
EP2     = $5A   ; 3 Bytes: zweiter Eintragszeiger (lang)

PB      = $60   ; Zeiger auf Pfadpuffer
PB2     = $62
DROP    = $64   ; offenes Fenstermenue (0 = keins, 1..4)
DHOVER  = $66
ISBIN   = $68
ACT     = $6A
TXTMODE = $6C   ; 0 = umbenennen, 1 = neuer Ordner
ERRC    = $6E
LP      = $70   ; Long-Zeiger in das Asset-Segment
LP2     = $74
DEPTH   = $7A
CONFACT = $7C   ; 1 = Papierkorb, 2 = endgueltig, 3 = leeren
FCOUNT  = $7E

PSRC    = $D6   ; 4 Bytes: Bilder laden, Quelle
PDST    = $EC   ; 4 Bytes: Ziel
CBP     = $F0   ; 3 Bytes: Zeiger auf Kopierpuffer (andere Bank)
VLEN    = $F4
VOFF    = $F6
VLN     = $F8
VCNT    = $FA
LCLKT   = $FC   ; Zeit des letzten Klicks
LCLKI   = $FE   ; Element des letzten Klicks
PTR     = $80
RX      = $84
RY      = $86
RW      = $88
RH      = $8A
COLLO   = $8C
COLHI   = $8E
COLBYTE = $90
SX      = $92
SY      = $94
SPR     = $96
W       = $98
H       = $9A
BPR     = $9C
ROW     = $9E
COL     = $A0
BITS    = $A2
DPTR    = $A4
TMP     = $A6
TMP2    = $A8
MX      = $AA
MY      = $AC
SAVB    = $AE
SAVY    = $B0
MENU    = $B2
WIN     = $B4
SEL     = $B6
HOVER   = $B8
CURVIS  = $BC
C1      = $BE
C2      = $C0
XE      = $C2
XS      = $C4
YC      = $C6
FRAMES  = $C8
VBLPREV = $CA
LASTMIN = $CC
HOUR    = $CE
MINUTE  = $D0
XFDIR   = $D2
SHUT    = $D4
BX      = $DA
BY      = $DC
BW      = $DE
BH      = $E0
CYC     = $E2
ITEM    = $E4
IY      = $E6
YEND    = $E8
CLKOK   = $EA

DVSEL = 16                      ; Auswahlnummer des ersten Desktop-Laufwerks
WX = 40
WY = 16
WW = 240
WH = 160
MNX = 2
MNW = 112
MNH = 7*18+6+6
MNY = 186-MNH
ICX = 20
GX = WX+6
TREEW = 72                      ; Breite des Ordnerbaums
GY = WY+32
CELLW = 72
CELLH = 27
PERPAGE = 12
MAXENT = 400
DPMAX  = 16
ENTSZ = 48
; Eintrag: +0 Typ, +2 Namenslaenge, +3 Name (31), +34 Groesse (4), +38 Zusatztyp,
; +40 Finder-Symbol (0 = unbekannt, $FFFF = keins, sonst Nr.+1),
; +42 Datum: Minute, Stunde, Jahr, Tag, Monat (je 1 Byte)
E_EOF  = 34
E_AUX  = 38
E_ICN  = 40
E_DATE = 42

; ---------- Fernaufrufe in die zweite Code-Bank (Segment XCODE) ----------
FARMODE .set 0
WDEF_ON = 1
.macro JSLF target
:       .byte $22
        .addr target
        .byte 0
        .pushseg
  .if FARMODE = 4
        .segment "FXE"
  .elseif FARMODE = 3
        .segment "FXD"
  .elseif FARMODE = 2
        .segment "FXC"
  .elseif FARMODE
        .segment "FXB"
  .else
        .segment "FXA"
  .endif
        .addr :-
        .popseg
.endmacro
; Unterprogramm der Hauptbank aufrufen (im XCODE ueber Sprungstelle tp_name)
.macro CALL r
  .if FARMODE
        JSLF .ident(.concat("tp_", .string(r)))
  .else
        jsr r
  .endif
.endmacro
.macro TOOL num
        tdc                     ; Toolbox (und NDAs) mit eigener Direct Page
        sta SAVD
        lda SCRDP
        tcd
        ldx #num
        jsl $E10000
        php
        pha
        lda SAVD
        tcd
        pla
        plp
.endmacro
.macro GSOS num,pb
        lda #num
        ldx #pb
        CALL gsos
.endmacro
.macro RECT xx,yy,ww,hh
        lda #xx
        sta RX
        lda #yy
        sta RY
        lda #ww
        sta RW
        lda #hh
        sta RH
.endmacro
.macro FILL xx,yy,ww,hh,cc
        RECT xx,yy,ww,hh
        lda #cc
        CALL setcolor
        CALL fillrect
.endmacro
.macro BEVEL xx,yy,ww,hh,c1,c2
        RECT xx,yy,ww,hh
        lda #c1
        sta C1
        lda #c2
        sta C2
        CALL bevel
.endmacro
.macro BUTTON xx,yy,ww,hh
        FILL xx,yy,ww,hh,GRAY
        BEVEL xx,yy,ww,hh,WHITE,BLACK
        BEVEL xx+1,yy+1,ww-2,hh-2,GRAY,DKGRAY
.endmacro
.macro PRESSED xx,yy,ww,hh
        FILL xx,yy,ww,hh,GRAY
        BEVEL xx,yy,ww,hh,BLACK,WHITE
        BEVEL xx+1,yy+1,ww-2,hh-2,DKGRAY,GRAY
.endmacro
.macro SUNKEN xx,yy,ww,hh
        BEVEL xx,yy,ww,hh,DKGRAY,WHITE
        BEVEL xx+1,yy+1,ww-2,hh-2,BLACK,GRAY
.endmacro
.macro LSPR lbl
        lda #lbl
        ldx LANG
        beq :+
        lda #.ident(.concat(.string(lbl),"_en"))
:       sta SPR
.endmacro
.macro TEXT xx,yy,lbl,cc
        lda #xx
        sta SX
        lda #yy
        sta SY
        LSPR lbl
        lda #cc
        CALL setcolor
        CALL blit1
.endmacro
.macro TEXTC cx,yy,lbl,cc
        lda #cx
        sta CX
        lda #yy
        sta SY
        LSPR lbl
        lda #cc
        CALL setcolor
        CALL blit1c
.endmacro
.macro ICON xx,yy,lbl
        lda #xx
        sta SX
        lda #yy
        sta SY
        lda #lbl
        sta SPR
        CALL blit4
.endmacro
.macro WS lbl
        lda #lbl
        ldx LANG
        beq :+
        lda #.ident(.concat(.string(lbl),"_en"))
:
.endmacro
.macro HIT xx,yy,ww,hh
        RECT xx,yy,ww,hh
        CALL inrect
.endmacro

        .segment "CODE"
codebase:
start:
        phk
        plb
        sta USERID
        sep #$20
        phk
        pla
        rep #$20
        and #$00FF
        sta MYBANK
        tdc
        sta MYDP
        sta SCRDP
        jsr clip_full
        stz NDAON
        stz NDAWAS
        stz OFFON
        stz NEEDREF
        stz INCB
        stz REFR
        lda abank_ref+2
        and #$00FF
        sta ABANK
        ; Bank in alle Zeiger der Parameterbloecke eintragen
        ldy #0
@pf:    ldx ptrfix,y
        beq @pfd
        lda MYBANK
        sta a:$0000,x
        iny
        iny
        bra @pf
@pfd:
        ; ---- Toolbox starten ----
        stz SPLON
        stz SPLEND
        JSLF xf_net_boot
        JSLF xf_strbk           ; Bank der Browser-Texte (fuer net.s)
        jsr boot_probe          ; Bootbild noch da? Stand des Ladebalkens merken
        TOOL $0201              ; TLStartUp
        pha
        TOOL $0202              ; MMStartUp
        pla
        sta USERID
        TOOL $0203              ; MTStartUp
        JSLF xf_spl_save        ; Bootbild sichern, Farben schwarz (vor QDStartUp)
        ; 4 Seiten Direct Page in Bank 0 fuer QuickDraw II + Event Manager
        pha
        pha
        pea $0000
        pea $0400
        lda USERID
        pha
        pea $C015
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta HND
        pla
        sta HND+2
        lda [HND]
        sta QDDP
        ; Zeilentabelle
        lda #$00E1
        sta PTR+2
        ldx #0
        lda #$2000
@lt:    sta linetab,x
        clc
        adc #160
        inx
        inx
        cpx #400
        bcc @lt
        stz MENU
        stz WIN
        stz SHUT
        stz CURVIS
        stz CYC
        stz CLKOK
        stz MODE
        stz COUNT
        stz LASTWHAT
        stz VOLOK
        stz CLRCELL
        stz DROP
        stz ISBIN
        stz clipok
        stz ORGDEP
        stz WAITOFF
        stz SUBM
        lda #320
        sta CLIPR
        jsr win_closeall
        stz WSEQ
        lda #NAVY
        sta ACTW
        lda QDDP                ; Direct Page fuer QuickDraw (3 Seiten aus dem Handle)
        pha                     ; dPageAddr
        pea $0000               ; masterSCB (320er Modus)
        pea $0000               ; maxWidth
        lda USERID
        pha
        TOOL $0204              ; QDStartUp
        ldx #0
@pl:    lda palettes,x
        sta f:$E19E00,x
        inx
        inx
        cpx #512
        bcc @pl
        jsr splash              ; sofort wieder das Startbild (QDStartUp loescht es)
        lda QDDP
        clc
        adc #$0300
        pha
        pea 20
        pea 0
        pea 319
        pea 0
        pea 199
        lda USERID
        pha
        TOOL $0206              ; EMStartUp
        TOOL $9004              ; HideCursor (wir zeichnen unseren eigenen)
        tdc
        clc
        adc #EVREC
        sta EVPTR
        ; Kopierpuffer (8 KB) vom Memory Manager
        pha
        pha
        pea $0000
        pea COPYBUF
        lda USERID
        pha
        pea $C010
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta HND
        pla
        sta HND+2
        ldy #2
        lda [HND],y
        sta io_pb+6
        lda [HND]
        sta io_pb+4
        sta CBP
        lda io_pb+6
        sta CBP+2
        ; Eintragspuffer der Ordnerfenster, je ein Block ohne Bankgrenze
        ldx #0
@sa:    phx
        pha
        pha
        pea ^(MAXENT*ENTSZ)
        pea .loword(MAXENT*ENTSZ)
        lda USERID
        pha
        pea $C010               ; fest, gesperrt, keine Bankgrenze
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        plx
        stx HND
        plx
        stx HND+2
        plx
        bcs @sf                 ; kein Speicher: Slot sperren
        ldy #2
        lda [HND],y
        and #$00FF
        sta slotbank,x
        lda [HND]
        sta slotbase,x
        stz slotuse,x
        bra @sn
@sf:    lda #2
        sta slotuse,x
        lda slotbase            ; Platzhalter (wird nie benutzt)
        sta slotbase,x
        lda slotbank
        sta slotbank,x
@sn:    inx
        inx
        cpx #NSLOT*2
        bcc @sa
        lda slotbank
        sta ENTBK
        lda slotbase
        sta ENTBASE
        GSOS $200E,expand_pb    ; ExpandPath "*" -> Startvolume
        lda #$FFFF
        sta SEL
        sta SEL2
        sta HOVER
        lda #160
        sta MX
        lda #100
        sta MY
        lda #$FFFF
        sta CPSEL
        jsr load_prefs
        JSLF xf_setup_dbl
        jsr spl_step
        jsr read_clock
        lda MINUTE
        sta LASTMIN
        JSLF xf_icons_load          ; Finder-Symbole aus *:Icons
        jsr spl_step
        JSLF xf_pl_loadall          ; Verknuepfungen, Startmenue, Dokumente
        JSLF xf_dv_scan         ; Laufwerke fuer den Desktop
        jsr spl_step
        jsr spl_finish
        jsr redraw_all          ; erst der Schreibtisch, dann die Fenster vom letzten
        jsr cur_show            ; Mal (Laufwerke abfragen kann dauern)
        JSLF xf_win_restore
        jsr redraw_all
        JSLF xf_boot_chime          ; Startklang nur kurz nach dem Einschalten
        JSLF xf_idle_reset

; ================= Hauptschleife =================
mainloop:
        pea $0000               ; Ergebnis
        pea $FFFF               ; alle Ereignisse
        pea $0000               ; Zeiger auf Ereignis-Record (Bank 0)
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        lda VTON                ; Vollbild-Terminal: alle Ereignisse dorthin
        beq @nvt
        JSLF xf_vt_event
        jmp mainloop
@nvt:   jsr desk_event
        bcc @own
        jmp @fr
@own:   ; Mausposition aus dem Record
        lda EVREC+12
        cmp #320
        bcc @h1
        lda #319
@h1:    sta TMP
        lda EVREC+10
        cmp #200
        bcc @v1
        lda #199
@v1:    sta TMP2
        cmp MY
        bne @mv
        lda TMP
        cmp MX
        beq @nomove
@mv:    JSLF xf_idle_reset
        lda TMP2
        jsr move_to
@nomove:
        lda EVREC
        beq @nact
        JSLF xf_idle_reset
@nact:  lda EVREC                ; Leerlauf: Telnet/IRC abfragen
        bne @ntn
        lda TNON
        beq @ntn
        inc TNCNT
        lda TNCNT
        and #3
        bne @ntn
        JSLF xf_tn_poll
@ntn:   lda EVREC
        cmp #7                  ; Diskette eingelegt
        bne @nd7
        JSLF xf_dv_disk
        jmp @fr
@nd7:   cmp #1                  ; mouseDown
        bne @nk
        jsr click
        bra @fr
@nk:    cmp #3                  ; keyDown
        beq @key
        cmp #5                  ; autoKey
        bne @fr
@key:   lda EVREC+14        ; Wahl-Apfel-C oder Strg-Apfel-C: Bluescreen (Chef kommt)
        and #$0100
        beq @key2
        lda EVREC+2
        and #$007F
        cmp #$03
        beq @bsod
        and #$005F
        cmp #'C'
        bne @key2
        lda EVREC+14
        and #$0800
        beq @key2
@bsod:  JSLF xf_bsod
        jmp @fr
@key2:  lda EVREC+2
        and #$007F
        ldx EVREC
        cpx #5                  ; Wiederholung nur fuer Pfeiltasten
        bne @kok
        cmp #$08
        beq @kok
        cmp #$15
        beq @kok
        cmp #$0A
        beq @kok
        cmp #$0B
        beq @kok
        cmp #$7F                ; Delete (Editor)
        bne @fr
@kok:   jsr key_wrap
@fr:    lda VTON
        beq @fr2
        jmp mainloop
@fr2:   jsr desk_after
        jsr poll_frame
        JSLF xf_idle_check
        JSLF xf_mine_tick       ; Minesweeper-Uhr
        jmp mainloop

; A = neue Y-Position, TMP = neue X-Position
move_to:
        cmp #200
        bcc @y
        lda #199
@y:     pha
        jsr cur_hide
        pla
        sta MY
        lda TMP
        sta MX
        lda MENU
        beq @nm
        jsr org0
        jsr menu_hover
        jsr orgr
@nm:    lda DROP
        beq @nd
        jsr drop_hover
@nd:    jmp cur_show

key_wrap:
        pha
        jsr cur_hide
        pla
        jsr on_key
        jmp cur_show

; ---------- Tastaturbedienung ----------
on_key:
        sta KEYC
        ldx SHUT
        beq @k0
        jmp do_restart
@k0:    cmp #$1B                ; Esc schliesst das vorderste Zubehoer
        bne @k0a
        lda NDAWAS
        beq @k0a
        lda MENU
        ora DROP
        bne @k0a
        jmp nda_esc
@k0a:   lda DROP
        beq @k00
        jmp drop_key
@k00:   lda MENU
        beq @nm
        jmp menu_key
@nm:    lda KEYC
        cmp #$09                ; Tab = Startmenue, Apfel+Tab = naechstes Fenster
        bne @k1
        lda EVREC+14
        and #$0100
        beq @tab
        lda NWIN
        cmp #2
        bcc @tr
        lda #0
        jsr win_unmin
        jsr win_front
        jmp redraw_all
@tr:    rts
@tab:   lda KEYC
        lda #1
        sta MENU
        lda #0
        sta HOVER
        jsr draw_taskbar
        jsr org0
        jsr draw_menu
        jmp orgr
@k1:    lda NWIN                ; oberstes Fenster minimiert: wie ohne Fenster
        beq @k1n
        dec a
        jsr win_ismin
        bcc @k1n
        jmp desk_key
@k1n:   lda WIN
        bne @w
        jmp desk_key
@w:     cmp #5
        bne @w5
        JSLF xf_view_key
        rts
@w5:    cmp #1
        bne @w10
        jmp explorer_key
@w10:   cmp #10
        bne @w11
        jmp quit_key
@w11:   cmp #11
        bne @w12
        jmp edit_key
@w12:   cmp #12
        bne @wd
        jmp confirm_key
@wd:    lda WIN
        cmp #6
        bne @wd0
        JSLF xf_ctrl_wkey
        rts
@wd0:   cmp #14
        bne @wd0b
        jmp access_key
@wd0b:  cmp #21                 ; Browser
        bne @wd0c
        JSLF xf_web_key
        rts
@wd0c:  cmp #22                 ; Netzwerk-Dialog, CD-Player (23)
        bcc @wd1
        JSLF xf_ndlg_key
        rts
@wd1:   cmp #20
        beq @wd2
        cmp #15
        bcc @wd2
        lda KEYC
        cmp #$0D
        bne @wd2
        JSLF xf_sdlg_ok
        rts
@wd2:   lda KEYC
        cmp #$1B
        beq @close
        cmp #$0D
        bne @r
        lda WIN
        cmp #7
        bcc @r
@close: jmp close_win
@r:     rts

drop_key:
        lda KEYC
        cmp #$1B
        bne @k1
        stz DROP
        jmp redraw_all
@k1:    cmp #$0D
        bne @k2
        lda DHOVER
        bmi @r
        jsr drop_item
        lda DROP
        cmp #5
        bcc @nc
        JSLF xf_ctx_clear
@nc:    stz DROP
        lda ACT
        jmp do_action
@k2:    cmp #$0A
        bne @k3
        jsr drop_count
        sta TMP
        lda DHOVER
        inc a
        cmp TMP
        bcc @set
        lda #0
        bra @set
@k3:    cmp #$0B
        bne @k4
        lda DHOVER
        dec a
        bpl @set
        jsr drop_count
        dec a
@set:   sta DHOVER
        jmp draw_drop
@k4:    ldx DROP
        cpx #5
        bcs @r
        cmp #$15
        bne @k5
        lda DROP
        inc a
        cmp #5
        bcc @nd
        lda #1
        bra @nd
@k5:    cmp #$08
        bne @r
        lda DROP
        dec a
        bne @nd
        lda #4
@nd:    sta DROP
        lda #0
        sta DHOVER
        jmp redraw_all
@r:     rts

menu_key:
        JSLF xf_sub_key
        bcc @m0
        rts
@m0:    lda KEYC
        cmp #$15                ; rechts: Untermenue
        bne @m00
        lda HOVER
        cmp #2
        bcs @r
        jmp menu_action
@m00:   cmp #$1B
        bne @m1
        stz MENU
        lda #$FFFF
        sta HOVER
        jmp redraw_menu
@m1:    cmp #$0A
        bne @m2
        lda HOVER
        inc a
        cmp #7
        bcc @set
        lda #0
        bra @set
@m2:    cmp #$0B
        bne @m3
        lda HOVER
        dec a
        bpl @set
        lda #6
@set:   sta HOVER
        jsr org0
        jsr draw_menu
        jmp orgr
@m3:    cmp #$0D
        bne @r
        lda HOVER
        bmi @r
        sta ITEM
        stz MENU
        lda #$FFFF
        sta HOVER
        lda ITEM
        jmp menu_action
@r:     rts

desk_key:
        lda LNKN
        clc
        adc #3
        sta TMP2                ; Anzahl Desktop-Symbole
        lda KEYC
        cmp #$0A
        bne @d1
        lda SEL
        inc a
        cmp TMP2
        bcc @set
        lda #0
        bra @set
@d1:    cmp #$0B
        bne @d2
        lda SEL
        dec a
        bpl @set
        lda TMP2
        dec a
@set:   sta SEL
        jmp desk_sel_redraw
@d2:    cmp #$7F                ; Entf / Strg+D: Verknuepfung entfernen
        beq @dl
        cmp #$04
        bne @d2b
@dl:    lda SEL
        cmp #3
        bcc @r
        cmp #DVSEL              ; Laufwerke kann man nicht entfernen
        bcs @r
        sec
        sbc #3
        JSLF xf_lnk_remove
        rts
@d2b:   cmp #$0D
        bne @r
        lda SEL
        bmi @r
        cmp #DVSEL
        bcc @d2l
        sbc #DVSEL
        JSLF xf_dv_open
        rts
@d2l:   cmp #3
        bcc @d2c
        sec
        sbc #3
        JSLF xf_lnk_open
        rts
@d2c:   cmp #0
        bne @d3
        jmp open_explorer
@d3:    cmp #1
        bne @d4
        jmp open_bin
@d4:    asl
        tax
        lda iconwin,x
        jsr win_open
        jmp redraw_all
@r:     rts

explorer_key:
        lda KEYC
        cmp #$06                ; Ctrl-F = Datei-Menue
        bne @ef
        lda #1
        sta DROP
        stz DHOVER
        jmp draw_drop
@ef:    jsr ctrl_key
        bcc @ek
        rts
@ek:    lda KEYC
        cmp #$1B
        bne @e0
        jmp close_win
@e0:    cmp #$12                ; Ctrl-R = aktualisieren
        bne @e0b
        stz VOLOK
        lda MODE
        bne @rdir
        jsr load_volumes
        jmp redraw_top
@rdir:  jsr load_dir
        jmp redraw_top
@e0b:   cmp #$7F                ; Delete = Aufwaerts
        bne @e1
        jmp go_up
@e1:    cmp #$0D
        bne @e1a
@e1o:   lda SEL2
        bmi @r
        jmp open_entry
@e1a:   lda EVREC+14            ; Apfel-Pfeil hoch/runter
        and #$0100
        beq @e2
        lda KEYC
        cmp #$0A
        beq @e1o
        cmp #$0B
        bne @e2
        jmp go_up
@e2:    ldx #0
@kt:    lda ekeys,x
        beq @r
        cmp KEYC
        beq @mv
        inx
        inx
        inx
        inx
        bra @kt
@mv:    lda COUNT
        beq @r
        lda SEL2
        bpl @hv
        lda #0
        bra @st
@hv:    lda ekeys+2,x
        sta TMP
        cmp #3                  ; hoch/runter = eine Zeile
        bne @hk1
        lda NCV
        sta TMP
@hk1:   cmp #$FFFD
        bne @hk
        lda #0
        sec
        sbc NCV
        sta TMP
@hk:    lda SEL2
        clc
        adc TMP
        bmi @r
        cmp COUNT
        bcs @r
@st:    pha
        jsr win_geom
        pla
        pha
        jsr ensure_vis
        pla
        bcs @newpg
        jmp sel_change          ; sichtbar: nur zwei Zellen neu
@newpg: sta SEL2
        jmp redraw_cont
@r:     rts
ekeys:  .word $15,1,$08,$FFFF,$0A,3,$0B,$FFFD,0

quit_key:
        lda KEYC
        and #$00DF              ; gross
        cmp #'H'
        bne @q1
        jsr win_closeall
        jmp shutdown
@q1:    cmp #'N'
        bne @q2
        jmp do_restart
@q2:    cmp #'F'
        bne @q3
        jmp launch_finder
@q3:    lda KEYC
        cmp #$1B
        bne @r
        jmp close_win
@r:     rts

click:
        jsr cur_hide
        jsr on_click
        jmp cur_show

poll_frame:
        sep #$20
        lda f:$E0C019
        and #$80
        cmp VBLPREV
        bne @chg
        rep #$20
        rts
@chg:   .a8
        sta VBLPREV
        rep #$20
        inc FRAMES
        lda FRAMES
        and #$007F
        bne @r
        lda SHUT
        bne @r
        JSLF xf_dpoll               ; Disketten eingelegt/entnommen?
        jsr read_clock
        lda NSTATE              ; Netzwerksymbol: Zustand geaendert?
        cmp TRAYST
        beq @nm
        jsr cur_hide
        jsr org0
        jsr draw_tray
        jsr orgr
        jsr cur_show
@nm:    lda MINUTE
        cmp LASTMIN
        beq @r
        sta LASTMIN
        jsr cur_hide
        jsr org0
        jsr draw_clock
        jsr orgr
        JSLF xf_cal_tick        ; Kalender offen: Zeiger nachstellen
        jsr cur_show
@r:     rts

waitvbl:
        sep #$20
@1:     lda f:$E0C019
        bmi @1
@2:     lda f:$E0C019
        bpl @2
        rep #$20
        rts

read_clock:
        pha
        pha
        pha
        pha
        TOOL $0D03              ; ReadTimeHex
        pla
        sta TMP
        pla
        sta TMP2
        pla
        pla
        lda TMP
        xba
        and #$00FF
        sta MINUTE
        lda TMP2
        and #$00FF
        sta HOUR
        lda #1
        sta CLKOK
        rts

; ---------- GS/OS-Aufruf (Stack-Variante): A = Nummer, X = Parameterblock ----------
gsos:
        sta GSNUM
        stx GSPB
        lda MYBANK
        pha
        lda GSPB
        pha
        lda GSNUM
        pha
        jsl $E100B0
        rts

; ================= Zeichenroutinen =================
setcolor:
        and #$000F
        sta COLLO
        asl
        asl
        asl
        asl
        sta COLHI
        ora COLLO
        sta COLBYTE
        rts

setpixel:
        txa
        clc
        adc DXO
        tax
        tya
        clc
        adc DYO
        tay
setpixel_raw:
        cpx CLIPR
        bcs @out
        cpx KR
        bcs @out
        cpx KL
        bcc @out
        cpy KB
        bcs @out
        cpy KT
        bcc @out
        tya
        asl
        tay
        lda linetab,y
        sta PTR
        txa
        lsr
        tay
        bcs @odd
        sep #$20
        lda [PTR],y
        and #$0F
        ora COLHI
        sta [PTR],y
        rep #$20
        rts
@odd:   sep #$20
        lda [PTR],y
        and #$F0
        ora COLLO
        sta [PTR],y
        rep #$20
@out:   rts

fillrect:
        lda RX
        clc
        adc DXO
        sta FX
        lda RY
        clc
        adc DYO
        sta FY
        lda RW
        sta FW
        lda RH
        sta FH
        bra @clip
@q:     rts
@clip:  ; Clipping gegen KL..KR / KT..KB (vorzeichenbehaftet)
        lda FX
        clc
        adc FW
        sta XE                  ; rechter Rand (exklusiv)
        lda FX
        sec
        sbc KL
        bvc *+5
        eor #$8000
        bpl @cl
        lda KL
        sta FX
@cl:    lda XE
        sec
        sbc KR
        bvc *+5
        eor #$8000
        bmi @cr
        lda KR
        sta XE
@cr:    lda XE
        sec
        sbc FX
        bvc *+5
        eor #$8000
        bmi @q
        beq @q
        sta FW
        lda FY
        clc
        adc FH
        sta YEND
        lda FY
        sec
        sbc KT
        bvc *+5
        eor #$8000
        bpl @ct
        lda KT
        sta FY
@ct:    lda YEND
        sec
        sbc KB
        bvc *+5
        eor #$8000
        bmi @cb0
        lda KB
        sta YEND
@cb0:   lda YEND
        sec
        sbc FY
        bvc *+5
        eor #$8000
        bmi @q
        beq @q
        sta FH
@cb:    lda FW                  ; 1-2 Pixel breit: senkrechte Linie
        cmp #3
        bcs @gen
        jmp fill_narrow
@gen:   lda FY
        sta YC
        clc
        adc FH
        sta YEND
@row:   lda FX
        sta XS
        clc
        adc FW
        sta XE
        lda XS
        lsr
        bcc @evs
        ldx XS
        ldy YC
        jsr setpixel_raw
        inc XS
@evs:   lda XE
        lsr
        bcc @eve
        dec XE
        ldx XE
        ldy YC
        jsr setpixel_raw
@eve:   lda XS
        cmp XE
        bcs @next
        lda YC
        asl
        tax
        lda linetab,x
        sta PTR
        lda XE
        lsr
        sta TMP
        dec a
        sta TMP2
        lda XS
        lsr
        tay
        lda COLBYTE
        and #$00FF
        sta BITS
        xba
        ora BITS
        sta BITS                ; Fuellwort
        lda TMP
        sty TMP2
        sec
        sbc TMP2                ; Anzahl Bytes
        beq @next
        cmp #1
        bne @fw
        lda BITS
        sep #$20
        sta [PTR],y
        rep #$20
        bra @next
@fw:    pha
        lda BITS
        sta [PTR],y
        pla
        cmp #3
        bcc @next
        ; Rest der Zeile durch ueberlappendes MVN fuellen
        sbc #3
        pha
        lda PTR+2
        and #$00FF
        sta TMP2
        xba
        ora TMP2
        sta @fmv+1
        tya
        clc
        adc PTR
        tax                     ; Quelle = Zeilenanfang
        inc a
        inc a
        tay                     ; Ziel = +2
        pla
        phb
@fmv:   .byte $54,0,0
        plb
@next:  inc YC
        lda YC
        cmp YEND
        bcs @done
        jmp @row
@done:  rts

bevel:
        lda RX
        sta BX
        lda RY
        sta BY
        lda RW
        sta BW
        lda RH
        sta BH
        lda C1
        jsr setcolor
        lda #1
        sta RH
        jsr fillrect
        lda BH
        sta RH
        lda #1
        sta RW
        jsr fillrect
        lda C2
        jsr setcolor
        lda BY
        clc
        adc BH
        dec a
        sta RY
        lda BW
        sta RW
        lda #1
        sta RH
        jsr fillrect
        lda BX
        clc
        adc BW
        dec a
        sta RX
        lda BY
        sta RY
        lda #1
        sta RW
        lda BH
        sta RH
        jsr fillrect
        lda BX
        sta RX
        lda BW
        sta RW
        rts

blit1:
        lda SPR
        sta LP
        lda ABANK
        sta LP+2
        lda [LP]
        and #$00FF
        sta W
        lda [LP]
        xba
        and #$00FF
        sta H
        lda W
        clc
        adc #7
        lsr
        lsr
        lsr
        sta BPR
        lda LP
        clc
        adc #2
        sta LP
        lda SX
        clc
        adc DXO
        sta XS
        lda SY
        clc
        adc DYO
        sta YC
        jsr blitclip
        bcc @go
        rts
@go:    ; ganz im Clip? -> schneller Weg ueber Maskentabelle
        lda XS
        sec
        sbc KL
        bvc *+5
        eor #$8000
        bmi @slow
        lda XS
        clc
        adc W
        cmp ER
        beq @f1
        bcs @slow
@f1:    lda YC
        sec
        sbc KT
        bvc *+5
        eor #$8000
        bmi @slow
        clc
        adc H
        cmp KH
        beq @f2
        bcs @slow
@f2:    jmp blit1_fast
@slow:  stz ROW
@r:     lda YC
        sec
        sbc KT
        cmp KH
        bcs @rowdone
        lda YC
        asl
        tax
        lda linetab,x
        sta PTR
        stz COL
        stz TMP
@b:     ldy TMP
        lda [LP],y
        xba
        and #$FF00
        bne @nb
        lda COL                 ; leeres Byte: 8 Spalten weiter
        clc
        adc #8
        sta COL
        cmp W
        bcs @rowdone
        inc TMP
        bra @b
@nb:    sta BITS
        lda #8
        sta TMP2
@bit:   asl BITS
        bcc @nz
        lda XS
        clc
        adc COL
        cmp ER
        bcs @nz
        cmp KL
        bcc @nz
        lsr
        tay
        sep #$20
        bcs @odd
        lda [PTR],y
        and #$0F
        ora COLHI
        sta [PTR],y
        bra @dn
@odd:   lda [PTR],y
        and #$F0
        ora COLLO
        sta [PTR],y
@dn:    rep #$20
@nz:    inc COL
        lda COL
        cmp W
        bcs @rowdone
        dec TMP2
        beq @nxb
        lda BITS                ; Rest des Bytes leer?
        bne @bit
        lda COL
        clc
        adc TMP2
        sta COL
        cmp W
        bcs @rowdone
@nxb:   inc TMP
        bra @b
@rowdone:
        lda LP
        clc
        adc BPR
        sta LP
        inc YC
        inc ROW
        lda ROW
        cmp H
        bcs *+5
        jmp @r
        rts

; geclipptes Rechteck FX,FY,FW (1 oder 2),FH als senkrechte Linie(n)
fill_narrow:
        lda FX
        jsr vline
        lda FW
        cmp #2
        bcc @r
        lda FX
        inc a
        jsr vline
@r:     rts
vline:  lsr
        sta VLO
        lda #$00F0              ; ungerade: unteres Nibble setzen
        ldx COLLO
        bcs @o
        lda #$000F
        ldx COLHI
@o:     sta VKEEP
        stx VVAL
        lda FH
        sta VLN2
        lda FY
        asl
        tax
@l:     lda linetab,x
        sta PTR
        ldy VLO
        sep #$20
        lda [PTR],y
        and VKEEP
        ora VVAL
        sta [PTR],y
        rep #$20
        inx
        inx
        dec VLN2
        bne @l
        rts

; 4-Bit-Sprite ohne Clipping (Werte >= 16 = durchsichtig)
blit4_fast:
        stz ROW
@r:     lda YC
        asl
        tax
        lda linetab,x
        sta PTR
        lda W
        sta B4N
        lda XS
        lsr
        tay
        bcc @hi
        ; erstes Pixel im unteren Nibble
        lda [LP]
        inc LP
        and #$00FF
        cmp #16
        bcs @s0
        sep #$20
        sta BITS
        lda [PTR],y
        and #$F0
        ora BITS
        sta [PTR],y
        rep #$20
@s0:    iny
        dec B4N
        beq @nr
@hi:    lda [LP]
        inc LP
        and #$00FF
        cmp #16
        bcs @s1
        asl
        asl
        asl
        asl
        sep #$20
        sta BITS
        lda [PTR],y
        and #$0F
        ora BITS
        sta [PTR],y
        rep #$20
@s1:    dec B4N
        beq @nr
        lda [LP]
        inc LP
        and #$00FF
        cmp #16
        bcs @s2
        sep #$20
        sta BITS
        lda [PTR],y
        and #$F0
        ora BITS
        sta [PTR],y
        rep #$20
@s2:    iny
        dec B4N
        bne @hi
@nr:    inc YC
        inc ROW
        lda ROW
        cmp H
        bcs *+5
        jmp @r
        rts

; 1-Bit-Sprite ohne Clipping: je Sprite-Byte 4 (ungerade: 5) Bildschirmbytes
blit1_fast:
        lda COLBYTE
        and #$00FF
        sta GW
        xba
        ora GW
        sta GW                  ; Farbe in allen 4 Nibbles
        stz ROW
@r:     lda YC
        asl
        tax
        lda linetab,x
        sta PTR
        lda XS
        sta GX0
        stz TMP
@b:     ldy TMP
        lda [LP],y
        and #$00FF
        bne *+5
        jmp @nx
        sta GB
        lda GX0
        lsr
        tay
        bcs @odd
        lda GB
        asl
        asl
        tax
        lda gmask,x
        beq @e2
        sta GM
        lda [PTR],y
        eor GW
        and GM
        eor [PTR],y
        sta [PTR],y
@e2:    iny
        iny
        lda gmask+2,x
        beq @nx
        sta GM
        lda [PTR],y
        eor GW
        and GM
        eor [PTR],y
        sta [PTR],y
        bra @nx
@odd:   lda GB                  ; Pixel ab x-1 (gerade) mit Byte >> 1
        and #$00FE
        asl
        tax
        lda gmask,x
        beq @o2
        sta GM
        lda [PTR],y
        eor GW
        and GM
        eor [PTR],y
        sta [PTR],y
@o2:    lda gmask+2,x
        beq @o3
        sta GM
        iny
        iny
        lda [PTR],y
        eor GW
        and GM
        eor [PTR],y
        sta [PTR],y
        dey
        dey
@o3:    lda GB                  ; letztes Bit -> oberes Nibble von Byte +4
        lsr
        bcc @nx
        iny
        iny
        iny
        iny
        sep #$20
        lda [PTR],y
        and #$0F
        ora COLHI
        sta [PTR],y
        rep #$20
@nx:    lda GX0
        clc
        adc #8
        sta GX0
        inc TMP
        lda TMP
        cmp BPR
        bcs *+5
        jmp @b
        lda LP
        clc
        adc BPR
        sta LP
        inc YC
        inc ROW
        lda ROW
        cmp H
        bcs *+5
        jmp @r
        rts

blit1c:
        jsr sprhdr
        and #$00FF
        lsr
        sta TMP
        lda CX
        sec
        sbc TMP
        sta SX
        jmp blit1

blit4:
        lda SPR
        sta LP
        lda ABANK
        sta LP+2
        lda [LP]
        and #$00FF
        sta W
        lda [LP]
        xba
        and #$00FF
        sta H
        lda LP
        clc
        adc #2
        sta LP
        lda SX
        clc
        adc DXO
        sta XS
        lda SY
        clc
        adc DYO
        sta YC
        jsr blitclip
        bcc @go
        rts
@go:    lda XS                  ; ganz im Clip? -> schneller Weg
        sec
        sbc KL
        bvc *+5
        eor #$8000
        bmi @slow
        lda XS
        clc
        adc W
        cmp ER
        beq @f1
        bcs @slow
@f1:    lda YC
        sec
        sbc KT
        bvc *+5
        eor #$8000
        bmi @slow
        clc
        adc H
        cmp KH
        beq @f2
        bcs @slow
@f2:    jmp blit4_fast
@slow:  stz ROW
@r:     lda YC
        sec
        sbc KT
        cmp KH
        bcc @ok
        lda LP
        clc
        adc W
        sta LP
        bra @nextr
@ok:    lda YC
        asl
        tax
        lda linetab,x
        sta PTR
        stz COL
@c:     lda [LP]
        and #$00FF
        cmp #16
        bcs @skip
        sta TMP2
        lda XS
        clc
        adc COL
        cmp ER
        bcs @skip
        cmp KL
        bcc @skip
        lsr
        tay
        sep #$20
        bcs @odd
        lda TMP2
        asl
        asl
        asl
        asl
        sta BITS
        lda [PTR],y
        and #$0F
        ora BITS
        sta [PTR],y
        bra @dn
@odd:   lda [PTR],y
        and #$F0
        ora TMP2
        sta [PTR],y
@dn:    rep #$20
@skip:  inc LP
        inc COL
        lda COL
        cmp W
        bcc @c
@nextr: inc YC
        inc ROW
        lda ROW
        cmp H
        bcc @r
        rts

; Sprite XS,YC (W x H) ganz ausserhalb von KL..KR/KT..KB? -> C=1
; sonst ER = min(CLIPR,KR), KH = KB-KT
blitclip:
        lda XS
        sec
        sbc KR
        bvc *+5
        eor #$8000
        bpl @out
        lda XS
        clc
        adc W
        sec
        sbc KL
        bvc *+5
        eor #$8000
        bmi @out
        beq @out
        lda YC
        sec
        sbc KB
        bvc *+5
        eor #$8000
        bpl @out
        lda YC
        clc
        adc H
        sec
        sbc KT
        bvc *+5
        eor #$8000
        bmi @out
        beq @out
        lda KB
        sec
        sbc KT
        sta KH
        lda CLIPR
        cmp KR
        bcc @e
        lda KR
@e:     sta ER
        clc
        rts
@out:   sec
        rts

; erstes Wort (Breite/Hoehe) des Sprites SPR
sprhdr: lda SPR
        sta LP2
        lda ABANK
        sta LP2+2
        lda [LP2]
        rts

; Zeichenkette STRP (Laenge STRN) an SX,SY in aktueller Farbe
drawstr:
        stz STRI
@l:     lda STRI
        cmp STRN
        bcs @d
        tay
        lda (STRP),y
        jsr glyphptr
        sta SPR
        jsr blit1
        lda SX
        sec
        adc W
        sta SX
        inc STRI
        bra @l
@d:     rts

; Breite von STRP/STRN -> A
strwidth:
        stz TMP2
        ldy #0
@l:     cpy STRN
        bcs @d
        lda (STRP),y
        phy
        jsr glyphptr
        sta LP2
        lda ABANK
        sta LP2+2
        lda [LP2]
        and #$00FF
        sec
        adc TMP2
        sta TMP2
        ply
        iny
        bra @l
@d:     lda TMP2
        beq @z
        dec a
@z:     rts

glyphptr:
        and #$007F
        cmp #32
        bcs @a
        lda #'?'
@a:     cmp #127
        bcc @b
        lda #'?'
@b:     sec
        sbc #32
        asl
        tax
        lda font_tab,x
        rts

; Zahl A dezimal an SX,SY
drawnum:
        stz DIGS
@d1:    ldy #0
@dv:    cmp #10
        bcc @dd
        sbc #10
        iny
        bra @dv
@dd:    pha
        inc DIGS
        tya
        bne @d1
@out:   pla
        clc
        adc #'0'
        jsr glyphptr
        sta SPR
        jsr blit1
        lda SX
        sec
        adc W
        sta SX
        dec DIGS
        bne @out
        rts

inrect:
        lda MX
        sec
        sbc DXO
        sec
        sbc RX
        bcc @no
        cmp RW
        bcs @no
        lda MY
        sec
        sbc DYO
        sec
        sbc RY
        bcc @no
        cmp RH
        bcs @no
        sec
        rts
@no:    clc
        rts

; ---------- Mauszeiger ----------
cur_show:
        lda VTON                ; Vollbild-Terminal: nichts zeichnen
        beq *+3
        rts
        lda SHUT
        ora CURVIS
        ora OFFON
        ora CURNDA              ; Zeiger ueber Zubehoer zeichnet QuickDraw
        bne @r
        lda MX
        lsr
        sta SAVB
        lda MY
        sta SAVY
        stz XFDIR
        jsr cur_xfer
        lda MX
        sta SX
        lda MY
        sta SY
        lda #i_cursor
        sta SPR
        jsr org0
        jsr blit4
        jsr orgr
        lda #1
        sta CURVIS
@r:     rts

cur_hide:
        lda VTON                ; Vollbild-Terminal: nichts zeichnen
        beq *+3
        rts
        lda OFFON
        bne @r
        lda CURVIS
        beq @r
        lda #1
        sta XFDIR
        jsr cur_xfer
        stz CURVIS
@r:     rts

cur_xfer:
        stz ROW
        ldx #0
@r:     lda SAVY
        clc
        adc ROW
        cmp #200
        bcs @done
        asl
        tay
        lda linetab,y
        sta PTR
        phx
        ldy SAVB
        lda #7
        sta TMP2
@c:     cpy #160
        bcs @rowend
        sep #$20
        lda XFDIR
        bne @rest
        lda [PTR],y
        sta savebuf,x
        bra @cont
@rest:  lda savebuf,x
        sta [PTR],y
@cont:  rep #$20
        inx
        iny
        dec TMP2
        bne @c
@rowend:
        pla
        clc
        adc #7
        tax
        inc ROW
        lda ROW
        cmp #17
        bcc @r
@done:  rts

; ================= Splash =================
; ---------- Startbild: gleiches Bild wie beim Booten (START.GS.OS) ----------
BARADR = $E12000+167*160+30     ; Ladebalken: 101 Bytes, Zeilen 167..170
; Ist das Bootbild noch zu sehen? Dann Stand des Ladebalkens nach BOOTP
boot_probe:
        stz BOOTP
        lda f:$E19DA6           ; Balkenzeilen: Palette 0 (640er-Modus im Ersatzbild,
        and #$007F              ; 320er-Modus im Wolkenbild seit 2.1.41)
        bne @r
        lda f:$E12000+171*160+40 ; unterer Balkenrahmen
        bne @r
        lda f:BARADR             ; linker Balkenrahmen: $0F leer, $05 gefuellt
        and #$00FF
        beq @r
        and #$00F0
        bne @r
        ldx #1
@l:     lda f:BARADR,x
        and #$00FF
        cmp #$00FF
        beq @d
        inx
        cpx #101
        bcc @l
@d:     stx BOOTP
@r:     rts

splash: stz DXO
        stz DYO
        stz SPLPIC
        JSLF xf_spl_pic         ; Startbild-Datei (wie beim Booten)?
        bcs @own
        inc SPLPIC
        jmp spl_bar
@own:   ldx #0                  ; zuerst der Farbverlauf, dann das Bild
        sep #$20
@s:     lda splash_scb,x
        sta f:$E19D00,x
        inx
        cpx #200
        bcc @s
        rep #$20
        ldx #0                  ; Verlauf: je Zeile ein Fuellbyte (Farben 12/13 im
        ldy #0                  ; 2x2-Raster gemischt, siehe splashgrad.py)
@g:     lda splash_fill,y
        and #$00FF
        sta TMP
        xba
        ora TMP
        phy
        ldy #80
@gw:    sta f:$E12000,x
        inx
        inx
        dey
        bne @gw
        ply
        iny
        cpy #200
        bcc @g
        TEXTC 163,63,t_logo,BLACK
        TEXTC 160,60,t_logo,WHITE
        TEXTC 160,98,t_ed,YELLOW
        TEXTC 160,114,t_by,WHITE
        TEXTC 160,130,t_build,14
        FILL 60,166,202,6,BLACK
        FILL 61,167,200,4,12
spl_bar:
        stz SPLP
        lda #101
        sec
        sbc BOOTP
        lsr
        lsr
        sta SPLQ                ; ein Viertel des Rests je Ladeabschnitt
        lda #1
        sta SPLON
        lda BOOTP
; Balken bis Byte A (0..101) fuellen: Bloecke wie im Bootbild
bar_to: cmp #102
        bcc @c
        lda #101
@c:     sta TMP
@l:     ldx SPLP
        cpx TMP
        bcs @r
        lda #$00FC              ; ungerade: 1 Punkt weiss, 1 Punkt Luecke
        cpx #100
        bne @n1
        lda #$00F0              ; rechter Rahmen
        bra @w
@n1:    cpx #0
        bne @n2
        lda #$000F              ; linker Rahmen
        bra @w
@n2:    pha
        txa
        lsr
        pla
        bcs @w
        lda #$00FF
@w:     ldy SPLPIC              ; Startbild-Datei: Muster wie START.GS.OS
        beq @w2                 ; (Palette 0 aus der Datei)
        cmp #$00F0
        beq @w2
        ldy #$0055
        cmp #$00FF
        beq @w1
        ldy #$005F
        cmp #$00FC
        beq @w1
        ldy #$0005
@w1:    tya
@w2:    sep #$20
        sta f:BARADR,x
        sta f:BARADR+160,x
        sta f:BARADR+320,x
        sta f:BARADR+480,x
        rep #$20
        inc SPLP
        bra @l
@r:     rts
; ein Ladeabschnitt ist fertig
spl_step:
        lda SPLON
        beq @r
        lda SPLP
        clc
        adc SPLQ
        jmp bar_to
@r:     rts
; Balken voll laufen lassen; das Bild bleibt stehen, bis der Schreibtisch
; fertig aufgebaut ist (SPLEND, siehe redraw_rect)
spl_finish:
        lda SPLON
        beq @r
@l:     jsr waitvbl
        lda SPLP
        clc
        adc #3
        jsr bar_to
        lda SPLP
        cmp #101
        bcc @l
        lda #15
        sta TMP2
@w:     jsr waitvbl
        dec TMP2
        bne @w
        stz SPLON
        lda #1
        sta SPLEND
@r:     rts

desktop_scb:
        ldx #0
        lda #0
@s:     sta f:$E19D00,x
        inx
        inx
        cpx #200
        bcc @s
        ldx #510                ; Paletten zurueck (Startbild-Datei bringt eigene mit)
@p:     lda palettes,x
        sta f:$E19E00,x
        dex
        dex
        bpl @p
        rts

; ================= Desktop =================
redraw_all:
        lda VTON                ; Vollbild-Terminal: nichts zeichnen
        beq *+3
        rts
        lda SPLON               ; Startbild steht noch: nichts zeichnen
        beq *+3
        rts
        lda NDAWAS              ; NDA-Fenster offen: ueber den Window Manager
        beq @own                ; neu aufbauen lassen (richtige Reihenfolge)
        lda INCB
        bne redraw_core
        lda #1
        sta NEEDREF
        rts
@own:   jsr clip_full
        jmp redraw_rect
redraw_core:
        jsr win_save
        stz IDX
        ; liegt der Clip ganz im obersten Fenster? -> nur dieses zeichnen
        lda NWIN
        beq @desk
        dec a
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        cmp KL
        beq @x0
        bpl @desk
@x0:    clc
        ldy #R_W
        adc (PB),y
        cmp KR
        bmi @desk
        ldy #R_Y
        lda (PB),y
        cmp KT
        beq @y0
        bpl @desk
@y0:    clc
        ldy #R_H
        adc (PB),y
        cmp KB
        bmi @desk
        lda NWIN
        dec a
        sta IDX
        bra @w
@desk:  jsr org0
        jsr draw_desktop
        jsr orgr
@w:     lda IDX
        cmp NWIN
        bcs @wd
        ; Fenster ausserhalb des Clips ueberspringen
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        cmp KR
        bpl @skip
        clc
        ldy #R_W
        adc (PB),y
        cmp KL
        bmi @skip
        beq @skip
        ldy #R_Y
        lda (PB),y
        cmp KB
        bpl @skip
        clc
        ldy #R_H
        adc (PB),y
        cmp KT
        bmi @skip
        beq @skip
        lda IDX
        pha
        inc a
        cmp NWIN
        bcc @gr
        lda NDAFOC              ; Zubehoer-Fenster hat den Fokus: alle grau
        bne @gr
@nv:    lda #NAVY
        bra @act
@gr:    lda #DKGRAY
@act:   sta ACTW
        pla
        pha
        jsr win_load
        jsr draw_window
        pla
        sta IDX
@skip:  inc IDX
        jmp @w
@wd:    lda NWIN
        beq @wd2
        dec a
        jsr win_load            ; Zustand des obersten Fensters
@wd2:    lda DROP
        beq @nd
        jsr draw_drop
@nd:
        jsr draw_taskbar
        lda MENU
        beq @nm
        jsr org0
        jsr draw_menu
        jsr orgr
@nm:    rts

.macro DICON idx,yy,ic,tx
        .local nosel
        ICON ICX,yy,ic
        LSPR tx
        jsr sprhdr
        and #$00FF
        sta BW
        lsr
        sta TMP
        lda #ICX+8
        sec
        sbc TMP
        sta SX
        lda SEL
        cmp #idx
        bne nosel
        lda SX
        dec a
        sta RX
        lda #yy+18
        sta RY
        lda BW
        clc
        adc #2
        sta RW
        lda #10
        sta RH
        lda #NAVY
        jsr setcolor
        jsr fillrect
nosel:  lda #yy+19
        sta SY
        lda #WHITE
        jsr setcolor
        jsr blit1
.endmacro

draw_desktop:
        RECT 0,0,320,186
        lda DESKC
        jsr setcolor
        jsr fillrect
        JSLF xf_deskpat         ; Hintergrundmuster
draw_icons:
        DICON 0,8,i_comp,t_arbeit
        DICON 1,44,i_bin,t_papier
        DICON 2,80,i_doc,t_lies
        JSLF xf_draw_links
        JSLF xf_dv_draw         ; Laufwerke (rechte Spalte)
        rts

desk_sel_redraw:
        lda NWIN
        ora MENU
        ora DVCNT               ; Laufwerke rechts: ganz neu zeichnen
        beq @part
        jmp redraw_all
@part:  lda DESKPAT             ; mit Muster: ganz neu zeichnen
        beq @pp
        jmp redraw_all
@pp:    RECT 0,0,120,186
        lda DESKC
        jsr setcolor
        jsr fillrect
        jmp draw_icons

draw_taskbar:
        lda VTON                ; Vollbild-Terminal: nichts zeichnen
        beq *+3
        rts
        jsr org0
        jsr draw_taskbar0
        jsr draw_tasks
        jmp orgr
draw_taskbar0:
        FILL 0,186,320,14,GRAY
        FILL 0,187,320,1,WHITE
        lda MENU
        beq @up
        PRESSED 2,188,42,11
        bra @lg
@up:    BUTTON 2,188,42,11
@lg:    ICON 6,190,i_startlogo
        TEXT 18,190,t_start,BLACK
        ICON 49,189,i_qcomp     ; Schnellstart: Arbeitsplatz, Einstellungen, Ausfuehren
        ICON 61,189,i_qset
        ICON 73,189,i_qrun
        ICON 85,189,i_qmine
        ICON 97,189,i_qweb      ; Internet
        ICON 109,189,i_qterm    ; Terminal (Mailbox-Telefonbuch)
        ICON 121,189,i_qdesk
        FILL 132,188,1,11,DKGRAY
        FILL 133,188,1,11,WHITE
        SUNKEN 268,188,50,11    ; Infobereich: Netzwerksymbol und Uhr
        jsr draw_tray
        jmp draw_clock

; Netzwerksymbol im Infobereich: zwei Bildschirme, blau = verbunden, grau = nicht
draw_tray:
        lda VTON
        beq *+3
        rts
        lda NSTATE
        sta TRAYST
        FILL 270,189,12,9,GRAY
        FILL 271,189,7,6,BLACK          ; hinterer Bildschirm
        FILL 275,192,7,6,BLACK          ; vorderer Bildschirm
        lda #DKGRAY
        ldx TRAYST
        beq @c
        lda #TEAL
@c:     pha
        jsr setcolor
        RECT 272,190,5,4
        jsr fillrect
        pla
        jsr setcolor
        RECT 276,193,5,4
        jmp fillrect

; Fenstertitel an SX,SY (Farbe gesetzt)
draw_title:
        lda WIN
        cmp #21                 ; Browser: Seitentitel
        bne @t28
        JSLF xf_web_title
        rts
@t28:   cmp #28                 ; Editor: Dateiname
        bne @t21
        JSLF xf_ed_title
        rts
@t21:   cmp #19                 ; Fortschritt beim Loeschen
        bne @t19
        lda PRGM
        cmp #2
        bne @t12
        LSPR ti_19b
        jmp blit1
@t19:   cmp #12
        bne @t12
        lda CONFACT
        cmp #6
        bne @t6
        LSPR ti_12c
        jmp blit1
@t6:    cmp #5
        bne @t12
        LSPR ti_12b
        jmp blit1
@t12:   lda WIN
        cmp #11
        bne @t11
        lda TXTMODE
        bne @tn
        jmp @static
@tn:    cmp #2
        bcs @t2
        LSPR ti_11b
        jmp blit1
@t2:    LSPR ti_11c
        lda TXTMODE
        cmp #3
        bne @t2c
        LSPR ti_11d
@t2c:   lda TXTMODE
        cmp #4
        bne @t2b
        LSPR ti_11e
@t2b:   jmp blit1
@t11:   cmp #5
        beq @tb2
        cmp #1
        bne @static
        lda ISBIN
        beq @tb
        LSPR ti_2
        jmp blit1
@tb:    lda MODE
        beq @static
@tb2:   lda pathbuf
        sta TMP
        tax
@f:     dex
        bmi @all
        lda pathbuf+2,x
        and #$00FF
        cmp #':'
        bne @f
@all:   inx
        txa
        sta STRN
        clc
        adc #pathbuf+2
        sta STRP
        lda TMP
        sec
        sbc STRN
        cmp #24
        bcc @ok
        lda #24
@ok:    sta STRN
        jmp drawstr
@static:
        lda WIN
        asl
        tax
        lda LANG
        beq @de
        lda titles_en,x
        bra @st
@de:    lda titles,x
@st:    sta SPR
        jmp blit1

draw_clock:
        lda VTON                ; Vollbild-Terminal: nichts zeichnen
        beq *+3
        rts
        FILL 282,190,34,7,GRAY
        lda CLKOK
        bne @ok
        rts
@ok:    lda #BLACK
        jsr setcolor
        lda #287
        sta SX
        lda #190
        sta SY
        lda HOUR
        jsr two_digits
        lda #10
        jsr glyph
        lda MINUTE
        jsr two_digits
        rts
two_digits:
        ldx #0
@d:     cmp #10
        bcc @e
        sbc #10
        inx
        bra @d
@e:     pha
        txa
        jsr glyph
        pla
glyph:  asl
        tax
        lda digits,x
        sta SPR
        jsr blit1
        lda SX
        sec
        adc W
        sta SX
        rts

titles: .word 0,ti_1,ti_2,ti_3,ti_4,ti_5,ti_6,ti_7,ti_8,ti_9,ti_10,ti_11,ti_12,ti_13,ti_14,ti_15,ti_16,ti_17,ti_18,ti_19,ti_20,ti_21,ti_22,ti_23,ti_24,ti_25,ti_26,ti_27,ti_28
titles_en: .word 0,ti_1_en,ti_2_en,ti_3_en,ti_4_en,ti_5_en,ti_6_en,ti_7_en,ti_8_en,ti_9_en,ti_10_en,ti_11_en,ti_12_en,ti_13_en,ti_14_en,ti_15_en,ti_16_en,ti_17_en,ti_18_en,ti_19_en,ti_20_en,ti_21_en,ti_22_en,ti_23_en,ti_24_en,ti_25_en,ti_26_en,ti_27_en,ti_28_en

; ================= Fenster =================
wincont: .word 0,wc_explorer,wc_papier,wc_lies,wc_explorer,wc_view_m,wc_ctrl_m,wc_info,wc_run,wc_msg,wc_quit,wc_edit,wc_confirm,wc_props,wc_access,wc_sdlg_m,wc_sdlg_m,wc_sdlg_m,wc_sdlg_m,wc_prog_m,wc_mine,wc_web,wc_ndlg,wc_ndlg,wc_ndlg,wc_ndlg,wc_ndlg,wc_ndlg,wc_ndlg
wc_mine: JSLF xf_mine_draw
        rts
wc_web: JSLF xf_web_draw
        rts
wc_ndlg: JSLF xf_wc_ndlg
        rts
wc_ctrl_m: JSLF xf_wc_ctrl
        rts
wc_sdlg_m: JSLF xf_wc_sdlg
        rts
mcolor: .byte 4,5,7,13,2,12,0,1  ; Farben der Zahlen 1..8
; Hintergrundmuster: Maskenbits der beiden Punkte von Byte b, einzeln und je Punkt
pbit:   .word $C000,$3000,$0C00,$0300,$00C0,$0030,$000C,$0003
pbith:  .word $8000,$2000,$0800,$0200,$0080,$0020,$0008,$0002
pbitl:  .word $4000,$1000,$0400,$0100,$0040,$0010,$0004,$0001
pbit1:  .word $8000,$4000,$2000,$1000,$0800,$0400,$0200,$0100,$0080,$0040,$0020,$0010,$0008,$0004,$0002,$0001


explorer:
        TEXT WX+7,WY+18,t_menubar,BLACK
        SUNKEN WX+4,WY+28,WW-8,WH-46
        FILL WX+6,WY+30,WW-12,WH-50,WHITE
        SUNKEN WX+4,WY+WH-16,WW-8,12
        rts

.macro WICON col,ic,tx
        ICON WX+30+col*68,WY+38,ic
        TEXTC WX+38+col*68,WY+57,tx,BLACK
.endmacro

wc_papier:
        jsr explorer
        TEXT WX+12,WY+36,t_leer,BLACK
        TEXT WX+8,WY+WH-14,t_obj0,BLACK
        rts
wc_doku:
        jsr explorer
        TEXT WX+12,WY+36,t_keinedok,BLACK
        TEXT WX+8,WY+WH-14,t_obj0,BLACK
        rts
wc_quit:
        ICON WX+14,WY+28,i_comp
        TEXT WX+40,WY+32,t_q1,BLACK
        BUTTON WX+60,WY+50,120,15
        TEXTC WX+120,WY+54,t_bshut,BLACK
        BUTTON WX+60,WY+70,120,15
        TEXTC WX+120,WY+74,t_brest,BLACK
        BUTTON WX+60,WY+90,120,15
        TEXTC WX+120,WY+94,t_bfind,BLACK
        BUTTON WX+WW-72,WY+WH-26,62,15
        TEXTC WX+WW-41,WY+WH-22,t_abbr,BLACK
        rts
wc_lies:
        TEXT WX+7,WY+18,t_menubar,BLACK
        SUNKEN WX+4,WY+28,WW-8,WH-32
        FILL WX+6,WY+30,WW-12,WH-36,WHITE
        lda #BLACK
        jsr setcolor
        lda #WY+33
        sta IY
        ldx #0
@l:     phx
        lda LANG
        beq @nde
        lda note_tab_en,x
        bra @nst
@nde:   lda note_tab,x
@nst:
        beq @skip
        sta SPR
        lda #WX+9
        sta SX
        lda IY
        sta SY
        jsr blit1
@skip:  lda IY
        clc
        adc #10
        sta IY
        plx
        inx
        inx
        cpx #NOTE_LINES*2
        bcc @l
        rts
wc_info:
        ICON WX+14,WY+26,i_comp
        TEXT WX+40,WY+28,t_i1,BLACK
        TEXT WX+40,WY+44,t_i2,BLACK
        TEXT WX+40,WY+56,t_i3,BLACK
        FILL WX+40,WY+72,WW-52,1,DKGRAY
        FILL WX+40,WY+73,WW-52,1,WHITE
        TEXT WX+40,WY+82,t_i4,BLACK
        TEXT WX+40,WY+94,t_i5,BLACK
okbtn:  BUTTON WX+WW/2-28,WY+WH-26,56,15
        TEXTC WX+WW/2,WY+WH-22,t_ok,BLACK
        rts
wc_run:
        ICON WX+14,WY+28,i_run
        TEXT WX+40,WY+32,t_r1,BLACK
        TEXT WX+14,WY+66,t_r2,BLACK
        SUNKEN WX+50,WY+62,WW-64,14
        FILL WX+52,WY+64,WW-68,10,WHITE
        TEXT WX+55,WY+66,t_r3,BLACK
        BUTTON WX+WW-136,WY+WH-26,56,15
        TEXTC WX+WW-108,WY+WH-22,t_ok,BLACK
        BUTTON WX+WW-72,WY+WH-26,62,15
        TEXTC WX+WW-41,WY+WH-22,t_abbr,BLACK
        rts
wc_msg:
        ICON WX+14,WY+30,i_help
        lda ERRMSG
        asl
        tax
        lda LANG
        beq @ede
        lda errs_en,x
        bra @est
@ede:   lda errs,x
@est:   sta SPR
        lda #WX+40
        sta SX
        lda #WY+34
        sta SY
        lda #BLACK
        jsr setcolor
        jsr blit1
        lda ERRMSG
        bne @n
        TEXT WX+40,WY+46,t_e2,BLACK
@n:     lda ERRMSG
        cmp #4
        bne @n4
        lda #WX+40
        sta SX
        lda #WY+48
        sta SY
        lda #'$'
        jsr putch
        lda ERRC
        xba
        and #$00FF
        jsr puthex2
        lda ERRC
        and #$00FF
        jsr puthex2
@n4:    jmp okbtn

; ---------- Explorer mit echten Dateien ----------
wc_explorer:
        jsr win_geom
        jsr exp_frame
        jsr draw_menubar
        ; leerer Papierkorb
        lda ISBIN
        beq @nobin
        lda COUNT
        bne @nobin
        TEXT WX+12,WY+36,t_leer,BLACK
@nobin: ; Statuszeile
        jsr anc_b
        lda #BLACK
        jsr setcolor
        lda #WX+8
        sta SX
        lda #WY+WH-14
        sta SY
        lda COUNT
        jsr drawnum
        lda COUNT
        cmp #MAXENT
        bcc @nplus
        lda #'+'                ; Ordner hat mehr Eintraege als darstellbar
        jsr putch
@nplus: LSPR t_objsuf
        jsr blit1
        jsr anc_end
        jsr draw_sbar
@nopg:  lda VIEWM
        beq @icv
        jmp wc_list
@icv:   lda TW
        beq @ntr
        JSLF xf_tree_draw       ; Ordnerbaum links
@ntr:   ; sichtbare Eintraege
        stz SLOT
@sl:    jsr pg_first
        clc
        adc SLOT
        sta IDX
        cmp COUNT
        bcc @go
        rts
@go:    jsr draw_entry
        inc SLOT
        lda SLOT
        cmp PPG
        bcc @sl
        rts

; Auswahl im Explorer wechseln (A = neuer Index, gleiche Seite)
sel_change:
        ldx SEL2
        stx OLDSEL
        sta SEL2
        lda OLDSEL
        jsr redraw_cell
        lda SEL2
        jmp redraw_cell

redraw_cell:
        bmi @r
        sta IDX
        cmp COUNT
        bcs @r
        jsr pg_first
        sta TMP
        lda IDX
        sec
        sbc TMP
        bcc @r
        cmp PPG
        bcs @r
        sta SLOT
        lda #1
        sta CLRCELL
        jsr draw_slot
        stz CLRCELL
@r:     rts

draw_slot:
        lda VIEWM
        beq draw_entry
        jmp draw_row

; Zelle SLOT mit Eintrag IDX zeichnen
draw_entry:
        jsr slot_xy
        lda CLRCELL
        beq @nocl
        lda CX
        sec
        sbc #36
        sta RX
        lda IY
        sta RY
        lda #72
        sta RW
        lda #27
        sta RH
        lda #WHITE
        jsr setcolor
        jsr fillrect
@nocl:
        ; Eintragsadresse
        lda IDX
        jsr entep
        ; Icon waehlen: Finder-Symbol aus *:Icons, sonst eigenes
        lda MODE
        beq @own
        JSLF xf_ent_icon
        cmp #$FFFF
        beq @own
        JSLF xf_ficon_draw
        lda IDX
        jsr entep
        bra @icd
@own:   lda [EP]
        jsr type_icon
        sta SPR
        lda CX
        sec
        sbc #8
        sta SX
        lda IY
        sta SY
        jsr blit4
@icd:
        ; Name
        jsr ep_nm
        ldy #2
        lda [EP],y
        and #$00FF
        cmp #13
        bcc @ln
        ; zu lang: 9 Zeichen + "..."
        ldy #3
        ldx #0
@lc:    lda [EP],y
        sep #$20
        sta lnbuf,x
        rep #$20
        iny
        inx
        cpx #9
        bcc @lc
        lda #$2E2E
        sta lnbuf+9
        sta lnbuf+10
        lda #lnbuf
        sta STRP
        lda #12
@ln:    sta STRN
        jsr strwidth
        sta BW
        lsr
        sta TMP
        lda CX
        sec
        sbc TMP
        sta SX
        lda IY
        clc
        adc #17
        sta SY
        lda IDX
        JSLF xf_is_sel
        bcc @nosel
        lda SX
        dec a
        sta RX
        lda SY
        dec a
        sta RY
        lda BW
        clc
        adc #2
        sta RW
        lda #10
        sta RH
        lda #NAVY
        jsr setcolor
        jsr fillrect
        lda #WHITE
        bra @col
@nosel: lda #BLACK
@col:   jsr setcolor
        jmp drawstr

entaddr:
        asl
        asl
        asl
        asl                     ; *16
        sta TMP
        asl                     ; *32
        clc
        adc TMP                 ; *48
        clc
        adc ENTBASE
        rts

; Dateityp in A -> Symbol in A, Typname (Sprache) in TYPN
type_icon:
        phx
        sta TYPQ
        ldx #0
@l:     lda typtab,x
        cmp #$FFFF
        beq @hit
        cmp TYPQ
        beq @hit
        txa
        clc
        adc #8
        tax
        bra @l
@hit:
        lda LANG
        beq @de
        lda typtab+6,x
        bra @st
@de:    lda typtab+4,x
@st:    sta TYPN
        lda typtab+2,x
        plx
        rts

.macro TYP t,ic,tx
        .word t,ic,tx,.ident(.concat(.string(tx),"_en"))
.endmacro
typtab:
        TYP $0100,i_hdd,p_vol
        TYP $0101,i_floppy,p_unf
        TYP $0102,i_floppy,p_525
        TYP $000F,i_folder,p_folder
        TYP $00B3,i_app,p_app
        TYP $00FF,i_app,p_app
        TYP $00B5,i_app,p_app
        TYP $0004,i_doc,p_text
        TYP $00B0,i_doc,p_src
        TYP $00C0,i_pic,p_pic
        TYP $00C1,i_pic,p_pic
        TYP $0008,i_pic,p_pic
        TYP $0050,i_aw,p_aw
        TYP $001A,i_aw,p_aw
        TYP $0019,i_aw,p_aw
        TYP $001B,i_aw,p_aw
        TYP $00C8,i_font,p_font
        TYP $00D8,i_snd,p_snd
        TYP $00B8,i_sys,p_nda
        TYP $00B9,i_sys,p_cda
        TYP $00C7,i_sys,p_cdev
        TYP $00B6,i_sys,p_init
        TYP $00B7,i_sys,p_init
        TYP $00BA,i_sys,p_tool
        TYP $00BB,i_sys,p_drv
        TYP $00BC,i_sys,p_sys
        TYP $00BD,i_sys,p_sys
        TYP $0006,i_binf,p_bin
        TYP $00CA,i_sys,p_icn
        TYP $00C9,i_blank,p_fdat
        TYP $00FC,i_bas,p_bas
        TYP $FFFF,i_blank,p_file

; ---------- Laufwerke und Ordner einlesen ----------
load_volumes:
        lda #$FFFF              ; Ordnerbaum beim naechsten Zeichnen neu aufbauen
        sta TRSUM
        JSLF xf_msel_clear
        stz PAGE
        lda #$FFFF
        sta SEL2
        lda VOLOK
        beq @scan
        lda VOLN
        sta COUNT
        ldx #volcache
        ldy ENTBASE
        lda #16*ENTSZ-1
        jmp xc_me
@scan:  jsr show_wait
        jsr scan_volumes
        lda COUNT
        sta VOLN
        lda #1
        sta VOLOK
        ldx ENTBASE
        ldy #volcache
        lda #16*ENTSZ-1
        jmp xc_em

; 5,25"-Laufwerk IDX abfragen; Erfolg: Eintrag wird zum Datentraeger. C=1: leer
probe525:
        jsr show_wait
        lda IDX
        asl
        tax
        lda voldev,x
        sta dinfo_pb+2
        GSOS $202C,dinfo_pb
        bcs @r
        GSOS $2008,volume_pb
        bcs @r
        lda IDX
        jsr entep
        lda #$0100
        sta [EP]
        lda #volname_buf+4
        sta STRP
        lda volname_buf+2
        sta STRN
        jsr store_name
        ldx ENTBASE             ; auch im Laufwerks-Zwischenspeicher
        ldy #volcache
        lda #16*ENTSZ-1
        jsr xc_em
        lda IDX
        jsr entep
        clc
@r:     rts

; "Bitte warten..." in der Statuszeile des Fensters (falls offen)
show_wait:
        lda WAITOFF             ; neues Fenster noch nicht gezeichnet
        bne @r
        lda WIN
        cmp #1
        bne @r
        jsr anc_b
        FILL WX+6,WY+WH-14,110,9,GRAY
        TEXT WX+8,WY+WH-14,t_wait,BLACK
        jsr anc_end
@r:     rts

scan_volumes:
        stz COUNT
        lda #1
        sta TCNT
@dev:   lda TCNT
        sta dinfo_pb+2
        GSOS $202C,dinfo_pb     ; DInfo
        bcc @ok
        cmp #$0011
        bne *+5
        jmp @done
        jmp @next
@ok:    lda dinfo_pb+8          ; 5,25"-Laufwerk (Blockgeraet, wechselbar, ID 0)?
        and #$0084              ; nicht abfragen: ohne Diskette dauert das lange
        cmp #$0084
        bne @ask
        lda dinfo_pb+20
        bne @ask
        lda COUNT
        jsr entep
        lda #$0102
        sta [EP]
        jsr ent_zero
        lda #devname_buf+4
        sta STRP
        lda devname_buf+2
        sta STRN
        jsr store_name
        bra @dv
@ask:   GSOS $2008,volume_pb    ; Volume
        bcc @vol
        cmp #$0052              ; unbekanntes Format
        beq @unf
        cmp #$0027              ; E/A-Fehler (leer/unformatiert)
        bne @next
@unf:   lda COUNT
        jsr entep
        lda #$0101
        sta [EP]
        jsr ent_zero
        lda #devname_buf+4
        sta STRP
        lda devname_buf+2
        sta STRN
        jsr store_name
        bra @dv
@vol:   ; Name ohne fuehrenden Trenner speichern
        lda COUNT
        jsr entep
        lda #$0100
        sta [EP]
        jsr ent_zero
        lda #volname_buf+4
        sta STRP
        lda volname_buf+2
        sta STRN
        jsr store_name
@dv:
        lda COUNT
        asl
        tax
        lda TCNT
        sta voldev,x
        inc COUNT
        lda COUNT
        cmp #16
        bcs @done
@next:  inc TCNT
        lda TCNT
        cmp #40
        bcs @done
        jmp @dev
@done:  rts

; Name STRP/STRN in Eintrag EP (Typ schon gesetzt), fuehrendes ':'/'/' weg
store_name:
        lda (STRP)
        and #$00FF
        cmp #':'
        beq @skip
        cmp #'.'                ; Geraetenamen (.APPLEDISK3.5A) ohne Punkt
        beq @skip
        cmp #'/'
        bne @go
@skip:  inc STRP
        dec STRN
@go:    lda STRN
        cmp #32
        bcc @l
        lda #31
@l:     sta STRN
        sep #$20
        ldy #2
        sta [EP],y
        ldy #0
@c:     cpy STRN
        bcs @e
        lda (STRP),y
        iny
        iny
        iny
        sta [EP],y
        dey
        dey
        bra @c
@e:     rep #$20
        rts

load_dir:
        JSLF xf_msel_clear
        stz COUNT
        stz PAGE
        lda #$FFFF
        sta SEL2
        GSOS $2010,open_pb      ; Open
        bcs @err
        lda open_pb+2
        sta dirent_pb+2
        sta close_pb+2
@ent:   lda #1
        sta dirent_pb+6
        sta dirent_pb+8
        GSOS $201C,dirent_pb    ; GetDirEntry
        bcs @close
        lda COUNT
        jsr entep
        lda dirent_pb+16
        and #$00FF
        sta [EP]
        jsr ent_extra
        lda #namebuf+4
        sta STRP
        lda namebuf+2
        sta STRN
        jsr store_name
        jsr hide_bin            ; RECYCLED in der Wurzel nicht zeigen
        bcs @hid
        inc COUNT
@hid:   lda COUNT
        cmp #MAXENT
        bcc @ent
@close: GSOS $2014,close_pb     ; Close
        jsr sort_entries
        lda #1                  ; Ordnerbaum mit Nachbarordnern neu aufbauen
        JSLF xf_tree_build
        clc
        rts
@err:   sec
        rts

; C=1, wenn Eintrag EP der Papierkorb-Ordner in einer Laufwerkswurzel ist
hide_bin:
        lda [EP]
        cmp #$000F
        bne @no
        ldy #2
        lda [EP],y
        and #$00FF
        cmp #8
        bne @no
        ldx #1                  ; Wurzel: nur ein ':' in pathbuf
@c:     cpx pathbuf
        bcs @root
        lda pathbuf+2,x
        and #$00FF
        cmp #':'
        beq @no
        inx
        bra @c
@root:  ldy #3
        ldx #0
@n:     lda [EP],y
        jsr upc8
        sep #$20
        cmp binname,x
        rep #$20
        bne @no
        iny
        inx
        cpx #8
        bcc @n
        sec
        rts
@no:    clc
        rts
binname: .byte "RECYCLED"

; Zusatzdaten aus dirent_pb in den Eintrag EP
ent_extra:
        ldy #E_EOF
        lda dirent_pb+18
        sta [EP],y
        iny
        iny
        lda dirent_pb+20
        sta [EP],y
        ldy #E_AUX
        lda dirent_pb+44
        sta [EP],y
        ldy #E_ICN
        lda #0
        sta [EP],y
        ldy #E_DATE
        lda dirent_pb+35        ; Minute, Stunde
        sta [EP],y
        iny
        iny
        lda dirent_pb+37        ; Jahr, Tag
        sta [EP],y
        iny
        iny
        lda dirent_pb+39        ; Monat
        and #$00FF
        sta [EP],y
        rts

; Felder ab E_EOF loeschen (Laufwerke)
ent_zero:
        ldy #E_EOF
        lda #0
@z:     sta [EP],y
        iny
        iny
        cpy #ENTSZ
        bcc @z
        rts

; Pfad: pathbuf += ':' + Name aus Eintrag EP
path_append:
        sep #$20
        ldx pathbuf
        lda #':'
        sta pathbuf+2,x
        inx
        ldy #2
        lda [EP],y
        sta TMP
        stz TMP+1
        ldy #0
@c:     cpy TMP
        bcs @e
        iny
        iny
        iny
        lda [EP],y
        dey
        dey
        dey
        sta pathbuf+2,x
        inx
        iny
        bra @c
@e:     stx pathbuf
        rep #$20
        rts

; ---------- Oeffnen ----------
; A = Eintragsnummer
open_entry:
        sta IDX
        jsr entep
        lda pathbuf
        sta LASTWHAT
        lda MODE
        sta NUMV
        bne @indir
        lda [EP]
        cmp #$0102              ; 5,25"-Laufwerk: erst jetzt abfragen
        bne @n525
        jsr probe525
        bcc @vok
        lda #22                 ; keine Diskette
        jmp show_msg
@n525:  cmp #$0101
        bne @vok
        lda IDX
        sta SEL2
        JSLF xf_vol_bad         ; CD-ROM: CD-Wiedergabe, sonst Formatieren anbieten
        rts
@vok:   stz pathbuf
        jsr path_append
        bra @enter
@indir: lda [EP]
        cmp #$000F
        bne @file
        stz ISBIN
        lda pathbuf
        sta TMP2
        jsr path_append
@enter: lda #1
        sta MODE
        jsr load_dir
        bcc @shown
        lda NUMV
        sta MODE
        bne @rd
        jsr load_volumes
        bra @em
@rd:    lda LASTWHAT
        sta pathbuf
        jsr load_dir
@em:    lda #2
        jmp show_msg
@shown: jmp redraw_top
@file:  cmp #$00FC
        bne @nbas
        JSLF xf_launch_bas
        rts
@nbas:  cmp #$00B3
        beq @launch
        cmp #$00FF
        beq @launch
        cmp #$00B5
        beq @launch
        JSLF xf_doc_note            ; fuer Start > Dokumente merken
        JSLF xf_open_assoc          ; zugehoeriges Programm aus *:Icons
        bcc @od
        rts
@od:    JSLF xf_open_doc
        bcc @nodoc
        rts
@nodoc: lda #0
        jmp show_msg
@launch:
        ; Arbeitsverzeichnis (Prefix 8 und 0) = Programmordner, wie im Finder,
        ; im ProDOS-Format /VOL/ORDNER/ (fuer aeltere ProDOS-16-Programme)
        ldx #0
        sep #$20
@pc:    cpx pathbuf
        bcs @pe
        lda pathbuf+2,x
        cmp #':'
        bne @pn
        lda #'/'
@pn:    sta prefbuf+2,x
        inx
        bra @pc
@pe:    lda #'/'
        sta prefbuf+2,x
        inx
        stx prefbuf
        rep #$20
        lda #8
        sta prefix_pb+2
        GSOS $2009,prefix_pb    ; SetPrefix
        stz prefix_pb+2
        GSOS $2009,prefix_pb
        ; Startpfad = pathbuf + ':' + Name, danach pathbuf wiederherstellen
        lda pathbuf
        pha
        jsr path_append
        ldx #0
@cp:    lda pathbuf,x
        sta launchbuf,x
        inx
        inx
        cpx #260
        bcc @cp
        pla
        sta pathbuf
        jmp launch

show_msg:
        jsr show_msg_nr
        jmp redraw_all

go_up:
        stz ISBIN
        lda MODE
        beq @r
        ldx pathbuf
        stx UPL
@f:     dex
        beq @root
        lda pathbuf+2,x
        and #$00FF
        cmp #':'
        bne @f
        stx pathbuf
        stx UPX
        jsr load_dir
        bra @s
@root:  stz UPX
        stz MODE
        jsr load_volumes
@s:     JSLF xf_up_sel          ; Ordner, aus dem man kam, markieren
        jmp redraw_top
@r:     rts

; ---------- Programm starten (GS/OS Quit mit Ruecksprung) ----------
launch:
        jsr tools_down
        GSOS $2029,quit_pb      ; Quit -> startet Programm, kehrt danach zurueck
        ; nur bei Fehler hier
        jmp restart_app

do_quit:
        jsr tools_down
        GSOS $2029,quit0_pb
        jmp restart_app

tools_down:
        JSLF xf_win_store           ; offene Ordnerfenster merken
        JSLF xf_snd_down
        jsr desk_down
        TOOL $0306              ; EMShutDown
        TOOL $0304              ; QDShutDown
        TOOL $0303              ; MTShutDown
        rts

restart_app:
        lda USERID
        jmp start

; ================= Startmenue =================
draw_menu:
        FILL MNX,MNY,MNW,MNH,GRAY
        BEVEL MNX,MNY,MNW,MNH,GRAY,BLACK
        BEVEL MNX+1,MNY+1,MNW-2,MNH-2,WHITE,DKGRAY
        FILL MNX+3,MNY+3,16,MNH-6,DKGRAY
        TEXT MNX+7,MNY+MNH-4-T_VERT_H,t_vert,GRAY
        FILL MNX+20,MNY+3+6*18+2,MNW-23,1,DKGRAY
        FILL MNX+20,MNY+3+6*18+3,MNW-23,1,WHITE
        lda #0
@i:     pha
        jsr draw_item
        pla
        inc a
        cmp #7
        bcc @i
        JSLF xf_draw_sub
        rts

draw_item:
        sta ITEM
        asl
        tax
        lda itemy,x
        sta IY
        sta RY
        lda #MNX+20
        sta RX
        lda #MNW-23
        sta RW
        lda #18
        sta RH
        lda ITEM
        cmp HOVER
        bne @n
        lda #NAVY
        bra @f
@n:     lda #GRAY
@f:     jsr setcolor
        jsr fillrect
        lda ITEM
        asl
        tax
        lda itemicon,x
        sta SPR
        lda #MNX+22
        sta SX
        lda IY
        inc a
        sta SY
        jsr blit4
        lda ITEM
        cmp HOVER
        bne @n2
        lda #WHITE
        bra @f2
@n2:    lda #BLACK
@f2:    jsr setcolor
        lda ITEM
        asl
        tax
        lda LANG
        beq @ide
        lda itemtext_en,x
        bra @ist
@ide:   lda itemtext,x
@ist:
        sta SPR
        lda #MNX+42
        sta SX
        lda IY
        clc
        adc #6
        sta SY
        jsr blit1
        lda ITEM
        cmp #2
        bcs @r
        lda #m_arrow
        sta SPR
        lda #MNX+MNW-10
        sta SX
        lda IY
        clc
        adc #6
        sta SY
        jsr blit1
@r:     rts

itemy:    .word MNY+3,MNY+21,MNY+39,MNY+57,MNY+75,MNY+93,MNY+117
itemicon: .word i_folder,i_doc,i_sys,i_ctrl,i_help,i_run,i_comp
itemtext: .word m_prog,m_dok,m_zub,m_einst,m_hilfe,m_ausf,m_beenden
itemtext_en: .word m_prog_en,m_dok_en,m_zub_en,m_einst_en,m_hilfe_en,m_ausf_en,m_beenden_en

menu_hit:
        HIT MNX+20,MNY+3,MNW-23,MNH-6
        bcc @none
        lda MY
        sec
        sbc #MNY+3
        cmp #108
        bcs @low
        ldx #0
@d:     cmp #18
        bcc @got
        sbc #18
        inx
        bra @d
@got:   txa
        rts
@low:   cmp #114
        bcc @none
        lda #6
        rts
@none:  lda #$FFFF
        rts

menu_hover:
        JSLF xf_sub_hit             ; im Untermenue?
        bmi @main
        JSLF xf_sub_hover
        rts
@main:  jsr menu_hit
        bmi @hv
        cmp #2                  ; Programme/Dokumente: Untermenue auf
        bcs @cs
        pha
        JSLF xf_sub_open
        pla
        bra @hv
@cs:    pha
        JSLF xf_sub_close
        pla
@hv:    cmp HOVER
        beq @r
        ldx HOVER
        sta HOVER
        stx TMP
        lda TMP
        bmi @n1
        jsr draw_item
@n1:    lda HOVER
        bmi @r
        jsr draw_item
@r:     rts

; ================= Klicks =================
menu_action:
        cmp #2
        bcs @n01
        ; Programme / Dokumente: Untermenue zeigen, Menue bleibt offen
        sta HOVER
        pha
        lda #1
        sta MENU
        pla
        pha
        JSLF xf_sub_open
        pla
        lda #0
        sta SUBH
        jsr org0
        jsr draw_menu
        jmp orgr
@n01:   cmp #6
        bne @n6
        lda #10
        jsr win_open
        jmp redraw_all
@n6:    cmp #2
        bne @n7
        jmp open_access
@n7:    cmp #4
        bne @act
        JSLF xf_help_open       ; Hilfe: Tastenkuerzel (about:keys)
        rts
@act:   clc
        adc #3
        jsr win_open
        jmp redraw_all

open_bin:
        jsr new_explorer
        bcc @ob
        jmp redraw_all
@ob:    lda #1
        sta MODE
        sta ISBIN
        ldx #binpath
        ldy #pathbuf
        jsr pcopy
        jsr load_dir
        bcc @ok
        stz COUNT
@ok:    jmp redraw_all

open_explorer:
        jsr new_explorer
        bcc @oe
        jmp redraw_all
@oe:    stz ISBIN
        stz MODE
        inc WAITOFF
        jsr load_volumes
        stz WAITOFF
        jmp redraw_all

on_click:
        lda SHUT
        beq @c0
        jmp do_restart
@c0:    lda DROP
        beq @c00
        jmp drop_click
@c00:   lda MENU
        beq @nomenu
        jsr org0
        JSLF xf_sub_hit
        bmi @nsub
        pha
        jsr orgr
        pla
        JSLF xf_sub_action
        rts
@nsub:  jsr menu_hit
        sta ITEM
        jsr orgr
        stz MENU
        lda #$FFFF
        sta HOVER
        lda ITEM
        bmi @closeonly
        jmp menu_action
@closeonly:
        jmp redraw_menu
@nomenu:
        jsr org0
        HIT 2,188,42,11
        php
        jsr orgr
        plp
        bcc @nostart
        lda #1
        sta MENU
        lda #$FFFF
        sta HOVER
        jsr draw_taskbar
        jsr org0
        jsr draw_menu
        jsr menu_hover
        jmp orgr
@nostart:
        lda MY                  ; Schnellstart-Symbole neben dem Start-Knopf
        cmp #188
        bcc @noq
        lda MX
        cmp #47
        bcc @noq
        cmp #131
        bcs @noq
        cmp #119
        bcc @qtm
        jmp win_showdesk        ; Desktop anzeigen
@qtm:   cmp #107
        bcc @qwb
        JSLF xf_bbs_open        ; Terminal: Mailbox-Telefonbuch
        rts
@qwb:   cmp #95
        bcc @qm
        JSLF xf_web_open        ; Internet
        rts
@qm:    cmp #83
        bcc @qa
        JSLF xf_mine_new        ; Minesweeper: neues Spiel
        lda #20
        bra @qw
@qa:    cmp #59
        bcs @q2
        jmp open_explorer
@q2:    cmp #71
        bcs @q3
        lda #6
        bra @qw
@q3:    lda #8
@qw:    jsr win_open
        jmp redraw_all
@noq:   lda MY                  ; Klick auf die Uhr: Datum/Uhrzeit einstellen
        cmp #188
        bcc @ncl
        lda MX
        cmp #268
        bcc @ncl
        cmp #284                ; Netzwerksymbol: Einstellungen > Netzwerk
        bcs @clk
        JSLF xf_ndlg_init
        lda #22
        bra @qw
@clk:   lda MX
        cmp #284
        bcc @ncl
        JSLF xf_cal_open        ; Datum/Uhrzeit mit Kalender
        rts
@ncl:   jsr task_click
        bcc @nt
        rts
@nt:    lda NWIN
        bne @hw
        jmp @nowin
@hw:    lda NWIN
        dec a
        jsr win_hit
        bcs @win
        lda WIN                 ; Dialog oben: modal
        cmp #21                 ; (Browser und CD-Player nicht)
        beq @lower
        cmp #23
        bcs @lower              ; (CD-Player, Rechner)
        cmp #7
        bcc @lower
        rts
@lower: lda NWIN
        dec a
        sta TMPK
@lw:    dec TMPK
        bmi @nw2
        lda TMPK
        jsr win_hit
        bcc @lw
        lda TMPK
        jsr win_front
        jmp redraw_all
@nw2:   jmp @nowin
@win:   jsr win_tbar_click
        bcc @nc
        rts
@nc:    lda WIN
        cmp #20
        bne @nmi
        JSLF xf_mine_click      ; Minesweeper
        rts
@nmi:   cmp #21
        bne @nwb
        JSLF xf_web_click       ; Browser
        rts
@nwb:   cmp #22
        bcc @nnd
        JSLF xf_ndlg_click      ; Netzwerk-Dialog, CD-Player (23)
        rts
@nnd:   cmp #15
        bcc @nc2
        JSLF xf_sdlg_click
        rts
@nc2:   cmp #14
        bne @n14
        jmp access_click
@n14:   cmp #13
        bne @n13
        JSLF xf_props_click
        bcc @n13b
        rts
@n13b:  lda WIN
@n13:   cmp #11
        bne @n11
        jmp edit_click
@n11:   cmp #12
        bne @n12
        jmp confirm_click
@n12:   cmp #10
        bne @n10
        jmp quit_click
@n10:   cmp #6
        bne @n6
        JSLF xf_ctrl_click
        rts
@n6:    cmp #7
        bcc @nd
        HIT WX+40,WY+WH-26,WW-48,15
        bcc @nd
        jmp close_win
@nd:    lda WIN
        cmp #1
        bne @inwin
        jsr explorer_click
@inwin: rts
@nowin:
        jsr org0
        ldx #0
@ic:    lda icony,x
        sta RY
        lda #4
        sta RX
        lda #48
        sta RW
        lda #30
        sta RH
        phx
        jsr inrect
        plx
        bcs @hiticon0
        inx
        inx
        cpx #6
        bcc @ic
        JSLF xf_lnk_hit             ; Verknuepfung?
        bcc @nl
        pha
        jsr orgr
        pla
        pha
        clc
        adc #$1003
        JSLF xf_dblclick
        pla
        bcs @lo
        clc
        adc #3
        cmp SEL
        beq @r
        sta SEL
        jmp desk_sel_redraw
@lo:    JSLF xf_lnk_open
        rts
@nl:    JSLF xf_dv_hit          ; Laufwerk?
        bcc @ndv
        pha
        jsr orgr
        pla
        pha
        clc
        adc #$1010
        JSLF xf_dblclick
        pla
        bcs @do
        clc
        adc #DVSEL
        cmp SEL
        beq @r
        sta SEL
        jmp redraw_all
@do:    JSLF xf_dv_open
        rts
@ndv:   jsr orgr
        lda SEL
        bmi @r
        lda #$FFFF
        sta SEL
        jmp redraw_all
@r:     rts
@hiticon0:
        phx
        jsr orgr
        plx
        txa
        lsr
        pha
        clc
        adc #$1000              ; eigene Kennung fuer Desktop-Elemente
        JSLF xf_dblclick
        pla
        bcs @open
        cmp SEL
        bne @nsel
        rts
@nsel:  sta SEL
        jmp desk_sel_redraw
@open:  cmp #0
        bne @o1
        jmp open_explorer
@o1:    cmp #1
        bne @o2
        jmp open_bin
@o2:    lda iconwin,x
        jsr win_open
        jmp redraw_all
icony:   .word 6,42,78
iconwin: .word 1,2,3

quit_click:
        HIT WX+60,WY+50,120,15
        bcc @q1
        jsr win_closeall
        jmp shutdown
@q1:    HIT WX+60,WY+70,120,15
        bcc @q2
        jmp do_restart
@q2:    HIT WX+60,WY+90,120,15
        bcc @q3
        jmp launch_finder
@q3:    HIT WX+WW-72,WY+WH-26,62,15
        bcc @q4
        jmp close_win
@q4:    rts

launch_finder:
        ldx #0
@c:     lda finderpath,x
        sta launchbuf,x
        inx
        inx
        cpx #20
        bcc @c
        jmp launch

do_restart:
        jsr tools_down
        GSOS $2003,shut_pb      ; OSShutdown: Neustart
        jmp restart_app

toggle_lang:
        lda LANG
        eor #1
        sta LANG
        jsr save_prefs
        jmp redraw_all

load_prefs:
        stz LANG
        stz SSMIN
        stz SNDOFF
        stz TREEOFF
        stz DESKPAT
        stz prefsbuf+2
        stz prefsbuf+4
        lda #TEAL
        sta DESKC
        GSOS $2010,popen_pb     ; Open
        bcc *+3
        rts
        lda popen_pb+2
        sta prw_pb+2
        sta pclose_pb+2
        GSOS $2012,prw_pb       ; Read
        php
        GSOS $2014,pclose_pb
        plp
        bcs @r
        lda prefsbuf
        and #1
        sta LANG
        lda prefsbuf
        lsr
        and #1
        sta VIEWM
        lda prefsbuf
        lsr
        lsr
        and #3
        sta SORTM
        lda prefsbuf+2
        and #$00FF
        cmp #16
        bcs @ss
        sta SSMIN
@ss:    lda prefsbuf+3
        and #1
        sta SNDOFF
        lda prefsbuf+3
        lsr
        and #1
        sta DVON
        lda prefsbuf+6          ; Minesweeper-Bestzeit
        cmp #1000
        bcc *+5
        lda #0
        sta MBEST
        lda prefsbuf+5          ; Hintergrundmuster 0..4
        and #$00FF
        cmp #5
        bcc *+5
        lda #0
        sta DESKPAT
        lda prefsbuf+4          ; Ordnerbaum aus nur bei Kennwert $A5
        and #$00FF
        cmp #$00A5
        bne @tr
        lda #1
        sta TREEOFF
@tr:
        lda prefsbuf            ; Byte 1: $80 + Desktopfarbe
        bpl @r
        xba
        and #$000F
        sta DESKC
@r:     rts

save_prefs:
        GSOS $2001,pcreate_pb   ; Create (Fehler egal, falls vorhanden)
        lda #2
        sta popen_pb+8          ; Schreibzugriff
        lda #3
        sta popen_pb
        GSOS $2010,popen_pb
        php
        lda #2
        sta popen_pb
        plp
        bcs @r
        lda popen_pb+2
        sta prw_pb+2
        sta pclose_pb+2
        lda SORTM
        asl
        ora VIEWM
        asl
        ora LANG
        sta prefsbuf
        lda DESKC
        cmp #TEAL
        bne @dk
        lda #0
        bra @ds
@dk:    ora #$0080
@ds:    sep #$20
        sta prefsbuf+1
        rep #$20
        lda DVON
        and #1
        asl
        sta TMP
        lda SNDOFF
        and #1
        ora TMP
        xba
        ora SSMIN
        sta prefsbuf+2
        lda #0
        ldx TREEOFF
        beq *+5
        lda #$00A5
        sta prefsbuf+4
        lda DESKPAT
        xba
        ora prefsbuf+4
        sta prefsbuf+4
        lda MBEST
        sta prefsbuf+6
        GSOS $2013,prw_pb       ; Write
        GSOS $2014,pclose_pb
@r:     rts

close_win:
        jmp win_close

; Klick im Explorer: C=1 wenn behandelt
explorer_click:
        jsr menubar_click
        bcc @nup
        rts
@nup:   jsr win_geom
        jsr sbar_click
        bcc @nopg
        rts
@nopg:  lda TW
        beq @ntr
        JSLF xf_tree_click      ; Klick in den Ordnerbaum
        bcc @ntr
        rts
@ntr:   lda EVREC+14            ; Wahltaste: Kontextmenue
        and #$0800
        beq @nctx
        JSLF xf_ctx_menu
        sec
        rts
@nctx:  lda VIEWM
        beq @grid
        jsr list_slot
        bcs @slot
        jmp @no
@grid:  jsr grid_slot
        bcs @slot
        jmp @no
@slot:  lda SLOT
        cmp PPG
        bcc @sv
        jmp @empty
@sv:    jsr pg_first
        clc
        adc SLOT
        cmp COUNT
        bcs @empty
        pha
        JSLF xf_dblclick
        pla
        bcs @open
        pha
        lda EVREC+14            ; Shift ($0200) oder Apfel ($0100)?
        and #$0300
        beq @plain
        pla
        JSLF xf_msel_toggle
        jsr redraw_win
        sec
        rts
@plain: lda MSELN
        beq @single
        lda 1,s
        JSLF xf_is_sel
        bcs @keep               ; markierten Eintrag ziehen: Auswahl behalten
        JSLF xf_msel_clear
        lda 1,s
        sta SEL2
        jsr redraw_win
        bra @same
@keep:  lda 1,s
        sta SEL2
        bra @same
@single:
        lda 1,s
        cmp SEL2
        beq @same
        jsr sel_change
@same:  pla
        jsr drag_entry
        sec
        rts
@open:  jsr open_entry
        sec
        rts
@empty: lda SEL2
        bmi @no
        lda #$FFFF
        sta SEL2
        jsr redraw_win
@no:    sec
        rts

; ================= Herunterfahren =================
shutdown:
        FILL 0,0,320,200,BLACK
        TEXTC 160,96,t_down,WHITE
        lda #90
        sta ITEM
@w:     jsr waitvbl
        dec ITEM
        bne @w
        FILL 0,0,320,200,BLACK
        TEXTC 160,70,t_s1,ORANGE
        TEXTC 160,92,t_s2,ORANGE
        TEXTC 160,170,t_s3,DKGRAY
        lda #1
        sta SHUT
        stz CURVIS
        rts

        .include "finder.s"
        .include "windows.s"
        .include "docs.s"
        .include "dragdrop.s"
        .include "desk.s"
        .include "icons.s"
        .include "listview.s"
        .include "multisel.s"
        .include "ext.s"
        .include "settings.s"
        .include "more.s"
        .include "geom.s"
        .include "prog.s"
        .include "links.s"
        .include "plist.s"
        .include "startsub.s"
        .include "snd.s"
        .include "wdef.s"
        .include "tree.s"
        .include "mines.s"
        .include "preview.s"
        .include "far2.s"
        .include "net.s"
        .include "web.s"
        .include "x4.s"
        .include "tramp.s"

code_end:
        .segment "RODATA"
ico_pb:  .word 2, 0
         .word 0, 0
icd_pb:  .word 8, 0, 0, 1, 1
         .word icname, 0
         .word 0, 0, 0, 0
icc_pb:  .word 1, 0
icf_pb:  .word 2, 0
         .word 0, 0
icr_pb:  .word 4, 0, 0, 0, 0, 0, 0, 0
icx_pb:  .word 1, 0
icdir:   .word 7
         .byte "*:Icons"
icname:  .word 68, 0
         .res 64
wpath:   .word 19
         .byte "*:System:GSOS95.WIN"
wcr_pb:  .word 3
         .word wpath, 0
         .word $C3, $06
wop_pb:  .word 3, 0
         .word wpath, 0
         .word 3
wrw_pb:  .word 4, 0
         .word winbuf, 0
         .word 0, 0, 0, 0
wef_pb:  .word 3, 0, 0, 0, 0
wcl_pb:  .word 1, 0
lpath:   .word 19
         .byte "*:System:GSOS95.LNK"
ppath:   .word 19
         .byte "*:System:GSOS95.PRG"
dpath:   .word 19
         .byte "*:System:GSOS95.DOC"
bpath:   .word 19
         .byte "*:System:GSOS95.BIN"
lrw_pb:  .word 4, 0
         .word LNKN, 0
         .word 0, 0, 0, 0
offloc: .word 0                 ; SCB: 320 Pixel
        .word 0, 0              ; Zeiger auf den Puffer (zur Laufzeit)
        .word 160               ; Bytes pro Zeile
        .word 0, 0, 200, 320    ; boundsRect
offrect: .word 0, 0, 200, 320
amenu:  .byte ">>@\XN1",0
        .byte "==-\N256D",0
        .byte "."
; Felder, in die beim Start die Programmbank eingetragen wird
ptrfix: .word dinfo_pb+6, volume_pb+4, volume_pb+8, open_pb+6, dirent_pb+12
        .word quit_pb+4, prefix_pb+6, popen_pb+6, prw_pb+6, pcreate_pb+4
        .word create_pb+4, chpath_pb+4, chpath_pb+8, destroy_pb+4, gfi_pb+4
        .word wopen_pb+6, wdirent_pb+12, o4_pb+6, expand_pb+4, expand_pb+8, erase_pb+4, erase_pb+8, ico_pb+6, icd_pb+12, icf_pb+6, wcr_pb+4, wop_pb+6, wrw_pb+6, lrw_pb+6, 0

dinfo_pb:  .word 8, 0
           .word devname_buf, 0
           .res 14                 ; Merkmale, Bloecke, Slot, Einheit, Version, Geraete-ID
volume_pb: .word 2
           .word devname_buf+2, 0
           .word volname_buf, 0
open_pb:   .word 2, 0
           .word pathbuf, 0
dirent_pb: .word 13, 0, 0, 1, 1
           .word namebuf, 0
           .word 0, 0
           .res 48-18           ; Groesse, Bloecke, Datum, Zugriff, Zusatztyp
close_pb:  .word 1, 0
quit_pb:   .word 2
           .word launchbuf, 0
           .word $8000           ; nach Programmende zurueck zu GS/OS95
quit0_pb:  .word 0                ; zurueck zum vorigen Programm
popen_pb:  .word 2, 0
           .word prefspath, 0
           .word 0
pcreate_pb: .word 3
           .word prefspath, 0
           .word $C3, $06
prw_pb:    .word 4, 0
           .word prefsbuf, 0
           .word 8, 0
           .word 0, 0
pclose_pb: .word 1, 0
prefsbuf:  .word 0, 0, 0, 0       ; Byte 2: Schoner-Minuten, Byte 3 Bit 0: kein Startklang, Byte 4: $A5 = Ordnerbaum aus
prefspath: .word 21
           .byte "*:System:GSOS95.PREFS"
prefix_pb: .word 2, 8
           .word prefbuf, 0
errs:    .word t_e1,t_e1,t_e3,t_e4,t_e5,t_e7,t_e6,t_e8,t_e9,t_e10,t_e11,t_e12,t_e13,t_e14,t_e15,t_e16,t_e17,t_e18,t_e19,t_e20,t_e21,t_e22,t_e23,t_e24
errs_en: .word t_e1_en,t_e1_en,t_e3_en,t_e4_en,t_e5_en,t_e7_en,t_e6_en,t_e8_en,t_e9_en,t_e10_en,t_e11_en,t_e12_en,t_e13_en,t_e14_en,t_e15_en,t_e16_en,t_e17_en,t_e18_en,t_e19_en,t_e20_en,t_e21_en,t_e22_en,t_e23_en,t_e24_en
create_pb: .word 5
           .word 0, 0
           .word $C3, 0, 0, 0, 0
chpath_pb: .word 2
           .word 0, 0
           .word 0, 0
destroy_pb: .word 1
           .word 0, 0
gfi_pb:    .word 12
           .word 0, 0
           .res 48
wopen_pb:  .word 2, 0
           .word 0, 0
wdirent_pb: .word 7, 0, 0, 1, 1
           .word wname, 0
           .word 0, 0
wclose_pb: .word 1, 0
o4_pb:     .word 4, 0
           .word 0, 0
           .word 1, 0
io_pb:     .word 4, 0
           .word 0, 0
           .word 0, 0
           .word 0, 0
cl_pb:     .word 1, 0
expand_pb: .word 2
           .word starstr, 0
           .word bootvolbuf, 0
starstr:   .word 1
           .byte "*"
binpath:   .word 10
           .byte "*:RECYCLED"
wname:     .word 68, 0
           .res 64
bootvolbuf: .word 64
bootvol:   .word 0
           .res 62
dctl_pb:   .word 5, 0, $0002
           .word 0, 0
           .word 0, 0
           .word 0, 0
shut_pb:   .word 1, $0001         ; OSShutdown, Bit 0 = Neustart
finderpath: .word 15
           .byte "*:System:Finder"
           .byte 0
erase_pb:  .word 4
           .word devname_buf+2, 0
           .word erasename, 0
           .word 0, 1
erasename: .word 0
           .res 18
devname_buf: .word 36, 0
           .res 32
volname_buf: .word 68, 0
           .res 64
namebuf:   .word 68, 0
           .res 64
pathbuf:   .word 0
           .res 258
launchbuf: .word 0
           .res 258
prefbuf:   .word 0
           .res 260

        .include "assets_tab.inc"
abank_ref: .faraddr spr_base

        .segment "ASSETS"
spr_base:
        .include "assets_spr.inc"
i_close_m_en = i_close_m
i_close_m:
        .byte 8,7
        .byte $C3,$66,$3C,$18,$3C,$66,$C3

        .segment "BSS"
linetab: .res 400
ABANK:   .res 2
TYPN:    .res 2
TYPQ:    .res 2
PICH:    .res 4
PICP:    .res 4
PFH:     .res 4
PFP:     .res 4
PNH:     .res 4
PKIND:   .res 2
PSI:     .res 2
PDI:     .res 2
PSEND:   .res 2
PDEND:   .res 2
PCNT:    .res 2
PLINE:   .res 2
PB0:     .res 6
PBLEN:   .res 2
PMODE:   .res 2
PPPL:    .res 2
PNCT:    .res 2
PNSL:    .res 2
PDIR:    .res 2
PBPL:    .res 2
DGIDX:   .res 2
DGSRC:   .res 2
DGHIT:   .res 2
DGBIN:   .res 2
DGK:     .res 2
XFTYPE:  .res 2
XCUT:    .res 2
NDAON:   .res 2
NDAN:    .res 2
NDAI:    .res 2
NDAOP:   .res 2
CURNDA:  .res 2
fwptr2:  .res 4
NDAFOC:  .res 2
FPI:     .res 2
VKL:     .res 2
VKI:     .res 2
PVN:     .res 2
PVIMG:   .res 2
PVR:     .res 2
PVB:     .res 2
PVPAL:   .res 2
PVC:     .res 2
PVRGB:   .res 2
PVBD:    .res 2
PVBK:    .res 2
PVQ:     .res 2
PVS2:    .res 2
PVMK:    .res 2
PVT1:    .res 2
pvmap:   .res 16
PVI:     .res 2
PVS:     .res 2
PVL:     .res 2
pvbuf:   .res 84
DESKPAT: .res 2
PD: .res 2
PXS: .res 2
PXE: .res 2
PY: .res 2
PYE: .res 2
PM: .res 2
PX0: .res 2
PX1: .res 2
PT: .res 2
PT2: .res 2
TMIDX:   .res 2
MINC:    .res 2
mgrid:   .res 82
mnb:     .res 10
mstack:  .res 84
MST: .res 2
MT0: .res 2
MTIME: .res 2
MBEST: .res 2
MFLAGS: .res 2
MSEED: .res 2
MNBN: .res 2
MROW: .res 2
MCOL: .res 2
MDR: .res 2
MDC: .res 2
MT: .res 2
MT2: .res 2
MEX: .res 2
MI: .res 2
MC: .res 2
MSTK: .res 2
MLEFT: .res 2
MLX: .res 2
MLY: .res 2
MCELL: .res 2
MCX: .res 2
MCY: .res 2
MV: .res 2
TW:      .res 2
TREEOFF: .res 2
TRK:     .res 2
TRS:     .res 2
TRE:     .res 2
TRN:     .res 2
TRROW:   .res 2
TRMAX:   .res 2
TRI:     .res 2
trname:  .res 34
treebuf: .res 32*18
tbrow:   .res 18
TRCNT: .res 2
TRCUR: .res 2
TRSUM: .res 2
TRSTART: .res 2
TBS: .res 2
TBAT: .res 2
TBB: .res 2
TBE: .res 2
TBN: .res 2
TBC: .res 2
TBD: .res 2
TBI: .res 2
TBJ: .res 2
TBP: .res 2
TBK: .res 2
TBAFT: .res 2
TBFULL: .res 2
TBQ: .res 2
TBNS: .res 2
TBNL: .res 2
TBPL: .res 2
TBX: .res 2
TBF: .res 2
TRFW:    .res 2
TRMW:    .res 2
TRSP:    .res 2
BOOTP:   .res 2
SPLP:    .res 2
SPLQ:    .res 2
SPLON:   .res 2
SPLEND:  .res 2
SPLPIC:  .res 2
SPLSW:   .res 2
NLW:     .res 4
DSKH:    .res 4
DSKDP:   .res 2
MNUH:    .res 4
MYDP:    .res 2
INCB:    .res 2
REFR:    .res 2
NEEDREF: .res 2
FWCODE:  .res 2
fwptr:   .res 4
NDAWAS:  .res 2
NWL:     .res 2
WLI:     .res 2
WLL:     .res 2
WLH:     .res 2
FNL:     .res 2
wlist:   .res 24
OFFON:   .res 2
VIEWM:   .res 2                 ; 0 = grosse Symbole, 1 = Details
SORTM:   .res 2                 ; 0 Name, 1 Groesse, 2 Typ
SRI:     .res 2
SRJ:     .res 2
SRL1:    .res 2
SRL2:    .res 2
LVCOL:   .res 2
MSI:     .res 2
MSI2:    .res 2
MSELN:   .res 2
MSELW:   .res 2
CURWID:  .res 2
SLN:     .res 2
lnbuf:   .res 14
MSP:     .res 2
PSTI:    .res 2
DUPN:    .res 2
DUPI:    .res 2
MSGPRT:  .res 2
FNDL:    .res 2
FNDSKIP: .res 2
FNDCNT:  .res 2
FNDD:    .res 2
FNDI:    .res 2
FNDM:    .res 2
FNDP:    .res 2
fndidx:  .res 32
fndtxt:  .res 16
fndroot: .res 262
WSN:     .res 2
WSP:     .res 2
WSK:     .res 2
WSL:     .res 2
winbuf:  .res 800
msel:    .res MAXENT/8
sellist: .res MAXENT*2
srtmp:   .res ENTSZ
SRG:     .res 2
SRGAP:   .res 2
ICN:     .res 2
ICFN:    .res 2
ICREM:   .res 4
ICL:     .res 2
ICBEST:  .res 2
ICBS:    .res 2
ICT:     .res 2
ICA:     .res 2
ICS:     .res 2
ICI:     .res 2
ICNL:    .res 2
ICPL:    .res 2
ICPRE:   .res 2
ICSUF:   .res 2
ICDEV:   .res 2
ICPATHL: .res 2
APNL:    .res 2
FISZ:    .res 2
FIH:     .res 2
FIW:     .res 2
FIOFF:   .res 2
FIBPR:   .res 2
FIX0:    .res 2
FIY0:    .res 2
FIR:     .res 2
FIC:     .res 2
FIOB:    .res 2
FIM:     .res 2
FIP:     .res 2
FIMX:    .res 2
MSGH:    .res 4
icpath:  .res 100
icnm:    .res 32
icpat:   .res 16
appnm:   .res 64
boss:    .res 68
icrec:   .res ICMAX*4
OFFA:    .res 2
OFFB:    .res 2
SAVD:    .res 2
SCRDP:   .res 2
TRAYST:  .res 2                 ; gezeichneter Zustand des Netzwerksymbols
ndanames: .res NDAMAX*NDANL
dgents:  .res SELMAX*ENTSZ
dgent = dgents
DGN:     .res 2
dgdir:   .res 262
dstdir:  .res 262
MBXT:    .res 10
savebuf: .res 7*17
volcache: .res 16*ENTSZ
COPYBUF = 8192
srcpath:  .res 262
dstpath:  .res 262
workpath: .res 262
clipdir:  .res 262
clipents: .res SELMAX*ENTSZ
clipent = clipents
CLIPN:    .res 2
clipok:   .res 2
clipcut:  .res 2
refsrc:   .res 2
refdst:   .res 2
NUMLO:    .res 2
NUMHI:    .res 2
editbuf:  .res 20
editsel:  .res 2
editent:  .res 3
editname: .res 34
NWIN:     .res 2
voldev:   .res 32
nstack:   .res 20
WSEQ:     .res 2
LASTID:   .res 2
BESTID:   .res 2
BESTIX:   .res 2
ACTW:     .res 2
slotuse:  .res NSLOT*2
ORGDEP:   .res 2
SDXO:     .res 2
SDYO:     .res 2
CLIPR:    .res 2
WAITOFF:  .res 2
WDS:      .res 2
WDS2:     .res 2
WDOP:     .res 2
WDW:      .res 4
WDP:      .res 4
WDD:      .res 2
WDB:      .res 2
WDR:      .res 4
WDPOP:    .res 2
WDF:      .res 2
WFT:      .res 2
WFL:      .res 2
WFB:      .res 2
WFR:      .res 2
WCT:      .res 2
WRL:      .res 2
WRT:      .res 2
WRR:      .res 2
WRB:      .res 2
WDC:      .res 2
WDC2:     .res 2
WDT:      .res 4
WX1:      .res 2
WY1:      .res 2
WX2:      .res 2
WY2:      .res 2
WBL:      .res 2
WBR:      .res 2
WBT:      .res 2
WBB:      .res 2
WDPR:     .res 2
WDALL:    .res 2
WDBC:     .res 2
WDTC:     .res 2
WBH:      .res 2
WDOPORT:  .res 4
WDWMP:    .res 4
WDTMPR:   .res 4
WDCLR:    .res 4
WDIT:     .res 4
WDFG:     .res 2
WDBG:     .res 2
wdrect:   .res 8
wdsave:   .res 12
wdtab:    .res 8*WDMAX
WDRP:     .res 4
B4N:      .res 2
VLO:      .res 2
VKEEP:    .res 2
VVAL:     .res 2
VLN2:     .res 2
GW:       .res 2
GB:       .res 2
GM:       .res 2
GX0:      .res 2
SNDOFF:   .res 2
SNDMINE:  .res 2
SSMIN:    .res 2
SSALT:    .res 2                ; welcher Bildschirmschoner als naechstes
IDLET:    .res 4
SVX:      .res 2
SVY:      .res 2
SVD:      .res 2
SVA:      .res 2
STK:      .res 2
RNDS:     .res 2
sndpb:    .res 18
stardx:   .res NSTAR*2
stardy:   .res NSTAR*2
stard:    .res NSTAR*2
SUBM:     .res 2
SUBH:     .res 2
SUBTOP:   .res 2
SUBVIS:   .res 2
SUBAR:    .res 2
SUBAI:    .res 2
SUBAK:    .res 2
UPX:      .res 2
UPL:      .res 2
SYSSORT:  .res 2
SUBY:     .res 2
SUBHT:    .res 2
SUBI:     .res 2
SUBK:     .res 2
PLML:     .res 2
LNKI:     .res 2
LNKP:     .res 2
LNKT:     .res 2
LNKO:     .res 2
LNKL:     .res 2
LX:       .res 2
LY:       .res 2
lnkdir:   .res 262
lnkname:  .res 262
DVON:     .res 2                ; Laufwerke auf dem Desktop zeigen
DVCNT:    .res 2
DVI:      .res 2
DVT:      .res 2
dvtab:    .res DVMAX*DVSZ
dvlab:    .res 12
LNKN:     .res 2                ; direkt vor lnktab (Dateiformat)
lnktab:   .res LNKMAX*LNKSZ
PRGLN:    .res 2
prgtab:   .res PRGMAX*LNKSZ
DOCLN:    .res 2
doctab:   .res DOCMAX*LNKSZ
BINLN:    .res 2
bintab:   .res BINMAX*LNKSZ
BINSV:    .res 2
BINI:     .res 2
BINC:     .res 2
PLSEL:      .res 2
PLI:      .res 2
PLJ:      .res 2
PLTYP:    .res 2
PLAUX:    .res 2
CMX:      .res 2
CMY:      .res 2
PRGN:     .res 2
PRGM:     .res 2
PRGI:     .res 2
PRGON:    .res 2
PRGNL:    .res 2
XABORT:   .res 2
prgname:  .res 32
WSV:      .res 2
TNOWN:    .res 2
TNNDA:    .res 2
TNALL:    .res 2
TNK:      .res 2
TBWV:     .res 2
ndawl:    .res 16
ndatbuf:  .res 26
ndamap:   .res 24
ADX:      .res 2
ADY:      .res 2
CW:       .res 2
CH:       .res 2
NCOL:     .res 2
NROW:     .res 2
NCV:      .res 2
NRV:      .res 2
PPG:      .res 2
TROWS:    .res 2
MAXPG:    .res 2
SBON:     .res 2
GMA:      .res 2
GMR:      .res 2
GMB:      .res 2
GMC:      .res 2
SBTL:     .res 2
SBTH:     .res 2
SBTY:     .res 2
SBGRAB:   .res 2
RSW:      .res 2
RSH:      .res 2
CURREC:   .res 2
DFW:      .res 2
DFH:      .res 2
UW:       .res 2
UH:       .res 2
slotbank: .res NSLOT*2
ACCSEL:   .res 2
DSX:      .res 2
DSY:      .res 2
DOX:      .res 2
DOY:      .res 2
DNX:      .res 2
DNY:      .res 2
wtemp:    .res RECSZ
wrecs:    .res MAXW*RECSZ
NUMV:     .res 2
CLRCELL:  .res 2
OLDSEL:   .res 2
ENTBK:    .res 2
slotbase: .res NSLOT*2
epnm:     .res 34
KL:       .res 2
KT:       .res 2
KR:       .res 2
KB:       .res 2
KH:       .res 2
ER:       .res 2
UL:       .res 2
UR:       .res 2
UT:       .res 2
UB:       .res 2
CPI:      .res 2
CPSEL:    .res 2
IX:       .res 2
SDP:      .res 2
SDN:      .res 2
SDR:      .res 2
SDX:      .res 2
SDI:      .res 2
SDK:      .res 2
sval:     .res 12
DESKC:    .res 2
DBLT:     .res 2
NSTC:     .res 2
basbuf:   .res 66
VCSRC:    .res 2
VCDST:    .res 2
VCOK:     .res 2
vcname:   .res 18
DCN:      .res 4
DCB:      .res 4
DVD:      .res 2
DVN:      .res 2
DVR:      .res 2
MULX:     .res 2
DPCHG:    .res 2
DPNEW:    .res 2
DPDEV:    .res 2
DPINI:    .res 2
dpst:     .res DPMAX*2
dsbuf:    .res 4
TYVAL:    .res 2
TYAUX:    .res 2
NSW:      .res 4
nscur:    .res 10
nstab:    .res 10
