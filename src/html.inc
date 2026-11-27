; ================================================================
; GS/OS95 Browser: HTML (Rohpuffer HR..HE) -> Text mit Steuerzeichen (ab WB_TXT)
; Teil von web.s
; ================================================================
        .pushseg
        .segment "BSS"
HSC:    .res 2                  ; Tag endet mit "/>"
UTGT:   .res 2                  ; Ziel fuer einen Attributwert (0 = keins)
UMAX:   .res 2
UOVF:   .res 2
UQT:    .res 2
        .popseg

h2t:    jsr h2t_init
        jmp h2t_lp
h2t_init:
        jsr net_ptr0
        jsr f_reset
        lda #WB_TXT
        sta HO
        stz HWS
        stz HNL
        stz HTN
        lda #1
        sta HSTART
        lda HPLAIN
        sta HPRE
        stz HSKIP
        stz HBOLD
        stz HLINK
        stz HLTXT
        stz HDEP
        stz HQ
        stz HTD
        stz HTITLE
        stz HFULL
        ldy #WB_TITLE
        lda #0
        BWR
        rts
; Ende des Puffers: bei HMORE zurueck (es kommt noch mehr), sonst abschliessen
h2t_end:
        lda HMORE
        beq h2t_fin
        rts
h2t_lp:
@l:     lda HR
        cmp HE
        bcc *+5
        jmp h2t_end
        lda HO
        cmp #WB_TXTE
        bcc *+5
        jmp @full
        lda HFULL               ; schon voll (vorheriges Stueck)
        beq *+5
        jmp h2t_end
        ldy HR
        BRD
        ldx HSKIP
        beq *+5
        jmp @skip
        ldx HPLAIN
        bne @pl
        cmp #'<'
        bne @n1
        jsr h_tag
        jmp @l
@n1:    cmp #'&'
        bne @pl
        jsr h_ent
        jmp @l
@pl:    cmp #' '+1
        bcs @ch
        inc HR
        jsr h_ws
        jmp @l
@ch:    cmp #$80
        bcs @hi
        inc HR
        jsr vis
        jmp @l
@hi:    jsr h_utf
        jsr emit_cp
        jmp @l
@skip:  jsr h_skip
        jmp @l
@full:  lda #1
        sta HFULL
h2t_fin:
        jsr link_end
        lda HBOLD
        beq @nb
        lda #$10
        jsr out_b
@nb:    lda HFULL
        beq @e
        lda #$0A                ; Hinweis "gekuerzt" (dafuer ist Platz frei)
        jsr out_f
        lda #$0A
        jsr out_f
        WS ws_cut
        sta PDST
        CBANK
        sta PDST+2
        lda [PDST]
        and #$00FF
        sta UN
        ldy #1
@c:     lda UN
        beq @e
        lda [PDST],y
        phy
        jsr out_f
        ply
        iny
        dec UN
        bra @c
@e:     lda HO
        sta TXTEND
        rts

; Byte A ausgeben (bis WB_TXTE)
out_b:  ldy HO
        cpy #WB_TXTE
        bcs @r
        BWR
        inc HO
@r:     rts
; auch in den Reservebereich (fuer den Schlusshinweis)
out_f:  ldy HO
        cpy #WB_FRM-2
        bcs @r
        BWR
        inc HO
@r:     rts

; Zeilenumbruch(e) vor dem naechsten sichtbaren Zeichen: A = 1 (neue Zeile) / 2 (Leerzeile)
block:  cmp HNL
        bcc @r
        sta HNL
@r:     rts

; anstehende Umbrueche / Leerzeichen ausgeben
flush:  lda HSTART
        bne @clr
        lda HNL
        beq @ws
        sec
        sbc HTN
        beq @clr
        bmi @clr
        sta HT
@nl:    lda #$0A
        jsr out_b
        dec HT
        bne @nl
        lda HNL
        sta HTN
