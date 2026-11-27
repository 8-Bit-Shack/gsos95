; ================================================================
; GS/OS95: Paint (Fensterart 26, XCODE4 / x4.s)
; Fenster ganz oben links (0/0, 320x186) wie Solitaer. Bild 320x200 in
; 16 Farben (Palette des Schreibtischs), gespeichert als $C1-Bild
; (32 KB: Punkte, SCBs, Paletten) - der Bildbetrachter zeigt es an.
; Sichtbar: 284x144 Punkte, Pfeiltasten verschieben den Ausschnitt.
; Werkzeuge: Stift, Pinsel, Radierer, Spruehdose, Linie, Rechteck,
; gefuelltes Rechteck, Fuellen. Tasten: S speichern, Z/Strg-Z
; rueckgaengig, N neues Bild, Pfeile verschieben, Esc schliesst.
; Im Bildbetrachter oeffnet B das angezeigte Bild in Paint.
; Bildschirmkoordinaten (Fenster bei 0/0): logisch = Schirm + WX/WY.
; ================================================================
PTKIND = 26
PT_CX  = 32                     ; Zeichenflaeche auf dem Schirm
PT_CY  = 17
PT_CW  = 284
PT_CH  = 144
PT_TX  = 3                      ; Werkzeuge (2 Spalten, 4 Zeilen)
PT_TY  = 18
PT_PX  = 32                     ; Farben
PT_PY  = 165
PT_SY  = 177                    ; Statuszeile
PT_NT  = 12                     ; Werkzeuge
        .pushseg
        .segment "BSS"
PTH:    .res 4                  ; Griff: Bild (32 KB)
PTU:    .res 4                  ; Griff: Rueckgaengig (32000)
PTS:    .res 4                  ; Griff: Fuellstapel (8 KB)
PDOC:   .res 4                  ; Zeiger Bild
PUND:   .res 4
PSTK:   .res 4
PTOOL:  .res 2
PCOL:   .res 2
PCB:    .res 2                  ; Farbe als Byte (beide Halbbytes)
QOFX:    .res 2
QOFY:    .res 2
QPX:     .res 2                  ; Bildpunkt
QPY:     .res 2
QPX0:    .res 2
QPY0:    .res 2
QPX1:    .res 2
QPY1:    .res 2
QPLX:    .res 2                  ; letzter Punkt beim Ziehen
QPLY:    .res 2
QPSX:    .res 2                  ; Startpunkt (Linie, Rechteck)
QPSY:    .res 2
QPSO:    .res 2                  ; 1 = nur auf den Schirm (Vorschau)
QPVX0:   .res 2                  ; Vorschau-Rechteck (Schirm) zum Zuruecksetzen
QPVY0:   .res 2
QPVX1:   .res 2
QPVY1:   .res 2
QPDX:    .res 2
QPDY:    .res 2
QPSXS:   .res 2
QPSYS:   .res 2
QPER:    .res 2
QPE2:    .res 2
PTA:     .res 2
PTB:    .res 2
QPI:     .res 2
QPJ:     .res 2
QPN:     .res 2
QPMX:    .res 2                  ; Maus (Schirm)
QPMY:    .res 2
QPDIRTY: .res 2
QPUOK:   .res 2
QPRND:   .res 2
QPOLD:   .res 2                  ; Fuellen: zu ersetzende Farbe
QPSP:    .res 2                  ; Fuellstapel: Hoehe (Bytes)
PNAME:  .res 2+64               ; Pfad (Laenge + Text) beim Speichern
PPB:    .res 24                 ; GS/OS-Parameter
QPW:     .res 2
PPREV:  .res 2                  ; Werkzeug vor der Pipette
PDONLY: .res 2                  ; 1 = pt_pset nur ins Bild
PIW:    .res 2
PIH:    .res 2
PIO:    .res 2
PESC:   .res 2                  ; Esc einmal gedrueckt (ungespeichert)
PTXT:   .res 2+48               ; Text-Werkzeug: Laenge (1 Byte) + Zeichen
PTL:    .res 2
PTX:    .res 2
PTY:    .res 2
PEHA:    .res 2                  ; Ellipse: Halbachsen, Mitte
PEHB:    .res 2
PECX:   .res 2
PECY:   .res 2
PES:    .res 2
PEDX:   .res 2
PEDY:   .res 2
PEPX:   .res 2
PEPY:   .res 2
PEF:    .res 2
PEQ:    .res 2
PSV1:   .res 2
PSV2:   .res 2
PSV3:   .res 2
PSV4:   .res 2
        .popseg

xf_pt_open:     jsr pt_open
        rtl
xf_pt_draw:     jsr pt_draw
        rtl
xf_pt_click:    jsr pt_click
        rtl
xf_pt_key:      jsr pt_key
        rtl
; Bildbetrachter: Bild vom Schirm (32 KB ab $E1:2000) in Paint, Pfad in pathbuf
xf_pt_screen:   jsr pt_screen
        rtl
; Browser (Strg-P): erstes Bild der Seite in Paint. C=1: kein Bild
xf_pt_img:      jsr pt_img
        rtl

; ---------- Speicher ----------
; A = Groesse (Bytes, < 64 KB), Griff -> PTA/PTB, Zeiger -> X(lo)/Y(hi). C=1 kein Speicher
pt_alloc:
        tax
        pha
        pha
        pea $0000
        phx
        lda USERID
        pha
        pea $C010               ; fest, gesperrt, nicht ueber Bankgrenzen
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta PTA
        pla
        sta PTB
        bcs @r
        lda PTA
        sta PSRC
        lda PTB
        sta PSRC+2
        ldy #2
        lda [PSRC],y
        tay
        lda [PSRC]
        tax
        clc
@r:     rts

pt_mem: lda PTH+2
        ora PTH
        bne @ok
        lda #$8000
        jsr pt_alloc
        bcs @no
        stx PDOC
        sty PDOC+2
        lda PTA
        sta PTH
        lda PTB
        sta PTH+2
        lda #32000
        jsr pt_alloc
        bcs @no
        stx PUND
        sty PUND+2
        lda PTA
        sta PTU
        lda PTB
        sta PTU+2
        lda #$2000
        jsr pt_alloc
        bcs @no
        stx PSTK
        sty PSTK+2
        lda PTA
        sta PTS
        lda PTB
        sta PTS+2
@ok:    clc
        rts
@no:    sec
        rts

; Zeiger: PDST = Bild, PSRC = Rueckgaengig
pt_dp:  lda PDOC
        sta PDST
        lda PDOC+2
        sta PDST+2
        rts
pt_up:  lda PUND
        sta PSRC
        lda PUND+2
        sta PSRC+2
        rts

