; ================================================================
; GS/OS95 Fensterverwaltung: Stapel, Verschieben, Taskleiste
; ================================================================
MAXW    = 10
RECSZ   = 298
R_KIND  = 0
R_X     = 2
R_Y     = 4
R_MODE  = 6
R_ISBIN = 8
R_COUNT = 10
R_PAGE  = 12
R_SEL   = 14
R_SLOT  = 16
R_INH   = 18
R_ID    = 20
R_W     = 22                    ; Fenstergroesse
R_H     = 24
R_OX    = 26                    ; Lage/Groesse vor dem Maximieren
R_OY    = 28
R_OW    = 30
R_OH    = 32
R_MAXI  = 34                    ; 1 = maximiert
R_PATH  = 36
NSLOT   = 6

; ---------- Minimieren: Fenster liegt dann 1000 Punkte rechts ausserhalb ----------
MINOFF = 1000
; Datensatz A minimiert? C=1 ja; PB = Datensatz, Y = R_X
win_ismin:
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        bmi @n
        cmp #600
        rts
@n:     clc
        rts
; Datensatz A wieder einblenden (A bleibt)
win_unmin:
        pha
        jsr win_ismin
        bcc @r
        lda (PB),y
        sec
        sbc #MINOFF
        sta (PB),y
@r:     pla
        rts
; oberstes Fenster minimieren und nach hinten legen
win_minimize:
        jsr win_min1
        jmp redraw_all
; dasselbe ohne Neuzeichnen
win_min1:
        lda NWIN
        bne @go
        rts
@go:    dec a
        jsr win_ismin
        bcs @rd
        lda (PB),y
        clc
        adc #MINOFF
        sta (PB),y
        lda NWIN
        dec a
        sta MINC
@l:     lda MINC
        beq @rd
        lda #0
        jsr win_front
        dec MINC
        bra @l
@rd:    rts
; "Desktop anzeigen": alle Ordnerfenster minimieren; sind schon alle weg, wieder einblenden
win_showdesk:
        lda NWIN
        bne @go
        rts
@go:    dec a
        jsr win_ismin
        bcs @rest
        lda #MAXW
        sta TMIDX
@m:     lda NWIN
        dec a
        jsr win_ismin
        bcs @d
        jsr is_mini
        bcc @d
        jsr win_min1
        dec TMIDX
        bne @m
@d:     jmp redraw_all
@rest:  stz TMIDX
@u:     lda TMIDX
        cmp NWIN
        bcs @ud
        jsr win_unmin
        inc TMIDX
        bra @u
@ud:    lda NWIN
        dec a
        jsr win_load
        jmp redraw_all

; A = Index -> A = Adresse des Datensatzes
recaddr:
        tax
        lda #wrecs
@l:     cpx #0
        beq @r
        clc
        adc #RECSZ
        dex
        bra @l
@r:     rts

; aktuellen Zustand in den obersten Datensatz schreiben
win_save:
        lda NWIN
        bne @go
        rts
@go:    dec a
; Zustand in Datensatz A schreiben
win_savek:
        jsr recaddr
        sta PB
        ldy #R_KIND
        lda WIN
        sta (PB),y
        ldy #R_MODE
        lda MODE
        sta (PB),y
        ldy #R_ISBIN
        lda ISBIN
        sta (PB),y
        ldy #R_COUNT
        lda COUNT
        sta (PB),y
        ldy #R_PAGE
        lda PAGE
        sta (PB),y
        ldy #R_SEL
        lda SEL2
        sta (PB),y
        ; Pfad
        lda PB
        clc
        adc #R_PATH
        tay
        ldx #pathbuf
        jmp pcopy

