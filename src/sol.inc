; ================================================================
; GS/OS95: Solitaer (Klondike, Fensterart 25), Teil von net.s (XCODE3)
; Stapel: 0 Talon, 1 Ablage, 2..5 Zielstapel, 6..12 Spalten.
; Karte (Byte): Bit 0-5 Nummer 0..51 (Farbe = /13: 0 Kreuz, 1 Karo,
; 2 Herz, 3 Pik; Wert = mod 13: 0 = As .. 12 = Koenig), Bit 7 = offen.
; Ziehen mit der Maus (Hintergrund wird gemerkt), Doppelklick legt ab,
; eine oder drei Karten ziehen, Rueckgaengig (ein Schritt), Punkte wie
; bei Windows (Standard), Zeit, Gewinn-Animation mit huepfenden Karten.
; Tasten: N neues Spiel, U oder Strg-Z rueckgaengig, D 1/3 Karten,
; A alles Moegliche ablegen, Esc schliesst.
; Logik (sl_*) mit Datenbank = diese Bank (LG), Oberflaeche (so_*) mit
; Datenbank = Hauptbank.
; ================================================================
SOKIND = 25
SO_CW  = 40                     ; Karte
SO_CH  = 46
SO_X0  = WX+6                   ; erste Spalte
SO_DX  = 44
SO_TY  = WY+18                  ; obere Reihe
SO_BY  = WY+70                  ; Spalten
SO_SY  = WY+174                 ; Statuszeile
SO_AV  = 102                    ; Hoehe fuer eine Spalte
SO_BG  = 5                      ; Tisch (gruen)
SO_LG  = 10                     ; leere Plaetze
SO_RED = 7
SO_BLU = 4
SO_FAN = 10                     ; Ablage: Versatz beim Auffaechern

; ---------- Einsprung ----------
xf_sol_open:    jsr so_open
        rtl
xf_sol_tick:    jsr so_tick
        rtl

; ================================================================
; Daten (diese Bank)
; ================================================================
SPN:    .res 26                 ; Anzahl je Stapel
SPC:    .res 13*52              ; Karten je Stapel
SUN:    .res 26                 ; Rueckgaengig: Kopie
SUC:    .res 13*52
SUSC:   .res 2
SUOK:   .res 2
SSC:    .res 2                  ; Punkte
SD3:    .res 2                  ; 1 = drei Karten ziehen
SRUN:   .res 2                  ; Zeit laeuft
STS:    .res 2                  ; Sekunden
SWON:   .res 2
SRND:   .res 4
SA:     .res 2
SB:     .res 2
SCN:    .res 2
SCI:    .res 2
SDCK:    .res 52
SPT1:   .res 2
SPT2:   .res 2
SPT3:   .res 2
SBT1:   .res 2
SBT2:   .res 2

; Stapel A: X = Index fuer SPC (A*52)
sl_base:
        sta SBT1
        asl
        asl
        sta SBT2                ; *4
        asl
        asl
        asl                     ; *32
        clc
        adc SBT2                ; *36
        sta SBT2
        lda SBT1
        asl
        asl
        asl
        asl                     ; *16
        clc
        adc SBT2                ; *52
        tax
        rts

; Zufall (Xorshift, 16 Bit) -> A
sl_rnd: lda SRND                ; x ^= x<<7; x ^= x>>9; x ^= x<<8
        asl
        asl
        asl
        asl
        asl
        asl
        asl
        eor SRND
        sta SRND
        xba
        and #$00FF
        lsr
        eor SRND
        sta SRND
        xba
        and #$FF00
        eor SRND
        sta SRND
        rts
; Zufallszahl 0..A-1
sl_rmod:
        sta SCN
        jsr sl_rnd
@l:     cmp SCN
        bcc @r
        sec
        sbc SCN
        bra @l
@r:     rts

; neues Spiel austeilen (A = Zufallsstart)
sl_new: ora #1
        sta SRND
        ldx #24
@z:     stz SPN,x
        dex
        dex
        bpl @z
        ldx #0                  ; Karten 0..51 mischen
@i:     txa
        sep #$20
        sta SDCK,x
        rep #$20
        inx
        cpx #52
        bcc @i
        lda #51
        sta SCI
@m:     lda SCI
        inc a
        jsr sl_rmod
        tax
        ldy SCI
        sep #$20
        lda SDCK,x
        pha
        lda SDCK,y
        sta SDCK,x
        pla
        sta SDCK,y
        rep #$20
        dec SCI
        bne @m
        stz SCI                 ; Spalte k bekommt k+1 Karten, die letzte offen
        lda #0
        sta SA
@c:     lda SA                  ; Spalte SA
        clc
        adc #6
        sta SB
        stz SCN
@c2:    lda SCN
        cmp SA
        bcs @c3
        jsr @take
        ldx SB
        jsr sl_push
        inc SCN
        bra @c2
@c3:    jsr @take
        ora #$80
        ldx SB
        jsr sl_push
        lda SA
        inc a
        sta SA
        cmp #7
        bcc @c
@s:     lda SCI                 ; Rest auf den Talon
        cmp #52
        bcs @e
        jsr @take
        ldx #0
        jsr sl_push
        bra @s
@e:     stz SSC
        stz SRUN
        stz STS
        stz SWON
        stz SUOK
        rts
@take:  ldx SCI
        lda SDCK,x
        and #$00FF
        inc SCI
        rts

; Karte A oben auf Stapel X legen
sl_push:
        sta SPT1
        stx SPT2
        txa
        jsr sl_base
        stx SPT3
        lda SPT2
        asl
        tax
        lda SPN,x
        inc SPN,x
        clc
        adc SPT3
        tay
        lda SPT1
        sep #$20
        sta SPC,y
        rep #$20
        rts

