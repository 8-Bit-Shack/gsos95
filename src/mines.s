; ================================================================
; GS/OS95: Minesweeper (Fensterart 20), 9x9 Felder, 10 Minen
; Feld-Byte: $80 Mine, $40 aufgedeckt, $20 Fahne, $10 vorgemerkt,
; untere 4 Bit = Zahl der Nachbarminen. Liegt in der zweiten Codebank.
; ================================================================
        .segment "XCODE"
FARMODE .set 1
MSZ   = 11                      ; Feldgroesse in Punkten
MFX   = WX+70                   ; Spielfeld links oben
MFY   = WY+48
MNB   = 10                      ; Minen
MSMX  = MFX+42                  ; Smiley-Knopf
MSMY  = MRY+1
MRY   = WY+26

xf_mine_new:
        jsr mine_new
        rtl
mine_new:
        ldx #80
        sep #$20
        lda #0
@c:     sta mgrid,x
        dex
        bpl @c
        rep #$20
        stz MST
        stz MFLAGS
        stz MTIME
        rts

; Zufallszahl 0..80 -> A
m_rnd:  lda MSEED
        asl
        asl
        clc
        adc MSEED
        clc
        adc #$3711
        sta MSEED
        xba
        and #$007F
        cmp #81
        bcs m_rnd
        rts

; Nachbarn von Feld MI -> mnb (Bytes), Anzahl MNBN
m_nbrs: stz MNBN
        lda MI
        ldx #9
        CALL gdiv               ; A = Zeile, GMR = Spalte
        sta MROW
        lda GMR
        sta MCOL
        lda #$FFFF
        sta MDR
@r:     lda #$FFFF
        sta MDC
@c:     lda MDR
        ora MDC
        beq @nx                 ; das Feld selbst
        lda MROW
        clc
        adc MDR
        cmp #9
        bcs @nx
        sta MT
        lda MCOL
        clc
        adc MDC
        cmp #9
        bcs @nx
        pha
        lda MT
        asl
        asl
        asl
        clc
        adc MT                  ; Zeile * 9
        sta MT
        pla
        clc
        adc MT
        ldx MNBN
        sep #$20
        sta mnb,x
        rep #$20
        inc MNBN
@nx:    inc MDC
        lda MDC
        cmp #2
        bne @c
        inc MDR
        lda MDR
        cmp #2
        bne @r
        rts

; Minen legen (nicht auf Feld A), Nachbarzahlen eintragen
m_place:
        sta MEX
        lda FRAMES
        eor MX
        asl
        eor MY
        eor MSEED
        sta MSEED
        lda #MNB
        sta MT2
@p:     jsr m_rnd
        cmp MEX
        beq @p
        tax
        lda mgrid,x
        and #$0080
        bne @p
        sep #$20
        lda #$80
        sta mgrid,x
        rep #$20
        dec MT2
        bne @p
        stz MI
@n:     jsr m_nbrs
        stz MC
        ldy #0
@q:     cpy MNBN
        bcs @s
        lda mnb,y
        and #$00FF
        tax
        lda mgrid,x
        and #$0080
        beq @z
        inc MC
@z:     iny
        bra @q
@s:     ldx MI
        lda mgrid,x
        and #$00F0
        ora MC
        sep #$20
        sta mgrid,x
        rep #$20
        inc MI
        lda MI
        cmp #81
        bcc @n
        rts

; Feld A aufdecken; leere Felder decken ihre Nachbarn mit auf
m_reveal:
        stz MSTK
        jsr @push
@loop:  lda MSTK
        bne @pop
        rts
@pop:   dec MSTK
        ldx MSTK
        lda mstack,x
        and #$00FF
        sta MI
        tax
        lda mgrid,x
        and #$0060
        bne @loop               ; schon offen oder Fahne
        lda mgrid,x
        ora #$0040
        sep #$20
        sta mgrid,x
        rep #$20
        dec MLEFT
        lda mgrid,x
        and #$008F
        bne @loop               ; Zahl oder Mine: keine Ausbreitung
        jsr m_nbrs
        ldy #0
@nb:    cpy MNBN
        bcs @loop
        lda mnb,y
        and #$00FF
        phy
        jsr @push
        ply
        iny
        bra @nb
@push:  tax
        lda mgrid,x
        and #$0050
        bne @pr                 ; offen oder schon vorgemerkt
        lda mgrid,x
        ora #$0010
        sep #$20
        sta mgrid,x
        txa
        ldx MSTK
        sta mstack,x
        rep #$20
        inc MSTK
@pr:    rts

; ---------- Klick ----------
xf_mine_click:
        jsr mine_click
        rtl
mine_click:
        CALL lmx
        sta MLX
        CALL lmy
        sta MLY
        ; Smiley-Knopf: neues Spiel
        lda MLX
        sec
        sbc #MSMX
        cmp #16
        bcs @board
        lda MLY
        sec
        sbc #MSMY
        cmp #16
        bcs @board
        jsr mine_new
        jmp @rd
