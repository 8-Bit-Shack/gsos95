; ================================================================
; GS/OS95: Dialog "Netzwerk" (Fensterart 22), Teil von net.s
;   Karte (Automatisch, Uthernet II, CoGS, FujiNet, Marinetti, WLAN-Modem),
;   Slot bzw. Modem-Anschluss, DHCP oder feste Adressen, Baudrate;
;   "Testen" startet die Karte und fragt DNS
; ================================================================
ND_FX = WX+112                  ; Eingabefelder
ND_FW = 100
ND_Y0 = WY+56                   ; erste Feldzeile, Abstand 14 (Feld 4 = Proxy)
ND_H  = WH+20                   ; Hoehe des Dialogs (windows.s: Fensterart 22)
ND_PF = 4                       ; Nummer des Proxy-Felds


        .pushseg
        .segment "BSS"
DSLOT:   .res 2
DDHCP:   .res 2
DCSEL:   .res 2                 ; gewaehlte Karte (NC_*, 0 = automatisch)
DMPORT:  .res 2                 ; Modem-Anschluss 1/2
DMBAUD:  .res 2                 ; Baudrate 0..4
DPRX:    .res 24                ; Proxy "rechner:port" (Laengenbyte + bis 21 Zeichen)
NDPY:    .res 2                 ; Y der -/+ Knoepfe
DFOC:    .res 2
DFLD:    .res 64                ; 4 Felder: Laengenbyte + 15 Zeichen
DSM1:    .res 2                 ; Meldungen (Adresse des Texts, 0 = keine)
DSM1X:   .res 2                 ; 1 = IP anhaengen, 2 = Slot anhaengen
DSM1D:   .res 2                 ; 1 = "(DHCP)", 2 = "(fest)"
DSM2:    .res 2
DSM2X:   .res 2
DSIP1:   .res 4
DSIP2:   .res 4
DI:      .res 2
DY:      .res 2
DRT:     .res 2
        .popseg

; Zeichenketten (webstr.inc) liegen in der Bank des Browsers (XCODE2), Bank in STRBK


; Adressfelder benutzt (automatisch oder Uthernet II)? C=1 ja
nd_adr: lda DCSEL
        cmp #NC_W5100+1
        rts
; Name der Karte A (0..5) -> Text (diese Bank: STRBK)
nd_cname:
        asl
        tax
        lda nd_cn,x
        ldy LANG
        beq @r
        lda nd_cn_en,x
@r:     rts
; Baudrate DMBAUD -> A
nd_baud:
        lda DMBAUD
        asl
        tax
        lda nd_bd,x
        rts

; Feld A (0..3): Zeilen-Y -> A
nd_rowy:
        asl
        sta DRT
        asl
        asl
        asl
        sec
        sbc DRT                 ; *14
        clc
        adc #ND_Y0
        rts

ndlg_init:
        jsr net_load
        ldx #22                 ; Proxy in das Feld
@px:    lda PROXP,x
        sta DPRX,x
        dex
        dex
        bpl @px
        lda DPRX
        and #$00FF
        cmp #22
        bcc @pk
        stz DPRX
@pk:
        lda NCSEL
        sta DCSEL
        lda MDPORT
        sta DMPORT
        lda MDBAUD
        sta DMBAUD
        lda NCSLOT
        sta DSLOT
        lda NDHCP
        sta DDHCP
        stz DFOC
        lda NDHCP
        beq @f
        lda #$FFFF              ; mit DHCP: kein Feld aktiv
        sta DFOC
@f:     stz DI
@l:     lda DI
        asl
        asl
        clc
        adc #NCFG
        tax
        jsr ip_text
        lda DI
        asl
        asl
        asl
        asl
        tax
        ldy #0
@c:     lda IPB,y
        sta DFLD,x
        inx
        iny
        cpy #16
        bcc @c
        inc DI
        lda DI
        cmp #4
        bcc @l
        stz DSM1
        stz DSM2
        lda NSTATE              ; schon verbunden: Adresse zeigen
        beq @r
        jsr nd_msgcard
