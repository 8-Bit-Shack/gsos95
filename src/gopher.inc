; ================================================================
; GS/OS95 Browser: Gopher (Teil von web.s)
; gopher://rechner[:port]/Tselektor  (T = Typ: 1 Menue, 0 Text, 7 Suche, sonst Datei)
; Suchanfragen: Selektor %09 Suchtext
; ================================================================
        .pushseg
        .segment "BSS"
GTYPE:  .res 2
GQRY:   .res 2                  ; Selektor enthaelt einen Tabulator (Suche)
GP:     .res 2
GE:     .res 2
GT0:    .res 2                  ; Tabulatoren
GT1:    .res 2
GT2:    .res 2
GT3:    .res 2
GNT:    .res 2
GC:     .res 2
        .popseg

; Anfrage: Selektor (dekodiert) + CR LF nach WB_REQ
gop_req:
        lda #WB_REQ
        sta UWR
        lda #'1'
        sta GTYPE
        stz GQRY
        ldy #WB_PATH
        BRD
        sta GNT
        cmp #2
        bcc @end                ; "/" oder leer: Hauptmenue
        ldy #WB_PATH+2
        BRD
        sta GTYPE
        ldy #WB_PATH+3
        lda GNT
        clc
        adc #WB_PATH+1
        sta GE
@l:     cpy GE
        bcs @end
        BRD
        cmp #'%'
        bne @c
        iny                     ; %XX
        BRD
        jsr g_hex
        asl
        asl
        asl
        asl
        sta GC
        iny
        BRD
        jsr g_hex
        ora GC
        cmp #9
        bne @c
        inc GQRY
@c:     ldx GQRY                ; im Suchtext: '+' = Leerzeichen
        beq @c2
        cmp #'+'
        bne @c2
        lda #' '
@c2:    phy
        jsr rq_c
        ply
        iny
        bra @l
@end:   lda #13
        jsr rq_c
        lda #10
        jsr rq_c
        lda UWR
        sec
        sbc #WB_REQ
        sta WCNT
        lda #WB_REQ
        sta WPTR
        clc
        rts
g_hex:  and #$00FF
        cmp #'A'
        bcc @d
        and #$00DF
        sec
        sbc #'A'-10
        and #$000F
        rts
@d:     sec
        sbc #'0'
        and #$000F
        rts

; Antwort im Rohpuffer (0..WPTR) anzeigen oder speichern
gop_page:
        jsr net_ptr0
        lda WPTR
        sta RAWEND
        sta BEND
        stz HEND
        stz HCHUNK
        lda #200
        sta HCODE
        lda #1
        sta HTYPOK
        stz HPLAIN
        lda GTYPE
        and #$00FF
        cmp #'1'
        bne *+5
        jmp @menu
        cmp #'7'
        bne @n7
        lda GQRY
        bne @menu
        jsr g_close
        jmp gop_search
@n7:    cmp #'g'                ; GIF: anzeigen, wenn es ganz da ist
        bne @n8
        lda NFULL
        bne @n9
        jmp gif_show
@n8:    cmp #'0'
        beq @txt
        cmp #'h'
        beq @html
@n9:    stz HTYPOK              ; Datei: herunterladen
        jmp dl_run
@txt:   inc HPLAIN
        lda BEND                ; Schlusszeile "." weglassen
        cmp #3
        bcc @html
        sec
        sbc #3
        tay
        BRD
        cmp #'.'
        bne @html
        iny
        BRD
        cmp #13
        bne @html
        cpy #4
        bcc @cut
        dey
        dey
        BRD
        cmp #10
        bne @html
@cut:   lda BEND
        sec
        sbc #3
        sta BEND
@html:  jsr g_close
        stz HR
        lda BEND
        sta HE
        jsr h2t
        jsr g_title
        jmp page_done
@menu:  jsr g_close
        jsr gop_menu
        jsr g_title
        jmp page_done
g_close:
        lda NFULL
        beq @r
        JSLF xf_tcp_close
        stz NFULL
@r:     rts
; Titel: Rechnername
g_title:
        ldy #WB_TITLE
        BRD
        bne @r
        ldx #WB_HOST
        ldy #WB_TITLE
        jmp bs_copy
@r:     rts

; Suchseite (Typ 7 ohne Suchtext)
gop_search:
        jsr net_ptr0
        stz HO
        lda #w_gs1
        jsr raw_m
        WS ws_gsrch
        jsr raw_x
        lda #w_gs2
        jsr raw_m
        WS ws_gsrch
        jsr raw_x
        lda #w_gs3
        jsr raw_m
        ldx #WB_TURL
        jsr raw_b
        lda #w_gs4
        jsr raw_m
        lda HO
        sta HE
        stz HR
        jsr h2t
        jmp page_done

; Menue -> Textstrom
gop_menu:
        jsr h2t_init
        lda #1
        sta HPRE
        stz HR
@line:  lda HR
        cmp BEND
        bcc *+5
        jmp @fin
        lda HO
        cmp #WB_TXTE-256
        bcc *+5
        jmp @full
        sta GP
        lda HR                  ; Zeilenende und Tabulatoren suchen
        sta GP
        stz GNT
        tay
@f:     cpy BEND
        bcs @fe
        BRD
        cmp #13
        beq @fe
        cmp #10
        beq @fe
        cmp #9
        bne @f1
        ldx GNT
        cpx #8
        bcs @f1
        tya
        sta GT0,x
        inx
        inx
        stx GNT
@f1:    iny
        bra @f
