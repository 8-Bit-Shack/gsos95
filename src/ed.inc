; ================================================================
; GS/OS95: Editor (Notepad), Fensterart 28, XCODE4
; Text bis 32000 Zeichen in einem eigenen Griff, Zeilenende CR (ProDOS).
; Feste Zeichenzellen, Umbruch am Fensterrand (an Leerzeichen).
; Tasten: Pfeile, Apfel-Pfeile (Zeilenanfang/-ende, Seite), Wahl-Pfeile
; (Wort), Return, Delete, Strg-D (Zeichen rechts loeschen), Apfel-S
; speichern, Apfel-N neu, Esc schliessen (fragt bei Aenderungen).
; ================================================================
EDKIND  = 28
ED_MAX  = 32000
ED_CW   = 6                     ; Zeichenzelle
ED_LH   = 10
ED_X0   = WX+5
ED_Y0   = WY+18

        .pushseg
        .segment "BSS"
EDH:    .res 4                  ; Griff des Textes
EDP:    .res 4
EDLEN:  .res 2
EDCUR:  .res 2
EDTOP:  .res 2                  ; erste sichtbare Bildschirmzeile (Anfang)
EDMOD:  .res 2                  ; 1 = geaendert
EDRO:   .res 2                  ; 1 = zu gross geladen, nur lesen
EDNEW:  .res 2                  ; 1 = noch ohne Namen
EDGOAL: .res 2                  ; gewuenschte Spalte (hoch/runter)
EDPR:   .res 2                  ; 1 = Dateiname wird eingegeben
EDESC:  .res 2                  ; 1 = Esc einmal gedrueckt (Rueckfrage)
EDMSG:  .res 2                  ; Meldung (Adresse deutsch) oder 0
EDMSGE: .res 2                  ; ... englisch
EDCOLS: .res 2
EDROWS: .res 2
EDS:    .res 2
EDE:    .res 2
EDN:    .res 2
EDSP:   .res 2
EDK:    .res 2
EDQ:    .res 2
EDVS:   .res 2
EDR:    .res 2
EDR0:   .res 2                  ; ab dieser Zeile neu zeichnen
EDI:    .res 2
EDT:    .res 2
EDOLD:  .res 2
EDR1:   .res 2                  ; nur Zeilen vor dieser zeichnen ($FFFF alle)
EDNL:   .res 2
EDDL:   .res 2
EDRNB:  .res 2
EDSD:   .res 2
EDSX:   .res 2
EDSY:   .res 2
EDSH:   .res 2
EDBL:   .res 2
EDSA:   .res 2
EDSB:   .res 2
EDSC:   .res 2
WCELL:  .res 2                  ; Zellbreite fuer xf_cells
EDPATH: .res 68                 ; GS/OS-Zeichenkette (Wortlaenge + Pfad)
EDRB:   .res 100                ; Zeile zum Zeichnen
EDPB:   .res 16                 ; GS/OS-Parameter
EDREF:  .res 2
EDCNT:  .res 2
        .popseg

; ---------- Oeffnen ----------
; Startmenue: leeres Dokument
xf_ed_new:
        jsr ed_find
        bcs @front
        jsr ed_mem
        bcs @r
        jsr ed_clear
        jsr ed_win
@r:     rtl
@front: jsr ed_tofront
        rtl

; Explorer: Datei srcpath (GS/OS-Zeichenkette, entry_path) oeffnen
xf_ed_file:
        jsr ed_find
        bcc @n
        lda EDMOD               ; schon offen und geaendert: erst speichern
        beq @ld
        jsr ed_tofront
        lda #ed_tbusy
        ldx #ed_tbusy_en
        jsr ed_smsg
        rtl
@n:     jsr ed_mem
        bcs @r
@ld:    jsr ed_clear
        ldx #0                  ; Pfad uebernehmen
@p:     lda srcpath,x
        sta EDPATH,x
        inx
        inx
        cpx #66
        bcc @p
        lda EDPATH
        cmp #64
        bcs @r
        stz EDNEW
        jsr ed_load
        jsr ed_find
        bcs @fr
        jsr ed_win
@r:     rtl
@fr:    jsr ed_tofront
        rtl

ed_tofront:
        pha
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        rts

ed_find:
        stz EDK
@l:     lda EDK
        cmp NWIN
        bcs @no
        CALL recaddr
        sta PB
        lda (PB)
        cmp #EDKIND
        beq @y
        inc EDK
        bra @l
@y:     lda EDK
        sec
        rts
@no:    clc
        rts

ed_win: lda #EDKIND
        CALL win_open
        bcc @o
        lda #8
        CALL show_msg_nr
        CALL redraw_all
        rts
@o:     lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda #8
        sta (PB),y
        ldy #R_Y
        lda #12
        sta (PB),y
        ldy #R_W
        lda #304
        sta (PB),y
        ldy #R_H
        lda #172
        sta (PB),y
        lda NWIN
        dec a
        CALL win_load
        CALL redraw_all
        rts