; Karte Nr. Y von Stapel X -> A
sl_get: sty SPT1
        txa
        jsr sl_base
        txa
        clc
        adc SPT1
        tax
        lda SPC,x
        and #$00FF
        rts
; Anzahl auf Stapel X -> A
sl_cnt: txa
        asl
        tax
        lda SPN,x
        rts

; Zustand fuer Rueckgaengig merken
sl_save:
        ldx #0
@l:     lda SPN,x
        sta SUN,x
        inx
        inx
        cpx #26
        bcc @l
        ldx #0
@m:     lda SPC,x
        sta SUC,x
        inx
        inx
        cpx #13*52
        bcc @m
        lda SSC
        sta SUSC
        lda #1
        sta SUOK
        sta SRUN                ; Zeit laeuft ab dem ersten Zug
        rts
sl_undo:
        lda SUOK
        bne @g
        rts
@g:     stz SUOK
        ldx #0
@l:     lda SUN,x
        sta SPN,x
        inx
        inx
        cpx #26
        bcc @l
        ldx #0
@m:     lda SUC,x
        sta SPC,x
        inx
        inx
        cpx #13*52
        bcc @m
        lda SUSC
        sta SSC
        rts

; Punkte A dazu (nicht unter 0)
sl_pts: clc
        adc SSC
        bpl @s
        lda #0
@s:     sta SSC
        rts

; Talon: ziehen bzw. Ablage zurueck
sl_deal:
        jsr sl_save
        ldx #0
        jsr sl_cnt
        bne @d
        ldx #1                  ; leer: Ablage umdrehen
        jsr sl_cnt
        bne @rc
        stz SUOK
        rts
@rc:    sta SCN
@r1:    lda SCN
        beq @rp
        dec a
        tay
        ldx #1
        jsr sl_get
        and #$003F
        ldx #0
        jsr sl_push
        dec SCN
        bra @r1
@rp:    stz SPN+2
        lda #$FF9C              ; -100 bzw. -20
        ldx SD3
        beq *+5
        lda #$FFEC
        jmp sl_pts
@d:     lda #1
        ldx SD3
        beq *+5
        lda #3
        sta SCN
@dl:    ldx #0
        jsr sl_cnt
        beq @de
        dec a
        sta SPN
        tay
        ldx #0
        jsr sl_get
        ora #$80
        ldx #1
        jsr sl_push
        dec SCN
        bne @dl
@de:    rts

; Darf Karte A (offen) auf Stapel X? C=1 ja
sl_ok:  sta SA
        stx SB
        cpx #2
        bcc @no
        cpx #6
        bcs @tab
        jsr sl_cnt              ; Zielstapel: As auf leer, sonst gleiche Farbe +1
        bne @f
        lda SA
        jsr sl_rank
        beq @y
        bra @no
@f:     dec a
        tay
        ldx SB
        jsr sl_get
        sta SCN
        jsr sl_suit
        sta SCI
        lda SA
        jsr sl_suit
        cmp SCI
        bne @no
        lda SCN
        jsr sl_rank
        inc a
        sta SCI
        lda SA
        jsr sl_rank
        cmp SCI
        beq @y
@no:    clc
        rts
@y:     sec
        rts
@tab:   jsr sl_cnt              ; Spalte: Koenig auf leer, sonst andere Farbe -1
        bne @t
        lda SA
        jsr sl_rank
        cmp #12
        beq @y
        bra @no
@t:     dec a
        tay
        ldx SB
        jsr sl_get
        bit #$0080
        beq @no
        sta SCN
        jsr sl_red
        sta SCI
        lda SA
        jsr sl_red
        cmp SCI
        beq @no
        lda SCN
        jsr sl_rank
        dec a
        sta SCI
        lda SA
        jsr sl_rank
        cmp SCI
        beq @y
        bra @no
; Wert / Farbe / rot
sl_rank:
        and #$003F
@l:     cmp #13
        bcc @r
        sbc #13
        bra @l
@r:     ora #0                  ; Z setzen (As = 0)
        rts
sl_suit:
        and #$003F
        ldx #0
@l:     cmp #13
        bcc @r
        sbc #13
        inx
        bra @l
@r:     txa
        rts
sl_red: jsr sl_suit
        cmp #1
        beq @y
        cmp #2
        beq @y
        lda #0
        rts
@y:     lda #1
        rts

; Karten ab Nr. SCI von Stapel SA nach Stapel SB (wenn erlaubt). C=1 bewegt
sl_move:
        ldy SCI
        ldx SA
        jsr sl_get
        sta SCN
        lda SB                  ; Zielstapel: nur eine Karte
        cmp #6
        bcs @t
        ldx SA
        jsr sl_cnt
        dec a
        cmp SCI
        bne @no
@t:     lda SA                  ; (sl_ok zerstoert SA/SB/SCI)
        pha
        lda SB
        pha
        lda SCI
        pha
        lda SCN
        ldx SB
        jsr sl_ok
        pla
        sta SCI
        pla
        sta SB
        pla
        sta SA
        bcs @go
@no:    clc
        rts
@go:    jsr sl_save
        lda SA                  ; Punkte
        cmp #1
        bne @p1
        lda SB
        cmp #6
        lda #5
        bcs @pp
        lda #10
        bra @pp
@p1:    cmp #6
        bcc @p2
        lda SB
        cmp #6
        bcs @pq
        lda #10
        bra @pp
@p2:    lda SB                  ; vom Zielstapel zurueck
        cmp #6
        bcc @pq
        lda #$FFF1
@pp:    jsr sl_pts
@pq:    ldx SA                  ; Karten umsetzen
        jsr sl_cnt
        sta SCN
        lda SCI
        sta SDCK
