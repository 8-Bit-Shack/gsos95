; ================================================================
; GS/OS95: SSH ueber die FujiNet (XCODE4, x4.s)
; ssh://benutzer@rechner[:port] im Browser: ssh_go (term.inc) oeffnet das
; Vollbild-Terminal, hier werden Benutzer und Passwort abgefragt. Danach
; oeffnet fn_open "N:SSH://rechner:port/" (vorher $FD Benutzer, $FE
; Passwort) - die Karte verschluesselt, der IIgs sieht nur Klartext.
; Andere Karten koennen kein SSH (Meldung ws_e_ssh).
; ================================================================
        .pushseg
        .segment "BSS"
SSHPW:  .res 2                  ; 1 Benutzer, 2 Passwort wird eingegeben
SSHON:  .res 2                  ; SSH-Sitzung laeuft (Return ohne NUL)
FNSSH:  .res 2                  ; naechstes fn_open als SSH
SSHU:   .res 2+32               ; Benutzer (Laenge + Zeichen)
SSHP:   .res 2+64               ; Passwort
SSK:    .res 2
SST:    .res 2
SSI:    .res 2
        .popseg

; WB_HOST "benutzer@rechner" -> SSHU und WB_HOST "rechner"
xf_ssh_prep:
        lda NWB
        sta PSRC+2
        stz PSRC
        stz SSHU
        stz SSHP
        lda #1
        sta FNSSH
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        sta SST
        ldx #0
@f:     cpx SST                 ; '@' suchen
        bcs @r
        txa
        clc
        adc #WB_HOST+1
        tay
        lda [PSRC],y
        and #$00FF
        cmp #'@'
        beq @at
        inx
        bra @f
@at:    stx SSI
        ldx #0                  ; Benutzer kopieren (hoechstens 32)
@c:     cpx SSI
        bcs @c2
        cpx #32
        bcs @c2
        txa
        clc
        adc #WB_HOST+1
        tay
        lda [PSRC],y
        sep #$20
        sta SSHU+2,x
        rep #$20
        inx
        bra @c
@c2:    stx SSHU
        ldx #0                  ; Rechner nach vorn
        lda SSI
        inc a
        sta SSI
@m:     lda SSI
        cmp SST
        bcs @m2
        clc
        adc #WB_HOST+1
        tay
        lda [PSRC],y
        pha
        txa
        clc
        adc #WB_HOST+1
        tay
        pla
        sep #$20
        sta [PSRC],y
        rep #$20
        inc SSI
        inx
        bra @m
@m2:    txa
        ldy #WB_HOST
        sep #$20
        sta [PSRC],y
        rep #$20
@r:     rtl

; erste Frage im Terminal
xf_ssh_ask:
        lda #0
        jsr x4_cur
        lda #12                 ; Schirm leer
        jsr x4_put
        lda #ss_head
        ldx #ss_head
        jsr ss_str
        jsr ss_who
        jsr x4_nl
        jsr x4_nl
        jsr ss_q
        lda #1
        jsr x4_cur
        rtl

; Frage nach Benutzer (SSHU leer) oder Passwort
ss_q:   lda SSHU
        bne @pw
        lda #1
        sta SSHPW
        lda #ss_user
        ldx #ss_user_en
        jmp ss_str
@pw:    lda #2
        sta SSHPW
        stz SSHP
        jsr ss_who
        lda #ss_pass
        ldx #ss_pass_en
        jmp ss_str

; "benutzer@rechner"
ss_who: ldx #0
@u:     cpx SSHU
        bcs @h
        lda SSHU+2,x
        and #$00FF
        phx
        jsr x4_put
        plx
        inx
        bra @u
@h:     lda SSHU
        beq @h0
        lda #'@'
        jsr x4_put
@h0:    lda NWB
        sta PSRC+2
        stz PSRC
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        sta SST
        ldy #WB_HOST+1
@l:     lda SST
        beq @r
        lda [PSRC],y
        and #$00FF
        phy
        jsr x4_put
        ply
        iny
        dec SST
        bra @l
@r:     rts

; Text A (Deutsch) / X (Englisch) dieser Bank
ss_str: ldy LANG
        beq @d
        txa
@d:     jsr x4_kb
        lda [LP2]
        and #$00FF
        sta SST
        ldy #1
@l:     lda SST
        beq @r
        lda [LP2],y
        and #$00FF
        phy
        jsr x4_put
        ply
        iny
        dec SST
        bra @l
@r:     rts

; Taste A. C=1: Terminal verlassen
xf_ssh_key:
        and #$007F
        sta SSK
        lda #0
        jsr x4_cur
        lda SSK
        cmp #$1B
        bne @k1
        jmp ss_quit
