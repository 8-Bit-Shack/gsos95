; ================================================================
; GS/OS95: weitere Routinen in der zweiten Codebank (XCODE)
; Fokuswechsel zu Zubehoer-Fenstern, Laufwerkspruefung beim Wiederherstellen
; ================================================================
        .segment "XCODE2"
FARMODE .set 2

xf_foc_paint:
        jsr foc_paint
        rtl
xf_vol_known:
        jsr vol_known
        rtl

; ---------- Fokuswechsel: Titelleiste und Taskleiste direkt neu zeichnen ----------
; Der Window Manager frischt nur verdeckte Stellen auf; deshalb hier selbst,
; aber nur Bereiche, die kein Zubehoer-Fenster ueberlappt.
foc_paint:
        CALL nda_list
        stz KL                  ; Taskleiste
        lda #320
        sta KR
        lda #186
        sta KT
        lda #200
        sta KB
        jsr fp_try
        lda NWIN                ; Titelleiste des obersten eigenen Fensters
        bne @w
        rts
@w:     dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        cmp #600                ; minimiert
        bcs @r
        sta KL
        ldy #R_W
        clc
        adc (PB),y
        sta KR
        ldy #R_Y
        lda (PB),y
        sta KT
        clc
        adc #18
        sta KB
        jsr fp_try
@r:     rts
; KL..KR/KT..KB zeichnen, wenn kein Zubehoer-Fenster darueber liegt
fp_try:
        stz FPI
@l:     lda FPI
        cmp TNNDA
        bcs @draw
        asl
        asl
        tax
        lda ndawl,x             ; Fensterrecord + WD_STRUC -> Regionen-Handle
        clc
        adc #190
        sta LP
        lda ndawl+2,x
        adc #0
        sta LP+2
        ldy #2
        lda [LP],y
        tax
        lda [LP]
        sta LP
        stx LP+2
        ldy #2                  ; Handle -> Zeiger
        lda [LP],y
        tax
        lda [LP]
        sta LP
        stx LP+2
        ldy #2                  ; Begrenzung: oben, links, unten, rechts
        lda [LP],y
        cmp KB
        bpl @n
        ldy #6
        lda [LP],y
        cmp KT
        bmi @n
        beq @n
        ldy #4
        lda [LP],y
        cmp KR
        bpl @n
        ldy #8
        lda [LP],y
        cmp KL
        bmi @n
        beq @n
        rts                     ; ueberlappt: nicht selbst zeichnen
@n:     inc FPI
        bra @l
@draw:  lda NDAWAS
        pha
        stz NDAWAS
        CALL redraw_rect
        pla
        sta NDAWAS
        rts

; C=1, wenn das Laufwerk am Anfang von pathbuf angemeldet ist
vol_known:
        lda VOLOK
        bne @h
        CALL load_volumes
@h:     ldx #1                  ; Namenslaenge bis zum zweiten ':'
@ln:    cpx pathbuf
        bcs @le
        lda pathbuf+2,x
        and #$00FF
        cmp #':'
        beq @le
        inx
        bra @ln
@le:    dex
        stx VKL
        stz VKI
@vl:    lda VKI
        cmp VOLN
        bcs @no
        ldx #ENTSZ
        CALL gmul
        tax
        lda volcache,x
        cmp #$0100
        bne @nx
        lda volcache+2,x
        and #$00FF
        cmp VKL
        bne @nx
        ldy #0
@cc:    cpy VKL
        bcs @yes
        lda volcache+3,x
        eor pathbuf+3,y
        and #$00DF
        bne @nx
        inx
        iny
        bra @cc
@nx:    inc VKI
        bra @vl
@yes:   sec
        rts
@no:    clc
        rts

FARMODE .set 0
        .segment "CODE"
