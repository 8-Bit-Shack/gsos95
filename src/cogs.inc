; ================================================================
; GS/OS95: CoGS-Karte (Raspberry Pi Pico 2 W, Teil von net.s)
; 16 Register ab $C080+n*16: +0 DATA, +1 STATUS, +2/+3 RXLO/RXHI, +4 CONTROL,
; +5 ID ($A2), +6 VERSION. Rahmen: Code, Laenge (2, LE), Inhalt.
; Die Karte erledigt DNS, TCP und TLS selbst (Socket 0 fuer den Browser).
; Alle Zugriffe absolut lang indiziert (keine Blindlesezyklen auf DATA).
; ================================================================
CIO     = $E0C080
C_DATA  = 0
C_STAT  = 1
C_CTRL  = 4
C_ID    = 5
C_VER   = 6

        .pushseg
        .segment "BSS"
CGOP:   .res 2
CGLEN:  .res 2
CGI:    .res 2
CGWANT: .res 2
CGTMO:  .res 2
CGBUF:  .res 34
CGFW:   .res 2                  ; Firmware: Haupt-/Nebennummer
CGLNK:  .res 2                  ; 2 = verbunden
CGWIFI: .res 2                  ; 1 = WLAN, 0 = Kabel
        .popseg

; CoGS im Slot NCSLOT? C=0 ja (nur Lesezugriffe)
cg_probe:
        lda NCSLOT
        and #$0007
        beq @no
        asl
        asl
        asl
        asl
        sta NSLOTX
        tax
        sep #$20
        lda f:CIO+C_ID,x
        cmp #$A2
        bne @no8
        lda f:CIO+C_VER,x
        beq @no8
        lda f:CIO+C_STAT,x
        cmp #$FF
        beq @no8
        rep #$20
        clc
        rts
@no8:   rep #$20
@no:    sec
        rts

; Byte A an die Karte (wartet auf Platz)
cg_put: pha
        ldx NSLOTX
        ldy #0
@w:     sep #$20
        lda f:CIO+C_STAT,x
        rep #$20
        and #$0040
        bne @ok
        dey
        bne @w
@ok:    pla
        sep #$20
        sta f:CIO+C_DATA,x
        rep #$20
        rts

; Byte von der Karte -> A. Wartet hoechstens CGTMO Ticks, Esc bricht ab. C=1 Fehler
cg_get: ldx NSLOTX
        sep #$20
        lda f:CIO+C_STAT,x
        rep #$20
        and #$0080
        bne @rd
        jsr tm_start
@w:     ldx NSLOTX
        sep #$20
        lda f:CIO+C_STAT,x
        rep #$20
        and #$0080
        bne @rd
        jsr net_esc
        bcs @r
        jsr t_elapsed
        cmp CGTMO
        bcc @w
        lda #NE_TIME
        sta NERR
        sec
@r:     rts
@rd:    ldx NSLOTX
        sep #$20
        lda f:CIO+C_DATA,x
        rep #$20
        and #$00FF
        clc
        rts

; Rahmenkopf senden: A = Code, X = Laenge
cg_hdr_out:
        phx
        jsr cg_put
        lda 1,s
        jsr cg_put
        pla
        xba
        jmp cg_put

; Rahmenkopf lesen -> CGOP, CGLEN. C=1 Fehler
cg_hdr: jsr cg_get
        bcs @r
        sta CGOP
        jsr cg_get
        bcs @r
        sta CGLEN
        jsr cg_get
        bcs @r
        xba
        ora CGLEN
        sta CGLEN
        clc
@r:     rts

; CGLEN Bytes lesen, die ersten 32 nach CGBUF
cg_buf: stz CGI
@l:     lda CGI
        cmp CGLEN
        bcs @d
        jsr cg_get
        bcs @r
        ldx CGI
        cpx #32
        bcs @n
        sep #$20
        sta CGBUF,x
        rep #$20
@n:     inc CGI
        bra @l
@d:     clc
@r:     rts

; Fehlercode der Karte A -> NERR
cg_err: ldx #NE_CONN
        cmp #1
        bne @1
        ldx #NE_LINK
@1:     cmp #2
        bne @2
        ldx #NE_DNS
@2:     cmp #3
        bne @3
        ldx #NE_TIME