; Speicher holen (32 KB, fest). C=1 kein Speicher
ed_mem: lda EDH
        ora EDH+2
        bne @ok
        pha
        pha
        pea $0000
        pea $8000
        lda USERID
        pha
        pea $C018
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta EDH
        pla
        sta EDH+2
        bcc @g
        stz EDH
        stz EDH+2
        lda #8
        CALL show_msg_nr
        CALL redraw_all
        sec
        rts
@g:     lda EDH
        sta PDST
        lda EDH+2
        sta PDST+2
        ldy #2
        lda [PDST],y
        sta EDP+2
        lda [PDST]
        sta EDP
@ok:    clc
        rts

ed_free:
        lda EDH
        ora EDH+2
        beq @r
        lda EDH+2
        pha
        lda EDH
        pha
        TOOL $1002              ; DisposeHandle
        stz EDH
        stz EDH+2
@r:     rts

ed_clear:
        lda #$FFFF
        sta EDR1
        stz EDLEN
        stz EDCUR
        stz EDTOP
        stz EDMOD
        stz EDRO
        stz EDGOAL
        stz EDPR
        stz EDESC
        stz EDMSG
        stz EDPATH
        lda #1
        sta EDNEW
        rts

; Zeiger auf den Text nach PSRC
ed_ptr: lda EDP
        sta PSRC
        lda EDP+2
        sta PSRC+2
        rts

; GS/OS-Aufruf A mit EDPB
ed_gs:  sta EDT
        lda MYBANK
        pha
        lda #EDPB
        pha
        lda EDT
        pha
        jsl $E100B0
        rts

; ---------- Laden: EDPATH -> Text ----------
ed_load:
        lda #3                  ; Open (lesen)
        sta EDPB
        lda #EDPATH
        sta EDPB+4
        lda MYBANK
        sta EDPB+6
        lda #1
        sta EDPB+8
        lda #$2010
        jsr ed_gs
        bcc @o
        lda #ed_tnold
        ldx #ed_tnold_en
        jmp ed_smsgq
@o:     lda EDPB+2
        sta EDREF
        lda #4                  ; Read
        sta EDPB
        lda EDREF
        sta EDPB+2
        lda EDP
        sta EDPB+4
        lda EDP+2
        sta EDPB+6
        lda #ED_MAX+1
        sta EDPB+8
        stz EDPB+10
        lda #$2012
        jsr ed_gs
        lda EDPB+12
        bcc @n
        cmp #0                  ; Fehler, aber gelesen? sonst leer
        bne @n
        lda #0
@n:     sta EDCNT
        jsr ed_close
        lda EDCNT
        cmp #ED_MAX+1
        bcc @fit
        lda #1
        sta EDRO
        lda #ED_MAX
        sta EDCNT
        lda #ed_tbig
        ldx #ed_tbig_en
        jsr ed_smsgq
@fit:   jsr ed_ptr              ; LF und CR LF -> CR
        ldy #0
        ldx #0
        stz EDT                 ; vorheriges Zeichen
@c:     cpy EDCNT
        bcs @e
        lda [PSRC],y
        and #$00FF
        cmp #10
        bne @nl
        lda EDT
        cmp #13
        beq @sk
        lda #13
@nl:    sta EDT
        phy
        txy
        sep #$20
        sta [PSRC],y
        rep #$20
        ply
        inx
        iny
        bra @c
@sk:    stz EDT
        iny
        bra @c
@e:     stx EDLEN
        rts

ed_close:
        lda #1
        sta EDPB
        lda EDREF
        sta EDPB+2
        lda #$2014
        jmp ed_gs

; ---------- Speichern ----------
ed_save:
        lda EDRO
        beq @w
        lda #ed_tro
        ldx #ed_tro_en
        jmp ed_smsg
@w:     lda EDNEW               ; ohne Namen: erst fragen
        beq @go
        jmp ed_prompt
@go:    lda #3                  ; Create (gibt es sie schon: egal)
        sta EDPB
        lda #EDPATH
        sta EDPB+2
        lda MYBANK
        sta EDPB+4
        lda #$00C3
        sta EDPB+6
        lda #$0004              ; TXT
        sta EDPB+8
        lda #$2001
        jsr ed_gs
        lda #3                  ; Open (lesen und schreiben)
        sta EDPB
        lda #EDPATH
        sta EDPB+4
        lda MYBANK
        sta EDPB+6
        lda #3
        sta EDPB+8
        lda #$2010
        jsr ed_gs
        bcs @err
        lda EDPB+2
        sta EDREF
        lda #4                  ; Write
        sta EDPB
        lda EDREF
        sta EDPB+2
        lda EDP
        sta EDPB+4
        lda EDP+2
        sta EDPB+6
        lda EDLEN
        sta EDPB+8
        stz EDPB+10
        lda #$2013
        jsr ed_gs
        php
        lda #3                  ; SetEOF auf die Laenge
        sta EDPB
        lda EDREF
        sta EDPB+2
        stz EDPB+4
        lda EDLEN
        sta EDPB+6
        stz EDPB+8
        lda #$2018
        jsr ed_gs
        jsr ed_close
        plp
        bcs @err
        stz EDMOD
        stz EDESC
        lda #ed_tsaved
        ldx #ed_tsaved_en
        jsr ed_smsg
        CALL redraw_all         ; Titel (auch Taskleiste)
        rts
