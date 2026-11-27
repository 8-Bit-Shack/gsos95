; ================================================================
; GS/OS95: Datum/Uhrzeit (Fensterart 27, Klick auf die Uhr), XCODE4
; Links Monatskalender mit Blaettern, rechts Zifferblatt, darunter
; Uhrzeit und Datum. "Aendern..." oeffnet die Einstellung (Art 15).
; ================================================================
CALKIND = 27
CAL_W   = 232
CAL_H   = 150
CAL_CX  = WX+186                ; Zifferblatt
CAL_CY  = WY+62
CAL_GX  = WX+8                  ; Tagesraster
CAL_GY  = WY+47
CAL_CW  = 18
CAL_LH  = 10

        .pushseg
        .segment "BSS"
CALY:   .res 2                  ; angezeigter Monat (Jahr, Monat 1..12)
CALM:   .res 2
CTY:    .res 2                  ; heute
CTM:    .res 2
CTD:    .res 2
CTH:    .res 2
CTMI:   .res 2
CALK:   .res 2
CALI:   .res 2
CALJ:   .res 2
CALT:   .res 2
CALF:   .res 2                  ; erster Tag: Spalte
CALN:   .res 2                  ; Tage im Monat
CALB:   .res 100                ; Textpuffer (Laenge + Zeichen)
CBT:    .res 2
CLX0:   .res 2                  ; Linie
CLY0:   .res 2
CLX1:   .res 2
CLY1:   .res 2
CLDX:   .res 2
CLDY:   .res 2
CLSX:   .res 2
CLSY:   .res 2
CLER:   .res 2
CLLEN:  .res 2
        .popseg

; ---------- Uhr der Taskleiste angeklickt ----------
xf_cal_open:
        jsr cal_now
        lda CTY
        sta CALY
        lda CTM
        sta CALM
        jsr cal_find            ; schon offen: nach vorn
        bcc @new
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        rtl
@new:   lda #CALKIND
        CALL win_open
        bcs @r
        lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda #(320-CAL_W)/2
        sta (PB),y
        ldy #R_Y
        lda #18
        sta (PB),y
        ldy #R_W
        lda #CAL_W
        sta (PB),y
        ldy #R_H
        lda #CAL_H
        sta (PB),y
        lda NWIN
        dec a
        CALL win_load
@r:     CALL redraw_all
        rtl

cal_find:
        stz CALK
@l:     lda CALK
        cmp NWIN
        bcs @no
        CALL recaddr
        sta PB
        lda (PB)
        cmp #CALKIND
        beq @y
        inc CALK
        bra @l
@y:     lda CALK
        sec
        rts
@no:    clc
        rts

; jede Minute (poll_frame): Kalender oben -> neu zeichnen
xf_cal_tick:
        lda NWIN
        beq @r
        dec a
        CALL recaddr
        sta PB
        lda (PB)
        cmp #CALKIND
        bne @r
        lda NDAWAS
        bne @r
        CALL redraw_win
@r:     rtl

; Uhrzeit lesen
cal_now:
        pha
        pha
        pha
        pha
        TOOL $0D03              ; ReadTimeHex
        pla                     ; Sekunde, Minute
        xba
        and #$00FF
        sta CTMI
        pla                     ; Stunde, Jahr
        pha
        and #$00FF
        sta CTH
        pla
        xba
        and #$00FF
        clc
        adc #1900
        sta CTY
        pla                     ; Tag, Monat (ab 0)
        pha
        and #$00FF
        inc a
        sta CTD
        pla
        xba
        and #$00FF
        inc a
        sta CTM
        pla
        rts

; Wochentag (0 = Sonntag) von CALY/CALM/Tag A  (Sakamoto)
cal_dow:
        sta CALT
        lda CALM
        dec a
        tax
        TBL cal_tt
        and #$00FF
        clc
        adc CALT
        sta CALT
        lda CALY
        ldx CALM
        cpx #3
        bcs @y
        dec a
@y:     sta CALK
        clc
        adc CALT
        sta CALT
        lda CALK
        lsr
        lsr
        clc
        adc CALT
        sta CALT
        lda CALK
        ldx #100
        CALL gdiv
        sta CALI
        lda CALT
        sec
        sbc CALI
        sta CALT
        lda CALI
        lsr
        lsr
        clc
        adc CALT
        ldx #7
        CALL gdiv
        lda GMR
        rts
