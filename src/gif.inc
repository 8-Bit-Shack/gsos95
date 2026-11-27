; ================================================================
; F_GS/OS95 Browser: GIF-Bilder anzeigen (Teil von web.s)
; Erstes Bild einer GIF-Datei (bis 32 KB) wird entpackt (LZW), auf die 16
; Farben des Schreibtischs abgebildet, bei Bedarf verkleinert (jeder s-te
; Bildpunkt) und in einer eigenen 64-KB-Bank abgelegt (1 Byte je Punkt).
; LZW-Tabellen liegen dabei im Textbereich der Pufferbank ($8000..$BFFF),
; Farbtabelle und Zuordnung in WB_UDP ($F000..$F3FF).
; ================================================================
; Bildbank (IMBK): $0000..$AFFF Bildpunkte, $B000 Bildadressen, $B800 Optionen,
; $C000..$FFFF LZW-Tabellen (damit der Text der Seite unberuehrt bleibt)
G_PREF  = $C000                 ; 4096 Woerter
G_SUFX  = $E000                 ; 4096 Bytes
G_STK   = $F000                 ; 4096 Bytes
G_PIXE  = $B000                 ; Ende des Bildpunktbereichs
        .macro IRD
        lda [PDST],y
        and #$00FF
        .endmacro
        .macro IWR
        sep #$20
        sta [PDST],y
        rep #$20
        .endmacro
G_PAL   = $F000                 ; 256 x RGB
G_MAP   = $F300                 ; 256 Bytes: GIF-Farbe -> Palette

        .pushseg
        .segment "BSS"
GIFON:  .res 2                  ; Seite ist ein Bild
IMBK:   .res 2                  ; Bank des Bildes
F_IMH:    .res 4
F_GW:     .res 2                  ; Bildgroesse (Quelle)
F_GH:     .res 2
F_GS:     .res 2                  ; Verkleinerung
F_SW:     .res 2                  ; Bildgroesse (Ziel)
F_SH:     .res 2
F_GTR:    .res 2                  ; durchsichtige Farbe ($FFFF = keine)
F_GIL:    .res 2                  ; Zeilensprung
F_GPASS:  .res 2
F_GX:     .res 2
F_GXP:    .res 2
F_GDX:    .res 2
F_GY:     .res 2
F_GKEEP:  .res 2
F_GROWB:  .res 2
F_GDONE:  .res 2
F_GBR:    .res 2                  ; Lesezeiger in den Datenbloecken
F_GBL:    .res 2                  ; Rest im aktuellen Block
F_GAL:    .res 2                  ; Bitspeicher
F_GAH:    .res 2
F_GNB:    .res 2
F_GCS:    .res 2                  ; Codelaenge
F_GMCS:   .res 2
F_GCLR:   .res 2
F_GNXT:   .res 2
F_GOLD:   .res 2
F_GFST:   .res 2
F_GIN:    .res 2
F_GSP:    .res 2
F_GT:     .res 2
F_GT2:    .res 2
F_GNC:    .res 2
F_GPR:    .res 32                 ; Palette zerlegt (Woerter)
F_GPG:    .res 32
F_GPB:    .res 32
F_GBD:    .res 2
F_GQX:    .res 2
F_BASE:   .res 2                  ; Ziel in der Bildbank
F_LIMW:   .res 2                  ; Grenzen: Breite, Hoehe, Flaeche
F_LIMH:   .res 2
F_LIMA:   .res 2
F_GBI:    .res 2
        .popseg

; ---------- Speicher fuer das Bild ----------
gif_mem:
        lda IMBK
        beq @get
        clc
        rts
@get:   pha
        pha
        pea $0001
        pea $0000
        lda USERID
        pha
        pea $C018
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta F_IMH
        pla
        sta F_IMH+2
        bcs @f
        lda F_IMH
        sta PDST
        lda F_IMH+2
        sta PDST+2
        ldy #2
        lda [PDST],y
        and #$00FF
        sta IMBK
        clc
        rts