@err:   lda #ed_terr
        ldx #ed_terr_en
        jmp ed_smsg

; Dateiname eingeben (Statuszeile)
ed_prompt:
        lda #1
        sta EDPR
        lda EDPATH              ; Vorschlag *:TEXT
        bne @s
        lda #6
        sta EDPATH
        ldx #0
@c:     TBL ed_tdef
        sep #$20
        sta EDPATH+2,x
        rep #$20
        inx
        cpx #6
        bcc @c
@s:     jsr ed_kwin
        jmp ed_stat

; ---------- Meldungen ----------
ed_smsgq:                       ; nur merken (Fenster noch nicht da)
        sta EDMSG
        stx EDMSGE
        rts
ed_smsg:
        sta EDMSG
        stx EDMSGE
        jsr ed_kwin
        jmp ed_stat

; ---------- Zeilen berechnen ----------
; Bildschirmzeile ab A: EDE = Ende (ohne CR), EDN = naechste ($FFFF am Textende)
ed_vl:  sta EDS
        stz EDK
        lda #$FFFF
        sta EDSP
        ldy EDS
@l:     cpy EDLEN
        bcs @eof
        lda [PSRC],y
        and #$00FF
        cmp #13
        beq @cr
        ldx EDK
        cpx EDCOLS
        bcs @wrap
        cmp #' '
        bne @n
        sty EDSP
@n:     iny
        inc EDK
        bra @l
@cr:    sty EDE
        iny
        sty EDN
        rts
@eof:   sty EDE
        lda #$FFFF
        sta EDN
        rts
@wrap:  cmp #' '                ; genau am Rand ein Leerzeichen: dort umbrechen
        bne @w2
        sty EDE
        iny
        sty EDN
        rts
@w2:    lda EDSP
        cmp #$FFFF
        beq @hard
        inc a
        sta EDE
        sta EDN
        rts
@hard:  sty EDE
        sty EDN
        rts

; letzte erlaubte Cursorstelle der Zeile ab EDS (nach ed_vl) -> A
ed_maxp:
        lda EDN
        cmp #$FFFF
        beq @e
        ldy EDE
        cpy EDLEN
        bcs @w
        lda [PSRC],y
        and #$00FF
        cmp #13
        beq @e
@w:     lda EDN
        dec a
        rts
@e:     lda EDE
        rts

; Anfang der Textzeile, in der A liegt
ed_ls:  tay
@l:     cpy #0
        beq @r
        dey
        lda [PSRC],y
        and #$00FF
        cmp #13
        bne @l
        iny
@r:     tya
        rts

; Bildschirmzeile mit Stelle A -> EDVS (Anfang), EDE/EDN
ed_vof: sta EDQ
        jsr ed_ls
@l:     sta EDVS
        jsr ed_vl
        lda EDN
        cmp #$FFFF
        beq @r
        lda EDQ
        cmp EDN
        bcc @r
        lda EDN
        bra @l
@r:     rts

; Cursor in Zeile ab A auf Spalte EDGOAL (hoechstens Zeilenende)
ed_col: sta EDVS
        jsr ed_vl
        jsr ed_maxp
        sta EDK
        lda EDVS
        clc
        adc EDGOAL
        bcs @m
        cmp EDK
        bcc @ok
@m:     lda EDK
@ok:    sta EDCUR
        rts

; Spalte merken
ed_goal:
        lda EDCUR
        jsr ed_vof
        lda EDCUR
        sec
        sbc EDVS
        sta EDGOAL
        rts

; Groesse aus CW/CH
ed_geom:
        lda CW
        sec
        sbc #10
        ldx #ED_CW
        CALL gdiv
        cmp #8
        bcs @c
        lda #8
@c:     cmp #90
        bcc @c2
        lda #90
@c2:    sta EDCOLS
        lda CH
        sec
        sbc #35
        ldx #ED_LH
        CALL gdiv
        cmp #1
        bcs @r
        lda #1
@r:     sta EDROWS
        rts

; erste Zeile gueltig machen und Cursor sichtbar halten
ed_show:
        jsr ed_ptr
        lda EDTOP               ; nach Aenderungen: Anfang einer Bildschirmzeile
        cmp EDLEN
        bcc @t
        lda EDLEN
@t:     jsr ed_vof
        lda EDVS
        sta EDTOP
        lda EDCUR
        cmp EDTOP
        bcs @a
        jsr ed_vof
        lda EDVS
        sta EDTOP
        rts
@a:     lda EDTOP
        stz EDR
@l:     sta EDS
        jsr ed_vl
        lda EDN
        cmp #$FFFF
        beq @vis
        lda EDCUR
        cmp EDN
        bcc @vis
        inc EDR
        lda EDR
        cmp EDROWS
        bcs @nv
        lda EDN
        bra @l
