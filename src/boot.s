; ProDOS-Bootblock: laedt GS/OS95 von zusammenhaengenden Bloecken nach $2000
        .setcpu "65C02"
        .org $0800
        .byte $01
        sei
        stx $43
        stx $54
        txa
        lsr
        lsr
        lsr
        lsr
        ora #$C0
        sta $3F
        sta call+2
        lda #$FF
        sta $3E
        ldy #0
        lda ($3E),y
        sta call+1
        lda #$20
        sta $50
        lda #<FIRST
        sta $51
        lda #>FIRST
        sta $52
        lda #COUNT
        sta $53
loop:   lda #1
        sta $42
        lda $54
        sta $43
        stz $44
        lda $50
        sta $45
        lda $51
        sta $46
        lda $52
        sta $47
        jsr call
        bcs err
        inc $50
        inc $50
        inc $51
        bne :+
        inc $52
:       dec $53
        bne loop
        jmp $2000
call:   jmp $FFFF
err:    lda #$C5                ; 'E' auf den Textbildschirm
        sta $0400
        lda #$D2
        sta $0401
        sta $0402
:       bra :-