@m:     lda SDCK
        cmp SCN
        bcs @md
        tay
        ldx SA
        jsr sl_get
        ldx SB
        jsr sl_push
        inc SDCK
        bra @m
@md:    lda SA
        asl
        tax
        lda SCI
        sta SPN,x
        sec
        rts

; oberste Karte von Stapel X umdrehen, wenn verdeckt (Spalte). C=1 gedreht
sl_flip:
        stx SA
        jsr sl_cnt
        beq @no
        dec a
        sta SCI
        tay
        ldx SA
        jsr sl_get
        bit #$0080
        bne @no
        jsr sl_save
        lda SA
        jsr sl_base
        txa
        clc
        adc SCI
        tax
        sep #$20
        lda SPC,x
        ora #$80
        sta SPC,x
        rep #$20
        lda #5
        jsr sl_pts
        sec
        rts
@no:    clc
        rts

; oberste Karte von Stapel X auf einen Zielstapel. C=1 bewegt
sl_auto:
        stx SA
        jsr sl_cnt
        beq @no
        dec a
        sta SCI
        tay
        ldx SA
        jsr sl_get
        bit #$0080
        beq @no
        lda #2
        sta SB
@l:     lda SA
        pha
        lda SCI
        pha
        jsr sl_move
        pla
        sta SCI
        pla
        sta SA
        bcs @y
        inc SB
        lda SB
        cmp #6
        bcc @l
@no:    clc
        rts
@y:     sec
        rts

; alles Moegliche ablegen (Ablage und Spalten), C=1 wenn etwas ging
sl_all: stz SCI+0
        lda #0
        sta SDCK+2
@r:     stz SDCK+4
        ldx #1
@l:     stx SDCK+6
        jsr sl_auto
        bcc @n
        inc SDCK+2
        inc SDCK+4
@n:     ldx SDCK+6
        inx
        cpx #13
        bcs @e
        cpx #2
        bne @l
        ldx #6
        bra @l
@e:     lda SDCK+4
        bne @r
        lda SDCK+2
        cmp #1
        rts

; gewonnen? (52 Karten auf den Zielstapeln) -> SWON
sl_won: lda SPN+4
        clc
        adc SPN+6
        adc SPN+8
        adc SPN+10
        cmp #52
        bne @r
        lda SWON
        bne @r
        inc SWON
        stz SRUN
        lda STS                 ; Zeitbonus 700000 / Sekunden (ab 30 s)
        cmp #30
        bcc @r
        sta SA
        stz SB                  ; 32-Bit-Teilung 700000 / SA
        lda #$000A
        sta SCN                 ; hoch
        lda #$AE60
        sta SCI                 ; niedrig
        ldx #32
        stz SDCK
@d:     asl SCI
        rol SCN
        rol SDCK
        lda SDCK
        cmp SA
        bcc @n
        sbc SA
        sta SDCK
        inc SCI
@n:     dex
        bne @d
        lda SCN
        bne @big
        lda SCI
        bpl @ok
@big:   lda #$7FFF
@ok:    jsr sl_pts
@r:     rts

; ================================================================
; Oberflaeche (Datenbank = Hauptbank)
; ================================================================
        .pushseg
        .segment "BSS"
SOW:    .res 2
SOX:    .res 2                  ; Karte zeichnen: Lage, Karte, sichtbare Hoehe
SOY:    .res 2
SOC:    .res 2
SOH:    .res 2
SOP:    .res 2                  ; Stapel
SOI:    .res 2
SON:    .res 2
SOD:    .res 2                  ; Spalte: verdeckte Karten, Abstaende
SODO:   .res 2
SOUO:   .res 2
SOHP:   .res 2                  ; beim Ziehen versteckt: Stapel, ab Karte
SOHI:   .res 2
SOT:    .res 2
SOT2:   .res 2
SOT3:   .res 2
SOGX:   .res 2                  ; Ziehen: Griffpunkt
SOGY:   .res 2
SOMX:   .res 2
SOMY:   .res 2
SOLT:   .res 2                  ; Doppelklick: Zeit, Karte
SOLC:   .res 2
SOSEC:  .res 2
SOHND:  .res 4                  ; Merkpuffer fuer das Ziehen
SOBW:   .res 2
SOBH:   .res 2
SOBX:   .res 2
SOBY:   .res 2
SOBV:   .res 2
SOVX:   .res 4                  ; Animation
SOVY:   .res 2
SOFR:   .res 2
SOSRC:  .res 2
SODN:   .res 2
SOSI:   .res 2
        .popseg

; Variable X dieser Bank lesen / schreiben
sog:    phb
        phk
        plb
        lda a:0,x
        plb
        ora #0
        rts
sos:    phb
        phk
        plb
        sta a:0,x
        plb
        rts
; Anzahl auf Stapel A -> A
so_cnt: tax
        LG sl_cnt
        rts
; Karte Y von Stapel X -> A
so_get: LG sl_get
        rts

so_find:
        stz SOW
@l:     lda SOW
        cmp NWIN
        bcs @no
        CALL recaddr
        sta PB
        lda (PB)
        cmp #SOKIND
        beq @y
        inc SOW
        bra @l
@y:     lda SOW
        sec
        rts
@no:    clc
        rts

so_open:
        jsr so_find
        bcc @new
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        rts
@new:   lda SOHND+2             ; Merkpuffer (4 KB) fuer das Ziehen
        ora SOHND
        bne @m
        pha
        pha
        pea $0000
        pea $1000
        lda USERID
        pha
        pea $C010
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta SOHND
        pla
        sta SOHND+2
        bcc @m
        stz SOHND
        stz SOHND+2