@board: lda MST
        cmp #2
        bcc *+5
        jmp @r
        lda MLX
        sec
        sbc #MFX
        bcs *+5
        jmp @r
        ldx #MSZ
        CALL gdiv
        cmp #9
        bcc *+5
        jmp @r
        sta MT
        lda MLY
        sec
        sbc #MFY
        bcs *+5
        jmp @r
        ldx #MSZ
        CALL gdiv
        cmp #9
        bcc *+5
        jmp @r
        sta MT2
        asl
        asl
        asl
        clc
        adc MT2
        clc
        adc MT
        sta MCELL
        tax
        lda EVREC+14            ; Wahltaste oder Apfel: Fahne setzen/entfernen
        and #$0900
        beq @open
        lda mgrid,x
        and #$0040
        beq *+5
        jmp @r
        lda mgrid,x
        eor #$0020
        sep #$20
        sta mgrid,x
        rep #$20
        jmp @rd
@open:  lda mgrid,x
        and #$0060
        beq *+5
        jmp @r
        lda MST
        bne @go
        lda MCELL
        jsr m_place
        lda #1
        sta MST
        lda #81-MNB
        sta MLEFT
        pha                     ; Startzeit merken
        pha
        TOOL $2503              ; GetTick
        pla
        sta MT0
        pla
        stz MTIME
@go:    ldx MCELL
        lda mgrid,x
        and #$0080
        beq @safe
        jsr m_elapsed           ; Zeit anhalten
        sta MTIME
        lda #2                  ; Mine getroffen
        sta MST
        bra @rd
@safe:  lda MCELL
        jsr m_reveal
        lda MLEFT
        bne @rd
        jsr m_elapsed
        sta MTIME
        lda #3
        sta MST
        ldx #80                 ; uebrige Minen bekommen Fahnen (Zaehler 000)
@wf:    lda mgrid,x
        and #$0080
        beq @wn
        sep #$20
        lda mgrid,x
        ora #$20
        sta mgrid,x
        rep #$20
@wn:    dex
        bpl @wf
        lda MBEST               ; neue Bestzeit?
        beq @nb
        cmp MTIME
        bcc @rd
        beq @rd
@nb:    lda MTIME
        bne *+3
        inc a                   ; 0 Sekunden zaehlt als 1
        sta MBEST
        CALL save_prefs
@rd:    CALL redraw_win
@r:     rts

; Sekunden seit Spielbeginn -> A (hoechstens 999)
m_elapsed:
        lda MTIME
        cmp #999
        bcs @max
        pha
        pha
        TOOL $2503              ; GetTick
        pla
        sec
        sbc MT0
        tax
        pla
        txa
        cmp #59940
        bcs @max
        ldx #60
        CALL gdiv
        cmp MTIME               ; nie rueckwaerts
        bcs @r
        lda MTIME
@r:     rts
@max:   lda #999
        rts

; Zahl A dreistellig in Rot an SX, Zeile MRY+5
m_lcd:  cmp #1000
        bcc @ok
        lda #999
@ok:    sta MV
        lda #MRY+5
        sta SY
        lda #7
        CALL setcolor
        lda MV
        ldx #100
        CALL gdiv
        clc
        adc #'0'
        CALL putch
        lda GMR
        ldx #10
        CALL gdiv
        pha
        lda GMR
        sta MV
        pla
        clc
        adc #'0'
        CALL putch
        lda MV
        clc
        adc #'0'
        CALL putch
        rts

; ---------- Uhr: einmal je Hauptschleife ----------
xf_mine_tick:
        jsr mine_tick
        rtl
mine_tick:
        lda MST
        cmp #1
        bne @r
        lda MENU
        ora DROP
        bne @r
        jsr m_elapsed
        cmp MTIME
        beq @r
        sta MTIME
        stz MI                  ; Fenster suchen (darf hinten liegen, nicht minimiert)
@w:     lda MI
        cmp NWIN
        bcs @r
        CALL recaddr
        sta PB
        lda (PB)
        cmp #20
        bne @n
        ldy #R_X
        lda (PB),y
        cmp #600
        bcs @n
        clc
        adc #MFX+76-WX
        sta KL
        clc
        adc #21
        sta KR
        ldy #R_Y
        lda (PB),y
        clc
        adc #MRY+2-WY
        sta KT
        clc
        adc #14
        sta KB
        CALL redraw_rect
        rts
@n:     inc MI
        bra @w
@r:     rts

; ---------- Zeichnen ----------
xf_mine_draw:
        jsr mine_draw
        rtl
; Rechteck RX,RY,RW,RH in Farbe A
m_fill: CALL setcolor
        CALL fillrect
        rts
