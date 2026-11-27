; ================================================================
; GS/OS95 Terminal: Dateiuebertragung (Teil von net.s, XCODE3)
; Empfangen: ZMODEM startet von selbst (Mailbox sendet "**" ZDLE "B00"),
;   Apfel-D: XMODEM (CRC/Pruefsumme, 128/1K) und YMODEM-Batch.
; Senden: Apfel-U, XMODEM-CRC (128 Bytes je Block).
; Dateien kommen nach *:Downloads (Name von YMODEM/ZMODEM oder eingetippt).
; Ueber Telnet ist die Verbindung fehlerfrei (TCP): Pruefsummen werden nur
; gesendet, beim Empfang nicht nachgerechnet. 255 wird beim Senden
; verdoppelt (Telnet), beim Empfang erledigt das tn_feed.
; Laeuft mit Datenbank = Hauptbank; Puffer in der Pufferbank (NWB):
;   XB_FW  Dateipuffer (4 KB), XB_SB Sendedaten (128), XB_OUT Ausgang.
; Bildschirm ueber das Terminal (xf_vt_put, xf_vt_cur in web.s).
; ================================================================
XB_FW  = $D000
XB_SB  = $E000
XB_OUT = $F000
XFWMAX = $1000
X_SOH = 1
X_STX = 2
X_EOT = 4
X_ACK = 6
X_NAK = $15
X_CAN = $18
ZDLE = $18
; XMON: 0 aus, 1 Name fuer Empfang, 2 Datei zum Senden, 3 X/YMODEM empfangen,
;       4 XMODEM senden, 5 ZMODEM empfangen
        .pushseg
        .segment "BSS"
XMON:   .res 2
XST:    .res 2                  ; Unterzustand
XBI:    .res 2                  ; Bytes im Block
XN:     .res 2                  ; Blocklaenge (mit Nummern und Pruefsumme)
XLEN:   .res 2                  ; Daten je Block
XBLK:   .res 2                  ; erwartete Blocknummer
XB0:    .res 2
XB1:    .res 2
XCRC:   .res 2                  ; 1 = CRC, 0 = Pruefsumme
XRTY:   .res 2
XT0:    .res 2
XYM:    .res 2                  ; YMODEM (Groesse bekannt in XSIZ)
XSIZ:   .res 4
XTOT:   .res 4                  ; geschrieben
XFO:    .res 2                  ; Datei offen
XFI:    .res 2                  ; Bytes im Dateipuffer
XCANS:  .res 2
XACT:   .res 2                  ; Bytes seit dem letzten Leerlauf
XCV:    .res 2                  ; CRC
XO:     .res 2                  ; Ausgang: Schreibstelle
XPL:    .res 2                  ; Eingabezeile
XPB:    .res 64
XEOF:   .res 2
XKB:    .res 2                  ; zuletzt gezeigte KB
XNM:    .res 2                  ; Name eingetippt
XPATH:  .res 2+64               ; Senden: Pfad
ZST:    .res 2
ZFMT:   .res 2                  ; 'A' / 'B' / 'C'
ZESC:   .res 2
ZHN:    .res 2
ZHB:    .res 10                 ; Kopf: Art, 4 Bytes, Pruefsumme
ZMODE:  .res 2                  ; Datenpaket: 0 verwerfen, 1 ZFILE, 2 Daten, 3 ZSINIT
ZFE:    .res 2
ZCRN:   .res 2
ZPOS:   .res 4
ZSCN:   .res 2                  ; Erkennung "**" ZDLE "B"
ZOO:    .res 2                  ; "OO" nach ZFIN verschlucken
ZTX:    .res 4                  ; Kopf zum Senden: 4 Bytes
        .popseg

; ---------- Einspruenge (aus web.s) ----------
; Apfel-D: Name fuer den Empfang erfragen
xf_xm_recv:
        lda XMON
        bne @r
        lda #1
        bra xm_ask
@r:     rtl
; Apfel-U: Datei zum Senden erfragen
xf_xm_send:
        lda XMON
        bne @r
        lda #2
        bra xm_ask
@r:     rtl
xm_ask: sta XMON
        stz XPL
        lda #0
        jsr xv_cur
        jsr xv_nl
        lda XMON
        cmp #1
        bne @s
        lda #xs_ask1
        ldx #xs_ask1_en
        bra @p
@s:     lda #xs_ask2
        ldx #xs_ask2_en
@p:     jsr xv_strl
        lda #1
        jsr xv_cur
        rtl

; Taste A waehrend einer Uebertragung bzw. Eingabe
xf_xm_key:
        and #$007F
        sta XB0
        lda #0
        jsr xv_cur
        lda XMON
        cmp #3
        bcs @run
        lda XB0                 ; Eingabezeile
        cmp #$1B
        beq @can
        cmp #$0D
        beq @ret
        cmp #$08
        beq @bs
        cmp #$7F
        beq @bs
        cmp #' '
        bcc @e
        ldx XPL
        cpx #60
        bcs @e
        sep #$20
        sta XPB,x
        rep #$20
        inc XPL
        jsr xv_put
        bra @e
@bs:    lda XPL
        beq @e
        dec XPL
        lda #8
        jsr xv_put
        lda #' '
        jsr xv_put
        lda #8
        jsr xv_put
        bra @e
@can:   stz XMON
        lda #xs_ab
        ldx #xs_ab_en
        jsr xv_line
        bra @e