@m:     lda #SOKIND
        CALL win_open
        bcc @o
        lda #8
        CALL show_msg_nr
        CALL redraw_all
        rts
@o:     jsr so_newg
        lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda #0
        sta (PB),y
        ldy #R_Y
        sta (PB),y
        ldy #R_W
        lda #320
        sta (PB),y
        ldy #R_H
        lda #186
        sta (PB),y
        lda NWIN
        dec a
        CALL win_load
        CALL redraw_all
        rts
so_newg:
        jsr t_now
        LG sl_new
        stz SOHP
        lda #$FFFF
        sta SOHP
        stz SOLT
        rts

; ---------- Lage ----------
; Stapel A -> RX/RY der ersten Karte
so_pxy: sta SOT
        cmp #6
        bcs @t
        cmp #2
        bcc @s
        inc a                   ; Zielstapel in Spalte 3..6
@s:     ldx #SO_DX
        CALL gmul
        clc
        adc #SO_X0
        sta RX
        lda #SO_TY
        sta RY
        rts
@t:     sec
        sbc #6
        ldx #SO_DX
        CALL gmul
        clc
        adc #SO_X0
        sta RX
        lda #SO_BY
        sta RY
        rts

; Spalte A: Abstaende (SOD verdeckt, SODO, SOUO), SON Anzahl
so_tlay:
        sta SOP
        jsr so_cnt
        sta SON
        stz SOD
@d:     lda SOD
        cmp SON
        bcs @dd
        ldy SOD
        ldx SOP
        jsr so_get
        bit #$0080
        bne @dd
        inc SOD
        bra @d
@dd:    lda #4
        sta SODO
        lda #11
        sta SOUO
        lda SON
        sec
        sbc SOD
        beq @r
        dec a
        beq @r
        sta SOT                 ; offene - 1
        jsr @need
        cmp #SO_AV+1
        bcc @r
        lda #2                  ; enger
        sta SODO
        lda SOD
        asl
        sta SOT2
        lda #SO_AV-SO_CH
        sec
        sbc SOT2
        bpl *+5
        lda #0
        ldx SOT
        CALL gdiv
        cmp #3
        bcs @u
        lda #3
@u:     cmp #11
        bcc @u2
        lda #11
@u2:    sta SOUO
@r:     rts
@need:  lda SOD
        ldx SODO
        CALL gmul
        sta SOT2
        lda SOT
        ldx SOUO
        CALL gmul
        clc
        adc SOT2
        adc #SO_CH
        rts
; y der Karte A in der Spalte (nach so_tlay), relativ zum Anfang
so_ty:  cmp SOD
        bcs @u
        ldx SODO
        CALL gmul
        rts
@u:     sec
        sbc SOD
        ldx SOUO
        CALL gmul
        sta SOT2
        lda SOD
        ldx SODO
        CALL gmul
        clc
        adc SOT2
        rts

; ---------- Zeichnen ----------
; Rahmen RX,RY,RW,RH 1 Punkt in Farbe A
so_frame:
        CALL setcolor
        lda RH
        sta SOFR
        lda #1
        sta RH
        CALL fillrect           ; oben
        lda RY
        clc
        adc SOFR
        dec a
        sta RY
        CALL fillrect           ; unten
        lda RY
        sec
        sbc SOFR
        inc a
        sta RY
        lda SOFR
        sta RH
        lda RW
        sta SOFR
        lda #1
        sta RW
        CALL fillrect           ; links
        lda RX
        clc
        adc SOFR
        dec a
        sta RX
        CALL fillrect           ; rechts
        lda RX
        sec
        sbc SOFR
        inc a
        sta RX
        lda SOFR
        sta RW
        rts

; Karte SOC an SOX/SOY, sichtbar SOH Punkte
so_card:
        lda SOX
        sta RX
        lda SOY
        sta RY
        lda #SO_CW
        sta RW
        lda SOH
        sta RH
        lda SOC
        bit #$0080
        bne @up
        lda #SO_BLU             ; Rueckseite: blau, weisser Innenrahmen
        CALL setcolor
        CALL fillrect
        lda SOH
        cmp #6
        bcc @bk
        lda RX
        clc
        adc #3
        sta RX
        lda RY
        clc
        adc #3
        sta RY
        lda #SO_CW-6
        sta RW
        lda SOH
        sec
        sbc #3
        ldx SOH
        cpx #SO_CH
        bne @bh
        sec
        sbc #3
@bh:    sta RH
        lda #WHITE
        jsr so_frame
        lda SOX
        sta RX
        lda SOY
        sta RY
        lda #SO_CW
        sta RW
        lda SOH
        sta RH
@bk:    brl @fr
@up:    lda #WHITE
        CALL setcolor
        CALL fillrect
        lda SOC                 ; Farbe der Schrift
        LG sl_red
        tax
        lda #BLACK
        cpx #0
        beq *+5
        lda #SO_RED
        CALL setcolor
        lda SOX                 ; Wert und kleines Symbol oben links
        clc
        adc #3
        sta SX
        lda SOY
        clc
        adc #2
        sta SY
        lda SOC
        LG sl_rank
        jsr so_rkch
        lda SX
        inc a
        sta SX
        lda SOC
        LG sl_suit
        asl
        tax
        lda so_ssm,x
        sta SPR
        CALL blit1
        lda SOH
        cmp #SO_CH
        bne @fr
        lda SOX                 ; grosses Symbol in der Mitte
        clc
        adc #(SO_CW-11)/2
        sta SX
        lda SOY
        clc
        adc #20
        sta SY
        lda SOC
        LG sl_suit
        asl
        tax
        lda so_sbg,x
        sta SPR
        CALL blit1
        lda SOC                 ; Wert unten rechts
        LG sl_rank
        sta SOT
        lda SOX
        clc
        adc #SO_CW-10
        ldx SOT
        cpx #9
        bne @r10
        sec
        sbc #4