@k1:    cmp #$0D
        bne @k2
        jmp ss_ret
@k2:    cmp #$08
        beq @bs
        cmp #$7F
        beq @bs
        cmp #' '
        bcc @x
        ldx SSHPW
        cpx #2
        beq @p
        ldx SSHU                ; Benutzer: sichtbar
        cpx #32
        bcs @x
        sep #$20
        sta SSHU+2,x
        rep #$20
        inc SSHU
        jsr x4_put
        bra @x
@p:     ldx SSHP                ; Passwort: nichts anzeigen
        cpx #64
        bcs @x
        sep #$20
        sta SSHP+2,x
        rep #$20
        inc SSHP
        bra @x
@bs:    lda SSHPW
        cmp #2
        beq @bp
        lda SSHU
        beq @x
        dec SSHU
        lda #8
        jsr x4_put
        lda #' '
        jsr x4_put
        lda #8
        jsr x4_put
        bra @x
@bp:    lda SSHP
        beq @x
        dec SSHP
@x:     lda #1
        jsr x4_cur
        clc
        rtl

ss_ret: jsr x4_nl
        lda SSHPW
        cmp #2
        beq @go
        lda SSHU                ; Benutzer fertig: jetzt das Passwort
        beq @x
        jsr ss_q
@x:     lda #1
        jsr x4_cur
        clc
        rtl
@go:    stz SSHPW
        lda #ss_conn
        ldx #ss_conn_en
        jsr ss_str
        lda WPORT
        sta DPORT
        stz NTLS
        lda #1
        sta FNSSH
        JSLF xf_tcp_open
        php
        jsr ss_wipe
        plp
        bcs @fail
        lda #1                  ; verbunden: weiter wie Telnet
        sta TNON
        sta SSHON
        lda #1
        jsr x4_cur
        clc
        rtl
@fail:  JSLF xf_tcp_close
        lda #ss_fail
        ldx #ss_fail_en
        jsr ss_str
        lda #1                  ; naechste Taste: zurueck zum Browser
        sta VTWAIT
        clc
        rtl

ss_quit:
        stz SSHPW
        stz FNSSH
        stz TNON
        jsr ss_wipe
        sec
        rtl

; Passwort aus dem Speicher loeschen
ss_wipe:
        ldx #64
@l:     stz SSHP,x
        dex
        dex
        bpl @l
        rts

ss_head   : .byte 4,$73,$73,$68,$20
ss_user   : .byte 10,$42,$65,$6E,$75,$74,$7A,$65,$72,$3A,$20
ss_user_en: .byte 10,$6C,$6F,$67,$69,$6E,$20,$61,$73,$3A,$20
ss_pass   : .byte 13,$20,$2D,$20,$50,$61,$73,$73,$77,$6F,$72,$74,$3A,$20
ss_pass_en: .byte 13,$27,$73,$20,$70,$61,$73,$73,$77,$6F,$72,$64,$3A,$20
ss_conn   : .byte 24,$56,$65,$72,$62,$69,$6E,$64,$65,$20,$28,$46,$75,$6A,$69,$4E,$65,$74,$29,$20,$2E,$2E,$2E,$0D,$0A
ss_conn_en: .byte 26,$43,$6F,$6E,$6E,$65,$63,$74,$69,$6E,$67,$20,$28,$46,$75,$6A,$69,$4E,$65,$74,$29,$20,$2E,$2E,$2E,$0D,$0A
ss_fail   : .byte 84,$0D,$0A,$41,$6E,$6D,$65,$6C,$64,$75,$6E,$67,$20,$66,$65,$68,$6C,$67,$65,$73,$63,$68,$6C,$61,$67,$65,$6E,$20,$6F,$64,$65,$72,$20,$52,$65,$63,$68,$6E,$65,$72,$20,$6E,$69,$63,$68,$74,$20,$65,$72,$72,$65,$69,$63,$68,$62,$61,$72,$20,$2D,$20,$62,$69,$74,$74,$65,$20,$65,$69,$6E,$65,$20,$54,$61,$73,$74,$65,$20,$64,$72,$81,$63,$6B,$65,$6E,$2E
ss_fail_en: .byte 51,$0D,$0A,$4C,$6F,$67,$69,$6E,$20,$66,$61,$69,$6C,$65,$64,$20,$6F,$72,$20,$68,$6F,$73,$74,$20,$75,$6E,$72,$65,$61,$63,$68,$61,$62,$6C,$65,$20,$2D,$20,$70,$72,$65,$73,$73,$20,$61,$6E,$79,$20,$6B,$65,$79,$2E
