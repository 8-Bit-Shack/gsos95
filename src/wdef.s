; ================================================================
; GS/OS95: Win95-Rahmen fuer Zubehoer-Fenster (eigene Window-defProc)
; Nur wDraw und wHit werden selbst behandelt; alles andere (Regionen,
; Ziehen, Groesse, Dispose ...) geht unveraendert an die Original-defProc
; des Fensters. Liegt in der zweiten Code-Bank (XCODE).
; ================================================================
        .segment "XCODE"
FARMODE .set 1

WD_DEFPROC = 174                ; Offsets im Fensterrecord
WD_STRUC   = 190
WD_CONT    = 194
WD_FRAME   = 210
F_TITLE    = $8000
F_CLOSE    = $4000
F_HILITE   = $0001
WDMAX      = 8                  ; Eintraege in wdtab (je 8 Bytes: Fenster, Original)

; Stapel ab WDS (Bank 0) nach den 6 gesicherten Bytes (P, B, D, SAVD):
; WDS+1 SAVD, +3 D, +5 B, +6 P, +7 RTL(3), +10 param, +14 theWindow,
; +18 OperationCode, +20 windGlobals, +24 result
O_SAVD  = 1
O_D     = 3
O_B     = 5
O_PARAM = 10
O_WIN   = 14
O_OP    = 18
O_GLOB  = 20
O_RES   = 24

; ---------- Einstieg (vom Window Manager per JSL) ----------
x_wdef:
        php
        rep #$30
        phb
        phd
        .byte $F4               ; PEA: Hauptbank zweimal (bei Installation gesetzt)
wdbankim: .word 0
        plb
        plb
        lda SAVD
        pha
        tsc
        sta WDS
        lda MYDP
        tcd
        ldx #6                  ; LP2 und PTR des Hauptprogramms sichern
@sv:    lda LP2,x
        sta wdsave,x
        dex
        dex
        bpl @sv
        lda PTR
        sta wdsave+8
        lda PTR+2
        sta wdsave+10
        ldx WDS
        lda f:O_OP,x
        sta WDOP
        lda f:O_WIN,x
        sta WDW
        lda f:O_WIN+2,x
        sta WDW+2
        lda f:O_PARAM,x
        sta WDP
        lda f:O_PARAM+2,x
        sta WDP+2
        lda f:O_D,x
        sta WDD
        lda f:O_B,x
        and #$00FF
        sta WDB
        jsr wd_orig
        lda WDOP
        beq @draw
        cmp #1
        beq @hit
        bra @pass
@draw:  jsr wd_draw
        bcs @pass
        bra @done
@hit:   jsr wd_hit
        bcs @pass
        bra @done
; ---------- unveraendert an das Original weiterreichen ----------
@pass:  jsr wd_restdp
        pla
        sta SAVD
        pld
        plb
        plp
        jmp wdjml               ; Stapel genau wie beim Aufruf
; ---------- selbst behandelt: Ergebnis eintragen, Parameter abraeumen ----------
@done:  ldx WDS
        lda WDR
        sta f:O_RES,x
        lda WDR+2
        sta f:O_RES+2,x
        jsr wd_restdp
        ldx WDPOP               ; vor dem Zurueckholen von B lesen
        pla
        sta SAVD
        pld
        plb
        plp
        rep #$30
        txa
        and #$00FF
        beq @np
        lda 2,s                 ; Ruecksprung ueber die 14 Parameterbytes schieben
        sta 16,s
        lda 1,s
        sta 15,s
        tsc
        clc
        adc #14
        tcs
        clc
        rtl
@np:    clc
        rtl

; Direct-Page-Zeiger des Hauptprogramms zurueck
wd_restdp:
        ldx #6
@l:     lda wdsave,x
        sta LP2,x
        dex
        dex
        bpl @l
        lda wdsave+8
        sta PTR
        lda wdsave+10
        sta PTR+2
        rts