@clr:   stz HNL
        stz HWS
        rts
@ws:    lda HWS
        beq @r
        stz HWS
        lda HTN
        bne @r
        lda #' '
        jmp out_b
@r:     rts

; sichtbares Zeichen A (interner Code)
vis:    sta HC
        lda HSEL                ; in einer Auswahl: nur Optionstext
        beq @ns
        lda HOPT
        beq @r
        lda HC
        jmp opt_add
@ns:    lda HTXA                ; in <textarea>: Feldwert
        beq @nt
        lda HC
        jmp txa_add
@nt:    lda HTITLE
        beq @n
        lda HC
        jmp t_add
@n:     jsr flush
        lda HC
        jsr out_b
        stz HTN
        stz HSTART
        lda HLINK
        beq @r
        lda #1
        sta HLTXT
@r:     rts

; Leerraum-Zeichen A
h_ws:   ldx HSEL
        beq @ns
        ldx HOPT
        beq @rr
        jmp opt_sp
@rr:    rts
@ns:    ldx HTXA
        beq @nx
        cmp #$0A                ; Zeilen in der Textarea: Leerzeichen
        bne @t1
        lda #' '
@t1:    cmp #' '
        bne @rr
        jmp txa_add
@nx:    ldx HTITLE
        beq @nt
        jmp t_sp
@nt:    ldx HPRE
        bne @pre
        lda #1
        sta HWS
        rts
@pre:   cmp #$0A
        bne @p1
        jmp h_nl
@p1:    cmp #$09
        bne @p2
        lda #' '
        jsr vis
        lda #' '
        jsr vis
        lda #' '
        jsr vis
        lda #' '
        jmp vis
@p2:    cmp #' '
        bne @r
        jmp vis
@r:     rts

; harter Zeilenumbruch (pre)
h_nl:   lda HSTART
        bne @r
        jsr flush
        lda #$0A
        jsr out_b
        lda HTN
        cmp #2
        bcs @r
        inc HTN
@r:     rts
; <br>
h_br:   jsr flush
        lda HSTART
        bne @r
        lda HTN
        cmp #2
        bcs @r
        lda #$0A
        jsr out_b
        inc HTN
@r:     rts

; Titel
t_add:  sta HC
        ldy #WB_TITLE
        BRD
        cmp #62
        bcs @r
        inc a
        BWR
        clc
        adc #WB_TITLE
        tay
        lda HC
        BWR
@r:     rts
t_sp:   ldy #WB_TITLE
        BRD
        beq @r
        clc
        adc #WB_TITLE
        tay
        BRD
        cmp #' '
        beq @r
        lda #' '
        jmp t_add
@r:     rts

; Byte aus einer Tabelle dieser Codebank: A = Tabelle, X = Index -> A
xtab:   sta PDST
        CBANK
        sta PDST+2
        txy
        lda [PDST],y
        and #$00FF
        rts

; Text dieser Codebank (Laengenbyte) als sichtbare Zeichen
h_xstr: sta PDST
        CBANK
        sta PDST+2
        lda [PDST]
        and #$00FF
        sta UN
        ldy #1
@l:     lda UN
        beq @r
        lda [PDST],y
        and #$00FF
        phy
        pha
        lda PDST                ; vis darf PDST nicht stoeren
        sta HV
        lda PDST+2
        sta HV+2
        pla
        jsr vis
        lda HV
        sta PDST
        lda HV+2
        sta PDST+2
        ply
        iny
        dec UN
        bra @l
@r:     rts

; ---------- Zeichen ab $80: UTF-8 oder Latin-1 -> A = Unicode, HR weiter ----------
h_utf:  ldy HR
        BRD
        sta HV
        cmp #$C2
        bcs *+5
        jmp @lat
        cmp #$F5
        bcs @lat
        cmp #$E0
        bcs @3
        iny                     ; 2 Bytes
        jsr @cont
        bcs @lat
        sta HV+2
        lda HV
        and #$001F
        xba
        lsr
        lsr                     ; << 6
        ora HV+2
        inc HR
        inc HR
        rts