@f:     sec
        rts

; ---------- GIF im Rohpuffer HEND..BEND zeigen ----------
gif_show:
        jsr net_ptr0
        stz F_BASE              ; ganzes Fenster, ganzer Bildbereich
        lda WRAPW
        sta F_LIMW
        lda #$7FFF
        sta F_LIMH
        lda #G_PIXE
        sta F_LIMA
        jsr gif_dec
        bcc @ok
        WS ws_gifbad
        jmp err_page
@ok:    jsr h2t_init            ; leere Zeilen als Platz fuer das Bild
        lda F_SH
        clc
        adc #LH-1
        ldx #LH
        CALL gdiv
        inc a
        sta F_GT
@nl:    lda #$0A
        jsr out_b
        dec F_GT
        bne @nl
        lda #' '
        jsr out_b
        jsr h2t_fin
        lda #'G'                ; Titel "GIF 320x200"
        jsr t_add
        lda #'I'
        jsr t_add
        lda #'F'
        jsr t_add
        lda #' '
        jsr t_add
        lda F_GW
        jsr t_dec
        lda #'x'
        jsr t_add
        lda F_GH
        jsr t_dec
        lda F_GS
        cmp #2
        bcc @t
        lda #' '
        jsr t_add
        lda #'1'
        jsr t_add
        lda #':'
        jsr t_add
        lda F_GS
        jsr t_dec
@t:     lda #1
        sta GIFON
        jmp page_done
; Zahl A dezimal an den Titel
t_dec:  sta F_GT
        stz F_GT2
        ldx #0
@d:     lda gdpow,x
        sta F_GNC
        ldy #0
        lda F_GT
@s:     cmp F_GNC
        bcc @o
        sbc F_GNC
        iny
        bra @s
@o:     sta F_GT
        tya
        ora F_GT2
        bne @w
        cpx #8
        bne @n
@w:     inc F_GT2
        tya
        clc
        adc #'0'
        phx
        jsr t_add
        plx
@n:     inx
        inx
        cpx #10
        bcc @d
        rts
        .pushseg
        .segment "RODATA"
gdpow:  .word 10000, 1000, 100, 10, 1
        .popseg

; ---------- Entpacken. C=1 bei Fehler ----------
gif_dec:
        jsr gif_mem
        bcc *+3
        rts
        jsr net_ptr0
        ldy HEND
        BRD
        cmp #'G'
        beq *+5
        jmp @bad
        iny
        BRD
        cmp #'I'
        beq *+5
        jmp @bad
        iny
        BRD
        cmp #'F'
        beq *+5
        jmp @bad
        lda #$FFFF
        sta F_GTR
        ldx #G_PAL              ; Farbtabelle leeren (schwarz)
        txy
        lda #0
@cz:    BWR
        iny
        cpy #G_PAL+768
        bcc @cz
        lda HEND
        clc
        adc #10
        tay
        BRD                     ; Kennzeichen
        sta F_GT
        lda HEND
        clc
        adc #13
        sta F_GBR
        lda F_GT
        and #$80
        beq @blk
        jsr g_pal               ; globale Farbtabelle
@blk:   ldy F_GBR
        cpy BEND
        bcs @bad
        BRD
        iny
        sty F_GBR
        cmp #$2C
        beq @img
        cmp #$21
        bne @bad
        BRD                     ; Erweiterung
        iny
        sty F_GBR
        cmp #$F9
        bne @skp
        iny                     ; Groesse, Kennzeichen, Zeit (2), Index
        BRD
        and #1
        beq @skp
        iny
        iny
        iny
        BRD
        sta F_GTR
@skp:   ldy F_GBR                 ; Unterbloecke ueberspringen
@sk1:   cpy BEND
        bcs @bad
        BRD
        beq @sk2
        sec                     ; Y += Laenge + 1
        adc #0
        sta F_GT
        tya
        clc
        adc F_GT
        tay
        bra @sk1
@sk2:   iny
        sty F_GBR
        bra @blk
