; ================================================================
; GS/OS95: Hintergrundmuster des Schreibtischs (DESKPAT 1..4)
; Zeichnet nur die dunklen Punkte in die frisch gefuellte Flaeche,
; begrenzt auf den Zeichenbereich KL..KR/KT..KB. Zweite Codebank.
; ================================================================


xf_deskpat:
        jsr deskpat
        rtl
deskpat:
        lda DESKPAT
        bne @go
        rts
@go:    lda #0                  ; Punktfarbe: schwarz (auf schwarzem Grund weiss)
        ldx DESKC
        bne @c
        lda #15
@c:     sta PD
        lda KL
        bpl @l
        lda #0
@l:     sta PXS
        lda KR
        cmp #321
        bcc @r
        lda #320
@r:     sta PXE
        lda KT
        bpl @t
        lda #0
@t:     sta PY
        lda KB
        cmp #187
        bcc @b
        lda #186
@b:     sta PYE
        lda PXS
        cmp PXE
        bcc @row
        rts
@row:   lda PY
        cmp PYE
        bcc @do
        rts
@do:    jsr pat_mask
        lda PM
        bne *+5
        jmp @next
        lda PY
        asl
        tax
        lda linetab,x
        sta PTR
        lda PXS
        sta PX0
        lda PXE
        sta PX1
        lda PX0                 ; ungerader linker Rand: einzelner Punkt
        lsr
        bcc @le
        lda PX0
        jsr pat_plot
        inc PX0
@le:    lda PX1                 ; ungerader rechter Rand
        lsr
        bcc @re
        dec PX1
        lda PX1
        cmp PX0
        bcc @next
        jsr pat_plot
@re:    lda PX0
        lsr
        tay
        lda PX1
        lsr
        sta PX1
@by:    cpy PX1
        bcs @next
        tya
        and #7
        asl
        tax
        lda pbit,x              ; Bits 15/14 des Bytes b: Maske um 2*b nach links
        and PM
        beq @nb                 ; kein dunkler Punkt in diesem Byte
        sta PT
        lda pbith,x
        and PT
        beq @lo
        lda pbitl,x
        and PT
        beq @hi
        lda PD                  ; beide Punkte dunkel
        asl
        asl
        asl
        asl
        ora PD
        sep #$20
        sta [PTR],y
        rep #$20
        bra @nb
@hi:    sep #$20                ; nur der linke Punkt
        lda [PTR],y
        and #$0F
        sta PT2
        lda PD
        asl
        asl
        asl
        asl
        ora PT2
        sta [PTR],y
        rep #$20
        bra @nb
@lo:    sep #$20                ; nur der rechte Punkt
        lda [PTR],y
        and #$F0
        ora PD
        sta [PTR],y
        rep #$20
@nb:    iny
        bra @by
@next:  inc PY
        jmp @row

; einzelnen Punkt A (X-Koordinate) setzen, wenn die Maske ihn enthaelt
pat_plot:
        sta PT
        and #15
        asl
        tax
        lda pbit1,x
        and PM
        beq @r
        lda PT
        lsr
        tay
        bcs @odd
        sep #$20
        lda [PTR],y
        and #$0F
        sta PT2
        lda PD
        asl
        asl
        asl
        asl
        ora PT2
        sta [PTR],y
        rep #$20
        rts
@odd:   sep #$20
        lda [PTR],y
        and #$F0
        ora PD
        sta [PTR],y
        rep #$20
@r:     rts

; Maske der 16 Punkte fuer Zeile PY -> PM (Bit 15 = Punkt 0)
pat_mask:
        lda DESKPAT
        cmp #2
        beq @p2
        cmp #3
        beq @p3
        cmp #4
        beq @p4
        lda PY                  ; 1: Punkte
        and #3
        bne @z
        lda #$8888
        sta PM
        rts
@z:     stz PM
        rts
@p2:    lda PY                  ; 2: Karo
        and #2
        bne @p2b
        lda #$CCCC
        sta PM
        rts
@p2b:   lda #$3333
        sta PM
        rts
@p3:    lda PY                  ; 3: schraege Linien
        and #7
        tax
        lda #$8080
@p3l:   cpx #0
        beq @p3d
        lsr
        bcc *+5
        ora #$8000
        dex
        bra @p3l
@p3d:   sta PM
        rts
@p4:    lda PY                  ; 4: Ziegel
        and #7
        bne @p4b
        lda #$FFFF
        sta PM
        rts
@p4b:   lda PY
        and #8
        bne @p4c
        lda #$8000
        sta PM
        rts
@p4c:   lda #$0080
        sta PM
        rts