@3:     cmp #5
        bne @5
        ldx #NE_TLS
@5:     cmp #6
        bne @6
        ldx #NE_CERT
@6:     stx NERR
        rts

; auf Rahmen A warten (andere Rahmen ueberspringen); Inhalt nach CGBUF.
; ERROR ($93) fuer Socket 0 oder ohne Socket beendet mit C=1
cg_wait:
        sta CGWANT
@f:     jsr cg_hdr
        bcs @r
        lda CGOP
        cmp CGWANT
        beq @got
        jsr cg_buf
        bcs @r
        lda CGOP
        cmp #$93
        bne @f
        lda CGBUF
        and #$00FF
        beq @err
        cmp #$FF
        bne @f
@err:   lda CGBUF+1
        and #$00FF
        jsr cg_err
        sec
        rts
@got:   jsr cg_buf
@r:     rts

; Puffer der Karte leeren und Parser zuruecksetzen
cg_flush:
        ldx NSLOTX
        sep #$20
        lda #$0C
        sta f:CIO+C_CTRL,x
        rep #$20
        ldy #2000
@d:     dey
        bne @d
        rts

; ---------- Karte bereit machen: PING, NETINFO ----------
cg_start:
        jsr cg_flush
        lda #120
        sta CGTMO
        lda #$00                ; PING
        ldx #0
        jsr cg_hdr_out
        lda #$80
        jsr cg_wait
        bcc @p
        lda #NE_CARD
        sta NERR
        rts
@p:     lda CGBUF
        sta CGFW
        lda #$08                ; NETINFO
        ldx #0
        jsr cg_hdr_out
        lda #$88
        jsr cg_wait
        bcs @r
        lda CGBUF
        and #$00FF
        sta CGLNK
        ldx #0                  ; IP, Maske, Gateway, DNS
@c:     lda CGBUF+1,x
        sta NCUR,x
        inx
        inx
        cpx #16
        bcc @c
        lda CGBUF+28            ; SSID-Laenge: 0 = Kabel
        and #$00FF
        beq @k
        lda #1
@k:     sta CGWIFI
        lda #1
        sta NDHOK
        lda CGLNK
        cmp #2
        beq @ok
        lda #NE_LINK
        sta NERR
        sec
        rts
@ok:    clc
@r:     rts

; ---------- Name WB_HOST -> DIP ----------
cg_resolve:
        lda #600
        sta CGTMO
        jsr net_ptr
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        tax
        lda #$0B
        jsr cg_hdr_out
        jsr cg_host
        lda #$8B
        jsr cg_wait
        bcs @r
        lda CGBUF
        and #$00FF
        bne @bad
        lda CGBUF+1
        and #$00FF
        beq @bad
        lda CGBUF+2
        sta DIP
        lda CGBUF+4
        sta DIP+2
        clc
        rts
@bad:   lda #NE_DNS
        sta NERR
        sec
@r:     rts

; Bytes des Rechnernamens senden
cg_host:
        jsr net_ptr
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        sta CGI+0
        beq @r
        iny
@l:     lda [PSRC],y
        phy
        jsr cg_put
        ply
        iny
        dec CGI+0
        bne @l
@r:     rts

; ---------- Verbindung: Socket 0 zu WB_HOST:DPORT, NTLS = verschluesselt ----------
cg_open:
        lda #2400               ; 40 s (TLS-Handshake)
        sta CGTMO
        jsr net_ptr
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        clc
        adc #5
        tax
        lda #$10                ; CONNECT
        jsr cg_hdr_out
        lda #0                  ; Socket
        jsr cg_put
        lda NTLS                ; Bit 0: TLS (mit Zertifikatspruefung)
        and #1
        jsr cg_put
        lda DPORT
        jsr cg_put
        lda DPORT
        xba
        jsr cg_put
        jsr net_ptr
        ldy #WB_HOST
        lda [PSRC],y
        jsr cg_put
        jsr cg_host
        lda #$90                ; CONNECTED
        jsr cg_wait
        bcs @r
        lda CGBUF+1
        and #$00FF
        beq @ok
        jsr cg_err
        sec
        rts
@ok:    clc
@r:     rts

