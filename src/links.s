        .segment "XCODE2"
FARMODE .set 2
xf_act_link:
        jsr act_link
        rtl
xf_draw_links:
        jsr draw_links
        rtl
xf_lnk_hit:
        jsr lnk_hit
        rtl
xf_lnk_open:
        jsr lnk_open
        rtl
xf_lnk_remove:
        jsr lnk_remove
        rtl
xf_path_open:
        jsr path_open
        rtl
xf_pl_label:
        jsr pl_label
        rtl
; ================================================================
; GS/OS95: Verknuepfungen auf dem Desktop (*:System:GSOS95.LNK)
; ================================================================
LNKMAX = 6
LNKSZ  = 72                     ; +0 Typ, +2 Zusatztyp, +4 Pfad (Laenge + 64 Zeichen)
ICX2   = 72                     ; zweite Spalte

; Lage der Verknuepfung A -> LX (Symbol links), LY (oben)
lnk_pos:
        cmp #2
        bcs @c2
        ldx #36
        CALL gmul
        clc
        adc #114
        sta LY
        lda #ICX
        sta LX
        rts
@c2:    sec
        sbc #2
        ldx #36
        CALL gmul
        clc
        adc #6
        sta LY
        lda #ICX2
        sta LX
        rts

; Adresse des Eintrags A -> A
lnk_addr:
        pha
        ldx #0
        JSLF xf_pl_sel
        pla
        JSLF xf_pl_addr
        rts

; alle Verknuepfungen zeichnen (Ursprung 0)
draw_links:
        stz LNKI
@l:     lda LNKI
        cmp LNKN
        bcc @go
        rts
@go:    jsr lnk_pos
        lda LNKI
        jsr lnk_addr
        sta PB2
        lda (PB2)               ; Typ -> Symbol
        CALL type_icon
        sta SPR
        lda LX
        sta SX
        lda LY
        sta SY
        CALL blit4
        ; kleiner Pfeil unten links = Verknuepfung
        lda LX
        sta RX
        lda LY
        clc
        adc #10
        sta RY
        lda #6
        sta RW
        sta RH
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda LX
        inc a
        sta RX
        lda LY
        clc
        adc #13
        sta RY
        lda #4
        sta RW
        lda #1
        sta RH
        lda #BLACK
        CALL setcolor
        CALL fillrect
        lda LX
        clc
        adc #4
        sta RX
        lda LY
        clc
        adc #11
        sta RY
        lda #1
        sta RW
        lda #3
        sta RH
        CALL fillrect
        ; Name = letzter Pfadteil
        jsr lnk_label
        CALL strwidth
        sta BW
        lsr
        sta TMP
        lda LX
        clc
        adc #8
        sec
        sbc TMP
        sta SX
        lda LY
        clc
        adc #19
        sta SY
        lda LNKI
        clc
        adc #3
        cmp SEL
        bne @ns
        lda SX
        dec a
        sta RX
        lda SY
        dec a
        sta RY
        lda BW
        clc
        adc #2
        sta RW
        lda #10
        sta RH
        lda #NAVY
        CALL setcolor
        CALL fillrect
@ns:    lda #WHITE
        CALL setcolor
        jsr lnk_label
        CALL drawstr
        inc LNKI
        jmp @l

; Beschriftung der Verknuepfung LNKI -> STRP/STRN (hoechstens 10 Zeichen)
lnk_label:
        lda LNKI
        jsr lnk_addr
        ldx #10
; Beschriftung (letzter Pfadteil) des Eintrags an Adresse A, hoechstens X Zeichen
pl_label:
        stx PLML
        clc
        adc #4
        sta PB2                 ; Pfad (Laenge + Zeichen)
        lda (PB2)
        tay
        iny
@f:     dey
        cpy #2
        bcc @a
        lda (PB2),y
        and #$00FF
        cmp #':'
        bne @f
@a:     iny                     ; Y = Offset des ersten Zeichens
        tya
        clc
        adc PB2
        sta STRP
        lda (PB2)
        clc
        adc #2
        sty TMP
        sec
        sbc TMP
        cmp PLML
        bcc @s
        lda PLML
@s:     sta STRN
        rts