@r:     rts

; Meldung "Karte bereit, IP ..." aus NCUR
nd_msgcard:
        lda NCARD
        cmp #NC_FUJI
        bne @m1
        WS ws_fujiok
        bra @t
@m1:    cmp #NC_MODEM
        bne @m2
        WS ws_mdmok
@t:     sta DSM1
        stz DSM1X
        stz DSM1D
        rts
@m2:    cmp #NC_MARI
        bne @m3
        WS ws_mariok
        sta DSM1
        lda #1
        sta DSM1X
        stz DSM1D
        lda NCUR
        sta DSIP1
        lda NCUR+2
        sta DSIP1+2
        rts
@m3:    cmp #NC_COGS            ; CoGS: eigene Meldung, WLAN oder Kabel
        bne @w5
        WS ws_cogsok
        sta DSM1
        lda #1
        sta DSM1X
        lda NCUR
        sta DSIP1
        lda NCUR+2
        sta DSIP1+2
        lda #4
        ldx CGWIFI
        beq @k
        lda #3
@k:     sta DSM1D
        rts
@w5:    WS ws_cardok
        sta DSM1
        lda #1
        sta DSM1X
        lda NCUR
        sta DSIP1
        lda NCUR+2
        sta DSIP1+2
        lda #2
        ldx NDHOK
        beq @f
        lda #1
@f:     sta DSM1D
        rts

; ---------- Zeichnen ----------
; vertieftes weisses Feld RX,RY,RW,RH
nd_sunk:
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #BLACK
        sta C1
        lda #GRAY
        sta C2
        CALL bevel
        dec RX
        dec RY
        inc RW
        inc RW
        inc RH
        inc RH
        rts
; Knopf RX,RY,RW,RH
nd_btn: lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #GRAY
        sta C1
        lda #DKGRAY
        sta C2
        CALL bevel
        dec RX
        dec RY
        inc RW
        inc RW
        inc RH
        inc RH
        rts

; Text A (diese Bank) an X/Y in Farbe BLACK
nd_text:
        stx SX
        sty SY
        pha
        lda #BLACK
        CALL setcolor
        pla
        jmp ui_str

ndlg_draw:
        lda #WY+24
        sta NDPY
        WS ws_card               ; Karte: [Name] - +
        ldx #WX+10
        ldy #WY+27
        jsr nd_text
        lda #WX+44
        sta RX
        lda #WY+24
        sta RY
        lda #80
        sta RW
        lda #13
        sta RH
        jsr nd_sunk
        lda DCSEL
        jsr nd_cname
        ldx #WX+48
        ldy #WY+27
        jsr nd_text
        lda #WX+126
        jsr nd_pm
        lda #'-'
        CALL putch
        lda #WX+142
        jsr nd_pm
        lda #'+'
        CALL putch
        lda DCSEL               ; Slot (Uthernet II, CoGS, automatisch) / Port (Modem)
        cmp #NC_FUJI
        bne *+5
        jmp @row2
        cmp #NC_MARI
        beq @row2
        WS ws_slot2
        ldx DCSEL
        cpx #NC_MODEM
        bne @sl
        WS ws_port
@sl:    ldx #WX+162
        ldy #WY+27
        jsr nd_text
        lda #WX+184
        sta RX
        lda #WY+24
        sta RY
        lda #14
        sta RW
        lda #13
        sta RH
        jsr nd_sunk
        lda #WX+188
        sta SX
        lda #WY+27
        sta SY
        lda #BLACK
        CALL setcolor
        lda DSLOT
        ldx DCSEL
        cpx #NC_MODEM
        bne @sd
        lda DMPORT
@sd:    clc
        adc #'0'
        CALL putch
        lda #WX+200
        jsr nd_pm
        lda #'-'
        CALL putch
        lda #WX+216
        jsr nd_pm
        lda #'+'
        CALL putch
