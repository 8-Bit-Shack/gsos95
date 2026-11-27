; ================================================================
; GS/OS95: veraenderbare Fenstergroesse, Bildlaufleiste, Maximieren
; ================================================================
; CW/CH = Groesse des aktuellen Fensters. Rechts/unten verankerte
; Elemente werden mit konstanten WX+WW/WY+WH-Koordinaten gezeichnet,
; nachdem anc_r/anc_b den Ursprung um CW-WW bzw. CH-WH verschoben hat.

SBW   = 12                      ; Breite der Bildlaufleiste
MINW  = 200
MINH  = 96
LROW0 = 40                      ; erste Listenzeile relativ zu WY

; ---------- Ursprung verschieben ----------
anc_r:  lda DXO
        sta ADX
        lda DYO
        sta ADY
        lda DXO
        clc
        adc CW
        sec
        sbc #WW
        sta DXO
        rts
anc_rb: jsr anc_r
        bra anc_b2
anc_b:  lda DXO
        sta ADX
        lda DYO
        sta ADY
anc_b2: lda DYO
        clc
        adc CH
        sec
        sbc #WH
        sta DYO
        rts
anc_end:
        lda ADX
        sta DXO
        lda ADY
        sta DYO
        rts

; C=1 wenn das aktuelle Fenster in der Groesse veraenderbar ist
; Fenster mit Minimieren-Knopf: Ordnerfenster und Minesweeper
is_mini:
        jsr is_resz
        bcs @y
        lda WIN
        cmp #20
        beq @y
        cmp #21
        beq @y
        clc
        rts
@y:     sec
        rts
is_resz:
        lda WIN
        cmp #1
        beq @y
        cmp #4
        beq @y
        cmp #21                 ; Browser
        beq @y
        cmp #28                 ; Editor
        beq @y
        clc
        rts
@y:     sec
        rts

; ---------- kleine Rechenhilfen (eigene Variablen) ----------
; A * X -> A
gmul:   sta GMA             ; Schieben und Addieren (hoechstens 16 Runden)
        stx GMC
        stz GMR
@l:     lda GMC
        beq @r
        lsr GMC
        bcc @n
        lda GMR
        clc
        adc GMA
        sta GMR
@n:     asl GMA
        bra @l
@r:     ldx #0
        lda GMR
        rts
; A / X -> A und X (X > 0), Rest in GMR
gdiv:   sta GMC             ; Dividend -> Quotient
        stx GMA
        stz GMR
        ldx #16
@l:     asl GMC
        rol GMR
        lda GMR
        cmp GMA
        bcc @n
        sbc GMA
        sta GMR
        inc GMC
@n:     dex
        bne @l
        lda GMC
        tax
        rts

; ---------- Raster aus Groesse, Ansicht und Anzahl ----------
win_geom:
        lda WIN
        cmp #21
        bne @nweb
        JSLF xf_web_geom
        rts
@nweb:  stz TW                  ; Ordnerbaum: nur Symbolansicht, nicht im Papierkorb,
        lda TREEOFF             ; und nur wenn daneben noch genug Platz ist
        ora ISBIN               ; (Details: alle drei Spalten muessen passen)
        bne @ntw
        lda WIN
        cmp #1
        bne @ntw
        lda #12+SBW+TREEW+CELLW
        ldx VIEWM
        beq @tc
        lda #12+SBW+TREEW+190
@tc:    cmp CW
        beq @tok
        bcs @ntw
@tok:
        lda #TREEW
        sta TW
@ntw:   lda CW
        sec
        sbc #12+SBW
        sec
        sbc TW
        ldx #CELLW
        jsr gdiv
        bne @c
        inc a
@c:     sta NCOL
        lda CH
        sec
        sbc #50
        ldx #CELLH
        jsr gdiv
        bne @r
        inc a
@r:     sta NROW
        lda VIEWM
        beq @ic
        lda #1
        sta NCV
        lda CH
        sec
        sbc #LROW0+20
        lsr
        lsr
        lsr
        bne @lr
        inc a
@lr:    sta NRV
        bra @pp
@ic:    lda NCOL
        sta NCV
        lda NROW
        sta NRV
@pp:    lda NCV
        ldx NRV
        jsr gmul
        sta PPG
        ; Zeilen insgesamt (aufgerundet)
        lda COUNT
        ldx NCV
        jsr gdiv
        ldx GMR
        beq @nr
        inc a
@nr:    sta TROWS
        sec
        sbc NRV
        bcs @mp
        lda #0
