; ================================================================
; GS/OS95 Finder-Funktionen: Fenstermenues, Ordner, Umbenennen,
; Papierkorb, Kopieren/Ausschneiden/Einfuegen, Eigenschaften
; ================================================================

MBY     = WY+17
DDY     = WY+28
DDW     = 132

; ---------- Pfadhilfen ----------
; Pfad (Class-1-String) von X nach Y kopieren
pcopy:  stx PB
        sty PB2
        lda (PB)
        clc
        adc #2
        tax
        sep #$20
        ldy #0
@c:     lda (PB),y
        sta (PB2),y
        iny
        dex
        bne @c
        rep #$20
        rts

; an Pfad in Y ':' + Name des Eintrags EP anhaengen
pappend:
        sty PB
        sep #$20
        lda (PB)
        tax                     ; X = alte Laenge (8 Bit genuegt)
        rep #$20
        txa
        and #$00FF
        clc
        adc #2
        tay
        sep #$20
        lda #':'
        sta (PB),y
        iny
        phy
        ldy #2
        lda [EP],y
        sta TMP
        stz TMP+1
        ply
        ldx #0
@c:     cpx TMP
        bcs @e
        phy
        txy
        iny
        iny
        iny
        lda [EP],y
        ply
        sta (PB),y
        iny
        inx
        bra @c
@e:     rep #$20
        tya
        sec
        sbc #2
        sta (PB)
        rts

; Pfad in Y um letzte Komponente kuerzen
pstrip: sty PB
        lda (PB)
        tay
@f:     dey
        beq @z
        iny
        iny
        lda (PB),y
        dey
        dey
        and #$00FF
        cmp #':'
        bne @f
@z:     tya
        sta (PB)
        rts

; Pfad des Eintrags IDX -> srcpath
entry_path:
        lda IDX
        jsr entep
        lda MODE
        bne @d
        stz srcpath
        bra @a
@d:     ldx #pathbuf
        ldy #srcpath
        jsr pcopy
@a:     ldy #srcpath
        jmp pappend

; Volume-Name (bis zum 2. ':') von Pfad X mit Pfad Y vergleichen, C=1 wenn gleich
samevol:
        stx PB
        sty PB2
        ldy #3
@l:     tya
        sec
        sbc #2
        cmp (PB)
        bcs @end1               ; Pfad 1 zu Ende
        lda (PB),y
        and #$00FF
        cmp #':'
        beq @end1
        jsr upc
        sta TMP
        tya
        sec
        sbc #2
        cmp (PB2)
        bcs @no                 ; Pfad 2 kuerzer
        lda (PB2),y
        and #$00FF
        jsr upc
        cmp TMP
        bne @no
        iny
        bra @l
@end1:  ; Pfad 2 muss hier auch enden oder ':' haben
        tya
        sec
        sbc #2
        cmp (PB2)
        bcs @yes
        lda (PB2),y
        and #$00FF
        cmp #':'
        beq @yes
@no:    clc
        rts
@yes:   sec
        rts
upc:    cmp #$61
        bcc @r
        cmp #$7B
        bcs @r
        and #$00DF
@r:     rts

; GS/OS-Aufruf mit Pfadzeiger: A = Nummer, X = Parameterblock, Y = Pfad
gsosp:  pha
        tya
        sta a:2,x
        pla
        jmp gsos

; ---------- Fehler / Meldungen ----------
fail:   sta ERRC
        jsr reload_view
        ; bekannte Fehler im Klartext
        lda ERRC
        ldx #11
        cmp #$0047              ; Name existiert bereits
        beq @k
        inx
        cmp #$0048              ; Datentraeger voll
        beq @k
        inx
        cmp #$002B              ; schreibgeschuetzt
        beq @k
        cmp #$004E              ; gesperrt
        beq @k
        lda #4
        jmp show_msg
@k:     txa
        jmp show_msg

reload_view:
        lda MODE
        bne @d
        stz VOLOK
        jmp load_volumes
@d:     jsr load_dir
        bcc @r
        stz COUNT
@r:     rts

; ================= Menueleiste im Fenster =================
draw_menubar:
        lda #WX+7
        sta SX
        lda #MBY+1
        sta SY
        lda #BLACK
        jsr setcolor
        ldx #0
@m:     phx
        lda SX
        sec
        sbc #3
        sta MBXT,x
        lda LANG
        beq @de
        lda mbtab_en,x
        bra @st
@de:    lda mbtab,x
@st:    sta SPR
        jsr blit1
        lda SX
        clc
        adc W
        adc #10
        sta SX
        plx
        inx
        inx
        cpx #8
        bcc @m
        lda SX
        sec
        sbc #3
        sta MBXT+8
        ; Aufwaerts-Knopf rechts
        lda MODE
        bne *+5
        jmp @r
        jsr anc_r
        BUTTON WX+WW-60,MBY,54,10
        TEXTC WX+WW-33,MBY+1,t_up,BLACK
        jsr anc_end
@r:     rts
mbtab:    .word mb_file,mb_edit,mb_view,mb_help
mbtab_en: .word mb_file_en,mb_edit_en,mb_view_en,mb_help_en

; Menueleiste angeklickt? C=1 wenn behandelt
menubar_click:
        jsr lmy
        cmp #MBY
        bcc @no
        cmp #MBY+10
        bcs @no
        ldx #0
@l:     jsr lmx
        cmp MBXT,x
        bcc @n
        cmp MBXT+2,x
        bcs @n
        txa
        lsr
        inc a
        sta DROP
        lda #$FFFF
        sta DHOVER
        jsr draw_drop
        sec
        rts
@n:     inx
        inx
        cpx #8
        bcc @l
        lda MODE
        beq @no
        jsr anc_r
        HIT WX+WW-60,MBY,54,10
        php
        jsr anc_end
        plp
        bcc @no
        jsr go_up
        sec
        rts
@no:    clc
        rts

; Anzahl Eintraege des offenen Menues -> A
drop_count:
        lda DROP
        asl
        tax
        lda dcount-2,x
        rts
dcount: .word 9,3,8,1,8,5