@r10:   sta SX
        lda SOY
        clc
        adc #SO_CH-11
        sta SY
        lda SOT
        jsr so_rkch
@fr:    lda SOX
        sta RX
        lda SOY
        sta RY
        lda #SO_CW
        sta RW
        lda SOH
        sta RH
        cmp #SO_CH
        beq @f2
        inc RH                  ; nur oben und an den Seiten
@f2:    lda #BLACK
        jmp so_frame
; Wert A (0..12) als Zeichen an SX/SY
so_rkch:
        cmp #9
        bne @n
        lda #'1'
        CALL putch
        lda #'0'
        CALL putch
        rts
@n:     tax
        lda LANG
        beq @d
        lda so_rken,x
        bra @p
@d:     lda so_rkde,x
@p:     and #$00FF
        CALL putch
        rts
        .pushseg
        .segment "RODATA"
so_rkde: .byte "A23456789?BDK"
so_rken: .byte "A23456789?JQK"
so_ssm:  .addr s_suit0, s_suit1, s_suit2, s_suit3
so_sbg:  .addr s_big0, s_big1, s_big2, s_big3
        .popseg

; leerer Platz an RX/RY
so_empty:
        lda #SO_CW
        sta RW
        lda #SO_CH
        sta RH
        lda #SO_LG
        jmp so_frame

; Stapel A zeichnen (Hintergrund vorher fuellen, wenn SOT3 = 1)
so_pile:
        sta SOP
        jsr so_pxy
        lda RX
        sta SOX
        lda RY
        sta SOY
        lda SOP
        jsr so_cnt
        sta SON
        lda SOHP                ; gezogene Karten weglassen
        cmp SOP
        bne @n
        lda SOHI
        sta SON
@n:     lda SOP
        cmp #6
        bcc *+5
        jmp @tab
        cmp #1
        beq @w
        lda SON                 ; Talon, Zielstapel: oberste Karte
        bne @t
        lda SOX
        sta RX
        lda SOY
        sta RY
        jmp so_empty
@t:     dec a
        tay
        ldx SOP
        jsr so_get
        ldx SOP                 ; Talon verdeckt
        bne @t2
        and #$007F
@t2:    sta SOC
        lda #SO_CH
        sta SOH
        jmp so_card
@w:     lda SON                 ; Ablage: bis zu 3 aufgefaechert (bei 1 Karte nur die oberste)
        bne @w0
        rts
@w0:    ldx #1
        jsr so_sd3
        beq @w1
        cmp #3
        bcc @w2
        lda #3
@w2:    sta SOT
        bra @w3
@w1:    lda #1
        sta SOT
@w3:    lda SON
        sec
        sbc SOT
        sta SOI
@wl:    lda SOI
        cmp SON
        bcs @r
        tay
        ldx #1
        jsr so_get
        sta SOC
        lda #SO_CH
        sta SOH
        jsr so_card
        lda SOX
        clc
        adc #SO_FAN
        sta SOX
        inc SOI
        bra @wl
@r:     rts
@tab:   lda SOP                 ; Spalte
        jsr so_tlay
        lda SOHP
        cmp SOP
        bne @tn
        lda SOHI
        sta SON
@tn:    lda SON
        bne @tg
        lda SOX
        sta RX
        lda SOY
        sta RY
        jmp so_empty
@tg:    stz SOI
@tl:    lda SOI
        cmp SON
        bcs @r
        tay
        ldx SOP
        jsr so_get
        sta SOC
        lda SOI                 ; sichtbare Hoehe: bis zur naechsten Karte
        inc a
        cmp SON
        bcs @full
        jsr so_ty
        sta SOT
        lda SOI
        jsr so_ty
        sta SOT2
        lda SOT
        sec
        sbc SOT2
        bra @h
@full:  lda #SO_CH
@h:     sta SOH
        lda SOI
        jsr so_ty
        clc
        adc #SO_BY
        sta SOY
        jsr so_card
        inc SOI
        bra @tl
; A = Anzahl auf Stapel (fuer Ablage): bei 3er-Modus A, sonst 0 (Z)
so_sd3: ldx #SD3
        jsr sog
        beq @r
        lda SON
@r:     rts

; Statuszeile
so_status:
        lda #WX+2
        sta RX
        lda #SO_SY
        sta RY
        lda #316
        sta RW
        lda #11
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #BLACK
        CALL setcolor
        lda #WX+6
        sta SX
        lda #SO_SY+2
        sta SY
        lda #so_tpt
        ldx LANG
        beq *+5
        lda #so_tpt_en
        jsr cd_str
        ldx #SSC
        jsr sog
        CALL drawnum
        lda SX
        clc
        adc #12
        sta SX
        lda #so_ttm
        ldx LANG
        beq *+5
        lda #so_ttm_en
        jsr cd_str
        ldx #STS
        jsr sog
        ldx #60
        CALL gdiv
        pha
        lda GMR
        sta SOT2
        pla
        CALL drawnum
        lda #':'
        CALL putch
        lda SOT2
        cmp #10
        bcs @s
        pha
        lda #'0'
        CALL putch
        pla
@s:     CALL drawnum
        ldx #SWON               ; gewonnen
        jsr sog
        beq @b
        lda SX
        clc
        adc #12
        sta SX
        lda #so_twon
        ldx LANG
        beq *+5
        lda #so_twon_en
        jsr cd_str
