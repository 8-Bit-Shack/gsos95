; ================================================================
; GS/OS95: WLAN-Modem an der seriellen Schnittstelle (Teil von net.s)
; Fuer Hayes-kompatible WLAN-Modems (WiModem232, Zimodem, WiFi232 u.a.)
; und das virtuelle Modem von GSplus/KEGS: "ATDTrechner:port" baut eine
; TCP-Verbindung auf, danach fliessen die Daten unveraendert. Das Ende
; meldet das Modem mit "NO CARRIER" (oder DCD faellt ab).
; Der SCC 8530 wird ohne Interrupts abgefragt:
;   Modemanschluss (2) = Kanal B: Befehl $C038, Daten $C03A
;   Druckeranschluss (1) = Kanal A: Befehl $C039, Daten $C03B
; Baudratengenerator mit 3,6864 MHz: Zeitkonstante = 3686400/(32*Baud) - 2
; Eine Verbindung; kein TLS (https geht ueber einen Proxy).
; ================================================================
        .pushseg
        .segment "BSS"
MDX:    .res 2                  ; 0 Kanal B (Modem), 1 Kanal A (Drucker)
MDCON:  .res 2                  ; 1 = Verbindung steht
MDDCD:  .res 2                  ; 1 = DCD war an
MDEND:  .res 2                  ; 1 = Modem hat "NO CARRIER" gemeldet
MDN:    .res 2
MDI:    .res 2
MDK:    .res 2
MDTMO:  .res 2
MDT0:   .res 2
MDTC:   .res 2
MDL:    .res 42                 ; Antwortzeile des Modems (Laenge + Text)
        .popseg

SCC_CMD  = $E0C038
SCC_DATA = $E0C03A

; SCC-Register A mit Wert Y beschreiben
md_wreg:
        ldx MDX
        sep #$20
        sta f:SCC_CMD,x
        tya
        sta f:SCC_CMD,x
        rep #$20
        rts

; Kanal einstellen: 8N1, Baudrate MDBAUD, keine Interrupts, DTR/RTS an
md_init:
        stz MDX
        lda MDPORT
        cmp #1
        bne @b
        inc MDX
@b:     ldx MDX
        sep #$20
        lda f:SCC_CMD,x         ; Registerzeiger zuruecksetzen
        rep #$20
        lda #4                  ; x16, 1 Stoppbit, keine Paritaet
        ldy #$44
        jsr md_wreg
        lda #3                  ; Empfang 8 Bit, noch aus
        ldy #$C0
        jsr md_wreg
        lda #5                  ; Senden 8 Bit, DTR, RTS, noch aus
        ldy #$E2
        jsr md_wreg
        lda #11                 ; Takt fuer Senden/Empfang vom Baudratengenerator
        ldy #$50
        jsr md_wreg
        ldy #46                 ; Zeitkonstante
        lda MDBAUD
        beq @t
        ldy #10
        cmp #1
        beq @t
        ldy #4
        cmp #2
        beq @t
        ldy #1
        cmp #3
        beq @t
        ldy #0
@t:     lda #12
        jsr md_wreg
        lda #13
        ldy #0
        jsr md_wreg
        lda #14                 ; Generator aus, Quelle RTxC
        ldy #$00
        jsr md_wreg
        lda #14                 ; Generator an
        ldy #$01
        jsr md_wreg
        lda #3                  ; Empfang an
        ldy #$C1
        jsr md_wreg
        lda #5                  ; Senden an
        ldy #$EA
        jsr md_wreg
        lda #15                 ; keine externen Interrupts
        ldy #$00
        jsr md_wreg
        lda #1                  ; keine Interrupts dieses Kanals
        ldy #$00
        jmp md_wreg

; Byte A senden (wartet auf leeren Sendepuffer, hoechstens ~1 s)
md_tx:  pha
        lda #20000
        sta MDTC
@w:     ldx MDX
        sep #$20                ; nur 8 Bit: der andere Kanal liegt daneben
        lda f:SCC_CMD,x
        rep #$20
        and #$0004
        bne @go
        dec MDTC
        bne @w
@go:    pla
        sep #$20
        sta f:SCC_DATA,x
        rep #$20
        rts

; Byte empfangen? C=0 und A = Byte, sonst C=1
md_rx:  ldx MDX
        sep #$20
        lda f:SCC_CMD,x
        rep #$20
        and #$00FF
        bit #$0008              ; DCD
        beq @nd
        ldy #1
        sty MDDCD
@nd:    and #$0001
        beq @no
        sep #$20
        lda f:SCC_DATA,x
        rep #$20
        and #$00FF
        clc
        rts
@no:    sec
        rts

; DCD weg, nachdem es an war? C=1
md_lost:
        lda MDDCD
        beq @n
        ldx MDX
        sep #$20
        lda f:SCC_CMD,x
        rep #$20
        and #$0008
        bne @n
        sec
        rts
@n:     clc
        rts

