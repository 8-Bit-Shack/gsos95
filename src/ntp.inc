; ================================================================
; Uhrzeit per SNTP (UDP 123, pool.ntp.org): einmal je Sitzung beim ersten
; Seitenaufruf mit der Uthernet II. FujiNet, Marinetti und WLAN-Modem fragen
; per TCP den Time-Dienst (RFC 868, Port 37, time.nist.gov): 4 Bytes,
; Sekunden seit 1900 wie bei NTP. Zeitzone in NPREF+3:
;   Bit 0 DHCP, Bit 1-2 Regel (0 EU-Sommerzeit, 1 keine, 2 US, 3 aus),
;   Bit 3-7 Stunden+16 (0 = Voreinstellung +1)
; Liegt in XCODE2 (wie net.s).
; ================================================================
        .pushseg
        .segment "BSS"
NTPDONE: .res 2
DV_N:    .res 4                 ; Dividend -> Quotient
DV_D:    .res 4                 ; Divisor
DV_R:    .res 4                 ; Rest
NT_S:    .res 4                 ; Sekunden seit 1904 (Ortszeit)
NT_DAY:  .res 2                 ; Tage seit 1904
NT_Y:    .res 2
NT_M:    .res 2                 ; 0..11
NT_D:    .res 2                 ; 1..31
NT_H:    .res 2
NT_MI:   .res 2
NT_SE:   .res 2
NT_W:    .res 2                 ; Wochentag 0 = Sonntag
NT_R:    .res 2                 ; Regel
NT_TZ:   .res 2                 ; Stunden (mit Vorzeichen)
NT_X:    .res 2
        .popseg

xf_ntp_once:    jsr ntp_once
        rtl

; C=1: es wurde gefragt (WB_HOST ist dann ueberschrieben)
ntp_once:
        lda NTPDONE
        bne @no
        inc NTPDONE
        lda NCARD
        cmp #NC_FUJI            ; FujiNet, Marinetti, Modem: Time-Dienst ueber TCP
        bcc @w5
        jsr tz_get
        lda NT_R
        cmp #3
        beq @no
        jmp ntp_tcp
@w5:    cmp #1
        bne @no
        jsr tz_get
        lda NT_R
        cmp #3
        bne @go
@no:    clc
        rts
@go:    jsr net_ptr             ; Name nach WB_HOST
        ldx #0
        ldy #WB_HOST
@c:     lda ntp_name,x
        jsr pk_b
        inx
        cpx #13
        bcc @c
        jsr net_resolve
        bcc *+5
        jmp @yes
        jsr udp_sel
        jsr s_rport
        lda #2
        jsr s_open
        bcc *+5
        jmp @yes
        jsr net_ptr             ; Anfrage: 48 Bytes, LI 0 / Version 3 / Client
        ldy #WB_UDP
        lda #0
@z:     sta [PSRC],y
        iny
        iny
        cpy #WB_UDP+48
        bcc @z
        ldy #WB_UDP
        lda #$1B
        jsr pk_b
        lda #48
        sta WCNT
        lda #123
        sta DPORT
        jsr udp_send
        lda #120                ; 2 s
        jsr udp_wait
        php
        lda #$10
        jsr s_cmd
        plp
        bcc *+5
        jmp @yes
        lda WCNT
        cmp #48
        bcc @yes
        jsr net_ptr             ; Sendezeit (Sekunden seit 1900, hoechstes Byte zuerst)
        ldy #WB_UDP+40
        lda [PSRC],y
        xba
        sta NT_S+2
        ldy #WB_UDP+42
        lda [PSRC],y
        xba
        sta NT_S
        ora NT_S+2
        beq @yes
        jsr nt_set
@yes:   sec
        rts

; Sekunden seit 1900 (NT_S, UTC) -> Ortszeit -> Uhr des IIgs
nt_set:
        lda NT_S                ; 1900 -> 1904: 1460 Tage = $0784CE00
        sec
        sbc #$CE00
        sta NT_S
        lda NT_S+2
        sbc #$0784
        sta NT_S+2
        lda NT_TZ               ; Zeitzone
        jsr nt_addh
        jsr nt_date
        jsr nt_dst
        bcc @set
        lda #1
        jsr nt_addh
        jsr nt_date
