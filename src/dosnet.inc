; ================================================================
; GS/OS95: Netzwerkbefehle der MS-DOS-Eingabeaufforderung (dos.inc, XCODE4)
; IPCONFIG/WINIPCFG, PING (Zeit fuer den Verbindungsaufbau zu TCP-Port 80 -
; ICMP koennen nicht alle Karten), NSLOOKUP, FINGER benutzer@rechner.
; Benutzt die Netzwerkroutinen des Browsers (net.s) und dessen Pufferbank:
; Rechnername nach WB_HOST, Antwort ab WB_RAW (hoechstens 16 KB).
; ================================================================
        .pushseg
        .segment "BSS"
DNT0:   .res 4
DNK:    .res 2
DNN:    .res 2
DNU:    .res 2                  ; FINGER: Laenge des Benutzernamens
        .popseg

; Karte bereit machen. C=1: Meldung ausgegeben
d_netup:
        stz NABORT
        JSLF xf_net_start
        bcc @r
        jmp d_neterr
@r:     rts

; Meldung zu NERR
d_neterr:
        lda NERR
        ldx #DS_NONET
        cmp #1                  ; NE_CARD
        beq @m
        ldx #DS_NDNS
        cmp #3
        beq @m
        ldx #DS_NCONN
        cmp #4
        beq @m
        ldx #DS_NTIME
        cmp #5
        beq @m
        ldx #DS_NABRT
        cmp #6
        beq @m
        lda #DS_NOTH
        jsr d_msg
        lda NERR
        sta DSN_
        stz DSN_+2
        lda #0
        jsr d_num
        jsr x4_nl
        sec
        rts
@m:     txa
        jsr d_msg
        sec
        rts

; Argument (bis Leerzeichen, ab Zeichen A der Zeile, X Zeichen) -> WB_HOST
d_host: sta DNK
        stx DNN
        lda NWB
        sta PSRC+2
        stz PSRC
        ldy #WB_HOST+1
        ldx DNK
@c:     cpx DNN
        bcs @e
        lda DSLINE_,x
        and #$00FF
        cmp #' '
        beq @e
        cmp #'A'                ; Rechnernamen klein
        bcc @s
        cmp #'Z'+1
        bcs @s
        ora #$20
@s:     sep #$20
        sta [PSRC],y
        rep #$20
        iny
        inx
        cpy #WB_HOST+64
        bcc @c
@e:     tya
        sec
        sbc #WB_HOST+1
        ldy #WB_HOST
        sep #$20
        sta [PSRC],y
        rep #$20
        rts

; WB_HOST ausgeben
d_phost:
        lda NWB
        sta PSRC+2
        stz PSRC
        ldy #WB_HOST
        lda [PSRC],y
        and #$00FF
        sta DNN
        ldy #WB_HOST+1
@l:     lda DNN
        beq @r
        lda [PSRC],y
        and #$00FF
        phy
        jsr x4_put
        ply
        iny
        dec DNN
        bra @l
@r:     rts

; vier Bytes ab X (Hauptbank) als a.b.c.d
d_ip:   stx DNK
        stz DNN
@l:     ldx DNK
        lda a:0,x
        and #$00FF
        sta DSN_
        stz DSN_+2
        lda #0
        jsr d_num
        inc DNK
        inc DNN
        lda DNN
        cmp #4
        bcs @r
        lda #'.'
        jsr x4_put
        bra @l
@r:     rts

; Name aufloesen (wenn die Karte das nicht selbst tut). C=1: Meldung ausgegeben
d_resol:
        lda NCAPS
        and #1                  ; CAP_NAME: Karte nimmt den Namen
        beq @dns
        stz DIP
        stz DIP+2
        clc
        rts
@dns:   JSLF xf_net_resolve
        bcc @r
        jmp d_neterr
@r:     rts