; Mausklick auf eine Verknuepfung? C=1, A = Nummer
lnk_hit:
        stz LNKI
@l:     lda LNKI
        cmp LNKN
        bcs @no
        jsr lnk_pos
        lda LX
        sec
        sbc #16
        sta RX
        lda LY
        sta RY
        lda #48
        sta RW
        lda #30
        sta RH
        CALL inrect
        bcs @y
        inc LNKI
        bra @l
@y:     lda LNKI
        sec
        rts
@no:    clc
        rts

; ---------- Verknuepfung anlegen (Kontextmenue "Auf den Desktop") ----------
act_link:
        lda SEL2
        bmi @r
        cmp COUNT
        bcs @r
        sta IDX
        lda LNKN
        cmp #LNKMAX
        bcc @ok
        lda #20                 ; Desktop voll
        CALL show_msg
        rts
@ok:    CALL entry_path          ; srcpath, EP = Eintrag
        lda srcpath
        cmp #65
        bcs @r
        lda [EP]
        sta PLTYP
        ldy #E_AUX
        lda [EP],y
        sta PLAUX
        ldx #0
        JSLF xf_pl_sel
        JSLF xf_pl_append
        bcs @r
        JSLF xf_pl_save
        CALL redraw_all
        rts
@r:     rts

; Verknuepfung A entfernen
lnk_remove:
        pha
        ldx #0
        JSLF xf_pl_sel
        pla
        JSLF xf_pl_del
        lda #$FFFF
        sta SEL
        JSLF xf_pl_save
        CALL redraw_all
        rts

; ---------- Verknuepfung oeffnen ----------
lnk_open:
        jsr lnk_addr
; Pfadlisten-Eintrag an Adresse A oeffnen (Ordner, Laufwerk, Datei)
path_open:
        sta LNKP
        sta PB2
        lda (PB2)
        sta LNKT
        lda LNKP
        clc
        adc #4
        tax
        ldy #lnkdir
        CALL pcopy
        lda LNKT
        cmp #$000F
        beq @dir
        cmp #$0100
        bcs @dir
        ; Datei: Ordner oeffnen, Datei suchen und wie per Doppelklick oeffnen
        ldx #lnkdir
        ldy #lnkname
        CALL pcopy
        ldy #lnkdir
        CALL pstrip
        jsr lnk_newwin
        bcc @fnd
        rts
@fnd:   jsr lnk_find
        bcs @nf
        sta SEL2
        CALL win_geom
        lda SEL2
        CALL ensure_vis
        CALL redraw_all
        lda SEL2
        CALL open_entry
        rts
@nf:    CALL redraw_all
        lda #21                 ; Ziel nicht gefunden
        CALL show_msg
        rts
@dir:   jsr lnk_newwin
        bcs @r
        CALL redraw_all
        rts
@r:     rts

; neues Ordnerfenster fuer lnkdir. C=1 bei Fehler (Meldung schon gezeigt)
lnk_newwin:
        CALL new_explorer
        bcc @ok
        rts
@ok:    ldx #lnkdir
        ldy #pathbuf
        CALL pcopy
        stz ISBIN
        stz PAGE
        lda #$FFFF
        sta SEL2
        lda #1
        sta MODE
        CALL load_dir
        bcc @r
        CALL win_pop
        CALL redraw_all
        lda #21
        CALL show_msg
        sec
@r:     rts

; Eintrag mit dem letzten Teil von lnkname suchen -> A, C=0 gefunden
lnk_find:
        lda lnkname
        tay
        iny
@f:     dey
        cpy #2
        bcc @a
        lda lnkname,y
        and #$00FF
        cmp #':'
        bne @f
@a:     iny
        sty LNKO                ; erstes Zeichen
        lda lnkname
        clc
        adc #2
        sec
        sbc LNKO
        sta LNKL                ; Laenge
        stz LNKI
@l:     lda LNKI
        cmp COUNT
        bcs @no
        CALL entep
        ldy #2
        lda [EP],y
        and #$00FF
        cmp LNKL
        bne @n
        ldx LNKO
        ldy #3