; Datensatz A laden
win_load:
        jsr recaddr
        sta PB
        sta CURREC
        ldy #R_ID
        lda (PB),y
        sta CURWID
        ldy #R_KIND
        lda (PB),y
        sta WIN
        ldy #R_MODE
        lda (PB),y
        sta MODE
        ldy #R_ISBIN
        lda (PB),y
        sta ISBIN
        ldy #R_COUNT
        lda (PB),y
        sta COUNT
        ldy #R_PAGE
        lda (PB),y
        sta PAGE
        ldy #R_SEL
        lda (PB),y
        sta SEL2
        ldy #R_X
        lda (PB),y
        sec
        sbc #WX
        sta DXO
        ldy #R_Y
        lda (PB),y
        sec
        sbc #WY
        sta DYO
        ldy #R_W
        lda (PB),y
        sta CW
        ldy #R_H
        lda (PB),y
        sta CH
        ldy #R_SLOT
        lda (PB),y
        bmi @ns
        asl
        tax
        lda slotbase,x
        sta ENTBASE
        lda slotbank,x
        sta ENTBK
@ns:    lda PB
        clc
        adc #R_PATH
        tax
        ldy #pathbuf
        jsr pcopy
        jmp win_geom

; neues Fenster der Art A oeffnen (ohne Neuzeichnen). C=1 wenn kein Platz
win_open:
        sta TMPK
        lda NWIN
        cmp #MAXW
        bcc @ok
        sec
        rts
@ok:    jsr win_save
        lda NWIN
        jsr recaddr
        sta PB2
        lda TMPK
        cmp #7
        bcc @fresh
        cmp #20                 ; Minesweeper, Browser, CD-Player (23): eigene Fenster
        bcc @dlg                ; mit Taskleisten-Knopf
        cmp #22
        bne @fresh
@dlg:   lda NWIN
        beq @fresh
        ; Dialog: Zustand des darunterliegenden Fensters erben
        dec a
        jsr recaddr
        sta PB
        ldy #0
@cp:    lda (PB),y
        sta (PB2),y
        iny
        iny
        cpy #RECSZ
        bcc @cp
        ldy #R_INH
        lda #1
        sta (PB2),y
        bra @pos
@fresh: ldy #0
        lda #0
@z:     sta (PB2),y
        iny
        iny
        cpy #R_PATH+2
        bcc @z
        ldy #R_SEL
        lda #$FFFF
        sta (PB2),y
        ldy #R_SLOT
        sta (PB2),y
@pos:   ldy #R_W                ; Dialoge und neue Fenster: Standardgroesse
        lda #WW
        sta (PB2),y
        ldy #R_H
        lda #WH
        ldx TMPK
        cpx #22                 ; Netzwerk-Dialog: 20 Punkte hoeher
        bne @hh
        lda #WH+20
@hh:    sta (PB2),y
        ldy #R_MAXI
        lda #0
        sta (PB2),y
        ldy #R_KIND
        lda TMPK
        sta (PB2),y
        inc WSEQ
        ldy #R_ID
        lda WSEQ
        sta (PB2),y
        lda TMPK
        cmp #7
        bcc @casc
        lda TMPK
        cmp #22
        bne @dm
        lda #WX                 ; Netzwerk-Dialog: 10 Punkte hoeher setzen
        ldx #WY-10
        bra @sp
@dm:    lda #WX                 ; Dialoge mittig
        ldx #WY
        bra @sp
@casc:  lda NWIN                ; Fenster versetzt, rechts der Desktop-Symbole
        and #3
        asl
        asl
        asl                     ; *8
        pha
        clc
        adc #62
        tax
        pla
        clc
        adc #4
        sta TMP
        txa
        ldx TMP
@sp:    ldy #R_X
        sta (PB2),y
        txa
        ldy #R_Y
        sta (PB2),y
        lda NWIN
        inc NWIN
        jsr win_load
        clc
        rts

; Slot-Feld des obersten Fensters setzen (A = Slot)
win_setslot:
        pha
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        pla
        ldy #R_SLOT
        sta (PB),y
        asl
        tax
        lda slotbase,x
        sta ENTBASE
        lda slotbank,x
        sta ENTBK
        rts

