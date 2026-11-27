; ================================================================
; GS/OS95: Ordnerbaum im Explorer (linke Spalte, Symbolansicht)
; Zeigt Arbeitsplatz, den Pfad zum aktuellen Ordner und dessen
; Unterordner (bzw. die Laufwerke). Klick = dorthin wechseln.
; Liegt in der zweiten Codebank (XCODE).
; ================================================================
        .segment "XCODE"
FARMODE .set 1
TRH    = 9                      ; Zeilenhoehe
TRX0   = WX+8                   ; linker Rand
TRY0   = WY+32                  ; erste Zeile

; Bestandteil TRK (ab 1) von pathbuf -> TRS (Anfang), TRE (Ende, exklusiv). C=1: gibt es nicht
tr_scan:
        ldx #0
        ldy #0
@l:     cpx pathbuf
        bcs @no
        lda pathbuf+2,x
        and #$00FF
        cmp #':'
        bne @n
        iny
        cpy TRK
        beq @f
@n:     inx
        bra @l
@f:     inx
        stx TRS
@e:     cpx pathbuf
        bcs @d
        lda pathbuf+2,x
        and #$00FF
        cmp #':'
        beq @d
        inx
        bra @e
@d:     stx TRE
        clc
        rts
@no:    sec
        rts

; Anzahl der Pfadbestandteile -> TRN (0 im Arbeitsplatz)
tr_depth:
        stz TRN
        lda MODE
        beq @r
@l:     lda TRN
        inc a
        sta TRK
        jsr tr_scan
        bcs @r
        inc TRN
        lda TRN
        cmp #12
        bcc @l
@r:     rts

; Zeile TRROW: Y-Koordinate -> SY, X nach Tiefe A -> SX
tr_pos: cmp #6                  ; hoechstens 5 Stufen einruecken
        bcc @c
        lda #5
@c:     asl
        asl
        clc
        adc #TRX0
        sta SX
        lda TRROW
        ldx #TRH
        CALL gmul
        clc
        adc #TRY0
        sta SY
        rts

; Symbol A an SX,SY, danach SX hinter das Symbol
tr_icon:
        sta SPR
        lda SY
        pha
        inc SY
        CALL blit4
        pla
        sta SY
        lda SX
        clc
        adc #11
        sta SX
        rts

; Text STRP/STRN so kuerzen, dass er ab SX in die Baumspalte passt ("..")
tr_fit: lda #WX+6+TREEW-5
        sec
        sbc SX
        sta TRFW
        CALL strwidth
        cmp TRFW
        bcc @r
        beq @r
        ldy #0                  ; nach trname kopieren
        sep #$20
@cp:    cpy STRN
        bcs @cd
        cpy #30
        bcs @cd
        lda (STRP),y
        sta trname,y
        iny
        bra @cp
@cd:    rep #$20
        sty STRN
        lda #trname
        sta STRP
@sh:    lda STRN
        cmp #2
        bcc @r
        dec STRN
        ldy STRN
        lda #$2E2E
        sta trname,y
        inc STRN
        inc STRN
        CALL strwidth
        dec STRN
        dec STRN
        cmp TRFW
        bcc @ok
        beq @ok
        bra @sh
@ok:    inc STRN
        inc STRN
@r:     rts

; Markierung (aktueller Ordner): Balken von SX bis zum Spaltenrand
tr_mark:
        lda SX
        dec a
        sta RX
        lda SY
        sta RY
        lda #WX+6+TREEW-4
        sec
        sbc RX
        sta RW
        lda TRMW                ; nur so breit wie der Name (wie bei Win95)
        beq @fw
        clc
        adc #2
        cmp RW
        bcs @fw
        sta RW
@fw:
        lda #TRH
        sta RH
        lda #NAVY
        CALL setcolor
        CALL fillrect
        rts