@row2:  lda DCSEL               ; zweite Zeile je nach Karte
        cmp #NC_MODEM
        bne @r2a
        jmp nd_drmdm
@r2a:   cmp #NC_COGS
        bcc @dh
        WS ws_adrcard
        ldx DCSEL
        cpx #NC_MARI
        bne @ac
        WS ws_adrmari
@ac:    ldx #WX+10
        ldy #WY+43
        jsr nd_text
        jmp nd_msgs
@dh:    ; Ankreuzfeld DHCP
        lda #WX+10
        sta RX
        lda #WY+42
        sta RY
        lda #11
        sta RW
        sta RH
        jsr nd_sunk
        lda DDHCP
        beq @nx
        jsr nd_check
@nx:    WS ws_dhcp
        ldx #WX+26
        ldy #WY+43
        jsr nd_text
        ; Adressfelder
        stz DI
@f:     lda DI
        jsr nd_rowy
        sta DY
        lda DI
        asl
        tax
        lda nd_labs,x
        ldy LANG
        beq @de
        lda nd_labs_en,x
@de:    ldx #WX+10
        ldy DY
        iny
        iny
        iny
        jsr nd_text
        lda #ND_FX
        sta RX
        lda DY
        sta RY
        lda #ND_FW
        sta RW
        lda #13
        sta RH
        jsr nd_sunk
        lda #ND_FX+3
        sta SX
        lda DY
        clc
        adc #3
        sta SY
        lda #BLACK
        ldx DDHCP               ; mit DHCP nur Ersatzwerte: grau
        beq @c
        lda #DKGRAY
@c:     CALL setcolor
        lda DI
        asl
        asl
        asl
        asl
        clc
        adc #DFLD
        jsr ui_strm
        lda DI
        cmp DFOC
        bne @nf
        lda SX                  ; Schreibmarke
        inc a
        sta RX
        lda DY
        clc
        adc #2
        sta RY
        lda #1
        sta RW
        lda #9
        sta RH
        lda #BLACK
        CALL setcolor
        CALL fillrect
@nf:    inc DI
        lda DI
        cmp #4
        bcs *+5
        jmp @f
        jmp nd_msgs
; WLAN-Modem: Baudrate und Hinweis zum Anschluss
nd_drmdm:
        WS ws_baud
        ldx #WX+10
        ldy #WY+45
        jsr nd_text
        lda #WY+42
        sta NDPY
        lda #WX+112
        sta RX
        lda #WY+42
        sta RY
        lda #44
        sta RW
        lda #13
        sta RH
        jsr nd_sunk
        lda #WX+116
        sta SX
        lda #WY+45
        sta SY
        lda #BLACK
        CALL setcolor
        jsr nd_baud
        CALL drawnum
        lda #WX+158
        jsr nd_pm
        lda #'-'
        CALL putch
        lda #WX+174
        jsr nd_pm
        lda #'+'
        CALL putch
        WS ws_adrmdm
        ldx #WX+10
        ldy #WY+62
        jsr nd_text
; Proxy-Feld, Meldungen und Knoepfe
nd_msgs:
        lda #ND_PF
        jsr nd_rowy
        sta DY
        WS ws_proxy
        ldx #WX+10
        ldy DY
        iny
        iny
        iny
        jsr nd_text
        lda #ND_FX
        sta RX
        lda DY
        sta RY
        lda #ND_FW+20
        sta RW
        lda #13
        sta RH
        jsr nd_sunk
        lda #ND_FX+3
        sta SX
        lda DY
        clc
        adc #3
        sta SY
        lda #BLACK
        CALL setcolor
        lda #DPRX
        jsr ui_strm
        lda DFOC
        cmp #ND_PF
        bne @nf
        lda SX                  ; Schreibmarke
        inc a
        sta RX
        lda DY
        clc
        adc #2
        sta RY
        lda #1
        sta RW
        lda #9
        sta RH
        lda #BLACK
        CALL setcolor
        CALL fillrect