; oberstes Fenster schliessen (ohne Neuzeichnen)
win_pop:
        lda NWIN
        bne @go
        rts
@go:    jsr win_save
        lda NWIN
        dec a
        jsr recaddr
        sta PB2
        ldy #R_INH
        lda (PB2),y
        beq @own
        lda NWIN
        cmp #2
        bcc @drop
        ; Zustand an das Fenster darunter zurueckgeben
        sec
        sbc #2
        jsr recaddr
        sta PB
        ldy #R_MODE
@cb:    lda (PB2),y
        sta (PB),y
        iny
        iny
        cpy #R_INH
        bcc @cb
        ldy #R_PATH
@cb2:   lda (PB2),y
        sta (PB),y
        iny
        iny
        cpy #RECSZ
        bcc @cb2
        bra @drop
@own:   ldy #R_KIND
        lda (PB2),y
        cmp #1
        bne @drop
        ldy #R_SLOT
        lda (PB2),y
        bmi @drop
        asl
        tax
        stz slotuse,x
@drop:  dec NWIN
        lda NWIN
        beq @none
        dec a
        jmp win_load
@none:  stz WIN
        stz DXO
        stz DYO
        stz MODE
        stz ISBIN
        stz COUNT
        rts

win_close:
        stz DROP
        jsr top_rect            ; Flaeche des Fensters merken
        jsr win_pop
        lda NWIN                ; neues oberstes Fenster wird aktiv
        beq @r
        dec a
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        sta KL
        ldy #R_Y
        lda (PB),y
        sta KT
        jsr rect_union
@r:     jsr redraw_rect
        ; Taskleiste (Fensterknoepfe)
        lda #186
        sta KT
        lda #200
        sta KB
        stz KL
        lda #320
        sta KR
        jmp redraw_rect

win_closeall:
        stz NWIN
        ldx #NSLOT*2-2
@su:    lda slotuse,x           ; 2 = Slot ohne Speicher, bleibt gesperrt
        cmp #2
        beq @sk
        stz slotuse,x
@sk:    dex
        dex
        bpl @su
        stz WIN
        stz DXO
        stz DYO
        stz DROP
        rts

; Fenster A nach vorn holen
win_front:
        sta TMPK
        inc a
        cmp NWIN
        bne @go
        rts                     ; ist schon vorn
@go:    jsr win_save
        lda TMPK
        jsr recaddr
        sta PB
        ldy #0
@t:     lda (PB),y
        sta wtemp,y
        iny
        iny
        cpy #RECSZ
        bcc @t
        ; nachfolgende Datensaetze nach unten schieben
@sh:    lda TMPK
        inc a
        cmp NWIN
        bcs @put
        jsr recaddr
        sta PB2
        lda TMPK
        jsr recaddr
        sta PB
        ldy #0
@m:     lda (PB2),y
        sta (PB),y
        iny
        iny
        cpy #RECSZ
        bcc @m
        inc TMPK
        bra @sh
@put:   lda NWIN
        dec a
        jsr recaddr
        sta PB
        ldy #0
@p:     lda wtemp,y
        sta (PB),y
        iny
        iny
        cpy #RECSZ
        bcc @p
        lda NWIN
        dec a
        jmp win_load

; neues Explorer-Fenster. C=1 wenn keins mehr moeglich (dann aktuelles nutzen)
new_explorer:
        ldx #0
@f:     lda slotuse,x
        beq @free
        inx
        inx
        cpx #NSLOT*2
        bcc @f
        ; kein Slot frei: oberstes Explorer-Fenster weiterverwenden
        lda WIN
        cmp #1
        beq @reuse
        lda #8
        jsr show_msg_nr
        sec
        rts
@reuse: clc
        rts
@free:  phx
        lda #1
        jsr win_open
        plx
        bcc @ok
        rts
@ok:    lda #1
        sta slotuse,x
        txa
        lsr
        jsr win_setslot
        clc
        rts

show_msg_nr:
        sta ERRMSG
        lda #9
        jsr win_open
        rts