; ---------- neues Bild: weiss, SCBs 0, Paletten vom Schreibtisch ----------
pt_new: jsr pt_dp
        ldy #0
        lda #$FFFF
@w:     sta [PDST],y
        iny
        iny
        cpy #32000
        bcc @w
        lda #0
@s:     sta [PDST],y            ; SCBs und Rest
        iny
        iny
        cpy #32256
        bcc @s
        ldx #0
@p:     lda f:$E19E00,x         ; Paletten (Schreibtisch)
        sta [PDST],y
        iny
        iny
        inx
        inx
        cpx #512
        bcc @p
        stz QOFX
        stz QOFY
        stz PNAME
        stz QPUOK
        stz QPDIRTY
        rts

; ---------- Oeffnen ----------
pt_find:
        stz QPW
@l:     lda QPW
        cmp NWIN
        bcs @no
        CALL recaddr
        sta PB
        lda (PB)
        cmp #PTKIND
        beq @y
        inc QPW
        bra @l
@y:     lda QPW
        sec
        rts
@no:    clc
        rts

pt_open:
        jsr pt_find
        bcc @new
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        rts
@new:   jsr pt_mem
        bcs @nm
        jsr pt_new
        stz PTOOL
        stz PCOL
        jsr pt_win
        rts
@nm:    lda #8                  ; kein Speicher
        CALL show_msg_nr
        CALL redraw_all
        rts

pt_win: lda #PTKIND
        CALL win_open
        bcc @o
        lda #8
        CALL show_msg_nr
        CALL redraw_all
        rts
@o:     lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda #0
        sta (PB),y
        ldy #R_Y
        sta (PB),y
        ldy #R_W
        lda #320
        sta (PB),y
        ldy #R_H
        lda #186
        sta (PB),y
        lda NWIN
        dec a
        CALL win_load
        CALL redraw_all
        rts

; Bild vom Schirm uebernehmen (aus dem Bildbetrachter), Pfad = pathbuf
pt_screen:
        jsr pt_mem
        bcs @r
        jsr pt_dp
        ldy #0
@c:     tyx
        lda f:$E12000,x
        sta [PDST],y
        iny
        iny
        bpl @c
        stz QOFX
        stz QOFY
        stz QPUOK
        stz QPDIRTY
        ldx #0                  ; Pfad merken: dort wird gespeichert
@p:     lda pathbuf,x
        sta PNAME,x
        inx
        inx
        cpx #66
        bcc @p
        lda PNAME
        cmp #63
        bcc @pk
        stz PNAME
@pk:    JSLF xf_set_palettes
        CALL desktop_scb
        jsr pt_find             ; Paint schon offen: nach vorn
        bcc @new
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        rts
@new:   jsr pt_win
@r:     rts

; Bild aus dem Browser (Bildbank IMBK, 1 Byte je Punkt, Farben des Schreibtischs)
pt_img: lda GIFON               ; Seite ist ein Bild: ab 0, F_SW x F_SH
        beq @inl
        stz PIO
        lda F_SW
        sta PIW
        lda F_SH
        sta PIH
        bra @go
@inl:   ldx #0                  ; sonst das erste geladene Bild der Seite
@f:     txa
        lsr
        cmp IMGN
        bcc *+5
        jmp @no
        lda IMGH,x
        bne @y
        inx
        inx
        bra @f
@y:     sta PIH
        lda IMGW,x
        sta PIW
        lda IMGO,x
        sta PIO
@go:    lda PIW
        bne *+5
        jmp @no
        jsr pt_mem
        bcc *+5
        jmp @no
        jsr pt_new
        lda #1
        sta PDONLY
        stz QPSO
        lda IMBK
        sta PSRC+2
        stz PSRC
        stz QPY
@yl:    lda QPY
        cmp PIH
        bcs @done
        cmp #200
        bcs @done
        lda QPY                 ; Zeilenanfang in der Bildbank
        ldx PIW
        jsr pt_mul
        clc
        adc PIO
        sta QPJ
        stz QPX
@xl:    lda QPX
        cmp PIW
        bcs @nx
        cmp #320
        bcs @nx
        clc
        adc QPJ
        tay
        lda [PSRC],y
        jsr pt_setc
        jsr pt_dp
        jsr pt_pset
        inc QPX
        bra @xl
@nx:    inc QPY
        bra @yl
@done:  stz PDONLY
        lda #1
        sta QPDIRTY
        stz PNAME
        lda #15
        jsr pt_setc
        jsr pt_find             ; Paint schon offen: nach vorn
        bcc @new
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        clc
        rts
@new:   stz PTOOL
        stz PCOL
        jsr pt_win
        clc
        rts
@no:    sec
        rts

; ---------- Zeichnen des Fensters ----------
pt_draw:
        lda #WX+2               ; Hintergrund
        sta RX
        lda #WY+15
        sta RY
        lda #316
        sta RW
        lda #170
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        stz QPI                  ; Werkzeuge
@t:     lda QPI
        jsr pt_tbtn
        inc QPI
        lda QPI
        cmp #PT_NT
        bcc @t
        lda #WX+PT_CX-2         ; Rahmen der Zeichenflaeche
        sta RX
        lda #WY+PT_CY-2
        sta RY
        lda #PT_CW+4
        sta RW
        lda #PT_CH+4
        sta RH
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        jsr pt_ccur             ; aktuelle Farbe
        stz QPI                  ; Farbfelder
@f:     lda QPI
        jsr pt_sw
        inc QPI
        lda QPI
        cmp #16
        bcc @f
        jsr pt_stat
        ; Zeichenflaeche (innerhalb der Grenzen des Fensterzeichnens)
        lda KL
        sta QPVX0
        lda KR
        sta QPVX1
        lda KT
        sta QPVY0
        lda KB
        sta QPVY1
        jmp pt_blit

; Knopf fuer Werkzeug A
pt_tbtn:
        pha
        and #1
        beq @c0
        lda #14
@c0:    clc
        adc #WX+PT_TX
        sta RX
        sta SX
        lda 1,s
        lsr
        sta PTA
        asl
        asl
        asl
        asl
        sec
        sbc PTA
        sbc PTA                  ; *14
        clc
        adc #WY+PT_TY
        sta RY
        sta SY
        lda #13
        sta RW
        sta RH
        pla
        pha
        cmp PTOOL
        bne @up
        lda #1
        bra @b
@up:    lda #0
@b:     jsr pt_btn
        inc SX
        inc SX
        inc SY
        inc SY
        pla
        asl
        tax
        TBL pt_icons
        sta SPR
        CALL blit4
        rts
; Werkzeug-Symbole (assets.py: 9x9)
pt_icons: .addr i_ppen, i_pbrush, i_perase, i_pspray, i_pline, i_prect, i_pfill, i_pbuck
          .addr i_pell, i_pellf, i_ppick, i_ptext