@vis:   rts
@nv:    lda EDCUR               ; Cursor in die unterste Zeile
        jsr ed_vof
        lda EDVS
        sta EDTOP
        lda EDROWS
        dec a
        sta EDR
@up:    lda EDR
        beq @r
        lda EDTOP
        beq @r
        dec a
        jsr ed_vof
        lda EDVS
        sta EDTOP
        dec EDR
        bra @up
@r:     rts

; ---------- Zeichnen ----------
xf_ed_draw:
        jsr ed_geom
        jsr ed_show
        lda #WX+2               ; Textfeld
        sta RX
        lda #WY+15
        sta RY
        lda CW
        sec
        sbc #4
        sta RW
        lda CH
        sec
        sbc #29
        sta RH
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        stz EDR0
        lda #$FFFF
        sta EDR1
        jsr ed_rows
        jmp ed_stat

; Clip fuer Zeichnen ausserhalb des Neuzeichnens (oberstes Fenster)
ed_kwin:
        lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        sta KL
        ldy #R_W
        clc
        adc (PB),y
        sta KR
        ldy #R_Y
        lda (PB),y
        sta KT
        ldy #R_H
        clc
        adc (PB),y
        sta KB
        lda KB
        cmp #188
        bcc @r
        lda #188
        sta KB
@r:     rts

; Zeilen ab EDR0 zeichnen
ed_rows:
        jsr ed_ptr
        lda EDTOP
        sta EDS
        stz EDR
@row:   lda EDR
        cmp EDROWS
        bcs @end
        cmp EDR1
        bcc @go
@end:   rts
@go:    lda EDR
        cmp EDR0
        bcs @dr
        lda EDS                 ; nur weiterzaehlen
        cmp #$FFFF
        bne *+5
        jmp @nx
        jsr ed_vl
        lda EDN
        sta EDS
        brl @nx
@dr:    lda #ED_X0-1            ; Zeile weiss
        sta RX
        lda EDR
        ldx #ED_LH
        CALL gmul
        clc
        adc #ED_Y0
        sta RY
        sta SY
        lda EDCOLS
        ldx #ED_CW
        CALL gmul
        clc
        adc #2
        sta RW
        lda #ED_LH
        sta RH
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda EDS
        cmp #$FFFF
        beq @nx
        jsr ed_vl
        jsr ed_line             ; Zeichen
        lda EDCUR               ; Cursor in dieser Zeile?
        cmp EDS
        bcc @nc
        lda EDN
        cmp #$FFFF
        beq @cu
        lda EDCUR
        cmp EDN
        bcs @nc
@cu:    lda EDPR
        bne @nc
        lda EDCUR
        sec
        sbc EDS
        ldx #ED_CW
        CALL gmul
        clc
        adc #ED_X0-1
        sta RX
        lda EDR
        ldx #ED_LH
        CALL gmul
        clc
        adc #ED_Y0
        sta RY
        lda #1
        sta RW
        lda #ED_LH-1
        sta RH
        lda #BLACK
        CALL setcolor
        CALL fillrect
@nc:    lda EDN
        sta EDS
@nx:    inc EDR
        jmp @row

; Zeichen EDS..EDE der Zeile EDR ausgeben
ed_line:
        ldy EDS
        ldx #0
@c:     cpy EDE
        bcs @d
        cpx #96
        bcs @d
        lda [PSRC],y
        and #$00FF
        jsr ed_map
        sep #$20
        sta EDRB+1,x
        rep #$20
        inx
        iny
        bra @c
@d:     stx EDK
        lda EDK
        beq @r
        lda #BLACK
        CALL setcolor
        lda #ED_X0
        sta SX
        lda #ED_CW
        sta WCELL
        lda #EDRB+1
        sta PDST
        lda MYBANK
        sta PDST+2
        lda EDK
        JSLF xf_cells
@r:     rts

; Zeichen -> Code der Schrift (Umlaute Mac Roman und Latin-1)
ed_map: cmp #$80
        bcs @x
        cmp #' '
        bcs @r
        lda #' '
@r:     rts
@x:     phx
        sta EDQ
        ldx #0
@l:     TBL ed_um
        and #$00FF
        beq @q
        cmp EDQ
        beq @f
        inx
        inx
        bra @l
@f:     inx
        TBL ed_um
        and #$00FF
        plx
        rts
@q:     plx
        lda #'?'
        rts
; Paare: Zeichen, Code der Schrift (ae oe ue Ae Oe Ue ss)
ed_um:  .byte $8A,$80,$9A,$81,$9F,$82,$80,$83,$85,$84,$86,$85,$A7,$86
        .byte $E4,$80,$F6,$81,$FC,$82,$C4,$83,$D6,$84,$DC,$85,$DF,$86,0,0

