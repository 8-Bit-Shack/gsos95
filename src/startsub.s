        .segment "XCODE2"
FARMODE .set 2
xf_act_prgadd:
        jsr act_prgadd
        rtl
xf_draw_sub:
        jsr draw_sub
        jsr s2_redraw
        rtl
xf_sub_action:
        jsr sub_action
        rtl
xf_sub_close:
        jsr sub_close
        rtl
xf_sub_hit:
        jsr sub_hit
        rtl
xf_sub_hover:
        jsr sub_hover
        rtl
xf_sub_key:
        jsr sub_key
        rtl
xf_sub_open:
        jsr sub_open
        rtl
; ================================================================
; GS/OS95: Untermenues im Startmenue (Programme, Dokumente)
; ================================================================
SUBX = MNX+MNW-2
SUBW = 112
S2X  = SUBX+SUBW-2              ; Gruppe (zweites Untermenue)
S2W  = 320-S2X
SGN  = 3                        ; Gruppen in Programme: Werkzeuge, Internet, Spiele
SPRG = SGN+1                    ; erste Programmzeile (nach Arbeitsplatz)
SUBRH = 12
SUBMV = 14                      ; hoechstens so viele Zeilen sichtbar, sonst blaettern

; Zeilenzahl des offenen Untermenues -> A
sub_rows:
        lda SUBM
        cmp #1
        bne @d
        lda PRGLN               ; Gruppen, Arbeitsplatz, eigene Programme
        clc
        adc #SPRG
        rts
@d:     lda DOCLN
        bne @r
        lda #1                  ; "(leer)"
@r:     rts

; Untermenue zu Hauptmenue-Eintrag A (0/1) oeffnen
sub_open:
        inc a
        cmp SUBM
        bne @n
        rts
@n:     pha
        jsr sub_close
        pla
        sta SUBM
        lda #$FFFF
        sta SUBH
        ; Lage
        lda SUBM
        dec a
        asl
        tax
        lda itemy,x
        sta SUBY
        stz SUBTOP
        jsr sub_rows
        cmp #SUBMV+1
        bcc @v
        lda #SUBMV
@v:     sta SUBVIS
        ldx #SUBRH
        CALL gmul
        clc
        adc #4
        sta SUBHT
        clc
        adc SUBY
        cmp #186
        bcc @y
        lda #186
        sec
        sbc SUBHT
        sta SUBY
@y:     CALL org0
        jsr draw_sub
        CALL orgr
        rts

; Untermenue schliessen (Flaeche neu zeichnen)
sub_close:
        jsr s2_close
        lda SUBM
        bne @go
        rts
@go:    stz SUBM
        lda #SUBX
        sta KL
        lda #SUBX+SUBW
        sta KR
        lda SUBY
        sta KT
        clc
        adc SUBHT
        sta KB
        CALL redraw_rect
        rts

draw_sub:
        lda SUBM
        bne @go
        rts
@go:    lda #SUBX
        sta RX
        lda SUBY
        sta RY
        lda #SUBW
        sta RW
        lda SUBHT
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
; sichtbare Zeilen zeichnen
sub_redrows:
        stz SUBI
@l:     lda SUBI
        cmp SUBVIS
        bcs @e
        clc
        adc SUBTOP
        jsr draw_subrow
        inc SUBI
        bra @l
@e:     rts

; Zeile A zeichnen (nur wenn sichtbar), am Rand Pfeil fuer weitere Zeilen
draw_subrow:
        sta SUBK
        sec
        sbc SUBTOP
        bcc @r
        cmp SUBVIS
        bcs @r
        jsr dsr_body
        jmp sub_arrow
@r:     rts
dsr_body:
        ldx #SUBRH
        CALL gmul
        clc
        adc SUBY
        inc a
        inc a
        sta RY
        lda #SUBX+2
        sta RX
        lda #SUBW-4
        sta RW
        lda #SUBRH
        sta RH
        lda SUBK
        cmp SUBH
        bne @n
        lda #NAVY
        bra @f
@n:     lda #GRAY
@f:     CALL setcolor
        CALL fillrect
        lda RY
        clc
        adc #2
        sta SY
        lda #SUBX+8
        sta SX
        lda SUBK
        cmp SUBH
        bne @b
        lda #WHITE
        bra @c