cal_tt: .byte 0,3,2,5,0,3,5,1,4,6,2,4
cal_md: .byte 31,28,31,30,31,30,31,31,30,31,30,31

; Tage im Monat CALM/CALY -> A
cal_days:
        lda CALM
        dec a
        tax
        TBL cal_md
        and #$00FF
        ldx CALM
        cpx #2
        bne @r
        pha
        lda CALY
        and #3
        bne @n
        lda CALY
        ldx #100
        CALL gdiv
        lda GMR
        bne @l
        lda CALY
        ldx #400
        CALL gdiv
        lda GMR
        bne @n
@l:     pla
        inc a
        rts
@n:     pla
@r:     rts

; ---------- Textpuffer ----------
cb_clr: sep #$20
        stz CALB
        rep #$20
        rts
cb_ch:  phx
        sta CBT
        lda CALB
        and #$00FF
        cmp #96
        bcs @f
        inc a
        tax
        sep #$20
        sta CALB
        lda CBT
        sta CALB,x
        rep #$20
@f:     plx
        rts
; Text A (diese Bank, Laenge + Zeichen) anhaengen
cb_s:   jsr x4_kb
        lda [LP2]
        and #$00FF
        sta CALK
        ldy #1
@l:     lda CALK
        beq @r
        lda [LP2],y
        jsr cb_ch
        iny
        dec CALK
        bra @l
@r:     rts
; Text A (Deutsch) / X (Englisch)
cb_sl:  ldy LANG
        beq @d
        txa
@d:     jmp cb_s
; Zahl A dezimal (bei X=2 mit fuehrender Null)
cb_num: stx CALJ
        ldx #0
@d:     phx
        ldx #10
        CALL gdiv
        plx
        lda GMR
        pha
        inx
        lda GMC
        bne @d
        cpx CALJ
        bcs @o
        lda #0
        pha
        inx
@o:     pla
        clc
        adc #'0'
        jsr cb_ch
        dex
        bne @o
        rts
; Puffer bei SX/SY ausgeben
cb_draw:
        lda #CALB
        sta PDST
        lda MYBANK
        sta PDST+2
        JSLF xf_ui_s2
        rts
; Breite des Puffers -> A
cb_wid: lda #CALB
        sta PDST
        lda MYBANK
        sta PDST+2
        JSLF xf_ui_w2
        rts
; Puffer zentriert um X-Mitte A
cb_ctr: pha
        jsr cb_wid
        lsr
        eor #$FFFF
        sec
        adc 1,s
        sta SX
        pla
        jmp cb_draw

; Monatsname (CALM) anhaengen
cb_month:
        lda CALM
        dec a
        asl
        tax
        lda LANG
        beq @d
        txa
        clc
        adc #24
        tax
@d:     TBL cal_mn
        jmp cb_s

; ---------- Zeichnen ----------
xf_cal_draw:
        jsr cal_now
        lda #WX+2               ; Hintergrund
        sta RX
        lda #WY+15
        sta RY
        lda #CAL_W-4
        sta RW
        lda #CAL_H-17
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        ; Kopf: < Monat Jahr >
        lda #WX+6
        ldx #cal_tl
        jsr cal_sbtn
        lda #WX+123
        ldx #cal_tr
        jsr cal_sbtn
        jsr cb_clr
        jsr cb_month
        lda #' '
        jsr cb_ch
        lda CALY
        ldx #0
        jsr cb_num
        lda #BLACK
        CALL setcolor
        lda #WY+21
        sta SY
        lda #WX+71
        jsr cb_ctr
        ; Raster
        lda #WX+6
        sta RX
        lda #WY+34
        sta RY
        lda #130
        sta RW
        lda #77
        sta RH
        jsr cal_sunk
        stz CALI                ; Wochentage
@wd:    lda CALI
        asl
        tax
        lda LANG
        beq @wd1
        txa
        clc
        adc #14
        tax
@wd1:   TBL cal_wn
        pha
        jsr cb_clr
        pla
        jsr cb_s
        lda #DKGRAY
        CALL setcolor
        lda #WY+36
        sta SY
        lda CALI
        ldx #CAL_CW
        CALL gmul
        clc
        adc #CAL_GX+CAL_CW/2
        jsr cb_ctr
        inc CALI
        lda CALI
        cmp #7
        bcc @wd
        lda #WX+8               ; Linie unter den Wochentagen
        sta RX
        lda #WY+45
        sta RY
        lda #126
        sta RW
        lda #1
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #1                  ; erster Tag: Spalte (Montag zuerst, englisch Sonntag)
        jsr cal_dow
        ldx LANG
        bne @f
        clc
        adc #6
        ldx #7
        CALL gdiv
        lda GMR