; Zeit seit DNT0 (Ticks, 60/s) in ms -> DSN_
d_ms:   pha
        pha
        TOOL $2503              ; GetTick
        pla
        sec
        sbc DNT0
        tax
        pla
        txa
        ldx #1000
        jsr d4_mul
        ldx #60
        jsr d4_div
        sta DSN_
        stz DSN_+2
        rts
; 16 Bit: A * X -> A, A / X -> A
d4_mul: sta DNK
        stx DNN
        lda #0
        ldy #16
@l:     asl
        asl DNK
        bcc @n
        clc
        adc DNN
@n:     dey
        bne @l
        rts
d4_div: sta DNK
        stx DNN
        lda #0
        ldy #16
@l:     asl DNK
        rol
        cmp DNN
        bcc @n
        sbc DNN
        inc DNK
@n:     dey
        bne @l
        lda DNK
        rts

; ---------- IPCONFIG ----------
d_ipcfg:
        jsr d_netup
        bcc *+3
        rts
        lda #DS_IPC0
        jsr d_msg
        lda #DS_IPC1            ; Karte: Name aus der Liste "?|Uthernet II|..."
        jsr d_msg
        lda #DS_CARDS
        asl
        tax
        TBL ds_de
        ldy LANG
        beq @c0
        TBL ds_en
@c0:    jsr x4_kb
        lda [LP2]
        and #$00FF
        sta DNN
        lda NCARD
        sta DNK                 ; so viele '|' ueberspringen
        ldy #1
@sk:    lda DNK
        beq @pr
@s2:    cpy DNN
        bcs @nl
        lda [LP2],y
        iny
        and #$00FF
        cmp #'|'
        bne @s2
        dec DNK
        bra @sk
@pr:    cpy DNN
        beq @pe
        bcs @nl
@pe:    lda [LP2],y
        and #$00FF
        cmp #'|'
        beq @nl
        phy
        jsr x4_put
        ply
        iny
        bra @pr
@nl:    jsr x4_nl
        lda #DS_IPC2
        ldx #NCUR
        jsr @line
        lda #DS_IPC3
        ldx #NCUR+4
        jsr @line
        lda #DS_IPC4
        ldx #NCUR+8
        jsr @line
        lda #DS_IPC5
        ldx #NCUR+12
        jsr @line
        lda #DS_IPC6
        jsr d_msg
        lda NDHCP
        beq @dn
        lda #DS_JA
        bra @dj
@dn:    lda #DS_NEIN
@dj:    jsr d_msg
        lda NCARD               ; MAC nur bei der Uthernet II
        cmp #1
        bne @r
        lda #DS_IPC7
        jsr d_msg
        stz DNN
@mac:   ldx DNN
        lda NMAC,x
        and #$00FF
        jsr d_hex2
        inc DNN
        lda DNN
        cmp #6
        bcs @me
        lda #'-'
        jsr x4_put
        bra @mac
@me:    jsr x4_nl
@r:     rts
@line:  phx                     ; Text A, Adresse X (0.0.0.0: von der Karte)
        jsr d_msg
        plx
        lda a:0,x
        ora a:2,x
        bne @ip
        lda #DS_IPCK
        jsr d_msg
        jmp x4_nl
@ip:    jsr d_ip
        jmp x4_nl

; A als zwei Hex-Ziffern
d_hex2: pha
        lsr
        lsr
        lsr
        lsr
        jsr @d
        pla
@d:     and #$000F
        cmp #10
        bcc @n
        adc #6
@n:     adc #'0'
        jmp x4_put

; ---------- NSLOOKUP name ----------
d_nslook:
        lda DSA_
        cmp DSLN_
        bcc @a
        lda #DS_NEEDP
        jmp d_msg
@a:     jsr d_netup
        bcc *+3
        rts
        lda DSA_
        ldx DSLN_
        jsr d_host
        lda #DS_NSL1
        jsr d_msg
        jsr d_phost
        jsr x4_nl
        lda NCAPS
        and #1
        beq @d
        lda #DS_NSLC
        jmp d_msg