; Knopf RX..RH, A=1 gedrueckt (wie cd_btn)
pt_btn: pha
        lda #GRAY
        CALL setcolor
        CALL fillrect
        pla
        bne @dn
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        bra @b
@dn:    lda #BLACK
        sta C1
        lda #WHITE
        sta C2
@b:     CALL bevel
        rts

; Farbfeld A
pt_sw:  pha
        asl
        asl
        asl
        asl
        sec
        sbc 1,s
        sbc 1,s                 ; *14
        clc
        adc #WX+PT_PX
        sta RX
        lda #WY+PT_PY
        sta RY
        lda #12
        sta RW
        lda #9
        sta RH
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        pla
        CALL setcolor
        CALL fillrect
        rts

; aktuelle Farbe links unten
pt_ccur:
        lda #WX+4
        sta RX
        lda #WY+PT_PY-2
        sta RY
        lda #24
        sta RW
        lda #12
        sta RH
        lda #BLACK
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda PCOL
        CALL setcolor
        CALL fillrect
        rts

; Statuszeile
pt_stat:
        lda #BLACK
        CALL setcolor
        lda #WX+4
        sta SX
        lda #WY+PT_SY
        sta SY
        lda #pt_thelp
        ldx LANG
        beq @d
        lda #pt_thelp_en
@d:     jmp pt_str
; Text A dieser Bank bei SX/SY
pt_str: sta PDST
        phk
        sep #$20
        pla
        rep #$20
        and #$00FF
        sta PDST+2
        JSLF xf_ui_s2
        rts

; ---------- Zeichenflaeche aus dem Bild auf den Schirm ----------
; Bereich QPVX0..QPVX1 / QPVY0..QPVY1 (Schirm), begrenzt auf die Zeichenflaeche
pt_blit:
        lda QPVX0
        cmp #PT_CX
        bcs @a
        lda #PT_CX
@a:     sta QPVX0
        lda QPVX1
        cmp #PT_CX+PT_CW
        bcc @b
        lda #PT_CX+PT_CW
@b:     sta QPVX1
        lda QPVY0
        cmp #PT_CY
        bcs @c
        lda #PT_CY
@c:     sta QPVY0
        lda QPVY1
        cmp #PT_CY+PT_CH
        bcc @d
        lda #PT_CY+PT_CH
@d:     sta QPVY1
        lda QPVX0
        cmp QPVX1
        bcc *+3
        rts
        lda QPVY0
        cmp QPVY1
        bcc *+3
        rts
        jsr pt_dp
        lda QPVY0
        sta QPJ
@row:   lda QPJ                 ; Schirmzeile -> Bildzeile
        sec
        sbc #PT_CY
        clc
        adc QOFY
        jsr pt_m160
        sta PTA                 ; Bild: Zeilenanfang
        lda QPJ                 ; Ziel: Zeile ueber linetab (beim Neuzeichnen der
        asl                     ; Fenster ist das ein Puffer, sonst der Schirm)
        tax
        lda linetab,x
        sta PTB
        lda QPVX0
        sta QPI
        lda QPVX1
        sta QPN
        lda QPI                 ; ungerader Anfang: ein Halbbyte
        and #1
        beq @e0
        lda QPI
        jsr pt_nib
        inc QPI
@e0:    lda QPN                 ; ungerades Ende: ein Halbbyte
        and #1
        beq @e1
        dec QPN
        lda QPN
        jsr pt_nib
@e1:    lda QPI                 ; dazwischen ganze Bytes
        cmp QPN
        bcs @nx
        lsr
        clc
        adc PTB
        sta PTR
        lda QPI
        sec
        sbc #PT_CX
        clc
        adc QOFX
        lsr
        clc
        adc PTA
        clc
        adc PDST
        sta LP2
        lda PDST+2
        sta LP2+2
        lda QPN
        sec
        sbc QPI
        lsr
        sta QPW
        ldy #0
        sep #$20
@by:    lda [LP2],y
        sta [PTR],y
        iny
        cpy QPW
        bcc @by
        rep #$20
@nx:    inc QPJ
        lda QPJ
        cmp QPVY1
        bcs *+5
        jmp @row
        rts

; Halbbyte fuer Schirmspalte A (Zeilen PTA/PTB) kopieren
pt_nib: pha
        lsr
        clc
        adc PTB
        sta PTR
        lda 1,s
        sec
        sbc #PT_CX
        clc
        adc QOFX
        lsr
        clc
        adc PTA
        tay
        pla
        and #1
        sep #$20
        bne @o
        lda [PDST],y
        and #$F0
        sta QPE2
        lda [PTR]
        and #$0F
        ora QPE2
        sta [PTR]
        rep #$20
        rts
@o:
        .a8
        lda [PDST],y
        and #$0F
        sta QPE2
        lda [PTR]
        and #$F0
        ora QPE2
        sta [PTR]
        rep #$20
        rts

; A * 160
pt_m160:
        asl
        asl
        asl
        asl
        asl
        sta QPE2
        asl
        asl
        clc
        adc QPE2
        rts

; ganze Zeichenflaeche neu (Maus dabei versteckt)
pt_cblit:
        CALL cur_hide
        jsr pt_cblit0
        CALL cur_show
        rts
pt_cblit0:
        lda #PT_CX
        sta QPVX0
        lda #PT_CX+PT_CW
        sta QPVX1
        lda #PT_CY
        sta QPVY0
        lda #PT_CY+PT_CH
        sta QPVY1
        jmp pt_blit

; ---------- Punkt QPX/QPY (Bild) in Farbe PCB, auch auf dem Schirm ----------
pt_pset:
        lda QPX
        cmp #320
        bcc *+5
        jmp @r
        lda QPY
        cmp #200
        bcc *+5
        jmp @r
        lda QPSO                 ; Vorschau: nur Schirm
        bne @scr
        lda QPY
        jsr pt_m160
        sta PTA
        lda QPX
        lsr
        clc
        adc PTA
        tay
        lda QPX
        and #1
        sep #$20
        bne @o
        lda [PDST],y
        and #$0F
        sta QPE2
        lda PCB
        and #$F0
        ora QPE2
        sta [PDST],y
        bra @s0
@o:
        .a8
        lda [PDST],y
        and #$F0
        sta QPE2
        lda PCB
        and #$0F
        ora QPE2
        sta [PDST],y
@s0:    rep #$20
@scr:   lda PDONLY              ; nur ins Bild (Uebernahme aus dem Browser)
        beq @scr2
        rts