@f:     sta CALF
        jsr cal_days
        sta CALN
        lda #1
        sta CALI
@dl:    lda CALI
        clc
        adc CALF
        dec a
        ldx #7
        CALL gdiv
        sta CALJ                ; Zeile
        lda GMR
        ldx #CAL_CW
        CALL gmul
        clc
        adc #CAL_GX
        sta CALK                ; Spalte (x)
        lda CALJ
        ldx #CAL_LH
        CALL gmul
        clc
        adc #CAL_GY
        sta SY
        jsr cal_today           ; heute: blau hinterlegt
        bcc @nt
        lda CALK
        clc
        adc #2
        sta RX
        lda SY
        dec a
        sta RY
        lda #CAL_CW-3
        sta RW
        lda #CAL_LH
        sta RH
        lda #NAVY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        bra @dc
@nt:    lda #BLACK
@dc:    CALL setcolor
        jsr cb_clr
        lda CALI
        ldx #0
        jsr cb_num
        lda CALK
        clc
        adc #CAL_CW/2
        jsr cb_ctr
        inc CALI
        lda CALI
        cmp CALN
        bcs *+5
        jmp @dl
        bne *+5
        jmp @dl
        ; Zifferblatt
        jsr cal_face
        ; Uhrzeit digital
        jsr cb_clr
        lda CTH
        ldx #2
        jsr cb_num
        lda #':'
        jsr cb_ch
        lda CTMI
        ldx #2
        jsr cb_num
        lda #BLACK
        CALL setcolor
        lda #WY+98
        sta SY
        lda #CAL_CX
        jsr cb_ctr
        ; Datum ausgeschrieben
        jsr cb_clr
        lda CALY                ; Wochentag von heute
        pha
        lda CALM
        pha
        lda CTY
        sta CALY
        lda CTM
        sta CALM
        lda CTD
        jsr cal_dow
        asl
        tax
        lda LANG
        beq @dn
        txa
        clc
        adc #14
        tax
@dn:    TBL cal_dl
        jsr cb_s
        lda #','
        jsr cb_ch
        lda #' '
        jsr cb_ch
        lda LANG
        bne @en
        lda CTD
        ldx #0
        jsr cb_num
        lda #'.'
        jsr cb_ch
        lda #' '
        jsr cb_ch
        jsr cb_month
        bra @yr
@en:    jsr cb_month
        lda #' '
        jsr cb_ch
        lda CTD
        ldx #0
        jsr cb_num
        lda #','
        jsr cb_ch
@yr:    lda #' '
        jsr cb_ch
        lda CTY
        ldx #0
        jsr cb_num
        pla
        sta CALM
        pla
        sta CALY
        lda #BLACK
        CALL setcolor
        lda #WX+8
        sta SX
        lda #WY+116
        sta SY
        jsr cb_draw
        ; Knoepfe
        lda #0
        jsr cal_btn
        lda #1
        jmp cal_btn

; heute im angezeigten Monat? (Tag CALI) C=1
cal_today:
        lda CALI
        cmp CTD
        bne @n
        lda CALM
        cmp CTM
        bne @n
        lda CALY
        cmp CTY
        bne @n
        sec
        rts
@n:     clc
        rts

; vertieftes weisses Feld RX/RY/RW/RH
cal_sunk:
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        rts

; kleiner Knopf bei X=A, Y=WY+19 mit Text X (Laenge + Zeichen)
cal_sbtn:
        sta RX
        phx
        lda #WY+19
        sta RY
        lda #13
        sta RW
        lda #12
        sta RH
        lda #0
        jsr cal_bev
        jsr cb_clr
        pla
        jsr cb_s
        lda #BLACK
        CALL setcolor
        lda #WY+21
        sta SY
        lda RX
        clc
        adc #6
        jmp cb_ctr

; Knopf A (0 Aendern, 1 OK)
cal_brect:
        asl
        tax
        TBL cal_bx
        sta RX
        lda #WY+130
        sta RY
        TBL cal_bw
        sta RW
        lda #14
        sta RH
        rts
