; ================================================================
; ctl95.s - Knoepfe im Win95-Stil (Teil von GSOS95.INIT)
; Standardknoepfe ("simple button", ctlProc $80000000) bekommen eine
; eigene Prozedur: Zeichnen macht sie selbst, alle anderen Nachrichten
; gehen per CallCtlDefProc an Apples Originalprozedur.
; Eigene Direct Page (ctdp).
; ================================================================

; ---- Direct Page ----
CH    = $00     ; 4 Handle des Steuerelements
CP    = $04     ; 4 Zeiger auf den Datensatz
CPRM  = $08     ; 4 Parameter
CMSG  = $0C     ; Nachricht
CRES  = $0E     ; 4 Ergebnis
BT    = $12     ; Rechteck (lokal)
BL    = $14
BB    = $16
BR    = $18
BHL   = $1A     ; ctlHilite
BFL   = $1C     ; ctlFlag
BXS   = $1E     ; 1 / 2 (640er Modus)
KWH   = $20     ; Farben
KBK   = $22
KGR   = $24
KDK   = $26
KMONO = $28
TP    = $2A     ; 4 Titel (Pascal-String)
TRH   = $2E     ; 4 Ressourcen-Handle (0 = keins)
TRID  = $32     ; 4 Ressourcen-ID
GY    = $36     ; globale Zeile
QP    = $38     ; 4 Port
PX0   = $3C     ; Pixelrechteck fuer PaintRect: oben, links, unten, rechts
WK    = $44
CN    = $46     ; 4 Zeiger beim Durchlaufen
CORIG = $4A     ; oberes Wort der Originalprozedur ($0000/$8000 Knopf, $0200/$8200 Ankreuzfeld)
CVAL  = $4C     ; ctlValue
SVT   = $4E     ; gesichertes Rechteck
SVL   = $50
SVB   = $52
SVR   = $54

CPROC_BTN = $8000               ; ctlProc (oberes Wort) eines Standardknopfs

; ---------- nach NewControl(2): alle Knoepfe des Fensters umstellen ----------
ctlwalk:
        lda CH
        ora CH+2
        bne *+3
        rts
        ldy #2                  ; Datensatz -> Besitzer (Port)
        lda [CH],y
        sta CN+2
        lda [CH]
        sta CN
        ldy #4
        lda [CN],y
        sta CP
        ldy #6
        lda [CN],y
        sta CP+2
        ora CP
        bne *+3
        rts
        jmp wwalk