@scr2:  lda QPX                  ; sichtbar?
        sec
        sbc QOFX
        bmi @r
        cmp #PT_CW
        bcs @r
        clc
        adc #PT_CX
        sta PTA
        lda QPY
        sec
        sbc QOFY
        bmi @r
        cmp #PT_CH
        bcs @r
        clc
        adc #PT_CY
        asl
        tax
        lda linetab,x
        sta PTB
        lda PTA
        lsr
        clc
        adc PTB
        sta PTR
        lda PTA
        and #1
        sep #$20
        bne @so
        lda [PTR]
        and #$0F
        sta QPE2
        lda PCB
        and #$F0
        ora QPE2
        sta [PTR]
        rep #$20
@r:     rts
@so:
        .a8
        lda [PTR]
        and #$F0
        sta QPE2
        lda PCB
        and #$0F
        ora QPE2
        sta [PTR]
        rep #$20
        rts

; Farbe A setzen (PCB)
pt_setc:
        and #$000F
        sta PCB
        asl
        asl
        asl
        asl
        ora PCB
        sta PCB
        rts

; Farbe des Bildpunkts QPX/QPY -> A
pt_pget:
        lda QPY
        jsr pt_m160
        sta PTA
        lda QPX
        lsr
        clc
        adc PTA
        tay
        lda [PDST],y
        and #$00FF
        tax
        lda QPX
        and #1
        bne @o
        txa
        lsr
        lsr
        lsr
        lsr
        rts
@o:     txa
        and #$000F
        rts

; Pinselspitze am Punkt QPX/QPY je nach Werkzeug
pt_tip: lda PTOOL
        cmp #1
        beq @br
        cmp #2
        beq @er
        jmp pt_pset
@br:    lda #1                  ; Pinsel: 3x3
        ldx #3
        bra @sq
@er:    lda #3                  ; Radierer: 8x8 weiss
        ldx #8
@sq:    stx QPN
        sta PTB
        lda QPX
        pha
        lda QPY
        pha
        lda QPX
        sec
        sbc PTB
        sta QPSXS
        lda QPY
        sec
        sbc PTB
        sta QPSYS
        stz QPJ
@y:     stz QPI
@x:     lda QPSXS
        clc
        adc QPI
        sta QPX
        lda QPSYS
        clc
        adc QPJ
        sta QPY
        jsr pt_pset
        inc QPI
        lda QPI
        cmp QPN
        bcc @x
        inc QPJ
        lda QPJ
        cmp QPN
        bcc @y
        pla
        sta QPY
        pla
        sta QPX
        rts

; Linie QPX0/QPY0 -> QPX1/QPY1, an jedem Punkt pt_tip
pt_line:
        lda QPX1
        sec
        sbc QPX0
        ldx #1
        bcs @dx
        eor #$FFFF
        inc a
        ldx #$FFFF
@dx:    sta QPDX
        stx QPSXS
        lda QPY1
        sec
        sbc QPY0
        ldx #1
        bcs @dy
        eor #$FFFF
        inc a
        ldx #$FFFF
@dy:    sta QPDY
        stx QPSYS
        lda QPDX
        sec
        sbc QPDY
        sta QPER
@p:     lda QPX0
        sta QPX
        lda QPY0
        sta QPY
        lda QPSXS                ; (pt_tip benutzt QPSXS/QPSYS fuer Quadrate)
        pha
        lda QPSYS
        pha
        jsr pt_tip
        pla
        sta QPSYS
        pla
        sta QPSXS
        lda QPX0
        cmp QPX1
        bne @go
        lda QPY0
        cmp QPY1
        beq @r
@go:    lda QPER
        asl
        sta QPE2
        clc
        adc QPDY
        bmi @y
        beq @y
        lda QPER
        sec
        sbc QPDY
        sta QPER
        lda QPX0
        clc
        adc QPSXS
        sta QPX0
@y:     lda QPE2
        sec
        sbc QPDX
        bpl @p
        lda QPER
        clc
        adc QPDX
        sta QPER
        lda QPY0
        clc
        adc QPSYS
        sta QPY0
        bra @p
@r:     rts

; Rechteck QPSX/QPSY - QPX1/QPY1 (A=1 gefuellt)
pt_rect:
        pha
        lda QPSX                 ; geordnet nach QPI..QPN / QPJ..PTA
        cmp QPX1
        bcc @x
        lda QPX1
@x:     sta QPVX0
        lda QPSX
        cmp QPX1
        bcs @x2
        lda QPX1
@x2:    sta QPVX1
        lda QPSY
        cmp QPY1
        bcc @y
        lda QPY1
@y:     sta QPVY0
        lda QPSY
        cmp QPY1
        bcs @y2
        lda QPY1
@y2:    sta QPVY1
        pla
        beq @o
        lda QPVY0                ; gefuellt
        sta QPY
@fr:    lda QPVX0
        sta QPX
@fc:    jsr pt_pset
        inc QPX
        lda QPX
        cmp QPVX1
        bcc @fc
        beq @fc
        inc QPY
        lda QPY
        cmp QPVY1
        bcc @fr
        beq @fr
        rts
@o:     lda QPVX0                ; Rahmen
        sta QPX
@h:     lda QPVY0
        sta QPY
        jsr pt_pset
        lda QPVY1
        sta QPY
        jsr pt_pset
        inc QPX
        lda QPX
        cmp QPVX1
        bcc @h
        beq @h
        lda QPVY0
        sta QPY
@v:     lda QPVX0
        sta QPX
        jsr pt_pset
        lda QPVX1
        sta QPX
        jsr pt_pset
        inc QPY
        lda QPY
        cmp QPVY1
        bcc @v
        beq @v
        rts

; ---------- Rueckgaengig ----------
pt_save:
        jsr pt_dp
        jsr pt_up
        ldy #0
@l:     lda [PDST],y
        sta [PSRC],y
        iny
        iny
        cpy #32000
        bcc @l
        lda #1
        sta QPUOK
        rts
pt_undo:
        lda QPUOK
        beq @r
        jsr pt_dp
        jsr pt_up
        ldy #0
@l:     lda [PDST],y            ; tauschen: noch einmal Z = wiederherstellen
        tax
        lda [PSRC],y
        sta [PDST],y
        txa
        sta [PSRC],y
        iny
        iny
        cpy #32000
        bcc @l
        jsr pt_cblit
@r:     rts

; ---------- Maus ----------
; Schirm QPMX/QPMY -> Bild QPX/QPY
pt_m2p: lda QPMX
        sec
        sbc #PT_CX
        clc
        adc QOFX
        sta QPX
        lda QPMY
        sec
        sbc #PT_CY
        clc
        adc QOFY
        sta QPY
        rts

; Zeichnen ausserhalb des Neuzeichnens: ganzes Fenster
pt_kwin:
        stz KL
        stz KT
        lda #320
        sta KR
        lda #186
        sta KB
        rts