; JML auf die Original-defProc (Operand wird zur Laufzeit gesetzt)
wdjml:  .byte $5C
wdjmla: .byte 0, 0, 0

; Original-defProc zu Fenster WDW suchen und in wdjml eintragen
wd_orig:
        ldx #0
@l:     lda wdtab,x
        cmp WDW
        bne @n
        lda wdtab+2,x
        cmp WDW+2
        beq @f
@n:     txa
        clc
        adc #8
        tax
        cpx #8*WDMAX
        bcc @l
        ldx #0                  ; unbekannt (sollte nicht vorkommen)
@f:     lda #.loword(wdjmla)
        sta LP2
        lda ABANK
        sta LP2+2
        lda wdtab+4,x
        sta [LP2]
        ldy #2
        sep #$20
        .a8
        lda wdtab+6,x
        sta [LP2],y
        rep #$20
        .a16
        rts

; Original als Unterprogramm mit denselben Parametern aufrufen.
; WDPOP: 1 = Original raeumt seine Parameter selbst ab
wd_callorig:
        tsc
        sta WDS2
        pea $0000               ; Ergebnis
        pea $0000
        ldx WDS
        lda f:O_GLOB+2,x
        pha
        lda f:O_GLOB,x
        pha
        lda WDOP
        pha
        lda WDW+2
        pha
        lda WDW
        pha
        lda WDP+2
        pha
        lda WDP
        pha
        phk
        pea @back-1
        jsr wd_restdp
        lda WDD                 ; D und B wie beim Aufruf durch den Window Manager
        pha
        lda WDB
        xba
        ora WDB
        pha
        plb
        plb
        pld
        jmp wdjml
@back:  rep #$30
        .byte $F4               ; PEA Hauptbank
wdbankim2: .word 0
        plb
        plb
        lda MYDP
        tcd
        tsc
        eor #$FFFF
        sec
        adc WDS2                ; WDS2 - S = noch liegende Bytes
        stz WDPOP
        cmp #4
        bne @np
        inc WDPOP
@np:    lda WDS2
        tcs
        rts

; ---------- Rahmenmasse aus den Regionen (globale = lokale Koordinaten) ----------
wd_geom:
        lda WDW
        sta PTR
        lda WDW+2
        sta PTR+2
        ldy #WD_STRUC
        jsr wd_rgn
        ldy #2
        lda [LP2],y
        sta WFT
        ldy #4
        lda [LP2],y
        sta WFL
        ldy #6
        lda [LP2],y
        sta WFB
        ldy #8
        lda [LP2],y
        sta WFR
        ldy #WD_CONT
        jsr wd_rgn
        ldy #2
        lda [LP2],y
        sta WCT
        ldy #WD_FRAME
        lda [PTR],y
        sta WDF
        rts
; Region-Handle bei Offset Y im Fensterrecord -> LP2 = Zeiger auf die Region
wd_rgn: lda [PTR],y
        sta LP2
        iny
        iny
        lda [PTR],y
        sta LP2+2
        ldy #2
        lda [LP2],y
        tax
        lda [LP2]
        sta LP2
        stx LP2+2
        rts

; ---------- wDraw ---------- C=1: nicht behandelt
wd_draw:
        jsr wd_geom
        lda WDF
        and #F_TITLE
        bne @t
        sec
        rts
@t:     lda WDP+2
        and #$7FFF
        ora WDP
        beq @all
        lda WDP+2
        and #$7FFF
        bne @no
        lda WDP
        cmp #1
        beq @close
        cmp #2
        bne @no
        stz WDR                 ; Zoomfeld gibt es im Win95-Rahmen nicht
        stz WDR+2
        clc
        rts
@no:    sec                     ; Info-Leiste: Original
        rts
@close: lda WDF                 ; Schliessfeld (gedrueckt/normal) neu zeichnen
        and #F_HILITE
        beq @cd
        lda WDP+2
        and #$8000
        sta WDPR
        stz WDALL
        jsr wd_overlay