@3:     cmp #$F0
        bcs @4
        iny
        jsr @cont
        bcs @lat
        sta HV+2
        iny
        jsr @cont
        bcs @lat
        sta HT
        lda HV
        and #$000F
        xba
        asl
        asl
        asl
        asl                     ; << 12
        sta HV
        lda HV+2
        xba
        lsr
        lsr                     ; << 6
        ora HV
        ora HT
        inc HR
        inc HR
        inc HR
        rts
@4:     iny
        jsr @cont
        bcs @lat
        iny
        jsr @cont
        bcs @lat
        iny
        jsr @cont
        bcs @lat
        lda #4
        clc
        adc HR
        sta HR
        lda #'?'
        rts
@lat:   inc HR
        lda HV
        rts
; Folgebyte bei Y? -> A = 6 Bit, C=0
@cont:  cpy HE
        bcs @no
        BRD
        and #$00C0
        cmp #$0080
        bne @no
        BRD
        and #$003F
        clc
        rts
@no:    sec
        rts

; Unicode A ausgeben
emit_cp:
        cmp #$80
        bcs @hi
        cmp #$20
        bcs @v
        jmp h_ws
@v:     jmp vis
@hi:    cmp #$A0
        bcs @l1
        sec
        sbc #$80
        tax
        lda #w_cp
        jsr xtab
        bra @o
@l1:    cmp #$0100
        bcs @u
        sec
        sbc #$A0
        tax
        lda #w_lat1
        jsr xtab
@o:     cmp #0
        beq @r
        jmp vis
@r:     rts
@u:     sta HV                  ; Bereiche: von, bis, Zeichen
        ldx #0
@t:     lda w_umap,x
        beq @q
        lda HV
        cmp w_umap,x
        bcc @nx
        cmp w_umap+2,x
        beq @hit
        bcs @nx
@hit:   lda w_umap+4,x
        beq @r2
        cmp #$FF
        bne @v1
        lda #'.'
        jsr vis
        lda #'.'
        jsr vis
        lda #'.'
@v1:    jmp vis
@nx:    txa
        clc
        adc #6
        tax
        bra @t
@q:     lda #'?'
        jmp vis
@r2:    rts

        .pushseg
        .segment "RODATA"
; Unicode-Bereiche: von, bis, Zeichen (0 = nichts, $FF = "...")
w_umap: .word $2010,$2015,'-'
        .word $2018,$201B,$27
        .word $201C,$201F,'"'
        .word $2022,$2023,$87
        .word $2026,$2026,$FF
        .word $2002,$200A,' '
        .word $200B,$200F,0
        .word $2039,$2039,'<'
        .word $203A,$203A,'>'
        .word $2060,$2060,0
        .word $20AC,$20AC,'E'
        .word $2122,$2122,'T'
        .word $2190,$2190,'<'
        .word $2192,$2192,'>'
        .word $25CF,$25CF,$87
        .word $25AA,$25AA,$87
        .word $FEFF,$FEFF,0
        .word 0
        .popseg

; ---------- Entitaet ab HR ('&') ----------
h_ent:  ldy HR
        iny
        stz TNAME
@r:     cpy HE
        bcs @end
        BRD
        cmp #';'
        beq @semi
        cmp #'#'
        beq @ok
        cmp #'0'
        bcc @end
        cmp #'9'+1
        bcc @ok
        cmp #'A'
        bcc @end
        cmp #'Z'+1
        bcc @ok
        cmp #'a'
        bcc @end
        cmp #'z'+1
        bcs @end
@ok:    sta HT
        lda TNAME
        and #$00FF
        cmp #9
        bcs @end
        inc a
        tax
        sep #$20
        sta TNAME
        lda HT
        sta TNAME,x
        rep #$20
        iny
        bra @r