; ---------- Baumzeilen (zwischengespeichert) ----------
; Zeile = 18 Bytes: Tiefe, Merkmale (1 Pfad, 2 aktueller Ordner, 4 Laufwerk),
; Namenslaenge, 15 Zeichen. Gebaut nach jedem Verzeichniswechsel; beim Zeichnen
; wird nicht von der Platte gelesen.
TRCAP = 32
TROW  = 18
TF_PATH = 1
TF_CUR  = 2
TF_DRV  = 4

; Pruefsumme von MODE+pathbuf -> A
tb_sum: lda MODE
        sta TBS
        ldx #0
@l:     cpx pathbuf
        bcs @d
        lda pathbuf+2,x
        and #$00FF
        asl TBS
        adc TBS
        sta TBS
        inx
        bra @l
@d:     lda TBS
        eor pathbuf
        rts

; Zeile A -> X = Byte-Offset in treebuf
tb_off: ldx #TROW
        CALL gmul
        tax
        rts

; Zeile tbrow (Puffer) an Stelle TBAT einfuegen. C=1: kein Platz mehr
tb_ins: lda TRCNT
        cmp #TRCAP
        bcc @ok
        sec
        rts
@ok:    jsr tb_off              ; X = Ende der belegten Zeilen
        stx TBE
        lda TBAT
        jsr tb_off
        stx TBB
        ldx TBE                 ; Zeilen ab TBAT um eine Zeile nach hinten
        sep #$20
@mv:    cpx TBB
        beq @md
        dex
        lda treebuf,x
        sta treebuf+TROW,x
        bra @mv
@md:    ldy #0
        ldx TBB
@cp:    lda tbrow,y
        sta treebuf,x
        inx
        iny
        cpy #TROW
        bcc @cp
        rep #$20
        inc TRCNT
        clc
        rts
; Zeile anhaengen
tb_add: lda TRCNT
        sta TBAT
        jmp tb_ins

; tbrow aus Tiefe A, Merkmalen X und Name STRP/STRN fuellen
tb_set: sep #$20
        sta tbrow
        txa
        sta tbrow+1
        rep #$20
        lda STRN
        cmp #16
        bcc @c
        lda #15
@c:     sta TBN
        sep #$20
        sta tbrow+2
        ldy #0
@l:     cpy TBN
        bcs @d
        lda (STRP),y
        sta tbrow+3,y
        iny
        bra @l
@d:     rep #$20
        rts

; Name von Zeile A (ab Offset X) gleich STRP/STRN (ohne Gross/klein)? Z=1 ja
tb_same:
        jsr tb_off
        lda treebuf+2,x
        and #$00FF
        cmp STRN
        bne @r
        ldy #0
@l:     cpy STRN
        beq @r                  ; Z=1
        lda treebuf+3,x
        CALL upc8
        sta TBC
        lda (STRP),y
        CALL upc8
        cmp TBC
        bne @r
        inx
        iny
        bra @l
@r:     rts

; Index der Pfadzeile der Tiefe A -> A
tb_pathrow:
        sta TBD
        stz TBJ
@l:     lda TBJ
        cmp TRCNT
        bcs @r
        jsr tb_off
        lda treebuf+1,x
        and #TF_PATH
        beq @n
        lda treebuf,x
        and #$00FF
        cmp TBD
        beq @r
@n:     inc TBJ
        bra @l
@r:     lda TBJ
        rts

; Geschwister der Pfadzeile Tiefe TBK einordnen: Name STRP/STRN (nicht die
; Pfadzeile selbst). TBAFT=0: davor, sonst hinter ihren Unterbaum
tb_sib: lda TBK
        jsr tb_pathrow
        sta TBP
        lda TBAFT
        bne @aft
        lda TBP
        sta TBAT
        jsr tb_ins
        bcs @r
        inc TBP
@r:     rts
@aft:   lda TBP                 ; Ende des Unterbaums suchen
        inc a
        sta TBAT
@l:     lda TBAT
        cmp TRCNT
        bcs @ins
        jsr tb_off
        lda treebuf,x
        and #$00FF
        cmp TBK
        beq @ins
        bcc @ins
        inc TBAT
        bra @l
@ins:   jmp tb_ins