@cd:    stz WDR
        stz WDR+2
        clc
        rts
@all:   jsr wd_callorig         ; Standardrahmen samt Info-Leiste und Rollbalken
        jsr wd_geom
        lda #NAVY               ; aktiv: blau/weiss
        sta WDBC
        lda #WHITE
        sta WDTC
        lda WDF                 ; inaktiv: grau, aber nur wenn kein anderes
        and #F_HILITE           ; Zubehoer-Fenster davor liegt (GS/OS95-Fenster
        bne @act                ; liegen nie ueber Zubehoer-Fenstern)
        jsr wd_isfirst
        bcc @ad
        lda #DKGRAY
        sta WDBC
        lda #GRAY
        sta WDTC
@act:   stz WDPR
        lda #1
        sta WDALL
        jsr wd_overlay
@ad:    stz WDR
        stz WDR+2
        clc
        rts

; Liegt dieses Fenster (WDW = Record) vorn im Window Manager? C=1 ja
wd_isfirst:
        pha
        pha
        TOOL $520E              ; GetFirstWindow
        pla
        sec
        sbc #4                  ; Port -> Record
        sta WDIT
        pla
        sbc #0
        sta WDIT+2
        lda WDIT
        cmp WDW
        bne @n
        lda WDIT+2
        cmp WDW+2
        bne @n
        sec
        rts
@n:     clc
        rts

; Win95-Teile direkt auf den Bildschirm: der Rahmen des aktiven Fensters ist
; nie verdeckt. WDALL<>0: auch Apple-Schliessfeld/Zoomfeld uebermalen,
; WDPR<>0: X-Knopf gedrueckt
wd_overlay:
        jsr wd_onscr            ; teilweise ausserhalb: Apple-Rahmen stehen lassen
        bcs *+3
        rts
        lda WCT                 ; Hoehe der Titelleiste (ohne Randlinien)
        sec
        sbc WFT
        sec
        sbc #2
        sta WBH
        cmp #9
        bcs @ok
        rts                     ; ungewoehnlicher Rahmen: nichts tun
@ok:    lda #$00E1              ; Zeichenroutinen (auch der Mauszeiger) auf den
        sta PTR+2               ; Bildschirm: PTR zeigte auf den Fensterrecord
        lda CURVIS
        pha
        CALL cur_hide
        lda KL
        pha
        lda KT
        pha
        lda KR
        pha
        lda KB
        pha
        CALL clip_full
        CALL org0
        lda WDALL
        bne *+5
        jmp @btn
        lda WFT                 ; ganze Titelleiste dunkelblau
        inc a
        sta RY
        lda WBH
        sta RH
        lda WFL
        inc a
        sta RX
        lda WFR
        sec
        sbc WFL
        dec a
        dec a
        sta RW
        lda WDBC
        CALL setcolor
        CALL fillrect
        lda WDW                 ; Name links in der GS/OS95-Schrift
        clc
        adc #4
        sta TMP
        lda WDW+2
        adc #0
        sta TMP2
        CALL nda_name
        lda WFL
        clc
        adc #5
        sta SX
        lda WBH
        sec
        sbc #7
        lsr
        sec
        adc WFT
        sta SY
        lda WFR
        sec
        sbc #20
        sta CLIPR
        lda WDTC
        CALL setcolor
        CALL drawstr
        lda #320
        sta CLIPR
@btn:   lda WDF
        and #F_CLOSE
        bne *+5
        jmp @e
        jsr wd_brect
        lda #GRAY
        CALL setcolor
        CALL fillrect
        jsr wd_brect
        lda #WHITE
        ldx #BLACK
        ldy WDPR
        beq @b1
        lda #BLACK
        ldx #WHITE
