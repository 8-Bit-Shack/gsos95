; ================================================================
; GS/OS95 Browser: Telnet und IRC (Teil von web.s)
; telnet://rechner[:port]   irc://rechner[:port]/kanal
; Empfangenes erscheint als Text im Fenster (bis 8 KB, aeltere Zeilen fallen
; weg), die Adresszeile ist die Eingabezeile: Return sendet.
; Abgefragt wird im Leerlauf der Hauptschleife (xf_tn_poll).
; IRC: NICK/USER automatisch, PING -> PONG, nach 001 JOIN #kanal.
;   Eingabe ohne "/" geht an den Kanal, "/befehl ..." wird als BEFEHL ...
;   geschickt (/msg nick text -> PRIVMSG nick :text).
; ================================================================
TN_IN   = $F000                 ; IRC: Zeile im Aufbau (bis 500)
TNMAX   = WB_TXT+$2000          ; Textende hoechstens
TNCUT   = $1000                 ; so viel faellt vorn weg

        .pushseg
        .segment "BSS"
TNON:   .res 2                  ; 0 aus, 1 Telnet, 2 IRC
TNIAC:  .res 2
TNVERB: .res 2
TNECHO: .res 2                  ; Gegenstelle zeigt Eingaben selbst
TNLL:   .res 2
TNCH:   .res 34                 ; IRC-Kanal (Laenge + Text)
TNNICK: .res 12
TNNEW:  .res 2
TNP:    .res 2
TNE:    .res 2
TNQ:    .res 2
TNI:    .res 2
TNTMP:  .res 2
TNFR:   .res 2
TNC:    .res 2
TNCP:   .res 2
TNCMD:  .res 2
TNB:    .res 2
TNNE:   .res 2
TNPEND: .res 2
TNX:    .res 2
TNAC:   .res 2
TNPU:   .res 2
TNCNT:  .res 2
TNSBO:  .res 2
TNSBQ:  .res 2
TNSBN:  .res 2
        .popseg

; ---------- Verbindung steht (aus web_load) ----------
tn_start:
        jsr net_ptr0
        lda USCH
        sec
        sbc #3
        cmp #3                  ; SSH: wie Telnet
        bne *+5
        lda #1
        sta TNON
        stz TNIAC
        stz SSHON
        stz TNECHO
        stz TNLL
        stz TNPEND
        stz NDISC               ; Telnet/IRC: Gegenstelle spricht zuerst
        lda #WB_TXT
        sta TXTEND
        ldy #WB_TITLE           ; Titel: Rechnername
        lda #0
        BWR
        ldx #WB_HOST
        ldy #WB_TITLE
        jsr bs_copy
        lda TNON
        cmp #2
        bne @tn
        jsr irc_hello
@tn:    WS ws_ready             ; Statuszeile: Fertig
        sta STMSG
        stz STXK
        lda TNON                ; Telnet: Vollbild-Terminal
        cmp #1
        bne @lm
        jsr vt_enter
        bcs @lm                 ; kein Speicher: Zeilenmodus
        rts
@lm:    WS ws_tnon
        jsr tn_msg
        jmp tn_view

; ssh://benutzer@rechner: nur FujiNet. Terminal oeffnen, Benutzer/Passwort
; abfragen (ssh.inc, XCODE4), dann verbindet xf_ssh_key
ssh_go: lda NCARD
        cmp #NC_FUJI
        beq @f
        WS ws_e_ssh
        jmp err_page
@f:     JSLF xf_ssh_prep
        jsr tn_start
        stz TNON                ; noch keine Verbindung
        lda VTON
        beq @r
        JSLF xf_ssh_ask
@r:     rts

; Meldung A (Text dieser Codebank) als eigene Zeile
tn_msg: sta PDST
        CBANK
        sta PDST+2
        lda [PDST]
        and #$00FF
        sta TNTMP
        ldy #1
@l:     lda TNTMP
        beq @e
        lda [PDST],y
        and #$00FF
        phy
        jsr tn_put
        ply
        iny
        dec TNTMP
        bra @l
@e:     lda #$0A
        jmp tn_put

; Zeichen A an den Text (aeltere Zeilen fallen weg)
tn_put: sta TNC
        lda TXTEND
        cmp #TNMAX
        bcc @p
        jsr tn_cut
@p:     ldy TXTEND
        lda TNC
        BWR
        inc TXTEND
        lda #1
        sta TNNEW
        rts