@b:     stz SOT                 ; Knoepfe: Neu, Zurueck, 1/3
@bl:    lda SOT
        jsr so_brect
        lda #0
        jsr cd_btn
        lda #BLACK
        CALL setcolor
        lda SOT
        jsr so_brect
        lda RX
        clc
        adc #4
        sta SX
        lda RY
        inc a
        sta SY
        lda SOT
        asl
        tax
        lda LANG
        beq @de
        lda so_btxe,x
        bra @bt
@de:    lda so_btx,x
@bt:    ldx SOT                 ; dritter Knopf: Zustand
        cpx #2
        bne @bs
        ldx #SD3
        jsr sog
        beq @b1
        lda #so_b3
        ldx LANG
        beq @bs
        lda #so_b3_en
        bra @bs
@b1:    lda #so_b1
        ldx LANG
        beq @bs
        lda #so_b1_en
@bs:    jsr cd_str
        inc SOT
        lda SOT
        cmp #3
        bcc @bl
        rts
; Knopf A -> RX..RH
so_brect:
        asl
        tax
        lda so_bx,x
        sta RX
        lda so_bw,x
        sta RW
        lda #SO_SY
        sta RY
        lda #11
        sta RH
        rts
        .pushseg
        .segment "RODATA"
so_bx:  .word WX+168, WX+206, WX+262
so_bw:  .word 36, 54, 54
        .popseg
        .pushseg
        .segment "RODATA"
so_btx:  .addr so_bnew, so_bund
so_btxe: .addr so_bnew_en, so_bund_en
        .popseg
so_tpt:    .byte 8,"Punkte: "
so_tpt_en: .byte 7,"Score: "
so_ttm:    .byte 6,"Zeit: "
so_ttm_en: .byte 6,"Time: "
so_twon:   .byte 9,"Gewonnen!"
so_twon_en: .byte 8,"You won!"
so_bnew:   .byte 3,"Neu"
so_bnew_en: .byte 3,"New"
so_bund:   .byte 6,"Zur",$82,"ck"
so_bund_en: .byte 4,"Undo"
so_b1:     .byte 7,"1 Karte"
so_b1_en:  .byte 6,"Draw 1"
so_b3:     .byte 8,"3 Karten"
so_b3_en:  .byte 6,"Draw 3"

; ganzes Fenster
so_draw:
        lda #WX+2               ; Tisch
        sta RX
        lda #WY+15
        sta RY
        lda #316
        sta RW
        lda #SO_SY-(WY+15)
        sta RH
        lda #SO_BG
        CALL setcolor
        CALL fillrect
        lda #0
@l:     pha
        jsr so_pile
        pla
        inc a
        cmp #13
        bcc @l
        jmp so_status

; Bereich eines Stapels neu zeichnen (ueber redraw_rect, Fenster ganz oben)
so_rdp: pha
        jsr so_pxy
        lda RX
        jsr so_scx
        dec a
        dec a
        sta KL
        clc
        adc #SO_CW+4
        sta KR
        pla
        cmp #1                  ; Ablage: aufgefaechert
        bne @w
        lda KR
        clc
        adc #2*SO_FAN
        sta KR
@w:     lda RY
        jsr so_scy
        dec a
        dec a
        sta KT
        lda RY
        cmp #SO_BY
        beq @t
        lda KT
        clc
        adc #SO_CH+4
        sta KB
        bra @r
@t:     lda #SO_SY-WY
        sta KB
@r:     CALL redraw_rect
        rts
; virtuelles X/Y -> Bildschirm (Fenster an 0/0)
so_scx: sec
        sbc #WX
        rts
so_scy: sec
        sbc #WY
        rts

; Statuszeile neu
so_rds: lda #2
        sta KL
        lda #318
        sta KR
        lda #SO_SY-WY
        sta KT
        lda #SO_SY-WY+11
        sta KB
        CALL redraw_rect
        rts

; ---------- Sekundentakt ----------
so_tick:
        jsr so_find
        bcc @r
        ldx #SRUN
        jsr sog
        beq @r
        jsr t_now               ; 60 Ticks = 1 s
        sec
        sbc SOSEC
        cmp #60
        bcc @r
        jsr t_now
        sta SOSEC
        ldx #STS
        jsr sog
        inc a
        ldx #STS
        jsr sos
        jmp so_rds
@r:     rts

; ---------- Treffer ----------
; Maus (MX/MY) -> Stapel A und Karte SOI (C=1), sonst C=0
so_hit: lda #0
        sta SOP
@l:     lda SOP
        jsr so_pxy
        lda SOP
        cmp #6
        bcs @tab
        lda #SO_CW
        sta RW
        lda #SO_CH
        sta RH
        lda SOP
        cmp #1                  ; Ablage: aufgefaechert
        bne @h
        lda #SO_CW+2*SO_FAN
        sta RW
@h:     CALL inrect
        bcs @y
@nx:    inc SOP
        lda SOP
        cmp #13
        bcc @l
        clc
        rts
@y:     lda SOP
        jsr so_cnt
        dec a
        sta SOI
        lda SOP
        sec
        rts
@tab:   lda #SO_CW              ; Spalte: bis ganz unten
        sta RW
        lda #SO_SY-SO_BY
        sta RH
        CALL inrect
        bcc @nx
        lda SOP
        jsr so_tlay
        lda SON                 ; Karte von oben nach unten suchen
        sta SOI
@k:     lda SOI
        beq @ke
        dec SOI
        lda SOI
        jsr so_ty
        clc
        adc #SO_BY
        sta RY
        lda #SO_CH
        sta RH
        CALL inrect
        bcc @k
        lda SOP
        sec
        rts
@ke:    lda #$FFFF              ; leere Spalte / unter den Karten
        sta SOI
        lda SOP
        sec
        rts

; ---------- Klick ----------
so_click:
        lda #0                  ; Knoepfe in der Statuszeile
        sta SOT