@semi:  iny
@end:   lda TNAME
        and #$00FF
        sta UT_N
        beq @lit
        sty UK                  ; hinter der Entitaet
        lda TNAME+1
        and #$00FF
        cmp #'#'
        beq @num
        lda #w_ents
        jsr tbl_find
        bcs @lit
        ldx UK
        stx HR
        cmp #0
        beq @rr
        jmp vis
@rr:    rts
@lit:   inc HR
        lda #'&'
        jmp vis
@num:   stz HV                  ; &#123; / &#x1F;
        ldx #2
        lda TNAME+2
        and #$00DF
        cmp #'X'
        bne @dec
        inx
@hx:    cpx UT_N
        beq @hx1
        bcs @nd
@hx1:   lda TNAME,x
        and #$00FF
        jsr lower
        sec
        sbc #'0'
        cmp #10
        bcc @hd
        sbc #'a'-'0'-10
        cmp #16
        bcs @lit
@hd:    sta HT
        lda HV
        asl
        asl
        asl
        asl
        ora HT
        sta HV
        inx
        bra @hx
@dec:   cpx UT_N
        beq @d1
        bcs @nd
@d1:    lda TNAME,x
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcs @lit
        sta HT
        lda HV
        asl
        asl
        clc
        adc HV
        asl
        clc
        adc HT
        sta HV
        inx
        bra @dec
@nd:    lda UK
        sta HR
        lda HV
        jmp emit_cp

; Tabelle A (dieser Codebank: Laenge, Name, Wert) nach TNAME durchsuchen
; C=0 gefunden, A = Wert
tbl_find:
        sta PDST
        CBANK
        sta PDST+2
        lda TNAME
        and #$00FF
        sta UT_N
@e:     lda [PDST]
        and #$00FF
        beq @no
        sta HT
        cmp UT_N
        bne @nx
        ldy #1
@c:     lda [PDST],y
        eor TNAME,y
        and #$00FF
        bne @nx
        iny
        cpy HT
        bcc @c
        beq @c
        lda [PDST],y
        and #$00FF
        clc
        rts
@nx:    lda HT
        clc
        adc #2
        adc PDST
        sta PDST
        bra @e
@no:    sec
        rts

; ---------- in <script>/<style>...: bis zum passenden End-Tag ----------
h_skip: ldy HR
        BRD
        cmp #'<'
        bne @n
        iny
        BRD
        cmp #'/'
        bne @n
        iny
        lda SKNAME
        and #$00FF
        sta UT_N
        ldx #1
@c:     cpx UT_N
        beq @c1
        bcs @end
@c1:    BRD
        jsr lower
        sta HT
        lda SKNAME,x
        and #$00FF
        cmp HT
        bne @n
        iny
        inx
        bra @c
@end:   stz HSKIP               ; End-Tag normal lesen
        rts
@n:     inc HR
        rts

; ---------- Tag ab HR ('<') ----------
h_tag:  ldy HR
        iny
        cpy HE
        bcc *+5
        jmp @lit
        BRD
        cmp #'!'
        beq @cmt
        cmp #'?'
        bne @t0
        jmp @gt
@cmt:   iny                     ; <!-- ... -->
        BRD
        cmp #'-'
        beq *+5
        jmp @gt
        iny
        BRD
        cmp #'-'
        beq *+5
        jmp @gt
@cm:    iny
        cpy HE
        bcc *+5
        jmp @eof
        BRD
        cmp #'-'
        bne @cm
        iny
        BRD
        cmp #'-'
        bne @cm
        iny
        BRD
        cmp #'>'
        bne @cm
        iny
        sty HR
        rts
@t0:    stz HCL
        cmp #'/'
        bne @t1
        inc HCL
        iny
        BRD
@t1:    jsr lower
        cmp #'a'
        bcc @lit
        cmp #'z'+1
        bcs @lit
        stz TNAME