@mp:    sta MAXPG
        stz SBON
        beq @cl
        lda #1
        sta SBON
@cl:    lda PAGE
        cmp MAXPG
        bcc @ok
        beq @ok
        lda MAXPG
        sta PAGE
@ok:    rts

; erster sichtbarer Eintrag -> A
pg_first:
        lda PAGE
        ldx NCV
        jmp gmul

; Eintrag A sichtbar machen. C=1 wenn sich PAGE geaendert hat
ensure_vis:
        ldx NCV
        jsr gdiv                ; Zeile
        cmp PAGE
        bcs @a
        sta PAGE
        sec
        rts
@a:     sta GMB
        lda PAGE
        clc
        adc NRV
        cmp GMB
        beq @b
        bcs @no
@b:     lda GMB
        sec
        sbc NRV
        inc a
        sta PAGE
        sec
        rts
@no:    clc
        rts

; SLOT -> CX (Zellenmitte), IY (Zellenoberkante)
slot_xy:
        lda SLOT
        ldx NCOL
        jsr gdiv                ; A = Zeile, GMR = Spalte
        ldx GMR
        stx GMB
        ldx #CELLH
        jsr gmul
        clc
        adc #GY
        sta IY
        lda GMB
        ldx #CELLW
        jsr gmul
        clc
        adc #GX+CELLW/2
        clc
        adc TW
        sta CX
        rts

; Maus im Raster? C=1, SLOT gesetzt
grid_slot:
        lda #GX
        clc
        adc TW
        sta RX
        lda #GY
        sta RY
        lda NCOL
        ldx #CELLW
        jsr gmul
        sta RW
        lda NROW
        ldx #CELLH
        jsr gmul
        sta RH
        jsr inrect
        bcs @in
        rts
@in:    jsr lmy
        sec
        sbc #GY
        ldx #CELLH
        jsr gdiv
        ldx NCOL
        jsr gmul
        sta SLOT
        jsr lmx
        sec
        sbc #GX
        sec
        sbc TW
        ldx #CELLW
        jsr gdiv
        clc
        adc SLOT
        sta SLOT
        sec
        rts

; ---------- Explorer-Rahmen in Fenstergroesse ----------
exp_frame:
        ; SUNKEN WX+4,WY+28,CW-8,CH-46
        lda #WX+4
        sta RX
        lda #WY+28
        sta RY
        lda CW
        sec
        sbc #8
        sta RW
        lda CH
        sec
        sbc #46
        sta RH
        jsr sunken_r
        ; weisser Inhalt
        lda #WX+6
        sta RX
        lda #WY+30
        sta RY
        lda CW
        sec
        sbc #12
        sta RW
        lda CH
        sec
        sbc #50
        sta RH
        lda #WHITE
        jsr setcolor
        jsr fillrect
        ; Statuszeile
        lda #WX+4
        sta RX
        lda #WY+WH-16
        clc
        adc CH
        sec
        sbc #WH
        sta RY
        lda CW
        sec
        sbc #8
        sta RW
        lda #12
        sta RH
        jsr sunken_r
        ; Groessengriff unten rechts
        jsr is_resz
        bcs *+5
        jmp @r
        jsr anc_rb
        FILL WX+WW-13,WY+WH-6,2,1,DKGRAY
        FILL WX+WW-9,WY+WH-10,2,1,DKGRAY
        FILL WX+WW-9,WY+WH-6,2,1,DKGRAY
        FILL WX+WW-13,WY+WH-5,2,1,WHITE
        FILL WX+WW-9,WY+WH-9,2,1,WHITE
        FILL WX+WW-9,WY+WH-5,2,1,WHITE
        jsr anc_end
@r:     rts

; vertieftes Rechteck RX,RY,RW,RH
sunken_r:
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        jsr bevel
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
        jmp bevel

; erhabener Knopf RX,RY,RW,RH
raised_r:
        lda #GRAY
        jsr setcolor
        jsr fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        jsr bevel
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
        jmp bevel