@b:     lda SOT
        jsr so_brect
        CALL inrect
        bcs @bh
        inc SOT
        lda SOT
        cmp #3
        bcc @b
        bra @c
@bh:    lda SOT
        beq @new
        cmp #1
        beq @und
        ldx #SD3                ; 1/3: umschalten, neues Spiel
        jsr sog
        eor #1
        ldx #SD3
        jsr sos
@new:   jsr so_newg
        jmp so_rdall
@und:   LG sl_undo
        jmp so_rdall
@c:     jsr so_hit
        bcs *+3
        rts
        sta SOP
        cmp #0                  ; Talon
        bne @nt
        LG sl_deal
        lda #0
        jsr so_rdp
        lda #1
        jsr so_rdp
        jmp so_rds
@nt:    lda SOI
        bpl *+3
        rts
        ldy SOI
        ldx SOP
        jsr so_get
        bit #$0080
        bne @up
        lda SOP                 ; verdeckt: oberste umdrehen
        cmp #6
        bcs *+3
        rts
        lda SOP
        jsr so_cnt
        dec a
        cmp SOI
        beq *+3
        rts
        ldx SOP
        LG sl_flip
        lda SOP
        jsr so_rdp
        jmp so_rds
@up:    lda SOP                 ; Ablage / Zielstapel: nur die oberste
        cmp #6
        bcs @dr
        lda SOP
        jsr so_cnt
        dec a
        sta SOI
@dr:    jsr t_now               ; Doppelklick: ablegen
        sec
        sbc SOLT
        cmp #30
        bcs @sd
        lda SOLC
        cmp SOP
        bne @sd
        stz SOLT
        lda SOP
        jsr so_cnt
        dec a
        cmp SOI
        bne @sd
        ldx SOP
        LG sl_auto
        bcc @sd
        jmp so_after
@sd:    jsr t_now
        sta SOLT
        lda SOP
        sta SOLC
        jmp so_drag

; nach einem Zug: gewonnen? sonst neu zeichnen
so_after:
        LG sl_won
        ldx #SWON
        jsr sog
        beq so_rdall
        jsr so_rdall
        jsr so_anim
so_rdall:
        CALL redraw_win
        rts

; ---------- Ziehen ----------
; Karten ab SOI von Stapel SOP mit der Maus bewegen
so_drag:
        lda SOP                 ; Quelle merken (Zeichnen benutzt SOP/SOI)
        sta SOSRC
        lda SOI
        sta SOSI
        lda EVREC+12            ; Griff: Maus relativ zur Karte (Bildschirm)
        sta SOMX
        lda EVREC+10
        sta SOMY
        lda SOSRC
        jsr so_pxy
        lda SOSRC
        cmp #6
        bcc @g1
        jsr so_tlay
        lda SOSI
        jsr so_ty
        clc
        adc RY
        sta RY
@g1:    lda SOSRC                 ; Ablage: oberste Karte liegt rechts
        cmp #1
        bne @g2
        lda SOSRC
        jsr so_cnt
        sta SON
        ldx #1
        jsr so_sd3
        beq @g2
        cmp #3
        bcc @g3
        lda #3
@g3:    dec a
        ldx #SO_FAN
        CALL gmul
        clc
        adc RX
        sta RX
@g2:    lda RX
        jsr so_scx
        sta SOT
        lda SOMX
        sec
        sbc SOT
        sta SOGX
        lda RY
        jsr so_scy
        sta SOT
        lda SOMY
        sec
        sbc SOT
        sta SOGY
        ; erst bewegen, dann ziehen (sonst Klick)
@w:     jsr so_ev
        lda EVREC
        cmp #2
        bne @w1
        rts
@w1:    lda EVREC+12
        sec
        sbc SOMX
        bpl *+6
        eor #$FFFF
        inc a
        cmp #3
        bcs @go
        lda EVREC+10
        sec
        sbc SOMY
        bpl *+6
        eor #$FFFF
        inc a
        cmp #3
        bcc @w
@go:    lda SOHND               ; ohne Merkpuffer kein Ziehen
        ora SOHND+2
        bne @g4
        rts
@g4:    lda SOSRC                 ; Quelle ohne die gezogenen Karten zeichnen
        sta SOHP
        lda SOSI
        sta SOHI
        lda SOSRC
        jsr so_cnt
        sec
        sbc SOSI
        sta SODN                ; Anzahl gezogen (SON benutzt das Zeichnen)
        lda SOSRC
        jsr so_rdp
        CALL cur_hide
        lda SODN                ; Hoehe des Pakets
        dec a
        ldx #11
        CALL gmul
        clc
        adc #SO_CH
        sta SOBH
        stz SOBV
@lp:    lda EVREC+12            ; Lage (Bildschirm)
        sec
        sbc SOGX
        bpl *+5
        lda #0
        cmp #277
        bcc *+5
        lda #276
        sta SOBX
        lda EVREC+10
        sec
        sbc SOGY
        sta SOBY
        jsr so_save
        jsr so_pdraw
@wt:    jsr so_ev
        lda EVREC
        cmp #2
        beq @up
        lda EVREC+12
        sec
        sbc SOGX
        bpl *+5
        lda #0
        cmp #277
        bcc *+5
        lda #276
        cmp SOBX
        bne @mv
        lda EVREC+10
        sec
        sbc SOGY
        cmp SOBY
        beq @wt
@mv:    jsr so_rest
        bra @lp
@up:    jsr so_rest
        lda #$FFFF
        sta SOHP
        CALL cur_show
        ; Ziel: Stapel unter der Mitte des Pakets (oberer Teil)
        lda SOBX
        clc
        adc #SO_CW/2
        sta MX
        lda SOBY
        clc
        adc #8
        sta MY
        jsr so_hit
        bcc @back
        sta SOT
        cmp SOSRC
        beq @back
        lda SOSRC
        ldx #SA
        jsr sos
        lda SOT
        ldx #SB
        jsr sos
        lda SOSI
        ldx #SCI
        jsr sos
        LG sl_move
        bcc @back
        jmp so_after
