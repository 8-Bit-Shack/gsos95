; ================================================================
; GS/OS95 Browser: Mailbox-Telefonbuch (about:bbs)
; Teil von web.s (XCODE2). Eintraege in NPREF+1024 (in GSOS95.NET):
;   8 x 64 Bytes: +0 Name (Laenge + bis 23 Zeichen), +24 Rechner (Laenge +
;   bis 33 Zeichen), +58 Port (Wort). Name-Laenge 0 = frei.
;   NPREF+1534 "B1": Liste angelegt (sonst Voreinstellung mit zwei Mailboxen).
; Adressen:
;   about:bbs              Seite
;   about:bbseN            Seite mit Eintrag N (1..8) zum Aendern
;   about:bbsdN            Eintrag N loeschen
;   about:bbsc?h=..&p=..   direkt verbinden (telnet://rechner:port/)
;   about:bbss?i=N&n=..&h=..&p=..   speichern (i=0: neuer Eintrag)
; ================================================================
BBS0    = NPREF+1024
BBSN    = 8
BBSL    = 64
BBSMAG  = NPREF+1534

        .pushseg
        .segment "BSS"
BBSGO:  .res 2                  ; 1: WB_TURL neu laden (telnet://...)
BBSE:   .res 2                  ; Eintrag in Bearbeitung (1..8, 0 = neu)
BBSI:   .res 2
BBSA:   .res 2                  ; Adresse des Eintrags
BQK:    .res 2
BQE:    .res 2
BQN:    .res 2
BBSP:   .res 2                  ; Port
BQO:    .res 2                  ; Ausgabe fuer dec_out
BQD:    .res 2
BBSQ:   .res 42                 ; Wert aus der Abfrage (Laenge + Zeichen)
BBSH:   .res 42                 ; Rechner
BBSNM:  .res 42                 ; Name
        .popseg

; Eintrag A (0..7) -> X = Adresse
bbs_addr:
        and #$00FF
        asl
        asl
        asl
        asl
        asl
        asl
        clc
        adc #BBS0
        tax
        rts

; Liste beim ersten Mal mit den Voreinstellungen fuellen
bbs_init:
        lda BBSMAG
        cmp #$3142              ; "B1"
        beq @r
        ldx #BBSN*BBSL-2
@z:     stz BBS0,x
        dex
        dex
        bpl @z
        ldx #0
@c:     lda bbs_def,x
        sta BBS0,x
        inx
        inx
        cpx #2*BBSL
        bcc @c
        lda #$3142
        sta BBSMAG
@r:     rts

; ---------- about:bbs... ----------
; A = Befehlsbuchstabe (0 = keiner). C=0 zurueck (Seite im Rohpuffer
; oder BBSGO gesetzt)
bbs_cmd:
        sta BQK
        jsr bbs_init
        stz BBSE
        lda BQK
        cmp #'c'
        bne @n1
        jmp bbs_conn
@n1:    cmp #'s'
        bne @n2
        jmp bbs_save
@n2:    cmp #'d'
        bne @n3
        jsr bbs_num
        bcs @pg
        jsr bbs_addr
        sep #$20
        stz a:0,x               ; Name leer = frei
        rep #$20
        JSLF xf_net_save
        bra @pg
@n3:    cmp #'e'
        bne @pg
        jsr bbs_num
        bcs @pg
        inc a
        sta BBSE
@pg:    jmp bbs_page

; Ziffer an WB_TURL+11 -> A = 0..7, C=1 wenn ungueltig
bbs_num:
        ldy #WB_TURL
        BRD
        cmp #11
        bcc @no
        ldy #WB_TURL+11
        BRD
        sec
        sbc #'1'
        cmp #BBSN
        bcs @no
        clc
        rts
@no:    sec
        rts

; Wert zu Schluessel A aus der Abfrage in WB_TURL -> BBSQ. C=1: nicht da
q_val:  sta BQK
        sep #$20
        stz BBSQ
        rep #$20
        ldy #WB_TURL
        BRD
        clc
        adc #WB_TURL+1
        sta BQE
        ldy #WB_TURL+1
@s:     cpy BQE
        bcc *+5
        jmp @no
        BRD
        iny
        cmp #'?'
        beq @k
        cmp #'&'
        bne @s
@k:     cpy BQE
        bcc *+5
        jmp @no
        BRD
        ora #$20
        cmp BQK
        bne @s
        iny
        BRD
        cmp #'='
        bne @s
        iny
@c:     cpy BQE
        bcs @ok
        BRD
        cmp #'&'
        beq @ok
        cmp #'+'
        bne @n1
        lda #' '
        bra @put
@n1:    cmp #'%'
        bne @put
        iny
        BRD
        jsr q_hexv
        asl
        asl
        asl
        asl
        sta BQN
        iny
        BRD
        jsr q_hexv
        ora BQN
@put:   cmp #' '                ; Steuerzeichen weg
        bcc @sk
        pha
        lda BBSQ
        and #$00FF
        tax
        pla
        cpx #40
        bcs @sk
        phy
        sep #$20
        sta BBSQ+1,x
        inc BBSQ
        rep #$20
        ply
@sk:    iny
        bra @c
@ok:    lda BBSQ                ; Leerzeichen hinten weg
        and #$00FF
        beq @e
        tax
        lda BBSQ,x
        and #$00FF
        cmp #' '
        bne @e
        sep #$20
        dec BBSQ
        rep #$20
        bra @ok
@e:     clc
        rts
@no:    sec
        rts
; Hexziffer -> 0..15
q_hexv: and #$00FF
        cmp #'a'
        bcc @u
        sbc #$20
@u:     cmp #'A'
        bcc @d
        sbc #'A'-10
        and #$000F
        rts
@d:     sec
        sbc #'0'
        and #$000F
        rts

; BBSQ -> Zahl in A (Port), 0 wenn keine
q_num:  stz BQN
        lda BBSQ
        and #$00FF
        sta BQE
        ldx #0
@l:     cpx BQE
        bcs @e
        lda BBSQ+1,x
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcs @e
        pha
        lda BQN                 ; *10
        asl
        sta BQK
        asl
        asl
        clc
        adc BQK
        sta BQN
        pla
        clc
        adc BQN
        sta BQN
        inx
        bra @l
@e:     lda BQN
        rts

; BBSQ (Laenge + Zeichen) nach X kopieren
q_copy: stx BQE
        lda BBSQ
        and #$00FF
        sta BQN
        ldy #0
@l:     lda BBSQ,y
        phy
        pha
        tya
        clc
        adc BQE
        tax
        pla
        sep #$20
        sta a:0,x
        rep #$20
        ply
        iny
        cpy BQN
        beq @l
        bcc @l
        rts

; ---------- direkt verbinden ----------
bbs_conn:
        lda #'h'
        jsr q_val
        bcs @pg
        lda BBSQ
        and #$00FF
        beq @pg
        ldx #BBSH
        jsr q_copy
        lda #'p'
        jsr q_val
        jsr q_num
        bne @p
        lda #23
@p:     sta BBSP
        jsr bbs_go
        clc
        rts
@pg:    jmp bbs_page

; telnet://BBSH:BBSP/ nach WB_TURL, BBSGO setzen
bbs_go: jsr net_ptr0
        jsr tb_clr
        ldx #0
@t:     lda w_bbtel+1,x
        and #$00FF
        phx
        jsr tb_c
        plx
        inx
        cpx #9                  ; "telnet://"
        bcc @t
        lda BBSH
        and #$00FF
        sta BQN
        ldx #0
@h:     cpx BQN
        bcs @hp
        lda BBSH+1,x
        and #$00FF
        phx
        jsr tb_c
        plx
        inx
        bra @h
@hp:    lda #':'
        jsr tb_c
        lda BBSP
        ldx #1                  ; an WB_TURL
        jsr dec_out
        lda #'/'
        jsr tb_c
        lda #1
        sta BBSGO
        rts

; A dezimal (ohne fuehrende Nullen); X = 0: raw_c, 1: tb_c
dec_out:
        sta BQN
        stx BQO
        stz BQD
        ldx #0
@d:     lda bbs_pow,x
        sta BQE
        ldy #0
        lda BQN
@s:     cmp BQE
        bcc @o
        sbc BQE
        iny
        bra @s
@o:     sta BQN
        tya
        ora BQD
        bne @w
        cpx #8                  ; letzte Stelle immer
        bne @nx
@w:     tya
        clc
        adc #'0'
        phx
        jsr @out
        plx
        lda #1
        sta BQD
@nx:    inx
        inx
        cpx #10
        bcc @d
        rts
@out:   ldx BQO
        bne @tb
        jmp raw_c
@tb:    jmp tb_c

; ---------- speichern ----------
bbs_save:
        lda #'h'
        jsr q_val
        bcs @pg
        lda BBSQ
        and #$00FF
        bne @h
@pg:    jmp bbs_page
@h:     ldx #BBSH
        jsr q_copy
        lda #'n'
        jsr q_val
        lda BBSQ
        and #$00FF
        bne @n
        ldx #0                  ; kein Name: Rechner
@cp:    lda BBSH,x
        sta BBSNM,x
        inx
        inx
        cpx #42
        bcc @cp
        bra @n2
@n:     ldx #BBSNM
        jsr q_copy
@n2:    lda #'p'
        jsr q_val
        jsr q_num
        bne @p
        lda #23
@p:     sta BBSP
        lda #'i'                ; Nummer: 1..8 aendern, sonst neu
        jsr q_val
        jsr q_num
        dec a
        cmp #BBSN
        bcc @a
        lda #0                  ; freien Platz suchen
@f:     sta BBSI
        jsr bbs_addr
        lda a:0,x
        and #$00FF
        beq @a2
        lda BBSI
        inc a
        cmp #BBSN
        bcc @f
        stz BBSI
        jmp bbs_page            ; voll
@a:     sta BBSI
@a2:    lda BBSI
        jsr bbs_addr
        stx BBSA
        ldy #0                  ; Name (bis 23)
        lda BBSNM
        and #$00FF
        cmp #24
        bcc @nl
        lda #23
@nl:    sta BQN
        sep #$20
        sta a:0,x
        rep #$20
@nc:    cpy BQN
        bcs @hn
        lda BBSNM+1,y
        iny
        phx
        pha
        tya
        clc
        adc BBSA
        tax
        pla
        sep #$20
        sta a:0,x
        rep #$20
        plx
        bra @nc
@hn:    lda BBSA                ; Rechner (bis 33) an +24
        clc
        adc #24
        sta BBSA
        lda BBSH
        and #$00FF
        cmp #34
        bcc @hl
        lda #33
@hl:    sta BQN
        ldx BBSA
        sep #$20
        sta a:0,x
        rep #$20
        ldy #0
@hc:    cpy BQN
        bcs @pt
        lda BBSH+1,y
        iny
        pha
        tya
        clc
        adc BBSA
        tax
        pla
        sep #$20
        sta a:0,x
        rep #$20
        bra @hc
@pt:    lda BBSA                ; Port an +58
        clc
        adc #34
        tax
        lda BBSP
        sta a:0,x
        stz BBSI
        JSLF xf_net_save
        jmp bbs_page

; ---------- Seite ----------
; Adresszeile wird about:bbs; BBSE = Eintrag zum Aendern (0 = neu)
bbs_page:
        jsr net_ptr0
        ldx #0
        ldy #WB_TURL
@u:     lda w_bbu,x
        BWR
        iny
        inx
        cpx #10
        bcc @u
        ldx #WB_TURL
        ldy #WB_URL
        jsr bs_copy
        stz HO
        lda #w_fv1              ; <title>
        jsr raw_m
        WS ws_bbtit
        jsr raw_x
        lda #w_fv2              ; </title><h1>
        jsr raw_m
        WS ws_bbtit
        jsr raw_x
        lda #w_bb1              ; </h1><p>
        jsr raw_m
        WS ws_bbhelp
        jsr raw_x
        lda #w_bb2              ; </p><ul>
        jsr raw_m
        stz BBSI
        stz BQK                 ; Anzahl
@l:     lda BBSI
        jsr bbs_addr
        stx BBSA
        lda a:0,x
        and #$00FF
        bne *+5
        jmp @nx
        inc BQK
        lda #w_bb3              ; <li><a href="telnet://
        jsr raw_m
        jsr bbs_host
        lda #':'
        jsr raw_c
        jsr bbs_port
        lda #w_bb4              ; /">
        jsr raw_m
        lda BBSA
        jsr bbs_esc             ; Name
        lda #w_bb5              ; </a> (
        jsr raw_m
        jsr bbs_host
        lda #':'
        jsr raw_c
        jsr bbs_port
        lda #w_bb6              ; ) &middot; <a href="about:bbse
        jsr raw_m
        jsr bbs_dig
        lda #w_fv5              ; ">
        jsr raw_m
        WS ws_bbchg
        jsr raw_x
        lda #w_bb7              ; </a> &middot; <a href="about:bbsd
        jsr raw_m
        jsr bbs_dig
        lda #w_fv5
        jsr raw_m
        WS ws_bbdel
        jsr raw_x
        lda #w_fv8              ; </a>
        jsr raw_m
@nx:    inc BBSI
        lda BBSI
        cmp #BBSN
        bcs *+5
        jmp @l
        lda BQK
        bne @hv
        lda #w_fv9              ; <li>
        jsr raw_m
        WS ws_bbnone
        jsr raw_x
@hv:    lda #w_bb8              ; </ul><hr><h3>
        jsr raw_m
        WS ws_bbdir
        jsr raw_x
        lda #w_bb9              ; </h3><form action="about:bbsc">
        jsr raw_m
        WS ws_bbhost
        jsr raw_x
        lda #w_bba              ; <input name=h size=24>
        jsr raw_m
        WS ws_bbport
        jsr raw_x
        lda #w_bbb              ; <input name=p size=5 value=23> <input type=submit value="
        jsr raw_m
        WS ws_bbgo
        jsr raw_x
        lda #w_bbc              ; "></form><hr><h3>
        jsr raw_m
        lda BBSE
        beq @new
        WS ws_bbedit
        bra @t
@new:   WS ws_bbnew
@t:     jsr raw_x
        lda #w_bbd              ; </h3><form action="about:bbss"><input type=hidden name=i value=
        jsr raw_m
        lda BBSE
        clc
        adc #'0'
        jsr raw_c
        lda #'>'
        jsr raw_c
        WS ws_bbname
        jsr raw_x
        lda #w_bbe              ; <input name=n size=20 value="
        jsr raw_m
        lda BBSE
        beq @e1
        dec a
        jsr bbs_addr
        stx BBSA
        txa
        jsr bbs_esc
@e1:    lda #w_bbf              ; "><br>
        jsr raw_m
        WS ws_bbhost
        jsr raw_x
        lda #w_bbg              ; <input name=h size=24 value="
        jsr raw_m
        lda BBSE
        beq @e2
        jsr bbs_hostv
@e2:    lda #w_bbh              ; ">
        jsr raw_m
        WS ws_bbport
        jsr raw_x
        lda #w_bbi              ; <input name=p size=5 value="
        jsr raw_m
        lda BBSE
        bne @e3
        lda #23
        jsr raw_num0
        bra @e4
@e3:    jsr bbs_port
@e4:    lda #w_bbj              ; "><br><input type=submit value="
        jsr raw_m
        WS ws_bbsave
        jsr raw_x
        lda #w_bbk              ; "></form>
        jsr raw_m
        clc
        rts

; Ziffer BBSI+1
bbs_dig:
        lda BBSI
        clc
        adc #'1'
        jmp raw_c
; Rechner des Eintrags BBSA (fuer href) / mit Schutz (fuer value)
bbs_host:
        lda BBSA
        clc
        adc #24
        jmp raw_m
bbs_hostv:
        lda BBSA
        clc
        adc #24
        jmp bbs_esc
; Port des Eintrags BBSA dezimal
bbs_port:
        lda BBSA
        clc
        adc #58
        tax
        lda a:0,x
raw_num0:                       ; A dezimal ohne fuehrende Nullen
        ldx #0                  ; in den Rohpuffer
        jmp dec_out
; Text (Laenge + Zeichen) an A mit " < & geschuetzt
bbs_esc:
        sta BQE
        tax
        lda a:0,x
        and #$00FF
        sta BQN
@l:     lda BQN
        beq @r
        inc BQE
        ldx BQE
        lda a:0,x
        and #$00FF
        cmp #$22
        beq @q
        cmp #'<'
        beq @lt
        cmp #'&'
        beq @am
        jsr raw_c
        bra @n
@q:     lda #w_bbq
        bra @m
@lt:    lda #w_bbl
        bra @m
@am:    lda #w_bba2
@m:     jsr raw_m
@n:     dec BQN
        bra @l
@r:     rts

        .pushseg
        .segment "RODATA"
w_bbu:  .byte 9,"about:bbs"
w_bbtel: .byte 9,"telnet://"
bbs_pow: .word 10000,1000,100,10,1
w_bb1:  .byte w_bb1e-w_bb1-1,"</h1><p>"
w_bb1e:
w_bb2:  .byte w_bb2e-w_bb2-1,"</p><ul>"
w_bb2e:
w_bb3:  .byte w_bb3e-w_bb3-1,"<li><a href=",$22,"telnet://"
w_bb3e:
w_bb4:  .byte w_bb4e-w_bb4-1,"/",$22,">"
w_bb4e:
w_bb5:  .byte w_bb5e-w_bb5-1,"</a> ("
w_bb5e:
w_bb6:  .byte w_bb6e-w_bb6-1,") &middot; <a href=",$22,"about:bbse"
w_bb6e:
w_bb7:  .byte w_bb7e-w_bb7-1,"</a> &middot; <a href=",$22,"about:bbsd"
w_bb7e:
w_bb8:  .byte w_bb8e-w_bb8-1,"</ul><hr><h3>"
w_bb8e:
w_bb9:  .byte w_bb9e-w_bb9-1,"</h3><form action=",$22,"about:bbsc",$22,">"
w_bb9e:
w_bba:  .byte w_bbae-w_bba-1," <input name=h size=24> "
w_bbae:
w_bbb:  .byte w_bbbe-w_bbb-1," <input name=p size=5 value=23> <input type=submit value=",$22
w_bbbe:
w_bbc:  .byte w_bbce-w_bbc-1,$22,"></form><hr><h3>"
w_bbce:
w_bbd:  .byte w_bbde-w_bbd-1,"</h3><form action=",$22,"about:bbss",$22,"><input type=hidden name=i value="
w_bbde:
w_bbe:  .byte w_bbee-w_bbe-1," <input name=n size=20 value=",$22
w_bbee:
w_bbf:  .byte w_bbfe-w_bbf-1,$22,"><br>"
w_bbfe:
w_bbg:  .byte w_bbge-w_bbg-1," <input name=h size=24 value=",$22
w_bbge:
w_bbh:  .byte w_bbhe-w_bbh-1,$22,"> "
w_bbhe:
w_bbi:  .byte w_bbie-w_bbi-1," <input name=p size=5 value=",$22
w_bbie:
w_bbj:  .byte w_bbje-w_bbj-1,$22,"><br><input type=submit value=",$22
w_bbje:
w_bbk:  .byte w_bbke-w_bbk-1,$22,"></form>"
w_bbke:
w_bbq:  .byte 6,"&quot;"
w_bbl:  .byte 4,"&lt;"
w_bba2: .byte 5,"&amp;"
; Voreinstellung: zwei Mailboxen (je 64 Bytes)
bbs_def:
        .byte 18,"Captain's Quarters"
        .res 24-19
        .byte 14,"cqbbs.ddns.net"
        .res 34-15
        .word 6502
        .res 64-60
        .byte 18,"Electronic Chicken"
        .res 24-19
        .byte 25,"bbs.electronicchicken.com"
        .res 34-26
        .word 23
        .res 64-60
        .popseg