@ret:   jsr xv_nl
        lda XMON
        cmp #1
        bne @snd
        jsr xr_start
        bra @e
@snd:   jsr xs_start
        bra @e
@run:   lda XB0                 ; laeuft: Esc bricht ab
        cmp #$1B
        bne @e
        jsr xm_abort
        lda #xs_ab
        ldx #xs_ab_en
        jsr xv_line
@e:     lda #1
        jsr xv_cur
        rtl

; Verbindung weg / Terminal zu: alles beenden (Datei schliessen)
xf_xm_kill:
        lda XMON
        beq @r
        jsr xm_fclose
        stz XMON
@r:     rtl

; Empfangenes Byte A bei laufender Uebertragung
xf_xm_byte:
        and #$00FF
        sta XB0
        inc XACT
        lda XMON
        cmp #5
        beq @z
        cmp #4
        beq @s
        jsr xr_byte
        rtl
@s:     jsr xs_byte
        rtl
@z:     jsr zr_byte
        rtl

; Byte A ohne Uebertragung: ZMODEM-Anfang erkennen. C=1 verschluckt (A bleibt)
xf_zm_scan:
        and #$00FF
        sta XB1
        ldx ZOO                 ; Kopfende und "OO" nach dem Ende verschlucken
        beq @s0
        cmp #13
        beq @sw
        cmp #$8A
        beq @sw
        cmp #10
        beq @sw
        cmp #$11
        beq @sw
        cmp #'O'
        bne @s0
        dec ZOO
@sw:    sec
        rtl
@s0:    stz ZOO
        ldx ZSCN
        cpx #2
        bcs @s2
        cmp #'*'
        bne @z
        inc ZSCN
        bra @no
@s2:    bne @s3
        cmp #'*'
        beq @no
        cmp #ZDLE
        bne @z
        inc ZSCN
        bra @no
@s3:    cmp #'B'
        bne @z
        stz ZSCN
        lda XMON
        bne @no
        jsr zr_start
        sec
        rtl
@z:     stz ZSCN
        cmp #'*'
        bne @no
        inc ZSCN
@no:    lda XB1
        clc
        rtl

; Leerlauf: Zeitueberschreitungen
xf_xm_tick:
        lda XMON
        cmp #3
        bcs @g
        rtl
@g:     lda XACT                ; etwas empfangen: Uhr neu
        beq @g2
        stz XACT
        jsr t_now
        sta XT0
@g2:    jsr t_now
        sec
        sbc XT0
        sta XB1                 ; Ticks seit dem letzten Byte/Senden
        lda XMON
        cmp #4
        beq @s
        cmp #5
        beq @z
        jsr xr_tick
        rtl
@s:     jsr xs_tick
        rtl
@z:     lda XB1                 ; ZMODEM: 10 s still -> ZRINIT/ZRPOS erneut, 6x -> Ende
        cmp #600
        bcc @r
        inc XRTY
        lda XRTY
        cmp #7
        bcc @zr
        jsr xm_abort
        lda #xs_to
        ldx #xs_to_en
        jsr xv_line
        rtl
@zr:    jsr t_now
        sta XT0
        lda XFO
        bne @zp
        jsr zs_rinit
        rtl
@zp:    jsr zs_rpos
@r:     rtl

; ================================================================
; XMODEM / YMODEM empfangen
; ================================================================
xr_start:
        lda #3
        sta XMON
        stz XST
        stz XYM
        stz XFO
        stz XFI
        stz XRTY
        stz XCANS
        stz XTOT
        stz XTOT+2
        stz XKB
        lda #1
        sta XBLK
        sta XCRC
        lda XPL                 ; Name eingetippt?
        sta XNM
        beq @n
        lda #XPB
        sta LP2
        lda MYBANK
        sta LP2+2
        lda XPL
        sta FNLEN
        jsr fn_make
@n:     lda #xs_rdy
        ldx #xs_rdy_en
        jsr xv_line
        jsr xr_req
        rts
; 'C' bzw. X_NAK senden (Block anfordern)
xr_req: jsr t_now
        sta XT0
        lda #X_NAK
        ldx XCRC
        beq @n
        lda #'C'
@n:     jmp xo_one

xr_byte:
        lda XST
        bne @blk
        lda XB0
        cmp #X_SOH
        beq @soh
        cmp #X_STX
        beq @stx
        cmp #X_EOT
        bne @n1
        jmp xr_eot
@n1:    cmp #X_CAN
        bne @r
        inc XCANS
        lda XCANS
        cmp #2
        bcc @r
        jsr xm_fclose
        stz XMON
        lda #xs_rab
        ldx #xs_rab_en
        jmp xv_line
@r:     rts
@soh:   lda #128
        bra @st
@stx:   lda #1024
@st:    sta XLEN
        stz XCANS
        clc
        adc #3
        ldx XCRC
        beq @c
        inc a
@c:     sta XN
        stz XBI
        lda #1
        sta XST
        lda XFI                 ; Platz im Dateipuffer?
        clc
        adc XLEN
        cmp #XFWMAX+1
        bcc @r
        jmp xr_flush
@blk:   lda XBI                 ; Blocknummer, Gegenstueck, Daten, Pruefsumme
        bne @b1
        lda XB0
        sta XB1+0
        sta XCANS               ; (Blocknummer merken)
        bra @nx