; Pascal-String A (diese Codebank) senden
md_ks:  sta MDK
        lda PDST
        pha
        lda PDST+2
        pha
        jsr md_kp
        lda [PDST]
        and #$00FF
        sta MDN
        ldy #1
@l:     lda MDN
        beq @r
        lda [PDST],y
        phy
        jsr md_tx
        ply
        iny
        dec MDN
        bra @l
@r:     pla
        sta PDST+2
        pla
        sta PDST
        rts
; PDST = Bank dieses Codes : MDK
md_kp:  lda MDK
        sta PDST
        sep #$20
        phk
        pla
        rep #$20
        and #$00FF
        sta PDST+2
        rts

; Zeile vom Modem nach MDL (ohne CR/LF, leere Zeilen uebergehen).
; MDTMO = Wartezeit in Ticks. C=1: keine Zeile (Zeit um oder Esc)
md_line:
        stz MDL
        jsr t_now
        sta MDT0
@l:     jsr md_rx
        bcs @w
        cmp #$0D
        beq @l
        cmp #$0A
        beq @eol
        pha
        lda MDL
        and #$00FF
        tax
        pla
        cpx #40
        bcs @l
        sep #$20
        sta MDL+1,x
        inc MDL
        rep #$20
        bra @l
@eol:   lda MDL
        and #$00FF
        beq @l
        clc
        rts
@w:     jsr net_esc
        bcs @x
        jsr t_now
        sec
        sbc MDT0
        cmp MDTMO
        bcc @l
@x:     sec
        rts

; beginnt MDL mit dem Pascal-String A (diese Codebank)? C=0 ja
md_pre: sta MDK
        lda PDST
        pha
        lda PDST+2
        pha
        jsr md_kp
        lda [PDST]
        and #$00FF
        sta MDN
        lda MDL
        and #$00FF
        cmp MDN
        bcc @no
        ldy #1
@l:     cpy MDN
        beq @c
        bcs @yes
@c:     lda [PDST],y
        sep #$20
        cmp MDL,y
        rep #$20
        bne @no
        iny
        bra @l
@yes:   pla
        sta PDST+2
        pla
        sta PDST
        clc
        rts
@no:    pla
        sta PDST+2
        pla
        sta PDST
        sec
        rts

; A Ticks warten (Empfangenes verwerfen)
md_wait:
        sta MDN
        jsr tm_start
@w:     jsr md_rx
        jsr t_elapsed
        cmp MDN
        bcc @w
        rts

; ---------- Modem bereit? "AT" -> "OK" ----------
md_start:
        jsr md_init
        stz MDCON
        stz MDEND
        stz MDDCD
        ldx #14
@c:     stz NCUR,x
        dex
        dex
        bpl @c
        lda #2
        sta MDI
@try:   lda #6
        jsr md_wait
        lda #md_s_at
        jsr md_ks
        lda #120                ; 2 s
        sta MDTMO
@l:     jsr md_line
        bcs @nx
        lda #md_s_ok
        jsr md_pre
        bcs @l
        lda #1
        sta NDHOK
        clc
        rts
@nx:    lda NABORT
        bne @f
        dec MDI
        bne @try
        lda #NE_MDM
        sta NERR
@f:     sec
        rts

; ---------- Verbindung: ATDTrechner:port ----------
md_open:
        lda MDCON               ; noch verbunden? erst auflegen
        beq @d
        jsr md_close
@d:     stz MDEND
        stz MDDCD
        lda #3
        jsr md_wait             ; Reste verwerfen
        lda #md_s_dial
        jsr md_ks
        jsr net_ptr
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        sta MDI
        iny
@h:     lda MDI
        beq @p
        lda [PSRC],y
        phy
        jsr md_tx
        ply
        iny
        dec MDI
        bra @h
@p:     lda #':'
        jsr md_tx
        lda DPORT
        jsr md_dec
        lda #$0D
        jsr md_tx
        lda #2400               ; 40 s
        sta MDTMO
@l:     jsr md_line
        bcs @to
        lda #md_s_conn
        jsr md_pre
        bcc @ok
        lda #md_s_nocar
        jsr md_pre
        bcc @ref
        lda #md_s_err
        jsr md_pre
        bcc @ref
        lda #md_s_busy
        jsr md_pre
        bcc @ref
        lda #md_s_noans
        jsr md_pre
        bcc @ref
        bra @l                  ; Echo des Befehls u.a.
@to:    lda NABORT
        bne @ab
        lda #NE_TIME
        sta NERR
@ab:    lda #$0D                ; Waehlen abbrechen
        jsr md_tx
        sec
        rts
@ref:   lda #NE_CONN
        sta NERR
        sec
        rts
@ok:    lda #1
        sta MDCON
        clc
        rts

; Zahl A dezimal senden
md_dec: sta MDN
        lda #10000
        sta MDK
        stz MDI
@d:     ldx #'0'
@s:     lda MDN
        cmp MDK
        bcc @o
        sbc MDK
        sta MDN
        inx
        bra @s