@nf:    lda DSM1
        beq @m2
        ldx #WX+10
        ldy #WY+130
        jsr nd_text
        lda DSM1X
        cmp #1
        bne @s1
        ldx #DSIP1
        jsr put_ip
        bra @d1
@s1:    cmp #2
        bne @d1
        lda DSLOT
        clc
        adc #'0'
        CALL putch
@d1:    lda DSM1D
        beq @m2
        asl
        tax
        lda nd_dtab,x
        ldy LANG
        beq @dd
        lda nd_dtab_en,x
@dd:    jsr ui_str
@m2:    lda DSM2
        beq @btn
        ldx #WX+10
        ldy #WY+140
        jsr nd_text
        lda DSM2X
        beq @btn
        ldx #DSIP2
        jsr put_ip
@btn:   ; Testen / OK / Abbrechen
        lda #WX+8
        sta RX
        lda #WY+ND_H-24
        sta RY
        lda #60
        sta RW
        lda #15
        sta RH
        jsr nd_btn
        WS ws_test
        jsr ui_width
        lsr
        eor #$FFFF
        sec
        adc #WX+38
        sta DRT
        WS ws_test
        ldx DRT
        ldy #WY+ND_H-20
        jsr nd_text
        BUTTON WX+WW-136,WY+ND_H-24,60,15
        TEXTC WX+WW-106,WY+ND_H-20,t_ok,BLACK
        BUTTON WX+WW-70,WY+ND_H-24,62,15
        TEXTC WX+WW-39,WY+ND_H-20,t_abbr,BLACK
        rts

; Knopf 15x13 bei X=A, Y=WY+24; danach SX/SY/Farbe fuer das Zeichen
nd_pm:  sta RX
        pha
        lda NDPY
        sta RY
        lda #15
        sta RW
        lda #13
        sta RH
        jsr nd_btn
        pla
        clc
        adc #5
        sta SX
        lda #WY+27
        sta SY
        lda NDPY
        clc
        adc #3
        sta SY
        lda #BLACK
        CALL setcolor
        rts

; Haken im Ankreuzfeld bei RX,RY (11x11)
nd_check:
        lda #BLACK
        CALL setcolor
        lda RX
        clc
        adc #3
        sta DI
        lda RY
        clc
        adc #4
        sta DY
        ldx #0
@l:     phx
        lda nd_ckx,x
        and #$00FF
        clc
        adc DI
        sta RX
        lda nd_cky,x
        and #$00FF
        clc
        adc DY
        sta RY
        lda #1
        sta RW
        lda #2
        sta RH
        CALL fillrect
        plx
        inx
        cpx #6
        bcc @l
        rts
        .pushseg
        .segment "RODATA"
nd_ckx: .byte 0,1,2,3,4,5
nd_cky: .byte 1,2,3,2,1,0
nd_dtab:    .word 0, ws_dhcpok, ws_dhcpno, ws_cwifi, ws_cwire
nd_dtab_en: .word 0, ws_dhcpok_en, ws_dhcpno_en, ws_cwifi_en, ws_cwire_en
nd_labs:    .word ws_ip, ws_mask, ws_gw, ws_dns
nd_labs_en: .word ws_ip_en, ws_mask_en, ws_gw_en, ws_dns_en
nd_cn:      .word ws_c_auto, ws_c_w5, ws_c_cogs, ws_c_fuji, ws_c_mari, ws_c_mdm
nd_cn_en:   .word ws_c_auto_en, ws_c_w5_en, ws_c_cogs_en, ws_c_fuji_en, ws_c_mari_en, ws_c_mdm_en
nd_bd:      .word 2400, 9600, 19200, 38400, 57600
        .popseg

; ---------- Klicks ----------
ndlg_click:
        HIT WX+WW-136,WY+ND_H-24,60,15
        bcc @n0
        jmp nd_ok