@b1:    cmp #1
        bne @b2
        lda XB0
        sta ZHN                 ; Gegenstueck
        bra @nx
@b2:    sec
        sbc #2
        cmp XLEN
        bcs @nx
        clc
        adc XFI
        adc #XB_FW
        tay
        lda XB0
        BWR8
@nx:    inc XBI
        lda XBI
        cmp XN
        bcs *+3
        rts
        stz XST
        lda XCANS               ; Blocknummer
        and #$00FF
        sta XB1
        stz XCANS
        eor ZHN
        and #$00FF
        cmp #$00FF
        beq @ok
        lda #X_NAK
        jmp xo_one
@ok:    lda XB1
        cmp XBLK
        beq @good
        lda XB1                 ; YMODEM: Block 0 = Dateiname, Groesse
        bne @y0
        lda XFO
        bne @y0
        jmp xr_yhdr
@y0:    lda XBLK                ; Wiederholung des letzten Blocks
        dec a
        and #$00FF
        cmp XB1
        bne @bad
        lda #X_ACK
        jmp xo_one
@bad:   jsr xm_abort
        lda #xs_seq
        ldx #xs_seq_en
        jmp xv_line
@good:  lda XFO
        bne @w
        lda XNM                 ; XMODEM ohne Namen: DOWNLOAD
        bne @fo
        stz FNLEN
        jsr fn_make
@fo:    jsr xm_fopen
        bcc @w
        rts
@w:     lda XFI
        clc
        adc XLEN
        sta XFI
        lda XBLK
        inc a
        and #$00FF
        sta XBLK
        stz XRTY
        lda #X_ACK
        jsr xo_one
        jmp xv_prog

; YMODEM-Kopf im Dateipuffer (ab XFI = 0)
xr_yhdr:
        lda #1
        sta XYM
        ldy #XB_FW              ; leer: Ende des Stapels
        BRD8
        bne @f
        lda #X_ACK
        jsr xo_one
        stz XMON
        lda #xs_done
        ldx #xs_done_en
        jmp xv_line
@f:     ldy #XB_FW              ; Name bis 0 (ohne Ordner)
        sty PFA
@n:     BRD8
        beq @ne
        cmp #'/'
        bne @n1
        iny
        sty PFA
        dey
@n1:    iny
        cpy #XB_FW+128
        bcc @n
@ne:    sty PFB
        stz XSIZ
        stz XSIZ+2
        iny                     ; Groesse (dezimal)
@g:     cpy #XB_FW+128
        bcs @ge
        BRD8
        sec
        sbc #'0'
        cmp #10
        bcs @ge
        sta XB1
        phy
        jsr x_mul10
        lda XSIZ
        clc
        adc XB1
        sta XSIZ
        bcc *+5
        inc XSIZ+2
        ply
        iny
        bra @g
@ge:    lda PFA
        sta LP2
        lda NWB
        sta LP2+2
        lda PFB
        sec
        sbc PFA
        sta FNLEN
        jsr fn_make
        jsr xm_fopen
        bcc @o
        rts
@o:     lda #1
        sta XBLK
        stz XFI
        lda #X_ACK
        jsr xo_one
        lda #'C'
        jmp xo_one
x_mul10:
        lda XSIZ                ; XSIZ *= 10
        ldx XSIZ+2
        asl XSIZ
        rol XSIZ+2
        asl XSIZ
        rol XSIZ+2
        clc
        adc XSIZ
        sta XSIZ
        txa
        adc XSIZ+2
        sta XSIZ+2
        asl XSIZ
        rol XSIZ+2
        rts

; Ende einer Datei
xr_eot: lda XFO
        beq @ack
        lda #1                  ; letzter Block: Fuellbytes weg
        sta XEOF
        jsr xr_flush
        stz XEOF
        jsr xm_fclose
        jsr xv_done
@ack:   lda #X_ACK
        jsr xo_one
        lda XYM                 ; YMODEM: naechsten Kopf anfordern
        beq @end
        lda #1
        sta XBLK
        stz XFI
        stz XTOT
        stz XTOT+2
        stz XKB
        stz XRTY
        lda #'C'
        jmp xo_one
@end:   stz XMON
        rts

; Dateipuffer schreiben (XEOF: XMODEM-Fuellbytes $1A am Ende weg)
xr_flush:
        lda XFI
        sta XN
        lda XYM                 ; YMODEM: nicht ueber die Groesse hinaus
        beq @x
        lda XSIZ
        ora XSIZ+2
        beq @x
        lda XSIZ
        sec
        sbc XTOT
        tax
        lda XSIZ+2
        sbc XTOT+2
        bmi @z
        bne @w
        cpx XN
        bcs @w
        stx XN
        bra @w
@z:     stz XN
        bra @w
@x:     lda XEOF
        beq @w
        lda XFI                 ; nur im letzten Block suchen
        sec
        sbc XLEN
        bcs *+5
        lda #0
        sta XB1
@t:     lda XN
        cmp XB1
        beq @w
        bcc @w
        clc
        adc #XB_FW-1
        tay
        BRD8
        cmp #$1A
        bne @w
        dec XN
        bra @t
@w:     lda XN
        jsr xm_fwrite
        stz XFI
        rts

