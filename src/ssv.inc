; ================================================================
; GS/OS95: Bildschirmschoner "Linienspiel" (wie Mystify bei Windows 95)
; XCODE4 (x4.s). Zwei Vierecke wandern ueber den Schirm und ziehen eine
; Spur aus 8 Kopien hinter sich her; die Farben wechseln langsam.
; idle_check (snd.s) wechselt zwischen Sternenfeld und Linienspiel ab.
; Eigene Palette 0, alle Zeilen Palette 0; danach Farben/SCBs zurueck.
; ================================================================
MY_N    = 8                     ; Spurlaenge
        .pushseg
        .segment "BSS"
MYP:    .res 2*16               ; 8 Ecken (2 Vierecke): x, y
MYD:    .res 2*16               ; Geschwindigkeit dx, dy
MYH:    .res 2*16*MY_N          ; Verlauf der Ecken (Ring)
MYC:    .res 2*MY_N             ; Farbe je Verlaufseintrag (2 Vierecke: Farbe, Farbe+4)
MYI:    .res 2                  ; Ringposition
MYCOL:  .res 2
MYCT:   .res 2
MYX0:   .res 2
MYY0:   .res 2
MYX1:   .res 2
MYY1:   .res 2
MYDX:   .res 2
MYDY:   .res 2
MYSX:   .res 2
MYSY:   .res 2
MYER:   .res 2
MYCB:   .res 2                  ; Farbbyte (Nibble doppelt)
MYCLR:  .res 2
MYHV:   .res 2
MYQP:   .res 2
MYE2:   .res 2
MYT:    .res 2
MYSV:   .res 4                  ; Maus beim Start
MYR:    .res 2                  ; Zufall
        .popseg

xf_ss_myst:
        CALL cur_hide
        ldx #$1E                ; eigene Palette 0
@pal:   TBL my_pal
        sta f:$E19E00,x
        dex
        dex
        bpl @pal
        ldx #198                ; alle Zeilen: 320er-Modus, Palette 0
@scb:   lda #0
        sta f:$E19D00,x
        dex
        dex
        bpl @scb
        ldx #0                  ; Schirm schwarz
        lda #0
@clr:   sta f:$E12000,x
        inx
        inx
        cpx #32000
        bcc @clr
        lda EVREC+12
        sta MYSV
        lda EVREC+10
        sta MYSV+2
        ; Startpunkte und Richtungen
        lda MYSV
        eor #$5A3C
        sta MYR
        ldx #30
@st:    jsr my_rnd
        and #$00FF
        clc
        adc #20
        sta MYP,x               ; (x bzw. y, y wird unten begrenzt)
        jsr my_rnd
        and #3
        inc a
        sta MYD,x
        jsr my_rnd
        and #1
        beq @pos
        lda MYD,x
        eor #$FFFF
        inc a
        sta MYD,x
@pos:   dex
        dex
        bpl @st
        ldx #30                 ; y-Werte unter 200
@sy:    lda MYP,x
        cmp #180
        bcc @sy2
        sbc #100
        sta MYP,x
@sy2:   dex
        dex
        dex
        dex
        bpl @sy
        ldx #2*16*MY_N-2        ; Verlauf leer (-1 = nichts zu loeschen)
        lda #$FFFF
@h:     sta MYH,x
        dex
        dex
        bpl @h
        stz MYI
        lda #1
        sta MYCOL
        stz MYCT
; ---------- Bild fuer Bild ----------
@loop:  CALL waitvbl
        ; aeltesten Eintrag (an MYI) schwarz zeichnen
        lda MYI
        jsr my_hptr             ; X = Verlauf MYI
        lda MYH,x
        bmi @noer
        lda #0
        jsr my_quads
@noer:  ; Ecken bewegen
        ldx #30
@mv:    lda MYP,x
        clc
        adc MYD,x
        sta MYP,x
        txa
        and #2                  ; 0 = x, 2 = y
        bne @my
        lda MYP,x
        bmi @bx
        cmp #319
        bcc @nx
@bx:    jsr @flip
        bra @nx
@my:    lda MYP,x
        bmi @by
        cmp #199
        bcc @nx
@by:    jsr @flip
@nx:    dex
        dex
        bpl @mv
        ; in den Verlauf an MYI, dann zeichnen
        lda MYI
        jsr my_hptr
        ldy #0
@cp:    lda MYP,y
        sta MYH,x
        inx
        inx
        iny
        iny
        cpy #32
        bcc @cp
        lda MYI
        asl
        tax
        lda MYCOL
        sta MYC,x
        lda MYI
        jsr my_hptr
        lda MYI
        asl
        tay
        lda MYC,y
        jsr my_quads
        lda MYI                 ; Ring weiter
        inc a
        and #MY_N-1
        sta MYI
        inc MYCT                ; Farbe ab und zu wechseln
        lda MYCT
        and #63
        bne @ev
        lda MYCOL
        inc a
        cmp #8
        bcc @c1
        lda #1