; Eintrag k (A) -> Text (A), Kuerzel in TMP2, Aktion in ACT
drop_item:
        pha
        lda DROP
        dec a
        asl
        tax
        lda dbase,x
        sta TMP
        pla
        asl
        asl
        asl                     ; *8 Bytes pro Eintrag
        clc
        adc TMP
        tax
        lda a:4,x
        sta TMP2
        lda a:6,x
        sta ACT
        lda LANG
        beq @de
        lda a:2,x
        bra @r
@de:    lda a:0,x
@r:     ; Papierkorb: erster Eintrag = leeren
        pha
        lda ISBIN
        beq @nb
        lda ACT
        cmp #2                  ; Umbenennen / Oeffnen -> Wiederherstellen
        beq @rst
        cmp #25
        bne @nrs
@rst:   pla
        lda #28
        sta ACT
        stz TMP2
        LSPR fm_restore
        lda SPR
        rts
@nrs:   cmp #1
        bne @nb
        pla
        lda #12
        sta ACT
        stz TMP2
        LSPR fm_empty
        lda SPR
        rts
@nb:    lda MODE
        bne @nv
        lda ACT
        cmp #3
        bne @nb2
        pla
        lda #14
        sta ACT
        stz TMP2
        LSPR fm_erase
        lda SPR
        rts
@nb2:   lda ACT
        cmp #1
        bne @nv
        pla
        lda #13
        sta ACT
        LSPR fm_eject
        lda #sc_j
        sta TMP2
        lda SPR
        rts
@nv:    pla
        rts
dbase:  .word dm0,dm1,dm2,dm3,dm4,dm5
dm0:    .word fm_new,fm_new_en,sc_n,1, fm_ren,fm_ren_en,sc_e,2
        .word fm_del,fm_del_en,sc_d,3, fm_prop,fm_prop_en,sc_i,4
        .word fm_close,fm_close_en,sc_esc,5
        .word fm_dup,fm_dup_en,0,21, fm_print,fm_print_en,0,22
        .word fm_find,fm_find_en,0,23, fm_findn,fm_findn_en,0,24
dm1:    .word fm_cut,fm_cut_en,sc_x,6, fm_copy,fm_copy_en,sc_c,7
        .word fm_paste,fm_paste_en,sc_v,8
dm2:    .word fm_refr,fm_refr_en,sc_r,9, fm_up,fm_up_en,sc_del,10
        .word fm_large,fm_large_en,0,15, fm_details,fm_details_en,0,16
        .word fm_tree,fm_tree_en,0,29
        .word fm_sname,fm_sname_en,0,17, fm_ssize,fm_ssize_en,0,18
        .word fm_stype,fm_stype_en,0,19
dm3:    .word fm_about,fm_about_en,0,11
; Kontextmenue: Objekt / freie Flaeche
dm4:    .word fm_open,fm_open_en,0,25, fm_cut,fm_cut_en,sc_x,6
        .word fm_copy,fm_copy_en,sc_c,7, fm_del,fm_del_en,sc_d,3
        .word fm_ren,fm_ren_en,sc_e,2, fm_prop,fm_prop_en,sc_i,4
        .word fm_link,fm_link_en,0,26, fm_prgadd,fm_prgadd_en,0,27
dm5:    .word fm_paste,fm_paste_en,sc_v,8, fm_new,fm_new_en,sc_n,1
        .word fm_refr,fm_refr_en,sc_r,9, fm_large,fm_large_en,0,15
        .word fm_details,fm_details_en,0,16

; x-Position des offenen Menues -> A
drop_x: lda DROP
        cmp #5
        bcc @m
        lda CMX
        rts
@m:     dec a
        asl
        tax
        lda MBXT,x
        rts

; Oberkante des offenen Menues -> A
drop_y: lda DROP
        cmp #5
        bcc @m
        lda CMY
        rts
@m:     lda #DDY
        rts

draw_drop:
        jsr drop_x
        sta BX
        sta RX
        jsr drop_y
        sta RY
        lda #DDW
        sta RW
        jsr drop_count
        sta FCOUNT
        asl
        asl
        sta TMP
        asl
        clc
        adc TMP                 ; *12
        clc
        adc #4
        sta RH
        lda #GRAY
        jsr setcolor
        jsr fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        jsr bevel
        stz ITEM
@i:     lda ITEM
        jsr draw_drop_item
        inc ITEM
        lda ITEM
        cmp FCOUNT
        bcc @i
        rts

; A = Eintrag
draw_drop_item:
        sta IY
        asl
        asl
        sta TMP
        asl
        clc
        adc TMP
        sta RY
        jsr drop_y
        clc
        adc #2
        adc RY
        sta RY
        jsr drop_x
        clc
        adc #2
        sta RX
        lda #DDW-4
        sta RW
        lda #12
        sta RH
        lda IY
        cmp DHOVER
        bne @n
        lda #NAVY
        bra @f
@n:     lda #GRAY
@f:     jsr setcolor
        jsr fillrect
        lda RY
        clc
        adc #2
        sta SY
        lda RX
        clc
        adc #9                  ; Platz fuer Punkt/Haken
        sta SX
        lda IY
        jsr drop_item
        sta SPR
        lda IY
        cmp DHOVER
        bne @n2
        lda #WHITE
        bra @f2
@n2:    lda #BLACK
@f2:    jsr setcolor
        jsr blit1
        jsr drop_mark           ; Punkt vor der aktiven Ansicht / Sortierung
        ; Kuerzel rechtsbuendig
        lda IY
        jsr drop_item           ; TMP2 neu (blit1 hat TMP2 benutzt)
        lda TMP2
        beq @r
        jsr sc_lang
        sta SPR
        jsr sprhdr
        and #$00FF
        sta TMP
        jsr drop_x
        clc
        adc #DDW-6
        sec
        sbc TMP
        sta SX
        jsr blit1
@r:     rts

; Ansicht-Menue: kleiner Punkt (Ansicht, Sortierung) bzw. Haken (Ordnerbaum)
drop_mark:
        lda ACT
        cmp #15
        bne @m16
        lda VIEWM
        beq @dot
        rts
@m16:   cmp #16
        bne @m17
        lda VIEWM
        bne @dot
        rts
