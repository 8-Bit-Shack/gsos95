; GS/OS95 - Desktop-Mockup im Stil von 1995 fuer den Apple IIgs
; Laeuft ohne Betriebssystem direkt nach dem Booten, SHR 320x200x16
        .p816
        .smart +
        .a8
        .i8

; ---------- Farben ----------
BLACK  = 0
TEAL   = 1
NAVY   = 2
DKGRAY = 3
GRAY   = 4
WHITE  = 5
YELLOW = 6
ORANGE = 11

; ---------- Direct Page ----------
PTR     = $80   ; 3 Bytes, Bank $E1
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
BTNPREV = $BA
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
DX      = $D6
DY      = $D8
BX      = $DA
BY      = $DC
BW      = $DE
BH      = $E0
CYC     = $E2
ITEM    = $E4
IY      = $E6
YEND    = $E8
CLKOK   = $EA
MBTN    = $EC

; ---------- Geometrie ----------
WX = 40
WY = 16
WW = 240
WH = 160
MNX = 2
MNW = 112
MNH = 6*18+6+6
MNY = 186-MNH
ICX = 20

; ---------- Makros ----------
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
        jsr setcolor
        jsr fillrect
.endmacro
.macro BEVEL xx,yy,ww,hh,c1,c2
        RECT xx,yy,ww,hh
        lda #c1
        sta C1
        lda #c2
        sta C2
        jsr bevel
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
.macro TEXT xx,yy,lbl,cc
        lda #xx
        sta SX
        lda #yy
        sta SY
        lda #lbl
        sta SPR
        lda #cc
        jsr setcolor
        jsr blit1
.endmacro
.macro ICON xx,yy,lbl
        lda #xx
        sta SX
        lda #yy
        sta SY
        lda #lbl
        sta SPR
        jsr blit4
.endmacro
.macro HIT xx,yy,ww,hh
        RECT xx,yy,ww,hh
        jsr inrect
.endmacro

        .segment "CODE"
start:
        sei
        clc
        xce
        rep #$30
        ldx #$01FF
        txs
        lda #$0000
        tcd
        phk
        plb
        sep #$20
        lda $C036
        ora #$80
        sta $C036               ; schneller Takt
        lda $C034
        and #$F0
        sta $C034               ; Rahmen schwarz
        lda #$E1
        sta PTR+2
        sta $C010
        rep #$20
        ; Zeilentabelle
        ldx #0
        lda #$2000
@lt:    sta linetab,x
        clc
        adc #160
        inx
        inx
        cpx #400
        bcc @lt
        ; Paletten
        ldx #0
@pl:    lda palettes,x
        sta f:$E19E00,x
        inx
        inx
        cpx #512
        bcc @pl
        stz MENU
        stz WIN
        stz SHUT
        stz CURVIS
        stz CYC
        stz CLKOK
        lda #$FFFF
        sta SEL
        sta HOVER
        sta LASTMIN
        lda #$0080
        sta BTNPREV
        lda #160
        sta MX
        lda #100
        sta MY
        jsr splash
        jsr desktop_scb
        jsr read_clock
        lda MINUTE
        sta LASTMIN
        jsr redraw_all
        jsr cur_show

; ================= Hauptschleife =================
mainloop:
        sep #$20
        lda $C000
        bpl @nokey
        sta $C010
        rep #$20
        and #$007F
        jsr on_key
        bra @mouse
@nokey: rep #$20
@mouse: jsr poll_mouse
        jsr poll_frame
        bra mainloop

; ---------- Tastatur ----------
on_key:
        ldx SHUT
        beq @k0
        jmp restart
@k0:    stz DX
        stz DY
        cmp #$08
        bne @k1
        lda #$FFFC
        sta DX
        bra @move
@k1:    cmp #$15
        bne @k2
        lda #4
        sta DX
        bra @move
@k2:    cmp #$0B
        bne @k3
        lda #$FFFC
        sta DY
        bra @move
@k3:    cmp #$0A
        bne @k4
        lda #4
        sta DY
        bra @move
@k4:    cmp #$0D
        beq @click
        cmp #$20
        beq @click
        cmp #$1B
        bne @kend
        jsr cur_hide
        stz MENU
        stz WIN
        lda #$FFFF
        sta HOVER
        jsr redraw_all
        jsr cur_show