pt_click:
        lda EVREC+12
        sta QPMX
        lda EVREC+10
        sta QPMY
        ; Werkzeuge
        lda QPMX
        sec
        sbc #PT_TX
        cmp #28
        bcs @nt
        lsr                     ; Spalte: 0..13 / 14..27
        cmp #7
        lda #0
        bcc @c0
        lda #1
@c0:    sta QPI
        lda QPMY
        sec
        sbc #PT_TY
        cmp #PT_NT/2*14
        bcs @nt
        ldx #0
@r:     cmp #14
        bcc @rr
        sbc #14
        inx
        bra @r
@rr:    txa
        asl
        ora QPI
        cmp #10                 ; Pipette: danach zurueck zum vorigen Werkzeug
        bne @st
        ldx PTOOL
        cpx #10
        beq @st
        stx PPREV
@st:    sta PTOOL
        jsr pt_kwin
        stz QPI
@tb:    lda QPI
        jsr pt_tbtn
        inc QPI
        lda QPI
        cmp #PT_NT
        bcc @tb
        rts
@nt:    lda QPMY                 ; Farbfelder
        sec
        sbc #PT_PY
        cmp #9
        bcs @np
        lda QPMX
        sec
        sbc #PT_PX
        cmp #16*14
        bcs @np
        ldx #0
@pc:    cmp #14
        bcc @pcd
        sbc #14
        inx
        bra @pc
@pcd:   stx PCOL
        jsr pt_kwin
        jmp pt_ccur
@np:    lda QPMX                 ; Zeichenflaeche?
        sec
        sbc #PT_CX
        cmp #PT_CW
        bcs @r2
        lda QPMY
        sec
        sbc #PT_CY
        cmp #PT_CH
        bcs @r2
        lda PTOOL
        cmp #10
        bne @np2
        jmp pt_pick
@np2:   jmp pt_stroke
@r2:    rts

; naechstes Ereignis -> QPMX/QPMY; C=1 Maustaste losgelassen
pt_ev:  pha
        pea $FFFF
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        lda EVREC+12
        sta QPMX
        lda EVREC+10
        sta QPMY
        lda EVREC
        cmp #2
        beq @up
        clc
        rts
@up:    sec
        rts

pt_stroke:
        jsr pt_save
        lda #1
        sta QPDIRTY
        jsr pt_dp
        lda PCOL
        ldx PTOOL
        cpx #2
        bne @c
        lda #15                 ; Radierer: weiss
@c:     jsr pt_setc
        stz QPSO
        jsr pt_m2p
        lda QPX
        sta QPLX
        sta QPSX
        lda QPY
        sta QPLY
        sta QPSY
        lda PTOOL
        cmp #7
        bne @nf
        jmp pt_fill
@nf:    cmp #11
        bne @nf2
        jmp pt_text
@nf2:   cmp #4
        bcc @free
        jmp pt_shape
@free:  CALL cur_hide
@l:     lda PTOOL               ; freihand: Stift, Pinsel, Radierer, Spruehdose
        cmp #3
        beq @sp
        lda QPLX
        sta QPX0
        lda QPLY
        sta QPY0
        lda QPX
        sta QPX1
        lda QPY
        sta QPY1
        jsr pt_line
        bra @nx
@sp:    jsr pt_spray
@nx:    lda QPX
        sta QPLX
        lda QPY
        sta QPLY
        jsr pt_ev
        php
        jsr pt_m2p
        plp
        bcc @l
        CALL cur_show
        rts

; Spruehdose: 6 Punkte im Umkreis von 6
pt_spray:
        lda QPX
        pha
        lda QPY
        pha
        lda #6
        sta QPN
@l:     jsr pt_rnd
        and #15
        sec
        sbc #7
        clc
        adc 3,s
        sta QPX
        jsr pt_rnd
        and #15
        sec
        sbc #7
        clc
        adc 1,s
        sta QPY
        jsr pt_pset
        dec QPN
        bne @l
        pla
        sta QPY
        pla
        sta QPX
        rts
pt_rnd: lda QPRND
        bne @n
        lda #$7A31
@n:     asl
        eor QPRND
        sta QPRND
        lsr
        lsr
        lsr
        eor QPRND
        sta QPRND
        xba
        asl
        eor QPRND
        sta QPRND
        rts

; Linie / Rechteck: Vorschau beim Ziehen (nur Schirm), dann ins Bild
pt_shape:
        stz QPVX1
        CALL cur_hide
@l:     jsr pt_ev
        php
        jsr pt_m2p
        lda QPX
        sta QPX1
        lda QPY
        sta QPY1
        jsr pt_unprev           ; alte Vorschau weg
        plp
        bcs @end
        lda #1
        sta QPSO
        jsr pt_shp
        stz QPSO
        jsr pt_box              ; Bereich der Vorschau merken
        bra @l
@end:   stz QPSO
        jsr pt_shp
        CALL cur_show
        rts

pt_shp: lda PTOOL
        cmp #4
        bne @r
        lda QPSX
        sta QPX0
        lda QPSY
        sta QPY0
        lda #0                  ; (Werkzeug 4: Linie mit dem Stift)
        pha
        lda PTOOL
        pha
        stz PTOOL
        jsr pt_line
        pla
        sta PTOOL
        pla
        rts
@r:     cmp #8
        bcs @e
        sec
        sbc #5                  ; 5 Rahmen, 6 gefuellt
        jmp pt_rect
@e:     sbc #8                  ; 8 Ellipse, 9 gefuellt
        ldx QPX1                ; (pt_ell braucht QPX/QPY und QPX0..QPY1 selbst)
        stx PSV1
        ldx QPY1
        stx PSV2
        ldx QPX
        stx PSV3
        ldx QPY
        stx PSV4
        jsr pt_ell
        lda PSV1
        sta QPX1
        lda PSV2
        sta QPY1
        lda PSV3
        sta QPX
        lda PSV4
        sta QPY
        rts

; Schirmbereich der Vorschau (QPSX/QPSY .. QPX1/QPY1) -> QPVX0..QPVY1 (+1)
pt_box: lda QPSX
        cmp QPX1
        bcc @a
        lda QPX1
@a:     sec
        sbc QOFX
        clc
        adc #PT_CX
        sta QPVX0
        lda QPSX
        cmp QPX1
        bcs @b
        lda QPX1
@b:     sec
        sbc QOFX
        clc
        adc #PT_CX+1
        sta QPVX1
        lda QPSY
        cmp QPY1
        bcc @c
        lda QPY1
@c:     sec
        sbc QOFY
        clc
        adc #PT_CY
        sta QPVY0
        lda QPSY
        cmp QPY1
        bcs @d
        lda QPY1
@d:     sec
        sbc QOFY
        clc
        adc #PT_CY+1
        sta QPVY1
        rts