; ---------- Fensterrahmen in Fenstergroesse ----------
draw_window:
        lda #WX
        sta RX
        lda #WY
        sta RY
        lda CW
        sta RW
        lda CH
        sta RH
        lda #GRAY
        jsr setcolor
        jsr fillrect
        lda #GRAY
        sta C1
        lda #BLACK
        sta C2
        jsr bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #WHITE
        sta C1
        lda #DKGRAY
        sta C2
        jsr bevel
        lda #WX+3
        sta RX
        lda #WY+3
        sta RY
        lda CW
        sec
        sbc #6
        sta RW
        lda #13
        sta RH
        lda ACTW
        jsr setcolor
        jsr fillrect
        ; Titel, abgeschnitten vor den Knoepfen
        lda #WX-34
        clc
        adc CW
        clc
        adc DXO
        sta CLIPR
        lda #WX+6
        sta SX
        lda #WY+6
        sta SY
        lda #WHITE
        jsr setcolor
        jsr draw_title
        lda #320
        sta CLIPR
        jsr anc_r
        BUTTON WX+WW-17,WY+5,13,10
        TEXT WX+WW-14,WY+6,i_close_m,BLACK
        jsr is_resz
        bcs *+5
        jmp @nm
        BUTTON WX+WW-31,WY+5,13,10
        ; Symbol: Maximieren = grosses Rechteck, Wiederherstellen = zwei kleine
        jsr top_maxi
        beq *+5
        jmp @rs
        FILL WX+WW-28,WY+7,7,1,BLACK
        FILL WX+WW-28,WY+8,7,1,BLACK
        FILL WX+WW-28,WY+9,1,4,BLACK
        FILL WX+WW-22,WY+9,1,4,BLACK
        FILL WX+WW-28,WY+12,7,1,BLACK
        jmp @nm
@rs:    FILL WX+WW-26,WY+6,5,1,BLACK
        FILL WX+WW-22,WY+6,1,4,BLACK
        FILL WX+WW-28,WY+8,5,2,BLACK
        FILL WX+WW-28,WY+10,1,3,BLACK
        FILL WX+WW-24,WY+10,1,3,BLACK
        FILL WX+WW-28,WY+12,5,1,BLACK
@nm:    jsr is_resz             ; Minimieren-Knopf
        bcc @nmo
        BUTTON WX+WW-45,WY+5,13,10
        FILL WX+WW-42,WY+11,6,2,BLACK
        jmp @nmi
@nmo:   lda WIN                 ; Minesweeper: Minimieren an der Stelle von Maximieren
        cmp #20
        bne @nmi
        BUTTON WX+WW-31,WY+5,13,10
        FILL WX+WW-28,WY+11,6,2,BLACK
@nmi:   jsr anc_end
        lda WIN
        asl
        tax
        jsr (wincont,x)
        rts

; Z=0 wenn das aktuelle (zuletzt geladene) Fenster maximiert ist
top_maxi:
        lda CURREC
        sta PB2
        ldy #R_MAXI
        lda (PB2),y
        rts

; ---------- Bildlaufleiste ----------
; Spur relativ: oben WY+40, Laenge CH-70
sbar_geom:
        lda CH
        sec
        sbc #70
        sta SBTL
        ; Daumenhoehe = Spur * NRV / TROWS, mind. 8
        ldx NRV
        jsr gmul
        ldx TROWS
        jsr gdiv
        cmp #8
        bcs @h
        lda #8
@h:     cmp SBTL
        bcc @h2
        lda SBTL
@h2:    sta SBTH
        ; Lage = (Spur - Daumen) * PAGE / MAXPG
        lda SBTL
        sec
        sbc SBTH
        ldx PAGE
        jsr gmul
        ldx MAXPG
        jsr gdiv
        clc
        adc #WY+40
        sta SBTY
        rts

draw_sbar:
        lda SBON
        bne @go
        rts
@go:    jsr sbar_geom
        jsr anc_r
        ; Spur
        lda #WX+WW-18
        sta RX
        lda #WY+30
        sta RY
        lda #SBW
        sta RW
        lda CH
        sec
        sbc #50
        sta RH
        lda #GRAY
        jsr setcolor
        jsr fillrect
        ; Pfeil nach oben
        lda #WX+WW-18
        sta RX
        lda #WY+30
        sta RY
        lda #SBW
        sta RW
        lda #10
        sta RH
        jsr raised_r
        FILL WX+WW-13,WY+33,2,1,BLACK
        FILL WX+WW-14,WY+34,4,1,BLACK
        FILL WX+WW-15,WY+35,6,1,BLACK
        ; Daumen
        lda #WX+WW-18
        sta RX
        lda SBTY
        sta RY
        lda #SBW
        sta RW
        lda SBTH
        sta RH
        jsr raised_r
        jsr anc_end
        ; Pfeil nach unten (unten rechts verankert)
        jsr anc_rb
        lda #WX+WW-18
        sta RX
        lda #WY+WH-30
        sta RY
        lda #SBW
        sta RW
        lda #10
        sta RH
        jsr raised_r
        FILL WX+WW-15,WY+WH-27,6,1,BLACK
        FILL WX+WW-14,WY+WH-26,4,1,BLACK
        FILL WX+WW-13,WY+WH-25,2,1,BLACK
        jmp anc_end

