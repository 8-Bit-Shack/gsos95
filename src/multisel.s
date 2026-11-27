        .segment "XCODE3"
FARMODE .set 3
xf_is_sel:
        jsr is_sel
        rtl
xf_msel_all:
        jsr msel_all
        rtl
xf_msel_clear:
        jsr msel_clear
        rtl
xf_msel_toggle:
        jsr msel_toggle
        rtl
xf_sel_copy:
        jsr sel_copy
        rtl
xf_sel_list:
        jsr sel_list
        rtl
; ================================================================
; GS/OS95: Mehrfachauswahl (Shift-/Apfel-Klick, Strg+A)
; Bitfeld msel gehoert zum Fenster MSELW. Ohne Mehrfachauswahl gilt SEL2.
; ================================================================
SELMAX  = 16                    ; so viele Eintraege je Vorgang

; A = Index -> C=1 wenn markiert
is_sel: sta MSI
        lda MSELN
        bne @m
        lda MSI
        cmp SEL2
        beq @y
@n:     clc
        rts
@y:     sec
        rts
@m:     lda CURWID
        cmp MSELW
        bne @n
        lda MSI
        jsr msel_bit
        and msel,x
        beq @n
        sec
        rts

; Index A -> X = Byte, A = Maske
msel_bit:
        pha
        lsr
        lsr
        lsr
        tax
        pla
        and #7
        tay
        lda bitmask,y
        and #$00FF
        rts
        .pushseg
        .segment "RODATA"
bitmask: .byte 1,2,4,8,16,32,64,128

        .popseg
msel_clear:
        stz MSELN
        ldx #0
@z:     stz msel,x
        inx
        inx
        cpx #MAXENT/8
        bcc @z
        rts

; Fenster-ID des obersten Fensters -> A
top_wid:
        lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_ID
        lda (PB),y
        rts

; Eintrag A umschalten (Shift-/Apfel-Klick)
msel_toggle:
        sta MSI2
        jsr top_wid
        cmp MSELW
        beq @same
        sta MSELW
        jsr msel_clear
@same:  lda MSELN
        bne @t
        lda SEL2                ; bisherige Einzelauswahl uebernehmen
        bmi @t
        cmp MSI2
        beq @t
        jsr msel_bit
        sep #$20
        ora msel,x
        sta msel,x
        rep #$20
        inc MSELN
@t:     lda MSI2
        jsr msel_bit
        sta MSI
        and msel,x
        beq @set
        lda MSI                 ; abwaehlen
        eor #$00FF
        sep #$20
        and msel,x
        sta msel,x
        rep #$20
        dec MSELN
        bra @e
@set:   lda MSI
        sep #$20
        ora msel,x
        sta msel,x
        rep #$20
        inc MSELN
@e:     lda MSI2
        sta SEL2
        rts

; Strg+A: alles markieren
msel_all:
        lda COUNT
        beq @r
        jsr top_wid
        sta MSELW
        jsr msel_clear
        stz MSI2
@l:     lda MSI2
        cmp COUNT
        bcs @d
        jsr msel_bit
        sep #$20
        ora msel,x
        sta msel,x
        rep #$20
        inc MSI2
        bra @l
@d:     lda COUNT
        sta MSELN
        stz SEL2
@r:     rts

; markierte Eintraege (max. SELMAX) nach sellist, Anzahl SLN
sel_list:
        stz SLN
        lda MSELN
        bne @m
        lda SEL2
        bmi @r
        sta sellist
        inc SLN
@r:     rts
@m:     stz MSI2
@l:     lda MSI2
        cmp COUNT
        bcs @r
        lda MSI2
        jsr is_sel
        bcc @nx
        lda SLN
        asl
        tax
        lda MSI2
        sta sellist,x
        inc SLN
@nx:    inc MSI2
        bra @l

; Eintraege aus sellist nach Puffer Y kopieren (je ENTSZ)
sel_copy:
        sty MSP
        lda SLN                 ; Puffer fassen hoechstens SELMAX
        cmp #SELMAX
        bcc @k
        lda #SELMAX
        sta SLN
@k:     stz MSI2
@l:     lda MSI2
        cmp SLN
        bcs @r
        asl
        tax
        lda sellist,x
        CALL entaddr
        tax
        ldy MSP
        CALL ec_em
        lda MSP
        clc
        adc #ENTSZ
        sta MSP
        inc MSI2
        bra @l
@r:     rts

FARMODE .set 0
        .segment "CODE"