pt_unprev:
        lda QPVX1
        beq @r
        lda QPVX0                ; (negativ: vom Rand abschneiden)
        bpl @a
        stz QPVX0
@a:     lda QPVY0
        bpl @b
        stz QPVY0
@b:     jsr pt_blit
        stz QPVX1
        jsr pt_dp
@r:     rts

; ---------- Fuellen (Zeilenweise, Stapel im Griff PTS) ----------
pt_fill:
        jsr pt_pget
        sta QPOLD
        cmp PCOL
        bne @go
        rts
@go:    lda PSTK
        sta PSRC
        lda PSTK+2
        sta PSRC+2
        stz QPSP
        jsr pt_push
        CALL cur_hide
@pop:   lda QPSP
        bne @p1
        CALL cur_show
        rts
@p1:    sec
        sbc #4
        sta QPSP
        tay
        lda [PSRC],y
        sta QPX
        iny
        iny
        lda [PSRC],y
        sta QPY
        jsr pt_pget             ; schon gefuellt?
        cmp QPOLD
        bne @pop
@lf:    lda QPX                  ; nach links bis zum Rand
        beq @ls
        dec QPX
        jsr pt_pget
        cmp QPOLD
        beq @lf
        inc QPX
@ls:    stz QPI                  ; oben/unten schon gemerkt?
        stz QPJ
@sc:    jsr pt_pset             ; (Bild und Schirm)
        lda QPY                  ; Zeile darueber
        beq @nb
        dec QPY
        jsr pt_pget
        inc QPY
        cmp QPOLD
        bne @na
        lda QPI
        bne @nb
        dec QPY
        jsr pt_push
        inc QPY
        lda #1
        sta QPI
        bra @nb
@na:    stz QPI
@nb:    lda QPY                  ; Zeile darunter
        cmp #199
        bcs @nc
        inc QPY
        jsr pt_pget
        dec QPY
        cmp QPOLD
        bne @nd
        lda QPJ
        bne @nc
        inc QPY
        jsr pt_push
        dec QPY
        lda #1
        sta QPJ
        bra @nc
@nd:    stz QPJ
@nc:    inc QPX
        lda QPX
        cmp #320
        bcs @pp
        jsr pt_pget
        cmp QPOLD
        beq @sc
@pp:    jmp @pop
; QPX/QPY auf den Stapel (voll: weglassen)
pt_push:
        lda QPSP
        cmp #$2000-4
        bcs @r
        tay
        lda QPX
        sta [PSRC],y
        iny
        iny
        lda QPY
        sta [PSRC],y
        lda QPSP
        clc
        adc #4
        sta QPSP
@r:     rts

; ---------- Statuszeile: Text A (Deutsch) / X (Englisch) ----------
pt_smsg:
        ldy LANG
        beq @d
        txa
@d:     pha
        jsr pt_kwin
        lda #WX+2
        sta RX
        lda #WY+PT_SY-1
        sta RY
        lda #316
        sta RW
        lda #10
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #BLACK
        CALL setcolor
        lda #WX+4
        sta SX
        lda #WY+PT_SY
        sta SY
        pla
        jmp pt_str

; ---------- Pipette: Farbe aus dem Bild, dann voriges Werkzeug ----------
pt_pick:
        jsr pt_dp
        jsr pt_m2p
        lda QPX
        cmp #320
        bcs @r
        lda QPY
        cmp #200
        bcs @r
        jsr pt_pget
        sta PCOL
        lda PPREV
        cmp #10
        bcc @t
        lda #0
@t:     sta PTOOL
        jsr pt_kwin
        jsr pt_ccur
        stz QPI
@b:     lda QPI
        jsr pt_tbtn
        inc QPI
        lda QPI
        cmp #PT_NT
        bcc @b
@r:     rts

; ---------- kleine Rechenhilfen (16 Bit) ----------
; A * X -> A
pt_mul: sta PEQ
        stx PEF
        lda #0
        ldy #16
@l:     asl
        asl PEQ
        bcc @n
        clc
        adc PEF
@n:     dey
        bne @l
        rts
; A / X -> A (X > 0)
pt_div: sta PEQ
        stx PEF
        lda #0
        ldy #16
@l:     asl PEQ
        rol
        cmp PEF
        bcc @n
        sbc PEF
        inc PEQ
@n:     dey
        bne @l
        lda PEQ
        rts

; ---------- Ellipse im Rechteck QPSX/QPSY - QPX1/QPY1 (A=1 gefuellt) ----------
pt_ell: sta PEF+0               ; (PEF wird von pt_mul benutzt: merken)
        pha
        lda QPSX                ; links/rechts, oben/unten
        cmp QPX1
        bcc @x
        lda QPX1
@x:     sta QPVX0
        lda QPSX
        cmp QPX1
        bcs @x2
        lda QPX1
@x2:    sec
        sbc QPVX0
        lsr
        sta PEHA                 ; Halbachse x
        clc
        adc QPVX0
        sta PECX
        lda QPSY
        cmp QPY1
        bcc @y
        lda QPY1
@y:     sta QPVY0
        lda QPSY
        cmp QPY1
        bcs @y2
        lda QPY1
@y2:    sec
        sbc QPVY0
        lsr
        sta PEHB                 ; Halbachse y
        clc
        adc QPVY0
        sta PECY
        pla
        sta PTL                 ; gefuellt?
        lda PEHB
        bne @ok
        lda PEHA                 ; flach: eine Linie
        sta PEDX
        stz PEDY
        jmp pt_espan
@ok:    lda PEHB                 ; s = Wurzel(b*b - dy*dy), fallend
        sta PES
        lda PEHA
        sta PEPX                ; voriger Punkt: (a, 0)
        stz PEPY
        stz PEDY
@row:   lda PEDY                ; Grenze b*b - dy*dy
        tax
        jsr pt_mul
        sta PTX
        lda PEHB
        tax
        jsr pt_mul
        sec
        sbc PTX
        clc                     ; + b: rundere Enden
        adc PEHB
        sta PTX
@sq:    lda PES                 ; s so lange verkleinern, bis s*s <= Grenze
        beq @sd
        tax
        jsr pt_mul
        cmp PTX
        beq @sd
        bcc @sd
        dec PES
        bra @sq
@sd:    lda PES                 ; dx = a * s / b
        ldx PEHA
        jsr pt_mul
        ldx PEHB
        jsr pt_div
        sta PEDX
        lda PTL
        beq @o
        jsr pt_espan            ; gefuellt: zwei Zeilen
        bra @nx
@o:     jsr pt_eseg             ; Rahmen: vom vorigen Punkt verbinden
        lda PEDY                ; oberste/unterste Zeile ganz
        cmp PEHB
        bne @nx
        jsr pt_espan