; ---------- Baum aufbauen ----------
; A<>0: auch Nachbarordner von der Platte lesen
xf_tree_build:
        jsr tb_build
        rtl
tb_build:
        sta TBFULL
        lda TREEOFF
        beq @go
        rts
@go:    stz TRCNT
        stz TRCUR
        ; Arbeitsplatz
        stz STRN
        lda #0
        ldx #0
        ldy MODE
        bne @mc
        ldx #TF_CUR
@mc:    jsr tb_set
        jsr tb_add
        lda MODE
        bne @dir
        ; Arbeitsplatz: Laufwerke = Eintraege
        stz TRI
@vl:    lda TRI
        cmp COUNT
        bcs @fin
        CALL entep
        jsr tr_name
        lda #1
        ldx #TF_DRV
        jsr tb_set
        jsr tb_add
        inc TRI
        bra @vl
@fin:   jsr tb_sum
        sta TRSUM
        rts
@dir:   jsr tr_depth            ; Pfad
        lda #1
        sta TRK
@pl:    lda TRK
        cmp TRN
        beq @pg
        bcs @kids
@pg:    jsr tr_scan
        lda #pathbuf+2
        clc
        adc TRS
        sta STRP
        lda TRE
        sec
        sbc TRS
        sta STRN
        ldx #TF_PATH
        lda TRK
        cmp #1
        bne @pf
        ldx #TF_PATH+TF_DRV
@pf:    cmp TRN
        bne @pa
        txa
        ora #TF_CUR
        tax
        lda TRCNT
        sta TRCUR
        lda TRK
@pa:    jsr tb_set
        jsr tb_add
        inc TRK
        bra @pl
@kids:  stz TRI                 ; Unterordner des aktuellen Ordners
@kl:    lda TRI
        cmp COUNT
        bcs @sib
        jsr tr_isdir
        bcc @kn
        jsr tr_name
        lda TRN
        inc a
        ldx #0
        jsr tb_set
        jsr tb_add
@kn:    inc TRI
        bra @kl
@sib:   jsr tb_vols             ; Laufwerke neben dem aktuellen (ohne Plattenzugriff)
        lda TBFULL
        bne *+5
        jmp @done
        lda #2                  ; Nachbarordner Ebene 2..TRN
        sta TBK
@lv:    lda TBK
        cmp TRN
        beq @lg
        bcs @done
@lg:    jsr tb_level
        inc TBK
        bra @lv
@done:  lda TRCNT               ; aktuelle Zeile neu suchen (Einfuegungen davor)
        stz TBI
@fc:    lda TBI
        cmp TRCNT
        bcs @fz
        jsr tb_off
        lda treebuf+1,x
        and #TF_CUR
        bne @fz
        inc TBI
        bra @fc
@fz:    lda TBI
        sta TRCUR
        jmp @fin

; A=1, wenn das geladene Fenster (CURREC) das aktive ist, sonst A=0
tb_istop:
        lda NWIN
        sta TBQ
@l:     dec TBQ
        bmi @no
        lda TBQ
        CALL is_active
        bcc @l
        lda TBQ
        CALL recaddr
        cmp CURREC
        bne @no
        lda #1
        rts
@no:    lda #0
        rts

; andere Laufwerke aus dem Laufwerks-Zwischenspeicher
tb_vols:
        lda VOLOK
        beq @r
        stz TBAFT
        lda #1
        sta TBK
        stz TBI
@l:     lda TBI
        cmp VOLN
        bcs @r
        ldx #ENTSZ
        CALL gmul
        clc
        adc #volcache+2         ; Laenge + Name des Eintrags
        sta STRP
        lda (STRP)
        and #$00FF
        sta STRN
        inc STRP
        lda #1
        jsr tb_pathrow
        jsr tb_same
        beq @self
        lda #1
        ldx #TF_DRV
        jsr tb_set
        jsr tb_sib
        bra @n
@self:  lda #1
        sta TBAFT
@n:     inc TBI
        bra @l
@r:     rts