; Statuszeile
ed_stat:
        lda CH
        sec
        sbc #13
        clc
        adc #WY
        sta RY
        lda #WX+2
        sta RX
        lda CW
        sec
        sbc #4
        sta RW
        lda #11
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda RY
        inc a
        sta SY
        lda #WX+5
        sta SX
        lda #BLACK
        CALL setcolor
        jsr cb_clr
        lda EDPR
        beq @m
        lda #ed_tsas
        ldx #ed_tsas_en
        jsr cb_sl
        ldx #0                  ; Pfad (zu lang: Ende zeigen)
        lda EDPATH
        cmp #36
        bcc @p
        sec
        sbc #35
        tax
@p:     cpx EDPATH
        bcs @pe
        lda EDPATH+2,x
        and #$00FF
        jsr cb_ch
        inx
        bra @p
@pe:    lda #'_'
        jsr cb_ch
        jmp cb_draw
@m:     lda EDMSG
        beq @z
        ldx EDMSGE
        jsr cb_sl
        jmp cb_draw
@z:     lda #ed_tln             ; Zeile, Spalte
        ldx #ed_tln_en
        jsr cb_sl
        jsr ed_ptr
        lda #1
        sta EDK
        ldy #0
@lc:    cpy EDCUR
        bcs @le
        lda [PSRC],y
        and #$00FF
        cmp #13
        bne @ln
        inc EDK
@ln:    iny
        bra @lc
@le:    lda EDK
        ldx #0
        jsr cb_num
        lda #ed_tcol
        ldx #ed_tcol_en
        jsr cb_sl
        lda EDCUR
        jsr ed_ls
        eor #$FFFF
        sec
        adc EDCUR
        inc a
        ldx #0
        jsr cb_num
        lda EDMOD
        beq @h
        lda #ed_tmod
        ldx #ed_tmod_en
        jsr cb_sl
@h:     lda #ed_thelp
        ldx #ed_thelp_en
        jsr cb_sl
        jmp cb_draw

; Fenstertitel (app.s draw_title): "Name - Editor"
xf_ed_title:
        jsr cb_clr
        lda EDNEW
        bne @u
        ldx EDPATH              ; letzter Teil des Pfades
        beq @u
@b:     dex
        bmi @f
        lda EDPATH+2,x
        and #$00FF
        cmp #':'
        bne @b
@f:     inx
@c:     cpx EDPATH
        bcs @e
        lda EDPATH+2,x
        and #$00FF
        jsr cb_ch
        inx
        bra @c
@u:     lda #ed_tun
        ldx #ed_tun_en
        jsr cb_sl
@e:     lda #ed_ttl
        ldx #ed_ttl_en
        jsr cb_sl
        jsr cb_draw
        rtl

; ---------- Klick: Cursor setzen ----------
xf_ed_click:
        lda EDPR
        bne @r
        jsr ed_geom
        CALL lmy
        sec
        sbc #ED_Y0
        bcc @r
        ldx #ED_LH
        CALL gdiv
        cmp EDROWS
        bcs @r
        sta EDI
        CALL lmx
        sec
        sbc #ED_X0-ED_CW/2
        bpl @x
        lda #0
@x:     ldx #ED_CW
        CALL gdiv
        sta EDGOAL
        jsr ed_ptr
        lda EDTOP
@l:     sta EDS
        ldx EDI
        beq @at
        jsr ed_vl
        lda EDN
        cmp #$FFFF
        beq @end
        dec EDI
        lda EDN
        bra @l
@at:    lda EDS
        jsr ed_col
        bra @rd
@end:   lda EDLEN
        sta EDCUR
        jsr ed_goal
@rd:    stz EDMSG
        stz EDESC
        jsr ed_kwin
        stz EDR0
        jsr ed_rows
        jsr ed_stat
@r:     rtl

; ---------- Tasten ----------
xf_ed_key:
        jsr ed_geom
        jsr ed_ptr
        lda EVREC+2
        and #$00FF
        sta EDK
        lda EDPR
        beq @np
        jmp ed_pkey
@np:    lda EDCUR
        sta EDOLD
        lda EDTOP
        sta EDT
        lda KEYC
        cmp #$1B
        bne @ne
        jmp ed_esc
@ne:    stz EDESC
        stz EDMSG
        lda EVREC+14
        and #$0100              ; Apfel
        bne *+5
        jmp @na
        lda KEYC
        and #$005F
        cmp #'S'
        bne @an
        jsr ed_save
        rtl
@an:    cmp #'N'
        bne @aa
        lda EDMOD
        beq @nw
        lda #ed_tbusy
        ldx #ed_tbusy_en
        jsr ed_smsg
        rtl
@nw:    jsr ed_clear
        CALL redraw_win
        rtl
@aa:    lda KEYC
        cmp #$08
        bne @a1
        lda EDCUR               ; Zeilenanfang
        jsr ed_vof
        lda EDVS
        sta EDCUR
        jmp @mvh
@a1:    cmp #$15
        bne @a2
        lda EDCUR               ; Zeilenende
        jsr ed_vof
        jsr ed_maxp
        sta EDCUR
        jmp @mvh
@a2:    cmp #$0B
        bne @a3
        lda EDROWS              ; Seite hoch
        sta EDI
@pu:    jsr ed_up
        dec EDI
        bne @pu
        jmp @mv