@d:     JSLF xf_net_resolve
        bcc @ok
        jmp d_neterr
@ok:    lda #DS_NSL2
        jsr d_msg
        ldx #DIP
        jsr d_ip
        jmp x4_nl

; ---------- PING name ----------
d_ping: lda DSA_
        cmp DSLN_
        bcc @a
        lda #DS_NEEDP
        jmp d_msg
@a:     jsr d_netup
        bcc *+3
        rts
        lda DSA_
        ldx DSLN_
        jsr d_host
        lda #DS_PING1
        jsr d_msg
        jsr d_phost
        lda #DS_PING2
        jsr d_msg
        jsr d_resol
        bcc *+3
        rts
        lda #4
        sta DNU
@l:     pha                     ; Zeit messen: Verbindung aufbauen, schliessen
        pha
        TOOL $2503              ; GetTick
        pla
        sta DNT0
        pla
        sta DNT0+2
        lda #80
        sta DPORT
        stz NTLS
        JSLF xf_tcp_open
        php
        jsr d_ms
        JSLF xf_tcp_close
        plp
        bcc @ok
        jmp d_neterr
@ok:    lda #DS_PREP
        jsr d_msg
        jsr d_phost
        lda #DS_PTIM
        jsr d_msg
        lda #0
        jsr d_num
        lda #DS_PMS
        jsr d_msg
        dec DNU
        bne @l
        rts

; ---------- FINGER benutzer@rechner ----------
d_finger:
        lda DSA_
        cmp DSLN_
        bcc @a
        lda #DS_NEEDP
        jmp d_msg
@a:     ldx DSA_                ; '@' suchen
@f:     cpx DSLN_
        bcs @noat
        lda DSLINE_,x
        and #$00FF
        cmp #'@'
        beq @at
        inx
        bra @f
@noat:  lda #DS_NEEDP
        jmp d_msg
@at:    txa
        sec
        sbc DSA_
        sta DNU                 ; Laenge des Benutzers
        jsr d_netup
        bcc *+3
        rts
        lda DSA_
        clc
        adc DNU
        inc a
        ldx DSLN_
        jsr d_host
        lda #'['
        jsr x4_put
        jsr d_phost
        lda #']'
        jsr x4_put
        jsr x4_nl
        jsr d_resol
        bcc *+3
        rts
        lda #79
        sta DPORT
        stz NTLS
        JSLF xf_tcp_open
        bcc @o
        jmp d_neterr
@o:     lda NWB                 ; Anfrage "benutzer CR LF" nach WB_REQ
        sta PSRC+2
        stz PSRC
        ldx DSA_
        ldy #WB_REQ
        stz DNN
@c:     lda DNN
        cmp DNU
        bcs @ce
        lda DSLINE_,x
        sep #$20
        sta [PSRC],y
        rep #$20
        inx
        iny
        inc DNN
        bra @c
@ce:    lda #$0A0D
        sta [PSRC],y
        iny
        iny
        tya
        sec
        sbc #WB_REQ
        sta WCNT
        lda #WB_REQ
        sta WPTR
        JSLF xf_tcp_write
        bcc @w
        JSLF xf_tcp_close
        jmp d_neterr
@w:     stz WPTR                ; Antwort bis 16 KB
        lda #$4000
        sta NLIM
        JSLF xf_tcp_readall
        php
        JSLF xf_tcp_close
        plp
        bcc @rd
        jmp d_neterr
@rd:    lda NWB
        sta PSRC+2
        stz PSRC
        ldy #0
@p:     cpy WPTR
        bcs @pe
        lda [PSRC],y
        and #$00FF
        iny
        cmp #13
        beq @p
        cmp #10
        bne @pc
        phy
        jsr x4_nl
        ply
        bra @p
@pc:    cmp #' '
        bcc @p
        phy
        jsr x4_put
        ply
        bra @p
@pe:    rts