@kend:  rts
@click: jmp click
@move:  jmp move_by

; ---------- Maus ----------
poll_mouse:
        sep #$20
        lda $C027
        bmi @full
        rep #$20
        rts
@full:  .a8
        lda $C027
        and #$02
        beq @xfirst
        lda $C024               ; ausser Takt: Y verwerfen
        rep #$20
        rts
@xfirst:
        .a8
        lda $C024
        sta DX
        lda $C024
        sta DY
        sta MBTN
        rep #$20
        lda DX
        jsr sext7
        sta DX
        lda DY
        jsr sext7
        sta DY
        ora DX
        beq @btn
        jsr move_by
@btn:   lda MBTN
        and #$0080
        cmp BTNPREV
        beq @nob
        sta BTNPREV
        cmp #$0000
        bne @nob
        jmp click                ; Flanke: Taste gedrueckt
@nob:   rts

sext7:  and #$007F
        cmp #$0040
        bcc @p
        ora #$FF80
@p:     rts

move_by:
        jsr cur_hide
        lda MX
        clc
        adc DX
        bpl @x1
        lda #0
@x1:    cmp #320
        bcc @x2
        lda #319
@x2:    sta MX
        lda MY
        clc
        adc DY
        bpl @y1
        lda #0
@y1:    cmp #200
        bcc @y2
        lda #199
@y2:    sta MY
        lda MENU
        beq @nm
        jsr menu_hover
@nm:    jmp cur_show

click:
        jsr cur_hide
        jsr on_click
        jmp cur_show

; ---------- Frames / Uhr / Splash-Animation ----------
poll_frame:
        sep #$20
        lda $C019
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
        jsr read_clock
        lda MINUTE
        cmp LASTMIN
        beq @r
        sta LASTMIN
        jsr cur_hide
        jsr draw_clock
        jsr cur_show
@r:     rts

waitvbl:
        sep #$20
@1:     lda $C019
        bmi @1
@2:     lda $C019
        bpl @2
        rep #$20
        rts

read_clock:
        pha
        pha
        pha
        pha
        ldx #$0D03              ; _ReadTimeHex (Misc Tools, ROM)
        jsl $E10000
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

; X = x, Y = y
setpixel:
        cpx #320
        bcs @out
        cpy #200
        bcs @out
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
        lda RW
        beq @done
        bmi @done
        lda RH
        beq @done
        bmi @done
        lda RY
        sta YC
        clc
        adc RH
        sta YEND
@row:   lda RX
        sta XS
        clc
        adc RW
        sta XE
        lda XS
        lsr
        bcc @evs
        ldx XS
        ldy YC
        jsr setpixel
        inc XS
@evs:   lda XE
        lsr
        bcc @eve
        dec XE
        ldx XE
        ldy YC
        jsr setpixel
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
        lda XS
        lsr
        tay
        sep #$20
        lda COLBYTE
@fl:    sta [PTR],y
        iny
        cpy TMP
        bcc @fl
        rep #$20
@next:  inc YC
        lda YC
        cmp YEND
        bcc @row
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

; 1-Bit-Sprite (SPR) an SX,SY in aktueller Farbe
blit1:
        lda (SPR)
        and #$00FF
        sta W
        lda (SPR)
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
        lda SPR
        clc
        adc #2
        sta DPTR
        stz ROW
@r:     stz COL
        stz TMP
@b:     ldy TMP
        lda (DPTR),y
        xba
        and #$FF00
        sta BITS
        lda #8
        sta TMP2
@bit:   asl BITS
        bcc @nz
        lda SX
        clc
        adc COL
        tax
        lda SY
        clc
        adc ROW
        tay
        jsr setpixel
@nz:    inc COL
        lda COL
        cmp W
        bcs @rowdone
        dec TMP2
        bne @bit
        inc TMP
        bra @b
@rowdone:
        lda DPTR
        clc
        adc BPR
        sta DPTR
        inc ROW
        lda ROW
        cmp H
        bcc @r
        rts

; 4-Bit-Sprite, Wert 16 = transparent
blit4:
        lda (SPR)
        and #$00FF
        sta W
        lda (SPR)
        xba
        and #$00FF
        sta H
        lda SPR
        clc
        adc #2
        sta DPTR
        stz ROW