@m17:   cmp #17
        bcc @m29
        cmp #20
        bcs @m29
        sec
        sbc #17
        cmp SORTM
        beq @dot
        rts
@m29:   cmp #29
        bne @r
        lda TREEOFF
        bne @r
        ; Haken: zwei kleine Striche
        lda RX
        inc a
        inc a
        sta RX
        lda RY
        clc
        adc #6
        sta RY
        lda #1
        sta RW
        lda #2
        sta RH
        jsr fillrect
        inc RX
        inc RY
        jsr fillrect
        inc RX
        dec RY
        dec RY
        dec RY
        lda #4
        sta RH
        jsr fillrect
        inc RX
        dec RY
        dec RY
        jsr fillrect
@r:     rts
@dot:   lda RX
        inc a
        inc a
        sta RX
        lda RY
        clc
        adc #5
        sta RY
        lda #3
        sta RW
        sta RH
        jmp fillrect

; Kuerzel-Sprite (deutsch) A -> sprachabhaengig
sc_lang:
        ldx LANG
        beq @r
        ldx #0
@l:     cmp sctab,x
        beq @f
        inx
        inx
        inx
        inx
        cpx #44
        bcc @l
@r:     rts
@f:     lda sctab+2,x
        rts
sctab:  .word sc_j,sc_j_en,sc_n,sc_n_en,sc_e,sc_e_en,sc_d,sc_d_en,sc_i,sc_i_en,sc_esc,sc_esc_en
        .word sc_x,sc_x_en,sc_c,sc_c_en,sc_v,sc_v_en,sc_r,sc_r_en,sc_del,sc_del_en

; Eintrag unter Maus -> A ($FFFF = keiner)
drop_hit:
        jsr drop_x
        sta RX
        jsr drop_y
        clc
        adc #2
        sta RY
        lda #DDW
        sta RW
        jsr drop_count
        sta FCOUNT
        asl
        asl
        sta TMP
        asl
        clc
        adc TMP
        sta RH
        jsr inrect
        bcc @no
        jsr lmy
        sec
        sbc RY
        ldx #0
@d:     cmp #12
        bcc @g
        sbc #12
        inx
        bra @d
@g:     txa
        rts
@no:    lda #$FFFF
        rts

drop_hover:
        jsr drop_hit
        cmp DHOVER
        beq @r
        ldx DHOVER
        sta DHOVER
        stx IDX
        lda IDX
        bmi @n
        jsr draw_drop_item
@n:     lda DHOVER
        bmi @r
        jsr draw_drop_item
@r:     rts

drop_click:
        jsr drop_hit
        bmi @close
        jsr drop_item           ; braucht DROP (welches Menue) noch!
        lda DROP
        cmp #5
        bcc @nc
        JSLF xf_ctx_clear           ; Kontextmenue wegraeumen
@nc:    stz DROP
        lda ACT
        jmp do_action
@close: lda DROP
        cmp #5
        bcc @c2
        JSLF xf_ctx_clear
        rts
@c2:    stz DROP
        jmp redraw_all

; ================= Aktionen =================
do_action:
        ldx ISBIN               ; im Papierkorb: Umbenennen/Oeffnen = Wiederherstellen
        beq @nbin
        cmp #2
        beq @rs
        cmp #25
        bne @nbin
@rs:    lda #28
@nbin:  sta ACT
        cmp #21
        bcc @a2x
        cmp #30
        bcs @a2x
        asl
        tax
        jmp (exttab-42,x)
@a2x:   lda ACT
        cmp #20
        bne @a20
        lda MODE
        beq @a20r
        JSLF xf_msel_all            ; Strg+A
@a20r:  jmp redraw_all
@a20:   cmp #15
        bcc @av
        cmp #20
        bcs @av
        jmp act_view
@av:    lda ACT
        cmp #11
        bne @a0
        lda #7
        jsr win_open
        jmp redraw_all
@a0:    cmp #5
        bne @a1
        jmp close_win
@a1:    cmp #9
        bne @a2
        stz VOLOK
        jsr reload_view
        jmp redraw_all
@a2:    cmp #10
        bne @a3
        jmp go_up
@a3:    cmp #4
        bne @a4
        jmp act_props
@a4:    lda ACT
        cmp #13
        bne @a4b
        JSLF xf_act_eject
        rts
@a4b:   lda MODE
        bne @a5
        lda ACT
        cmp #3
        beq @er
        cmp #14
        bne @v2
@er:    jmp act_erase
@v2:    cmp #2
        bne @v3
        lda SEL2
        bmi @v4
        jmp act_rename
@v4:    jmp redraw_all
@v3:    cmp #7                  ; Laufwerk kopieren / einfuegen
        bne @v5
        JSLF xf_vol_clip
        rts
@v5:    cmp #8
        bne @v6
        JSLF xf_vol_paste
        rts
@v6:    lda #5                  ; nur in Ordnern moeglich
        jmp show_msg
@a5:    lda ACT
        cmp #1
        bne @b1
        jmp act_newfolder
@b1:    cmp #12
        bne @b2
        lda EVREC+14            ; Wahltaste: ohne Rueckfrage leeren
        and #$0800
        bne @bq
        lda #3
        jmp act_confirm
@bq:    lda #3
        sta CONFACT
        jmp bin_go
@b2:    cmp #8
        bne @b3
        jmp act_paste
@b3:    ; ab hier wird eine Auswahl gebraucht
        lda SEL2
        bpl @b4
        jmp redraw_all
@b4:    lda ACT
        cmp #2
        bne @b5
        jmp act_rename
@b5:    cmp #3
        bne @b6
        jmp act_delete
@b6:    cmp #6
        bne @b7
        lda #1
        jmp act_clip
@b7:    cmp #7
        bne @b8
        lda #0
        jmp act_clip
@b8:    jmp redraw_all

; Strg-Tasten im Explorer: C=1 wenn behandelt
ctrl_key:
        ldx #0
@l:     lda ckeys,x
        beq @no
        cmp KEYC
        beq @f
        inx
        inx
        inx
        inx
        bra @l