@c:     lda [EP],y
        CALL upc8
        sta TMP
        lda lnkname,x
        CALL upc8
        cmp TMP
        bne @n
        inx
        iny
        tya
        sec
        sbc #3
        cmp LNKL
        bcc @c
        lda LNKI
        clc
        rts
@n:     inc LNKI
        bra @l
@no:    sec
        rts


; ================================================================
; Laufwerke auf dem Desktop (Einstellung "Laufwerke auf dem Desktop")
; Rechte Spalte, wie beim Mac. 5,25"-Laufwerke werden nicht abgefragt.
; ================================================================
DVMAX = 5
DVSZ  = 20                      ; +0 Typ, +2 Geraet, +4 Name (Pascal, max. 15)
DVX   = 284                     ; Symbol links

xf_dv_scan:
        jsr dv_scan
        rtl
xf_dv_draw:
        jsr dv_draw
        rtl
xf_dv_hit:
        jsr dv_hit
        rtl
xf_dv_open:
        jsr dv_open
        rtl
xf_dv_disk:
        jsr dv_disk
        rtl

; Laufwerke abfragen -> dvtab, DVCNT
dv_scan:
        stz DVCNT
        lda DVON
        bne @go
        rts
@go:    lda #1
        sta DVI
@d:     lda DVI
        sta dinfo_pb+2
        GSOS $202C,dinfo_pb     ; DInfo
        bcc @ok
        cmp #$0011              ; keine weiteren Geraete
        bne @nj
        rts
@nj:    jmp @n
@ok:    lda dinfo_pb+8
        and #$0080              ; nur Blockgeraete
        beq @nj
        lda dinfo_pb+8          ; 5,25": nicht abfragen (ohne Diskette langsam)
        and #$0084
        cmp #$0084
        bne @ask
        lda dinfo_pb+20
        bne @ask
        lda #$0102              ; Name '5,25" A' (Buchstabe aus dem Geraetenamen)
        sta DVT
        ldx #6
@cp:    lda dv525,x
        sta dvlab,x
        dex
        dex
        bpl @cp
        ldx devname_buf+2
        lda devname_buf+3,x
        sep #$20
        .a8
        sta dvlab+6
        lda LANG                ; englisch: Punkt statt Komma
        beq @de
        lda #'.'
        sta dvlab+1
@de:    rep #$20
        .a16
        lda #dvlab
        sta STRP
        lda #7
        sta STRN
        bra @add
@ask:   GSOS $2008,volume_pb    ; Volume
        bcc @vol
        cmp #$0052              ; unbekanntes Format
        beq @unf
        cmp #$0027              ; leer / unformatiert
        bne @n
@unf:   lda #$0101
@dev:   sta DVT                 ; Name = Geraetename ohne Punkt
        lda #devname_buf+4
        sta STRP
        lda devname_buf+2
        sta STRN
        bra @add
@vol:   lda #$0100
        sta DVT
        lda #volname_buf+4
        sta STRP
        lda volname_buf+2
        sta STRN
@add:   jsr dv_store
        inc DVCNT
        lda DVCNT
        cmp #DVMAX
        bcc @n
        rts
@n:     inc DVI
        lda DVI
        cmp #40
        bcs @x
        jmp @d
@x:     rts

; Vorlage fuer 5,25"-Laufwerke (Buchstabe kommt dazu)
        .pushseg
        .segment "RODATA"
dv525:  .byte "5,25", $22, " ?", 0
        .popseg

; Eintrag DVCNT: Typ DVT, Geraet DVI, Name STRP/STRN (fuehrendes ':' / '.' weg)
dv_store:
        lda DVCNT
        jsr dv_addr
        sta PB2
        lda DVT
        sta (PB2)
        ldy #2
        lda DVI
        sta (PB2),y
        lda (STRP)
        and #$00FF
        cmp #':'
        beq @s
        cmp #'.'
        bne @ns
@s:     inc STRP
        dec STRN
@ns:    lda STRN
        bpl *+5
        lda #0
        cmp #16
        bcc *+5
        lda #15
        sta STRN
        sep #$20
        .a8
        ldy #4
        sta (PB2),y
        ldx #0
        txy