@n0:    HIT WX+WW-70,WY+ND_H-24,62,15
        bcc @n1
        jmp nd_cancel
@n1:    HIT WX+8,WY+ND_H-24,60,15
        bcc @n2
        jmp nd_test
@n2:    HIT WX+126,WY+24,15,13      ; Karte -
        bcc @n2b
        lda DCSEL
        bne @c1
        lda #NC_LAST+1
@c1:    dec a
        sta DCSEL
        stz DSM1
        stz DSM2
        jmp @rd
@n2b:   HIT WX+142,WY+24,15,13      ; Karte +
        bcc @n2c
        lda DCSEL
        inc a
        cmp #NC_LAST+1
        bcc @c2
        lda #0
@c2:    sta DCSEL
        stz DSM1
        stz DSM2
        jmp @rd
@n2c:   lda DCSEL               ; Slot/Port nur bei Karten im Steckplatz und beim Modem
        cmp #NC_FUJI
        beq @n4
        cmp #NC_MARI
        beq @n4
        HIT WX+200,WY+24,15,13
        bcc @n3
        lda DCSEL
        cmp #NC_MODEM
        bne @sm
        lda #1
        sta DMPORT
        jmp @rd
@sm:    lda DSLOT
        cmp #2
        bcs *+5
        jmp @rd
        dec DSLOT
        jmp @rd
@n3:    HIT WX+216,WY+24,15,13
        bcc @n4
        lda DCSEL
        cmp #NC_MODEM
        bne @sp
        lda #2
        sta DMPORT
        jmp @rd
@sp:    lda DSLOT
        cmp #7
        bcc *+5
        jmp @rd
        inc DSLOT
        jmp @rd
@n4:    lda DCSEL
        cmp #NC_MODEM
        bne @n4d
        HIT WX+158,WY+42,15,13      ; Baudrate -
        bcc @n4b
        lda DMBAUD
        bne *+5
        jmp @rd
        dec DMBAUD
        jmp @rd
@n4b:   HIT WX+174,WY+42,15,13      ; Baudrate +
        bcc @n4d                ; sonst Proxy-Feld pruefen
        lda DMBAUD
        cmp #4
        bcc *+5
        jmp @rd
        inc DMBAUD
        jmp @rd
@r0:    rts
@n4d:   lda #ND_PF              ; Proxy-Feld (alle Karten)
        jsr nd_rowy
        sta RY
        lda #ND_FX
        sta RX
        lda #ND_FW+20
        sta RW
        lda #13
        sta RH
        CALL inrect
        bcc @n4e
        lda #ND_PF
        sta DFOC
        bra @rd
@n4e:   jsr nd_adr              ; DHCP und Adressen nur ohne feste andere Karte
        bcs @r0
        HIT WX+8,WY+40,220,14
        bcc @n5
        lda DDHCP
        eor #1
        sta DDHCP
        bra @rd
@n5:    stz DI
@f:     lda DI
        jsr nd_rowy
        sta RY
        lda #ND_FX
        sta RX
        lda #ND_FW
        sta RW
        lda #13
        sta RH
        CALL inrect
        bcc @nx
        lda DI
        sta DFOC
        bra @rd
@nx:    inc DI
        lda DI
        cmp #4
        bcc @f
        rts
@rd:    CALL redraw_win
        rts

; ---------- Tasten ----------
ndlg_key:
        lda KEYC
        cmp #$1B
        bne @k1
        jmp nd_cancel
@k1:    cmp #$0D
        bne @k2
        jmp nd_ok
@k2:    cmp #$0A                ; runter: naechstes Feld
        bne @k3
        jsr nd_adr
        bcs @kp
        lda DFOC
        inc a
        cmp #ND_PF+1
        bcc @k2s
        lda #0
@k2s:   sta DFOC
        jmp @rd
@kp:    lda #ND_PF              ; ohne Adressfelder nur der Proxy
        sta DFOC
        jmp @rd