@f:     lda ckeys+2,x
        jsr do_action
        sec
        rts
@no:    clc
        rts
ckeys:  .word $0E,1, $05,2, $04,3, $10,4, $18,6, $03,7, $16,8, $0C,13, $01,20, 0

; ---------- Neuer Ordner / Umbenennen ----------
act_newfolder:
        lda #1
        sta editsel
        lda #1
        sta TXTMODE
        LSPR t_defdir
        jsr edit_from_sprite_name
        lda #11
        jsr win_open
        jmp redraw_all

act_rename:
        lda #1
        sta editsel
        stz TXTMODE
        lda SEL2
        sta IDX
        jsr entep
        jsr act_rename_buf
        lda #11
        jsr win_open
        jmp redraw_all

; editbuf = Name des Eintrags EP
act_rename_buf:
        ldy #2
        lda [EP],y
        and #$00FF
        sta editbuf
        tax
        ldy #0
        sep #$20
@c:     cpx #0
        beq @e
        phy
        iny
        iny
        iny
        lda [EP],y
        ply
        sta editbuf+2,y
        iny
        dex
        bra @c
@e:     rep #$20
        rts

; Standardnamen aus Tabelle (sprachabhaengig)
edit_from_sprite_name:
        ldx #0
        lda LANG
        beq @de
        ldx #defname_en-defname
@de:    ldy #0
@c:     lda defname,x
        and #$00FF
        beq @e
        sep #$20
        sta editbuf+2,y
        rep #$20
        inx
        iny
        bra @c
@e:     sty editbuf
        rts
defname:    .byte "NEUER.ORDNER",0
defname_en: .byte "NEW.FOLDER",0

wc_edit:
        lda TXTMODE
        cmp #4
        bne @e4
        ICON WX+14,WY+30,i_doc
        TEXT WX+40,WY+34,t_ftype,BLACK
        jmp @fld
@e4:    cmp #3
        bne @e3
        ICON WX+14,WY+30,i_folder
        TEXT WX+40,WY+34,t_find,BLACK
        jmp @fld
@e3:    cmp #2
        bne @e0
        ICON WX+14,WY+30,i_hdd
        TEXT WX+40,WY+34,t_volnm,BLACK
        bra @fld
@e0:    ICON WX+14,WY+30,i_folder
        lda TXTMODE
        bne @nf
        TEXT WX+40,WY+34,t_newname,BLACK
        bra @fld
@nf:    TEXT WX+40,WY+34,t_fname,BLACK
@fld:   SUNKEN WX+40,WY+48,WW-60,14
        FILL WX+42,WY+50,WW-64,10,WHITE
        jsr draw_editbuf
        ; Schreibmarke
        lda SX
        sta RX
        lda #WY+51
        sta RY
        lda #1
        sta RW
        lda #9
        sta RH
        lda #BLACK
        jsr setcolor
        jsr fillrect
        BUTTON WX+WW-136,WY+WH-26,56,15
        TEXTC WX+WW-108,WY+WH-22,t_ok,BLACK
        BUTTON WX+WW-72,WY+WH-26,62,15
        TEXTC WX+WW-41,WY+WH-22,t_abbr,BLACK
        rts

edit_key:
        lda KEYC
        cmp #$1B
        bne @k1
        jmp win_close
@k1:    cmp #$0D
        bne @k2
        jmp edit_ok
@k2:    cmp #$7F
        beq @bs
        cmp #$08
        bne @k3
@bs:    stz editsel
        lda editbuf
        beq @r
        dec editbuf
        jmp edit_redraw
@k3:    lda TXTMODE
        cmp #4
        bne @k3n
        JSLF xf_edit_hex
        rts
@k3n:   ; erlaubte Zeichen: A-Z a-z 0-9 . (erstes Zeichen Buchstabe)
        lda editsel
        beq @k3b
        lda KEYC                ; nur bei gueltigem Zeichen ersetzen
        and #$00DF
        cmp #'A'
        bcc @k3b
        cmp #'Z'+1
        bcs @k3b
        stz editbuf
        stz editsel
@k3b:   lda editbuf
        cmp #15
        bcs @r
        lda KEYC
        cmp #'.'
        beq @dig
        cmp #'0'
        bcc @r
        cmp #'9'+1
        bcc @dig
        and #$00DF
        cmp #'A'
        bcc @r
        cmp #'Z'+1
        bcs @r
        lda KEYC
        bra @add
@dig:   ldx editbuf
        beq @r
@add:   ldx editbuf
        sep #$20
        sta editbuf+2,x
        rep #$20
        inc editbuf
        jmp edit_redraw
@r:     rts

draw_editbuf:
        lda #editbuf+2
        sta STRP
        lda editbuf
        sta STRN
        lda #WX+45
        sta SX
        lda #WY+51
        sta SY
        lda editsel
        beq @n
        jsr strwidth
        sta RW
        lda #WX+44
        sta RX
        lda #WY+50
        sta RY
        lda #10
        sta RH
        inc RW
        inc RW
        lda #NAVY
        jsr setcolor
        jsr fillrect
        lda #WHITE
        bra @c
@n:     lda #BLACK
@c:     jsr setcolor
        jmp drawstr

edit_redraw:
        FILL WX+42,WY+50,WW-64,10,WHITE
        jsr draw_editbuf
        lda SX
        sta RX
        lda #WY+51
        sta RY
        lda #1
        sta RW
        lda #9
        sta RH
        lda #BLACK
        jsr setcolor
        jmp fillrect

edit_click:
        HIT WX+WW-136,WY+WH-26,56,15
        bcc @n
        jmp edit_ok
@n:     HIT WX+WW-72,WY+WH-26,62,15
        bcc @r
        jmp win_close
@r:     rts

edit_ok:
        lda editbuf
        bne @go
        rts
@go:    lda TXTMODE
        cmp #4
        bne @g4
        JSLF xf_type_set
        rts
@g4:    cmp #3
        bne @g0
        JSLF xf_find_start
        rts
@g0:    cmp #2
        bne @g1
        jsr win_pop
        lda #4
        jmp act_confirm
@g1:    ; dstpath = pathbuf + ':' + editbuf
        lda MODE
        bne @g2
        stz dstpath
        bra @g3