@a3:    cmp #$0A
        bne @rr
        lda EDROWS              ; Seite runter
        sta EDI
@pd:    jsr ed_down
        dec EDI
        bne @pd
        jmp @mv
@rr:    rtl
@na:    lda EDK                 ; Umlaut (Mac Roman, oberes Bit)
        cmp #$80
        bcc @na2
        jsr ed_isum
        bcs @umi
        rtl
@umi:   lda EDK
        jmp @ins
@na2:   lda KEYC
        cmp #$08
        bne @k1
        lda EVREC+14
        and #$0800              ; Wahl: Wort
        beq @l1
        jsr ed_wleft
        jmp @mvh
@l1:    lda EDCUR
        bne *+5
        jmp @mvh
        dec EDCUR
        jmp @mvh
@k1:    cmp #$15
        bne @k2
        lda EVREC+14
        and #$0800
        beq @r1
        jsr ed_wright
        jmp @mvh
@r1:    lda EDCUR
        cmp EDLEN
        bcc *+5
        jmp @mvh
        inc EDCUR
        jmp @mvh
@k2:    cmp #$0B
        bne @k3
        jsr ed_up
        jmp @mv
@k3:    cmp #$0A
        bne @k4
        jsr ed_down
        jmp @mv
@k4:    cmp #$7F
        beq @ro
        cmp #$04
        bne @k6
@ro:    ldx EDRO
        beq @k4b
        lda #ed_tro
        ldx #ed_tro_en
        jsr ed_smsg
        rtl
@k4b:   cmp #$7F
        bne @k5
        lda EDCUR               ; Delete: links loeschen
        beq @nop
        dec EDCUR
        lda #$FFFF
        jsr ed_pre
        jsr ed_del
        jmp @ed
@k5:    cmp #$04                ; Strg-D: rechts loeschen
        bne @k6
        lda EDCUR
        cmp EDLEN
        bcs @nop
        lda #$FFFF
        jsr ed_pre
        jsr ed_del
        jmp @ed
@k6:    cmp #$0D
        beq @ins
        lda EVREC+14
        and #$1000              ; Strg
        bne @nop
        lda EDK
        cmp #' '
        bcc @nop
        cmp #$7F
        bcs @nop
@ins:   pha
        lda #1
        jsr ed_pre
        pla
        cmp #13
        bne @in2
        lda #$FFFF              ; neue Zeile: alles darunter
        sta EDNL
        lda #13
@in2:   jsr ed_ins
        bcc @ed
@nop:   rtl
@mvh:   jsr ed_goal
@mv:    jsr ed_show
        jsr ed_kwin
        lda EDTOP
        cmp EDT
        beq @mv1
        jsr ed_scroll           ; um eine Zeile: Bild verschieben
        bcs @all
        jsr ed_rowof
        jsr ed_one
        jsr ed_old
        jsr ed_stat
        rtl
@mv1:
        jsr ed_rowof            ; nur alte und neue Cursorzeile
        jsr ed_one
        jsr ed_old
        jsr ed_stat
        rtl
@ed:    lda #1
        sta EDMOD
        jsr ed_goal
        jsr ed_show
        jsr ed_kwin
        lda EDTOP
        cmp EDT
        bne @all
        lda EDOLD               ; ab der Zeile vor der Aenderung neu
        cmp EDCUR
        bcc @o1
        lda EDCUR
@o1:    jsr ed_rowa
        ldx EDDL                ; Loeschen: Wort kann in die Zeile davor rutschen
        bpl @o2
        dec a
        bpl @o2
        lda #0
@o2:    sta EDR0
        lda EDNL                ; Zeilen der naechsten Textzeile unveraendert?
        cmp #$FFFF
        beq @o3
        clc
        adc EDDL
        jsr ed_rowa
        cmp EDRNB
        bne @o3
        sta EDR1                ; dann nur bis dahin
@o3:    jsr ed_rows
        lda #$FFFF
        sta EDR1
        jsr ed_stat
        rtl
@all:   stz EDR0
        jsr ed_rows
        jsr ed_stat
        rtl

; vor einer Aenderung an EDCUR (A = +1 einfuegen, -1 loeschen): Anfang der
; naechsten Textzeile (EDNL, $FFFF = keine/Zeilenende betroffen) und ihre Zeile
ed_pre: sta EDDL
        lda #$FFFF
        sta EDNL
        ldy EDCUR
@l:     cpy EDLEN
        bcs @r
        lda [PSRC],y
        and #$00FF
        cmp #13
        beq @cr
        iny
        bra @l
@cr:    cpy EDCUR               ; CR selbst wird geloescht: alles
        beq @r
        iny
        tya
        sta EDNL
        jsr ed_rowa
        sta EDRNB
@r:     rts