@b:     lda #BLACK
@c:     CALL setcolor
        ; Text
        lda SUBM
        cmp #1
        beq *+5
        jmp @doc
        lda SUBK
        cmp #SPRG
        bcs @pe
        asl
        tax
        lda sg_icon,x
        jsr ss_icon
        lda SUBK
        cmp #SGN
        bne @grp
        LSPR t_arbeit           ; Arbeitsplatz
        CALL blit1
        rts
@grp:   asl
        tax
        lda sg_text,x
        ldy LANG
        beq *+5
        lda sg_text_en,x
        JSLF xf_ui_str
        jmp ss_arrow
@pe:    lda #i_mprg
        jsr ss_icon
        lda SUBK
        sec
        sbc #SPRG
        ldx #1
        bra @lab
@doc:   lda DOCLN
        bne @de
        LSPR t_leerm
        lda #DKGRAY
        CALL setcolor
        CALL blit1
        rts
@de:    lda SUBK
        ldx #2
@lab:   pha
        JSLF xf_pl_sel
        pla
        JSLF xf_pl_addr
        ldx #18
        JSLF xf_pl_label
        CALL drawstr
        rts

; Maus im Untermenue? -> A = Zeile, sonst $FFFF
sub_hit:
        jsr s2_hit              ; erst die Gruppe ($4000 + Zeile)
        bmi sub_hit1
        rts
sub_hit1:
        lda SUBM
        beq @no
        lda #SUBX
        sta RX
        lda SUBY
        inc a
        inc a
        sta RY
        lda #SUBW
        sta RW
        lda SUBHT
        sec
        sbc #4
        sta RH
        CALL inrect
        bcc @no
        lda MY
        sec
        sbc RY
        ldx #SUBRH
        CALL gdiv
        clc
        adc SUBTOP
        rts
@no:    lda #$FFFF
        rts

; Pfeil (oben/unten) in Zeile SUBK, wenn dort weitergeblaettert werden kann
sub_arrow:
        lda SUBTOP
        beq @dn
        cmp SUBK
        bne @dn
        stz SUBAR               ; oben: Spitze nach oben
        bra @go
@dn:    lda SUBTOP
        clc
        adc SUBVIS
        sta SUBAR
        jsr sub_rows
        cmp SUBAR
        beq @r
        bcc @r
        lda SUBAR
        dec a
        cmp SUBK
        bne @r
        lda #1
        sta SUBAR               ; unten: Spitze nach unten
@go:    lda SUBK
        cmp SUBH
        bne @b
        lda #WHITE
        bra @c
@b:     lda #BLACK
@c:     CALL setcolor
        lda SUBK
        sec
        sbc SUBTOP
        ldx #SUBRH
        CALL gmul
        clc
        adc SUBY
        clc
        adc #6
        sta RY
        lda #1
        sta RH
        stz SUBAI
@l:     lda SUBAR
        beq @u
        lda #2
        sec
        sbc SUBAI
        bra @k
@u:     lda SUBAI
@k:     sta SUBAK
        asl
        inc a
        sta RW
        lda #SUBX+SUBW-10
        sec
        sbc SUBAK
        sta RX
        CALL fillrect
        inc RY
        inc SUBAI
        lda SUBAI
        cmp #3
        bcc @l
@r:     rts

; Maus auf der ersten/letzten sichtbaren Zeile: eine Zeile weiterblaettern
sub_edge:
        sta SUBAK
        cmp SUBTOP
        bne @d
        lda SUBTOP
        beq @x
        dec SUBTOP
        bra @rd
@d:     lda SUBTOP
        clc
        adc SUBVIS
        dec a
        cmp SUBAK
        bne @x
        inc a
        sta SUBAR
        jsr sub_rows
        cmp SUBAR
        beq @x
        bcc @x
        inc SUBTOP
@rd:    jsr sub_redrows
        jmp sub_hit
@x:     lda SUBAK
        rts

; Zeile A per Tastatur gewaehlt: sichtbar machen. C=1 wenn neu gezeichnet (X bleibt)
sub_kvis:
        phx
        cmp SUBTOP
        bcc @up
        sta SUBAK
        lda SUBTOP
        clc
        adc SUBVIS
        cmp SUBAK
        beq @dn
        bcc @dn
        plx
        clc
        rts