; ---------- WCNT Bytes ab WPTR senden ----------
cg_write:
        lda #600
        sta CGTMO
        lda WCNT
        inc a
        tax
        lda #$11                ; SEND
        jsr cg_hdr_out
        lda #0
        jsr cg_put
        jsr net_ptr
        ldy WPTR
        lda WCNT
        sta CGI+0
@l:     lda [PSRC],y
        phy
        jsr cg_put
        ply
        iny
        dec CGI+0
        bne @l
        lda #$9A                ; SEND_ACK
        jsr cg_wait
        bcs @r
        lda CGBUF+1
        and #$00FF
        beq @ok
        jsr cg_err
        sec
        rts
@ok:    clc
@r:     rts

; ---------- empfangen bis CLOSED: nach WPTR, hoechstens bis NLIM ----------
cg_readall:
        stz NFULL
        lda #1200               ; 20 s ohne Daten
        sta CGTMO
@f:     jsr cg_hdr
        bcc *+3
        rts
        lda CGOP
        cmp #$91                ; DATA
        beq @data
        jsr cg_buf
        bcc *+3
        rts
        lda CGOP
        cmp #$92                ; CLOSED
        bne @e
        lda CGBUF
        and #$00FF
        bne @f
        clc
        rts
@e:     cmp #$93                ; ERROR
        bne @f
        lda CGBUF
        and #$00FF
        beq @er
        cmp #$FF
        bne @f
@er:    lda CGBUF+1
        and #$00FF
        jsr cg_err
        sec
        rts
@data:  lda CGLEN
        beq @f
        jsr cg_get              ; Socket
        bcc *+3
        rts
        sta CGWANT
        dec CGLEN
        jsr net_ptr
@b:     lda CGLEN
        beq @bd
        jsr cg_get
        bcc *+3
        rts
        ldx CGWANT              ; fremder Socket: verwerfen
        bne @nx
        ldy WPTR
        cpy NLIM
        bcs @nx
        sep #$20
        sta [PSRC],y
        rep #$20
        inc WPTR
@nx:    dec CGLEN
        bra @b
@bd:    lda NSTRM               ; Platz fuer einen weiteren Block (1 KB)?
        beq @bq
        lda NLIM
        sec
        sbc WPTR
        cmp #1025
        bcs @bq
        lda #1
        sta NFULL
        clc
        rts
@bq:    lda NQUIET
        beq *+5
        jmp @f
        JSLF xf_web_prog
        jmp @f

; ---------- Verbindung schliessen ----------
cg_close:
        lda #60
        sta CGTMO
        lda #$12
        ldx #1
        jsr cg_hdr_out
        lda #0
        jsr cg_put
@f:     jsr cg_hdr              ; kurz auf CLOSED warten
        bcs @r
        jsr cg_buf
        bcs @r
        lda CGOP
        cmp #$92
        beq @r
        cmp #$93
        bne @f
@r:     clc
        rts

; ---------- ohne Warten: vorhandene Rahmen lesen (fuer Telnet) ----------
; C=1: Verbindung geschlossen oder Fehler
cg_poll:
        lda #120
        sta CGTMO
@f:     lda NLIM                ; Platz fuer einen ganzen Block?
        sec
        sbc WPTR
        cmp #1025
        bcc @ok
        ldx NSLOTX
        sep #$20
        lda f:CIO+C_STAT,x
        rep #$20
        and #$0080
        beq @ok
        jsr cg_hdr
        bcs @x
        lda CGOP
        cmp #$91
        beq @data
        jsr cg_buf
        bcs @x
        lda CGOP
        cmp #$92                ; CLOSED
        bne @er
        lda CGBUF
        and #$00FF
        bne @f
@x:     sec
        rts
@er:    cmp #$93
        bne @f
        lda CGBUF
        and #$00FF
        beq @x
        cmp #$FF
        beq @x
        bra @f
@data:  lda CGLEN
        beq @f
        jsr cg_get
        bcs @x
        sta CGWANT
        dec CGLEN
        jsr net_ptr
@b:     lda CGLEN
        beq @f
        jsr cg_get
        bcs @x
        ldx CGWANT
        bne @nx
        ldy WPTR
        sep #$20
        sta [PSRC],y
        rep #$20
        inc WPTR
@nx:    dec CGLEN
        bra @b
@ok:    clc
        rts