@r:     stz COL
@c:     lda (DPTR)
        and #$00FF
        cmp #16
        bcs @skip
        jsr setcolor
        lda SX
        clc
        adc COL
        tax
        lda SY
        clc
        adc ROW
        tay
        jsr setpixel
@skip:  inc DPTR
        inc COL
        lda COL
        cmp W
        bcc @c
        inc ROW
        lda ROW
        cmp H
        bcc @r
        rts

inrect:
        lda MX
        sec
        sbc RX
        bcc @no
        cmp RW
        bcs @no
        lda MY
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
        lda SHUT
        ora CURVIS
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
        jsr blit4
        lda #1
        sta CURVIS
@r:     rts

cur_hide:
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
splash:
        ldx #0
        sep #$20
@s:     lda splash_scb,x
        sta f:$E19D00,x
        inx
        cpx #200
        bcc @s
        rep #$20
        FILL 0,0,320,200,TEAL
        TEXT (320-T_LOGO_W)/2+3,63,t_logo,BLACK
        TEXT (320-T_LOGO_W)/2,60,t_logo,WHITE
        TEXT (320-T_ED_W)/2,98,t_ed,YELLOW
        TEXT (320-T_BOOT_W)/2,150,t_boot,WHITE
        FILL 60,166,200,10,BLACK
        ; Streifen 12..15
        lda #61
        sta XS
@st:    lda XS
        sta RX
        lda #167
        sta RY
        lda #8
        sta RW
        lda #8
        sta RH
        lda XS
        lsr
        lsr
        lsr
        and #3
        clc
        adc #12
        jsr setcolor
        jsr fillrect
        lda XS
        clc
        adc #8
        sta XS
        cmp #251
        bcc @st
        FILL 253,167,6,8,15
        ; SHR einschalten
        sep #$20
        lda $C029
        ora #$C0
        sta $C029
        rep #$20
        ; Animation (~3 s), Taste bricht ab
        lda #180
        sta ITEM
@an:    jsr waitvbl
        lda ITEM
        and #3
        bne @nc
        jsr cycle_pal
@nc:    sep #$20
        lda $C000
        bmi @key
        rep #$20
        dec ITEM
        bne @an
        rts
@key:   sta $C010
        rep #$20
        rts

cycle_pal:
        inc CYC
        ldx #0                  ; Paletten-Offset
@p:     ldy #0                  ; Eintrag 0..3
@e:     tya
        lsr
        clc
        adc CYC
        and #3
        asl
        phx
        tax
        lda cyccols,x
        plx
        sta TMP
        phx
        txa
        clc
        adc entoff,y
        tax
        lda TMP
        sta f:$E19E00,x
        plx
        iny
        iny
        cpy #8
        bcc @e
        txa
        clc
        adc #32
        tax
        cpx #512
        bcc @p
        rts
entoff: .word 24,26,28,30
cyccols: .word $000A,$003C,$006E,$009F

desktop_scb:
        ldx #0
        lda #0
@s:     sta f:$E19D00,x
        inx
        inx
        cpx #200
        bcc @s
        rts

; ================= Desktop =================
redraw_all:
        jsr draw_desktop
        jsr draw_taskbar
        lda WIN
        beq @nw
        jsr draw_window
@nw:    lda MENU
        beq @nm
        jsr draw_menu
@nm:    rts

.macro DICON idx,yy,ic,tx,tw
        .local nosel
        ICON ICX,yy,ic
        lda SEL
        cmp #idx
        bne nosel
        FILL ICX+8-tw/2-1,yy+18,tw+2,10,NAVY
nosel:  TEXT ICX+8-tw/2,yy+19,tx,WHITE
.endmacro

draw_desktop:
        FILL 0,0,320,186,TEAL
        DICON 0,8,i_comp,t_arbeit,T_ARBEIT_W
        DICON 1,44,i_bin,t_papier,T_PAPIER_W
        DICON 2,80,i_doc,t_lies,T_LIES_W
        rts

draw_taskbar:
        FILL 0,186,320,14,GRAY
        FILL 0,187,320,1,WHITE
        lda MENU
        beq @up
        PRESSED 2,188,42,11
        bra @lg
@up:    BUTTON 2,188,42,11
@lg:    ICON 6,190,i_startlogo
        TEXT 18,190,t_start,BLACK
        lda WIN
        beq @nw
        PRESSED 48,188,96,11
        lda #52
        sta SX
        lda #190
        sta SY
        lda WIN
        asl
        tax
        lda titles,x
        sta SPR
        lda #BLACK
        jsr setcolor
        jsr blit1