@b1:    sta C1
        stx C2
        CALL bevel
        jsr wd_brect
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #GRAY
        ldx #DKGRAY
        ldy WDPR
        beq @b2
        lda #DKGRAY
        ldx #GRAY
@b2:    sta C1
        stx C2
        CALL bevel
        jsr wd_brect
        lda RX
        clc
        adc #3
        sta SX
        lda RY
        inc a
        sta SY
        lda #i_close_m
        sta SPR
        lda #BLACK
        CALL setcolor
        CALL blit1
@e:     CALL orgr
        pla
        sta KB
        pla
        sta KR
        pla
        sta KT
        pla
        sta KL
        pla
        beq @nc
        CALL cur_show
@nc:    rts
; Rechteck des X-Knopfs (13 x 10, wie bei den eigenen Fenstern)
wd_brect:
        lda WFR
        sec
        sbc #17
        sta RX
        lda WFT
        clc
        adc #2
        sta RY
        lda #13
        sta RW
        lda #10
        sta RH
        rts

; ---------- wHit ---------- C=1: nicht behandelt
wd_hit:
        jsr wd_geom
        jsr wd_onscr            ; Rahmen nicht von uns gezeichnet: Original fragen
        bcc @no
        lda WDF
        and #F_TITLE
        beq @no
        lda WDP                 ; y
        cmp WFT
        bmi @no
        cmp WCT
        bpl @no
        lda WDP+2               ; x
        cmp WFL
        bmi @no
        cmp WFR
        bpl @no
        lda WDF
        and #F_CLOSE
        beq @drag
        lda WDP+2               ; rechte 18 Pixel = X-Knopf
        sec
        sbc WFR
        clc
        adc #18
        bmi @drag
        lda #22                 ; wInGoAway
        bra @r
@drag:  lda #20                 ; wInDrag
@r:     sta WDR
        stz WDR+2
        clc
        rts
@no:    sec
        rts

; ---------- Mausklick auf ein Zubehoer-Fenster (fwptr): X-Knopf? ----------
; C=1: behandelt (Zubehoer ggf. geschlossen)
xf_wdef_click:
        lda PTR
        pha
        lda PTR+2
        pha
        lda LP2
        pha
        lda LP2+2
        pha
        jsr wd_click
        pla
        sta LP2+2
        pla
        sta LP2
        pla
        sta PTR+2
        pla
        sta PTR
        rtl
wd_click:
        lda fwptr
        sec
        sbc #4
        sta WDW
        lda fwptr+2
        sbc #0
        sta WDW+2
        ldx #0
@l:     lda wdtab,x
        cmp WDW
        bne @n
        lda wdtab+2,x
        cmp WDW+2
        beq @f
@n:     txa
        clc
        adc #8
        tax
        cpx #8*WDMAX
        bcc @l
@no:    clc
        rts
@f:     jsr wd_geom
        lda WDF
        and #F_CLOSE
        beq @no
        lda WDF
        and #F_HILITE           ; inaktives Fenster: erst nach vorn holen lassen
        beq @no
        jsr wd_inbtn
        bcc @no
        lda #$00E1              ; Zeichenroutinen wieder auf den Bildschirm
        sta PTR+2
        CALL cur_hide           ; Mauszeiger weg, bevor das Fenster verschwindet
        lda fwptr+2
        pha
        lda fwptr
        pha
        TOOL $1C05              ; CloseNDAByWinPtr (wie bei Esc)
        lda #1
        sta NEEDREF
        sec
        rts
; Titelleiste vollstaendig auf dem Bildschirm? C=1 ja (sonst nichts zeichnen)
wd_onscr:
        lda WFL
        bmi @o
        lda WFT
        bmi @o
        lda WFR
        bmi @o
        cmp #321
        bcs @o
        lda WCT
        bmi @o
        cmp #187
        bcs @o
        sec
        rts
@o:     clc
        rts