@bad:   sec
        rts
@img:   lda F_GBR                 ; Bildbeschreibung
        clc
        adc #4
        tay
        BRD
        sta F_GW
        iny
        BRD
        xba
        ora F_GW
        sta F_GW
        iny
        BRD
        sta F_GH
        iny
        BRD
        xba
        ora F_GH
        sta F_GH
        iny
        BRD
        sta F_GT
        and #$40
        sta F_GIL
        iny
        sty F_GBR
        lda F_GW
        beq @bad
        lda F_GH
        beq @bad
        lda F_GT
        and #$80
        beq @np
        jsr g_pal               ; lokale Farbtabelle
@np:    jsr g_map
        jsr g_scale
        bcs @bad
        jsr g_clear
        ldy F_GBR                 ; kleinste Codelaenge
        BRD
        cmp #2
        bcc @bad
        cmp #9
        bcs @bad
        sta F_GMCS
        iny
        sty F_GBR
        stz F_GBL
        jsr g_lzw
        clc
        rts

; Farbtabelle ab F_GBR lesen (Groesse aus F_GT Bits 0..2)
g_pal:  lda F_GT
        and #7
        tax
        lda #2
@sh:    cpx #0
        beq @n
        asl
        dex
        bra @sh
@n:     sta F_GNC                 ; Anzahl Farben
        asl
        clc
        adc F_GNC
        sta F_GT2                 ; Bytes
        ldx #0
@c:     cpx F_GT2
        bcs @e
        txa
        clc
        adc F_GBR
        tay
        BRD
        pha
        txa
        clc
        adc #G_PAL
        tay
        pla
        BWR
        inx
        bra @c
@e:     lda F_GBR
        clc
        adc F_GT2
        sta F_GBR
        rts

; GIF-Farben auf die 16 Farben der Palette 0 abbilden
g_map:  ldx #0                  ; X = 2 * Farbe
@p:     lda f:$E19E00,x
        pha
        and #$000F
        sta F_GPB,x
        pla
        pha
        lsr
        lsr
        lsr
        lsr
        and #$000F
        sta F_GPG,x
        pla
        xba
        and #$000F
        sta F_GPR,x
        inx
        inx
        cpx #32
        bcc @p
        stz F_GBI                 ; jede GIF-Farbe
@f:     lda F_GBI
        asl
        clc
        adc F_GBI
        clc
        adc #G_PAL
        tay
        BRD
        lsr
        lsr
        lsr
        lsr
        sta F_GAL                 ; r
        iny
        BRD
        lsr
        lsr
        lsr
        lsr
        sta F_GAH                 ; g
        iny
        BRD
        lsr
        lsr
        lsr
        lsr
        sta F_GNB                 ; b
        lda #$7FFF
        sta F_GBD
        stz F_GT2
        ldx #0
@q:     stx F_GQX
        lda F_GPR,x
        sec
        sbc F_GAL
        jsr g_sq
        sta F_GT                  ; 3*dr^2
        asl
        clc
        adc F_GT
        sta F_GT
        lda F_GPG,x
        sec
        sbc F_GAH
        jsr g_sq
        asl
        asl
        clc
        adc F_GT                  ; + 4*dg^2
        sta F_GT
        lda F_GPB,x
        sec
        sbc F_GNB
        jsr g_sq
        asl
        clc
        adc F_GT                  ; + 2*db^2
        cmp F_GBD
        bcs @w
        sta F_GBD
        txa
        lsr
        sta F_GT2
@w:     inx
        inx
        cpx #32
        bcc @q
        lda F_GBI
        cmp F_GTR                 ; durchsichtig: weiss wie die Seite
        bne @nt
        lda #WHITE
        sta F_GT2
@nt:    lda F_GBI
        clc
        adc #G_MAP
        tay
        lda F_GT2
        BWR
        inc F_GBI
        lda F_GBI
        cmp #256
        bcs *+5
        jmp @f
        rts
; A (-15..15) zum Quadrat
g_sq:   bpl @p
        eor #$FFFF
        inc a