@nw:    SUNKEN 280,188,38,11
        jmp draw_clock

draw_clock:
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

titles: .word 0,ti_1,ti_2,ti_3,ti_4,ti_5,ti_6,ti_7,ti_8

; ================= Fenster =================
draw_window:
        FILL WX,WY,WW,WH,GRAY
        BEVEL WX,WY,WW,WH,GRAY,BLACK
        BEVEL WX+1,WY+1,WW-2,WH-2,WHITE,DKGRAY
        FILL WX+3,WY+3,WW-6,13,NAVY
        lda #WX+6
        sta SX
        lda #WY+6
        sta SY
        lda WIN
        asl
        tax
        lda titles,x
        sta SPR
        lda #WHITE
        jsr setcolor
        jsr blit1
        BUTTON WX+WW-17,WY+5,13,10
        TEXT WX+WW-14,WY+6,i_close_m,BLACK
        lda WIN
        asl
        tax
        jsr (wincont,x)
        rts
wincont: .word 0,wc_arbeit,wc_papier,wc_lies,wc_prog,wc_doku,wc_ctrl,wc_info,wc_run

i_close_m:
        .byte 8,7
        .byte $C3,$66,$3C,$18,$3C,$66,$C3

explorer:
        TEXT WX+7,WY+18,t_menubar,BLACK
        SUNKEN WX+4,WY+28,WW-8,WH-46
        FILL WX+6,WY+30,WW-12,WH-50,WHITE
        SUNKEN WX+4,WY+WH-16,WW-8,12
        rts

.macro WICON col,ic,tx,tw
        ICON WX+30+col*68,WY+38,ic
        TEXT WX+38+col*68-tw/2,WY+57,tx,BLACK
.endmacro

wc_arbeit:
        jsr explorer
        WICON 0,i_floppy,t_disk,T_DISK_W
        WICON 1,i_hdd,t_hdd,T_HDD_W
        WICON 2,i_printer,t_drucker,T_DRUCKER_W
        TEXT WX+8,WY+WH-14,t_obj3,BLACK
        rts
wc_papier:
        jsr explorer
        TEXT WX+12,WY+36,t_leer,BLACK
        TEXT WX+8,WY+WH-14,t_obj0,BLACK
        rts
wc_prog:
        jsr explorer
        WICON 0,i_folder,t_zubeh,T_ZUBEH_W
        WICON 1,i_folder,t_spiele,T_SPIELE_W
        WICON 2,i_folder,t_autost,T_AUTOST_W
        TEXT WX+8,WY+WH-14,t_obj3,BLACK
        rts
wc_doku:
        jsr explorer
        TEXT WX+12,WY+36,t_keinedok,BLACK
        TEXT WX+8,WY+WH-14,t_obj0,BLACK
        rts
wc_ctrl:
        jsr explorer
        WICON 0,i_comp,t_anzeige,T_ANZEIGE_W
        WICON 1,i_mouse,t_maus,T_MAUS_W
        WICON 2,i_clock,t_datum,T_DATUM_W
        TEXT WX+8,WY+WH-14,t_obj3,BLACK
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
        lda note_tab,x
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
        BUTTON WX+WW/2-28,WY+WH-26,56,15
        TEXT WX+WW/2-T_OK_W/2,WY+WH-22,t_ok,BLACK
        rts
wc_run:
        ICON WX+14,WY+28,i_run
        TEXT WX+40,WY+32,t_r1,BLACK
        TEXT WX+14,WY+66,t_r2,BLACK
        SUNKEN WX+50,WY+62,WW-64,14
        FILL WX+52,WY+64,WW-68,10,WHITE
        TEXT WX+55,WY+66,t_r3,BLACK
        BUTTON WX+WW-136,WY+WH-26,56,15
        TEXT WX+WW-108-T_OK_W/2,WY+WH-22,t_ok,BLACK
        BUTTON WX+WW-72,WY+WH-26,62,15
        TEXT WX+WW-41-T_ABBR_W/2,WY+WH-22,t_abbr,BLACK
        rts