; Nachbarordner der Pfadzeile Tiefe TBK: Elternverzeichnis lesen
tb_level:
        lda TBK                 ; Name der Pfadzeile merken
        sta TRK
        jsr tr_scan
        lda TRS
        sta TBNS
        lda TRE
        sec
        sbc TRS
        sta TBNL
        lda TBK                 ; Elternpfad: pathbuf bis Ende von Ebene TBK-1
        dec a
        sta TRK
        jsr tr_scan
        lda pathbuf
        sta TBPL
        lda TRE
        sta pathbuf
        stz TBAFT
        GSOS $2010,open_pb
        bcc *+5
        jmp @rs
        lda open_pb+2
        sta dirent_pb+2
        sta close_pb+2
@ent:   lda #1
        sta dirent_pb+6
        sta dirent_pb+8
        GSOS $201C,dirent_pb
        bcs @cl
        lda dirent_pb+16
        and #$00FF
        cmp #$000F
        bne @ent
        lda #namebuf+4
        sta STRP
        lda namebuf+2
        sta STRN
        lda TBK                 ; Papierkorb in der Wurzel nicht zeigen
        cmp #2
        bne @nb
        jsr tb_isbin
        beq @ent
@nb:    lda TBNL                ; die Pfadzeile selbst?
        cmp STRN
        bne @sb
        ldy #0
        ldx TBNS
@cmp:   cpy STRN
        beq @self
        lda pathbuf+2,x
        CALL upc8
        sta TBC
        lda (STRP),y
        CALL upc8
        cmp TBC
        bne @sb
        inx
        iny
        bra @cmp
@self:  lda #1
        sta TBAFT
        bra @ent
@sb:    lda TBK
        ldx #0
        jsr tb_set
        jsr tb_sib
        bra @ent
@cl:    GSOS $2014,close_pb
@rs:    lda TBPL
        sta pathbuf
        rts

; STRP/STRN = "RECYCLED"? Z=1 ja
tb_isbin:
        lda STRN
        cmp #8
        bne @r
        ldy #0
@l:     cpy #8
        beq @r
        lda (STRP),y
        CALL upc8
        sta TBC
        lda tbbin,y
        and #$00FF
        cmp TBC
        bne @r
        iny
        bra @l
@r:     rts
tbbin:  .byte "RECYCLED"

; ---------- Zeichnen ----------
xf_tree_draw:
        jsr tr_draw
        rtl
tr_draw:
        ; Trennlinie
        lda #WX+6+TREEW-3
        sta RX
        lda #WY+30
        sta RY
        lda #1
        sta RW
        lda CH
        sec
        sbc #50
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda CH                  ; Zeilen, die hineinpassen
        sec
        sbc #54
        ldx #TRH
        CALL gdiv
        sta TRMAX
        jsr tb_sum              ; gehoert der Zwischenspeicher zu diesem Fenster?
        cmp TRSUM
        beq @ok
        jsr tb_istop            ; aktives Fenster: vollstaendig (Nachbarordner),
        jsr tb_build            ; sonst schnell ohne Plattenzugriff
@ok:    lda TRCUR               ; aktuelle Zeile sichtbar halten
        stz TRSTART
        cmp TRMAX
        bcc @st
        sec
        sbc TRMAX
        clc
        adc #3
        sta TRSTART
@st:    lda #WX+6+TREEW-5       ; Text am Spaltenrand abschneiden
        clc
        adc DXO
        sta CLIPR
        stz TRROW
@row:   lda TRROW
        cmp TRMAX
        bcc *+5
        jmp @done
        clc
        adc TRSTART
        cmp TRCNT
        bcc *+5
        jmp @done
        jsr tb_off
        stx TBX
        lda treebuf,x
        and #$00FF
        sta TBD
        jsr tr_pos
        ldx TBX
        lda treebuf+1,x
        and #$00FF
        sta TBF
        lda #i_tfold
        ldx TBD
        bne @i1
        lda #i_qcomp
        bra @i2
@i1:    ldx TBF
        cpx #TF_DRV
        bcc @i2
        lda #i_tdisk