@nx:    lda PEDX
        sta PEPX
        lda PEDY
        sta PEPY
        inc PEDY
        lda PEDY
        cmp PEHB
        bcc @row
        beq @row
        rts

; Zeilen PECY +- PEDY von PECX-PEDX bis PECX+PEDX
pt_espan:
        lda PECY
        clc
        adc PEDY
        jsr @ln
        lda PECY
        sec
        sbc PEDY
@ln:    sta QPY
        lda PECX
        sec
        sbc PEDX
        sta QPX
        lda PECX
        clc
        adc PEDX
        sta PTY
@l:     jsr pt_pset
        inc QPX
        lda QPX
        cmp PTY
        bcc @l
        beq @l
        rts

; vier Linien vom vorigen Punkt (PEPX/PEPY) zum neuen (PEDX/PEDY)
pt_eseg:
        lda PTOOL
        pha
        stz PTOOL               ; mit dem Stift
        stz PTY
@q:     lda PTY                 ; Viertel 0..3: Vorzeichen x (Bit 0), y (Bit 1)
        and #1
        tax
        lda PEPX
        jsr pt_esg
        clc
        adc PECX
        sta QPX0
        lda PEDX
        jsr pt_esg
        clc
        adc PECX
        sta QPX1
        lda PTY
        lsr
        and #1
        tax
        lda PEPY
        jsr pt_esg
        clc
        adc PECY
        sta QPY0
        lda PEDY
        jsr pt_esg
        clc
        adc PECY
        sta QPY1
        jsr pt_line
        inc PTY
        lda PTY
        cmp #4
        bcc @q
        pla
        sta PTOOL
        rts
; A negativ, wenn X = 1
pt_esg: cpx #1
        bne @r
        eor #$FFFF
        inc a
@r:     rts

; ---------- Text: an der Klickstelle tippen, Return setzt, Esc bricht ab ----------
PT_TH = 10                      ; Zeilenhoehe
pt_text:
        lda QPMX
        sta PTX
        lda QPMY
        sta PTY
        stz PTXT
        stz PTL
        CALL cur_hide
@lp:    jsr @show
@ev:    pha
        pea $FFFF
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        beq @ev
        lda EVREC
        cmp #1
        beq @ok                 ; Klick: Text setzen
        cmp #3                  ; Taste (sonst weiter warten)
        beq @key
        cmp #5
        bne @ev
@key:   lda EVREC+2
        and #$00FF
        cmp #$0D
        beq @ok
        cmp #$1B
        beq @can
        cmp #$08
        beq @bs
        cmp #$7F
        beq @bs
        cmp #' '
        bcc @ev
        ldx PTL
        cpx #40
        bcs @ev
        sep #$20
        sta PTXT+1,x
        rep #$20
        inc PTL
        bra @lp
@bs:    lda PTL
        beq @ev
        dec PTL
        bra @lp
@can:   jsr @rest
        jsr pt_kwin
        CALL cur_show
        rts
@ok:    lda PTL                 ; ohne Schreibmarke zeichnen, dann ins Bild
        sep #$20
        sta PTXT
        rep #$20
        jsr @rest
        jsr @draw
        jsr pt_s2d
        jsr pt_kwin
        CALL cur_show
        rts
@show:  jsr @rest
        lda PTL                 ; mit Schreibmarke "_"
        tax
        inc a
        sep #$20
        sta PTXT
        lda #'_'
        sta PTXT+1,x
        rep #$20
        jmp @draw
@rest:  lda PTX                 ; Zeilenbereich aus dem Bild zuruecksetzen
        sta QPVX0
        lda #PT_CX+PT_CW
        sta QPVX1
        lda PTY
        sta QPVY0
        clc
        adc #PT_TH
        sta QPVY1
        jsr pt_blit
        jsr pt_dp
        rts
@draw:  lda #PT_CX              ; nur in der Zeichenflaeche
        sta KL
        lda #PT_CX+PT_CW
        sta KR
        lda #PT_CY
        sta KT
        lda #PT_CY+PT_CH
        sta KB
        lda PCOL
        CALL setcolor
        lda PTX
        clc
        adc #WX
        sta SX
        lda PTY
        clc
        adc #WY
        sta SY
        lda #PTXT
        sta PDST
        lda MYBANK
        sta PDST+2
        JSLF xf_ui_s2
        jsr pt_dp
        rts

; Schirmbereich der Textzeile ins Bild uebernehmen (Punkt fuer Punkt)
pt_s2d: lda PTY
        sta QPJ
@y:     lda QPJ
        cmp #PT_CY+PT_CH
        bcs @r
        lda PTX
        sta QPI
@x:     lda QPI
        cmp #PT_CX+PT_CW
        bcs @nx
        lda QPJ                 ; Schirmpunkt lesen
        asl
        tax
        lda linetab,x
        sta PTA
        lda QPI
        lsr
        clc
        adc PTA
        sta PTR
        lda [PTR]
        and #$00FF
        tax
        lda QPI
        and #1
        bne @o
        txa
        lsr
        lsr
        lsr
        lsr
        bra @c
@o:     txa
        and #$000F
@c:     jsr pt_setc
        lda QPI                 ; Bildpunkt
        sec
        sbc #PT_CX
        clc
        adc QOFX
        sta QPX
        lda QPJ
        sec
        sbc #PT_CY
        clc
        adc QOFY
        sta QPY
        jsr pt_pset
        inc QPI
        bra @x
@nx:    inc QPJ
        lda QPJ
        sec
        sbc PTY
        cmp #PT_TH
        bcc @y
@r:     lda PCOL
        jmp pt_setc

; ---------- Tasten ----------
pt_key: lda KEYC
        and #$00FF
        cmp #$1B
        bne @k0
        lda QPDIRTY             ; ungespeichert: erst nachfragen
        beq @cl
        lda PESC
        bne @cl
        inc PESC
        lda #pt_tesc
        ldx #pt_tesc_en
        jmp pt_smsg
@cl:    stz PESC
        CALL close_win
        rts
@k0:    pha
        lda PESC                ; andere Taste: Hilfezeile zurueck
        beq @k0b
        stz PESC
        lda #pt_thelp
        ldx #pt_thelp_en
        jsr pt_smsg
@k0b:   pla
@k1:    cmp #$1A                ; Strg-Z
        beq @u
        ora #$20
        cmp #'z'
        bne @k2
@u:     jmp pt_undo
@k2:    cmp #'n'
        bne @k3
        jsr pt_save
        jsr pt_new
        lda #1
        sta QPUOK
        jsr pt_cblit
        lda #pt_thelp
        ldx #pt_thelp_en
        jmp pt_smsg
@k3:    cmp #'s'
        bne @k4
        jmp pt_write
@k4:    lda KEYC
        and #$00FF
        cmp #$08                ; links
        bne @k5
        lda QOFX
        sec
        sbc #8
        bpl @kx
        lda #0
        bra @kx