@k3:    cmp #$0B
        bne @k4
        jsr nd_adr
        bcs @kp
        lda DFOC
        dec a
        bpl @k3s
        lda #ND_PF
@k3s:   cmp #ND_PF+1
        bcc @k3t
        lda #ND_PF
@k3t:   sta DFOC
        bra @rd
@k4:    lda DFOC
        cmp #ND_PF
        bne @k4a
        jmp nd_pkey
@k4a:   jsr nd_adr
        bcs @r
        ldx DFOC
        bmi @r
        lda DFOC
        asl
        asl
        asl
        asl
        tax                     ; X = Feldanfang
        lda KEYC
        cmp #$7F                ; Entf
        beq @del
        cmp #$08                ; Pfeil links wie Entf
        beq @del
        cmp #'.'
        beq @add
        cmp #'0'
        bcc @r
        cmp #'9'+1
        bcs @r
@add:   lda DFLD,x
        and #$00FF
        cmp #15
        bcs @r
        inc a
        sep #$20
        sta DFLD,x
        rep #$20
        stx DI
        clc
        adc DI
        tax
        lda KEYC
        sep #$20
        sta DFLD,x
        rep #$20
        bra @rd
@del:   lda DFLD,x
        and #$00FF
        beq @r
        dec a
        sep #$20
        sta DFLD,x
        rep #$20
@rd:    CALL redraw_win
@r:     rts

; Taste im Proxy-Feld: druckbare Zeichen ohne Leerzeichen, Entf
nd_pkey:
        lda KEYC
        cmp #$7F
        beq @del
        cmp #$08
        beq @del
        cmp #'!'
        bcc @r
        cmp #$7F
        bcs @r
        lda DPRX
        and #$00FF
        cmp #21
        bcs @r
        inc a
        sep #$20
        sta DPRX
        rep #$20
        tax
        lda KEYC
        sep #$20
        sta DPRX,x
        rep #$20
        bra @rd
@del:   lda DPRX
        and #$00FF
        beq @r
        dec a
        sep #$20
        sta DPRX
        rep #$20
@rd:    CALL redraw_win
@r:     rts

; Felder pruefen und nach NCFG, Slot/DHCP uebernehmen. C=1 bei Fehler
nd_apply:
        jsr dc_clear
        jsr nd_adr              ; Adressen nur pruefen, wenn sie benutzt werden
        bcs @ok
        stz DI
@l:     lda DI
        asl
        asl
        asl
        asl
        clc
        adc #DFLD
        tax
        lda DI
        asl
        asl
        clc
        adc #NCFG
        tay
        jsr nd_parse
        bcs @bad
        inc DI
        lda DI
        cmp #4
        bcc @l
@ok:    lda NCARD               ; alte Verbindung beenden (Modem auflegen, Marinetti abmelden)
        beq @nc
        jsr tcp_close
@nc:    ldx #22                 ; Proxy uebernehmen
@px:    lda DPRX,x
        sta PROXP,x
        dex
        dex
        bpl @px
        lda DSLOT
        sta NCSLOT
        lda DDHCP
        sta NDHCP
        lda DCSEL
        sta NCSEL
        lda DMPORT
        sta MDPORT
        lda DMBAUD
        sta MDBAUD
        stz NSTATE              ; beim naechsten Mal neu einrichten
        stz NCARD
        stz NCAPS
        clc
        rts
@bad:   lda DI
        sta DFOC
        WS ws_badip
        sta DSM1
        stz DSM1X
        stz DSM1D
        stz DSM2
        sec
        rts

; Text a.b.c.d (Laengenbyte bei X) -> 4 Bytes ab Y. C=1 wenn ungueltig
nd_parse:
        sty NTOP
        lda a:0,x
        and #$00FF
        stx NI
        clc
        adc NI
        sta NJ                  ; letztes Zeichen
        inc NI
        stz WT3                 ; Teile
@oct:   stz WT
        stz WV                  ; Ziffern