; Klick auf die Bildlaufleiste? C=1 wenn behandelt
sbar_click:
        lda SBON
        bne @go
        clc
        rts
@go:    jsr sbar_geom
        jsr anc_r
        lda #WX+WW-18
        sta RX
        lda #WY+30
        sta RY
        lda #SBW
        sta RW
        lda CH
        sec
        sbc #50
        sta RH
        jsr inrect
        bcs @in
        jsr anc_end
        clc
        rts
@in:    jsr lmy
        sta GMB
        jsr anc_end
        lda GMB
        cmp #WY+40
        bcs @n1
        lda PAGE                ; Pfeil hoch
        beq @hd
        dec PAGE
        bra @scr
@n1:    lda CH
        clc
        adc #(WY-30)&$FFFF
        cmp GMB
        beq @dn
        bcs @n2
@dn:    lda PAGE                ; Pfeil runter
        cmp MAXPG
        bcs @hd
        inc PAGE
        bra @scr
@n2:    lda GMB
        cmp SBTY
        bcs @n3
        lda PAGE                ; Spur oberhalb: Seite zurueck
        sec
        sbc NRV
        bpl @set
        lda #0
        bra @set
@n3:    lda SBTY
        clc
        adc SBTH
        cmp GMB
        beq @pd
        bcs @thumb
@pd:    lda PAGE                ; Spur unterhalb: Seite vor
        clc
        adc NRV
        cmp MAXPG
        bcc @set
        lda MAXPG
@set:   cmp PAGE
        beq @hd
        sta PAGE
@scr:   jsr redraw_cont
@hd:    sec
        rts
@thumb: ; Daumen ziehen
        lda GMB
        sec
        sbc SBTY
        sta SBGRAB
@tl:    pea $0000
        pea $FFFF
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        lda EVREC
        cmp #2                  ; mouseUp
        beq @hd
        lda EVREC+10
        sec
        sbc DYO
        sec
        sbc SBGRAB
        sec
        sbc #WY+40              ; Daumenoberkante relativ zur Spur
        bpl @t1
        lda #0
@t1:    ldx MAXPG
        jsr gmul
        sta GMB
        lda SBTL
        sec
        sbc SBTH
        tax
        bne @t2
        ldx #1
@t2:    lda GMB
        jsr gdiv
        cmp MAXPG
        bcc @t3
        lda MAXPG
@t3:    cmp PAGE
        beq @tl
        sta PAGE
        jsr redraw_cont
        bra @tl

; Inhaltsbereich des obersten Fensters neu zeichnen
redraw_cont:
        lda VTON                ; Vollbild-Terminal: nichts zeichnen
        beq *+3
        rts
        lda NWIN
        bne @w
        rts
@w:     dec a
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        clc
        adc #6
        sta UL
        ldy #R_W
        adc (PB),y
        sec
        sbc #12
        sta UR
        ldy #R_Y
        lda (PB),y
        clc
        adc #30
        sta UT
        ldy #R_H
        adc (PB),y
        sec
        sbc #48
        sta UB
        jsr rect_set
        jmp redraw_rect

; ---------- Maximieren / Wiederherstellen ----------
toggle_max:
        jsr top_rect            ; alte Flaeche in UL..UB
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        ldy #R_MAXI
        lda (PB),y
        bne @rest
        ldy #R_X                ; Lage merken
        lda (PB),y
        ldy #R_OX
        sta (PB),y
        ldy #R_Y
        lda (PB),y
        ldy #R_OY
        sta (PB),y
        ldy #R_W
        lda (PB),y
        ldy #R_OW
        sta (PB),y
        ldy #R_H
        lda (PB),y
        ldy #R_OH
        sta (PB),y
        lda #0
        ldy #R_X
        sta (PB),y
        ldy #R_Y
        sta (PB),y
        lda #320
        ldy #R_W
        sta (PB),y
        lda #186
        ldy #R_H
        sta (PB),y
        lda #1
        bra @set