@p:     tax
        lda g_sqt,x
        and #$00FF
        ldx F_GQX                 ; X wieder herstellen (Schleifenzaehler)
        rts
        .pushseg
        .segment "RODATA"
g_sqt:  .byte 0,1,4,9,16,25,36,49,64,81,100,121,144,169,196,225
        .popseg

; Verkleinerung waehlen: Breite <= WRAPW, Flaeche <= 64000
g_scale:
        lda #1
        sta F_GS
@l:     lda F_GW
        clc
        adc F_GS
        dec a
        ldx F_GS
        CALL gdiv
        sta F_SW
        lda F_GH
        clc
        adc F_GS
        dec a
        ldx F_GS
        CALL gdiv
        sta F_SH
        lda F_SW
        cmp F_LIMW
        beq @w
        bcs @nx
@w:     lda F_SH
        cmp F_LIMH
        beq @w2
        bcs @nx
@w2:    lda F_LIMA              ; F_SH <= Flaeche / F_SW ?
        ldx F_SW
        CALL gdiv
        cmp F_SH
        bcs @ok
@nx:    inc F_GS
        lda F_GS
        cmp #17
        bcc @l
        sec
        rts
@ok:    clc
        rts

; Bildspeicher weiss
g_clear:
        lda IMBK
        sta PDST+2
        stz PDST
        lda F_SW
        ldx F_SH
        CALL gmul
        clc
        adc F_BASE
        sta F_GT2               ; Ende
        lda #WHITE
        ldy F_BASE
@c:     cpy F_GT2
        bcs @r
        IWR
        iny
        bra @c
@r:     rts

; ---------- LZW ----------
g_lzw:  lda IMBK
        sta PDST+2
        stz PDST
        stz F_GX
        stz F_GXP
        stz F_GDX
        stz F_GY
        stz F_GPASS
        stz F_GDONE
        stz F_GAL
        stz F_GAH
        stz F_GNB
        jsr g_rowset
        ldx F_GMCS                ; Clear = 1 << mcs
        lda #1
@cs:    asl
        dex
        bne @cs
        sta F_GCLR
        ldy #0                  ; Wurzeln: Suffix = Code
@ro:    cpy F_GCLR
        bcs @rz
        phy
        tya
        pha
        tya
        clc
        adc #G_SUFX
        tay
        pla
        IWR
        ply
        iny
        bra @ro
@rz:    jsr g_reset
@code:  lda F_GDONE
        beq *+5
        jmp @end
        jsr g_get
        bcc *+5
        jmp @end
        sta F_GIN
        cmp F_GCLR
        bne @n1
        jsr g_reset
        bra @code
@n1:    lda F_GCLR
        inc a
        cmp F_GIN                 ; Ende
        bne *+5
        jmp @end
        lda F_GOLD
        bpl @n2
        lda F_GIN                 ; erster Code nach Clear
        cmp F_GCLR
        bcs @code
        sta F_GOLD
        sta F_GFST
        jsr g_pix
        bra @code
@n2:    lda #G_STK
        sta F_GSP
        lda F_GIN
        sta F_GT
        cmp F_GNXT
        bcc @dc
        beq @kw
        jmp @end                ; ungueltig
@kw:    lda F_GFST                ; KwKwK
        jsr g_push
        lda F_GOLD
        sta F_GT
@dc:    lda F_GT                  ; Kette abbauen
        cmp F_GCLR
        bcc @root
        clc
        adc #G_SUFX
        tay
        IRD
        jsr g_push
        lda F_GT
        asl
        clc
        adc #G_PREF
        tay
        lda [PDST],y
        sta F_GT
        lda F_GSP
        cmp #G_STK+4000
        bcc @dc
        jmp @end
@root:  sta F_GFST
        jsr g_push
@pop:   lda F_GSP                 ; ausgeben
        cmp #G_STK
        beq @add
        dec F_GSP
        ldy F_GSP
        IRD
        jsr g_pix
        bra @pop