cal_bx: .word WX+112, WX+172
cal_bw: .word 56, 52
cal_btn:
        pha
        jsr cal_brect
        lda #0
        jsr cal_bev
        pla
        bne @ok
        jsr cb_clr
        lda #cal_tch
        ldx #cal_tch_en
        jsr cb_sl
        bra @t
@ok:    jsr cb_clr
        lda #cal_tok
        jsr cb_s
@t:     lda #BLACK
        CALL setcolor
        lda RY
        clc
        adc #3
        sta SY
        lda RW
        lsr
        clc
        adc RX
        jmp cb_ctr
; Knopf zeichnen (A=1 gedrueckt)
cal_bev:
        pha
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

; ---------- Zifferblatt ----------
cal_face:
        lda #DKGRAY             ; Rand
        jsr cal_ell
        lda #WHITE
        jsr cal_ell
        ; Stundenstriche (alle 5 Minuten), Viertel dicker
        stz CALI
@m:     lda CALI
        ldx #31
        jsr cal_pt
        lda #BLACK
        CALL setcolor
        lda CLX1
        sta RX
        lda CLY1
        sta RY
        lda #2
        sta RW
        sta RH
        lda CALI
        ldx #15
        CALL gdiv
        lda GMR
        bne @s
        lda #3
        sta RW
        sta RH
        dec RX
        dec RY
@s:     CALL fillrect
        lda CALI
        clc
        adc #5
        sta CALI
        cmp #60
        bcc @m
        ; Stundenzeiger
        lda CTH
        ldx #12
        CALL gdiv
        lda GMR
        ldx #5
        CALL gmul
        sta CALT
        lda CTMI
        ldx #12
        CALL gdiv
        clc
        adc CALT
        ldx #18
        jsr cal_pt
        jsr cal_line
        inc CLX0
        inc CLX1
        jsr cal_line
        dec CLY0
        dec CLY1
        jsr cal_line
        ; Minutenzeiger
        lda CTMI
        ldx #27
        jsr cal_pt
        jsr cal_line
        inc CLX0
        inc CLX1
        jsr cal_line
        lda #CAL_CX-1           ; Mitte
        sta RX
        lda #CAL_CY-1
        sta RY
        lda #3
        sta RW
        sta RH
        lda #BLACK
        CALL setcolor
        CALL fillrect
        rts

; Ellipse fuellen, Farbe A; Rand: DKGRAY gross, WHITE eins kleiner
cal_ell:
        CALL setcolor
        stz CALJ
        cmp #WHITE
        bne @g
        inc CALJ
@g:     stz CALI                ; Zeile -ry..ry
@l:     lda CALI
        tax
        TBL cal_hw
        and #$00FF
        sec
        sbc CALJ
        bmi @nx
        beq @nx
        sta CALK
        asl
        sta RW
        lda #CAL_CX
        sec
        sbc CALK
        sta RX
        lda #1
        sta RH
        lda #CAL_CY
        clc
        adc CALI
        sta RY
        CALL fillrect
        lda #CAL_CY
        sec
        sbc CALI
        sta RY
        CALL fillrect
@nx:    inc CALI
        lda CALI
        cmp #31
        bcc @l
        rts
cal_hw: .byte 36,36,36,36,36,35,35,35,35,34,34,33,33,32,32,31,30,30,29,28,27,26,24,23,22,20,18,16,13,9,0

; Punkt fuer Minute A (0..59) mit Laenge X (in Punkten des Halbmessers) -> CLX1/CLY1,
; CLX0/CLY0 = Mitte
cal_pt: stx CLLEN
        pha
        tax
        TBL cal_sn
        jsr cal_scl
        clc
        adc #CAL_CX
        sta CLX1
        plx
        TBL cal_cs
        jsr cal_scl
        tax                     ; senkrecht: 30/36
        bpl @p
        eor #$FFFF
        inc a
        ldx #30
        CALL gmul
        ldx #36
        CALL gdiv
        eor #$FFFF
        inc a
        bra @y
@p:     ldx #30
        CALL gmul
        ldx #36
        CALL gdiv
@y:     clc
        adc #CAL_CY
        sta CLY1
        lda #CAL_CX
        sta CLX0
        lda #CAL_CY
        sta CLY0
        rts
; Byte A (mit Vorzeichen, -127..127) * CLLEN / 127
cal_scl:
        and #$00FF
        cmp #$80
        bcc @p
        eor #$00FF
        inc a
        ldx CLLEN
        CALL gmul
        ldx #127
        CALL gdiv
        eor #$FFFF
        inc a
        rts