; erste Zeile hat sich um genau eine Bildschirmzeile verschoben (EDT -> EDTOP)?
; Dann den Textbereich auf dem Schirm verschieben und die neue Zeile zeichnen.
; C=1: geht nicht (anders verschoben, Fenster nicht ganz sichtbar)
ed_scroll:
        lda OFFON
        beq *+5
        jmp @no
        lda EDT
        jsr ed_vl
        lda EDN
        cmp EDTOP
        bne @up
        stz EDSD                ; runter: Inhalt nach oben
        bra @go
@up:    lda EDTOP
        jsr ed_vl
        lda EDN
        cmp EDT
        bne @no
        lda #1
        sta EDSD
@go:    lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        clc
        adc #ED_X0-1-WX
        bmi @no
        sta EDSX
        lda EDCOLS
        ldx #ED_CW
        CALL gmul
        clc
        adc #2
        clc
        adc EDSX
        cmp #320
        bcs @no
        inc a
        lsr
        sta EDBL
        lda EDSX
        lsr
        sta EDSX
        lda EDBL
        sec
        sbc EDSX
        sta EDBL
        ldy #R_Y
        lda (PB),y
        clc
        adc #ED_Y0-WY
        bmi @no
        sta EDSY
        lda EDROWS
        ldx #ED_LH
        CALL gmul
        sta EDSH
        clc
        adc EDSY
        cmp #188
        bcc @ok
@no:    sec
        rts
@ok:    lda EDSD
        bne @cu
        lda EDSY                ; runter: Zeile y+LH -> y, von oben
        ldx #160
        CALL gmul
        clc
        adc #$2000
        adc EDSX
        sta EDSA                ; Ziel
        clc
        adc #ED_LH*160
        sta EDSB                ; Quelle
        lda EDSH
        sec
        sbc #ED_LH
        sta EDSC
        phb
@d:     lda EDBL
        dec a
        ldx EDSB
        ldy EDSA
        .byte $54,$E1,$E1       ; mvn $E1,$E1
        jsr ed_dbm
        lda EDSA
        clc
        adc #160
        sta EDSA
        lda EDSB
        clc
        adc #160
        sta EDSB
        dec EDSC
        bne @d
        plb
        lda EDROWS
        dec a
        jsr ed_one
        clc
        rts
@cu:    lda EDSY                ; hoch: Zeile y-LH -> y, von unten
        clc
        adc EDSH
        dec a
        ldx #160
        CALL gmul
        clc
        adc #$2000
        adc EDSX
        sta EDSA
        sec
        sbc #ED_LH*160
        sta EDSB
        lda EDSH
        sec
        sbc #ED_LH
        sta EDSC
        phb
@u:     lda EDBL
        dec a
        ldx EDSB
        ldy EDSA
        .byte $54,$E1,$E1       ; mvn $E1,$E1
        jsr ed_dbm
        lda EDSA
        sec
        sbc #160
        sta EDSA
        lda EDSB
        sec
        sbc #160
        sta EDSB
        dec EDSC
        bne @u
        plb
        lda #0
        jsr ed_one
        clc
        rts
; Datenbank wieder Hauptbank (nach MVN)
ed_dbm: sep #$20
        lda MYBANK
        pha
        plb
        rep #$20
        rts

; Bildschirmzeile des Cursors (EDCUR) -> A (oder EDROWS)
ed_rowof:
        lda EDCUR
ed_rowa:
        sta EDQ
        lda EDTOP
        stz EDI
@l:     sta EDS
        jsr ed_vl
        lda EDN
        cmp #$FFFF
        beq @y
        lda EDQ
        cmp EDN
        bcc @y
        inc EDI
        lda EDI
        cmp EDROWS
        bcs @y
        lda EDN
        bra @l
@y:     lda EDI
        rts

; nur Zeile A neu zeichnen (sofern sichtbar)
ed_one: cmp EDROWS
        bcs @r
        sta EDR0
        inc a
        sta EDR1
        jsr ed_rows
        lda #$FFFF
        sta EDR1
@r:     rts
; alte Cursorzeile (Cursor steht wieder auf dem neuen Platz)
ed_old: pha
        lda EDCUR
        pha
        lda EDOLD
        sta EDCUR
        jsr ed_rowof
        sta EDI
        pla
        sta EDCUR
        pla
        lda EDI
        jmp ed_one

; Zeichen A an der Cursorstelle einfuegen. C=1 kein Platz
ed_ins: sta EDQ
        lda EDRO
        bne @full
        lda EDLEN
        cmp #ED_MAX
        bcs @full
        ldy EDLEN               ; nach hinten schieben
@m:     cpy EDCUR
        beq @put
        dey
        lda [PSRC],y
        iny
        sep #$20
        sta [PSRC],y
        rep #$20
        dey
        bra @m
@put:   lda EDQ
        sep #$20
        sta [PSRC],y
        rep #$20
        inc EDLEN
        inc EDCUR
        clc
        rts
@full:  lda #ed_tfull
        ldx #ed_tfull_en
        jsr ed_smsg
        sec
        rts

; Zeichen an der Cursorstelle entfernen
ed_del: lda EDRO
        bne @r
        ldy EDCUR
@m:     iny
        cpy EDLEN
        bcs @e
        lda [PSRC],y
        dey
        sep #$20
        sta [PSRC],y
        rep #$20
        iny
        bra @m