@add:   lda F_GNXT                ; neuer Eintrag
        cmp #4096
        bcs @old
        asl
        clc
        adc #G_PREF
        tay
        lda F_GOLD
        sta [PDST],y
        lda F_GNXT
        clc
        adc #G_SUFX
        tay
        lda F_GFST
        IWR
        inc F_GNXT
        lda F_GNXT
        cmp #4096
        bcs @old
        ldx F_GCS                 ; Codelaenge erhoehen?
        lda #1
@p2:    asl
        dex
        bne @p2
        cmp F_GNXT
        bne @old
        lda F_GCS
        cmp #12
        bcs @old
        inc F_GCS
@old:   lda F_GIN
        sta F_GOLD
        jmp @code
@end:   rts

g_reset:
        lda F_GMCS
        inc a
        sta F_GCS
        lda F_GCLR
        inc a
        inc a
        sta F_GNXT
        lda #$FFFF
        sta F_GOLD
        rts

g_push: ldy F_GSP
        IWR
        inc F_GSP
        rts

; naechster Code -> A. C=1 am Datenende
g_get:
@f:     lda F_GNB
        cmp F_GCS
        bcs @h
        jsr g_byte
        bcc *+3
        rts
        sta F_GT2                 ; Byte << F_GNB in F_GAL/F_GAH einfuegen
        stz F_GNC
        ldx F_GNB
@s:     cpx #0
        beq @o
        asl F_GT2
        rol F_GNC
        dex
        bra @s
@o:     lda F_GT2
        ora F_GAL
        sta F_GAL
        lda F_GNC
        ora F_GAH
        sta F_GAH
        lda F_GNB
        clc
        adc #8
        sta F_GNB
        bra @f
@h:     lda F_GCS
        asl
        tax
        lda F_GAL
        and g_mask,x
        pha
        ldx F_GCS
@r:     lsr F_GAH
        ror F_GAL
        dex
        bne @r
        lda F_GNB
        sec
        sbc F_GCS
        sta F_GNB
        pla
        clc
        rts
        .pushseg
        .segment "RODATA"
g_mask: .word 0,1,3,7,15,31,63,127,255,511,1023,2047,4095
        .popseg

; naechstes Datenbyte aus den Unterbloecken. C=1 am Ende
g_byte: lda F_GBL
        bne @d
        ldy F_GBR
        cpy BEND
        bcs @e
        BRD
        beq @e
        sta F_GBL
        iny
        sty F_GBR
@d:     ldy F_GBR
        cpy BEND
        bcs @e
        BRD
        iny
        sty F_GBR
        dec F_GBL
        clc
        rts
@e:     sec
        rts

; Bildpunkt A (GIF-Farbe) ausgeben
g_pix:  ldx F_GDONE
        bne @r
        ldx F_GKEEP
        beq @nx
        ldx F_GXP
        bne @nx
        clc
        adc #G_MAP
        tay
        BRD
        pha
        lda F_GROWB
        clc
        adc F_GDX
        tay
        pla
        sep #$20
        sta [PDST],y
        rep #$20
        inc F_GDX
@nx:    inc F_GXP
        lda F_GXP
        cmp F_GS
        bcc @x
        stz F_GXP
@x:     inc F_GX
        lda F_GX
        cmp F_GW
        bcc @r
        stz F_GX
        stz F_GXP
        stz F_GDX
        jsr g_nextrow
@r:     rts

g_nextrow:
        lda F_GIL
        bne @il
        inc F_GY
        lda F_GY
        cmp F_GH
        bcc g_rowset
        inc F_GDONE
        rts
@il:    ldx F_GPASS               ; Zeilensprung: 4 Durchgaenge
        lda F_GY
        clc
        adc g_step,x
        sta F_GY
@chk:   lda F_GY
        cmp F_GH
        bcc g_rowset
        inc F_GPASS
        inc F_GPASS
        ldx F_GPASS
        cpx #8
        bcc @np
        inc F_GDONE
        rts