@set:   lda NT_M                ; WriteTimeHex(MonatTag, JahrStunde, MinuteSekunde)
        xba
        ora NT_D
        dec a
        pha
        lda NT_Y
        sec
        sbc #1900
        xba
        ora NT_H
        pha
        lda NT_MI
        xba
        ora NT_SE
        pha
        TOOL $0E03
        CALL read_clock
        rts

; Time-Dienst (RFC 868) ueber die TCP-Verbindung der Karte. C=1 (WB_HOST benutzt)
ntp_tcp:
        jsr net_ptr
        ldx #0
        ldy #WB_HOST
@c:     lda time_name,x
        jsr pk_b
        inx
        cpx #14
        bcc @c
        lda NCAPS               ; Name an die Karte oder selbst aufloesen
        and #CAP_NAME
        bne @o
        jsr net_resolve
        bcs @x
@o:     lda #37
        sta DPORT
        stz NTLS
        jsr tcp_open
        bcs @x
        lda #WB_UDP
        sta WPTR
        lda #WB_UDP+8
        sta NLIM
        stz NSTRM
        inc NQUIET
        jsr tcp_readall
        stz NQUIET
        php
        jsr tcp_close
        plp
        bcs @x
        lda WPTR
        sec
        sbc #WB_UDP
        cmp #4
        bcc @x
        jsr net_ptr             ; hoechstes Byte zuerst
        ldy #WB_UDP
        lda [PSRC],y
        xba
        sta NT_S+2
        ldy #WB_UDP+2
        lda [PSRC],y
        xba
        sta NT_S
        ora NT_S+2
        beq @x
        jsr nt_set
@x:     stz NERR
        sec
        rts

        .pushseg
        .segment "RODATA"
ntp_name: .byte 12,"pool.ntp.org"
time_name: .byte 13,"time.nist.gov"
nt_mlen: .byte 31,28,31,30,31,30,31,31,30,31,30,31
        .popseg

; Zeitzone aus NPREF+3 -> NT_TZ, NT_R
tz_get: lda NPREF+3
        and #$00FF
        pha
        lsr
        and #3
        sta NT_R
        pla
        lsr
        lsr
        lsr
        and #$001F
        bne @t
        lda #17                 ; Voreinstellung +1
@t:     sec
        sbc #16
        sta NT_TZ
        rts

; A Stunden (mit Vorzeichen) zu NT_S
nt_addh:
        sta NT_X
        ldx #3600
        stx DV_D
        jsr mul16s              ; DV_N:DV_N+2 = NT_X * 3600 (mit Vorzeichen)
        lda NT_S
        clc
        adc DV_N
        sta NT_S
        lda NT_S+2
        adc DV_N+2
        sta NT_S+2
        rts
; NT_X (mit Vorzeichen) * DV_D (16 Bit) -> DV_N (32 Bit)
mul16s: stz DV_N
        stz DV_N+2
        lda NT_X
        bpl @p
        eor #$FFFF
        inc a
@p:     tax
        beq @s
@l:     lda DV_N
        clc
        adc DV_D
        sta DV_N
        bcc @n
        inc DV_N+2
@n:     dex
        bne @l
@s:     lda NT_X
        bpl @r
        lda DV_N                ; negieren
        eor #$FFFF
        clc
        adc #1
        sta DV_N
        lda DV_N+2
        eor #$FFFF
        adc #0
        sta DV_N+2
@r:     rts

; DV_N / DV_D -> DV_N, Rest DV_R (32 Bit ohne Vorzeichen)
udiv32: stz DV_R
        stz DV_R+2
        ldx #32
@l:     asl DV_N
        rol DV_N+2
        rol DV_R
        rol DV_R+2
        lda DV_R
        sec
        sbc DV_D
        tay
        lda DV_R+2
        sbc DV_D+2
        bcc @s
        sta DV_R+2
        sty DV_R
        inc DV_N
@s:     dex
        bne @l
        rts
