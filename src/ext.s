        .segment "XCODE3"
FARMODE .set 3
xf_act_dup:
        jsr act_dup
        rtl
xf_act_find:
        jsr act_find
        rtl
xf_act_findnext:
        jsr act_findnext
        rtl
xf_act_open:
        jsr act_open
        rtl
xf_act_print:
        jsr act_print
        rtl
xf_ctx_clear:
        jsr ctx_clear
        rtl
xf_ctx_menu:
        jsr ctx_menu
        rtl
xf_find_start:
        jsr find_start
        rtl
xf_props_click:
        jsr props_click
        rtl
xf_props_lock_draw:
        jsr props_lock_draw
        rtl
xf_win_restore:
        jsr win_restore
        rtl
xf_win_store:
        jsr win_store
        rtl
; ================================================================
; GS/OS95: weitere Finder-Funktionen
; Sperren/Entsperren, Duplizieren, Drucken, Suchen/Weitersuchen
; ================================================================

; ---------- Eigenschaften: Schreibschutz (srcpath, gfi_pb aktuell) ----------
LKX = WX+40
LKY = WY+92
props_lock_draw:
        lda MODE
        bne @go
        rts                     ; Laufwerke: kein Schreibschutz hier
@go:
        SUNKEN LKX,LKY,9,9
        FILL LKX+1,LKY+1,7,7,WHITE
        lda gfi_pb+6            ; Zugriff: Bit 1 = Schreiben erlaubt
        and #$0002
        bne @un
        TEXT LKX+2,LKY,t_x,BLACK
@un:    TEXT LKX+14,LKY,t_lock,BLACK
@r:     rts

; Klick im Eigenschaften-Fenster: C=1 behandelt
props_click:
        JSLF xf_props_type_click
        bcc @lk
        rts
@lk:    lda MODE
        beq @no
        HIT LKX,LKY,100,10
        bcs @t
@no:    clc
        rts
@t:     lda gfi_pb+6
        and #$0002
        beq @unlock
        lda gfi_pb+6            ; sperren: Schreiben/Umbenennen/Loeschen weg
        and #$FF3D
        bra @set
@unlock:
        lda gfi_pb+6
        ora #$00C2
@set:   sta gfi_pb+6
        lda #3                  ; SetFileInfo: Pfad + Zugriff
        sta gfi_pb
        lda #$2005
        ldx #gfi_pb
        ldy #srcpath
        CALL gsosp
        php
        pha
        lda #12
        sta gfi_pb
        pla
        plp
        bcc @ok
        CALL win_pop
        CALL fail
        rts
@ok:    lda #$2006              ; neu lesen
        ldx #gfi_pb
        ldy #srcpath
        CALL gsosp
        CALL redraw_all
        sec
        rts

; ---------- Duplizieren: "KOPIE.Name" im selben Ordner ----------
act_dup:
        lda MODE
        bne @g
        lda #5
        CALL show_msg
        rts
@g:     lda SEL2
        bpl @h
        CALL redraw_all
        rts
@h:     sta IDX
        CALL entry_path          ; srcpath
        lda IDX
        CALL entep
        ldx #pathbuf
        ldy #dstpath
        CALL pcopy
        ; ':' + "KOPIE." + Name, hoechstens 15 Zeichen
        ldx dstpath
        sep #$20
        lda #':'
        sta dstpath+2,x
        inx
        ldy #0
@k:     lda kopie,y
        sta dstpath+2,x
        inx
        iny
        cpy #6
        bcc @k
        rep #$20
        ldy #2
        lda [EP],y
        and #$00FF
        sta DUPN
        cmp #10
        bcc @nl
        lda #9                  ; 6 + 9 = 15
        sta DUPN
@nl:    ldy #3
        stz DUPI
@c:     lda DUPI
        cmp DUPN
        bcs @e
        lda [EP],y
        sep #$20
        sta dstpath+2,x
        rep #$20
        inx
        iny
        inc DUPI
        bra @c
@e:     stx dstpath
        CALL show_busy
        lda [EP]
        sta XFTYPE
        stz XCUT
        CALL xfer
        bcc @ok
        CALL fail
        rts