xr_tick:
        lda XFO                 ; noch nichts empfangen: alle 3 s 'C', nach 3x X_NAK
        bne @run
        lda XST
        bne @run
        lda XB1
        cmp #180
        bcc @r
        inc XRTY
        lda XRTY
        cmp #12
        bcs @to
        cmp #4
        bne @q
        lda XYM                 ; YMODEM nach einer Datei: keine weitere
        bne @end
        stz XCRC
@q:     jmp xr_req
@run:   lda XB1                 ; 10 s Pause mitten drin: X_NAK
        cmp #600
        bcc @r
        stz XST
        inc XRTY
        lda XRTY
        cmp #10
        bcs @to
        jsr t_now
        sta XT0
        lda #X_NAK
        jmp xo_one
@to:    lda XYM
        beq @t2
        lda XFO
        bne @t2
@end:   stz XMON                ; YMODEM: Stapel zu Ende
        lda #xs_done
        ldx #xs_done_en
        jmp xv_line
@t2:    jsr xm_abort
        lda #xs_to
        ldx #xs_to_en
        jmp xv_line
@r:     rts

; ================================================================
; XMODEM senden
; ================================================================
xs_start:
        lda XPL
        bne @n
        stz XMON
        rts
@n:     ldx #0                  ; Pfad: mit ':' wie eingetippt, sonst *:Downloads:
@c:     lda XPB,x
        and #$00FF
        cmp #':'
        beq @full
        inx
        cpx XPL
        bcc @c
        ldx #10
@p:     TBL fn_pfx
        sta XPATH+2,x
        dex
        dex
        bpl @p
        ldy #12
        bra @cp
@full:  ldy #0
@cp:    ldx #0
@cp2:   cpx XPL
        bcs @cpe
        cpy #62
        bcs @cpe
        lda XPB,x
        sep #$20
        sta XPATH+2,y
        rep #$20
        inx
        iny
        bra @cp2
@cpe:   sty XPATH
        lda #XPATH              ; oeffnen (lesen)
        sta dl_opn+4
        lda MYBANK
        sta dl_opn+6
        lda #3
        sta dl_opn
        lda #1
        sta dl_opn+8
        GSOS $2010,dl_opn
        bcc @o
        stz XMON
        lda #xs_nf
        ldx #xs_nf_en
        jmp xv_line
@o:     lda dl_opn+2
        sta DLREF
        lda #1
        sta XFO
        lda #4
        sta XMON
        stz XST
        stz XRTY
        stz XCANS
        stz XTOT
        stz XTOT+2
        stz XKB
        stz XEOF
        lda #1
        sta XBLK
        jsr t_now
        sta XT0
        lda #xs_wsnd
        ldx #xs_wsnd_en
        jmp xv_line

xs_byte:
        lda XB0
        cmp #X_CAN
        bne @n
        inc XCANS
        lda XCANS
        cmp #2
        bcc @r
        jsr xm_fclose
        stz XMON
        lda #xs_rab
        ldx #xs_rab_en
        jmp xv_line
@n:     stz XCANS
        lda XST
        bne @s1
        lda XB0                 ; warten auf 'C' / X_NAK
        ldx #1
        cmp #'C'
        beq @go
        ldx #0
        cmp #X_NAK
        bne @r
@go:    stx XCRC
        lda #1
        sta XST
        jmp xs_rd
@s1:    cmp #2
        beq @s2
        lda XB0                 ; Block gesendet
        cmp #X_ACK
        bne @s1n
        jmp xs_next
@s1n:   cmp #X_NAK
        bne @r
        jmp xs_again
@s2:    lda XB0                 ; X_EOT gesendet
        cmp #X_ACK
        beq @fin
        cmp #X_NAK
        bne @r
        lda #X_EOT
        jmp xo_one
@fin:   jsr xm_fclose
        stz XMON
        lda #xs_sent
        ldx #xs_sent_en
        jmp xv_line
@r:     rts

; naechsten Block lesen und senden (am Ende X_EOT)
xs_next:
        lda #128
        jsr x_addtot
        jsr xv_prog
        lda XBLK
        inc a
        and #$00FF
        sta XBLK
xs_rd:  stz XRTY
        lda XEOF
        bne @eot
        lda #4                  ; 128 Bytes lesen
        sta dl_wr
        lda DLREF
        sta dl_wr+2
        lda #XB_SB
        sta dl_wr+4
        lda NWB
        sta dl_wr+6
        lda #128
        sta dl_wr+8
        stz dl_wr+10
        stz dl_wr+12
        GSOS $2012,dl_wr
        jsr net_ptr
        lda dl_wr+12
        beq @eot
        cmp #128
        bcs xs_again
        inc XEOF                ; kurzer Block: mit $1A auffuellen
        tay
@pad:   cpy #128
        bcs xs_again
        phy
        tya
        clc
        adc #XB_SB
        tay
        lda #$1A
        BWR8
        ply
        iny
        bra @pad
@eot:   lda #2
        sta XST
        lda #X_EOT
        jmp xo_one

; Block XBLK aus XB_SB senden
xs_again:
        inc XRTY
        lda XRTY
        cmp #12
        bcc @g
        jsr xm_abort
        lda #xs_to
        ldx #xs_to_en
        jmp xv_line
@g:     jsr t_now
        sta XT0
        jsr xo_beg
        lda #X_SOH
        jsr xo_c
        lda XBLK
        jsr xo_c
        lda XBLK
        eor #$00FF
        jsr xo_c
        stz XCV
        stz XB1                 ; Pruefsumme
        ldy #XB_SB