; ---------- Ursprung fuer Desktop-Elemente ----------
org0:   lda ORGDEP
        bne @n
        lda DXO
        sta SDXO
        lda DYO
        sta SDYO
@n:     inc ORGDEP
        stz DXO
        stz DYO
        rts
orgr:   dec ORGDEP
        bne @r
        lda SDXO
        sta DXO
        lda SDYO
        sta DYO
@r:     rts

lmx:    lda MX
        sec
        sbc DXO
        rts
lmy:    lda MY
        sec
        sbc DYO
        rts

; ---------- Taskleisten-Schaltflaechen ----------
; X-Position der k-ten Schaltflaeche
TBX0 = 135                       ; nach Start-Knopf und Schnellstart-Symbolen
TBW  = 76
TBMAX = 8

; Index des k-ten Fensters (nach ID, ohne Dialoge) -> A, C=1 wenn keins
task_rec:
        sta TMPK
        stz LASTID
@k:     ; kleinste ID > LASTID suchen
        lda #$FFFF
        sta BESTID
        stz IDX
@l:     lda IDX
        cmp NWIN
        bcs @e
        jsr recaddr
        sta PB
        ldy #R_INH
        lda (PB),y
        bne @n
        ldy #R_ID
        lda (PB),y
        cmp LASTID
        beq @n
        bcc @n
        cmp BESTID
        bcs @n
        sta BESTID
        lda IDX
        sta BESTIX
@n:     inc IDX
        bra @l
@e:     lda BESTID
        cmp #$FFFF
        bne @f
        sec
        rts
@f:     sta LASTID
        lda TMPK
        beq @got
        dec TMPK
        bra @k
@got:   lda BESTIX
        clc
        rts

draw_tasks:
        jsr win_save
        jsr tb_count
        stz FCOUNT
@l:     lda FCOUNT
        cmp TNOWN
        bcs @end
        jsr task_rec
        bcs @end
        sta IDX
        jsr win_load
        stz DXO
        stz DYO
        lda FCOUNT
        jsr tb_x
        sta RX
        lda #188
        sta RY
        lda TBWV
        dec a
        dec a
        sta RW
        lda #11
        sta RH
        lda #GRAY
        jsr setcolor
        jsr fillrect
        lda IDX
        jsr is_active
        bcc @up
        lda #BLACK
        sta C1
        lda #WHITE
        sta C2
        bra @bv
@up:    lda #WHITE
        sta C1
        lda #BLACK
        sta C2
@bv:    jsr bevel
        lda RX
        clc
        adc #4
        sta SX
        clc
        adc TBWV
        sec
        sbc #9
        sta CLIPR
        lda #190
        sta SY
        lda #BLACK
        jsr setcolor
        jsr draw_title
        lda #320
        sta CLIPR
        inc FCOUNT
        jmp @l
@end:   jsr tb_ndadraw
        lda NWIN
        beq @r
        dec a
        jmp win_load
@r:     rts

tb_x:   tax
        lda #TBX0
@l:     cpx #0
        beq @r
        clc
        adc TBWV
        dex
        bra @l
@r:     rts

; Fenster zaehlen -> TNOWN (eigene), TNNDA (Zubehoer), Knopfbreite TBWV
tb_count:
        stz TNOWN
@l:     lda TNOWN
        cmp #TBMAX
        bcs @o
        jsr task_rec
        bcs @o
        inc TNOWN
        bra @l
@o:     jsr nda_list
        lda TNOWN
        clc
        adc TNNDA
        sta TNALL
        cmp #3
        bcs @c
        lda #3
@c:     tax
        lda #267-TBX0           ; bis zum Infobereich (Netzwerksymbol, Uhr)
        jsr gdiv
        sta TBWV
        rts

; Knoepfe fuer offene Zubehoer-Fenster
tb_ndadraw:
        stz TNK
@l:     lda TNK
        cmp TNNDA
        bcc @go
        rts