tn_cut: ldy #WB_TXT+TNCUT       ; ab dem naechsten Zeilenende behalten
@f:     cpy TXTEND
        bcs @all
        BRD
        iny
        cmp #$0A
        bne @f
        sty TNCP
        ldx #WB_TXT
@m:     ldy TNCP
        cpy TXTEND
        bcs @d
        BRD
        phy
        txy
        BWR
        ply
        inx
        inc TNCP
        bra @m
@d:     stx TXTEND
        rts
@all:   lda #WB_TXT
        sta TXTEND
        rts

; Zeilen neu, ans Ende blaettern, zeichnen wenn der Browser vorn ist
tn_view:
        jsr net_ptr0
        ldx #WB_TURL
        ldy #WB_BASE
        jsr bs_copy
        lda CW
        sec
        sbc #30
        sta WRAPW
        jsr lay_all
        lda NLINES
        sec
        sbc NRV
        bcs @p
        lda #0
@p:     sta PAGE
        stz TNNEW
        rts

; ---------- Leerlauf: Daten abholen ----------
tn_poll:
        lda TNON
        bne @go
        rts
@go:    lda NWB
        bne @g2
        rts
@g2:    jsr net_ptr0
        stz WPTR
        lda #$2000
        sta NLIM
        inc NQUIET
        JSLF xf_tcp_poll
        stz NQUIET
        php
        jsr vt_curoff
        plp
        bcc @op
        lda WPTR
        jsr tn_feed
        jsr net_ptr0
        lda VTON
        beq @lm
        JSLF xf_xm_kill         ; Uebertragung beenden
        WS ws_vtend             ; Terminal: Meldung, auf Taste warten
        jsr vt_msg
        lda #1
        sta VTWAIT
        stz TNON
        JSLF xf_tcp_close
        rts
@lm:    WS ws_tnoff
        jsr tn_msg
        lda #1                  ; Verbindung zu
        sta TNNEW
        stz TNON
        JSLF xf_tcp_close
        bra @show
@op:    lda WPTR
        beq @nd
        jsr tn_feed
        jsr tn_flush
@nd:    lda VTON
        beq @show
        JSLF xf_xm_tick         ; Uebertragung: Zeitueberschreitungen
        jsr net_ptr0
        jmp vt_curon
@show:  lda TNNEW
        beq @r
        jsr tn_view
        lda WIN                 ; Browser vorn: zeigen
        cmp #21
        bne @r
        CALL redraw_cont
@r:     rts

; Bytes 0..A verarbeiten (Telnet-Befehle, IRC-Zeilen)
tn_feed:
        sta TNE
        stz TNP
@l:     lda TNP
        cmp TNE
        bcc *+5
        jmp @e
        tay
        BRD
        inc TNP
        ldx TNIAC
        beq @n0
        jsr tn_iac
        bra @l
@n0:    cmp #$FF
        bne @n1
        lda #1
        sta TNIAC
        bra @l
@n1:    ldx TNON
        cpx #2
        bne @tel
        jsr irc_byte
        bra @l
@tel:   jsr tn_char
        bra @l
@e:     rts

; Telnet: Zeichen anzeigen
tn_char:
        ldx VTON                ; Vollbild-Terminal: alles roh dorthin
        beq @lm
        ldx XMON                ; Dateiuebertragung laeuft
        cpx #3
        bcs @xf
        JSLF xf_zm_scan         ; ZMODEM-Anfang?
        bcs @x0
        jmp vt_char
@xf:    JSLF xf_xm_byte
@x0:    rts
@lm:    cmp #13
        beq @r
        cmp #10
        beq @nl
        cmp #8                  ; Rueckschritt
        beq @bs
        cmp #9
        bne @n
        lda #' '
@n:     cmp #' '
        bcc @r
        cmp #$7F
        bcc @a
        jsr tn_hi
@a:     jmp tn_put
@nl:    lda #$0A
        jmp tn_put
@bs:    lda TXTEND
        cmp #WB_TXT+1
        bcc @r
        ldy TXTEND
        dey
        BRD
        cmp #$0A
        beq @r
        dec TXTEND
        lda #1
        sta TNNEW
@r:     rts
; Latin-1 -> Umlaute, sonst '?'
tn_hi:  cmp #$A0
        bcc @q
        cmp #$100
        bcs @q
        sec
        sbc #$A0
        tax
        lda #w_lat1
        jsr xtab
        cmp #0
        bne @r