@rest:  ldy #R_OX
        lda (PB),y
        ldy #R_X
        sta (PB),y
        ldy #R_OY
        lda (PB),y
        ldy #R_Y
        sta (PB),y
        ldy #R_OW
        lda (PB),y
        ldy #R_W
        sta (PB),y
        ldy #R_OH
        lda (PB),y
        ldy #R_H
        sta (PB),y
        lda #0
@set:   ldy #R_MAXI
        sta (PB),y
        jmp geom_apply

; neue Lage/Groesse des obersten Fensters uebernehmen und alte (UL..UB)
; plus neue Flaeche neu zeichnen
geom_apply:
        jsr win_save
        lda NWIN
        dec a
        jsr win_load
        jsr ensure_sel
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        sta KL
        ldy #R_Y
        lda (PB),y
        sta KT
        jsr rec_size
        jsr rect_union
        jmp redraw_rect

; Auswahl nach Groessenaenderung sichtbar halten
ensure_sel:
        lda SEL2
        bmi @r
        cmp COUNT
        bcs @r
        jsr ensure_vis
@r:     rts

; ---------- Groesse ziehen (Griff unten rechts) ----------
resize_window:
        jsr cur_hide
        jsr org0
        lda MX
        sta DSX
        lda MY
        sta DSY
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        jsr rec_size
        lda UW
        sta DFW
        sta RSW
        lda UH
        sta DFH
        sta RSH
        ldy #R_X
        lda (PB),y
        sta DNX
        sta DOX
        ldy #R_Y
        lda (PB),y
        sta DNY
        sta DOY
        jsr xor_frame
@loop:  pea $0000
        pea $FFFF
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        lda EVREC
        cmp #2                  ; mouseUp
        beq @done
        lda EVREC+12
        sec
        sbc DSX
        clc
        adc RSW
        cmp #MINW
        bpl @w1
        lda #MINW
@w1:    sta TMP
        lda #320
        sec
        sbc DNX
        cmp TMP
        bpl @w2
        sta TMP
@w2:    lda EVREC+10
        sec
        sbc DSY
        clc
        adc RSH
        cmp #MINH
        bpl @h1
        lda #MINH
@h1:    sta TMP2
        lda #186
        sec
        sbc DNY
        cmp TMP2
        bpl @h2
        sta TMP2
@h2:    lda TMP
        cmp DFW
        bne @mv
        lda TMP2
        cmp DFH
        beq @loop
@mv:    jsr xor_frame
        lda TMP
        sta DFW
        lda TMP2
        sta DFH
        jsr xor_frame
        jmp @loop
@done:  jsr xor_frame
        jsr orgr
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        ; alte Flaeche
        lda DOX
        sta KL
        lda DOY
        sta KT
        lda RSW
        sta UW
        lda RSH
        sta UH
        jsr rect_fromk
        lda DFW
        ldy #R_W
        sta (PB),y
        lda DFH
        ldy #R_H
        sta (PB),y
        lda #0
        ldy #R_MAXI
        sta (PB),y
        jmp geom_apply

; ---------- Klicks auf Titelleiste, Knoepfe und Groessengriff ----------
; C=1 wenn behandelt
win_tbar_click:
        jsr anc_r
        HIT WX+WW-17,WY+5,13,10
        bcc @n1
        jsr anc_end
        jsr close_win
        sec
        rts
@n1:    lda WIN                 ; Minesweeper: Minimieren
        cmp #20
        bne @n1r
        HIT WX+WW-31,WY+5,13,10
        bcc @n1r
        jsr anc_end
        jsr win_minimize
        sec
        rts
@n1r:   jsr is_resz
        bcc @n3
        HIT WX+WW-31,WY+5,13,10
        bcc @n2
        jsr anc_end
        jsr toggle_max
        sec
        rts
@n2:    HIT WX+WW-45,WY+5,13,10
        bcc @n2b
        jsr anc_end
        jsr win_minimize
        sec
        rts
@n2b:   jsr anc_end
        jsr anc_rb
        HIT WX+WW-13,WY+WH-13,12,12
        bcc @n3
        jsr anc_end
        jsr resize_window
        sec
        rts
@n3:    jsr anc_end
        lda #WX+3
        sta RX
        lda #WY+3
        sta RY
        lda CW
        sec
        sbc #50
        sta RW
        lda #13
        sta RH
        jsr inrect
        bcs @t
        rts
@t:     jsr is_resz
        bcc @drag
        lda #$7FF0              ; Doppelklick auf die Titelleiste = maximieren
        JSLF xf_dblclick
        bcc @drag
        jsr toggle_max
        sec
        rts
@drag:  jsr drag_window
        sec
        rts