@g2:    ldx #pathbuf
        ldy #dstpath
        jsr pcopy
@g3:
        lda #editname-3         ; Pseudo-Eintrag: Laenge bei +2, Name ab +3
        sta EP
        jsr epmy
        lda editbuf
        sep #$20
        sta editname-1
        rep #$20
        ldx #0
@c:     lda editbuf+2,x
        sta editname,x
        inx
        inx
        cpx #16
        bcc @c
        ldy #dstpath
        jsr pappend
        jsr win_pop
        lda TXTMODE
        beq @ren
        ; Ordner anlegen
        lda #$000F
        sta create_pb+8
        stz create_pb+10
        stz create_pb+12
        lda #$000D
        sta create_pb+14
        lda #$2001
        ldx #create_pb
        ldy #dstpath
        jsr gsosp
        bcc @ok
        jmp fail
@ren:   lda SEL2
        sta IDX
        jsr entry_path
        lda MODE
        bne @rn
        stz VOLOK
@rn:
        lda #srcpath
        sta chpath_pb+2
        lda #dstpath
        sta chpath_pb+6
        GSOS $2004,chpath_pb    ; ChangePath
        bcc @ok
        jmp fail
@ok:    lda MODE
        bne @ok2
        GSOS $200E,expand_pb    ; Startvolume neu bestimmen
@ok2:   jsr reload_view
        jmp redraw_all

; ---------- Loeschen / Papierkorb ----------
act_delete:
        lda ISBIN
        bne @perm
        ; liegt der Ordner auf dem Startvolume?
        ldx #pathbuf
        ldy #bootvol
        jsr samevol
        bcc @perm
        lda #1
        bra act_confirm
@perm:  lda #2
act_confirm:
        sta CONFACT
        lda #12
        jsr win_open
        jmp redraw_all

wc_confirm:
        lda CONFACT
        cmp #6
        bne @c6
        JSLF xf_wc_sys
        rts
@c6:    cmp #5
        bne @c5
        JSLF xf_wc_dcopy
        rts
@c5:    cmp #4
        bne @c0
        ICON WX+14,WY+30,i_hdd
        TEXT WX+40,WY+34,t_cerase,BLACK
        TEXT WX+40,WY+46,t_cerase2,BLACK
        jmp confirm_btns
@c0:    ICON WX+14,WY+30,i_bin
        lda CONFACT
        cmp #3
        bne @c1
        TEXT WX+40,WY+34,t_cempty,BLACK
        jmp confirm_btns
@c1:    cmp #1
        bne @c2
        TEXT WX+40,WY+34,t_cbin,BLACK
        bra @nm
@c2:    TEXT WX+40,WY+34,t_cdel,BLACK
@nm:    lda MSELN
        beq @nm1
        lda #WX+40
        sta SX
        lda #WY+46
        sta SY
        lda #BLACK
        jsr setcolor
        lda MSELN
        jsr drawnum
        LSPR t_nobj
        jsr blit1
        jmp confirm_btns
@nm1:   lda SEL2
        jsr entep
        jsr ep_nm
        ldy #2
        lda [EP],y
        and #$00FF
        sta STRN
        lda #WX+40
        sta SX
        lda #WY+50
        sta SY
        lda #NAVY
        jsr setcolor
        jsr drawstr
@btn:
confirm_btns:
        BUTTON WX+WW-136,WY+WH-26,56,15
        TEXTC WX+WW-108,WY+WH-22,t_yes,BLACK
        BUTTON WX+WW-72,WY+WH-26,62,15
        TEXTC WX+WW-41,WY+WH-22,t_no,BLACK
        rts

confirm_click:
        HIT WX+WW-136,WY+WH-26,56,15
        bcc @n
        jmp confirm_yes
@n:     HIT WX+WW-72,WY+WH-26,62,15
        bcc @r
        jmp confirm_no
@r:     rts

; "Nein": beim Systemordner trotzdem ablegen (nicht einsortiert)
confirm_no:
        lda CONFACT
        cmp #6
        beq @s
        jmp win_close
@s:     jsr win_pop
        jmp drop_xfer

confirm_key:
        lda KEYC
        and #$00DF
        cmp #'J'
        beq @y
        cmp #'Y'
        beq @y
        lda KEYC
        cmp #$0D
        beq @y
        cmp #$1B
        beq @n
        and #$00DF
        cmp #'N'
        bne @r
@n:     jmp confirm_no
@y:     jmp confirm_yes
@r:     rts

confirm_yes:
        jsr win_pop
        jsr show_busy
        lda CONFACT
        cmp #6
        bne @c6
        inc SYSSORT             ; Systemordner: einsortieren
        jmp drop_xfer
@c6:    cmp #5
        bne @c5
        JSLF xf_do_dcopy
        rts
@c5:    cmp #4
        bne @c4
        jmp do_erase
@c4:    cmp #3
        bne @c1
        jmp bin_go
@c1:    JSLF xf_sel_list
        stz PSTI
@dl:    lda PSTI
        cmp SLN
        bcc @dg
        jmp confirm_done
@dg:    asl
        tax
        lda sellist,x
        sta IDX
        jsr del_one
        bcc @dn
        jmp fail
@dn:    inc PSTI
        bra @dl

; einen Eintrag IDX loeschen bzw. in den Papierkorb. C=1/A Fehler
del_one:
        jsr entry_path
        lda CONFACT
        cmp #1
        bne @perm
        ; in den Papierkorb verschieben: *:RECYCLED anlegen (Fehler egal)
        lda #$000F
        sta create_pb+8
        stz create_pb+10
        stz create_pb+12
        lda #$000D
        sta create_pb+14
        lda #$2001
        ldx #create_pb
        ldy #binpath
        jsr gsosp
        ldx #binpath
        ldy #dstpath
        jsr pcopy
        lda IDX
        jsr entep
        ldy #dstpath
        jsr pappend
        lda #srcpath
        sta chpath_pb+2
        lda #dstpath
        sta chpath_pb+6
        GSOS $2004,chpath_pb
        bcs @e
        JSLF xf_bin_note            ; Herkunft merken (Wiederherstellen)
        rts
