        .segment "XCODE3"
FARMODE .set 3
xf_act_restore:
        jsr act_restore
        rtl
xf_bin_clear:
        jsr bin_clear
        rtl
xf_bin_note:
        jsr bin_note
        rtl
xf_doc_note:
        jsr doc_note
        rtl
xf_pl_addr:
        jsr pl_addr
        rtl
xf_pl_append:
        jsr pl_append
        rtl
xf_pl_del:
        jsr pl_del
        rtl
xf_pl_find:
        jsr pl_find
        rtl
xf_pl_loadall:
        jsr pl_loadall
        rtl
xf_pl_save:
        jsr pl_save
        rtl
xf_pl_sel:
        jsr pl_sel
        rtl
; ================================================================
; GS/OS95: Pfadlisten (0 = Desktop-Verknuepfungen, 1 = Startmenue-
; Programme, 2 = zuletzt geoeffnete Dokumente)
; Aufbau im Speicher und in der Datei: Anzahl (Wort), dann Eintraege
; zu LNKSZ Bytes: +0 Typ, +2 Zusatztyp, +4 Pfad (Laenge + Zeichen)
; ================================================================
PLQ = $78                       ; Direct-Page-Zeiger (2 Bytes)
PRGMAX = 12                      ; ab 15 Zeilen blaettert das Untermenue
DOCMAX = 8
BINMAX = 16                     ; gemerkte Herkunftspfade im Papierkorb

        .pushseg
        .segment "RODATA"
pl_cntp: .word LNKN, PRGLN, DOCLN, BINLN
pl_maxt: .word LNKMAX, PRGMAX, DOCMAX, BINMAX
pl_file: .word lpath, ppath, dpath, bpath

; Liste X (0/1/2) waehlen
        .popseg
pl_sel: txa
        asl
        sta PLSEL
        rts

; Anzahl -> A
pl_count:
        ldx PLSEL
        lda pl_cntp,x
        sta PLQ
        lda (PLQ)
        rts

; Adresse von Eintrag A -> A
pl_addr:
        ldx #LNKSZ
        CALL gmul
        ldx PLSEL
        clc
        adc pl_cntp,x
        inc a
        inc a
        rts

; Eintrag mit Pfad srcpath suchen -> A, C=0 gefunden
pl_find:
        stz PLI
@l:     jsr pl_count
        cmp PLI
        beq @no
        bcc @no
        lda PLI
        jsr pl_addr
        clc
        adc #4
        tax
        ldy #srcpath
        CALL pequal
        bcs @y
        inc PLI
        bra @l
@y:     lda PLI
        clc
        rts
@no:    sec
        rts

; Eintrag A loeschen
pl_del: sta PLI
@l:     lda PLI
        inc a
        sta PLJ
        jsr pl_count
        cmp PLJ
        beq @e
        bcc @e
        lda PLJ
        jsr pl_addr
        pha
        lda PLI
        jsr pl_addr
        tay
        plx
        jsr pl_mv
        inc PLI
        bra @l
@e:     jsr pl_count
        beq @r
        dec a
        sta (PLQ)
@r:     rts

; LNKSZ Bytes von X nach Y (Programmbank)
pl_mv:  lda MYBANK
        xba
        ora MYBANK
        sta ecmv+1
        lda #LNKSZ-1
        CALL ec_n
        rts

; srcpath mit PLTYP/PLAUX vorn einfuegen (doppelte vorher entfernen,
; bei voller Liste faellt der letzte weg)
pl_front:
        jsr pl_find
        bcs @nf
        jsr pl_del
@nf:    jsr pl_count
        ldx PLSEL
        cmp pl_maxt,x
        bcc @room
        dec a
        sta (PLQ)
@room:  jsr pl_count
        sta PLI                 ; von hinten nach vorn um eins verschieben
@sh:    lda PLI
        beq @put
        dec a
        jsr pl_addr
        pha
        lda PLI
        jsr pl_addr
        tay
        plx
        jsr pl_mv
        dec PLI
        bra @sh
@put:   lda #0
        jsr pl_store
        jsr pl_count
        inc a
        sta (PLQ)
        rts

; srcpath hinten anfuegen. C=1 wenn schon vorhanden oder voll
pl_append:
        jsr pl_find
        bcc @ex
        jsr pl_count
        ldx PLSEL
        cmp pl_maxt,x
        bcs @ex
        jsr pl_store
        jsr pl_count
        inc a
        sta (PLQ)
        clc
        rts
@ex:    sec
        rts

; Eintrag A mit PLTYP, PLAUX, srcpath beschreiben
pl_store:
        jsr pl_addr
        sta PLQ
        lda PLTYP
        sta (PLQ)
        ldy #2
        lda PLAUX
        sta (PLQ),y
        lda PLQ
        clc
        adc #4
        tay
        ldx #srcpath
        CALL pcopy
        rts