@np:    lda g_start,x
        sta F_GY
        bra @chk
g_rowset:
        stz F_GKEEP
        lda F_GY
        ldx F_GS
        CALL gdiv
        sta F_GT
        lda GMR
        bne @r
        inc F_GKEEP
        lda F_GT
        ldx F_SW
        CALL gmul
        clc
        adc F_BASE
        sta F_GROWB
@r:     rts
        .pushseg
        .segment "RODATA"
g_step:  .word 8, 8, 4, 2
g_start: .word 0, 4, 2, 1
        .popseg

; ---------- Zeichnen (aus web_draw) ----------
gif_draw:
        lda GIFON
        bne @go
        rts
@go:    lda IMBK
        sta PDST+2
        stz PDST
        lda NRV                 ; sichtbare Hoehe
        ldx #LH
        CALL gmul
        sta F_GT                  ; Zeilen im Fenster
        lda PAGE                ; erste Bildzeile
        ldx #LH
        CALL gmul
        sta F_GY
        stz F_GBI                 ; Zeile im Fenster
@row:   lda F_GBI
        cmp F_GT
        bcc *+5
        jmp @e
        lda F_GY
        cmp F_SH
        bcc *+5
        jmp @e
        lda #WY+31
        clc
        adc DYO
        clc
        adc F_GBI
        sta F_GT2                 ; Bildschirmzeile
        cmp KT
        bcs *+5
        jmp @nr
        cmp KB
        bcc *+5
        jmp @nr
        pha
        lda F_GY
        ldx F_SW
        CALL gmul
        sta F_GROWB
        lda F_SW
        sta F_GNC
        lda #WX+8
        clc
        adc DXO
        sta F_GDX
        pla
        jsr g_row
@nr:    inc F_GBI
        inc F_GY
        jmp @row
@e:     rts

; Inhaltstyp image/gif? C=0 ja
is_gif: ldy #WB_CTYP+1
        ldx #c_gif
        jmp pfx_ci
; Bildquelle in WB_HREF endet auf .gif? C=0 ja
img_gif:
        ldy #WB_HREF
        BRD
        cmp #5
        bcc @n
        clc
        adc #WB_HREF-3
        tay
        BRD
        cmp #'.'
        bne @n
        iny
        lda [PSRC],y
        ora #$2020
        cmp #$6967              ; "gi"
        bne @n
        iny
        iny
        BRD
        ora #$20
        cmp #'f'
        bne @n
        clc
        rts
@n:     sec
        rts
        .pushseg
        .segment "RODATA"
c_gif:  .byte 9,"image/gif"
        .popseg

; Eine Bildzeile zeichnen: A = Bildschirmzeile, F_GROWB = Quelle in der Bildbank,
; F_GNC = Breite, F_GDX = Bildschirmspalte (wird veraendert). PDST = Bildbank.
g_row:  asl
        tax
        lda linetab,x
        sta PTR
        lda F_GNC               ; Breite begrenzen
        cmp WRAPW
        bcc @wd
        lda WRAPW
@wd:    sta F_GNC
        stz F_GX
@px:    lda F_GX
        cmp F_GNC
        bcs @nr
        ldx F_GDX
        cpx KL
        bcc @sk
        cpx KR
        bcs @nr
        cpx CLIPR
        bcs @nr
        lda F_GROWB
        clc
        adc F_GX
        tay
        lda [PDST],y
        and #$000F
        sta F_GAL
        txa
        lsr
        tay
        bcs @odd
        lda F_GAL               ; linker Punkt: oberes Halbbyte
        asl
        asl
        asl
        asl
        sta F_GAL
        sep #$20
        lda [PTR],y
        and #$0F
        ora F_GAL
        sta [PTR],y
        rep #$20
        bra @sk
@odd:   sep #$20
        lda [PTR],y
        and #$F0
        ora F_GAL
        sta [PTR],y
        rep #$20
@sk:    inc F_GX
        inc F_GDX
        bra @px
@nr:    rts