; vorderstes Fenster durchgehen
frontwalk:
        pha                     ; Window Manager aktiv? (sonst liefert
        ldx #$060E              ; FrontWindow Unsinn -> fremder Speicher)
        jsl $E10000             ; WindStatus
        pla
        bcs @no
        cmp #0
        beq @no
        pha
        pha
        ldx #$150E              ; FrontWindow
        jsl $E10000
        pla
        sta CP
        pla
        sta CP+2
        bcs @no
        ora CP
        bne wwalk
@no:    rts
; alle Knoepfe des Fensters CP (Port) umstellen
wwalk:  lda CP                  ; Port -> Fensterdatensatz, wCtls bei 202
        sec
        sbc #4
        sta CP
        lda CP+2
        sbc #0
        sta CP+2
        ldy #202
        lda [CP],y
        sta CN
        ldy #204
        lda [CP],y
        sta CN+2
        ldx #200                ; hoechstens 200 Steuerelemente
@l:     lda CN
        ora CN+2
        bne *+5
        jmp @x
        ldy #2
        lda [CN],y
        sta CP+2
        lda [CN]
        sta CP
        ldy #20                 ; ctlProc unteres Wort muss 0 sein
        lda [CP],y
        bne @n
        ldy #22
        lda [CP],y
        jsr procfor             ; Knopf/Ankreuzfeld -> eigene Prozedur, sonst C=1
        bcs @n
@k:     ldy #20                 ; eigene Prozedur eintragen
        sta [CP],y
        ldy #22
        lda mybank
        sta [CP],y
@n:     ldy #2                  ; naechstes Steuerelement
        lda [CP],y
        sta CN+2
        lda [CP]
        sta CN
        dex
        bne @l
@x:     rts

; oberes Wort der Originalprozedur A -> eigene Prozedur A, C=0 (C=1: keine)
procfor:
        ldy #0
@l:     cmp ptab,y
        beq @f
        iny
        iny
        cpy #8
        bcc @l
        sec
        rts
@f:     lda ptab+8,y
        clc
        rts
ptab:   .word $0000, $8000, $0200, $8200
        .word btnproc0, btnproc8, chkproc0, chkproc8

; ---------- Prozedur fuer Standardknoepfe ----------
; Stapel nach php/phb/phd: 1-2 D, 3 B, 4 P, 5-7 RTL, 8-11 Handle,
; 12-15 Parameter, 16-17 Nachricht, 18-21 Ergebnis
chkproc8:
        php
        rep #$30
        phb
        phd
        phk
        plb
        jsr dpc
        tcd
        lda #$8200
        bra btnc
chkproc0:
        php
        rep #$30
        phb
        phd
        phk
        plb
        jsr dpc
        tcd
        lda #$0200
        bra btnc
btnproc8:
        php
        rep #$30
        phb
        phd
        phk
        plb
        jsr dpc
        tcd
        lda #CPROC_BTN
        bra btnc
btnproc0:
        php
        rep #$30
        phb
        phd
        phk
        plb
        jsr dpc
        tcd
        lda #0
btnc:   sta CORIG
        lda 8,s
        sta CH
        lda 10,s
        sta CH+2
        lda 12,s
        sta CPRM
        lda 14,s
        sta CPRM+2
        lda 16,s
        sta CMSG
        stz CRES
        stz CRES+2
        lda CMSG
        bne @orig
        lda CORIG
        and #$0200
        bne @ck
        jsr bdraw
        jmp @ret
@ck:    jsr cdraw
        jmp @ret
@orig:  jsr deref
        ldy #20                 ; Original eintragen
        lda #0
        sta [CP],y
        ldy #22
        lda CORIG
        sta [CP],y
        pha                     ; CallCtlDefProc(Handle, Nachricht, Parameter)
        pha
        lda CH+2
        pha
        lda CH
        pha
        lda CMSG
        pha
        lda CPRM+2
        pha
        lda CPRM
        pha
        ldx #$2C10
        jsl $E10000
        pla
        sta CRES
        pla
        sta CRES+2
        ; eigene Werte neu vom Stapel (ein anderer Knopf koennte die DP benutzt haben)
        lda 8,s
        sta CH
        lda 10,s
        sta CH+2
        lda 16,s
        cmp #4                  ; dispCtl: Datensatz verschwindet gleich
        beq @ret
        jsr deref
        ldy #22                 ; Original (oberes Wort) noch im Datensatz
        lda [CP],y
        sta CORIG
        jsr procfor
        ldy #20
        sta [CP],y
        ldy #22
        lda mybank
        sta [CP],y
        ; neuer Wert / neue Parameter: Apple hat selbst gezeichnet -> neu
        lda 16,s
        cmp #9
        beq @rd
        cmp #10
        bne @ret
@rd:    lda CRES
        pha
        lda CRES+2
        pha
        lda CORIG
        and #$0200
        bne @rc
        jsr bdraw
        bra @rp
@rc:    jsr cdraw
@rp:    pla
        sta CRES+2
        pla
        sta CRES
@ret:   lda CRES
        sta 18,s
        lda CRES+2
        sta 20,s
        pld
        plb
        plp
        rep #$30
        lda 2,s                 ; Ruecksprung ueber die 10 Parameterbytes
        sta 12,s
        lda 1,s
        sta 11,s
        tsc
        clc
        adc #10
        tcs
        clc
        rtl
deref:  ldy #2
        lda [CH],y
        sta CP+2
        lda [CH]
        sta CP
        rts

; ---------- Knopf zeichnen ----------
bdraw:  jsr deref
        ldy #16
        lda [CP],y
        and #$00FF
        sta BFL
        and #$0080              ; unsichtbar
        beq @v
        rts
@v:     ldy #17
        lda [CP],y
        and #$00FF
        sta BHL
        ldy #8
        lda [CP],y
        sta BT
        ldy #10
        lda [CP],y
        sta BL
        ldy #12
        lda [CP],y
        sta BB
        ldy #14
        lda [CP],y
        sta BR
        ; Port und globale Zeile (fuer Palette und Modus)
        pha
        pha
        ldx #$1C04              ; GetPort
        jsl $E10000
        pla
        sta QP
        pla
        sta QP+2
        lda BT
        inc a
        sta GY
        lda QP
        ora QP+2
        beq @g
        ldy #8                  ; boundsRect.oben: lokal = global + oben
        lda BT
        inc a
        sec
        sbc [QP],y
        sta GY
@g:     jsr kcolors
        jsr ksave
        ; Flaeche
        lda KGR
        ldx KMONO
        beq @f
        lda KWH
@f:     jsr kfillall
        ; Standardknopf: schwarzer Rahmen aussen
        lda BFL
        and #$0001
        beq @nd
        lda KBK
        jsr kframe
        jsr kinset
@nd:    lda KMONO
        beq @3d
        lda KBK                 ; wenig Farben: einfacher Rahmen
        jsr kframe
        lda BHL
        cmp #2
        bne @tt
        jsr kinset
        lda KBK
        jsr kframe
        bra @tt
@3d:    lda BHL
        cmp #2
        bne @up
        lda KBK                 ; gedrueckt: schwarz, innen dunkelgrau
        jsr kframe
        jsr kinset
        lda KDK
        jsr ktopleft
        bra @tt
@up:    lda KWH                 ; erhaben
        jsr ktopleft
        lda KBK
        jsr kbotright
        jsr kinset
        lda KDK
        jsr kbotright
@tt:    jsr ktitle
        jmp krestore

; Farben aus der Palette der Zeile GY (ueber die Routinen der Titelleiste)
kcolors:
        phd
        lda ourdp
        tcd
        lda GY
        bpl @p
        lda #0
@p:     cmp #200
        bcc @q
        lda #199
@q:     dec a                   ; scrmode nimmt die Zeile WFT+1
        sta WFT
        jsr scrmode
        lda XS
        sta kbuf
        lda #$0FFF
        jsr near
        sta kbuf+2
        lda #$0000
        jsr near
        sta kbuf+4
        lda #$0CCC
        jsr near
        sta kbuf+6
        lda #$0777
        jsr near
        sta kbuf+8
        pld
        lda kbuf
        sta BXS
        lda kbuf+2
        sta KWH
        lda kbuf+4
        sta KBK
        lda kbuf+6
        sta KGR
        lda kbuf+8
        sta KDK
        stz KMONO
        lda KGR
        cmp KWH
        bne @x
        inc KMONO
@x:     rts

; QuickDraw-Zustand sichern / wiederherstellen
ksave:  lda mybank
        pha
        pea kpen
        ldx #$2B04              ; GetPenState
        jsl $E10000
        pha
        ldx #$A104              ; GetForeColor
        jsl $E10000
        pla
        sta kfore
        pha
        ldx #$A304              ; GetBackColor
        jsl $E10000
        pla
        sta kback
        pha
        ldx #$9D04              ; GetTextMode
        jsl $E10000
        pla
        sta ktmode
        ldx #$3604              ; PenNormal
        jsl $E10000
        rts
krestore:
        lda mybank
        pha
        pea kpen
        ldx #$2A04              ; SetPenState
        jsl $E10000
        lda kfore
        pha
        ldx #$A004
        jsl $E10000
        lda kback
        pha
        ldx #$A204
        jsl $E10000
        lda ktmode
        pha
        ldx #$9C04
        jsl $E10000
        rts

; Rechteck PX0.. in Farbe A fuellen
kpaint: pha
        ldx #$3704              ; SetSolidPenPat
        jsl $E10000
        pea $0000
        lda ctdp
        clc
        adc #PX0
        pha
        ldx #$5404              ; PaintRect
        jsl $E10000
        rts
; ganze Flaeche BT..BR in Farbe A
kfillall:
        ldx BT
        stx PX0
        ldx BL
        stx PX0+2
        ldx BB
        stx PX0+4
        ldx BR
        stx PX0+6
        jmp kpaint
; Kante oben und links in Farbe A
ktopleft:
        sta WK
        lda BT
        sta PX0
        inc a
        sta PX0+4
        lda BL
        sta PX0+2
        lda BR
        sta PX0+6
        lda WK
        jsr kpaint
        lda BT
        sta PX0
        lda BB
        sta PX0+4
        lda BL
        clc
        adc BXS
        sta PX0+6
        lda WK
        jmp kpaint
; Kante unten und rechts in Farbe A
kbotright:
        sta WK
        lda BB
        sta PX0+4
        dec a
        sta PX0
        lda BL
        sta PX0+2
        lda BR
        sta PX0+6
        lda WK
        jsr kpaint
        lda BT
        sta PX0
        lda BB
        sta PX0+4
        lda BR
        sta PX0+6
        sec
        sbc BXS
        sta PX0+2
        lda WK
        jmp kpaint
; Rahmen ringsum in Farbe A
kframe: pha
        jsr ktopleft
        pla
        jmp kbotright
; Rechteck um eine Linie verkleinern
kinset: inc BT
        dec BB
        lda BL
        clc
        adc BXS
        sta BL
        lda BR
        sec
        sbc BXS
        sta BR
        rts

; Titel zentriert
ktitle: jsr kgettitle
        bcc @go
        rts
@go:    jmp ktdraw
; Titel-Zeiger holen -> TP (C=1: keiner)
kgettitle:
        jsr deref
        stz TRH
        stz TRH+2
        ldy #28                 ; ctlData = Titel-Referenz
        lda [CP],y
        sta TP
        ldy #30
        lda [CP],y
        sta TP+2
        lda CORIG               ; alter NewControl-Aufruf: Titel ist ein Zeiger
        bpl @p
        ldy #60                 ; moreFlags Bits 0-1: Art der Referenz
        lda [CP],y
        and #$0003
        beq @p
        cmp #1
        beq @h
        cmp #2
        bne @x
        lda TP                  ; Ressource rPString ($8006)
        sta TRID
        lda TP+2
        sta TRID+2
        pha
        pha
        pea $8006
        lda TRID+2
        pha
        lda TRID
        pha
        ldx #$0E1E              ; LoadResource (meldet Fehler, haelt nicht an)
        jsl $E10000
        pla
        sta TRH
        pla
        sta TRH+2
        bcs @x0
        lda TRH
        sta TP
        lda TRH+2
        sta TP+2
@h:     lda TP                  ; Handle -> Zeiger
        ora TP+2
        beq @x
        ldy #2
        lda [TP],y
        tax
        lda [TP]
        sta TP
        stx TP+2
@p:     lda TP
        ora TP+2
        beq @x
        clc
        rts
@x0:    stz TRH
        stz TRH+2
@x:     sec
        rts
; Titel TP zentriert im Rechteck zeichnen
ktdraw: pha                     ; Breite
        lda TP+2
        pha
        lda TP
        pha
        ldx #$A904              ; StringWidth
        jsl $E10000
        pla
        sta WK
        lda BL                  ; x = (links + rechts - Breite) / 2
        clc
        adc BR
        sec
        sbc WK
        cmp #$8000
        ror
        ldx BHL
        cpx #2
        bne @1
        inc a
@1:     pha
        lda BT                  ; Grundlinie = Mitte + 3
        clc
        adc BB
        cmp #$8000
        ror
        clc
        adc #3
        ldx BHL
        cpx #2
        bne @2
        inc a
@2:     pha
        ldx #$3A04              ; MoveTo
        jsl $E10000
        lda KBK                 ; inaktiv: dunkelgrau
        ldx BHL
        cpx #$FF
        bne @3
        lda KDK
@3:     pha
        ldx #$A004              ; SetForeColor
        jsl $E10000
        lda KGR
        ldx KMONO
        beq @4
        lda KWH
@4:     pha
        ldx #$A204              ; SetBackColor
        jsl $E10000
        pea $0000
        ldx #$9C04              ; SetTextMode: kopieren
        jsl $E10000
        lda TP+2
        pha
        lda TP
        pha
        ldx #$A504              ; DrawString
        jsl $E10000
        rts

; ---------- Ankreuzfeld zeichnen ----------
; Kaestchen 11 Zeilen hoch (640er: 22 Pixel breit), Haekchen 7x7, Titel rechts
cdraw:  jsr deref
        ldy #16
        lda [CP],y
        and #$00FF
        sta BFL
        and #$0080
        beq @v
        rts
@v:     ldy #17
        lda [CP],y
        and #$00FF
        sta BHL
        ldy #18
        lda [CP],y
        sta CVAL
        ldy #8
        lda [CP],y
        sta BT
        ldy #10
        lda [CP],y
        sta BL
        ldy #12
        lda [CP],y
        sta BB
        ldy #14
        lda [CP],y
        sta BR
        pha
        pha
        ldx #$1C04              ; GetPort
        jsl $E10000
        pla
        sta QP
        pla
        sta QP+2
        lda BT
        inc a
        sta GY
        lda QP
        ora QP+2
        beq @g
        ldy #8
        lda BT
        inc a
        sec
        sbc [QP],y
        sta GY
@g:     jsr kcolors
        jsr ksave
        ; ganzes Rechteck mit dem Hintergrund des Ports loeschen
        jsr deref
        lda CP+2
        pha
        lda CP
        clc
        adc #8
        pha
        ldx #$5504              ; EraseRect
        jsl $E10000
        ; Rechteck sichern, Kaestchen berechnen
        lda BT
        sta SVT
        lda BL
        sta SVL
        lda BB
        sta SVB
        lda BR
        sta SVR
        lda SVB                 ; oben = Mitte - 5
        clc
        adc SVT
        lsr
        sec
        sbc #5
        sta BT
        clc
        adc #11
        sta BB
        lda #11
        ldx BXS
        cpx #2
        bne @w
        asl
@w:     clc
        adc SVL
        sta BR
        ; Flaeche weiss (gedrueckt/inaktiv: grau)
        lda KWH
        ldx BHL
        beq @f
        ldx KMONO
        bne @f
        lda KGR
@f:     jsr kfillall
        lda KMONO
        beq @3d
        lda KBK
        jsr kframe
        jsr kinset
        bra @ck
@3d:    lda KDK                 ; vertieft: aussen dunkel/weiss, innen schwarz/grau
        jsr ktopleft
        lda KWH
        jsr kbotright
        jsr kinset
        lda KBK
        jsr ktopleft
        lda KGR
        jsr kbotright
@ck:    jsr kinset              ; Innenflaeche 7x7
        lda CVAL
        beq @tt
        ldx #0                  ; Haekchen aus Zeilenstuecken
@hl:    lda chkrow,x
        and #$00FF
        cmp #$00FF
        beq @tt
        clc
        adc BT
        sta PX0
        inc a
        sta PX0+4
        lda chkrow+1,x
        and #$00FF
        jsr kxs
        clc
        adc BL
        sta PX0+2
        lda chkrow+2,x
        and #$00FF
        inc a
        jsr kxs
        clc
        adc BL
        sta PX0+6
        phx
        lda KBK
        jsr kpaint
        plx
        inx
        inx
        inx
        bra @hl
@tt:    ; Titel rechts neben dem Kaestchen
        jsr kgettitle
        bcs @x
        lda #15                 ; Kaestchen (11) + 4 Einheiten Abstand
        jsr kxs
        clc
        adc SVL
        pha
        lda SVT
        clc
        adc SVB
        cmp #$8000
        ror
        clc
        adc #3
        pha
        ldx #$3A04              ; MoveTo
        jsl $E10000
        lda KBK
        ldx BHL
        cpx #$FF
        bne @c
        lda KDK
@c:     pha
        ldx #$A004              ; SetForeColor
        jsl $E10000
        pea $0000
        ldx #$9C04              ; Textmodus kopieren (Hintergrund wie der Port)
        jsl $E10000
        lda TP+2
        pha
        lda TP
        pha
        ldx #$A504
        jsl $E10000
@x:     jmp krestore
; A * BXS
kxs:    phx
        ldx BXS
        cpx #2
        bne @r
        asl
@r:     plx
        rts
; Haekchen: Zeile, von x, bis x (Innenflaeche 7x7)
chkrow: .byte 0,6,6, 1,5,6, 2,0,0, 2,4,6, 3,0,1, 3,3,5, 4,0,4, 5,1,3, 6,2,2, $FF

kbuf:   .res 10
kfore:  .word 0
kback:  .word 0
ktmode: .word 0
kpen:   .res 64
