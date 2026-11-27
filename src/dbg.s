; ---- Diagnose: Zaehler unten links anzeigen ----
dbg_show:
        lda SHUT
        beq *+3
        rts
        jsr cur_hide
        jsr org0
        FILL 0,176,160,9,BLACK
        lda #WHITE
        jsr setcolor
        lda #2
        sta SX
        ldx #0
@l:     phx
        lda dbgv,x
        jsr dbg_hex
        lda #177
        sta SY
        lda #dbgbuf
        sta STRP
        lda #5
        sta STRN
        jsr drawstr
        plx
        inx
        inx
        cpx #12
        bcc @l
        jsr orgr
        jmp cur_show
dbg_hex:
        pha
        lda #$2020
        sta dbgbuf+4
        pla
        ldy #3
@h:     pha
        and #$000F
        tax
        lda dbgdig,x
        sep #$20
        sta dbgbuf,y
        rep #$20
        pla
        lsr
        lsr
        lsr
        lsr
        dey
        bpl @h
        rts
dbgdig: .byte "0123456789ABCDEF"
