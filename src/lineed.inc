; ================================================================
; GS/OS95 Browser: Zeileneditor fuer Adresszeile und Formularfelder
; (Teil von web.s, XCODE2). Zeichenkette in der Pufferbank bei LEB
; (Laengenbyte + Text), hoechstens LEM Zeichen, Schreibmarke LEC.
;   Pfeil links/rechts      ein Zeichen
;   Wahl-Pfeil              ein Wort
;   Apfel-Pfeil, Strg-A/E   Anfang / Ende
;   Entf (Delete)           Zeichen vor der Schreibmarke loeschen
;   Strg-D                  Zeichen hinter der Schreibmarke loeschen (nur Formularfelder;
;                           in der Adresszeile merkt Strg-D einen Favoriten)
;   druckbare Zeichen       an der Schreibmarke einfuegen
; ================================================================
        .pushseg
        .segment "BSS"
LEB:    .res 2
LEM:    .res 2
LEC:    .res 2
LEN_:   .res 2
LEI:    .res 2
ADRCUR: .res 2                  ; Schreibmarke der Adresszeile
FCUR:   .res 2                  ; Schreibmarke im Formularfeld FFOC
UA:     .res 2
        .popseg

; Zeichen an Position A (0..) -> A
le_ch:  sec
        adc LEB                 ; LEB + 1 + A
        tay
        BRD
        rts
; ist A ein Wortzeichen (Ziffer, Buchstabe, Umlaut)? C=1 ja
le_word:
        cmp #$80
        bcs @y
        cmp #'0'
        bcc @n
        cmp #'9'+1
        bcc @y
        and #$00DF
        cmp #'A'
        bcc @n
        cmp #'Z'+1
        bcc @y
@n:     clc
        rts
@y:     sec
        rts

; Taste KEYC bearbeiten. C=0 benutzt (neu zeichnen), C=1 nicht fuer den Editor
le_key: ldy LEB
        BRD
        sta LEN_
        lda LEC
        cmp LEN_
        bcc @c0
        lda LEN_
        sta LEC
@c0:    lda KEYC
        cmp #$01                ; Strg-A
        bne @k1
@home:  stz LEC
        clc
        rts
@k1:    cmp #$05                ; Strg-E
        bne @k2
@end:   lda LEN_
        sta LEC
        clc
        rts
@k2:    cmp #$08                ; Pfeil links
        bne @k3
        lda EVREC+14
        bit #$0100
        bne @home
        bit #$0800
        bne @wl
        lda LEC
        beq @ok
        dec LEC
@ok:    clc
        rts
@wl:    lda LEC                 ; Wort links: Trenner, dann Wort
        beq @ok
        dec a
        jsr le_ch
        jsr le_word
        bcs @wl2
        dec LEC
        bra @wl
@wl2:   lda LEC
        beq @ok
        dec a
        jsr le_ch
        jsr le_word
        bcc @ok
        dec LEC
        bra @wl2
@k3:    cmp #$15                ; Pfeil rechts
        bne @k4
        lda EVREC+14
        bit #$0100
        bne @end
        bit #$0800
        bne @wr
        lda LEC
        cmp LEN_
        bcs @ok
        inc LEC
        clc
        rts
@wr:    lda LEC                 ; Wort rechts: Wort, dann Trenner
        cmp LEN_
        bcs @ok
        jsr le_ch
        jsr le_word
        bcc @wr2
        inc LEC
        bra @wr
@wr2:   lda LEC
        cmp LEN_
        bcs @ok
        jsr le_ch
        jsr le_word
        bcs @ok
        inc LEC
        bra @wr2
@k4:    cmp #$7F                ; Entf: Zeichen davor
        bne @k5
        lda LEC
        beq @ok
        dec LEC
        jmp le_cut
@k5:    cmp #$04                ; Strg-D: Zeichen dahinter
        bne @k6
        lda LEC
        cmp LEN_
        bcc *+5
        jmp @ok
        jmp le_cut
@k6:    cmp #' '
        bcc @no
        cmp #$7F
        bcs @no
        lda LEN_                ; einfuegen
        cmp LEM
        bcc *+5
        jmp @ok
        sta LEI                 ; Zeichen LEC..Laenge-1 eins nach rechts
@sh:    lda LEI
        cmp LEC
        beq @put
        dec a
        jsr le_ch
        pha
        lda LEI
        jsr le_ch               ; Y = Ziel (nur fuer die Adresse)
        pla
        BWR
        dec LEI
        bra @sh
@put:   lda LEC
        jsr le_ch
        lda KEYC
        BWR
        inc LEC
        lda LEN_
        inc a
        ldy LEB
        BWR
        clc
        rts
@no:    sec
        rts
; Zeichen an LEC entfernen
le_cut: lda LEC
        sta LEI
@l:     lda LEI
        inc a
        cmp LEN_
        bcs @e
        jsr le_ch
        pha
        lda LEI
        jsr le_ch
        pla
        BWR
        inc LEI
        bra @l
@e:     lda LEN_
        dec a
        ldy LEB
        BWR
        clc
        rts

; Breite der ersten A Zeichen ab LEB -> A
le_width:
        sta LEI
        stz LEN_
@l:     lda LEI
        beq @r
        dec LEI
        lda LEI
        jsr le_ch
        jsr cw
        clc
        adc LEN_
        sta LEN_
        bra @l
@r:     lda LEN_
        rts