@d:     BRD8
        phy
        pha
        jsr xo_c
        pla
        pha
        clc
        adc XB1
        sta XB1
        pla
        jsr crc_b
        ply
        iny
        cpy #XB_SB+128
        bcc @d
        lda XCRC
        beq @cs
        lda XCV
        xba
        jsr xo_c
        lda XCV
        jsr xo_c
        jmp xo_send
@cs:    lda XB1
        jsr xo_c
        jmp xo_send

xs_tick:
        lda XST                 ; auf den Empfaenger warten: 60 s
        bne @b
        lda XB1
        cmp #3600
        bcc @r
        bra @to
@b:     lda XB1                 ; Quittung: 10 s, dann nochmal
        cmp #600
        bcc @r
        lda XST
        cmp #2
        beq @e
        jmp xs_again
@e:     inc XRTY
        lda XRTY
        cmp #10
        bcs @to
        jsr t_now
        sta XT0
        lda #X_EOT
        jmp xo_one
@to:    jsr xm_abort
        lda #xs_to
        ldx #xs_to_en
        jmp xv_line
@r:     rts

; CRC-16 (XMODEM, Polynom $1021) um Byte A erweitern
crc_b:  and #$00FF
        xba
        eor XCV
        ldx #8
@l:     asl
        bcc @n
        eor #$1021
@n:     dex
        bne @l
        sta XCV
        rts

; ================================================================
; ZMODEM empfangen
; ================================================================
ZRQINIT = 0
ZRINIT  = 1
ZSINIT  = 2
ZACK    = 3
ZFILE   = 4
ZFIN    = 8
ZRPOS   = 9
ZDATA   = 10
ZEOF    = 11

zr_start:
        lda #5
        sta XMON
        lda #4                  ; Hex-Kopf folgt
        sta ZST
        lda #'B'
        sta ZFMT
        stz ZHN
        stz ZESC
        stz XFO
        stz XFI
        stz XRTY
        stz XCANS
        stz ZPOS
        stz ZPOS+2
        stz XTOT
        stz XTOT+2
        stz XKB
        stz XACT
        jsr t_now
        sta XT0
        lda #xs_zm
        ldx #xs_zm_en
        jmp xv_line

zr_byte:
        lda XB0
        cmp #ZDLE               ; 5x X_CAN: Abbruch durch die Gegenstelle
        bne @nc
        inc XCANS
        lda XCANS
        cmp #5
        bcc @g
        jsr xm_fclose
        stz XMON
        lda #2
        sta ZOO
        lda #xs_rab
        ldx #xs_rab_en
        jmp xv_line
@nc:    stz XCANS
@g:     lda ZST
        asl
        tax
        jmp (@tb,x)
@tb:    .addr zr_hunt, zr_pad, zr_dle, zr_bin, zr_hex, zr_data, zr_crc

zr_hunt:
        lda XB0
        cmp #'*'
        bne @r
        lda #1
        sta ZST
@r:     rts
zr_pad: lda XB0
        cmp #'*'
        beq @r
        cmp #ZDLE
        bne @h
        lda #2
        sta ZST
        rts
@h:     stz ZST
@r:     rts
zr_dle: lda XB0
        sta ZFMT
        stz ZHN
        stz ZESC
        ldx #4
        cmp #'B'
        beq @s
        ldx #3
        cmp #'A'
        beq @s
        cmp #'C'
        beq @s
        ldx #0
@s:     stx ZST
        rts
; Binaerkopf: Art + 4 Bytes + 2/4 Bytes Pruefsumme
zr_bin: jsr zr_unesc
        bcs @r
        ldx ZHN
        cpx #10
        bcs @n
        sep #$20
        sta ZHB,x
        rep #$20
@n:     inc ZHN
        lda #7
        ldx ZFMT
        cpx #'C'
        bne @c
        lda #9
@c:     cmp ZHN
        bne @r
        jmp zr_head
@r:     rts
; Hex-Kopf: 14 Ziffern
zr_hex: lda XB0
        and #$007F
        jsr x_hexd
        bcs @r
        sta XB1
        lda ZHN
        lsr
        tax
        bcs @lo
        lda XB1
        asl
        asl
        asl
        asl
        sep #$20
        sta ZHB,x
        rep #$20
        bra @n
@lo:    sep #$20
        lda ZHB,x
        ora XB1
        sta ZHB,x
        rep #$20
@n:     inc ZHN
        lda ZHN
        cmp #14
        bne @r
        jmp zr_head
@r:     rts
; Hex-Ziffer A -> Wert, C=1 keine
x_hexd: cmp #'0'
        bcc @no
        cmp #'9'+1
        bcc @d
        ora #$20
        cmp #'a'
        bcc @no
        cmp #'f'+1
        bcs @no
        sbc #'a'-11
        clc
        rts
@d:     sbc #'0'-1
        clc
        rts
@no:    sec
        rts

; ZDLE-Folgen aufloesen: C=1 Byte verbraucht (nichts geliefert), sonst A
; (im Datenteil: ZFE gesetzt bei Paketende)
zr_unesc:
        lda ZESC
        bne @e
        lda XB0
        cmp #ZDLE
        bne @x
        inc ZESC
        sec
        rts
@x:     and #$007F              ; XON/XOFF ignorieren
        cmp #$11
        beq @ig
        cmp #$13
        beq @ig
        lda XB0
        clc
        rts