; ================= Startmenue =================
draw_menu:
        FILL MNX,MNY,MNW,MNH,GRAY
        BEVEL MNX,MNY,MNW,MNH,GRAY,BLACK
        BEVEL MNX+1,MNY+1,MNW-2,MNH-2,WHITE,DKGRAY
        FILL MNX+3,MNY+3,16,MNH-6,DKGRAY
        TEXT MNX+7,MNY+MNH-4-T_VERT_H,t_vert,GRAY
        FILL MNX+20,MNY+3+5*18+2,MNW-23,1,DKGRAY
        FILL MNX+20,MNY+3+5*18+3,MNW-23,1,WHITE
        lda #0
@i:     pha
        jsr draw_item
        pla
        inc a
        cmp #6
        bcc @i
        rts

; A = Index
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
        lda itemtext,x
        sta SPR
        lda #MNX+42
        sta SX
        lda IY
        clc
        adc #6
        sta SY
        jsr blit1
        lda ITEM
        cmp #3
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

itemy:    .word MNY+3,MNY+21,MNY+39,MNY+57,MNY+75,MNY+99
itemicon: .word i_folder,i_doc,i_ctrl,i_help,i_run,i_comp
itemtext: .word m_prog,m_doku,m_einst,m_hilfe,m_ausf,m_beenden

; liefert A = Eintrag unter dem Zeiger oder $FFFF
menu_hit:
        HIT MNX+20,MNY+3,MNW-23,MNH-6
        bcc @none
        lda MY
        sec
        sbc #MNY+3
        cmp #90
        bcs @low
        ldx #0
@d:     cmp #18
        bcc @got
        sbc #18
        inx
        bra @d
@got:   txa
        rts
@low:   cmp #96
        bcc @none
        lda #5
        rts
@none:  lda #$FFFF
        rts

menu_hover:
        jsr menu_hit
        cmp HOVER
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
on_click:
        lda SHUT
        beq @c0
        jmp restart
@c0:    lda MENU
        beq @nomenu
        jsr menu_hit
        sta ITEM
        stz MENU
        lda #$FFFF
        sta HOVER
        lda ITEM
        bmi @closeonly
        cmp #5
        bne @act
        jmp shutdown
@act:   clc
        adc #4
        sta WIN
@closeonly:
        jmp redraw_all
@nomenu:
        HIT 2,188,42,11
        bcc @nostart
        lda #1
        sta MENU
        lda #$FFFF
        sta HOVER
        jsr draw_taskbar
        jsr draw_menu
        jmp menu_hover
@nostart:
        lda WIN
        beq @nowin
        HIT WX+WW-17,WY+5,13,10
        bcc @nc
        stz WIN
        jmp redraw_all
@nc:    lda WIN
        cmp #7
        bcc @nd
        HIT WX+40,WY+WH-26,WW-48,15
        bcc @nd
        stz WIN
        jmp redraw_all
@nd:    HIT WX,WY,WW,WH
        bcc @nowin
        rts
@nowin:
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
        bcs @hiticon
        inx
        inx
        cpx #6
        bcc @ic
        lda SEL
        bmi @r
        lda #$FFFF
        sta SEL
        jmp redraw_all
@r:     rts
@hiticon:
        txa
        lsr
        cmp SEL
        beq @open
        sta SEL
        jmp redraw_all
@open:  lda iconwin,x
        sta WIN
        jmp redraw_all
icony:   .word 6,42,78
iconwin: .word 1,2,3

; ================= Herunterfahren =================
shutdown:
        FILL 0,0,320,200,BLACK
        TEXT (320-T_DOWN_W)/2,96,t_down,WHITE
        lda #120
        sta ITEM
@w:     jsr waitvbl
        dec ITEM
        bne @w
        FILL 0,0,320,200,BLACK
        TEXT (320-T_S1_W)/2,70,t_s1,ORANGE
        TEXT (320-T_S2_W)/2,92,t_s2,ORANGE
        TEXT (320-T_S3_W)/2,170,t_s3,DKGRAY
        lda #1
        sta SHUT
        stz CURVIS
        sep #$20
        sta $C010
        rep #$20
        rts

restart:
        stz SHUT
        stz WIN
        stz MENU
        lda #$FFFF
        sta SEL
        sta HOVER
        jsr read_clock
        jsr redraw_all
        jmp cur_show

        .segment "RODATA"
        .include "assets.inc"

        .segment "BSS"
linetab: .res 400
savebuf: .res 7*17