@dig:   lda NI
        cmp NJ
        beq @in
        bcs @st
@in:    ldx NI
        lda a:0,x
        and #$00FF
        cmp #'.'
        beq @st
        sec
        sbc #'0'
        cmp #10
        bcs @bad
        sta WT2
        lda WT
        asl
        asl
        clc
        adc WT
        asl
        clc
        adc WT2
        sta WT
        cmp #256
        bcs @bad
        inc WV
        inc NI
        bra @dig
@st:    lda WV
        beq @bad
        lda WT3
        cmp #4
        bcs @bad
        ldx NTOP
        lda WT
        sep #$20
        sta a:0,x
        rep #$20
        inc NTOP
        inc WT3
        lda NI
        cmp NJ
        beq @dot
        bcs @end
@dot:   inc NI                  ; Punkt ueberspringen
        lda NI
        dec a
        cmp NJ                  ; Punkt am Ende: Fehler
        beq @bad
        bra @oct
@end:   lda WT3
        cmp #4
        bne @bad
        clc
        rts
@bad:   sec
        rts

; Abbrechen: beim naechsten Mal Einstellungen neu lesen
nd_cancel:
        stz NPLD
        stz NSTATE
        CALL close_win
        rts

nd_ok:  jsr nd_apply
        bcc @ok
        CALL redraw_win
        rts
@ok:    jsr net_save
        CALL close_win
        rts

nd_test:
        jsr nd_apply
        bcc @go
        CALL redraw_win
        rts
@go:    WS ws_testing
        sta DSM1
        stz DSM1X
        stz DSM1D
        stz DSM2
        CALL redraw_win
        stz NABORT
        jsr net_start
        bcs *+5
        jmp @card
        lda NERR                ; Meldung je nach Karte
        cmp #NE_MEM
        bne @nm
        WS ws_e_mem
        jmp @m0
@nm:    cmp #NE_MARI
        bne @nd
        WS ws_nomari
        bra @m0
@nd:    cmp #NE_MDM
        bne @nl
        WS ws_nomdm
        bra @m0
@nl:    cmp #NE_LINK
        bne @nf
        WS ws_e_link
        bra @m0
@nf:    lda DCSEL
        cmp #NC_FUJI
        bne @na
        WS ws_nofuji
        bra @m0
@na:    cmp #NC_MODEM
        bne @ns
        WS ws_nomdm
        bra @m0
@ns:    lda DCSEL               ; automatisch: keine Karte gefunden
        bne @sl
        WS ws_nocard2
        bra @m0
@sl:    WS ws_nocard            ; Uthernet II / CoGS: "in Slot n"
        sta DSM1
        lda #2
        sta DSM1X
        stz DSM1D
        CALL redraw_win
        rts
@m0:    sta DSM1
        stz DSM1X
        stz DSM1D
        CALL redraw_win
        rts
@card:  jsr nd_msgcard
        lda NCARD               ; FujiNet, Modem: Namen loest die Karte auf
        cmp #NC_FUJI
        beq @dc
        cmp #NC_MODEM
        bne @dp
@dc:    WS ws_dnscard
        sta DSM2
        stz DSM2X
        CALL redraw_win
        rts
@dp:    jsr net_ptr             ; DNS-Probe
        ldx #0
        ldy #WB_HOST
@h:     lda nd_test_host_m,x
        sep #$20
        sta [PSRC],y
        rep #$20
        iny
        inx
        cpx #13
        bcc @h
        jsr net_resolve
        bcs @dbad
        WS ws_dnsok
        sta DSM2
        lda #1
        sta DSM2X
        lda DIP
        sta DSIP2
        lda DIP+2
        sta DSIP2+2
        CALL redraw_win
        rts
@dbad:  WS ws_dnsbad
        sta DSM2
        stz DSM2X
        CALL redraw_win
        rts
        .pushseg
        .segment "RODATA"
nd_test_host_m: .byte 12,"frogfind.com"
        .popseg