@fe:    sty GE
        ldy GP                  ; "." allein: Ende
        BRD
        sta GTYPE
        cmp #'.'
        bne @nd
        lda GE
        sec
        sbc GP
        cmp #1
        bne @nd
        jmp @fin
@nd:    lda GE                  ; leere Zeile
        cmp GP
        bne *+5
        jmp @next
        lda GNT
        cmp #6                  ; mindestens Text, Selektor, Rechner
        bcc @info
        lda GTYPE
        cmp #'i'
        beq @info
        cmp #'3'
        beq @info
        jsr g_href
        bcs @info
        lda GTYPE               ; Kennzeichen vor dem Link
        ldx #0
@mk:    lda g_mark,x
        and #$00FF
        beq @md
        cmp GTYPE
        beq @md
        inx
        inx
        inx
        bra @mk
@md:    lda g_mark+1,x
        and #$00FF
        jsr vis
        lda g_mark+2,x
        and #$00FF
        jsr vis
        jsr ho_a
        lda GT0
        jsr g_text
        jsr link_end
        bra @nl
@info:  lda GNT
        bne @i1
        lda GE
        bra @i2
@i1:    lda GT0
@i2:    jsr g_text
@nl:    lda #$0A
        jsr h_ws
@next:  ldy GE                  ; CR/LF ueberspringen
@sk:    cpy BEND
        bcs @sn
        BRD
        cmp #13
        beq @s1
        cmp #10
        bne @sn
@s1:    iny
        bra @sk
@sn:    sty HR
        jmp @line
@full:  lda #1
        sta HFULL
@fin:   jmp h2t_fin

; Anzeigetext GP+1 .. A
g_text: sta GC
        lda GP
        inc a
        sta HR
@l:     lda HR
        cmp GC
        bcs @r
        tay
        BRD
        cmp #$80
        bcs @hi
        inc HR
        cmp #' '
        bcs @v
        lda #' '
@v:     jsr vis
        bra @l
@hi:    jsr h_utf
        jsr emit_cp
        bra @l
@r:     rts

; Link-Ziel nach WB_HREF. C=1 wenn es nicht passt
g_href: ldy #WB_HREF
        lda #0
        BWR
        lda GTYPE
        cmp #'h'                ; "URL:..." -> direkter Link
        bne @g
        ldy GT0
        iny
        BRD
        cmp #'U'
        bne @g
        lda GT0
        clc
        adc #5
        tay
@u:     cpy GT1
        bcc *+5
        jmp @ok
        BRD
        jsr g_hc
        bcc *+5
        jmp @no
        iny
        bra @u
@g:     ldx #0
@p:     lda u_preg+1,x
        and #$00FF
        phx
        jsr g_hc
        plx
        inx
        cpx #9
        bcc @p
        ldy GT1                 ; Rechner
@h:     iny
        cpy GT2
        bcs @hp
        BRD
        jsr g_hc
        bcc *+5
        jmp @no
        bra @h
@hp:    ldy GT2                 ; Port, wenn nicht 70
        iny
        BRD
        cmp #'7'
        bne @port
        iny
        BRD
        cmp #'0'
        bne @port
        iny
        cpy GE
        bcs @sel
        BRD
        cmp #9
        beq @sel
        cmp #'0'
        bcc @sel
        cmp #'9'+1
        bcc @port
        bra @sel
@port:  lda #':'
        jsr g_hc
        ldy GT2
@pl:    iny
        cpy GE
        bcs @sel
        BRD
        cmp #'0'
        bcc @sel
        cmp #'9'+1
        bcs @sel
        jsr g_hc
        bra @pl
@sel:   lda #'/'
        jsr g_hc
        lda GTYPE
        jsr g_hc
        ldy GT0
@s:     iny
        cpy GT1
        bcs @ok
        BRD
        cmp #' '+1
        bcc @esc
        cmp #'%'
        beq @esc
        cmp #'#'
        beq @esc
        cmp #$7F
        bcs @esc
        jsr g_hc
        bcs @no
        bra @s
@esc:   pha
        lda #'%'
        jsr g_hc
        pla
        pha
        lsr
        lsr
        lsr
        lsr
        and #$000F
        jsr u_hexd
        jsr g_hc
        pla
        and #$000F
        jsr u_hexd
        jsr g_hc
        bcs @no
        bra @s
@ok:    clc
        rts
@no:    sec
        rts
; Zeichen A an WB_HREF (hoechstens 180). C=1 wenn voll; Y bleibt
g_hc:   sta GC
        phy
        ldy #WB_HREF
        BRD
        cmp #180
        bcs @f
        inc a
        BWR
        clc
        adc #WB_HREF
        tay
        lda GC
        BWR
        ply
        clc
        rts
@f:     ply
        sec
        rts

        .pushseg
        .segment "RODATA"
; Typ, zwei Zeichen davor
g_mark: .byte '1',"> "
        .byte '0',"- "
        .byte '7',"? "
        .byte 'h',"@ "
        .byte 0,"* "
w_gs1:  .byte w_gs1e-w_gs1-1,"<title>"
w_gs1e:
w_gs2:  .byte w_gs2e-w_gs2-1,"</title><h1>"
w_gs2e:
w_gs3:  .byte w_gs3e-w_gs3-1,"</h1><form method=gopher action=",$22
w_gs3e:
w_gs4:  .byte w_gs4e-w_gs4-1,$22,"><input name=q> <input type=submit></form>"
w_gs4e:
        .popseg