; ---------- Laden / Speichern ----------
pl_load:
        ldx PLSEL
        lda pl_cntp,x
        sta PLQ
        lda #0
        sta (PLQ)
        lda pl_file,x
        sta wop_pb+4
        lda #1
        sta wop_pb+8
        GSOS $2010,wop_pb
        php
        lda #3
        sta wop_pb+8
        lda #wpath
        sta wop_pb+4
        plp
        bcs @r
        lda wop_pb+2
        sta lrw_pb+2
        sta wcl_pb+2
        ldx PLSEL
        lda pl_cntp,x
        sta lrw_pb+4
        lda pl_maxt,x
        ldx #LNKSZ
        CALL gmul
        inc a
        inc a
        sta lrw_pb+8
        GSOS $2012,lrw_pb
        GSOS $2014,wcl_pb
        jsr pl_count
        ldx PLSEL
        cmp pl_maxt,x
        bcc @r
        beq @r
        lda #0
        sta (PLQ)
@r:     rts

pl_save:
        ldx PLSEL
        lda pl_file,x
        sta wcr_pb+2
        GSOS $2001,wcr_pb       ; Create (Fehler egal)
        lda #wpath
        sta wcr_pb+2
        ldx PLSEL
        lda pl_file,x
        sta wop_pb+4
        GSOS $2010,wop_pb
        php
        lda #wpath
        sta wop_pb+4
        plp
        bcs @r
        lda wop_pb+2
        sta lrw_pb+2
        sta wcl_pb+2
        sta wef_pb+2
        ldx PLSEL
        lda pl_cntp,x
        sta lrw_pb+4
        jsr pl_count
        ldx #LNKSZ
        CALL gmul
        clc
        adc #2
        sta lrw_pb+8
        sta wef_pb+6
        GSOS $2013,lrw_pb
        GSOS $2018,wef_pb       ; SetEOF
        GSOS $2014,wcl_pb
@r:     rts

; alle Listen laden
pl_loadall:
        ldx #0
@l:     phx
        jsr pl_sel
        jsr pl_load
        plx
        inx
        cpx #4
        bcc @l
        rts

; ---------- Papierkorb: Herkunft merken / Wiederherstellen (Liste 3) ----------
; nach dem Verschieben in den Papierkorb: srcpath = alter Pfad
bin_note:
        lda srcpath
        cmp #65                 ; zu lang fuer die Liste: nicht merken
        bcs @r
        lda PLSEL
        sta BINSV
        ldx #3
        jsr pl_sel
        stz PLTYP
        stz PLAUX
        jsr pl_front
        jsr pl_save
        lda BINSV
        sta PLSEL
@r:     clc
        rts
; Papierkorb geleert
bin_clear:
        lda PLSEL
        sta BINSV
        ldx #3
        jsr pl_sel
        jsr pl_count
        lda #0
        sta (PLQ)
        jsr pl_save
        lda BINSV
        sta PLSEL
        rts
; Listeneintrag mit demselben Namen wie srcpath suchen -> A, C=0 gefunden
bin_find:
        stz PLI
@l:     jsr pl_count
        cmp PLI
        beq @no
        bcc @no
        lda PLI
        jsr pl_addr
        clc
        adc #4
        sta PLQ
        lda srcpath             ; von hinten vergleichen bis zum ':'
        sta TMP
        lda (PLQ)
        sta TMP2
@c:     lda TMP
        beq @nx
        lda TMP2
        beq @nx
        ldx TMP
        lda srcpath+1,x
        and #$00FF
        sta BINC
        ldy TMP2
        iny
        lda (PLQ),y
        and #$00FF
        cmp BINC
        bne @nx
        cmp #':'
        beq @y
        dec TMP
        dec TMP2
        bra @c
@nx:    inc PLI
        bra @l
@y:     lda PLI
        clc
        rts
@no:    sec
        rts
; markierte Eintraege des Papierkorbs an den alten Platz zurueck
act_restore:
        lda ISBIN
        bne @go
        rts
@go:    lda PLSEL
        sta BINSV
        ldx #3
        jsr pl_sel
        JSLF xf_sel_list
        stz PSTI
@l:     lda PSTI
        cmp SLN
        bcs @done
        asl
        tax
        lda sellist,x
        sta IDX
        CALL entry_path          ; srcpath = *:RECYCLED:Name
        jsr bin_find
        bcs @root
        sta BINI
        jsr pl_addr
        clc
        adc #4
        sta chpath_pb+6
        bra @mv
@root:  lda #$FFFF              ; Herkunft unbekannt: Hauptverzeichnis des Startvolumes
        sta BINI
        ldx #bootvol
        ldy #dstpath
        CALL pcopy
        lda IDX
        CALL entep
        ldy #dstpath
        CALL pappend
        lda #dstpath
        sta chpath_pb+6
@mv:    lda #srcpath
        sta chpath_pb+2
        GSOS $2004,chpath_pb
        bcc @ok
        pha
        jsr pl_save
        lda BINSV
        sta PLSEL
        pla
        CALL fail
        rts
@ok:    lda BINI
        bmi @n
        jsr pl_del
@n:     inc PSTI
        bra @l
@done:  jsr pl_save
        lda BINSV
        sta PLSEL
        CALL reload_view
        CALL redraw_all
        rts

; Eintrag IDX des Explorers als Dokument merken
doc_note:
        CALL entry_path
        lda srcpath
        cmp #65
        bcs @r
        lda [EP]
        sta PLTYP
        ldy #E_AUX
        lda [EP],y
        sta PLAUX
        ldx #2
        jsr pl_sel
        jsr pl_front
        jsr pl_save
@r:     lda IDX
        CALL entep
        rts

FARMODE .set 0
        .segment "CODE"