@e:     rts
@perm:  ldx #srcpath
        ldy #workpath
        jsr pcopy
        jmp destroy_tree

; Papierkorb leeren mit Fortschritt (auch Wahltaste ohne Rueckfrage)
bin_go: JSLF xf_bin_empty
        jsr reload_view
        jmp redraw_all

confirm_done:
@ok:    jsr reload_view
        jmp redraw_all

; "Bitte warten" im Fenster
show_busy:
        jsr redraw_all
        lda WIN
        cmp #1
        bne @r
        jsr anc_b
        FILL WX+6,WY+WH-14,110,9,GRAY
        TEXT WX+8,WY+WH-14,t_wait,BLACK
        jsr anc_end
@r:     rts

; Datei oder Ordner in workpath rekursiv loeschen. C=1/A = Fehler
destroy_tree:
        stz DEPTH
@loop:  lda #$2006
        ldx #gfi_pb
        ldy #workpath
        jsr gsosp               ; GetFileInfo
        bcc *+5
        jmp @err
        lda gfi_pb+14
        cmp #$000D
        beq @dir
        cmp #$000F
        beq @dir
@kill:  lda #$2002
        ldx #destroy_pb
        ldy #workpath
        jsr gsosp               ; Destroy
        bcs @err
@up:    lda DEPTH
        beq @done
        ldy #workpath
        jsr pstrip
        dec DEPTH
        jmp @loop
@dir:   ; ersten Eintrag des Ordners holen
        lda #workpath
        sta wopen_pb+4
        GSOS $2010,wopen_pb
        bcs @err
        lda wopen_pb+2
        sta wdirent_pb+2
        sta wclose_pb+2
        lda #1
        sta wdirent_pb+6
        sta wdirent_pb+8
        GSOS $201C,wdirent_pb
        php
        GSOS $2014,wclose_pb
        plp
        bcs @kill               ; leer -> Ordner selbst loeschen
        lda #wname+1            ; Pseudo-Eintrag: Laenge bei +2, Name ab +3
        sta EP
        jsr epmy
        lda wname+2
        sep #$20
        sta wname+3
        rep #$20
        ldy #workpath
        jsr pappend
        inc DEPTH
        jmp @loop
@done:  clc
        rts
@err:   sec
        rts

; ---------- Zwischenablage ----------
act_clip:
        sta clipcut
        lda MODE
        bne @files
        JSLF xf_vol_clip            ; Arbeitsplatz: Laufwerk als Quelle merken
        rts
@files:
        ldx #pathbuf
        ldy #clipdir
        jsr pcopy
        JSLF xf_sel_list
        ldy #clipents
        JSLF xf_sel_copy
        lda SLN
        sta CLIPN
        sta clipok
        jmp redraw_all

show_msg_idx:
        jmp show_msg

act_paste:
        lda MODE
        bne @files
        JSLF xf_vol_paste           ; Arbeitsplatz: Datentraeger kopieren
        rts
@files: lda clipok
        bne @go
        lda #7
        jmp show_msg
@go:    lda CLIPN
        ldx clipcut
        JSLF xf_prog_begin
        stz PSTI
@l:     lda PSTI
        cmp CLIPN
        bcs @done
        ; EP = clipents + i*ENTSZ
        asl
        asl
        asl
        asl
        sta TMP
        asl
        clc
        adc TMP
        clc
        adc #clipents
        sta EP
        jsr epmy
        ; srcpath = clipdir:name, dstpath = pathbuf:name
        ldx #clipdir
        ldy #srcpath
        jsr pcopy
        ldy #srcpath
        jsr pappend
        ldx #pathbuf
        ldy #dstpath
        jsr pcopy
        ldy #dstpath
        jsr pappend
        lda [EP]
        sta XFTYPE
        lda clipcut
        sta XCUT
        jsr xfer
        bcs @err
        JSLF xf_prog_step
        inc PSTI
        bra @l
@done:  JSLF xf_prog_end
        lda clipcut
        beq @ok
        stz clipok
@ok:    jsr reload_view
        jmp redraw_all
@err:   pha
        JSLF xf_prog_end
        pla
        cmp #XABORTC
        beq @ok
        jmp fail

; srcpath -> dstpath; XFTYPE = Dateityp, XCUT = verschieben. C=1/A = Fehler
xfer:
        lda XCUT
        beq @copy
        ldx #srcpath
        ldy #dstpath
        jsr samevol
        bcc @copy
        lda #srcpath
        sta chpath_pb+2
        lda #dstpath
        sta chpath_pb+6
        GSOS $2004,chpath_pb
        rts
@copy:  lda XFTYPE
        cmp #$000F
        bne @cf
        jsr copy_tree
        bcs @r
        lda XCUT
        beq @ok
        ldx #srcpath
        ldy #workpath
        jsr pcopy
        jmp destroy_tree
@cf:    jsr copy_file
        bcs @r
        lda XCUT
        beq @ok
        lda #$2002
        ldx #destroy_pb
        ldy #srcpath
        jmp gsosp
@ok:    clc
@r:     rts

; Ordner srcpath rekursiv nach dstpath kopieren. C=1/A = Fehler
copy_tree:
        jsr inside_chk
        bcc @c0
        lda #$0050              ; Ziel liegt im Quellordner
        sec
        rts
@c0:    jsr mkdir_dst
        bcc @c1
        rts
@c1:    stz DEPTH
        lda #1
        sta nstack
@loop:  lda #srcpath
        sta wopen_pb+4
        GSOS $2010,wopen_pb
        bcc @o
        rts
@o:     lda wopen_pb+2
        sta wdirent_pb+2
        sta wclose_pb+2
        stz wdirent_pb+6        ; absolut
        lda DEPTH
        asl
        tax
        lda nstack,x
        sta wdirent_pb+8
        GSOS $201C,wdirent_pb
        php
        GSOS $2014,wclose_pb
        plp
        bcs @up
        lda #wname+1
        sta EP
        jsr epmy
        lda wname+2
        sep #$20
        sta wname+3
        rep #$20
        ldy #srcpath
        jsr pappend
        ldy #dstpath
        jsr pappend
        lda wdirent_pb+16
        cmp #$000F
        bne @file
        jsr mkdir_dst
        bcc @md
        rts