; DV_N (32) / A (16) -> DV_N, Rest A
udiv16: sta DV_D
        stz DV_D+2
        jsr udiv32
        lda DV_R
        rts

; NT_S -> Datum und Uhrzeit (1904..2099)
nt_date:
        lda NT_S
        sta DV_N
        lda NT_S+2
        sta DV_N+2
        lda #$5180              ; 86400 = $15180
        sta DV_D
        lda #1
        sta DV_D+2
        jsr udiv32
        lda DV_N
        sta NT_DAY
        lda DV_R                ; Sekunden im Tag
        sta DV_N
        lda DV_R+2
        sta DV_N+2
        lda #60
        jsr udiv16
        sta NT_SE
        lda #60
        jsr udiv16
        sta NT_MI
        lda DV_N
        sta NT_H
        lda NT_DAY              ; 1.1.1904 war ein Freitag
        clc
        adc #5
        sta DV_N
        stz DV_N+2
        lda #7
        jsr udiv16
        sta NT_W
        lda #1904
        sta NT_Y
        lda NT_DAY
@y:     ldx #365
        pha
        lda NT_Y
        and #3
        bne @ny
        inx
@ny:    stx NT_X
        pla
        cmp NT_X
        bcc @ym
        sbc NT_X
        inc NT_Y
        bra @y
@ym:    stz NT_M
@m:     pha
        ldx NT_M
        lda nt_mlen,x
        and #$00FF
        sta NT_X
        cpx #1                  ; Februar im Schaltjahr
        bne @nf
        lda NT_Y
        and #3
        bne @nf
        inc NT_X
@nf:    pla
        cmp NT_X
        bcc @md
        sbc NT_X
        inc NT_M
        bra @m
@md:    inc a
        sta NT_D
        rts

; Sommerzeit nach Regel NT_R? C=1 ja. Umstellung jeweils 2 Uhr Normalzeit.
nt_dst: lda NT_R
        beq @eu
        cmp #2
        beq @us
@no:    clc
        rts
@eu:    lda NT_M
        cmp #2
        bcc @no                 ; Januar, Februar
        beq @eud                ; Maerz
        cmp #9
        bcc @yes                ; April..September
        bne @no                 ; November, Dezember
@eud:   lda #31                 ; Maerz/Oktober: letzter Sonntag = 31 - Wochentag des 31.
        jsr nt_wdof
        eor #$FFFF
        sec
        adc #31
        jsr nt_cmpd             ; C=1: ab diesem Sonntag 2 Uhr
        php
        ldx NT_M
        cpx #2
        beq @mz                 ; Maerz: ab Sonntag Sommerzeit
        plp
        jmp nt_inv              ; Oktober: bis Sonntag Sommerzeit
@mz:    plp
        rts
@us:    lda NT_M                ; April..Oktober immer
        cmp #2
        bcc @no
        beq @usm
        cmp #10
        bcc @yes
        bne @no
        lda #1                  ; November: vor dem ersten Sonntag
        jsr nt_fsun
        jsr nt_cmpd
        jmp nt_inv
@usm:   lda #1                  ; Maerz: ab dem zweiten Sonntag
        jsr nt_fsun
        clc
        adc #7
        jsr nt_cmpd
@ret:   rts
@yes:   sec
        rts
nt_inv: bcc @s
        clc
        rts
@s:     sec
        rts
; erster Sonntag des Monats
nt_fsun:
        jsr nt_wdof             ; Wochentag des 1.
        sta NT_X
        lda #8
        sec
        sbc NT_X                ; 1 + (7 - w) mod 7
        cmp #8
        bcc @r
        sbc #7
@r:     rts
; A = Tag -> Wochentag dieses Tages im aktuellen Monat
nt_wdof:
        clc
        adc NT_W
        adc #35
        sec
        sbc NT_D
        sta DV_N
        stz DV_N+2
        lda #7
        jmp udiv16
; C=1, wenn das Datum ab Tag A, 2 Uhr liegt
nt_cmpd:
        cmp NT_D
        beq @eq
        bcc @y                  ; Tag A liegt vor heute
        clc
        rts
@y:     sec
        rts
@eq:    lda NT_H
        cmp #2
        rts