@i2:    jsr tr_icon
        lda TBD
        bne @nm
        LSPR t_arbeit           ; Arbeitsplatz: Schriftzug
        lda SPR
        sta TRSP
        sta LP2
        lda ABANK
        sta LP2+2
        lda [LP2]
        and #$00FF
        sta TRMW
        lda TBF
        and #TF_CUR
        beq @b0
        jsr tr_mark
        lda #WHITE
        bra @c0
@b0:    lda #BLACK
@c0:    CALL setcolor
        lda TRSP
        sta SPR
        CALL blit1
        bra @nx
@nm:    lda TBX
        clc
        adc #treebuf+3
        sta STRP
        ldx TBX
        lda treebuf+2,x
        and #$00FF
        sta STRN
        jsr tr_fit
        lda TBF
        and #TF_CUR
        beq @b1
        CALL strwidth
        sta TRMW
        jsr tr_mark
        lda #WHITE
        bra @c1
@b1:    lda #BLACK
@c1:    CALL setcolor
        CALL drawstr
@nx:    inc TRROW
        jmp @row
@done:  lda #320
        sta CLIPR
        rts

tr_isdir:
        lda TRI
        CALL entep
        lda MODE
        beq @y
        lda [EP]
        cmp #$000F
        beq @y
        clc
        rts
@y:     sec
        rts

; Name des Eintrags EP -> trname, STRP/STRN
tr_name:
        ldy #2
        lda [EP],y
        and #$00FF
        cmp #33
        bcc @c
        lda #32
@c:     sta STRN
        ldx #0
        ldy #3
        sep #$20
@l:     cpx STRN
        bcs @d
        lda [EP],y
        sta trname,x
        inx
        iny
        bra @l
@d:     rep #$20
        lda #trname
        sta STRP
        rts

; ---------- Klick ---------- C=1: lag in der Baumspalte
xf_tree_click:
        jsr tr_click
        rtl
tr_click:
        CALL lmx
        cmp #WX+6
        bcc @no
        cmp #WX+6+TREEW-2
        bcs @no
        CALL lmy
        cmp #TRY0
        bcc @in
        sec
        sbc #TRY0
        ldx #TRH
        CALL gdiv
        clc
        adc TRSTART
        cmp TRCNT
        bcs @in
        sta TBI
        jsr tb_off
        stx TBX
        lda treebuf+1,x
        and #$00FF
        sta TBF
        and #TF_CUR
        bne @in                 ; aktueller Ordner
        lda treebuf,x
        and #$00FF
        sta TBD
        bne @n0
        stz MODE                ; Arbeitsplatz
        stz ISBIN
        CALL load_volumes
        CALL redraw_top
@in:    sec
        rts
@no:    clc
        rts
@n0:    lda MODE
        bne @path
        lda TBI                 ; im Arbeitsplatz: Laufwerk Nr. TBI-1 oeffnen
        dec a
        CALL open_entry
        sec
        rts
@path:  lda TBF
        and #TF_PATH
        beq @new
        lda TBD                 ; uebergeordneter Ordner: Pfad kuerzen
        sta TRK
        jsr tr_scan
        bcs @in
        lda TRE
        sta pathbuf
        bra @load
@new:   lda TBD                 ; Nachbar-/Unterordner: Elternpfad + ":" + Name
        dec a
        beq @vol
        sta TRK
        jsr tr_scan
        bcs @in
        lda TRE
        sta pathbuf
        bra @app
@vol:   stz pathbuf             ; anderes Laufwerk
@app:   ldx pathbuf
        sep #$20
        lda #':'
        sta pathbuf+2,x
        rep #$20
        inx
        ldy TBX
        lda treebuf+2,y
        and #$00FF
        sta TBN
@ac:    lda TBN
        beq @ad
        lda treebuf+3,y
        sep #$20
        sta pathbuf+2,x
        rep #$20
        inx
        iny
        dec TBN
        bra @ac
@ad:    stx pathbuf
        lda #1
        sta MODE
@load:  stz ISBIN
        CALL load_dir
        CALL redraw_top
        sec
        rts
FARMODE .set 0
        .segment "CODE"