@ok:    CALL reload_view
        CALL redraw_all
        rts
        .pushseg
        .segment "RODATA"
kopie:  .byte "KOPIE."

; ---------- Drucken: Dokument an sein Programm (Nachricht #1, Drucken) ----------
        .popseg
act_print:
        lda MODE
        beq @no
        lda SEL2
        bmi @no
        CALL entep
        lda #1
        sta MSGPRT
        JSLF xf_open_assoc
        stz MSGPRT
        bcs @r
@no:    lda #14                 ; kein Programm zugeordnet/gefunden
        CALL show_msg
        rts
@r:     rts

; ---------- Suchen ----------
act_find:
        lda #1
        sta editsel
        stz editbuf
        lda #3
        sta TXTMODE
        lda #11
        CALL win_open
        CALL redraw_all
        rts

; aus edit_ok (TXTMODE 3): Suchbegriff in editbuf
find_start:
        CALL win_pop
        ; Suchbegriff merken (Grossbuchstaben)
        lda editbuf
        sta FNDL
        ldx #0
@c:     cpx FNDL
        bcs @d
        lda editbuf+2,x
        CALL upc8
        sep #$20
        sta fndtxt,x
        rep #$20
        inx
        bra @c
@d:     ; Startordner: aktueller Ordner, sonst Startlaufwerk
        lda MODE
        beq @boot
        ldx #pathbuf
        ldy #fndroot
        CALL pcopy
        bra @go
@boot:  ldx #bootvol
        ldy #fndroot
        CALL pcopy
@go:    stz FNDSKIP
        jmp find_run

; Weitersuchen: einen Treffer weiter
act_findnext:
        lda FNDL
        bne @g
        jmp act_find
@g:     inc FNDSKIP
        ; fall through

find_run:
        CALL show_busy
        lda FNDSKIP
        sta FNDCNT
        ldx #fndroot
        ldy #workpath
        CALL pcopy
        stz FNDD
        lda #1
        sta fndidx
@loop:  ; Eintrag fndidx[FNDD] in workpath lesen
        lda #workpath
        sta ico_pb+4
        GSOS $2010,ico_pb
        bcc @op
        jmp @up
@op:    lda ico_pb+2
        sta icd_pb+2
        sta icc_pb+2
        stz icd_pb+6            ; absolut
        lda FNDD
        asl
        tax
        lda fndidx,x
        sta icd_pb+8
        GSOS $201C,icd_pb
        php
        GSOS $2014,icc_pb
        plp
        bcc @rd
        jmp @up
@rd:    ; Name vergleichen (enthaelt Suchbegriff?)
        jsr find_match
        bcc @nomatch
        lda FNDCNT
        bne @sk
        jmp @found
@sk:    dec FNDCNT
@nomatch:
        lda icd_pb+16
        and #$00FF
        cmp #$000F
        bne @next
        lda FNDD
        cmp #15
        bcs @next
        ; in den Ordner hinab
        ldx workpath
        sep #$20
        lda #':'
        sta workpath+2,x
        rep #$20
        inx
        ldy #0
@an:    cpy icname+2
        bcs @ae
        lda icname+4,y
        sep #$20
        sta workpath+2,x
        rep #$20
        inx
        iny
        bra @an
@ae:    stx workpath
        inc FNDD
        lda FNDD
        asl
        tax
        lda #1
        sta fndidx,x
        jmp @loop
@next:  lda FNDD
        asl
        tax
        inc fndidx,x
        jmp @loop
@up:    lda FNDD
        bne @u
        ; nichts (mehr) gefunden
        lda FNDSKIP
        beq @none
        dec FNDSKIP
@none:  lda #15
        CALL show_msg
        rts
@u:     ; letzten Pfadteil entfernen
        ldx workpath
@s:     dex
        lda workpath+2,x
        and #$00FF
        cmp #':'
        bne @s
        stx workpath
        dec FNDD
        jmp @next
@found: ; Ordner workpath im obersten Explorer zeigen, Treffer markieren
        ldx #workpath
        ldy #pathbuf
        CALL pcopy
        lda #1
        sta MODE
        stz ISBIN
        CALL load_dir
        ; Eintrag mit dem gefundenen Namen suchen
        stz FNDI
@f:     lda FNDI
        cmp COUNT
        bcs @shw
        CALL entep
        ldy #2
        lda [EP],y
        and #$00FF
        cmp icname+2
        bne @fn
        ldy #3
        ldx #0
@fc:    cpx icname+2
        bcs @hit
        lda [EP],y
        eor icname+4,x
        and #$00FF
        bne @fn
        iny
        inx
        bra @fc
@fn:    inc FNDI
        bra @f
@hit:   lda FNDI
        sta SEL2
        CALL win_geom
        lda SEL2
        CALL ensure_vis
@shw:   CALL redraw_all
        rts

; enthaelt icname den Suchbegriff fndtxt/FNDL? C=1 ja
find_match:
        lda icname+2
        sec
        sbc FNDL
        bmi @no
        sta FNDM                ; letzte Startposition
        stz FNDP
@p:     lda FNDP
        cmp FNDM
        beq @t
        bcs @no
@t:     ldx #0
        lda FNDP
        tay
@c:     cpx FNDL
        bcs @yes
        lda icname+4,y
        CALL upc8
        sep #$20
        cmp fndtxt,x
        rep #$20
        bne @nx
        inx
        iny
        bra @c
@nx:    inc FNDP
        bra @p
@yes:   sec
        rts
@no:    clc
        rts

; ---------- offene Ordnerfenster merken (*:System:GSOS95.WIN) ----------
; Format: Anzahl (Wort), je Fenster: Modus (Wort), Pfad (GS/OS-String)
win_store:
        CALL win_save
        stz WSN
        ldx #2                  ; Schreibposition in winbuf
        stx WSP
        stz WSK
@l:     lda WSK
        cmp NWIN
        bcc *+5
        jmp @w
        CALL recaddr
        sta PB
        ldy #R_KIND
        lda (PB),y
        cmp #1
        bne @n
        lda WSN
        cmp #NSLOT
        bcs @n
        lda WSP
        cmp #800-300
        bcs @n
        ldx WSP
        ldy #R_MODE
        lda (PB),y
        sta winbuf,x
        inx
        inx
        ldy #R_X                ; Lage, Groesse, maximiert, alte Lage
@g:     lda (PB),y
        sta winbuf,x
        inx
        inx
        iny
        iny
        cpy #R_Y+2
        bcc @g
        ldy #R_W
@g2:    lda (PB),y
        sta winbuf,x
        inx
        inx
        iny
        iny
        cpy #R_MAXI+2
        bcc @g2
        ldy #R_PATH
        lda (PB),y              ; Laenge
        sta winbuf,x
        sta WSL
        inx
        inx
        ldy #R_PATH+2
@c:     lda WSL
        beq @cd
        lda (PB),y
        sep #$20
        sta winbuf,x
        rep #$20
        inx
        iny
        dec WSL
        bra @c
@cd:    stx WSP
        inc WSN
@n:     inc WSK
        jmp @l
@w:     lda WSN
        ora #$8000              ; Format 2: mit Lage und Groesse
        sta winbuf
        GSOS $2001,wcr_pb       ; Create (Fehler egal)
        GSOS $2010,wop_pb
        bcs @r
        lda wop_pb+2
        sta wrw_pb+2
        sta wcl_pb+2
        lda WSP
        sta wrw_pb+8
        GSOS $2013,wrw_pb       ; Write
        lda WSP
        sta wef_pb+6            ; Position (Basis 0 = absolut)
        lda wop_pb+2
        sta wef_pb+2
        GSOS $2018,wef_pb       ; SetEOF
        GSOS $2014,wcl_pb
@r:     rts

win_restore:
        lda #1
        sta wop_pb+8            ; nur lesen
        GSOS $2010,wop_pb
        php
        lda #3
        sta wop_pb+8
        plp
        bcc @op
        rts
@op:    lda wop_pb+2
        sta wrw_pb+2
        sta wcl_pb+2
        lda #800
        sta wrw_pb+8
        GSOS $2012,wrw_pb       ; Read
        php
        GSOS $2014,wcl_pb
        plp
        bcc @rd
        rts
@rd:    lda winbuf
        and #$8000
        sta WSV
        lda winbuf
        and #$7FFF
        cmp #NSLOT+1
        bcc @nk
        rts
@nk:
        sta WSN
        ldx #2
        stx WSP
@l:     lda WSN
        bne @l1
        rts
@l1:    CALL new_explorer
        bcc @l2
        rts
@l2:
        ldx WSP
        lda winbuf,x            ; Modus
        sta WSK
        inx
        inx
        lda WSV
        beq @old
        phx
        lda NWIN                ; Lage und Groesse uebernehmen
        dec a
        CALL recaddr
        sta PB
        plx
        ldy #R_X
@g:     lda winbuf,x
        sta (PB),y
        inx
        inx
        iny
        iny
        cpy #R_Y+2
        bcc @g
        ldy #R_W
@g2:    lda winbuf,x
        sta (PB),y
        inx
        inx
        iny
        iny
        cpy #R_MAXI+2
        bcc @g2
        phx
        lda NWIN
        dec a
        CALL win_load
        plx
@old:   lda winbuf,x            ; Pfadlaenge
        cmp #255
        bcc @pl
        rts
@pl:
        sta pathbuf
        sta WSL
        inx
        inx
        ldy #0
@c:     cpy WSL
        bcs @cd
        lda winbuf,x
        sep #$20
        sta pathbuf+2,y
        rep #$20
        inx
        iny
        bra @c
@cd:    stx WSP
        stz ISBIN
        stz PAGE
        lda #$FFFF
        sta SEL2
        lda WSK
        beq @vol
        JSLF xf_vol_known           ; Laufwerk nicht da: nicht danach suchen lassen
        bcc @unk                ; (GS/OS fragt sonst alle Laufwerke ab)
        lda #1
        sta MODE
        CALL load_dir
        bcc @ok
@unk:   stz pathbuf
@vol:   stz MODE
        CALL load_volumes
@ok:    CALL win_save
        dec WSN
        jmp @l
@r:     rts

; ---------- Kontextmenue (Wahltaste-Klick) ----------
act_open:
        lda SEL2
        bmi @r
        cmp COUNT
        bcs @r
        CALL open_entry
        rts
@r:     rts

ctx_menu:
        CALL cell_at
        cmp #$FFFF
        beq @bg
        sta TMP2
        JSLF xf_is_sel
        bcs @sel
        JSLF xf_msel_clear
        lda TMP2
        sta SEL2
@sel:   lda #5
        bra @set
@bg:    lda #6
@set:   sta DROP
        lda #$FFFF
        sta DHOVER
        ; Lage: an der Maus, im Bildschirm gehalten
        lda MX
        cmp #320-DDW-2
        bcc @x
        lda #320-DDW-2
@x:     sec
        sbc DXO
        sta CMX
        CALL drop_count
        asl
        asl
        sta TMP
        asl
        clc
        adc TMP
        clc
        adc #4
        sta TMP                 ; Hoehe
        lda #184
        sec
        sbc TMP
        cmp MY
        bcc @y
        lda MY
@y:     sec
        sbc DYO
        sta CMY
        CALL redraw_win
        CALL cur_hide
        CALL draw_drop
        CALL cur_show
        rts

; Flaeche des Kontextmenues neu zeichnen (DROP noch gesetzt)
ctx_clear:
        lda CMX
        clc
        adc DXO
        sta UL
        clc
        adc #DDW
        sta UR
        lda CMY
        clc
        adc DYO
        sta UT
        CALL drop_count
        asl
        asl
        sta TMP
        asl
        clc
        adc TMP
        clc
        adc #4
        clc
        adc UT
        sta UB
        stz DROP
        CALL rect_set
        CALL redraw_rect
        rts

FARMODE .set 0
        .segment "CODE"
