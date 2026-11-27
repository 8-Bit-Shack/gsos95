; ================================================================
; GS/OS95 Browser: FTP (ftp://), Teil von XCODE4
; Uthernet II: Steuerverbindung auf Socket 0, Daten (PASV) auf Socket 1.
; FujiNet: N:FTP:// erledigt Anmeldung und Datenverbindung selbst.
; Andere Karten: Hinweisseite.
; Verzeichnisse werden als Seite mit Links gezeigt, Dateien landen
; wie andere Downloads in *:Downloads.
; ================================================================
        .pushseg
        .segment "BSS"
FTDIR:  .res 2                  ; 1 = Verzeichnis
FTC:    .res 2                  ; Zahl der letzten Antwort
FTL:    .res 2                  ; Anfang der Antwortzeile
FTQ:    .res 2                  ; Suchstelle
FTE:    .res 2                  ; Ende der Antwortzeile (0 = keine)
FTT0:   .res 2
FTK:    .res 2
FTN:    .res 2
FTI:    .res 2                  ; Liste: Zeilenanfang
FTS:    .res 2                  ; Name: Anfang
FTZ:    .res 2                  ; Groesse: Anfang / Ende
FTZE:   .res 2
FTLE:   .res 2                  ; Ende der Liste
FTDF:   .res 2                  ; Eintrag ist ein Verzeichnis
FTX:    .res 2                  ; Zeilenende (ohne CR)
        .popseg
FT_RB = WB_REQ+$100             ; Antworten der Steuerverbindung

ft_ptr: lda NWB
        sta PSRC+2
        stz PSRC
        rts

; Pfad leer oder mit / am Ende -> FTDIR = 1
ft_isdir:
        stz FTDIR
        ldy #WB_PATH
        lda [PSRC],y
        and #$00FF
        beq @d
        clc
        adc #WB_PATH
        tay
        lda [PSRC],y
        and #$00FF
        cmp #'/'
        bne @r
@d:     lda #1
        sta FTDIR
@r:     rts

; ---------- vor dem Verbinden: Karte pruefen. C=1: Seite steht (HR/HE) ----------
xf_ftp_pre:
        jsr h_begin
        stz FTE
        stz NSOCK
        jsr ft_isdir
        lda NCARD
        cmp #1                  ; Uthernet II
        beq @ok
        cmp #3                  ; FujiNet
        bne @no
        lda FTDIR
        inc a
        sta FNFTP
@ok:    clc
        rtl
@no:    lda #ft_snc
        ldx #ft_snc_en
        jsr ft_errp
        sec
        rtl

; Fehlerseite ab $4000: Text A (Deutsch) / X (Englisch), dazu die Serverzeile
ft_errp:
        phx
        pha
        jsr h_begin
        lda #$4000
        sta HO
        lda #ft_eh
        jsr h_z
        pla
        plx
        jsr h_zl
        lda FTE
        beq ft_fin
        lda #ft_br
        jsr h_z
        ldy FTL
@l:     cpy FTE
        bcs ft_fin
        lda [PSRC],y
        and #$00FF
        cmp #' '
        bcc @n
        phy
        jsr h_esc
        ply
@n:     iny
        bra @l
ft_fin: lda HO
        sta HE
        lda #$4000
        sta HR
        lda #$FFFF
        rts

; ---------- Steuerverbindung steht: anmelden, Datenverbindung, LIST/RETR ----------
; C=0: Daten kommen auf der aktuellen Verbindung (NSOCK). C=1: A=$FFFF Seite steht,
; sonst Netzfehler (NERR)
xf_ftp_run:
        jsr ft_ptr
        stz FTE
        stz NSOCK
        lda NCARD
        cmp #3
        bne @w
        clc
        rtl
@w:     jsr ft_rd               ; Begruessung (220)
        bcc *+5
        jmp @ne
        cmp #400
        bcc *+5
        jmp @se
        ldx #ft_cuser
        jsr ft_cmd0
        bcc *+5
        jmp @ne
        cmp #300
        bcc @in                 ; 230: ohne Passwort
        cmp #400
        bcc *+5
        jmp @se
        ldx #ft_cpass
        jsr ft_cmd0
        bcc *+5
        jmp @ne
        cmp #400
        bcs @se
@in:    ldx #ft_ctype           ; binaer
        jsr ft_cmd0
        bcc *+5
        jmp @ne
        ldy #WB_PATH            ; Verzeichnis? (Wurzel: nicht noetig)
        lda [PSRC],y
        and #$00FF
        cmp #2
        bcc @pasv
        ldx #ft_ccwd
        jsr ft_cmdp
        bcs @ne
        cmp #300
        bcs @nd
        lda #1
        sta FTDIR
        bra @pasv
@nd:    lda FTDIR               ; als Verzeichnis gemeint, gibt es aber nicht
        bne @se
@pasv:  ldx #ft_cpasv
        jsr ft_cmd0
        bcs @ne
        cmp #227
        bne @se
        jsr ft_pasv
        bcs @se
        lda #1                  ; Datenverbindung auf Socket 1
        sta NSOCK
        JSLF xf_tcp_open
        bcs @ne                 ; (schliesst beide)
        stz NSOCK
        jsr ft_ptr
        lda FTDIR
        beq @rt
        ldx #ft_clist
        jsr ft_cmd0
        bra @l2
@rt:    ldx #ft_cretr
        jsr ft_cmdp
@l2:    php
        ldx #1
        stx NSOCK
        plp
        bcs @ne
        cmp #400
        bcs @se
        clc
        rtl
@se:    JSLF xf_tcp_close
        jsr ft_ptr
        lda #ft_sse
        ldx #ft_sse_en
        jsr ft_errp
        sec
        rtl
@ne:    lda #0
        sec
        rtl

; Befehl X (Text dieser Bank) [+ Pfad] + CRLF senden, Antwort lesen -> A = Zahl
ft_cmdp:
        lda #1
        bra ft_c
ft_cmd0:
        lda #0
ft_c:   sta FTK
        txa
        jsr x4_kb
        jsr ft_ptr
        ldx #WB_REQ
        ldy #0
@m:     lda [LP2],y
        and #$00FF
        beq @p
        jsr ft_put
        iny
        bra @m
@p:     lda FTK
        beq @e
        ldy #WB_PATH            ; Pfad, %xx entschluesselt
        lda [PSRC],y
        and #$00FF
        clc
        adc #WB_PATH+1
        sta FTN
        ldy #WB_PATH+1
@pl:    cpy FTN
        bcs @e
        lda [PSRC],y
        and #$00FF
        cmp #'%'
        bne @pc
        iny
        jsr ft_hex
        asl
        asl
        asl
        asl
        sta FTZ
        iny
        jsr ft_hex
        ora FTZ
@pc:    jsr ft_put
        iny
        bra @pl
@e:     lda #13
        jsr ft_put
        lda #10
        jsr ft_put
        txa
        sec
        sbc #WB_REQ
        sta WCNT
        lda #WB_REQ
        sta WPTR
        JSLF xf_tcp_write
        bcc ft_rd
        rts
ft_hex: lda [PSRC],y
        and #$00FF
        ora #$20
        cmp #'a'
        bcc @d
        sbc #'a'-10
        and #$0F
        rts
@d:     and #$0F
        rts
ft_put: phy
        txy
        sep #$20
        sta [PSRC],y
        rep #$20
        ply
        inx
        rts

; Antwort lesen bis zur Zeile "nnn Text". A = FTC = nnn, Zeile FTL..FTE. C=1 Netzfehler
ft_rd:  jsr ft_ptr
        lda #FT_RB
        sta FTL
        sta FTQ
        sta WPTR
        stz FTE
        lda #FT_RB+$F0
        sta NLIM
        jsr ft_t0
@l:     jsr ft_ptr
        ldy FTQ
@s:     cpy WPTR
        bcs @poll
        lda [PSRC],y
        and #$00FF
        cmp #10
        beq @eol
        iny
        bra @s
@eol:   sty FTQ
        tya
        sec
        sbc FTL
        cmp #4
        bcc @skip
        ldy FTL
        jsr ft_num3
        bcs @skip
        sta FTC
        ldy FTL
        iny
        iny
        iny
        lda [PSRC],y
        and #$00FF
        cmp #' '
        bne @skip
        lda FTQ
        sta FTE
        lda FTC
        clc
        rts
@skip:  ldy FTQ                 ; Zeile verwerfen, Rest nach vorn
        iny
        ldx FTL
@mv:    cpy WPTR
        bcs @mvd
        lda [PSRC],y
        jsr ft_put
        iny
        bra @mv
@mvd:   stx WPTR
        lda FTL
        sta FTQ
        bra @l
@poll:  sty FTQ
        lda WPTR
        cmp NLIM
        bcc @p1
        lda FTL                 ; zu lange Zeile: kuerzen
        clc
        adc #4
        sta WPTR
        sta FTQ
@p1:    lda WPTR
        sta FTK
        JSLF xf_tcp_poll
        bcc @p2
        lda WPTR                ; Verbindung zu
        cmp FTK
        beq *+5
        jmp @l
        lda #4                  ; NE_CONN
        sta NERR
        sec
        rts
@p2:    lda WPTR
        cmp FTK
        beq *+5
        jmp @l
        jsr ft_el
        cmp #1200               ; 20 s
        bcs *+5
        jmp @l
        lda #5                  ; NE_TIME
        sta NERR
        sec
        rts

; drei Ziffern ab Y -> A. C=1 keine Zahl
ft_num3:
        stz FTN
        ldx #3
@d:     lda [PSRC],y
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcs @b
        sta FTK
        lda FTN
        asl
        asl
        adc FTN
        asl
        adc FTK
        sta FTN
        iny
        dex
        bne @d
        lda FTN
        clc
        rts
@b:     sec
        rts

ft_t0:  pha
        pha
        TOOL $2503              ; GetTick
        pla
        sta FTT0
        pla
        rts
ft_el:  pha
        pha
        TOOL $2503
        pla
        sec
        sbc FTT0
        tax
        pla
        txa
        rts

; "227 ... (h1,h2,h3,h4,p1,p2)" -> DPORT. Die Adresse bleibt die des Servers
; (hinter NAT steht dort oft eine innere Adresse). C=1 unlesbar
ft_pasv:
        ldy FTL
        iny
        iny
        iny
        iny
        ldx #0
@sk:    cpy FTE
        bcs @bad
        lda [PSRC],y
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcc @dg
        iny
        bra @sk
@dg:    stz FTN
@dl:    lda [PSRC],y
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcs @de
        sta FTK
        lda FTN
        asl
        asl
        adc FTN
        asl
        adc FTK
        sta FTN
        iny
        bra @dl
@de:    lda FTN
        cpx #4
        bcc @nx
        bne @lo
        xba
        and #$FF00
        sta DPORT
@nx:    inx
        bra @sk
@lo:    and #$00FF
        ora DPORT
        sta DPORT
        clc
        rts
@bad:   sec
        rts

; ---------- Verzeichnisliste (0..WPTR) als Seite ab $4000 ----------
xf_ftp_list:
        jsr h_begin
        lda WPTR
        cmp #$3F00
        bcc @c
        lda #$3F00
@c:     sta FTLE
        lda #$4000
        sta HO
        lda #ft_t1
        jsr h_z
        jsr ft_hp
        lda #ft_t2
        jsr h_z
        jsr ft_hp
        lda #ft_t3
        jsr h_z
        ldy #WB_PATH            ; eine Ebene hoeher
        lda [PSRC],y
        and #$00FF
        cmp #2
        bcc @np
        clc
        adc #WB_PATH
        tay
        lda [PSRC],y
        and #$00FF
        cmp #'/'
        bne @b
        dey
@b:     cpy #WB_PATH+2
        bcc @k
        lda [PSRC],y
        and #$00FF
        cmp #'/'
        beq @k
        dey
        bra @b
@k:     sty FTK
        lda #ft_aup0
        jsr h_z
        lda #ft_sftp
        jsr h_z
        ldy #WB_HOST
        jsr ft_cs
        ldy #WB_PATH+1
@pc:    lda [PSRC],y
        and #$00FF
        phy
        jsr h_c
        ply
        cpy FTK
        bcs @pd
        iny
        bra @pc
@pd:    lda #ft_aup
        jsr h_z
@np:    stz FTI
@ln:    ldy FTI
        cpy FTLE
        bcs @end
@eo:    cpy FTLE
        bcs @ee
        lda [PSRC],y
        and #$00FF
        cmp #10
        beq @ee
        iny
        bra @eo
@ee:    sty FTE
        jsr ft_line
        ldy FTE
        iny
        sty FTI
        bra @ln
@end:   stz FTE
        jsr ft_fin
        rtl

; Rechner und Pfad (geschuetzt)
ft_hp:  ldy #WB_HOST
        jsr ft_cs
        ldy #WB_PATH
; Zeichenkette (Laenge + Text) im Puffer ab Y, geschuetzt
ft_cs:  lda [PSRC],y
        and #$00FF
        sta FTN
        iny
@l:     lda FTN
        beq @r
        lda [PSRC],y
        and #$00FF
        phy
        jsr h_esc
        ply
        iny
        dec FTN
        bra @l
@r:     rts

; Adresse des Verzeichnisses: ftp://Rechner/Pfad/
ft_base:
        lda #ft_sftp
        jsr h_z
        ldy #WB_HOST
        jsr ft_cs
        ldy #WB_PATH
        jsr ft_cs
        ldy #WB_PATH
        lda [PSRC],y
        and #$00FF
        beq @s
        clc
        adc #WB_PATH
        tay
        lda [PSRC],y
        and #$00FF
        cmp #'/'
        beq @r
@s:     lda #'/'
        jmp h_c
@r:     rts

; Y bis FTX: Leerzeichen, dann X Woerter (je mit Leerzeichen danach) ueberspringen
ft_skipf:
@f:     jsr ft_sp
        cpx #0
        beq @r
@w:     cpy FTX
        bcs @r
        lda [PSRC],y
        and #$00FF
        cmp #' '
        beq @nx
        iny
        bra @w
@nx:    dex
        bra @f
@r:     rts
ft_sp:  cpy FTX
        bcs @r
        lda [PSRC],y
        and #$00FF
        cmp #' '
        bne @r
        iny
        bra ft_sp
@r:     rts
; Y bis zum naechsten Leerzeichen
ft_word:
        cpy FTX
        bcs @r
        lda [PSRC],y
        and #$00FF
        cmp #' '
        beq @r
        iny
        bra ft_word
@r:     rts

; eine Zeile FTI..FTE der Liste als Link
ft_line:
        ldy FTE
        sty FTX
        cpy FTI
        beq @r0
        dey
        lda [PSRC],y
        and #$00FF
        cmp #13
        bne @e
        sty FTX
@e:     lda FTX
        sec
        sbc FTI
        bne @go
@r0:    rts
@go:    stz FTDF
        stz FTZ
        ldy FTI                 ; Unix nur mit "?rw" bzw. "?r-", "?--" am Anfang
        iny
        lda [PSRC],y
        and #$00FF
        cmp #'r'
        beq @u2
        cmp #'-'
        bne @ct
@u2:    iny
        lda [PSRC],y
        and #$00FF
        cmp #'w'
        beq @u3
        cmp #'-'
        bne @ct
@u3:    ldy FTI
        lda [PSRC],y
        and #$00FF
        cmp #'d'
        bne *+5
        jmp @ux
        cmp #'l'
        beq @ux
        cmp #'-'
        beq @ux0
@ct:    ldy FTI
        lda [PSRC],y
        and #$00FF
        cmp #'t'                ; "total 123"
        bne @nt
        ldy FTI
        iny
        iny
        iny
        iny
        iny
        lda [PSRC],y
        and #$00FF
        cmp #' '
        beq @r0
@nt:    ldy FTI
        lda [PSRC],y
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcs @pl
        ; DOS/IIS: Datum Zeit <DIR>|Groesse Name
        ldy FTI
        ldx #2
        jsr ft_skipf
        lda [PSRC],y
        and #$00FF
        cmp #'<'
        bne @dsz
        inc FTDF
        jsr ft_word
        bra @dn
@dsz:   sty FTZ
        jsr ft_word
        sty FTZE
@dn:    ldx #0
        jsr ft_skipf
        bra @nm
@pl:    ldy FTI                 ; nur Namen (FujiNet usw.), / am Ende = Verzeichnis
        ldy FTX
        dey
        lda [PSRC],y
        and #$00FF
        cmp #'/'
        bne @pl1
        sty FTX
        inc FTDF
@pl1:   ldy FTI
        bra @nm
@ux:    inc FTDF
@ux0:   ldy FTI                 ; Unix: Rechte Links Besitzer Gruppe Groesse Mon Tag Zeit Name
        ldx #4
        jsr ft_skipf
        lda FTDF
        bne @us
        sty FTZ
@us:    jsr ft_word
        sty FTZE
        ldx #3
        jsr ft_skipf
        sty FTS                 ; "name -> ziel": nur den Namen
@ar:    cpy FTX
        bcs @ar2
        lda [PSRC],y
        cmp #'>'*256+'-'
        beq @ar1
        iny
        bra @ar
@ar1:   dey                     ; Leerzeichen davor
        sty FTX
@ar2:   ldy FTS
@nm:    sty FTS
        cpy FTX
        bcc @nm1
        rts
@nm1:   lda FTX                 ; "." und ".." weglassen
        sec
        sbc FTS
        cmp #3
        bcs @em
        lda [PSRC],y
        and #$00FF
        cmp #'.'
        bne @em
        iny
        cpy FTX
        bcs @rr
        lda [PSRC],y
        and #$00FF
        cmp #'.'
        bne @em
@rr:    rts
@em:    lda #ft_aopen
        jsr h_z
        jsr ft_base
        ldy FTS
@n1:    cpy FTX
        bcs @n2
        lda [PSRC],y
        and #$00FF
        phy
        cmp #' '
        beq @sp
        cmp #'#'
        beq @sp
        cmp #'?'
        beq @sp
        jsr h_esc
        bra @n1e
@sp:    pha                     ; %xx
        lda #'%'
        jsr h_c
        pla
        pha
        lsr
        lsr
        lsr
        lsr
        jsr ft_hx
        pla
        and #$000F
        jsr ft_hx
@n1e:   ply
        iny
        bra @n1
@n2:    lda FTDF
        beq @n3
        lda #'/'
        jsr h_c
@n3:    lda #ft_acl
        jsr h_z
        lda FTDF
        beq @n4
        lda #'['
        jsr h_c
@n4:    ldy FTS
@n5:    cpy FTX
        bcs @n6
        lda [PSRC],y
        and #$00FF
        phy
        jsr h_esc
        ply
        iny
        bra @n5
@n6:    lda FTDF
        beq @n7
        lda #']'
        jsr h_c
@n7:    lda #ft_aend
        jsr h_z
        lda FTDF
        bne @r
        lda FTZ
        beq @r
        lda #ft_sz
        jsr h_z
        ldy FTZ
@z:     cpy FTZE
        bcs @zb
        lda [PSRC],y
        and #$00FF
        jsr h_c
        iny
        bra @z
@zb:    lda #ft_by
        jsr h_z
@r:     rts
ft_hx:  cmp #10
        bcc @d
        adc #'A'-10-1
        jmp h_c
@d:     adc #'0'
        jmp h_c

ft_t1:  .byte "<title>ftp://",0
ft_t2:  .byte "</title><h1>ftp://",0
ft_t3:  .byte "</h1><p>",0
ft_sftp: .byte "ftp://",0
ft_aup0: .byte "<a href=",'"',0
ft_aup: .byte '"',">[..]</a>",0
ft_aopen: .byte "<br><a href=",'"',0
ft_acl: .byte '"',">",0
ft_aend: .byte "</a>",0
ft_sz:  .byte " &nbsp; ",0
ft_by:  .byte " Bytes",0
ft_eh:  .byte "<title>FTP</title><h1>FTP</h1><p>",0
ft_br:  .byte "<br><br>",0
ft_snc: .byte "FTP geht mit der Uthernet II oder der FujiNet. Bitte in der Systemsteuerung (Netzwerk) eine dieser Karten w&auml;hlen.",0
ft_snc_en: .byte "FTP needs an Uthernet II or a FujiNet. Please choose one of them in Control Panel (Network).",0
ft_sse: .byte "Der FTP-Server lehnt ab. Seine Antwort:",0
ft_sse_en: .byte "The FTP server refused. Its answer:",0
ft_cuser: .byte "USER anonymous",0
ft_cpass: .byte "PASS gs95@",0
ft_ctype: .byte "TYPE I",0
ft_ccwd: .byte "CWD ",0
ft_cpasv: .byte "PASV",0
ft_clist: .byte "LIST",0
ft_cretr: .byte "RETR ",0