@o:     cpx #'0'
        bne @w
        lda MDI
        bne @w
        lda MDK
        cmp #1
        bne @n
@w:     txa
        jsr md_tx
        inc MDI
@n:     lda MDK
        ldx #0
@q:     cmp #10
        bcc @qe
        sbc #10
        inx
        bra @q
@qe:    stx MDK
        lda MDK
        bne @d
        rts

; ---------- WCNT Bytes ab WPTR senden ----------
md_write:
        jsr net_ptr
        ldy WPTR
        lda WCNT
        sta MDI
@l:     lda MDI
        beq @r
        lda [PSRC],y
        phy
        jsr md_tx
        ply
        iny
        dec MDI
        bra @l
@r:     clc
        rts

; empfangenes Byte A nach WPTR; am Zeilenende auf "NO CARRIER" pruefen
md_put: pha
        jsr net_ptr
        pla
        ldy WPTR
        sep #$20
        sta [PSRC],y
        rep #$20
        inc WPTR
        cmp #$0A
        bne @r
        lda WPTR                ; "NO CARRIER" CR LF am Ende?
        cmp #12
        bcc @r
        sec
        sbc #12
        sta MDI
        lda PDST
        pha
        lda PDST+2
        pha
        lda #md_s_nocr
        sta MDK
        jsr md_kp
        inc PDST                ; hinter das Laengenbyte
        ldy #0
@c:     lda [PDST],y
        and #$00FF
        sta MDN
        phy
        tya
        clc
        adc MDI
        tay
        lda [PSRC],y
        ply
        and #$00FF
        cmp MDN
        bne @no
        iny
        cpy #12
        bcc @c
        lda MDI                 ; Meldung entfernen, mit CR LF davor
        sta WPTR
        cmp #2
        bcc @e
        tay
        dey
        dey
        lda [PSRC],y
        cmp #$0A0D
        bne @e
        dec WPTR
        dec WPTR
@e:     lda #1
        sta MDEND
        stz MDCON
@no:    pla
        sta PDST+2
        pla
        sta PDST
@r:     rts

; ---------- alles bis zum Ende lesen (nach WPTR, hoechstens bis NLIM) ----------
md_readall:
        stz NFULL
        jsr tm_start
@l:     lda MDEND
        bne @done
        jsr md_rx
        bcs @e
        ldx WPTR
        cpx NLIM
        bcs @full
        jsr md_put
        lda NSTRM               ; Platz fuer ein weiteres Stueck?
        beq @pr
        lda NLIM
        sec
        sbc WPTR
        cmp #513
        bcs @pr
        lda #1
        sta NFULL
        clc
        rts
@pr:    lda WPTR                ; alle 256 Bytes Fortschritt zeigen
        and #$00FF
        bne @q
        lda NQUIET
        bne @q
        JSLF xf_web_prog
@q:     jsr tm_start
        bra @l
@full:  lda NSTRM
        sta NFULL
        clc
        rts
@e:     jsr md_lost
        bcs @gone
        jsr net_esc
        bcs @x
        jsr t_elapsed
        cmp #1200               ; 20 s ohne Daten
        bcc @l
        lda #NE_TIME
        sta NERR
@x:     sec
        rts
@gone:  stz MDCON
@done:  lda NQUIET
        bne @d2
        JSLF xf_web_prog
@d2:    clc
        rts

; ---------- ohne Warten: was da ist. C=1: Verbindung zu ----------
md_poll:
        lda #400
        sta MDN
@l:     lda MDEND
        bne @cl
        ldx WPTR
        cpx NLIM
        bcs @ok
        jsr md_rx
        bcs @e
        jsr md_put
        lda #400                ; nach jedem Byte kurz weiter lauschen
        sta MDN
        bra @l
@e:     dec MDN
        bne @l
        jsr md_lost
        bcs @cl
@ok:    clc
        rts
@cl:    stz MDCON
        sec
        rts

; ---------- auflegen: Pause, "+++", Pause, "ATH" ----------
md_close:
        lda MDCON
        beq @r
        stz MDCON
        lda #66                 ; 1,1 s Ruhe
        jsr md_wait
        lda #md_s_esc
        jsr md_ks
        lda #66
        jsr md_wait
        lda #md_s_hang
        jsr md_ks
        lda #30
        jsr md_wait
@r:     clc
        rts

md_s_at:    .byte 3,"AT",13
md_s_ok:    .byte 2,"OK"
md_s_dial:  .byte 4,"ATDT"
md_s_conn:  .byte 7,"CONNECT"
md_s_nocar: .byte 10,"NO CARRIER"
md_s_nocr:  .byte 12,"NO CARRIER",13,10
md_s_err:   .byte 5,"ERROR"
md_s_busy:  .byte 4,"BUSY"
md_s_noans: .byte 9,"NO ANSWER"
md_s_esc:   .byte 3,"+++"
md_s_hang:  .byte 4,"ATH",13