@back:  lda SOSRC
        jmp so_rdp

; Ereignis holen (wie beim Ziehen von Dateien)
so_ev:  pea $0000
        pea $FFFF
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        rts

; Paket zeichnen: Karten ab SOI von SOP an SOBX/SOBY (Bildschirm)
so_pdraw:
        lda SOBX
        sta SOX
        lda SOBY
        sta SOY
        lda SOSI
        sta SOT2
@l:     lda SOT2
        sec
        sbc SOSI
        cmp SODN
        bcs @r
        ldy SOT2
        ldx SOSRC
        jsr so_get
        sta SOC
        lda #11
        sta SOH
        lda SOT2
        sec
        sbc SOSI
        inc a
        cmp SODN
        bcc @p
        lda #SO_CH
        sta SOH
@p:     CALL org0               ; ganzer Bildschirm (ueber anderen Fenstern)
        jsr so_card
        CALL orgr
        lda SOY
        clc
        adc #11
        sta SOY
        inc SOT2
        bra @l
@r:     rts

; Bildschirmbereich unter dem Paket merken / zurueck
; (Bytes $E1/2000 + y*160 + x/2, Breite 22 Bytes, Hoehe SOBH)
so_save:
        lda #0
        bra so_cpy
so_rest:
        lda #1
so_cpy: sta SOBV
        lda SOHND
        sta PDST
        lda SOHND+2
        sta PDST+2
        ldy #2
        lda [PDST],y
        tax
        lda [PDST]
        sta PDST
        stx PDST+2
        lda #$00E1
        sta PSRC+2
        stz SOT3                ; Zeile
@row:   lda SOT3
        cmp SOBH
        bcs @e
        lda SOBY
        clc
        adc SOT3
        bmi @nx
        cmp #200
        bcs @e
        ldx #160
        CALL gmul
        sta SOT2
        lda SOBX
        bpl *+5
        lda #0
        lsr
        clc
        adc SOT2
        adc #$2000
        sta PSRC
        ldy #0
@b:     lda SOBV
        bne @r1
        lda [PSRC],y
        sta [PDST],y
        bra @b2
@r1:    lda [PDST],y
        sta [PSRC],y
@b2:    iny
        iny
        cpy #22
        bcc @b
@nx:    lda PDST
        clc
        adc #22
        sta PDST
        inc SOT3
        bra @row
@e:     rts

; ---------- Tasten ----------
so_key: lda KEYC
        and #$00FF
        cmp #$1B
        bne @k1
        CALL close_win
        rts
@k1:    cmp #$1A                ; Strg-Z
        beq @u
        ora #$20
        cmp #'u'
        bne @k2
@u:     LG sl_undo
        jmp so_rdall
@k2:    cmp #'n'
        bne @k3
        jsr so_newg
        jmp so_rdall
@k3:    cmp #'d'
        bne @k4
        ldx #SD3
        jsr sog
        eor #1
        ldx #SD3
        jsr sos
        jsr so_newg
        jmp so_rdall
@k4:    cmp #'a'
        bne @k5
        LG sl_save
        LG sl_all
        bcc @r
        jmp so_after
@k5:    cmp #' '                ; Leertaste: Talon
        bne @r
        LG sl_deal
        jmp so_rdall
@r:     rts

; ---------- Gewinn: Karten huepfen von den Zielstapeln ----------
so_anim:
        CALL cur_hide
        lda #12                 ; Koenig zuerst
        sta SOT3
@rk:    lda #2
        sta SOP
@st:    lda SOP                 ; Karte SOT3 des Stapels SOP
        jsr so_pxy
        lda RX
        sta SOVX
        lda RY
        sta SOVX+2
        ldy SOT3
        ldx SOP
        jsr so_get
        sta SOC
        lda #SO_CH
        sta SOH
        LG sl_rnd               ; Richtung und Schwung
        and #$0007
        clc
        adc #3
        sta SOVY+0
        LG sl_rnd
        and #$0010
        beq @rt
        lda SOVY
        eor #$FFFF
        inc a
        sta SOVY
@rt:    lda #$FFFD
        sta SOT2                ; vy
@fl:    lda SOVX
        sta SOX
        lda SOVX+2
        sta SOY
        jsr so_card
        lda SOVX
        clc
        adc SOVY
        sta SOVX
        cmp #WX-SO_CW
        bmi @nx
        cmp #WX+320
        bpl @nx
        lda SOT2
        inc a
        sta SOT2
        clc
        adc SOVX+2
        sta SOVX+2
        cmp #SO_SY-SO_CH
        bmi @fl
        lda #SO_SY-SO_CH        ; aufprallen
        sta SOVX+2
        lda SOT2                ; vy = -(vy - vy/4)
        lsr
        lsr
        sta SOT
        lda SOT2
        sec
        sbc SOT
        eor #$FFFF
        inc a
        sta SOT2
        jsr so_ev               ; Taste/Klick bricht ab
        lda EVREC
        cmp #1
        beq @end
        cmp #3
        beq @end
        bra @fl
@nx:    inc SOP
        lda SOP
        cmp #6
        bcs *+5
        jmp @st
        dec SOT3
        bmi @wt
        jmp @rk
@wt:    jsr so_ev               ; fertig: Bild stehen lassen bis Taste/Klick
        lda EVREC
        cmp #1
        beq @end
        cmp #3
        bne @wt
@end:   CALL cur_show
        rts