@q:     lda #'?'
@r:     rts

; Telnet-Befehl (Zustand TNIAC): 1 nach IAC, 2 Option, 3 SB-Option, 4 SB-Daten
; Wir koennen: Binaer (0), SGA (3), Terminaltyp (24, "ANSI"), Fenstergroesse
; (31, 80 x 25). Die Gegenstelle darf: Binaer, Echo (1), SGA.
tn_iac: ldx TNIAC
        cpx #1
        bne @s2
        cmp #$FF                ; IAC IAC = Zeichen 255
        bne @sb
        stz TNIAC
        jmp tn_char
@sb:    cmp #$FA                ; SB
        bne @v
        lda #3
        sta TNIAC
        rts
@v:     cmp #$FB
        bcc @z
        sta TNVERB
        lda #2
        sta TNIAC
        rts
@z:     stz TNIAC
        rts
@s2:    cpx #2
        beq @opt
        jmp @s3
@opt:   stz TNIAC
        sta TNTMP               ; Option
        lda TNVERB
        cmp #$FB                ; WILL x
        bne @do
        lda TNTMP
        cmp #1
        bne @w3
        lda #1
        sta TNECHO
        bra @ydo
@w3:    cmp #3
        beq @ydo
        cmp #0
        bne @ndo
@ydo:   lda #$FD
        bra @send
@ndo:   lda #$FE
        bra @send
@do:    cmp #$FD                ; DO x
        bne @wo
        lda TNTMP
        cmp #0
        beq @will
        cmp #3
        beq @will
        cmp #24
        beq @will
        cmp #31
        bne @wont
        jsr tn_qbeg             ; WILL NAWS + Groesse 80 x 25
        lda #$FF
        jsr rq_c
        lda #$FB
        jsr rq_c
        lda #31
        jsr rq_c
        ldx #0
@nw:    lda tn_naws,x
        and #$00FF
        phx
        jsr rq_c
        plx
        inx
        cpx #9
        bcc @nw
        jmp tn_qend
@will:  lda #$FB
        bra @send
@wont:  lda #$FC
        bra @send
@wo:    cmp #$FC                ; WONT Echo: selbst zeigen
        bne @r
        lda TNTMP
        cmp #1
        bne @r
        stz TNECHO
@r:     rts
@send:  pha                     ; IAC Antwort Option
        jsr tn_qbeg
        lda #$FF
        jsr rq_c
        pla
        jsr rq_c
        lda TNTMP
        jsr rq_c
        jmp tn_qend
@s3:    cpx #3                  ; SB: Option merken
        bne @s4
        sta TNSBO
        stz TNSBQ
        stz TNSBN
        stz TNTMP
        lda #4
        sta TNIAC
        rts
@s4:    sta TNB                 ; SB-Daten bis IAC SE
        cmp #$F0
        bne @d
        lda TNTMP
        cmp #$FF
        bne @d
        stz TNIAC
        lda TNSBQ               ; Terminaltyp erfragt: IAC SB 24 0 ANSI IAC SE
        beq @r4
        jsr tn_qbeg
        ldx #0
@tt:    lda tn_ttype,x
        and #$00FF
        phx
        jsr rq_c
        plx
        inx
        cpx #10
        bcc @tt
        jmp tn_qend
@d:     lda TNSBN               ; erstes Datenbyte: 1 = SEND
        bne @d2
        inc TNSBN
        lda TNSBO
        cmp #24
        bne @d2
        lda TNB
        cmp #1
        bne @d2
        inc TNSBQ
@d2:    lda TNB
        sta TNTMP
@r4:    rts

        .pushseg
        .segment "RODATA"
tn_naws:  .byte $FF,$FA,31,0,80,0,25,$FF,$F0
tn_ttype: .byte $FF,$FA,24,0,"ANSI",$FF,$F0
        .popseg

; Antworten waehrend des Verarbeitens sammeln, danach senden
tn_qbeg:
        lda TNPEND
        bne @c
        lda #WB_REQ
        sta UWR
        lda #1
        sta TNPEND
        rts
@c:     lda TNPU
        sta UWR
        rts
tn_qend:
        lda UWR
        sta TNPU
        rts
tn_flush:
        lda TNPEND
        beq @r
        stz TNPEND
        lda TNPU
        sta UWR
        jmp tn_out