@go:    clc
        adc TNOWN
        jsr tb_x
        sta RX
        lda #188
        sta RY
        lda TBWV
        dec a
        dec a
        sta RW
        lda #11
        sta RH
        lda #GRAY
        jsr setcolor
        jsr fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        lda NDAFOC              ; vorderstes Zubehoer hat den Fokus: gedrueckt
        beq @up
        lda TNK
        bne @up
        lda #BLACK
        sta C1
        lda #WHITE
        sta C2
@up:    jsr bevel
        lda RX
        clc
        adc #4
        sta SX
        clc
        adc TBWV
        sec
        sbc #9
        sta CLIPR
        lda #190
        sta SY
        lda TNK
        jsr nda_title
        lda #BLACK
        jsr setcolor
        jsr drawstr
        lda #320
        sta CLIPR
        inc TNK
        bra @l

; C=1 wenn Fenster A das aktive ist (oberstes ohne Dialoge darueber)
is_active:
        pha
        lda NDAFOC              ; Zubehoer-Fenster vorne: keins der eigenen aktiv
        beq @nf
        pla
        clc
        rts
@nf:    pla
        pha                     ; minimierte Fenster sind nie aktiv
        jsr win_ismin
        pla
        bcc @vis
        clc
        rts
@vis:   sta TMP2
        lda NWIN
@l:     dec a
        bmi @no
        pha
        jsr recaddr
        sta PB2
        pla
        pha
        ldy #R_INH
        lda (PB2),y
        beq @f
        pla
        bra @l
@f:     pla
        cmp TMP2
        beq @yes
@no:    clc
        rts
@yes:   sec
        rts

; Klick auf Taskleisten-Schaltflaeche? C=1 wenn behandelt
task_click:
        lda MY
        cmp #188
        bcc @no
        lda MX
        cmp #TBX0
        bcc @no
        sec
        sbc #TBX0
        pha
        jsr tb_count
        pla
        ldx TBWV
        jsr gdiv
        cmp TNALL
        bcs @no
        cmp TNOWN
        bcs @nda
        jsr task_rec
        bcs @no
        sta TMIDX
        jsr win_ismin
        bcs @unm
        lda TMIDX               ; aktives Fenster angeklickt: minimieren
        inc a
        cmp NWIN
        bne @fr
        jsr is_mini
        bcc @fr
        jsr win_minimize
        sec
        rts
@unm:   lda TMIDX
        jsr win_unmin
@fr:    lda TMIDX
        jsr win_front
        jsr redraw_all
        sec
        rts
@nda:   sec
        sbc TNOWN
        jsr nda_select
        sec
        rts
@no:    clc
        rts

; Trifft der Klick das Fenster A? C=1 wenn ja
win_hit:
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        sta RX
        ldy #R_Y
        lda (PB),y
        sta RY
        ldy #R_W
        lda (PB),y
        sta RW
        ldy #R_H
        lda (PB),y
        sta RH
        lda MX
        sec
        sbc RX
        bmi @no
        cmp RW
        bcs @no
        lda MY
        sec
        sbc RY
        bmi @no
        cmp RH
        bcs @no
        sec
        rts
@no:    clc
        rts

; ---------- Fenster verschieben ----------
drag_window:
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
        lda UH
        sta DFH
        ldy #R_X
        lda (PB),y
        sta DOX
        sta DNX
        ldy #R_Y
        lda (PB),y
        sta DOY
        sta DNY
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
        adc DOX
        cmp #$8000
        bcc @x1
        cmp #$FF38
        bcs @x2
        lda #$FF38
        bra @x2
@x1:    cmp #300
        bcc @x2
        lda #300
@x2:    sta TMP
        lda EVREC+10
        sec
        sbc DSY
        clc
        adc DOY
        bpl @y1
        lda #0
@y1:    cmp #172
        bcc @y2
        lda #172
@y2:    cmp DNY
        bne @mv
        lda TMP
        cmp DNX
        bne @mv0
        jmp @loop