@nm:    cpy HE
        bcs @ne
        BRD
        jsr lower
        cmp #'0'
        bcc @ne
        cmp #'9'+1
        bcc @ad
        cmp #'a'
        bcc @ne
        cmp #'z'+1
        bcs @ne
@ad:    sta HT
        lda TNAME
        and #$00FF
        cmp #10
        bcs @sk
        inc a
        tax
        sep #$20
        sta TNAME
        lda HT
        sta TNAME,x
        rep #$20
@sk:    iny
        bra @nm
@ne:    sty HR
        lda #w_tags
        jsr tbl_find
        bcc @f
        lda #0
@f:     sta HACT
        jsr h_attrs
        lda HCL
        beq @open
        jmp h_close
@open:  jmp h_open
@lit:   inc HR
        lda #'<'
        jmp vis
@gt:    cpy HE                  ; bis '>' ueberspringen
        bcs @eof
        BRD
        iny
        cmp #'>'
        bne @gt
        sty HR
        rts
@eof:   lda HE
        sta HR
        rts

; ---------- Attribute bis '>' ----------
h_attrs:
        stz HSC
        stz HCHK
        ldy #WB_HREF
        lda #0
        BWR
        ldy #WB_ALT
        BWR
        ldy #WB_ITYP
        BWR
        ldy #WB_CTYP
        BWR
        ldy HR
@sw:    cpy HE
        bcc *+5
        jmp @end
        BRD
        cmp #' '+1
        bcs @n
        iny
        bra @sw
@n:     cmp #'>'
        bne @n1
        iny
        sty HR
        rts
@n1:    cmp #'/'
        bne @n2
        lda #1
        sta HSC
        iny
        bra @sw
@n2:    stz ANAME               ; Name
@an:    cpy HE
        bcc *+5
        jmp @end
        BRD
        cmp #'='
        beq @ae
        cmp #'>'
        beq @ae
        cmp #' '+1
        bcc @ae
        jsr lower
        sta HT
        lda ANAME
        and #$00FF
        cmp #8
        bcs @an1
        inc a
        tax
        sep #$20
        sta ANAME
        lda HT
        sta ANAME,x
        rep #$20
@an1:   iny
        bra @an
@ae:    cpy HE                  ; Leerraum, dann '='?
        bcs @end
        BRD
        cmp #' '+1
        bcs @ae1
        iny
        bra @ae
@ae1:   cmp #'='
        beq @ae3
        phy                     ; Attribut ohne Wert: "checked"?
        ldx #fa_chk
        jsr an_eq
        bcc @ae5
        ldx #fa_sel             ; oder "selected"
        jsr an_eq
@ae5:   ply
        bcs @ae4
        lda #1
        sta HCHK
@ae4:   jmp @sw
@ae3:
        iny
@ae2:   cpy HE
        bcs @end
        BRD
        cmp #' '+1
        bcs @val
        iny
        bra @ae2
@val:   stz UQT
        cmp #'"'
        beq @q
        cmp #$27
        bne @nq
@q:     sta UQT
        iny
@nq:    jsr at_target
@vl:    cpy HE
        bcs @end
        BRD
        ldx UQT
        beq @uq
        cmp UQT
        beq @vq
        bra @vc
@uq:    cmp #' '+1
        bcc @sw2
        cmp #'>'
        beq @sw2
@vc:    jsr at_put
        iny
        bra @vl
@vq:    iny
@sw2:   jmp @sw
@end:   sty HR
        rts

; Ziel fuer den Wert: href bei <a>, alt bei <img>
at_target:
        stz UTGT
        stz UOVF
        lda HCL
        bne @r
        jsr at_form             ; form/input/button
        bcc @nf
        rts
@nf:
        lda HACT
        cmp #A_A
        bne @i
        lda ANAME
        and #$00FF
        cmp #4
        bne @r
        lda ANAME+1
        cmp #$7268              ; "hr"
        bne @r
        lda ANAME+3
        cmp #$6665              ; "ef"
        bne @r
        lda #WB_HREF
        sta UTGT
        lda #190
        sta UMAX
        rts