@r:     rts

; WB_REQ..UWR senden
tn_out: lda UWR
        sec
        sbc #WB_REQ
        beq @r
        sta WCNT
        lda #WB_REQ
        sta WPTR
        JSLF xf_tcp_write
        jsr net_ptr0            ; tcp_write verstellt PSRC
@r:     rts

; ---------- Eingabezeile (WB_URL) senden ----------
tn_line:
        jsr net_ptr0
        lda TNON
        bne @go
        rts
@go:    cmp #2
        bne @tel
        jsr irc_cmd
        bra @clr
@tel:   lda #WB_REQ
        sta UWR
        ldx #WB_URL
        jsr rq_b
        lda #13
        jsr rq_c
        lda #10
        jsr rq_c
        jsr tn_out
        lda TNECHO              ; selbst zeigen, wenn die Gegenstelle es nicht tut
        bne @clr
        ldx #WB_URL
        jsr tn_pstr
        lda #$0A
        jsr tn_put
@clr:   jsr net_ptr0
        ldy #WB_URL
        lda #0
        BWR
        jsr tn_view
        CALL redraw_win         ; mit Adresszeile (jetzt leer)
        rts
; Pufferzeichenkette X anzeigen
tn_pstr:
        stx TNQ
        txy
        BRD
        sta TNI
@l:     lda TNI
        beq @r
        inc TNQ
        ldy TNQ
        BRD
        jsr tn_char
        dec TNI
        bra @l
@r:     rts

; ---------- IRC ----------
irc_hello:
        lda #WB_REQ             ; NICK GS95xxxx / USER
        sta UWR
        ldx #i_nick
        jsr rq_m
        jsr irc_mynick
        ldx #TNNICK
        jsr rq_m
        ldx #i_user
        jsr rq_m
        jsr tn_out
        ldy #WB_PATH            ; Kanal aus dem Pfad ("/kanal" oder "/#kanal")
        BRD
        stz TNCH
        cmp #2
        bcc @r
        dec a
        sta TNI
        ldy #WB_PATH+2
        BRD
        cmp #'#'
        beq @h
        lda #'#'
        jsr ch_c
@h:     ldx #0
@c:     cpx TNI
        bcs @r
        txa
        clc
        adc #WB_PATH+2
        tay
        BRD
        cmp #'/'
        beq @r
        phx
        jsr ch_c
        plx
        inx
        bra @c
@r:     rts
ch_c:   pha
        lda TNCH
        and #$00FF
        tax
        pla
        cpx #32
        bcs @r
        sep #$20
        sta TNCH+1,x
        inx
        txa
        sta TNCH
        rep #$20
@r:     rts
; Spitzname GS95 + 4 Ziffern (aus der Uhr)
irc_mynick:
        ldx #4
@c:     lda i_gs95,x
        sta TNNICK,x
        dex
        dex
        bpl @c
        JSLF xf_net_tick
        eor FRAMES
        sta TNTMP
        ldx #5
@d:     lda TNTMP
        and #7
        clc
        adc #'0'
        sep #$20
        sta TNNICK,x
        rep #$20
        lsr TNTMP
        lsr TNTMP
        lsr TNTMP
        inx
        cpx #9
        bcc @d
        rts

; ein Byte einer IRC-Zeile
irc_byte:
        cmp #13
        beq @r
        cmp #10
        beq @ln
        ldx TNLL
        cpx #500
        bcs @r
        pha
        txa
        clc
        adc #TN_IN
        tay
        pla
        BWR
        inc TNLL
@r:     rts
@ln:    jsr irc_line
        stz TNLL
        rts

; Zeichen der Zeile ab TNQ bis TNLL; A = Zeichen (C=1 am Ende)
il_get: lda TNQ
        cmp TNLL
        bcs @e
        clc
        adc #TN_IN
        tay
        BRD
        inc TNQ
        clc
        rts
@e:     sec
        rts
; bis zum naechsten Leerzeichen weiterlesen; Anfang -> TNI
il_skipw:
        lda TNQ
        sta TNI
@l:     jsr il_get
        bcs @r
        cmp #' '
        bne @l
@r:     rts
; Zeile TNI..TNQ-1 vergleichen mit Text X (Hauptbank). C=0 gleich
il_is:  stx TNTMP
        lda a:0,x
        and #$00FF
        sta TNFR
        ldy #0