@md:    lda DEPTH
        cmp #7
        bcc @deep
        lda #$0050
        sec
        rts
@deep:  inc DEPTH
        lda DEPTH
        asl
        tax
        lda #1
        sta nstack,x
        jmp @loop
@file:  jsr copy_file
        bcc @strip
        rts
@strip: ldy #srcpath
        jsr pstrip
        ldy #dstpath
        jsr pstrip
@next:  lda DEPTH
        asl
        tax
        inc nstack,x
        jmp @loop
@up:    lda DEPTH
        beq @done
        dec DEPTH
        bra @strip
@done:  clc
        rts

mkdir_dst:
        lda #$000F
        sta create_pb+8
        stz create_pb+10
        stz create_pb+12
        lda #$000D
        sta create_pb+14
        lda #$2001
        ldx #create_pb
        ldy #dstpath
        jmp gsosp

; C=1 wenn dstpath innerhalb von srcpath liegt
inside_chk:
        lda dstpath
        cmp srcpath
        beq @no
        bcc @no
        ldy #2
@l:     tya
        sec
        sbc #2
        cmp srcpath
        bcs @end
        lda srcpath,y
        and #$00FF
        jsr upc
        sta TMP
        lda dstpath,y
        and #$00FF
        jsr upc
        cmp TMP
        bne @no
        iny
        bra @l
@end:   lda dstpath,y
        and #$00FF
        cmp #':'
        bne @no
        sec
        rts
@no:    clc
        rts

; Datei srcpath -> dstpath kopieren (beide Forks)
copy_file:
        JSLF xf_prog_file
        jsr copy_file0
        bcc @r
        cmp #XABORTC            ; abgebrochen: halbe Kopie entfernen
        bne @e
        pha
        lda #$2002
        ldx #destroy_pb
        ldy #dstpath
        jsr gsosp
        pla
@e:     sec
@r:     rts
copy_file0:
        lda #$2006
        ldx #gfi_pb
        ldy #srcpath
        jsr gsosp
        bcc @c0
        rts
@c0:    lda gfi_pb+8
        sta create_pb+8
        lda gfi_pb+10
        sta create_pb+10
        lda gfi_pb+12
        sta create_pb+12
        ldx #1
        lda gfi_pb+14
        cmp #5
        bne @st
        ldx #5
@st:    stx create_pb+14
        lda #$2001
        ldx #create_pb
        ldy #dstpath
        jsr gsosp
        bcc @c1
        rts
@c1:    stz FCOUNT
        jsr copy_fork
        bcs @r
        lda gfi_pb+14
        cmp #5
        bne @ok
        lda #1
        sta FCOUNT
        jsr copy_fork
        bcs @r
@ok:    clc
@r:     rts

; Fork FCOUNT kopieren
copy_fork:
        lda FCOUNT
        sta o4_pb+10
        lda #1
        sta o4_pb+8
        lda #srcpath
        sta o4_pb+4
        GSOS $2010,o4_pb
        bcc @o1
        rts
@o1:
        lda o4_pb+2
        sta refsrc
        lda #2
        sta o4_pb+8
        lda #dstpath
        sta o4_pb+4
        GSOS $2010,o4_pb
        bcc @o2
        pha
        lda refsrc
        sta cl_pb+2
        GSOS $2014,cl_pb
        pla
        sec
        rts
@o2:    lda o4_pb+2
        sta refdst
@loop:  JSLF xf_prog_poll
        bcc @lp
        lda #XABORTC
        jmp @fail
@lp:    lda refsrc
        sta io_pb+2
        lda #COPYBUF
        sta io_pb+8
        GSOS $2012,io_pb        ; Read
        bcc @w
        cmp #$004C              ; Dateiende
        beq @done
        bra @fail
@w:     lda io_pb+12
        beq @done
        sta io_pb+8
        lda refdst
        sta io_pb+2
        GSOS $2013,io_pb        ; Write
        bcs @fail
        lda io_pb+8
        cmp #COPYBUF
        bcc @done
        jmp @loop
@done:  jsr @closeboth
        clc
        rts
@fail:  pha
        jsr @closeboth
        pla
        sec
@r:     rts
@closeboth:
        lda refsrc
        sta cl_pb+2
        GSOS $2014,cl_pb
        lda refdst
        sta cl_pb+2
        GSOS $2014,cl_pb
        rts

; ---------- Eigenschaften ----------
act_props:
        lda SEL2
        bmi @r
        sta IDX
        jsr entry_path
        lda MODE
        bne @f
        lda IDX                 ; leeres oder 5,25"-Laufwerk: nicht abfragen
        jsr entep               ; (GS/OS wuerde alle Laufwerke durchsuchen)
        lda [EP]
        cmp #$0101
        bcc @f
        JSLF xf_pv_none
        lda #13
        jsr win_open
        jmp redraw_all
@f:     lda #$2006
        ldx #gfi_pb
        ldy #srcpath
        jsr gsosp
        bcc @ok
        jmp fail
@ok:    JSLF xf_pv_load         ; Textvorschau laden
        lda #13
        jsr win_open
        jmp redraw_all
@r:     rts

wc_props:
        lda SEL2
        jsr entep
        lda [EP]
        jsr type_icon
        sta SPR
        lda #WX+14
        sta SX
        lda #WY+28
        sta SY
        jsr blit4
        TEXT WX+40,WY+32,p_name,BLACK
        jsr ep_nm
        ldy #2
        lda [EP],y
        and #$00FF
        sta STRN
        lda #WX+100
        sta SX
        lda #WY+32
        sta SY
        jsr drawstr
        TEXT WX+40,WY+46,p_type,BLACK
        ; Typbeschreibung
        lda [EP]
        jsr type_icon
        lda TYPN
        sta SPR
@tt:    lda #WX+100
        sta SX
        lda #WY+46
        sta SY
        jsr blit1
        ; ($XX), nicht bei Laufwerken
        lda [EP]
        cmp #$0100
        bcc @hx
        jmp @nosz