@up:    sta SUBTOP
        bra @rd
@dn:    lda SUBAK
        sec
        sbc SUBVIS
        inc a
        sta SUBTOP
@rd:    jsr sub_redrows
        plx
        sec
        rts

sub_hover:
        jsr s2_hit              ; Maus in der Gruppe
        bmi @h1
        and #$00FF
        jmp s2_hov
@h1:    jsr sub_hit1
        bmi @h0
        jsr sub_edge
        pha                     ; Gruppe unter der Maus: aufklappen, sonst zu
        ldx SUBM
        cpx #1
        bne @h2
        cmp #SGN
        bcs @h2
        jsr s2_open
        bra @h3
@h2:    jsr s2_close
@h3:    pla
@h0:    cmp SUBH
        beq @r
        ldx SUBH
        sta SUBH
        txa
        bmi @n
        jsr draw_subrow
@n:     lda SUBH
        bmi @r
        jsr draw_subrow
@r:     rts

; Zeile A ausfuehren
sub_action:
        cmp #$4000              ; Eintrag einer Gruppe
        bcc @s1
        and #$00FF
        jmp s2_action
@s1:    ldx SUBM                ; Gruppe: aufklappen (Tastatur: hinein)
        cpx #1
        bne @s2
        cmp #SGN
        bcs @s2
        sta SUBH
        CALL org0
        lda SUBH
        jsr s2_open
        lda #1
        sta S2F
        stz S2H
        jsr s2_rows
        CALL orgr
        rts
@s2:    sta SUBK
        lda SUBM
        sta SUBI                ; Art merken (TMP2 wird beim Zeichnen benutzt)
        stz SUBM
        stz MENU
        lda #$FFFF
        sta HOVER
        CALL redraw_menu
        lda SUBI
        cmp #1
        bne @doc
        lda SUBK
        cmp #SPRG
        bcs @pr
        cmp #SGN
        bne @r
        CALL open_explorer
@r:     rts
@pr:    sec
        sbc #SPRG
        ldx #1
        bra @go
@doc:   lda SUBK
        cmp DOCLN
        bcc @dd
        rts
@dd:    ldx #2
@go:    pha
        JSLF xf_pl_sel
        pla
        JSLF xf_pl_addr
        JSLF xf_path_open
        rts

; Tasten bei offenem Untermenue. C=1 wenn behandelt
sub_key:
        lda SUBM
        bne @go
        clc
        rts
@go:    lda S2F                 ; Tastatur in der Gruppe
        beq @g1
        jmp s2_key
@g1:    lda KEYC
        cmp #$15                ; rechts: Gruppe oeffnen
        bne @g2
        lda SUBM
        cmp #1
        bne @h0
        lda SUBH
        bmi @h0
        cmp #SGN
        bcs @h0
        jsr sub_action
@h0:    sec
        rts
@g2:    lda KEYC
        cmp #$0A
        bne @k1
        lda SUBH
        inc a
        sta TMP
        jsr sub_rows
        cmp TMP
        beq @z
        bcc @z
        lda TMP
        bra @set
@z:     lda #0
        bra @set
@k1:    cmp #$0B
        bne @k2
        lda SUBH
        dec a
        bpl @set
        jsr sub_rows
        dec a
        bra @set
@k2:    cmp #$0D
        bne @k3
        lda SUBH
        bmi @h
        jsr sub_action
@h:     sec
        rts
@k3:    cmp #$08                ; links / Escape: Untermenue zu
        beq @cl
        cmp #$1B
        bne @h
@cl:    jsr sub_close
        CALL org0
        CALL draw_menu
        CALL orgr
        sec
        rts
@set:   pha
        ldx SUBH
        sta SUBH
        CALL org0
        pla
        pha
        jsr sub_kvis            ; ausserhalb: blaettern, alles neu
        bcs @n3
        txa
        bmi @n
        jsr draw_subrow
@n:     pla
        jsr draw_subrow
        bra @n4
@n3:    pla
@n4:    CALL orgr
        sec
        rts