@i:     cmp #A_IMG
        bne @r
        lda ANAME
        and #$00FF
        cmp #3
        bne @r
        lda ANAME+1
        cmp #$6C61              ; "al"
        bne @src
        bne @r
        lda ANAME+3
        and #$00FF
        cmp #'t'
        bne @r
        lda #WB_ALT
        sta UTGT
        lda #60
        sta UMAX
@r:     rts
@src:   lda ANAME+1             ; "src" -> WB_HREF (fuer GIF-Links)
        cmp #$7273              ; "sr"
        bne @r
        lda ANAME+3
        and #$00FF
        cmp #'c'
        bne @r
        lda #WB_HREF
        sta UTGT
        lda #190
        sta UMAX
        rts

; Zeichen A des Werts anhaengen (&amp; -> &), Y bleibt
at_put: ldx UTGT
        bne @go
        rts
@go:    sta HC
        phy
        cmp #'&'                ; &amp; &lt; &gt; &quot; &apos; &nbsp; (auch ohne ';')
        bne @p
        jsr at_ent
        bcs @p
        sta HC
        ply                     ; Entitaet ueberspringen
        ldy ATE
        phy
@p:     ldy UTGT
        BRD
        cmp UMAX
        bcc @ok
        lda #1                  ; zu lang
        sta UOVF
        ldy UTGT
        lda #0
        BWR
        stz UTGT
        bra @r
@ok:    inc a
        BWR
        clc
        adc UTGT
        tay
        lda HC
        BWR
@r:     ply
        rts

; Entitaet ab Y ('&') in einem Attribut: C=0, A = Zeichen, ATE = letztes Zeichen
at_ent: stz ATN
        stz ATN+2
        stz ATN+4
        ldx #0
@n:     iny
        BRD
        ora #$20
        cmp #'a'
        bcc @s
        cmp #'z'+1
        bcs @s
        cpx #5
        bcs @no
        sep #$20
        sta ATN,x
        rep #$20
        inx
        bra @n
@s:     dey                     ; Y = letzter Buchstabe
        sty ATE
        iny
        BRD
        cmp #';'
        bne @t
        sty ATE
@t:     ldx #0                  ; Tabelle: 6 Bytes Name, 1 Byte Zeichen... (8 je Eintrag)
@l:     lda at_etab,x
        beq @no
        cmp ATN
        bne @nx
        lda at_etab+2,x
        cmp ATN+2
        bne @nx
        lda at_etab+4,x
        cmp ATN+4
        bne @nx
        lda at_etab+6,x
        and #$00FF
        clc
        rts
@nx:    txa
        clc
        adc #8
        tax
        bra @l
@no:    sec
        rts
        .pushseg
        .segment "RODATA"
at_etab: .byte "amp",0,0,0,"&",0
        .byte "lt",0,0,0,0,"<",0
        .byte "gt",0,0,0,0,">",0
        .byte "quot",0,0,$22,0
        .byte "apos",0,0,"'",0
        .byte "nbsp",0,0," ",0
        .byte "auml",0,0,$80,0
        .byte "ouml",0,0,$81,0
        .byte "uuml",0,0,$82,0
        .byte "szlig",0,$86,0
        .word 0
        .popseg
        .pushseg
        .segment "BSS"
ATN:    .res 6
ATE:    .res 2
        .popseg

; Rand (Zitat + Liste) ausgeben
mg_out: lda HQ
        clc
        adc HDEP
        sta HT
        asl
        clc
        adc HT                  ; *3
        cmp #16
        bcc @c
        lda #15
@c:     sta HT
        lda #$14
        jsr out_b
        lda HT
        jmp out_b

bold_on:
        inc HBOLD
        lda HBOLD
        cmp #1
        bne @r
        lda #$11
        jmp out_b