; Maus (EVREC) im X-Knopf? C=1 ja
wd_inbtn:
        jsr wd_onscr
        bcs *+3
        rts
        jsr wd_brect
        lda EVREC+12
        sec
        sbc RX
        clc
        adc #2
        bmi @o
        cmp #18
        bcs @o
        lda EVREC+10
        cmp WFT
        bcc @o
        cmp WCT
        bcs @o
        sec
        rts
@o:     clc
        rts

; ---------- Installation (aus nda_style, NSW = Fensterzeiger) ----------
xf_wdef_install:
        lda PTR                 ; PTR/PTR+2 ist der Bildschirmzeiger der Zeichenroutinen
        pha
        lda PTR+2
        pha
        lda LP2
        pha
        lda LP2+2
        pha
        jsr wd_install
        pla
        sta LP2+2
        pla
        sta LP2
        pla
        sta PTR+2
        pla
        sta PTR
        rtl
wd_install:
        lda #.loword(wdbankim)  ; Hauptbank in die PEA-Befehle eintragen
        sta LP2
        lda ABANK
        sta LP2+2
        lda MYBANK
        xba
        ora MYBANK
        sta [LP2]
        lda #.loword(wdbankim2)
        sta LP2
        lda MYBANK
        xba
        ora MYBANK
        sta [LP2]
        lda NSW                 ; Fensterzeiger zeigt auf den GrafPort (Record+4)
        sec
        sbc #4
        sta PTR
        sta WDRP
        lda NSW+2
        sbc #0
        sta PTR+2
        sta WDRP+2
        ldy #WD_FRAME           ; nur Fenster mit Titelleiste
        lda [PTR],y
        and #F_TITLE
        beq @r
        ldy #WD_DEFPROC
        lda [PTR],y
        cmp #.loword(x_wdef)
        bne @new
        ldy #WD_DEFPROC+2
        lda [PTR],y
        cmp ABANK
        beq @r                  ; schon umgestellt
@new:   ldx #0                  ; freien Platz suchen
@l:     lda wdtab,x
        ora wdtab+2,x
        beq @f
        txa
        clc
        adc #8
        tax
        cpx #8*WDMAX
        bcc @l
        rts                     ; voll: Fenster bleibt im Apple-Stil
@f:     lda WDRP
        sta wdtab,x
        lda WDRP+2
        sta wdtab+2,x
        ldy #WD_DEFPROC
        lda [PTR],y
        sta wdtab+4,x
        ldy #WD_DEFPROC+2
        lda [PTR],y
        sta wdtab+6,x
        ldy #WD_DEFPROC
        lda #.loword(x_wdef)
        sta [PTR],y
        ldy #WD_DEFPROC+2
        lda ABANK
        sta [PTR],y
        lda #1
        sta WDPOP
@r:     rts

; Eintraege zu nicht mehr vorhandenen Fenstern loeschen
xf_wdef_sync:
        ldx #0
@e:     phx
        lda wdtab,x
        ora wdtab+2,x
        beq @nx
        pha                     ; Fensterliste durchsuchen
        pha
        TOOL $520E              ; GetFirstWindow
        pla
        sta NSW
        pla
        sta NSW+2
@w:     lda NSW
        ora NSW+2
        beq @gone
        lda 1,s
        tax
        lda NSW
        sec
        sbc #4
        cmp wdtab,x
        bne @wn
        lda NSW+2
        sbc #0
        cmp wdtab+2,x
        beq @nx                 ; noch da
@wn:    pha
        pha
        lda NSW+2
        pha
        lda NSW
        pha
        TOOL $2A0E              ; GetNextWindow
        pla
        sta NSW
        pla
        sta NSW+2
        bra @w
@gone:  lda 1,s
        tax
        stz wdtab,x
        stz wdtab+2,x
@nx:    plx
        txa
        clc
        adc #8
        tax
        cpx #8*WDMAX
        bcs *+5
        jmp @e
        rtl

FARMODE .set 0
        .segment "CODE"