@mv0:   lda DNY
@mv:    pha
        jsr xor_frame           ; alten Rahmen loeschen
        pla
        sta DNY
        lda TMP
        sta DNX
        jsr xor_frame
        jmp @loop
@done:  jsr xor_frame
        lda EVREC+12
        sta MX
        lda EVREC+10
        sta MY
        jsr orgr
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        ldy #R_X
        lda DNX
        sta (PB),y
        ldy #R_Y
        lda DNY
        sta (PB),y
        sec
        sbc #WY
        sta DYO
        lda DNX
        sec
        sbc #WX
        sta DXO
        ; nur alte + neue Fensterflaeche neu zeichnen
        lda DOX
        sta KL
        lda DOY
        sta KT
        jsr rect_fromk
        lda DNX
        sta KL
        lda DNY
        sta KT
        jsr rect_union
        jmp redraw_rect

; XOR-Rahmen (2 Pixel) bei DNX,DNY in Fenstergroesse
xor_frame:
        lda #0
        sta TMPK
@h:     ; obere und untere Kante
        lda DNX
        clc
        adc TMPK
        tax
        lda DNY
        tay
        jsr xorpix
        lda DNX
        clc
        adc TMPK
        tax
        lda DNY
        inc a
        tay
        jsr xorpix
        lda DNX
        clc
        adc TMPK
        tax
        lda DNY
        clc
        adc DFH
        dec a
        tay
        jsr xorpix
        lda DNX
        clc
        adc TMPK
        tax
        lda DNY
        clc
        adc DFH
        dec a
        dec a
        tay
        jsr xorpix
        inc TMPK
        lda TMPK
        cmp DFW
        bcc @h
        lda #2
        sta TMPK
@v:     lda DNX
        tax
        lda DNY
        clc
        adc TMPK
        tay
        jsr xorpix
        lda DNX
        inc a
        tax
        lda DNY
        clc
        adc TMPK
        tay
        jsr xorpix
        lda DNX
        clc
        adc DFW
        dec a
        tax
        lda DNY
        clc
        adc TMPK
        tay
        jsr xorpix
        lda DNX
        clc
        adc DFW
        dec a
        dec a
        tax
        lda DNY
        clc
        adc TMPK
        tay
        jsr xorpix
        inc TMPK
        lda TMPK
        inc a
        inc a
        cmp DFH
        bcc @v
        rts

; Pixel X,Y invertieren (Bildschirmkoordinaten)
xorpix: cpx #320
        bcs @out
        cpy #186
        bcs @out
        tya
        asl
        tay
        lda linetab,y
        sta PTR
        txa
        lsr
        tay
        bcs @odd
        sep #$20
        lda [PTR],y
        eor #$F0
        sta [PTR],y
        rep #$20
        rts
@odd:   sep #$20
        lda [PTR],y
        eor #$0F
        sta [PTR],y
        rep #$20
@out:   rts

; ================= Teilweises Neuzeichnen =================
; Clip = ganzer Bildschirm
clip_full:
        stz KL
        stz KT
        lda #320
        sta KR
        lda #200
        sta KB
        rts

; Rechteck des obersten Fensters -> KL..KB (gemerkt in UL..UB)
top_rect:
        lda NWIN
        bne @w
        jmp clip_full
@w:     dec a
        jsr recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        sta KL
        ldy #R_Y
        lda (PB),y
        sta KT
        jsr rec_size
; KL/KT = Fensterecke (Groesse in UW/UH) -> ganzes Fensterrechteck, als UL..UB merken
rect_fromk:
        lda KL
        sta UL
        clc
        adc UW
        sta UR
        lda KT
        sta UT
        clc
        adc UH
        sta UB
        bra rect_set
; Fenster bei KL/KT mit UL..UB vereinigen
rect_union:
        lda KL
        cmp UL
        bpl @a
        sta UL
@a:     clc
        adc UW
        cmp UR
        bmi @b
        sta UR