@r:     rts
bold_off:
        lda HBOLD
        beq @r
        dec HBOLD
        bne @r
        lda #$10
        jmp out_b
@r:     rts

link_end:
        lda HLINK
        bne @go
        rts
@go:    lda HLTXT
        bne @e
        WS ws_link
        jsr h_xstr
@e:     lda #$02
        jsr out_b
        stz HLINK
        rts

; ---------- oeffnende Tags ----------
h_open: lda HACT
        asl
        tax
        jmp (h_otab,x)
h_otab: .word ho_none, ho_blk1, ho_p, ho_h, ho_br, ho_hr, ho_pre, ho_a, ho_b
        .word ho_ul, ho_ol, ho_li, ho_tr, ho_td, ho_title, ho_skip, ho_img, ho_bq
        .word ho_form, ho_input, ho_button, ho_select, ho_option, ho_textarea, ho_table
h_ctab: .word ho_none, ho_blk1, ho_p, hc_h, ho_none, ho_none, hc_pre, hc_a, hc_b
        .word hc_ul, hc_ul, ho_none, ho_blk1, ho_none, hc_title, ho_none, ho_none, hc_bq
        .word hc_form, ho_none, ho_none, hc_select, hc_option, hc_textarea, hc_table
ho_none:
        rts
ho_blk1:
        lda #1
        jmp block
ho_p:   lda #2
        jmp block
ho_h:   lda #2
        jsr block
        jmp bold_on
ho_br:  jmp h_br
ho_hr:  lda #1
        jsr block
        jsr flush
        lda #$16
        jsr out_b
        lda #1
        sta HTN
        stz HSTART
        rts
ho_pre: lda #2
        jsr block
        inc HPRE
        rts
ho_a:   jsr link_end
        ldy #WB_HREF
        BRD
        bne @go
        rts
@go:    sta UN
        clc
        adc #8                  ; Platz?
        adc HO
        cmp #WB_TXTE
        bcc @ok
        rts
@ok:    jsr flush
        lda #$01
        jsr out_b
        lda UN
        jsr out_b
        ldx #0
@c:     txa
        clc
        adc #WB_HREF+1
        tay
        BRD
        phx
        jsr out_b
        plx
        inx
        cpx UN
        bcc @c
        lda #1
        sta HLINK
        stz HLTXT
        rts
ho_b:   jmp bold_on
ho_ul:  lda #0
        bra ho_list
ho_ol:  lda #1
ho_list:
        sta HT
        ldx HDEP
        lda #1
        cpx #0
        bne @b
        lda #2
@b:     jsr block
        lda HDEP
        cmp #4
        bcs @d
        asl
        tax
        lda HT
        sta HOLT,x
        stz HOLN,x
@d:     inc HDEP
        rts
ho_li:  lda #1
        jsr block
        jsr flush
        lda HDEP
        bne @in
        inc HDEP                ; <li> ausserhalb einer Liste
        jsr @mk
        dec HDEP
        rts
@in:    jsr @mk
        rts
@mk:    lda HQ                  ; Rand fuer den Punkt
        clc
        adc HDEP
        sta HT
        asl
        clc
        adc HT
        dec a
        dec a
        cmp #14
        bcc @c
        lda #13
@c:     pha
        lda #$14
        jsr out_b
        pla
        jsr out_b
        lda HDEP
        dec a
        cmp #3
        bcc @lv
        lda #3
@lv:    asl
        tax
        lda HOLT,x
        beq @dot
        inc HOLN,x              ; Nummer
        lda HOLN,x
        cmp #10
        bcc @one
        ldy #0
@tn:    cmp #10
        bcc @t1
        sbc #10
        iny
        bra @tn
@t1:    pha
        tya
        clc
        adc #'0'
        jsr vis
        pla
@one:   clc
        adc #'0'
        jsr vis
        lda #'.'
        jsr vis
        bra @sp