@p:     ldx CLLEN
        CALL gmul
        ldx #127
        CALL gdiv
        rts
cal_sn: .byte 0,13,26,39,52,63,75,85,94,103,110,116,121,124,126,127,126,124,121,116,110,103,94,85,75,63,52,39,26,13
        .byte 0,256-13,256-26,256-39,256-52,256-63,256-75,256-85,256-94,256-103,256-110,256-116,256-121,256-124,256-126
        .byte 256-127,256-126,256-124,256-121,256-116,256-110,256-103,256-94,256-85,256-75,256-63,256-52,256-39,256-26,256-13
cal_cs: .byte 256-127,256-126,256-124,256-121,256-116,256-110,256-103,256-94,256-85,256-75,256-64,256-52,256-39,256-26,256-13
        .byte 0,13,26,39,52,63,75,85,94,103,110,116,121,124,126,127,126,124,121,116,110,103,94,85,75,64,52,39,26,13
        .byte 0,256-13,256-26,256-39,256-52,256-64,256-75,256-85,256-94,256-103,256-110,256-116,256-121,256-124,256-126

; Linie CLX0/CLY0 -> CLX1/CLY1 schwarz (Bresenham)
cal_line:
        lda #BLACK
        CALL setcolor
        lda CLX0
        sta CALI
        lda CLY0
        sta CALJ
        ldx #1
        lda CLX1
        sec
        sbc CLX0
        bpl @x
        eor #$FFFF
        inc a
        ldx #$FFFF
@x:     sta CLDX
        stx CLSX
        ldx #1
        lda CLY1
        sec
        sbc CLY0
        bpl @y
        eor #$FFFF
        inc a
        ldx #$FFFF
@y:     eor #$FFFF              ; dy negativ
        inc a
        sta CLDY
        stx CLSY
        lda CLDX
        clc
        adc CLDY
        sta CLER
@l:     ldx CALI
        ldy CALJ
        CALL setpixel
        lda CALI
        cmp CLX1
        bne @c
        lda CALJ
        cmp CLY1
        beq @r
@c:     lda CLER
        asl
        sta CALT
        sec
        sbc CLDY
        bmi @ny                 ; e2 >= dy
        lda CLER
        clc
        adc CLDY
        sta CLER
        lda CALI
        clc
        adc CLSX
        sta CALI
@ny:    lda CLDX
        sec
        sbc CALT
        bmi @l                  ; e2 <= dx
        lda CLER
        clc
        adc CLDX
        sta CLER
        lda CALJ
        clc
        adc CLSY
        sta CALJ
        bra @l
@r:     rts

; ---------- Klick ----------
xf_cal_click:
        CALL lmx
        sec
        sbc #WX
        sta CALI
        CALL lmy
        sec
        sbc #WY
        sta CALJ
        lda CALJ                ; Monat blaettern
        cmp #19
        bcc @b
        cmp #31
        bcs @b
        lda CALI
        sec
        sbc #6
        cmp #13
        bcc @prev
        lda CALI
        sec
        sbc #123
        cmp #13
        bcc @next
        rtl
@b:     lda #0
@bl:    pha
        jsr cal_brect
        CALL inrect
        bcs @hit
        pla
        inc a
        cmp #2
        bcc @bl
        rtl
@hit:   lda #1                  ; kurz gedrueckt
        jsr cal_bev
        pla
        bne cal_close
        jmp cal_change
@prev:  jsr cal_mprev
        CALL redraw_win
        rtl
@next:  jsr cal_mnext
        CALL redraw_win
        rtl
cal_close:
        CALL close_win
        rtl
cal_change:                     ; Einstellung Datum/Uhrzeit (wie Klick auf die Uhr vorher)
        CALL close_win
        lda #15
        JSLF xf_sdlg_init
        lda #15
        CALL win_open
        CALL redraw_all
        rtl
cal_mprev:
        dec CALM
        bne @r
        lda #12
        sta CALM
        dec CALY
@r:     rts
cal_mnext:
        inc CALM
        lda CALM
        cmp #13
        bcc @r
        lda #1
        sta CALM
        inc CALY
@r:     rts

; ---------- Tasten: Pfeile blaettern, Bild hoch/runter Jahr, Return/Esc schliessen ----------
xf_cal_key:
        lda KEYC
        cmp #$08
        bne @k1
        jsr cal_mprev
        bra @rd