@ig:    sec
        rts
@e:     stz ZESC
        lda XB0
        cmp #'h'
        bcc @x2
        cmp #'k'+1
        bcs @x2
        sta ZFE                 ; Paketende (ZCRCE..ZCRCW)
        sec
        rts
@x2:    cmp #'l'
        bne @x3
        lda #$7F
        clc
        rts
@x3:    cmp #'m'
        bne @x4
        lda #$FF
        clc
        rts
@x4:    eor #$40
        and #$00FF
        clc
        rts

; Datenpaket
zr_data:
        stz ZFE
        jsr zr_unesc
        bcc @d
        lda ZFE
        beq @r
        lda #2                  ; Paketende: Pruefsumme folgt
        ldx ZFMT
        cpx #'C'
        bne @c
        lda #4
@c:     sta ZCRN
        lda #6
        sta ZST
@r:     rts
@d:     ldx ZMODE
        cpx #2
        beq @fd
        cpx #0
        beq @r
        ldy XFI                 ; ZFILE/ZSINIT: Inhalt sammeln (1 KB)
        cpy #1024
        bcs @r
        pha
        tya
        clc
        adc #XB_FW
        tay
        pla
        BWR8
        inc XFI
        rts
@fd:    pha                     ; Dateidaten
        lda XFI
        clc
        adc #XB_FW
        tay
        pla
        BWR8
        inc ZPOS
        bne *+5
        inc ZPOS+2
        inc XFI
        lda XFI
        cmp #XFWMAX
        bcc @r
        lda XFI
        jsr xm_fwrite
        stz XFI
        jmp xv_prog
zr_crc: jsr zr_unesc
        bcs @r
        dec ZCRN
        bne @r
        lda ZFE                 ; Paket fertig
        cmp #'j'                ; ZCRCQ / ZCRCW: quittieren
        bcc @nq
        lda ZMODE
        cmp #2
        bne @nq
        jsr zs_ack
@nq:    lda #5
        sta ZST
        lda ZFE
        cmp #'h'                ; ZCRCE / ZCRCW: Kopf folgt
        beq @hd
        cmp #'k'
        bne @r
@hd:    stz ZST
        lda ZMODE
        cmp #1
        bne @n1
        jmp zr_file
@n1:    cmp #3
        bne @r
        stz XFI
        jsr zs_ack0
@r:     rts

; Kopf ZHB auswerten
zr_head:
        stz ZST
        stz XRTY
        lda ZHB
        and #$00FF
        cmp #ZRQINIT
        bne @n1
        jmp zs_rinit
@n1:    cmp #ZSINIT
        bne @n2
        lda #3
        bra @pk
@n2:    cmp #ZFILE
        bne @n3
        lda #1
@pk:    sta ZMODE
        stz XFI
        lda #5
        sta ZST
        rts
@n3:    cmp #ZDATA
        bne @n4
        lda ZHB+1               ; Stelle passt?
        cmp ZPOS
        bne @rp
        lda ZHB+3
        cmp ZPOS+2
        bne @rp
        lda #2
        sta ZMODE
        lda #5
        sta ZST
        rts
@rp:    stz ZMODE               ; nein: verwerfen, richtige Stelle anfordern
        lda #5
        sta ZST
        jmp zs_rpos
@n4:    cmp #ZEOF
        bne @n5
        lda ZHB+1
        cmp ZPOS
        bne @r
        lda ZHB+3
        cmp ZPOS+2
        bne @r
        lda XFI
        jsr xm_fwrite
        stz XFI
        jsr xm_fclose
        jsr xv_done
        stz ZPOS
        stz ZPOS+2
        stz XTOT
        stz XTOT+2
        stz XKB
        jmp zs_rinit
@n5:    cmp #ZFIN
        bne @r
        lda #ZFIN
        jsr zs_hdr0
        stz XMON
        lda #2
        sta ZOO
        lda #xs_done
        ldx #xs_done_en
        jmp xv_line
@r:     rts

; ZFILE-Inhalt: Name (ohne Ordner) -> Datei anlegen, ab 0 anfordern
zr_file:
        ldy #XB_FW
        sty PFA
@n:     cpy #XB_FW+1024
        bcs @ne
        BRD8
        beq @ne
        cmp #'/'
        bne @n1
        iny
        sty PFA
        dey
@n1:    iny
        bra @n
@ne:    sty PFB
        lda PFA
        sta LP2
        lda NWB
        sta LP2+2
        lda PFB
        sec
        sbc PFA
        sta FNLEN
        jsr fn_make
        stz XFI
        jsr xm_fopen
        bcc @o
        rts
@o:     stz ZPOS
        stz ZPOS+2
        jmp zs_rpos

; ---------- Koepfe senden (hex) ----------
zs_rinit:
        stz ZTX
        lda #$0300              ; ZF0 = CANFDX | CANOVIO
        sta ZTX+2
        lda #ZRINIT
        bra zs_hdr
zs_rpos:
        lda ZPOS
        sta ZTX
        lda ZPOS+2
        sta ZTX+2
        lda #ZRPOS
        bra zs_hdr
zs_ack: lda ZPOS
        sta ZTX
        lda ZPOS+2
        sta ZTX+2
        lda #ZACK
        bra zs_hdr
zs_ack0:
        lda #ZACK
zs_hdr0:
        stz ZTX
        stz ZTX+2