@b:     lda KT
        cmp UT
        bpl @c
        sta UT
@c:     clc
        adc UH
        cmp UB
        bmi rect_set
        sta UB
; UL..UB auf den Bildschirm begrenzen -> KL..KB
rect_set:
        lda UL
        bpl @l
        lda #0
@l:     sta KL
        lda UR
        cmp #320
        bmi @r
        lda #320
@r:     sta KR
        lda UT
        bpl @t
        lda #0
@t:     sta KT
        lda UB
        cmp #200
        bmi @u
        lda #200
@u:     sta KB
        rts

; Groesse des Datensatzes (PB) -> UW/UH
rec_size:
        ldy #R_W
        lda (PB),y
        sta UW
        ldy #R_H
        lda (PB),y
        sta UH
        rts

; nur das oberste Fenster neu zeichnen
redraw_win:
        lda NDAWAS
        beq @own
        jmp redraw_all
@own:   jsr top_rect
        jmp redraw_rect

; nur das oberste Fenster und die Taskleiste neu zeichnen
redraw_top:
        lda NDAWAS
        beq @own
        jmp redraw_all
@own:   jsr top_rect
        jsr redraw_rect
        lda #186
        sta KT
        lda #200
        sta KB
        stz KL
        lda #320
        sta KR
; Bereich KL..KR/KT..KB im Puffer aufbauen und auf den Bildschirm kopieren
redraw_rect:
        lda VTON                ; Vollbild-Terminal: nichts zeichnen
        beq *+3
        rts
        lda SPLON
        beq *+3
        rts
        lda NDAWAS
        beq @own
        jsr clip_full
        jmp redraw_all
@own:   lda KR
        cmp KL
        bne *+5
        jmp @e
        bcs *+5
        jmp @e
        lda KB
        cmp KT
        bne *+5
        jmp @e
        bcs *+5
        jmp @e
        jsr off_alloc
        lda CURVIS
        pha
        jsr cur_hide
        jsr scr_off
        lda #1
        sta OFFON
        jsr redraw_core
        stz OFFON
        jsr scr_on
        stz SPLSW
        lda SPLEND              ; Schreibtisch fertig aufgebaut: Startbild abloesen
        beq @nsp
        stz SPLEND
        inc SPLSW               ; Zeile fuer Zeile: erst Punkte, dann SCB 0
        jsr waitvbl
        ldx #30                 ; Palette 0 (nur Balkenzeilen benutzen sie noch)
@p0:    lda palettes,x
        sta f:$E19E00,x
        dex
        dex
        bpl @p0
@nsp:   ; Zeilen kopieren: Bytes KL/2 .. (KR+1)/2
        lda #$E1
        xba
        ora OFFB
        xba                     ; lo = Ziel E1, hi = Quelle
        sta ecmv+1
        lda KL
        lsr
        sta TMP                 ; erstes Byte
        lda KR
        inc a
        lsr
        sec
        sbc TMP
        dec a
        sta TMP2                ; Anzahl-1
        lda KT
        sta YC
@row:   lda YC
        asl
        tax
        lda linetab,x
        clc
        adc TMP
        tay
        sec
        sbc #$2000
        clc
        adc OFFA
        tax
        lda TMP2
        jsr ec_n
        lda SPLSW
        beq @ns
        ldx YC
        sep #$20
        lda #0
        sta f:$E19D00,x
        rep #$20
@ns:    inc YC
        lda YC
        cmp KB
        bcc @row
        lda SPLSW
        beq @nd
        jsr desktop_scb         ; alle SCBs 0, Paletten des Schreibtischs
@nd:
        pla
        beq @e
        jsr cur_show
@e:     jmp clip_full

; Startmenue geschlossen: nur dessen Flaeche + Taskleiste
redraw_menu:
        stz SUBM
        lda #MNX
        sta KL
        lda #SUBX+SUBW+2
        sta KR
        lda #MNY-2
        sta KT
        lda #200
        sta KB
        jmp redraw_rect