mine_draw:
        ; Kopfleiste (vertieft): Minenzaehler, Smiley, Zeit
        SUNKEN MFX-1,MRY-1,9*MSZ+1,20
        FILL MFX+2,MRY+2,21,14,BLACK
        FILL MFX+76,MRY+2,21,14,BLACK
        lda #MNB                ; Minen minus gesetzte Fahnen
        sta MC
        ldx #80
@fc:    lda mgrid,x
        and #$0020
        beq @fn
        lda MC
        beq @fn
        dec MC
@fn:    dex
        bpl @fc
        lda #MFX+4
        sta SX
        lda MC
        jsr m_lcd
        lda #MFX+78
        sta SX
        lda MTIME
        jsr m_lcd
        ; Smiley-Knopf
        lda #MSMX
        sta RX
        lda #MSMY
        sta RY
        lda #16
        sta RW
        sta RH
        lda #GRAY
        jsr m_fill
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
        lda #i_smile
        ldx MST
        cpx #2
        bcc @fs
        lda #i_smdead
        cpx #3
        bcc @fs
        lda #i_smcool
@fs:    sta SPR
        lda #MSMX+3
        sta SX
        lda #MSMY+3
        sta SY
        CALL blit4
        ; Bestzeit links
        TEXT WX+10,MRY+1,t_mbest,BLACK
        lda #WX+10
        sta SX
        lda #MRY+11
        sta SY
        lda #BLACK
        CALL setcolor
        lda MBEST
        bne @bt
        lda #'-'
        CALL putch
        bra @bd
@bt:    CALL drawnum
        lda #' '
        CALL putch
        lda #'s'
        CALL putch
@bd:    ; Zeile unten: Zustand oder Hinweis
        lda MST
        cmp #2
        bcc @hint
        beq @lost
        TEXTC WX+WW/2,WY+WH-10,t_mwon,5
        bra @fld
@lost:  TEXTC WX+WW/2,WY+WH-10,t_mlost,7
        bra @fld
@hint:  TEXTC WX+WW/2,WY+WH-10,t_mhint,BLACK
@fld:
        ; Spielfeld: dunkles Gitter, darauf die Felder
        lda #MFX-1
        sta RX
        lda #MFY-1
        sta RY
        lda #9*MSZ+1
        sta RW
        sta RH
        lda #DKGRAY
        jsr m_fill
        stz MI
@cell:  lda MI
        ldx #9
        CALL gdiv
        pha
        lda GMR                 ; (gmul veraendert GMR)
        sta MCOL
        pla
        ldx #MSZ
        CALL gmul
        clc
        adc #MFY
        sta MCY
        lda MCOL
        ldx #MSZ
        CALL gmul
        clc
        adc #MFX
        sta MCX
        sta RX
        lda MCY
        sta RY
        lda #MSZ-1
        sta RW
        sta RH
        ldx MI
        lda mgrid,x
        and #$00FF
        sta MV
        and #$0040
        beq *+5
        jmp @open
        lda MST                 ; verloren: Minen zeigen
        cmp #2
        bne @closed
        lda MV
        and #$0080
        bne @mine
@closed:
        lda #MSZ                ; geschlossene Felder stossen aneinander
        sta RW
        sta RH
        lda #GRAY
        jsr m_fill
        lda #WHITE
        sta C1
        lda #DKGRAY
        sta C2
        CALL bevel
        inc RX                  ; zweiter Rand (wie bei Win95 erhaben)
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #WHITE
        sta C1
        lda #DKGRAY
        sta C2
        CALL bevel
        lda MV
        and #$0020
        bne *+5
        jmp @next
        lda MCX                 ; Fahne
        clc
        adc #3
        sta RX
        lda MCY
        clc
        adc #2
        sta RY
        lda #4
        sta RW
        lda #3
        sta RH
        lda #7
        jsr m_fill
        lda MCX
        clc
        adc #6
        sta RX
        lda #1
        sta RW
        lda #6
        sta RH
        lda #BLACK
        jsr m_fill
        bra @next
@mine:  lda #GRAY                ; Mine: rot nur das getroffene Feld
        ldx MI
        cpx MCELL
        bne @mg
        lda #7
@mg:    jsr m_fill
        lda MCX                 ; runde Mine
        sta SX
        lda MCY
        sta SY
        lda #i_qmine
        sta SPR
        CALL blit4
        jmp @next
@open:  lda #GRAY
        jsr m_fill
        lda MV
        and #$000F
        beq @next
        pha
        tax
        lda mcolor-1,x
        and #$00FF
        CALL setcolor
        lda MCX
        clc
        adc #3
        sta SX
        lda MCY
        inc a
        sta SY
        pla
        clc
        adc #'0'
        CALL putch
@next:  inc MI
        lda MI
        cmp #81
        bcs *+5
        jmp @cell
        rts
FARMODE .set 0
        .segment "CODE"