; Kopf Art A mit ZTX senden: "**" ZDLE "B" + 14 Hex-Ziffern + CR LF (XON)
zs_hdr: sta XB1
        jsr xo_beg
        lda #'*'
        jsr xo_c
        lda #'*'
        jsr xo_c
        lda #ZDLE
        jsr xo_c
        lda #'B'
        jsr xo_c
        stz XCV
        lda XB1
        jsr zs_hb
        ldx #0
@l:     lda ZTX,x
        phx
        jsr zs_hb
        plx
        inx
        cpx #4
        bcc @l
        lda XCV
        xba
        and #$00FF
        jsr zs_hx
        lda XCV
        and #$00FF
        jsr zs_hx
        lda #13
        jsr xo_c
        lda #$8A
        jsr xo_c
        lda XB1
        cmp #ZACK
        beq @s
        cmp #ZFIN
        beq @s
        lda #$11
        jsr xo_c
@s:     jmp xo_send
zs_hb:  and #$00FF              ; Byte hex + CRC
        pha
        jsr crc_b
        pla
zs_hx:  pha
        lsr
        lsr
        lsr
        lsr
        jsr @d
        pla
        and #$000F
@d:     cmp #10
        bcc @n
        adc #'a'-11
        jmp xo_c
@n:     adc #'0'
        jmp xo_c

; ================================================================
; Gemeinsames
; ================================================================
; Ausgang: XB_OUT.. sammeln (255 verdoppeln), senden
xo_beg: lda #XB_OUT
        sta XO
        rts
xo_c:   and #$00FF
        ldy XO
        cpy #XB_OUT+$5F0
        bcs @r
        BWR8
        inc XO
        cmp #$FF
        bne @r
        iny
        BWR8
        inc XO
@r:     rts
xo_send:
        lda XO
        sec
        sbc #XB_OUT
        beq @r
        sta WCNT
        lda #XB_OUT
        sta WPTR
        jsr tcp_write
        jsr net_ptr
@r:     rts
xo_one: pha
        jsr xo_beg
        pla
        jsr xo_c
        jmp xo_send

; Abbruch: X_CAN x8 (ZMODEM: + Rueckschritt x8), Datei schliessen
xm_abort:
        jsr xo_beg
        ldx #8
@c:     phx
        lda #X_CAN
        jsr xo_c
        plx
        dex
        bne @c
        lda XMON
        cmp #5
        bne @s
        ldx #8
@b:     phx
        lda #8
        jsr xo_c
        plx
        dex
        bne @b
@s:     jsr xo_send
        jsr xm_fclose
        stz XMON
        rts

; Datei DLPATH anlegen (Typ nach Endung). C=1 Fehler (Meldung, Abbruch)
xm_fopen:
        JSLF xf_dl_open
        jsr net_ptr
        bcs @e
        lda #1
        sta XFO
        jsr xv_nl
        ldx #0                  ; Name zeigen
@l:     cpx DLPATH
        bcs @x
        lda DLPATH+2,x
        and #$00FF
        phx
        jsr xv_put
        plx
        inx
        bra @l
@x:     jsr xv_nl
        clc
        rts
@e:     jsr xm_abort
        lda #xs_fe
        ldx #xs_fe_en
        jsr xv_line
        sec
        rts
; A Bytes ab XB_FW schreiben
xm_fwrite:
        sta dl_wr+8
        cmp #0
        beq @r
        jsr x_addtot
        lda #4
        sta dl_wr
        lda DLREF
        sta dl_wr+2
        lda #XB_FW
        sta dl_wr+4
        lda NWB
        sta dl_wr+6
        stz dl_wr+10
        GSOS $2013,dl_wr
        jsr net_ptr
        bcc @r
        jsr xm_abort
        lda #xs_fe
        ldx #xs_fe_en
        jmp xv_line
@r:     rts
xm_fclose:
        lda XFO
        beq @r
        stz XFO
        lda #1
        sta dl_cls
        lda DLREF
        sta dl_cls+2
        GSOS $2014,dl_cls
        jsr net_ptr
@r:     rts
x_addtot:
        clc
        adc XTOT
        sta XTOT
        bcc @r
        inc XTOT+2
@r:     rts

; ---------- Anzeige im Terminal ----------
xv_put: JSLF xf_vt_put
        rts
xv_cur: JSLF xf_vt_cur
        rts
xv_nl:  lda #13
        jsr xv_put
        lda #10
        jmp xv_put
; Text A (DE) / X (EN) dieser Bank
xv_strl:
        ldy LANG
        beq @d
        txa
@d:     jsr pf_kb
        lda [LP2]
        and #$00FF
        sta XB1
        ldy #1
@l:     lda XB1
        beq @r
        lda [LP2],y
        and #$00FF
        phy
        jsr xv_put
        ply
        iny
        dec XB1
        bra @l
@r:     rts
; als eigene Zeile
xv_line:
        pha
        phx
        jsr xv_nl
        plx
        pla
        jsr xv_strl
        jmp xv_nl
; Fortschritt: Zeile mit KB (nur bei neuem Wert)
xv_prog:
        lda XTOT+1              ; KB = Bytes / 1024
        lsr
        lsr
        cmp XKB
        beq @r
        sta XKB
        lda #13
        jsr xv_put
        lda XKB
        jsr xv_num
        lda #xs_kb
        ldx #xs_kb
        jmp xv_strl