@hx:    lda SX
        clc
        adc W
        adc #6
        sta SX
        lda #'$'
        jsr putch
        lda [EP]
        and #$00FF
        jsr puthex2
        ; Groesse (nur Dateien)
        lda [EP]
        cmp #$000F
        beq @nosz
        cmp #$0100
        bcc *+5
        jmp @nosz
        TEXT WX+40,WY+60,p_size,BLACK
        lda #WX+100
        sta SX
        lda #WY+60
        sta SY
        lda gfi_pb+36
        sta NUMLO
        lda gfi_pb+38
        sta NUMHI
        jsr putdec32
        LSPR p_bytes
        jsr blit1
@nosz:  lda [EP]                ; Laufwerk ohne Datentraeger: kein Datum
        cmp #$0101
        bcc @dt
        jmp @nodt
@dt:    ; Aenderungsdatum
        TEXT WX+40,WY+74,p_mod,BLACK
        lda #WX+100
        sta SX
        lda #WY+74
        sta SY
        lda gfi_pb+28           ; Tag (0-basiert) / Monat (0-basiert)
        and #$00FF
        inc a
        jsr put2dec
        lda #'.'
        jsr putch
        lda gfi_pb+29
        and #$00FF
        inc a
        jsr put2dec
        lda #'.'
        jsr putch
        lda gfi_pb+27           ; Jahr seit 1900
        and #$00FF
        clc
        adc #1900
        sta NUMLO
        stz NUMHI
        jsr putdec32
        lda #' '
        jsr putch
        lda #' '
        jsr putch
        lda gfi_pb+26           ; Stunde
        and #$00FF
        jsr put2dec
        lda #':'
        jsr putch
        lda gfi_pb+25           ; Minute
        and #$00FF
        jsr put2dec
@nodt:  JSLF xf_props_lock_draw
        JSLF xf_props_type_draw
        JSLF xf_pv_draw
        jmp okbtn

; Zeichen A an SX,SY
putch:  jsr glyphptr
        sta SPR
        jsr blit1
        lda SX
        sec
        adc W
        sta SX
        rts

puthex2:
        pha
        lsr
        lsr
        lsr
        lsr
        jsr @nib
        pla
@nib:   and #$000F
        cmp #10
        bcc @d
        adc #6                  ; C=1 -> +7
@d:     clc
        adc #'0'
        jmp putch

put2dec:
        ldx #0
@d:     cmp #10
        bcc @e
        sbc #10
        inx
        bra @d
@e:     pha
        txa
        clc
        adc #'0'
        jsr putch
        pla
        clc
        adc #'0'
        jmp putch

; 32-Bit-Zahl NUMHI:NUMLO dezimal
putdec32:
        stz DIGS
@d1:    ; durch 10 teilen
        ldx #32
        lda #0
        sta TMP2                ; Rest
@sh:    asl NUMLO
        rol NUMHI
        rol TMP2
        lda TMP2
        cmp #10
        bcc @ns
        sbc #10
        sta TMP2
        inc NUMLO
@ns:    dex
        bne @sh
        lda TMP2
        pha
        inc DIGS
        lda NUMLO
        ora NUMHI
        bne @d1
@out:   pla
        clc
        adc #'0'
        jsr putch
        dec DIGS
        bne @out
        rts

; ---------- Datentraeger loeschen / initialisieren ----------
act_erase:
        lda SEL2
        bpl @g
        jmp redraw_all
@g:     sta IDX
        jsr entep
        lda [EP]
        cmp #$0101
        beq @unf
        jsr entry_path
        ldx #srcpath
        ldy #bootvol
        jsr samevol
        bcc @nb
        lda #10                 ; Startlaufwerk: nicht erlaubt
        jmp show_msg
@nb:    ; bisherigen Namen vorschlagen
        jsr act_rename_buf
        bra @dlg
@unf:   ldx #0
        lda LANG
        beq @de
        ldx #untit_en-untit
@de:    ldy #0
@c:     lda untit,x
        and #$00FF
        beq @e
        sep #$20
        sta editbuf+2,y
        rep #$20
        inx
        iny
        bra @c
@e:     sty editbuf
@dlg:   lda #1
        sta editsel
        lda #2
        sta TXTMODE
        lda #11
        jsr win_open
        jmp redraw_all
untit:    .byte "NEUE.DISK",0
untit_en: .byte "NEW.DISK",0

do_erase:
        lda SEL2
        asl
        tax
        lda voldev,x
        sta dinfo_pb+2
        GSOS $202C,dinfo_pb     ; Geraetename holen
        bcc @d
        jmp fail
@d:     ; erasename = ':' + editbuf
        sep #$20
        lda #':'
        sta erasename+2
        rep #$20
        ldx #0
@c:     lda editbuf+2,x
        sta erasename+3,x
        inx
        inx
        cpx #16
        bcc @c
        lda editbuf
        inc a
        sta erasename
        stz erase_pb+10
        lda #1
        sta erase_pb+12         ; ProDOS, ohne Apple-Dialog
        GSOS $2025,erase_pb     ; EraseDisk
        bcc @ok
        stz erase_pb+10
        GSOS $2024,erase_pb     ; Format (unformatierte Datentraeger)
        bcc @ok
        jmp fail
@ok:    stz VOLOK
        jsr reload_view
        jmp redraw_all

exttab: .word act_dup_m, act_print_m, act_find_m, act_findnext_m, act_open_m, act_link_m, act_prgadd_m, act_restore_m, act_tree

; Ordnerbaum ein/aus
act_tree:
        lda TREEOFF
        eor #1
        sta TREEOFF
        stz PAGE
        jsr save_prefs
        jmp redraw_all

; ---------- Ansicht: 15/16 Symbole/Details, 17-19 Sortierung ----------
act_view:
        cmp #17
        bcs @sort
        sec
        sbc #15
        sta VIEWM
        stz PAGE
        jsr save_prefs
        jmp redraw_win
@sort:  sec
        sbc #17
        sta SORTM
        lda #$FFFF
        sta SEL2
        jsr save_prefs
        jsr sort_entries
        jmp redraw_win