@l:     cpy TNFR
        bcs @y
        tya
        clc
        adc TNI
        clc
        adc #TN_IN
        phy
        tay
        BRD
        ply
        sta TNX
        tyx
        inx
        txa
        clc
        adc TNTMP
        tax
        lda a:0,x
        and #$00FF
        cmp TNX
        bne @n
        iny
        bra @l
@y:     clc
        rts
@n:     sec
        rts
; Rest der Zeile ab A anzeigen
il_show:
        sta TNQ
@l:     jsr il_get
        bcs @r
        cmp #$80
        bcc @a
        jsr tn_hi
@a:     cmp #' '
        bcc @l
        jsr tn_put
        bra @l
@r:     rts
; Bereich TNI..A-1 anzeigen
il_rng: sta TNFR
        lda TNI
        sta TNQ
@l:     lda TNQ
        cmp TNFR
        bcs @r
        jsr il_get
        jsr tn_put
        bra @l
@r:     rts

; vollstaendige Zeile in TN_IN (TNLL Zeichen)
irc_line:
        stz TNQ
        jsr il_skipw            ; "PING :x" -> "PONG :x"
        ldx #i_ping
        jsr il_is
        bcs @np
        jsr tn_qbeg
        ldx #i_pong
        jsr rq_m
@pp:    jsr il_get
        bcs @pe
        jsr rq_c
        bra @pp
@pe:    lda #13
        jsr rq_c
        lda #10
        jsr rq_c
        jmp tn_qend
@np:    stz TNQ                 ; Absender (":nick!...") merken
        stz TNNE                 ; Ende des Spitznamens
        stz TNI
        lda TNLL
        bne *+5
        jmp @r0
        ldy #TN_IN
        BRD
        cmp #':'
        bne @cmd
        jsr il_skipw
        ldx #1                  ; Spitzname bis '!' oder Leerzeichen
@nk:    txa
        clc
        adc #TN_IN
        tay
        BRD
        cmp #'!'
        beq @nke
        cmp #' '
        beq @nke
        inx
        cpx TNLL
        bcc @nk
@nke:   stx TNNE
@cmd:   jsr il_skipw            ; Befehl
        lda TNI
        sta TNCMD
        lda TNQ
        sta TNAC                 ; nach dem Befehl
        ldx #i_001
        jsr il_is
        bcs @n1
        jsr irc_join
        bra @num
@n1:    ldx #i_priv
        jsr il_is
        bcc @msg
        ldx #i_note
        jsr il_is
        bcc @msg
        lda TNI                 ; Zahl? dann nur der Text nach " :"
        clc
        adc #TN_IN
        tay
        BRD
        cmp #'0'
        bcc @oth
        cmp #'9'+1
        bcs @oth
@num:   jsr il_colon
        bcs @r0
        jsr il_show
        lda #$0A
        jmp tn_put
@r0:    rts
@msg:   jsr il_skipw            ; Ziel
        jsr il_colon
        bcs @r0
        pha
        lda #'<'
        jsr tn_put
        lda #1
        sta TNI
        lda TNNE
        jsr il_rng
        lda #'>'
        jsr tn_put
        lda #' '
        jsr tn_put
        pla
        jsr il_show
        lda #$0A
        jmp tn_put
@oth:   lda #'*'                ; JOIN, PART, QUIT, NICK ...: "* nick BEFEHL rest"
        jsr tn_put
        lda #' '
        jsr tn_put
        lda TNNE
        beq @o2
        lda #1
        sta TNI
        lda TNNE
        jsr il_rng
        lda #' '
        jsr tn_put
@o2:    lda TNCMD               ; Befehl und Rest
        jsr il_show
        lda #$0A
        jmp tn_put
; ab TNQ " :" suchen -> A = Anfang des Texts. C=1 wenn keiner
il_colon:
        lda TNQ
        sta TNFR
@l:     lda TNFR
        cmp TNLL
        bcs @n
        clc
        adc #TN_IN
        tay
        BRD
        inc TNFR
        cmp #':'
        bne @l
        lda TNFR
        clc
        rts
@n:     sec
        rts
irc_join:
        lda TNCH
        and #$00FF
        beq @r
        jsr tn_qbeg
        ldx #i_join
        jsr rq_m
        ldx #TNCH
        jsr rq_m
        lda #13
        jsr rq_c
        lda #10
        jsr rq_c
        jsr tn_qend