@r:     rts
; Datei fertig: Groesse
xv_done:
        jsr xv_nl
        lda #xs_ok
        ldx #xs_ok_en
        jsr xv_strl
        lda XTOT+1
        lsr
        lsr
        and #$3FFF
        jsr xv_num
        lda #xs_kb
        ldx #xs_kb
        jsr xv_strl
        jmp xv_nl
; Zahl A dezimal
xv_num: sta XB1
        stz PFC
        lda #10000
        jsr @d
        lda #1000
        jsr @d
        lda #100
        jsr @d
        lda #10
        jsr @d
        lda XB1
        clc
        adc #'0'
        jmp xv_put
@d:     sta PFT
        ldy #0
        lda XB1
@l:     cmp PFT
        bcc @o
        sbc PFT
        iny
        bra @l
@o:     sta XB1
        tya
        bne @p
        lda PFC
        beq @r
        tya
@p:     inc PFC
        clc
        adc #'0'
        jmp xv_put
@r:     rts

; ---------- Texte ----------
xs_ask1:   .byte xs_ask1_e-xs_ask1-1,"Datei empfangen (XMODEM/YMODEM). Name (leer = vom Absender), Esc = Abbruch: "
xs_ask1_e:
xs_ask1_en: .byte xs_ask1_en_e-xs_ask1_en-1,"Receive file (XMODEM/YMODEM). Name (empty = from sender), Esc = cancel: "
xs_ask1_en_e:
xs_ask2:   .byte xs_ask2_e-xs_ask2-1,"Datei senden (XMODEM). Name in *:Downloads oder ganzer Pfad, Esc = Abbruch: "
xs_ask2_e:
xs_ask2_en: .byte xs_ask2_en_e-xs_ask2_en-1,"Send file (XMODEM). Name in *:Downloads or full path, Esc = cancel: "
xs_ask2_en_e:
xs_rdy:    .byte xs_rdy_e-xs_rdy-1,"Bereit - startet, sobald die Mailbox sendet (Esc = Abbruch)"
xs_rdy_e:
xs_rdy_en: .byte xs_rdy_en_e-xs_rdy_en-1,"Ready - starts as soon as the BBS sends (Esc = cancel)"
xs_rdy_en_e:
xs_wsnd:   .byte xs_wsnd_e-xs_wsnd-1,"Bereit - jetzt an der Mailbox das Hochladen (XMODEM) starten (Esc = Abbruch)"
xs_wsnd_e:
xs_wsnd_en: .byte xs_wsnd_en_e-xs_wsnd_en-1,"Ready - now start the upload (XMODEM) on the BBS (Esc = cancel)"
xs_wsnd_en_e:
xs_zm:     .byte xs_zm_e-xs_zm-1,"ZMODEM-Empfang nach *:Downloads (Esc = Abbruch)"
xs_zm_e:
xs_zm_en:  .byte xs_zm_en_e-xs_zm_en-1,"ZMODEM download to *:Downloads (Esc = cancel)"
xs_zm_en_e:
xs_ok:     .byte xs_ok_e-xs_ok-1,"Gespeichert: "
xs_ok_e:
xs_ok_en:  .byte xs_ok_en_e-xs_ok_en-1,"Saved: "
xs_ok_en_e:
xs_kb:     .byte 3," KB"
xs_done:   .byte xs_done_e-xs_done-1,"Uebertragung beendet."
xs_done_e:
xs_done_en: .byte xs_done_en_e-xs_done_en-1,"Transfer complete."
xs_done_en_e:
xs_sent:   .byte xs_sent_e-xs_sent-1,"Datei gesendet."
xs_sent_e:
xs_sent_en: .byte xs_sent_en_e-xs_sent_en-1,"File sent."
xs_sent_en_e:
xs_ab:     .byte xs_ab_e-xs_ab-1,"Abgebrochen."
xs_ab_e:
xs_ab_en:  .byte xs_ab_en_e-xs_ab_en-1,"Cancelled."
xs_ab_en_e:
xs_rab:    .byte xs_rab_e-xs_rab-1,"Von der Gegenstelle abgebrochen."
xs_rab_e:
xs_rab_en: .byte xs_rab_en_e-xs_rab_en-1,"Cancelled by the other side."
xs_rab_en_e:
xs_to:     .byte xs_to_e-xs_to-1,"Keine Antwort - abgebrochen."
xs_to_e:
xs_to_en:  .byte xs_to_en_e-xs_to_en-1,"No response - cancelled."
xs_to_en_e:
xs_seq:    .byte xs_seq_e-xs_seq-1,"Blockfolge gestoert - abgebrochen."
xs_seq_e:
xs_seq_en: .byte xs_seq_en_e-xs_seq_en-1,"Block sequence error - cancelled."
xs_seq_en_e:
xs_fe:     .byte xs_fe_e-xs_fe-1,"Datei-Fehler (Datentraeger voll oder schreibgeschuetzt?)"
xs_fe_e:
xs_fe_en:  .byte xs_fe_en_e-xs_fe_en-1,"File error (disk full or locked?)"
xs_fe_en_e:
xs_nf:     .byte xs_nf_e-xs_nf-1,"Datei nicht gefunden."
xs_nf_e:
xs_nf_en:  .byte xs_nf_en_e-xs_nf_en-1,"File not found."
xs_nf_en_e:
