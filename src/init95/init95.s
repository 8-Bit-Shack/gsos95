; ================================================================
; GSOS95.INIT - Systemzusatz (Permanent Init, Typ $B6)
; Gibt den Fenstern ALLER Programme eine Titelleiste im Win95-Stil:
; blaue Leiste mit weissem Titel, rechts Maximieren- und X-Knopf.
;
; Technik: Der Tool-Dispatcher ($E1/0000) wird umgeleitet. Nach jedem
; NewWindow/NewWindow2 bekommt das neue Fenster (falls es eine
; Titelleiste hat) eine eigene defProc, die die Original-defProc
; aufruft und danach die Titelleiste mit QuickDraw II uebermalt.
; Gezeichnet wird mit QuickDraw im Port des Window Managers, also mit
; dessen Clipping (verdeckte Fenster bleiben unversehrt) und in den
; Farben der aktuellen Palette (320er und 640er Modus).
; ================================================================
        .setcpu "65816"
        .a16
        .i16

; Fensterrecord
WR_DEFPROC = 174
WR_STRUC   = 190
WR_CONT    = 194
WR_FRAME   = 210
F_TITLE    = $8000
F_CLOSE    = $4000
F_ZOOM     = $0100
F_INFO     = $0010
F_HILITE   = $0001
P_DRAG     = 20
P_GOAWAY   = 22
P_ZOOM     = 23
NTAB       = 32                 ; Fenster in der Tabelle (je 8 Bytes)
NDEPTH     = 8                  ; Verschachtelung von NewWindow

; eigene Direct Page
REC  = $00      ; 4 Fensterrecord
PTR  = $04      ; 4
PAL  = $08      ; 4 Palette (lang)
WFT  = $0C
WFL  = $0E
WFB  = $10
WFR  = $12
WCT  = $14
WDF  = $16
RT   = $18      ; Rechteck fuer QuickDraw: oben, links, unten, rechts
RL   = $1A
RB   = $1C
RR   = $1E
BH   = $20      ; Hoehe der Titelleiste
XS   = $22      ; 1 (320) oder 2 (640)
M640 = $24
CBAR = $26
CTXT = $28
CFACE= $2A
CHI  = $2C
CSH  = $2E
CBLK = $30
TGT  = $32
BEST = $34
BESTK= $36
KK   = $38
SUM  = $3A
T0   = $3C
T1   = $3E
T2   = $40
BW   = $42
XL   = $44      ; linker Rand des X-Knopfs
ZL   = $46      ; linker Rand des Maximieren-Knopfs
CX   = $48
CY   = $4A
PRESS= $4C
OPC  = $4E
PRM  = $50      ; 4
RES  = $54      ; 4
TMAX = $58
PEND = $5A      ; rechter Rand fuer den Titel
RGW  = $5C      ; 4 Arbeitsregion (neue Clip-Region)
RGS  = $60      ; 4 gesicherte Clip-Region
RGV  = $64      ; 4 gesicherte visRgn
VIS  = $68      ; 4 visRgn-Handle des WM-Ports
WPT  = $6C      ; 4 Fensterzeiger beim Durchlaufen
SPRT = $70      ; 4 vorheriger Port
WPRT = $74      ; 4 Port des Window Managers
RH   = $78      ; 4 Hilfshandle
DX   = $7C      ; Versatz global -> lokal (boundsRect des Ports)
DY   = $7E
CITX = $80      ; Titelfarbe inaktiver Fenster

        .segment "CODE"
; ---------- Einstieg (GS/OS: A = User-ID, eigener 4-KB-Stapel/DP) ----------
entry:  php
        rep #$30
        phb
        phk
        plb
        sta myid
        phk
        phk
        pla
        and #$00FF
        sta mybank
        ; Direct-Page-Seite in Bank 0
        pha
        pha
        pea $0000
        pea $0300               ; 768 Bytes: Rahmen, Dialoge, Knoepfe
        lda myid
        pha
        pea DPATTR              ; fest, Loeschstufe 1 (NICHT gesperrt: GS/OS darf
        pea $0000               ; den Block vor ProDOS-8-Programmen entfernen)
        pea $0000               ; Bank 0
        ldx #$0902              ; NewHandle
        jsl $E10000
        pla
        sta $00
        pla
        sta $02
        bcc *+5
        jmp @fail
        lda $00                 ; Handle merken und in dpchk eintragen
        sta dphnd
        sta dpm0b+1
        sta dpm1b+1
        sta dpm2+1
        sta dpm3+1
        inc a
        inc a
        sta dpm0+1
        sta dpm1+1
        lda $02
        sta dphnd+2
        sep #$20
        .a8
        sta dpm0+3
        sta dpm1+3
        sta dpm0b+3
        sta dpm1b+3
        sta dpm2+3
        sta dpm3+3
        rep #$20
        .a16
        lda [$00]
        sta ourdp
        clc
        adc #$0100
        sta sfdp
        adc #$0100
        sta ctdp
        ; NewHandle-Speicher ist auf echter Hardware nicht leer -> nullen
        phd
        lda ourdp
        tcd
        ldx #$02FE
@z0:    stz $00,x
        dex
        dex
        bpl @z0
        pld
        ; Tool-Dispatcher umleiten (JML am Anfang von $E1/0000)
        php
        sei
        sep #$20
        .a8
        lda f:$E10000
        cmp #$5C
        bne @no
        lda f:$E10003
        sta jmlorig+3
        rep #$20
        .a16
        lda f:$E10001
        sta jmlorig+1
        lda #.loword(hook)
        sta f:$E10001
        sep #$20
        .a8
        lda mybank
        sta f:$E10003
@no:    rep #$20
        .a16
        plp
@fail:  plb
        plp
        clc
        rtl

; ---------- Tool-Dispatcher ----------
hook:   cpx #$0917              ; SFGetFile (alt)
        bne @s0
        jmp sfget1
@s0:    cpx #$0A17              ; SFPutFile (alt)
        bne @s00
        jmp sfput1
@s00:   cpx #$0E17              ; SFGetFile2
        bne @s1
        jmp sfget2
@s1:    cpx #$0F17              ; SFPutFile2
        bne @s2
        jmp sfput2
@s2:    cpx #$090E              ; NewWindow
        beq @h
        cpx #$610E              ; NewWindow2
        beq @h
        cpx #$0910              ; NewControl
        beq @hc
        cpx #$3110              ; NewControl2
        beq @hc
        cpx #$0A06              ; GetNextEvent
        beq @pre
        cpx #$0F15              ; ModalDialog
        beq @pre
        cpx #$2C15              ; ModalDialog2
        beq @pre
        jmp jmlorig
; vorab: Knoepfe im vordersten Fenster umstellen (Dialoge, die die
; Toolbox intern anlegt, z.B. Standard File oder Alerts)
@pre:   php
        rep #$30
        pha
        phx
        phy
        phb
        phd
        phk
        plb
        jsr dpc
        tcd
        jsr frontwalk
        pld
        plb
        ply
        plx
        pla
        plp
        jmp jmlorig
@hc:    php                     ; wie unten, Fortsetzung ccont
        phb
        phk
        plb
        phy
        ldy depth
        cpy #4*NDEPTH
        bcs @full
        lda 5,s
        sta rets,y
        lda 7,s
        and #$00FF
        sta rets+2,y
        lda #.loword(ccont-1)
        sta 5,s
        bra @bk
@h:     php
        phb
        phk
        plb
        phy
        ; Stapel: 1-2 Y, 3 B, 4 P, 5-7 Ruecksprung
        ldy depth
        cpy #4*NDEPTH
        bcs @full
        lda 5,s
        sta rets,y
        lda 7,s
        and #$00FF
        sta rets+2,y
        lda #.loword(hcont-1)
        sta 5,s
@bk:    sep #$20
        .a8
        lda mybank
        sta 7,s
        rep #$20
        .a16
        iny
        iny
        iny
        iny
        sty depth
@full:  ply
        plb
        plp
jmlorig: .byte $5C, 0, 0, 0

; zurueck vom Tool: oben auf dem Stapel liegt der neue Fensterzeiger
hcont:  php
        phb
        phk
        plb
        rep #$30
        sta savA
        stx savX
        sty savY
        phd
        jsr dpo
        tcd
        ; Stapel: 1-2 D, 3 B, 4 P, 5-8 Fensterzeiger
        lda depth
        sec
        sbc #4
        sta depth
        lda savA
        bne @ni
        lda 5,s
        sec
        sbc #4                  ; Port -> Record
        sta REC
        lda 7,s
        sbc #0
        sta REC+2
        jsr install
        bcc @ni
        jsr redraw
@ni:    pld
        pla                     ; B (unten) und P (oben)
        sta savBP
        ldy depth
        sep #$20
        .a8
        lda rets+2,y
        pha
        rep #$20
        .a16
        lda rets,y
        pha
        lda savBP
        pha
        lda savA
        ldx savX
        ldy savY
        plb
        plp
        rtl

; zurueck von NewControl/NewControl2: oben liegt das Handle
ccont:  php
        phb
        phk
        plb
        rep #$30
        sta savA
        stx savX
        sty savY
        phd
        jsr dpc
        tcd
        lda depth
        sec
        sbc #4
        sta depth
        lda savA
        bne @ni
        lda 5,s
        sta z:CH
        lda 7,s
        sta z:CH+2
        jsr ctlwalk
@ni:    pld
        pla
        sta savBP
        ldy depth
        sep #$20
        .a8
        lda rets+2,y
        pha
        rep #$20
        .a16
        lda rets,y
        pha
        lda savBP
        pha
        lda savA
        ldx savX
        ldy savY
        plb
        plp
        rtl

; ---------- neue defProc eintragen (REC) ----------
install:
        ldy #WR_FRAME
        lda [REC],y
        and #F_TITLE
        bne @t
        clc
        rts
@t:     ldy #WR_DEFPROC
        lda [REC],y
        cmp #.loword(xdef)
        bne @go
        iny
        iny
        lda [REC],y
        cmp mybank
        bne @go
        clc
        rts
@go:    jsr tfind
        bcc @have
        jsr tfree
        bcc @have
        jsr tpurge
        jsr tfree
        bcc @have
        clc
        rts                     ; Tabelle voll: Fenster bleibt im Apple-Stil
@have:  lda REC
        sta wtab,x
        lda REC+2
        sta wtab+2,x
        ldy #WR_DEFPROC
        lda [REC],y
        sta wtab+4,x
        iny
        iny
        lda [REC],y
        sta wtab+6,x
        lda deforig+2
        bne @dk
        lda wtab+4,x
        sta deforig
        lda wtab+6,x
        sta deforig+2
@dk:    ldy #WR_DEFPROC
        lda #.loword(xdef)
        sta [REC],y
        iny
        iny
        lda mybank
        sta [REC],y
        sec
        rts

; ---------- Rahmen eines eben erzeugten Fensters neu zeichnen ----------
; NewWindow hat das Fenster schon im Apple-Stil gezeichnet, bevor wir die
; defProc tauschen konnten. Zweimal HiliteWindow (umschalten und zurueck)
; laesst den Window Manager den Rahmen mit korrektem Clipping neu zeichnen.
; Die defProc benutzt dieselbe DP, deshalb liegen alle Werte auf dem Stapel.
redraw: ldy #WR_FRAME
        lda [REC],y
        bit #$0020              ; fVis
        beq @x
        and #F_HILITE
        pha                     ; 2. Aufruf: urspruenglicher Zustand
        lda REC
        clc
        adc #4
        tax
        lda REC+2
        adc #0
        pha
        phx
        lda 5,s
        eor #1
        pha                     ; 1. Aufruf: umgeschaltet
        lda 5,s
        pha
        lda 5,s
        pha
        ldx #$220E              ; HiliteWindow
        jsl $E10000
        ldx #$220E
        jsl $E10000
@x:     rts

; Eintrag zu REC -> X, C=0 gefunden
tfind:  ldx #0
@l:     lda wtab,x
        cmp REC
        bne @n
        lda wtab+2,x
        cmp REC+2
        bne @n
        clc
        rts
@n:     txa
        clc
        adc #8
        tax
        cpx #8*NTAB
        bcc @l
        sec
        rts
; freier Eintrag -> X, C=0
tfree:  ldx #0
@l:     lda wtab,x
        ora wtab+2,x
        bne @n
        clc
        rts
@n:     txa
        clc
        adc #8
        tax
        cpx #8*NTAB
        bcc @l
        sec
        rts
; Eintraege geschlossener Fenster loeschen
tpurge: ldx #0
@e:     lda wtab,x
        ora wtab+2,x
        beq @nx
        phx
        pha
        pha
        ldx #$520E              ; GetFirstWindow
        jsl $E10000
        pla
        sta PTR
        pla
        sta PTR+2
        plx
@w:     lda PTR
        ora PTR+2
        beq @gone
        lda PTR
        sec
        sbc #4
        sta T0
        lda PTR+2
        sbc #0
        sta T1
        lda T0
        cmp wtab,x
        bne @nw
        lda T1
        cmp wtab+2,x
        beq @nx                 ; noch offen
@nw:    phx
        pha
        pha
        lda PTR+2
        pha
        lda PTR
        pha
        ldx #$2A0E              ; GetNextWindow
        jsl $E10000
        pla
        sta PTR
        pla
        sta PTR+2
        plx
        bra @w
@gone:  stz wtab,x
        stz wtab+2,x
@nx:    txa
        clc
        adc #8
        tax
        cpx #8*NTAB
        bcc @e
        rts

; Original-defProc zu REC in xjml eintragen; C=1: unbekannt
lookup: jsr tfind
        bcs @no
        lda wtab+4,x
        sta xjml+1
        sep #$20
        .a8
        lda wtab+6,x
        sta xjml+3
        rep #$20
        .a16
        clc
        rts
@no:    lda deforig             ; unbekanntes Fenster: zuerst gesehene defProc
        sta xjml+1
        sep #$20
        .a8
        lda deforig+2
        sta xjml+3
        rep #$20
        .a16
        sec
        rts

; ---------- eigene defProc ----------
; Stapel nach php/phb/phd: 1-2 D, 3 B, 4 P, 5-7 RTL, 8-11 Parameter,
; 12-15 Fenster, 16-17 Operation, 18-21 Globals, 22-25 Ergebnis
xdef:   php
        rep #$30
        phb
        phd
        phk
        plb
        jsr dpo
        tcd
        lda 12,s
        sta REC
        lda 14,s
        sta REC+2
        lda 8,s
        sta PRM
        lda 10,s
        sta PRM+2
        lda 16,s
        sta OPC
        lda 18,s                ; Zeiger auf die Window-Manager-Globals
        sta glob
        lda 20,s
        sta glob+2
        jsr lookup
        bcc @k
        jmp @lost
@k:     lda OPC
        beq @draw
        cmp #1
        beq @hit
        jmp @pass
@hit:   lda popflag             ; Aufrufkonvention noch unbekannt: Original
        bpl *+5
        jmp @pass
        jsr geom
        jsr dohit
        bcc *+5
        jmp @pass
        jmp @done
@draw:  jsr geom
        lda WDF
        and #F_TITLE
        bne *+5
        jmp @pass
        lda PRM+2
        and #$7FFF
        ora PRM
        beq @full
        lda PRM+2
        and #$7FFF
        bne @pass
        lda popflag
        bmi @pass
        lda PRM
        cmp #1
        beq @cls
        cmp #2
        bne @pass
        lda WDF                 ; Maximieren-Knopf gedrueckt/normal
        and #F_ZOOM
        beq @d0
        jsr prep
        bcs @d0
        lda PRM+2
        and #$8000
        sta PRESS
        jsr bzoom
        jsr unprep
        bra @d0
@cls:   lda WDF                 ; X-Knopf gedrueckt/normal
        and #F_CLOSE
        beq @d0
        jsr prep
        bcs @d0
        lda PRM+2
        and #$8000
        sta PRESS
        jsr bclose
        jsr unprep
@d0:    stz RES
        stz RES+2
        jmp @done
@full:  ; Original zeichnen lassen, danach (dcont) uebermalen
        lda 5,s
        sta dret
        lda 7,s
        and #$00FF
        sta dret+2
        lda #.loword(dcont-1)
        sta 5,s
        sep #$20
        .a8
        lda mybank
        sta 7,s
        rep #$20
        .a16
        lda REC
        sta drec
        lda REC+2
        sta drec+2
        tsc
        clc
        adc #4                  ; nach pld/plb/plp
        sta ds0
@pass:  pld
        plb
        plp
        jmp xjml
@lost:  pld                     ; Fenster unbekannt: Standard-defProc
        plb
        plp
        jmp xjml
@done:  lda RES
        sta 22,s
        lda RES+2
        sta 24,s
        ldx popflag
        pld
        plb
        plp
        rep #$30
        txa
        beq @np
        lda 2,s                 ; Ruecksprung ueber die 14 Parameterbytes
        sta 16,s
        lda 1,s
        sta 15,s
        tsc
        clc
        adc #14
        tcs
@np:    clc
        rtl
xjml:   .byte $5C, 0, 0, 0

; Original hat gezeichnet: Win95-Titelleiste darueber
dcont:  php
        rep #$30
        phb
        phd
        phk
        plb
        sta savA
        jsr dpo
        tcd
        tsc                     ; S vor php = S + 4
        clc
        adc #4
        sec
        sbc ds0                 ; 3 (nur RTL) oder 17 (RTL + Parameter)
        cmp #17
        beq @p
        stz popflag
        bra @c
@p:     lda #1
        sta popflag
@c:     lda drec
        sta REC
        lda drec+2
        sta REC+2
        jsr geom
        jsr overlay
        pld
        pla
        sta savBP
        sep #$20
        .a8
        lda dret+2
        pha
        rep #$20
        .a16
        lda dret
        pha
        lda savBP
        pha
        lda savA
        plb
        plp
        rtl

; ---------- Masse aus den Regionen ----------
geom:   ldy #WR_STRUC
        jsr rgnptr
        ldy #2
        lda [PTR],y
        sta WFT
        ldy #4
        lda [PTR],y
        sta WFL
        ldy #6
        lda [PTR],y
        sta WFB
        ldy #8
        lda [PTR],y
        sta WFR
        ldy #WR_CONT
        jsr rgnptr
        ldy #2
        lda [PTR],y
        sta WCT
        ldy #WR_FRAME
        lda [REC],y
        sta WDF
        ; Hoehe der Titelleiste
        lda WDF
        and #F_INFO
        bne @inf
        lda WCT
        sec
        sbc WFT
        sec
        sbc #2
        sta BH
        cmp #8
        bcc @r
        cmp #17
        bcs @r
        sta lastbh
@r:     rts
@inf:   lda lastbh          ; Infozeile: titleHeight aus den Globals,
        ldx glob                ; sonst wie bei normalen Fenstern
        bne @i1
        ldx glob+2
        beq @i2
@i1:
        lda glob
        sta PTR
        lda glob+2
        sta PTR+2
        ldy #2
        lda [PTR],y
        dec a
        dec a
@i2:    sta BH
        rts
; Region-Handle bei Offset Y im Record -> PTR = Zeiger auf die Region
rgnptr: lda [REC],y
        sta PTR
        iny
        iny
        lda [REC],y
        sta PTR+2
        ldy #2
        lda [PTR],y
        tax
        lda [PTR]
        sta PTR
        stx PTR+2
        rts

; ---------- Treffer in der Titelleiste ---------- C=0: RES gesetzt
dohit:  lda WDF
        and #F_TITLE
        beq @no
        lda BH
        cmp #8
        bcc @no
        jsr scrmode
        pha
        pha
        ldx #$1C04              ; GetPort
        jsl $E10000
        pla
        sta PTR
        pla
        sta PTR+2
        jsr toloc
        lda PRM                 ; y
        cmp WFT
        bmi @no
        sec
        sbc WFT
        cmp BH
        bcs @no
        lda PRM+2               ; x
        cmp WFL
        bmi @no
        cmp WFR
        bpl @no
        lda WDF
        and #F_CLOSE
        beq @z
        lda PRM+2
        cmp XL
        bmi @z
        lda #P_GOAWAY
        bra @r
@z:     lda WDF
        and #F_ZOOM
        beq @d
        lda PRM+2
        cmp ZL
        bmi @d
        cmp XL
        bpl @d
        lda #P_ZOOM
        bra @r
@d:     lda #P_DRAG
@r:     sta RES
        stz RES+2
        clc
        rts
@no:    sec
        rts

; Modus und Massstab aus dem SCB der Zeile unter der Oberkante
scrmode:
        lda WFT
        inc a
        bpl @p
        lda #0
@p:     cmp #200
        bcc @k
        lda #199
@k:     tax
        lda f:$E19D00,x
        and #$00FF
        sta T0
        and #$0080
        sta M640
        lda #1
        sta XS
        lda M640
        beq @s
        inc XS
@s:     lda T0                  ; Palette: $E1/9E00 + Nr*32
        and #$000F
        asl
        asl
        asl
        asl
        asl
        clc
        adc #$9E00
        sta PAL
        lda #$00E1
        sta PAL+2
        rts

; Knopfpositionen: XL (X-Knopf), ZL (Maximieren), BW, PEND
btnpos: lda #13
        ldx XS
        cpx #2
        bne @w
        lda #22
@w:     sta BW
        lda WFR
        sec
        sbc #3
        ldx XS
        cpx #2
        bne @w2
        dec a
        dec a
@w2:    sta T1                  ; rechter Rand des naechsten Knopfs
        sta PEND
        lda WDF
        and #F_CLOSE
        beq @nx
        lda T1
        sec
        sbc BW
        sta XL
        sta PEND
        sec
        sbc #2
        sta T1
        bra @z
@nx:    lda #$7FFF
        sta XL
@z:     lda WDF
        and #F_ZOOM
        beq @nz
        lda T1
        sec
        sbc BW
        sta ZL
        sta PEND
        rts
@nz:    lda #$7FFF
        sta ZL
        rts

; ---------- Zeichnen ----------
; QuickDraw-Zustand sichern, Farben bestimmen. C=1: nichts zeichnen
prep:   lda BH
        cmp #8
        bcs @go
        sec                     ; zu flach: nichts zeichnen
        rts
@go:
        jsr scrmode
        jsr btnpos
        jsr colors
        ; Zustand sichern
        pha
        ldx #$A104              ; GetForeColor
        jsl $E10000
        pla
        sta sv_fore
        pha
        ldx #$A304              ; GetBackColor
        jsl $E10000
        pla
        sta sv_back
        pha
        ldx #$9D04              ; GetTextMode
        jsl $E10000
        pla
        sta sv_tmode
        pha
        ldx #$9B04              ; GetTextFace
        jsl $E10000
        pla
        sta sv_face
        pha
        ldx #$2F04              ; GetPenMode
        jsl $E10000
        pla
        sta sv_pmode
        lda mybank
        pha
        pea sv_pat
        ldx #$3104              ; GetPenPat
        jsl $E10000
        jsr penv                ; eigener Port + Clipping
        bcc @pv
        rts                     ; C=1: nichts zeichnen
@pv:    lda mybank
        pha
        pea sv_pen
        ldx #$2904              ; GetPen
        jsl $E10000
        pea $0000               ; Stift: kopieren
        ldx #$2E04
        jsl $E10000
        clc
        rts
unprep: lda mybank
        pha
        pea sv_pat
        ldx #$3004              ; SetPenPat
        jsl $E10000
        lda sv_pmode
        pha
        ldx #$2E04
        jsl $E10000
        lda sv_fore
        pha
        ldx #$A004
        jsl $E10000
        lda sv_back
        pha
        ldx #$A204
        jsl $E10000
        lda sv_tmode
        pha
        ldx #$9C04
        jsl $E10000
        lda sv_face
        pha
        ldx #$9A04
        jsl $E10000
        lda sv_pen+2            ; Point: v, h
        pha
        lda sv_pen
        pha
        ldx #$3A04              ; MoveTo(h,v)
        jsl $E10000
        jmp penvx

; ---------- Zeichenumgebung ----------
; Wir zeichnen immer selbst im Port des Window Managers (globale
; Koordinaten). Clip = Rahmenregion des Fensters minus aller sichtbaren
; Fenster davor; visRgn = ganzer Bildschirm. Alles wird in penvx
; wiederhergestellt. C=1: Speicher fehlt, nichts zeichnen.
; Geometrie in lokale Koordinaten des Ports PTR umrechnen
; (lokal = global + boundsRect.oben/links), danach Knopfpositionen neu
toloc:  lda PTR
        ora PTR+2
        bne @k
        stz DX
        stz DY
        bra @b
@k:     ldy #8
        lda [PTR],y
        sta DY
        ldy #10
        lda [PTR],y
        sta DX
        lda WFT
        clc
        adc DY
        sta WFT
        lda WFB
        clc
        adc DY
        sta WFB
        lda WCT
        clc
        adc DY
        sta WCT
        lda WFL
        clc
        adc DX
        sta WFL
        lda WFR
        clc
        adc DX
        sta WFR
@b:     jmp btnpos

newrgn: pha
        pha
        ldx #$6704              ; NewRgn
        jsl $E10000
        pla
        sta RH
        pla
        sta RH+2
        rts
penv:   pha
        pha
        ldx #$1C04              ; GetPort
        jsl $E10000
        pla
        sta SPRT
        pla
        sta SPRT+2
        pha
        pha
        ldx #$200E              ; GetWMgrPort
        jsl $E10000
        pla
        sta WPRT
        pla
        sta WPRT+2
        ora WPRT
        bne @p1
        sec
        rts
@p1:    jsr newrgn
        lda RH
        sta RGW
        lda RH+2
        sta RGW+2
        jsr newrgn
        lda RH
        sta RGS
        lda RH+2
        sta RGS+2
        jsr newrgn
        lda RH
        sta RGV
        lda RH+2
        sta RGV+2
        lda RGW
        ora RGW+2
        beq @fail
        lda RGS
        ora RGS+2
        beq @fail
        lda RGV
        ora RGV+2
        bne @ok
@fail:  jsr freergn
        sec
        rts
@ok:    lda WPRT+2
        pha
        lda WPRT
        pha
        ldx #$1B04              ; SetPort(WM-Port)
        jsl $E10000
        lda WPRT
        sta PTR
        lda WPRT+2
        sta PTR+2
        jsr toloc
        lda RGS+2               ; Clip sichern
        pha
        lda RGS
        pha
        ldx #$2504              ; GetClip
        jsl $E10000
        pha
        pha
        ldx #$C904              ; GetVisHandle
        jsl $E10000
        pla
        sta VIS
        pla
        sta VIS+2
        lda VIS+2               ; visRgn sichern
        pha
        lda VIS
        pha
        lda RGV+2
        pha
        lda RGV
        pha
        ldx #$6904              ; CopyRgn(vis, RGV)
        jsl $E10000
        lda VIS+2               ; visRgn = ganzer Bildschirm
        pha
        lda VIS
        pha
        lda mybank
        pha
        pea scrrect
        ldx #$6C04              ; RectRgn
        jsl $E10000
        ; Clip = Rahmenregion minus Fenster davor
        ldy #WR_STRUC+2
        lda [REC],y
        pha
        dey
        dey
        lda [REC],y
        pha
        lda RGW+2
        pha
        lda RGW
        pha
        ldx #$6904              ; CopyRgn(struct, RGW)
        jsl $E10000
        pha
        pha
        ldx #$520E              ; GetFirstWindow
        jsl $E10000
        pla
        sta WPT
        pla
        sta WPT+2
@wl:    lda WPT
        ora WPT+2
        beq @wd
        lda WPT                 ; Record = Port - 4
        sec
        sbc #4
        sta PTR
        lda WPT+2
        sbc #0
        sta PTR+2
        lda PTR                 ; unser Fenster erreicht?
        cmp REC
        bne @oth
        lda PTR+2
        cmp REC+2
        beq @wd
@oth:
        ldy #WR_FRAME
        lda [PTR],y
        and #$0020              ; fVis
        beq @nx
        lda RGW+2
        pha
        lda RGW
        pha
        ldy #WR_STRUC+2
        lda [PTR],y
        pha
        dey
        dey
        lda [PTR],y
        pha
        lda RGW+2
        pha
        lda RGW
        pha
        ldx #$7304              ; DiffRgn(RGW, struct, RGW)
        jsl $E10000
@nx:    pha                     ; Ergebnisplatz
        pha
        lda WPT+2
        pha
        lda WPT
        pha
        ldx #$2A0E              ; GetNextWindow
        jsl $E10000
        pla
        sta WPT
        pla
        sta WPT+2
        bra @wl
@wd:    lda RGW+2               ; Region global -> lokal
        pha
        lda RGW
        pha
        lda DX
        pha
        lda DY
        pha
        ldx #$6F04              ; OffsetRgn
        jsl $E10000
        lda RGW+2
        pha
        lda RGW
        pha
        ldx #$2404              ; SetClip(RGW)
        jsl $E10000
        clc
        rts

penvx:  lda RGS+2               ; Clip zurueck
        pha
        lda RGS
        pha
        ldx #$2404
        jsl $E10000
        lda RGV+2               ; visRgn zurueck
        pha
        lda RGV
        pha
        lda VIS+2
        pha
        lda VIS
        pha
        ldx #$6904              ; CopyRgn(RGV, vis)
        jsl $E10000
        lda SPRT+2
        pha
        lda SPRT
        pha
        ldx #$1B04              ; SetPort(vorher)
        jsl $E10000
freergn:
        ldx #RGW
        jsr @d
        ldx #RGS
        jsr @d
        ldx #RGV
@d:     lda 0,x
        ora 2,x
        beq @e
        lda 2,x
        pha
        lda 0,x
        pha
        phx
        ldx #$6804              ; DisposeRgn
        jsl $E10000
        plx
        stz 0,x
        stz 2,x
@e:     rts

; Titelleiste komplett
overlay:
        lda WDF
        and #F_TITLE
        bne *+3
        rts
        jsr prep
        bcc *+3
        rts
        lda WDF
        and #F_HILITE
        sta T2                  ; aktiv?
        ; Leiste
        lda WFT
        inc a
        sta RT
        clc
        adc BH
        sta RB
        lda WFL
        inc a
        sta RL
        lda WFR
        dec a
        sta RR
        lda CBAR
        ldx T2
        bne @a
        lda CSH                 ; inaktiv: grau
@a:     jsr penc
        jsr fillr
        ; Titel
        jsr title
        ; Knoepfe
        stz PRESS
        lda WDF
        and #F_ZOOM
        beq @nz
        jsr bzoom
@nz:    lda WDF
        and #F_CLOSE
        beq @nc
        jsr bclose
@nc:    jsr unprep
        rts

penc:   pha
        ldx #$3704              ; SetSolidPenPat
        jsl $E10000
        rts
fillr:  lda mybank
        pha
        pea RT
        tdc                     ; Rechteck liegt in der Direct Page (Bank 0)
        clc
        adc #RT
        sta 1,s
        lda #0
        sta 3,s
        ldx #$5404              ; PaintRect
        jsl $E10000
        rts
; Linie von (T0,T1) nach (CX,CY) in Farbe A
line:   jsr penc
        lda T0
        pha
        lda T1
        pha
        ldx #$3A04
        jsl $E10000
        lda CX
        pha
        lda CY
        pha
        ldx #$3C04
        jsl $E10000
        rts

; Fenstertitel links in der Leiste
title:  pha
        pha
        lda REC+2               ; Record -> Port
        tax
        lda REC
        clc
        adc #4
        bcc *+3
        inx
        phx
        pha
        ldx #$0E0E              ; GetWTitle
        jsl $E10000
        pla
        sta PTR
        pla
        sta PTR+2
        ora PTR
        bne *+3
        rts
        lda [PTR]               ; Pascal-String in den Puffer
        and #$00FF
        cmp #60
        bcc *+5
        lda #60
        sta tbuf
        tax
        ldy #1
        sep #$20
        .a8
@cp:    cpx #0
        beq @cd
        lda [PTR],y
        sta tbuf,y
        iny
        dex
        bra @cp
@cd:    rep #$20
        .a16
        pea $0001               ; fett
        ldx #$9A04
        jsl $E10000
        ; Platz bis zu den Knoepfen
        lda PEND
        sec
        sbc WFL
        sec
        sbc #8
        bpl *+5
        lda #0
        sta TMAX
@fit:   lda tbuf
        and #$00FF
        beq @dr
        pha
        lda mybank
        pha
        pea tbuf
        ldx #$A904              ; StringWidth
        jsl $E10000
        pla
        cmp TMAX
        bcc @dr
        beq @dr
        sep #$20                ; ein Zeichen kuerzer
        .a8
        dec tbuf
        rep #$20
        .a16
        bra @fit
@dr:    lda CTXT
        ldx T2
        bne @a
        lda CITX
@a:     pha
        ldx #$A004              ; SetForeColor
        jsl $E10000
        lda CBAR
        ldx T2
        bne @b
        lda CSH
@b:     pha
        ldx #$A204              ; SetBackColor
        jsl $E10000
        pea $0000               ; Textmodus: kopieren
        ldx #$9C04
        jsl $E10000
        lda WFL                 ; Grundlinie: Leiste unten - 2
        clc
        adc #4
        ldx XS
        cpx #2
        bne @x1
        clc
        adc #4
@x1:    pha
        lda WFT
        clc
        adc BH
        dec a
        pha
        ldx #$3A04              ; MoveTo
        jsl $E10000
        lda mybank
        pha
        pea tbuf
        ldx #$A504              ; DrawString
        jsl $E10000
        rts

; Knopf: Rechteck RL..RR (Spalten), Hoehe aus BH; PRESS<>0 gedrueckt
; A = linker Rand
button: sta RL
        clc
        adc BW
        sta RR
        lda WFT
        clc
        adc #2
        sta RT
        lda WFT
        clc
        adc BH
        sta RB
        lda CFACE
        jsr penc
        jsr fillr
        ; Kanten: oben/links hell, unten/rechts dunkel (gedrueckt umgekehrt)
        lda CHI
        ldx PRESS
        beq *+4
        lda CBLK
        sta T2+0
        lda CBLK
        ldx PRESS
        beq *+4
        lda CHI
        sta TGT                 ; dunkle Kante (TGT als Zwischenspeicher)
        lda RL                  ; oben
        sta T0
        lda RT
        sta T1
        lda RR
        dec a
        sta CX
        lda RT
        sta CY
        lda T2
        jsr line
        lda RL                  ; links
        sta T0
        lda RT
        sta T1
        lda RL
        sta CX
        lda RB
        dec a
        sta CY
        lda T2
        jsr line
        lda RL                  ; unten
        sta T0
        lda RB
        dec a
        sta T1
        lda RR
        dec a
        sta CX
        lda RB
        dec a
        sta CY
        lda TGT
        jsr line
        lda RR                  ; rechts
        dec a
        sta T0
        lda RT
        sta T1
        lda RR
        dec a
        sta CX
        lda RB
        dec a
        sta CY
        lda TGT
        jsr line
        ; Mitte
        lda RL
        clc
        adc RR
        lsr
        sta CX
        lda RT
        clc
        adc RB
        lsr
        sta CY
        rts

; X-Knopf
bclose: lda T2                  ; T2 (aktiv?) gegen Ueberschreiben sichern
        pha
        lda XL
        jsr button
        lda CX
        sta T2
        lda CY
        sta TGT
        ldx #0
@two:   phx
        ; Diagonale 1
        lda T2
        sec
        sbc #3
        sbc XS
        clc
        adc 1,s
        sta T0
        lda TGT
        sec
        sbc #3
        sta T1
        lda T2
        clc
        adc #2
        adc XS
        clc
        adc 1,s
        sta CX
        lda TGT
        clc
        adc #2
        sta CY
        lda CBLK
        jsr line
        ; Diagonale 2
        lda T2
        clc
        adc #2
        adc XS
        clc
        adc 1,s
        sta T0
        lda TGT
        sec
        sbc #3
        sta T1
        lda T2
        sec
        sbc #3
        sbc XS
        clc
        adc 1,s
        sta CX
        lda TGT
        clc
        adc #2
        sta CY
        lda CBLK
        jsr line
        plx
        inx
        cpx XS                  ; im 640er Modus zweimal (dickere Linien)
        bcc @two
        pla
        sta T2
        rts

; Maximieren-Knopf (Rahmen mit dicker Oberkante)
bzoom:  lda T2
        pha
        lda ZL
        jsr button
        lda CX
        sec
        sbc #3
        sbc XS
        sta RL
        lda CX
        clc
        adc #3
        adc XS
        sta RR
        lda CY
        sec
        sbc #3
        sta RT
        lda CY
        clc
        adc #3
        sta RB
        lda CBLK
        jsr penc
        lda mybank
        pha
        pea RT
        tdc
        clc
        adc #RT
        sta 1,s
        lda #0
        sta 3,s
        ldx #$5304              ; FrameRect
        jsl $E10000
        lda RL                  ; dicke Oberkante
        sta T0
        lda RT
        inc a
        sta T1
        lda RR
        dec a
        sta CX
        lda RT
        inc a
        sta CY
        lda CBLK
        jsr line
        pla
        sta T2
        rts

; ---------- Farben passend zur Palette ----------
colors: lda #$0008              ; Titelleiste: dunkelblau
        jsr near
        sta CBAR
        lda #$0FFF
        jsr near
        sta CTXT
        sta CHI
        lda #$0CCC
        jsr near
        sta CFACE
        lda #$0888
        jsr near
        sta CSH
        lda #$0000
        jsr near
        sta CBLK
        lda CFACE               ; inaktiver Titel: hellgrau auf grau,
        cmp CSH                 ; bei zu wenigen Farben Kontrast erzwingen
        bne @c1
        lda CBLK
        cmp CSH
        bne @c1
        lda CTXT
@c1:    sta CITX
        rts
; naechste Farbe zu $0RGB in A -> Farbnummer (320: 0-15, 640: 0-3)
near:   sta TGT
        lda #$7FFF
        sta BEST
        stz BESTK
        stz KK
@k:     lda M640
        bne @m6
        lda KK                  ; 320: Eintrag KK
        asl
        tay
        lda [PAL],y
        jsr dist
        bra @cmp
@m6:    stz SUM                 ; 640: Eintraege KK, KK+4, KK+8, KK+12
        lda KK
        asl
        tay
        ldx #4
@g:     phx
        phy
        lda [PAL],y
        jsr dist
        clc
        adc SUM
        sta SUM
        ply
        tya
        clc
        adc #8
        tay
        plx
        dex
        bne @g
        lda SUM
@cmp:   cmp BEST
        bcs @n
        sta BEST
        lda KK
        sta BESTK
@n:     inc KK
        lda KK
        ldx M640
        beq @16
        cmp #4
        bcc @k
        bra @e
@16:    cmp #16
        bcc @k
@e:     lda BESTK
        rts
; Abstand der Farbe A zu TGT (Summe der Betraege der Anteile)
dist:   sta T0
        stz T1
        lda #$0F00
        jsr @one
        lda #$00F0
        jsr @one
        lda #$000F
        jsr @one
        lda T1
        rts
@one:   sta CY
        lda T0
        and CY
        sta CX
        lda TGT
        and CY
        sec
        sbc CX
        bpl @p
        eor #$FFFF
        inc a
@p:     cmp #$0100
        bcc @s1
        xba
        and #$00FF
@s1:    cmp #$0010
        bcc @s2
        lsr
        lsr
        lsr
        lsr
@s2:    clc
        adc T1
        sta T1
        rts

; ---------- Direct Page pruefen ----------
; Der DP-Block ist loeschbar (Loeschstufe 1), damit ProDOS-8-Programme
; starten koennen. Bei jedem Einstieg: noch da? sonst neu anlegen + nullen.
DPATTR = $4115
dpo:    jsr dpchk
        lda ourdp
        rts
dpc:    jsr dpchk
        lda ctdp
        rts
dps:    jsr dpchk
        lda sfdp
        rts
dpchk:  php
        rep #$30
        phx
        phy
dpm0:   lda f:$000002           ; Masterzeiger Bank (Adresse zur Laufzeit eingetragen)
dpm0b:  ora f:$000000           ; | unteres Wort
        bne dpld
        pea $0000               ; neu anlegen
        pea $0300
        lda myid
        pha
        pea DPATTR
        pea $0000
        pea $0000
        lda dphnd+2
        pha
        lda dphnd
        pha
        ldx #$0A02              ; ReAllocHandle
        jsl $E10000
dpm1:   lda f:$000002
dpm1b:  ora f:$000000
        beq dpok                ; kein Speicher (sollte nie passieren)
dpm2:   lda f:$000000           ; unteres Wort = Adresse in Bank 0
        phd
        tcd
        ldx #$02FE
dpz:    stz $00,x
        dex
        dex
        bpl dpz
        tdc
        pld
        bra dpok
dpld:
dpm3:   lda f:$000000
dpok:   sta ourdp
        clc
        adc #$0100
        sta sfdp
        adc #$0100
        sta ctdp
        ply
        plx
        plp
        rts

; ---------- Daten ----------
dphnd:  .res 4
myid:   .word 0
mybank: .word 0
ourdp:  .word 0
sfdp:   .word 0
ctdp:   .word 0
depth:  .word 0
rets:   .res 4*NDEPTH
savA:   .word 0
savX:   .word 0
savY:   .word 0
savBP:  .word 0
popflag: .word $FFFF            ; unbekannt, bis das Original einmal gezeichnet hat
dret:   .res 4
drec:   .res 4
ds0:    .word 0
deforig: .res 4
glob:   .word 0, 0
lastbh: .word 11
sv_fore: .word 0
sv_back: .word 0
sv_tmode: .word 0
sv_face: .word 0
sv_pmode: .word 0
sv_pen: .res 4
scrrect: .word 0, 0, 200, 640
sv_pat: .res 32
tbuf:   .res 64
wtab:   .res 8*NTAB

        .include "sf95.s"
        .include "ctl95.s"