@r:     rts

; Eingabe (WB_URL) als IRC-Befehl senden
irc_cmd:
        lda #WB_REQ
        sta UWR
        ldy #WB_URL
        BRD
        bne *+3
        rts
        sta TNI
        ldy #WB_URL+1
        BRD
        cmp #'/'
        beq @sl
        lda TNCH                ; Text an den Kanal
        and #$00FF
        bne *+3
        rts
        ldx #i_privs
        jsr rq_m
        ldx #TNCH
        jsr rq_m
        lda #' '
        jsr rq_c
        lda #':'
        jsr rq_c
        ldx #WB_URL
        jsr rq_b
        jsr @crlf
        lda #'<'                ; selbst zeigen
        jsr tn_put
        ldx #0
@nn:    lda TNNICK+1,x
        and #$00FF
        jsr tn_put
        inx
        cpx #8
        bcc @nn
        lda #'>'
        jsr tn_put
        lda #' '
        jsr tn_put
        ldx #WB_URL
        jsr tn_pstr
        lda #$0A
        jmp tn_put
@sl:    ldx #2                  ; "/befehl rest": Befehl gross
        stz TNFR                ; 1 = "/msg"
        ldy #WB_URL+2
        lda [PSRC],y
        ora #$2020
        cmp #$736D              ; "ms"
        bne @up
        iny
        iny
        BRD
        ora #$20
        cmp #'g'
        bne @up
        inc TNFR
        ldx #i_privs
        jsr rq_m
        ldx #6                  ; nach "/msg "
        bra @tg
@up:    cpx TNI
        beq @u1
        bcs @end
@u1:    txa
        clc
        adc #WB_URL
        tay
        BRD
        cmp #' '
        beq @rest
        cmp #'a'
        bcc @u2
        cmp #'z'+1
        bcs @u2
        and #$DF
@u2:    phx
        jsr rq_c
        plx
        inx
        bra @up
@rest:  phx                     ; Rest unveraendert
@rl:    plx
        cpx TNI
        beq @rl1
        bcs @end
@rl1:   txa
        clc
        adc #WB_URL
        tay
        BRD
        phx
        jsr rq_c
        plx
        inx
        phx
        bra @rl
@end:   jmp @crlf
@tg:    cpx TNI                 ; /msg: Ziel, dann " :" und Text
        beq @t1
        bcs @end
@t1:    txa
        clc
        adc #WB_URL
        tay
        BRD
        cmp #' '
        beq @tt
        phx
        jsr rq_c
        plx
        inx
        bra @tg
@tt:    phx
        lda #' '
        jsr rq_c
        lda #':'
        jsr rq_c
        plx
        inx
        phx
        bra @rl
@crlf:  lda #13
        jsr rq_c
        lda #10
        jsr rq_c
        jmp tn_out

        .pushseg
        .segment "RODATA"
i_nick:  .byte 5,"NICK "
i_user:  .byte i_user_e-i_user-1,13,10,"USER gsos95 0 * :GS/OS95 Apple IIgs",13,10
i_user_e:
i_gs95:  .byte 8,"GS95"
         .byte 0
i_ping:  .byte 4,"PING"
i_pong:  .byte 5,"PONG "
i_001:   .byte 3,"001"
i_priv:  .byte 7,"PRIVMSG"
i_note:  .byte 6,"NOTICE"
i_privs: .byte 8,"PRIVMSG "
i_join:  .byte 5,"JOIN "
        .popseg

; Eingabezeile ist eine Adresse ("xyz://..." oder "about:")? C=0 ja
tn_isurl:
        jsr net_ptr0
        ldy #WB_URL
        BRD
        sta TNFR
        ldx #1
@l:     cpx TNFR
        bcs @n
        cpx #10
        bcs @n
        txa
        clc
        adc #WB_URL
        tay
        BRD
        cmp #':'
        beq @c
        ora #$20
        cmp #'a'
        bcc @n
        cmp #'z'+1
        bcs @n
        inx
        bra @l
@c:     cpx #6                  ; "about:"
        bne @s
        ldy #WB_URL+1
        lda [PSRC],y
        ora #$2020
        cmp #$6261              ; "ab"
        beq @y
@s:     iny
        BRD
        cmp #'/'
        bne @n
        iny
        BRD
        cmp #'/'
        bne @n
@y:     clc
        rts
@n:     sec
        rts