@dot:   lda #$87
        jsr vis
@sp:    lda #' '
        jsr vis
        jmp mg_out
ho_tr:  lda #1
        jsr block
        stz HTD
        lda #$1E                ; Zeilenanfang (fuer die Spaltenbreiten)
        jmp out_b
; Tabellen: $1C t ... $1D t (t = Nummer 0..15, $FF = innere/zu viele)
ho_table:
        lda #1
        jsr block
        stz HTD
        inc HTDEP
        lda #$1C
        jsr out_b
        lda #$FF
        ldx HTDEP
        cpx #1
        bne @o
        lda HTBL
        cmp #16
        bcc @n
        lda #$FF
        bra @s
@n:     inc HTBL
@s:     sta HTOUT
@o:     jmp out_b
hc_table:
        lda HTDEP
        bne @go
        rts
@go:    jsr flush
        lda #$1D
        jsr out_b
        lda #$FF
        ldx HTDEP
        cpx #1
        bne @o
        lda HTOUT
@o:     jsr out_b
        dec HTDEP
        lda #1
        jmp block
ho_td:  lda HTD
        beq @f
        jsr flush
        lda #$17                ; naechste Spalte: $17 Spalte Breite (traegt lay_all ein)
        jsr out_b
        lda HTD
        ldx HTDEP
        cpx #2
        bcc @c
        lda #$FF                ; innere Tabelle: nur Abstand
@c:     jsr out_b
        lda #0
        jsr out_b
        stz HTN
        stz HSTART
@f:     inc HTD
        rts
ho_title:
        lda HSC
        bne @r
        lda #1
        sta HTITLE
        ldy #WB_TITLE
        lda #0
        BWR
@r:     rts
ho_skip:
        lda HSC
        bne @r
        lda #1
        sta HSKIP
        ldx #10
@c:     lda TNAME,x
        sta SKNAME,x
        dex
        dex
        bpl @c
@r:     rts
ho_img: jsr img_gif              ; GIF: in die Seite holen
        bcs @ng
        jsr img_reg
        bcs @ng
        lda HLINK               ; ausserhalb eines Links: kein [Bild] mehr noetig
        bne @ng
        rts
@ng:    lda HLINK                ; GIF ausserhalb eines Links: als Link zum Bild
        bne @nl
        jsr img_gif
        bcs @nl
        jsr ho_a
        lda HLINK
        beq @nl
        jsr @nl
        lda HLTXT
        bne @lt
        WS ws_img
        jsr h_xstr
@lt:    jmp link_end
@nl:    ldy #WB_ALT
        BRD
        beq @noalt
        sta UN
        lda #'['
        jsr vis
        lda #WB_ALT+1
        sta UK
@l:     lda UN
        beq @e
        ldy UK
        BRD
        jsr emit_cp
        inc UK
        dec UN
        bra @l
@e:     lda #']'
        jmp vis
@noalt: lda HLINK
        beq @r
        lda HLTXT
        bne @r
        WS ws_img
        jmp h_xstr
@r:     rts
ho_bq:  lda #1
        jsr block
        inc HQ
        jmp mg_out

; ---------- schliessende Tags ----------
h_close:
        lda HACT
        asl
        tax
        jmp (h_ctab,x)
hc_h:   jsr bold_off
        lda #2
        jmp block
hc_pre: lda HPRE
        beq @b
        ldx HPLAIN
        bne @b
        dec HPRE
@b:     lda #2
        jmp block
hc_a:   jmp link_end
hc_b:   jmp bold_off
hc_ul:  lda HDEP
        beq @r
        dec HDEP
        lda #1
        ldx HDEP
        bne @b
        lda #2
@b:     jsr block
        jmp mg_out
@r:     rts
hc_title:
        stz HTITLE
        rts
hc_bq:  lda HQ
        beq @r
        dec HQ
        lda #1
        jsr block
        jmp mg_out
@r:     rts