@k1:    cmp #$15
        bne @k2
        jsr cal_mnext
        bra @rd
@k2:    cmp #$0B
        bne @k3
        dec CALY
        bra @rd
@k3:    cmp #$0A
        bne @k4
        inc CALY
        bra @rd
@k4:    cmp #$1B
        beq cal_close
        cmp #$0D
        beq cal_close
        cmp #'h'                ; H: heute
        beq @hy
        cmp #'H'
        beq @hy
        cmp #'t'
        beq @hy
        cmp #'T'
        bne @r
@hy:    jsr cal_now
        lda CTY
        sta CALY
        lda CTM
        sta CALM
@rd:    CALL redraw_win
@r:     rtl

; ---------- Texte ----------
cal_tl:  .byte 1,"<"
cal_tr:  .byte 1,">"
cal_tok: .byte 2,"OK"
cal_tch: .byte 9,$83,"ndern..."
cal_tch_en: .byte 9,"Change..."
cal_mn: .addr cal_m1,cal_m2,cal_m3,cal_m4,cal_m5,cal_m6,cal_m7,cal_m8,cal_m9,cal_m10,cal_m11,cal_m12
        .addr cal_e1,cal_e2,cal_e3,cal_e4,cal_e5,cal_e6,cal_e7,cal_e8,cal_e9,cal_e10,cal_e11,cal_e12
cal_m1:  .byte 6,"Januar"
cal_m2:  .byte 7,"Februar"
cal_m3:  .byte 4,"M",$80,"rz"
cal_m4:  .byte 5,"April"
cal_m5:  .byte 3,"Mai"
cal_m6:  .byte 4,"Juni"
cal_m7:  .byte 4,"Juli"
cal_m8:  .byte 6,"August"
cal_m9:  .byte 9,"September"
cal_m10: .byte 7,"Oktober"
cal_m11: .byte 8,"November"
cal_m12: .byte 8,"Dezember"
cal_e1:  .byte 7,"January"
cal_e2:  .byte 8,"February"
cal_e3:  .byte 5,"March"
cal_e4:  .byte 5,"April"
cal_e5:  .byte 3,"May"
cal_e6:  .byte 4,"June"
cal_e7:  .byte 4,"July"
cal_e8:  .byte 6,"August"
cal_e9:  .byte 9,"September"
cal_e10: .byte 7,"October"
cal_e11: .byte 8,"November"
cal_e12: .byte 8,"December"
; Spaltenkoepfe: deutsch ab Montag, englisch ab Sonntag
cal_wn: .addr cal_w1,cal_w2,cal_w3,cal_w4,cal_w5,cal_w6,cal_w0
        .addr cal_v0,cal_v1,cal_v2,cal_v3,cal_v4,cal_v5,cal_v6
cal_w0: .byte 2,"So"
cal_w1: .byte 2,"Mo"
cal_w2: .byte 2,"Di"
cal_w3: .byte 2,"Mi"
cal_w4: .byte 2,"Do"
cal_w5: .byte 2,"Fr"
cal_w6: .byte 2,"Sa"
cal_v0: .byte 1,"S"
cal_v1: .byte 1,"M"
cal_v2: .byte 1,"T"
cal_v3: .byte 1,"W"
cal_v4: .byte 1,"T"
cal_v5: .byte 1,"F"
cal_v6: .byte 1,"S"
; Wochentage ausgeschrieben (0 = Sonntag)
cal_dl: .addr cal_d0,cal_d1,cal_d2,cal_d3,cal_d4,cal_d5,cal_d6
        .addr cal_f0,cal_f1,cal_f2,cal_f3,cal_f4,cal_f5,cal_f6
cal_d0: .byte 7,"Sonntag"
cal_d1: .byte 6,"Montag"
cal_d2: .byte 8,"Dienstag"
cal_d3: .byte 8,"Mittwoch"
cal_d4: .byte 10,"Donnerstag"
cal_d5: .byte 7,"Freitag"
cal_d6: .byte 7,"Samstag"
cal_f0: .byte 6,"Sunday"
cal_f1: .byte 6,"Monday"
cal_f2: .byte 7,"Tuesday"
cal_f3: .byte 9,"Wednesday"
cal_f4: .byte 8,"Thursday"
cal_f5: .byte 6,"Friday"
cal_f6: .byte 8,"Saturday"