@c1:    sta MYCOL
        ; Ende bei Taste, Klick oder Mausbewegung
@ev:    pha
        pea $002A
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        bne @end
        lda EVREC+12
        cmp MYSV
        bne @end
        lda EVREC+10
        cmp MYSV+2
        bne @end
        jmp @loop
@end:   JSLF xf_set_palettes
        CALL desktop_scb
        CALL orgr
        CALL redraw_all
        CALL cur_show
        rtl
@flip:  lda MYD,x               ; Richtung umkehren, Punkt zurueck
        eor #$FFFF
        inc a
        sta MYD,x
        lda MYP,x
        clc
        adc MYD,x
        clc
        adc MYD,x
        sta MYP,x
        rts

; X = Verlauf-Eintrag A (32 Bytes je Eintrag)
my_hptr:
        asl
        asl
        asl
        asl
        asl
        tax
        rts

; beide Vierecke des Verlaufs X in Farbe A (0 = loeschen; sonst A und A+8)
my_quads:
        sta MYCLR
        stx MYHV
        lda MYCLR
        jsr my_setc
        lda MYHV
        jsr my_quad
        lda MYCLR
        beq @b
        clc
        adc #8
@b:     jsr my_setc
        lda MYHV
        clc
        adc #16
        jmp my_quad

my_setc:
        and #$000F
        sta MYCB
        asl
        asl
        asl
        asl
        ora MYCB
        sta MYCB
        rts

; Viereck: 4 Ecken ab MYH+A (x, y je 2 Bytes), ringsum verbunden
my_quad:
        sta MYQP
        ldy #0
@l:     tya
        asl
        asl
        clc
        adc MYQP
        tax
        lda MYH,x
        sta MYX0
        lda MYH+2,x
        sta MYY0
        tya
        inc a
        and #3
        asl
        asl
        clc
        adc MYQP
        tax
        lda MYH,x
        sta MYX1
        lda MYH+2,x
        sta MYY1
        phy
        jsr my_line
        ply
        iny
        cpy #4
        bcc @l
        rts

; Linie MYX0/MYY0 -> MYX1/MYY1 (Bresenham), Farbe MYCB
my_line:
        lda MYX1
        sec
        sbc MYX0
        ldx #1
        bcs @dx
        eor #$FFFF
        inc a
        ldx #$FFFF
@dx:    sta MYDX
        stx MYSX
        lda MYY1
        sec
        sbc MYY0
        ldx #1
        bcs @dy
        eor #$FFFF
        inc a
        ldx #$FFFF
@dy:    sta MYDY
        stx MYSY
        lda MYDX
        sec
        sbc MYDY
        sta MYER                ; err = dx - dy
@p:     jsr my_plot
        lda MYX0
        cmp MYX1
        bne @go
        lda MYY0
        cmp MYY1
        beq @r
@go:    lda MYER
        asl
        sta MYE2
        clc                     ; e2 > -dy ?
        adc MYDY
        bmi @y
        beq @y
        lda MYER
        sec
        sbc MYDY
        sta MYER
        lda MYX0
        clc
        adc MYSX
        sta MYX0
@y:     lda MYE2                ; e2 < dx ?
        sec
        sbc MYDX
        bpl @p
        lda MYER
        clc
        adc MYDX
        sta MYER
        lda MYY0
        clc
        adc MYSY
        sta MYY0
        bra @p
@r:     rts

; Punkt MYX0/MYY0 in Farbe MYCB (320er-Modus: gerades x = oberes Halbbyte)
my_plot:
        lda MYX0
        cmp #320
        bcs @r
        lda MYY0
        cmp #200
        bcs @r
        asl
        asl
        asl
        asl
        asl
        sta MYT                 ; y*32
        asl
        asl
        clc
        adc MYT                 ; y*160
        sta MYT
        lda MYX0
        lsr
        clc
        adc MYT
        tax
        sep #$20
        lda MYX0
        lsr a
        bcs @odd
        lda f:$E12000,x
        and #$0F
        sta MYT
        lda MYCB
        and #$F0
        ora MYT
        sta f:$E12000,x
        rep #$20
        rts
@odd:
        .a8                     ; (ca65 folgt sep/rep nur geradeaus)
        lda f:$E12000,x
        and #$F0
        sta MYT
        lda MYCB
        and #$0F
        ora MYT
        sta f:$E12000,x
        rep #$20
@r:     rts

my_rnd: lda MYR
        asl
        eor MYR
        sta MYR
        lsr
        lsr
        lsr
        eor MYR
        sta MYR
        xba
        asl
        eor MYR
        sta MYR
        rts

my_pal: .word $0000,$0F00,$0F80,$0FF0,$00F0,$00FF,$008F,$0F0F
        .word $0444,$0A00,$0A50,$0AA0,$00A0,$00AA,$005A,$0A0A
