; ================================================================
; GS/OS95 Browser: Spaltenbreiten fuer Tabellen (Teil von web.s)
; Textstrom: $1C t (Tabelle t beginnt), $1D t (endet), $1E (neue Zeile),
; $17 s b (Zelle in Spalte s; b = Breite bis dorthin, traegt lay_all ein).
; tab_meas misst vorab je Tabelle die breiteste Zeile jeder Spalte (bis 8
; Spalten, 16 Tabellen); col_w richtet die Zellen danach aus. Ist eine
; Tabelle breiter als das Fenster, gelten Tabstopps alle 40 Punkte.
; ================================================================
TMAXT   = 16
TMAXC   = 8
TCAP    = 140                   ; breiteste Spalte, die zaehlt
TGAP    = 10                    ; Abstand zwischen Spalten

        .pushseg
        .segment "BSS"
TCW:    .res TMAXT*TMAXC*2      ; Spaltenbreiten
TCUR:   .res 2                  ; aktuelle Tabelle ($FF = keine)
TMC:    .res 2                  ; Messen: Tabelle, Spalte, Breite, fett
TMS:    .res 2
TMW:    .res 2
TMB:    .res 2
TMQ:    .res 2
TMX:    .res 2
TMT:    .res 2
HTDEP:  .res 2                  ; h2t: Tabellentiefe, Zaehler, Nummer der aeusseren
HTBL:   .res 2
HTOUT:  .res 2
        .popseg

; ---------- Durchgang 1: messen ----------
tab_meas:
        ldx #TMAXT*TMAXC*2-2
@z:     stz TCW,x
        dex
        dex
        bpl @z
        lda #$FF
        sta TMC
        stz TMS
        stz TMW
        stz TMB
        lda #WB_TXT
        sta TMQ
tm_loop:
        ldy TMQ
        cpy TXTEND
        bcc @g
        rts
@g:     BRD
        cmp #$20
        bcc @ctl
        jsr cw
        clc
        adc TMW
        adc TMB
        sta TMW
        inc TMQ
        bra tm_loop
@ctl:   asl
        tax
        jmp (tm_tab,x)
tm_1:   inc TMQ                 ; 1 Byte ueberspringen
        bra tm_loop
tm_2:   inc TMQ
        inc TMQ
        bra tm_loop
tm_lnk: iny                     ; $01 n Ziel
        BRD
        clc
        adc #2
        adc TMQ
        sta TMQ
        bra tm_loop
tm_b0:  stz TMB
        bra tm_1
tm_b1:  lda #1
        sta TMB
        bra tm_1
tm_gap: lda TMW
        clc
        adc #8
        sta TMW
        bra tm_1
tm_fld: lsr                     ; $18/$19 f
        tax
        iny
        BRD
        jsr fld_w
        clc
        adc TMW
        sta TMW
        bra tm_2
tm_nl:  jsr tm_fin              ; Zeilenwechsel in einer Zelle
        bra tm_1
tm_row: jsr tm_fin
        stz TMS
        bra tm_1
tm_col: jsr tm_fin              ; $17 s b
        iny
        BRD
        sta TMS
        inc TMQ
        bra tm_2
tm_tb:  jsr tm_fin              ; $1C t
        iny
        BRD
        sta TMC
        stz TMS
        bra tm_2
tm_te:  jsr tm_fin              ; $1D t
        iny
        BRD
        cmp TMC
        beq *+5
        jmp tm_2
        lda #$FF
        sta TMC
        jmp tm_2
; Zelle beenden: Breite merken (hoechstens TCAP)
tm_fin: lda TMW
        stz TMW
        sta TMX
        lda TMC
        cmp #TMAXT
        bcs @r
        lda TMS
        cmp #TMAXC
        bcs @r
        jsr tc_addr
        lda TMX
        cmp #TCAP
        bcc @c
        lda #TCAP
@c:     cmp TCW,x
        bcc @r
        sta TCW,x
@r:     rts
; X = Adresse in TCW fuer Tabelle TMC/TCUR (A = Spalte)
tc_addr:
        asl
        sta TMT
        lda TMC
        asl
        asl
        asl
        asl                     ; *16 = 8 Spalten * 2
        clc
        adc TMT
        tax
        rts

; Sprungtabelle muss im Codesegment liegen (jmp (abs,x) liest aus der Programmbank)
tm_tab: .word tm_1, tm_lnk, tm_1, tm_1, tm_1, tm_1, tm_1, tm_1          ; 00-07
        .word tm_1, tm_1, tm_nl, tm_1, tm_1, tm_1, tm_1, tm_1           ; 08-0F
        .word tm_b0, tm_b1, tm_1, tm_1, tm_2, tm_gap, tm_1, tm_col      ; 10-17
        .word tm_fld, tm_fld, tm_2, tm_1, tm_tb, tm_te, tm_row, tm_1    ; 18-1F

; ---------- Durchgang 2: Breite fuer Spalte A ab WLX ----------
col_w:  sta TMS
        lda TCUR
        cmp #TMAXT
        bcs @gap
        lda TMS
        cmp #TMAXC
        bcs @gap
        lda TCUR                ; Anfang der Spalte = Summe der vorigen
        sta TMC
        stz TMW
        ldy #0
@s:     cpy TMS
        bcs @have
        tya
        jsr tc_addr
        lda TCW,x
        clc
        adc #TGAP
        clc
        adc TMW
        sta TMW
        iny
        bra @s
@have:  ldx #TMAXC-1            ; passt die ganze Tabelle? sonst Tabstopps            ; alle Spalten
        stz TMB
@t:     phx
        txa
        jsr tc_addr
        lda TCW,x
        plx
        beq @t2
        clc
        adc #TGAP
        adc TMB
        sta TMB
@t2:    dex
        bpl @t
        lda LMG
        jsr mg_w
        sta TMX
        clc
        adc TMB
        cmp WRAPW
        beq @fit
        bcs @tab
@fit:   lda TMX                 ; Ziel = Rand + Spaltenanfang
        clc
        adc TMW
        sec
        sbc WLX
        bcc @min
        cmp #6
        bcs @r
@min:   lda #6
@r:     rts
@tab:   jmp tab_w
@gap:   lda #8
        rts