; ---------- Kontextmenue "Ins Startmenue" (nochmal = wieder entfernen) ----------
act_prgadd:
        lda SEL2
        bmi @r
        cmp COUNT
        bcs @r
        sta IDX
        CALL entry_path
        lda srcpath
        cmp #65
        bcs @r
        lda [EP]
        sta PLTYP
        ldy #E_AUX
        lda [EP],y
        sta PLAUX
        ldx #1
        JSLF xf_pl_sel
        JSLF xf_pl_find
        bcs @add
        JSLF xf_pl_del
        bra @sv
@add:   JSLF xf_pl_append
        bcs @r
@sv:    JSLF xf_pl_save
        rts
@r:     rts

; ================================================================
; Gruppen im Untermenue Programme (zweite Ebene): Werkzeuge, Internet, Spiele
; ================================================================
        .pushseg
        .segment "BSS"
S2G:    .res 2                  ; offene Gruppe + 1 (0 = keine)
S2Y:    .res 2
S2HT:   .res 2
S2N:    .res 2                  ; Eintraege
S2A:    .res 2                  ; erster Eintrag (Index in si_*)
S2H:    .res 2                  ; markierte Zeile ($FFFF keine)
S2F:    .res 2                  ; Tastatur in der Gruppe
S2I:    .res 2
        .popseg
        .pushseg
        .segment "RODATA"
sg_icon:   .addr i_tfold, i_tfold, i_tfold, i_qcomp
sg_text:   .addr ws_gtools, ws_ginet, ws_ggames
sg_text_en: .addr ws_gtools_en, ws_ginet_en, ws_ggames_en
sg_n:      .byte 5, 5, 2
sg_first:  .byte 0, 5, 10
si_icon:   .addr i_mcalc, i_mcd, i_mdos, i_mpaint, i_medit, i_qweb, i_qterm, i_marc, i_mnews, i_mwx, i_qmine, i_msol
si_text:   .addr ws_calc, ws_cdp, ws_dos, ws_paint, ws_edit, ws_inet, ws_term, ws_arc, ws_news, ws_wx, ws_mine, ws_sol
si_text_en: .addr ws_calc_en, ws_cdp_en, ws_dos_en, ws_paint_en, ws_edit_en, ws_inet_en, ws_term_en, ws_arc_en, ws_news_en, ws_wx_en, ws_mine_en, ws_sol_en
        .popseg

; Symbol A links in der Zeile (RY), Text danach (SX/SY)
ss_icon:
        sta SPR
        lda RY
        inc a
        sta SY
        lda RX
        clc
        adc #2
        sta SX
        CALL blit4
        lda RX
        clc
        adc #14
        sta SX
        lda RY
        clc
        adc #2
        sta SY
        rts
; Pfeil nach rechts am Ende der Zeile (RY), Farbe wie der Text
ss_arrow:
        lda RY
        clc
        adc #3
        sta RY
        lda #SUBX+SUBW-9
        sta RX
        lda #1
        sta RW
        lda #7
        sta RH
        stz S2I
@l:     CALL fillrect
        inc RX
        inc RY
        dec RH
        dec RH
        inc S2I
        lda S2I
        cmp #4
        bcc @l
        rts

; Gruppe A (0..2) aufklappen (ohne org0)
s2_open:
        inc a
        cmp S2G
        bne @n
        rts
@n:     pha
        jsr s2_close
        pla
        sta S2G
        stz S2F
        lda #$FFFF
        sta S2H
        lda S2G
        dec a
        tax
        lda sg_n,x
        and #$00FF
        sta S2N
        lda sg_first,x
        and #$00FF
        sta S2A
        lda S2G
        dec a
        sec
        sbc SUBTOP
        ldx #SUBRH
        CALL gmul
        clc
        adc SUBY
        sta S2Y
        lda S2N
        ldx #SUBRH
        CALL gmul
        clc
        adc #4
        sta S2HT
        clc
        adc S2Y
        cmp #186
        bcc @d
        lda #186
        sec
        sbc S2HT
        sta S2Y
@d:
s2_frame:
        lda #S2X                ; Rahmen
        sta RX
        lda S2Y
        sta RY
        lda #S2W
        sta RW
        lda S2HT
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
s2_rows:
        stz S2I
@l:     lda S2I
        cmp S2N
        bcs @e
        jsr s2_row
        inc S2I
        bra @l