@e:     dec EDLEN
@r:     rts

; Umlaut der Tastatur (Mac Roman)? C=1 ja
ed_isum:
        ldx #0
@l:     cpx #14
        bcs @n
        TBL ed_um
        and #$00FF
        cmp EDK
        beq @y
        inx
        inx
        bra @l
@y:     sec
        rts
@n:     clc
        rts

ed_up:  lda EDCUR
        jsr ed_vof
        lda EDVS
        beq @r
        dec a
        jsr ed_vof
        lda EDVS
        jsr ed_col
@r:     rts
ed_down:
        lda EDCUR
        jsr ed_vof
        lda EDN
        cmp #$FFFF
        beq @r
        jsr ed_col
@r:     rts
; Wort links/rechts
ed_wleft:
        ldy EDCUR
@s:     cpy #0                  ; Leerzeichen davor ueberspringen
        beq @e
        dey
        lda [PSRC],y
        and #$00FF
        cmp #'!'
        bcc @s
@w:     cpy #0
        beq @e
        dey
        lda [PSRC],y
        and #$00FF
        cmp #'!'
        bcs @w
        iny
@e:     sty EDCUR
        rts
ed_wright:
        ldy EDCUR
@w:     cpy EDLEN
        bcs @e
        lda [PSRC],y
        and #$00FF
        cmp #'!'
        bcc @s
        iny
        bra @w
@s:     cpy EDLEN
        bcs @e
        lda [PSRC],y
        and #$00FF
        cmp #'!'
        bcs @e
        iny
        bra @s
@e:     sty EDCUR
        rts

; Esc: bei Aenderungen erst nachfragen
ed_esc: lda EDMOD
        beq @cl
        lda EDESC
        bne @cl
        inc EDESC
        lda #ed_tesc
        ldx #ed_tesc_en
        jsr ed_smsg
        rtl
@cl:    jsr ed_free
        jsr ed_clear
        CALL close_win
        rtl

; Tasten waehrend der Eingabe des Dateinamens
ed_pkey:
        lda KEYC
        cmp #$1B
        bne @k1
        stz EDPR
        bra @st
@k1:    cmp #$0D
        bne @k2
        stz EDPR
        lda EDPATH
        beq @st
        stz EDNEW
        jsr ed_save
        CALL redraw_win
        rtl
@k2:    cmp #$7F
        bne @k3
        lda EDPATH
        beq @st
        dec EDPATH
        bra @st
@k3:    cmp #' '
        bcc @st
        cmp #$7F
        bcs @st
        ldx EDPATH
        cpx #63
        bcs @st
        sep #$20
        sta EDPATH+2,x
        rep #$20
        inc EDPATH
@st:    jsr ed_kwin
        jsr ed_stat
        rtl

; ---------- Texte ----------
ed_tdef:     .byte "*:TEXT"
ed_ttl:      .byte 9," - Editor"
ed_ttl_en:   .byte 10," - Notepad"
ed_tun:      .byte 9,"Unbenannt"
ed_tun_en:   .byte 8,"Untitled"
ed_tln:      .byte 6,"Zeile "
ed_tln_en:   .byte 5,"Line "
ed_tcol:     .byte 9,", Spalte "
ed_tcol_en:  .byte 6,", Col "
ed_tmod:     .byte 12,"  (ge",$80,"ndert)"
ed_tmod_en:  .byte 11,"  (changed)"
ed_thelp:    .byte 30,"   Apfel-S Speichern, Esc Ende"
ed_thelp_en: .byte 26,"   Apple-S save, Esc close"
ed_tsas:     .byte 15,"Speichern als: "
ed_tsas_en:  .byte 9,"Save as: "
ed_tsaved:   .byte 12,"Gespeichert."
ed_tsaved_en: .byte 6,"Saved."
ed_terr:     .byte 38,"Speichern ging nicht (Pfad, gesperrt?)"
ed_terr_en:  .byte 30,"Could not save (path, locked?)"
ed_tnold:    .byte 29,"Datei lie",$86,"t sich nicht lesen."
ed_tnold_en: .byte 24,"Could not read the file."
ed_tbig:     .byte 37,"Datei zu gro",$86,": nur Anfang, nur lesen."
ed_tbig_en:  .byte 38,"File too large: start only, read only."
ed_tro:      .byte 30,"Nur lesen (Datei war zu gro",$86,")."
ed_tro_en:   .byte 31,"Read only (file was too large)."
ed_tfull:    .byte 30,"Text ist voll (32000 Zeichen)."
ed_tfull_en: .byte 27,"Text is full (32000 chars)."
ed_tbusy:    .byte 40,"Erst speichern (Apfel-S) oder schlie",$86,"en."
ed_tbusy_en: .byte 30,"Save (Apple-S) or close first."
ed_tesc:     .byte 44,"Nicht gespeichert! Esc verwirft, Apfel-S sp."
ed_tesc_en:  .byte 39,"Not saved! Esc discards, Apple-S saves."