@c:     cpx STRN
        beq @e
        lda (STRP),y
        phy
        pha
        txa
        clc
        adc #5
        tay
        pla
        sta (PB2),y
        ply
        iny
        inx
        bra @c
@e:     rep #$20
        .a16
        rts

; Adresse des Eintrags A -> A
dv_addr:
        asl
        asl
        sta TMP                 ; *4
        asl
        asl                     ; *16
        clc
        adc TMP                 ; *20
        clc
        adc #dvtab
        rts

; Lage des Laufwerks A -> LX, LY
dv_pos: asl
        asl
        sta TMP                 ; *4
        asl
        asl
        asl                     ; *32
        clc
        adc TMP                 ; *36
        clc
        adc #6
        sta LY
        lda #DVX
        sta LX
        rts

; Name des Laufwerks A -> STRP/STRN; laenger als 10 Zeichen: 8 Zeichen + ".."
dv_label:
        jsr dv_addr
        clc
        adc #4
        sta STRP
        lda (STRP)
        and #$00FF
        inc STRP
        sta STRN
        cmp #11
        bcc @r
        ldy #0
@c:     lda (STRP),y
        sta dvlab,y
        iny
        iny
        cpy #8
        bcc @c
        lda #$2E2E              ; ".."
        sta dvlab+8
        lda #dvlab
        sta STRP
        lda #10
        sta STRN
@r:     rts

; alle Laufwerke zeichnen (Ursprung 0)
dv_draw:
        stz DVI
@l:     lda DVI
        cmp DVCNT
        bcc @go
        rts
@go:    jsr dv_pos
        lda DVI
        jsr dv_addr
        sta PB2
        lda (PB2)
        CALL type_icon
        sta SPR
        lda LX
        sta SX
        lda LY
        sta SY
        CALL blit4
        lda DVI
        jsr dv_label
        CALL strwidth
        sta BW
        lsr
        sta TMP
        lda LX
        clc
        adc #8
        sec
        sbc TMP
        sta SX
        clc                     ; nicht ueber den rechten Rand
        adc BW
        cmp #319
        bcc @rx
        lda #318
        sec
        sbc BW
        sta SX
@rx:    lda LY
        clc
        adc #19
        sta SY
        lda DVI
        clc
        adc #DVSEL
        cmp SEL
        bne @ns
        lda SX
        dec a
        sta RX
        lda SY
        dec a
        sta RY
        lda BW
        clc
        adc #2
        sta RW
        lda #10
        sta RH
        lda #NAVY
        CALL setcolor
        CALL fillrect
@ns:    lda #WHITE
        CALL setcolor
        lda DVI
        jsr dv_label
        CALL drawstr
        inc DVI
        jmp @l

; Mausklick auf ein Laufwerk? C=1, A = Nummer
dv_hit: stz DVI
@l:     lda DVI
        cmp DVCNT
        bcs @no
        jsr dv_pos
        lda LX
        sec
        sbc #14
        sta RX
        lda LY
        sta RY
        lda #320
        sec
        sbc RX
        sta RW
        lda #30
        sta RH
        CALL inrect
        bcs @y
        inc DVI
        bra @l
@y:     lda DVI
        sec
        rts
@no:    clc
        rts

; Laufwerk A oeffnen: Datentraeger im Explorer, sonst Arbeitsplatz
dv_open:
        jsr dv_addr
        sta PB2
        lda (PB2)
        cmp #$0100
        beq @v
        CALL open_explorer
        rts
@v:     ldy #4                  ; lnkdir = ":" + Name
        lda (PB2),y
        and #$00FF
        inc a
        sta lnkdir
        tax
        sep #$20
        .a8
        lda #':'
        sta lnkdir+2
        ldx #1
        ldy #5
@c:     cpx lnkdir
        beq @e
        lda (PB2),y
        sta lnkdir+2,x
        iny
        inx
        bra @c
@e:     rep #$20
        .a16
        jsr lnk_newwin
        bcs @r
        CALL redraw_all
@r:     rts

; Diskette eingelegt/ausgeworfen: neu abfragen und Desktop zeichnen
dv_disk:
        lda DVON
        beq @r
        jsr dv_scan
        CALL redraw_all
@r:     rts


FARMODE .set 0
        .segment "CODE"