@e:     rts
; Zeile S2I der Gruppe zeichnen
s2_row: lda S2I
        ldx #SUBRH
        CALL gmul
        clc
        adc S2Y
        inc a
        inc a
        sta RY
        lda #S2X+2
        sta RX
        lda #S2W-4
        sta RW
        lda #SUBRH
        sta RH
        lda S2I
        cmp S2H
        bne @n
        lda #NAVY
        bra @f
@n:     lda #GRAY
@f:     CALL setcolor
        CALL fillrect
        lda S2A
        clc
        adc S2I
        asl
        tax
        lda si_icon,x
        jsr ss_icon
        lda S2I
        cmp S2H
        bne @b
        lda #WHITE
        bra @c
@b:     lda #BLACK
@c:     CALL setcolor
        lda S2A
        clc
        adc S2I
        asl
        tax
        lda si_text,x
        ldy LANG
        beq *+5
        lda si_text_en,x
        JSLF xf_ui_str
        rts

; offene Gruppe neu zeichnen (mit dem Menue)
s2_redraw:
        lda S2G
        bne *+3
        rts
        jmp s2_frame

; Gruppe schliessen
s2_close:
        lda S2G
        bne @g
        rts
@g:     stz S2G
        stz S2F
        lda #S2X
        sta KL
        lda #320
        sta KR
        lda S2Y
        sta KT
        clc
        adc S2HT
        sta KB
        CALL redraw_rect
        rts

; Maus in der Gruppe? -> A = $4000 + Zeile, sonst $FFFF
s2_hit: lda S2G
        beq @no
        lda #S2X
        sta RX
        lda S2Y
        inc a
        inc a
        sta RY
        lda #S2W
        sta RW
        lda S2HT
        sec
        sbc #4
        sta RH
        CALL inrect
        bcc @no
        lda MY
        sec
        sbc RY
        ldx #SUBRH
        CALL gdiv
        cmp S2N
        bcs @no
        ora #$4000
        rts
@no:    lda #$FFFF
        rts

; Maus ueber Zeile A der Gruppe
s2_hov: cmp S2H
        beq @r
        ldx S2H
        sta S2H
        txa
        bmi @n
        sta S2I
        jsr s2_row
@n:     lda S2H
        sta S2I
        jsr s2_row
@r:     rts

; Tasten in der Gruppe
s2_key: lda KEYC
        cmp #$0A
        bne @k1
        lda S2H
        inc a
        cmp S2N
        bcc @set
        lda #0
        bra @set
@k1:    cmp #$0B
        bne @k2
        lda S2H
        dec a
        bpl @set
        lda S2N
        dec a
        bra @set
@k2:    cmp #$0D
        bne @k3
        lda S2H
        bmi @h
        jsr s2_action
@h:     sec
        rts
@k3:    cmp #$08                ; links / Esc: zurueck ins Untermenue
        beq @cl
        cmp #$1B
        bne @h
@cl:    jsr s2_close
        sec
        rts
@set:   pha
        CALL org0
        pla
        jsr s2_hov
        CALL orgr
        sec
        rts

; Eintrag A der offenen Gruppe starten
s2_action:
        clc
        adc S2A
        sta S2I
        stz S2G
        stz S2F
        stz SUBM
        stz MENU
        lda #$FFFF
        sta HOVER
        CALL redraw_menu
        lda S2I
        asl
        tax
        jmp (@t,x)
@t:     .addr @calc, @cd, @dos, @paint, @edit, @web, @term, @arc, @news, @wx, @mine, @sol
@calc:  JSLF xf_calc_open
        rts
@cd:    JSLF xf_cd_open
        rts
@dos:   JSLF xf_dos_open
        rts
@paint: JSLF xf_pt_open
        rts
@edit:  JSLF xf_ed_new
        rts
@web:   JSLF xf_web_open
        rts
@term:  JSLF xf_bbs_open
        rts
@arc:   JSLF xf_arc_open
        rts
@news:  JSLF xf_rss_open
        rts
@wx:    JSLF xf_wx_open
        rts
@mine:  JSLF xf_mine_new
        lda #20
        CALL win_open
        CALL redraw_all
        rts
@sol:   JSLF xf_sol_open
        rts

FARMODE .set 0
        .segment "CODE"