@k5:    cmp #$15                ; rechts
        bne @k6
        lda QOFX
        clc
        adc #8
        cmp #320-PT_CW
        bcc @kx
        lda #320-PT_CW
@kx:    sta QOFX
        jmp pt_cblit
@k6:    cmp #$0B                ; hoch
        bne @k7
        lda QOFY
        sec
        sbc #8
        bpl @ky
        lda #0
        bra @ky
@k7:    cmp #$0A                ; runter
        bne @r
        lda QOFY
        clc
        adc #8
        cmp #200-PT_CH
        bcc @ky
        lda #200-PT_CH
@ky:    sta QOFY
        jmp pt_cblit
@r:     rts

; ---------- Speichern als $C1-Bild ----------
; Pfad PNAME (vom Bildbetrachter) oder freier Name *:BILD1 ... *:BILD99
pt_write:
        lda PNAME
        bne @have
        lda #1
        sta QPI
@try:   jsr pt_mkname
        lda #3                  ; GetFileInfo: gibt es den Namen schon?
        sta PPB
        lda #PNAME
        sta PPB+2
        lda MYBANK
        sta PPB+4
        lda #$2006
        ldx #PPB
        jsr pt_gs
        bcs @free
        inc QPI
        lda QPI
        cmp #100
        bcc @try
        jmp pt_err
@free:  lda #5                  ; Create: Typ $C1, Zusatz 0, normale Datei
        sta PPB
        lda #1
        sta PPB+14
        lda #PNAME
        sta PPB+2
        lda MYBANK
        sta PPB+4
        lda #$00C3
        sta PPB+6
        lda #$00C1
        sta PPB+8
        stz PPB+10
        stz PPB+12
        lda #$2001
        ldx #PPB
        jsr pt_gs
        bcc @have
        jmp pt_err
@have:  lda #2                  ; Open
        sta PPB
        lda #PNAME
        sta PPB+4
        lda MYBANK
        sta PPB+6
        lda #$2010
        ldx #PPB
        jsr pt_gs
        bcc @o
        jmp pt_err
@o:     lda PPB+2
        sta QPW
        lda #4                  ; Write 32768
        sta PPB
        lda QPW
        sta PPB+2
        lda PDOC
        sta PPB+4
        lda PDOC+2
        sta PPB+6
        lda #$8000              ; 32 KB
        sta PPB+8
        stz PPB+10
        lda #$2013
        ldx #PPB
        jsr pt_gs
        php
        lda #3                  ; SetEOF 32768 (alte Datei kuerzen)
        sta PPB
        lda QPW
        sta PPB+2
        stz PPB+4
        lda #$8000
        sta PPB+6
        stz PPB+8
        lda #$2018
        ldx #PPB
        jsr pt_gs
        lda #1                  ; Close
        sta PPB
        lda QPW
        sta PPB+2
        lda #$2014
        ldx #PPB
        jsr pt_gs
        plp
        bcs pt_err
        stz QPDIRTY
        jsr pt_kwin             ; Meldung in der Statuszeile: Name
        lda #WX+2
        sta RX
        lda #WY+PT_SY-1
        sta RY
        lda #316
        sta RW
        lda #10
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #BLACK
        CALL setcolor
        lda #WX+4
        sta SX
        lda #WY+PT_SY
        sta SY
        lda #pt_tsaved
        ldx LANG
        beq @d
        lda #pt_tsaved_en
@d:     jsr pt_str
        lda #PNAME+1            ; Pfad (Laenge im ersten Byte)
        sta PDST
        lda MYBANK
        sta PDST+2
        sep #$20
        lda PNAME
        sta PNAME+1             ; (Laenge 1 Byte vor dem Text)
        rep #$20
        JSLF xf_ui_s2
        jsr pt_fixname
        rts
pt_err: CALL fail               ; GS/OS-Fehler A im Klartext
        CALL redraw_all
        rts

; *:BILD<QPI> nach PNAME (Laenge 2 Bytes + Text)
pt_mkname:
        ldx #0
@c:     TBL pt_sname
        sta PNAME+2,x
        inx
        inx
        cpx #8
        bcc @c
        ldx #6                  ; nach "*:BILD"
        lda QPI
        cmp #10
        bcc @one
        ldy #'0'
@t:     cmp #10
        bcc @td
        sbc #10
        iny
        bra @t
@td:    pha
        tya
        sep #$20
        sta PNAME+2,x
        rep #$20
        inx
        pla
@one:   ora #'0'
        sep #$20
        sta PNAME+2,x
        rep #$20
        inx
        stx PNAME
        rts
pt_sname: .byte "*:BILD",0,0
; nach der Anzeige: Laenge wieder als Wort (2 Bytes)
pt_fixname:
        lda PNAME+1
        and #$00FF
        sta PNAME
        rts

pt_gs:  sta PTA
        lda MYBANK
        pha
        phx
        lda PTA
        pha
        jsl $E100B0
        rts

pt_thelp: .byte 52,$53,$20,$73,$70,$65,$69,$63,$68,$65,$72,$6E,$20,$20,$5A,$20,$72,$82,$63,$6B,$67,$80,$6E,$67,$69,$67,$20,$20,$4E,$20,$6E,$65,$75,$20,$20,$50,$66,$65,$69,$6C,$65,$3A,$20,$41,$75,$73,$73,$63,$68,$6E,$69,$74,$74
pt_thelp_en: .byte 44,$53,$20,$73,$61,$76,$65,$20,$20,$5A,$20,$75,$6E,$64,$6F,$20,$20,$4E,$20,$6E,$65,$77,$20,$20,$41,$72,$72,$6F,$77,$73,$3A,$20,$6D,$6F,$76,$65,$20,$74,$68,$65,$20,$76,$69,$65,$77
pt_tsaved: .byte 13,$47,$65,$73,$70,$65,$69,$63,$68,$65,$72,$74,$3A,$20
pt_tsaved_en: .byte 7,$53,$61,$76,$65,$64,$3A,$20
pt_tesc: .byte 49,$4E,$69,$63,$68,$74,$20,$67,$65,$73,$70,$65,$69,$63,$68,$65,$72,$74,$21,$20,$53,$20,$3D,$20,$73,$70,$65,$69,$63,$68,$65,$72,$6E,$2C,$20,$45,$73,$63,$20,$3D,$20,$76,$65,$72,$77,$65,$72,$66,$65,$6E
pt_tesc_en: .byte 34,$4E,$6F,$74,$20,$73,$61,$76,$65,$64,$21,$20,$53,$20,$3D,$20,$73,$61,$76,$65,$2C,$20,$45,$73,$63,$20,$3D,$20,$64,$69,$73,$63,$61,$72,$64
