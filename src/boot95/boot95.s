; GS/OS95-Bootbild: ersetzt in START.GS.OS (System 6.1) den Bildschirmaufbau
; (Aufruf ueber JSR bei $740E). Laeuft in Bank 0 ab $7919 (statt Startlogo-Daten).
.setcpu "65816"
MAGIC = $3539                   ; "95" in $E1/9DFE (SCB-Bereich, ungenutzt)
.smart off
.a16
.i16
; Variablen im nicht mehr benutzten Code (6.1: Bildschirm-Aufbau ab $7686)
CNT = $7686
POFF = $7688
ROW = $768A
COLV = $768C
SPP = $768E
SXV = $7690
SYV = $7692
CLO = $7694
CHI = $7696
W = $7698
H = $769A
BPR = $769C
DPT = $769E
RR = $76A0
T = $76A2
II = $76A4
BB = $76A6
SCL = $76A8
YY = $76AA
SY2 = $76AC
XX = $76AE
        .segment "CODE"
start:  jmp main
; Ladebalken: zwei Schritte pro geladener Datei (Balken ist breiter als im Original)
step2:  jsr $7572
        jsr $7572
        jmp curve
main:   php
        phb
        phk
        plb
        rep #$30
        lda f:$E19DFE           ; Bild schon da (SYSTEM:PHOENIX beim Laden von PRODOS)?
        cmp #MAGIC
        bne @draw
        plb
        plp
        rts
@draw:
        ; ---- Paletten: 0 = Balken (640er-Modus), 1..14 = Flaeche + Schrift,
        ; 15 = nur Flaeche (unten: Symbole der Systemerweiterungen unsichtbar).
        ; Das Bild selbst (Paletten 1..13 und Zeilen 0..165) setzt danach
        ; SPLASH95 aus *:System:GSOS95.SPL ein ----
        ldx #30
@p0:    lda d_pal0,x
        sta f:$E19E00,x
        lda d_p14,x
        sta f:$E19E00+14*32,x
        lda #BANDC
        sta f:$E19E00+15*32,x
        dex
        dex
        bpl @p0
        ldx #13*32-2
@pn:    txa                     ; Palette 14 auch nach 1..13

        and #$001F
        tay
        lda d_p14,y
        sta f:$E19E00+32,x
        dex
        dex
        bpl @pn
        ; ---- Zeilen: SCB und Fuellung ----
        ldx #0
        stz ROW
@fr:    lda ROW
        cmp #BAR0
        bcc @g
        cmp #BAR1+1
        bcs @u
        lda #$FFFF              ; Balkenzeilen: 640er-Modus, Wert 3 = Flaeche
        sta COLV
        lda #$0080
        bra @s
@u:     stz COLV                ; unten: Palette 15
        lda #15
        bra @s
@g:     stz COLV                ; Flaeche: Palette 14, Farbe 0
        lda #14
@s:     phx
        ldx ROW
        sep #$20
        .a8
        sta f:$E19D00,x
        rep #$20
        .a16
        plx
        lda COLV
        ldy #80
@fw:    sta f:$E12000,x
        inx
        inx
        dey
        bne @fw
        inc ROW
        lda ROW
        cmp #200
        bcc @fr
        ; ---- Pixel-Schriftzug GS/OS 95 (weisser Schatten, dunkelblau) ----
        lda #4
        sta SCL
        lda #2
        sta COLV
        lda #d_logo
        ldx #LOGO_X+3
        ldy #LOGO_Y+3
        jsr blit
        lda #1
        sta COLV
        lda #d_logo
        ldx #LOGO_X
        ldy #LOGO_Y
        jsr blit
        ; ---- "Created by ..." und Build wie im Startbild ----
        lda #1
        sta SCL
        sta COLV
        lda #d_by
        ldx #BY_X
        ldy #BY_Y
        jsr blit
        lda #d_build
        ldx #BUILD_X
        ldy #BUILD_Y
        jsr blit
fin:    lda #$00E1
        sta $02
        plb
        plp
        rts

; 1-Bit-Sprite A an X,Y in Farbe COLV
blit:   sta SPP
        stx SXV
        sty SYV
        lda COLV
        sta CLO
        asl
        asl
        asl
        asl
        sta CHI
        ldx SPP
        lda a:0,x
        and #$00FF
        sta W
        lda a:1,x
        and #$00FF
        sta H
        lda W
        clc
        adc #7
        lsr
        lsr
        lsr
        sta BPR
        inx
        inx
        stx DPT
        stz RR
@row:   lda RR
        ldx SCL
        jsr mul
        clc
        adc SYV
        sta YY
        stz SY2
@sub:   lda YY
        clc
        adc SY2
        asl
        asl
        asl
        asl
        asl
        sta T
        asl
        asl
        clc
        adc T
        clc
        adc #$2000
        sta $00
        lda #$00E1
        sta $02
        stz II
@col:   lda II
        lsr
        lsr
        lsr
        clc
        adc DPT
        tax
        lda a:0,x
        and #$00FF
        sta BB
        lda II
        and #7
        tax
        lda BB
@sh:    cpx #0
        beq @g
        asl
        dex
        bra @sh
@g:     and #$0080
        beq @nx
        lda II
        ldx SCL
        jsr mul
        clc
        adc SXV
        sta XX
        ldx SCL
@px:    phx
        lda XX
        lsr
        tay
        sep #$20
        .a8
        bcs @odd
        lda [$00],y
        and #$0F
        ora CHI
        sta [$00],y
        bra @d
@odd:   lda [$00],y
        and #$F0
        ora CLO
        sta [$00],y
@d:     rep #$20
        .a16
        inc XX
        plx
        dex
        bne @px
@nx:    inc II
        lda II
        cmp W
        bcs *+5
        jmp @col
        inc SY2
        lda SY2
        cmp SCL
        bcs *+5
        jmp @sub
        lda DPT
        clc
        adc BPR
        sta DPT
        inc RR
        lda RR
        cmp H
        bcs *+5
        jmp @row
        rts

; A * X -> A
mul:    sta T
        lda #0
@m:     cpx #0
        beq @r
        clc
        adc T
        dex
        bra @m
@r:     rts

        .include "data.inc"
endmark:
; Build-Zeile, Paletten, Text im nicht mehr benutzten Code (6.1: ab $76B0)
        .segment "DATA2"
        .include "data2.inc"

; ---- im nicht mehr benutzten Tabellenbereich des Originals ($78BC-$7904) ----
        .segment "DATA3"
; Bildschirm loeschen nur, wenn das Bild nicht schon da ist (Sprung aus $73C6)
clrchk: lda f:$E19DFE
        cmp #MAGIC
        bne @o
        jmp $7405
@o:     lda f:$0078BA
        jmp $73CA
; Ladebalken wird langsamer, je voller er ist (Abstand in VBL-Takten fuer den
; naechsten Schritt: 2 + p*p/256 + p/16, p = Position 0..100)
curve:  lda $782A
        tax
        jsr mul
        xba
        and #$00FF
        sta CRT
        lda $782A
        lsr
        lsr
        lsr
        lsr
        clc
        adc CRT
        adc #2
        sta $7826
        rts
CRT:    .word 0
