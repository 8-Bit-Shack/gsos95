; ================================================================
; GS/OS95 Browser: Formulare (Teil von web.s)
; Textstrom: $18 id = Eingabefeld, $19 id = Absenden-Knopf
; Formulare ab WB_FRM (4 x 132: Methode, Ziel), Felder ab WB_FLD (14 x 100:
; Art, Formular, Name (1+31), Wert (1+63)). Art: 1 Text, 2 versteckt,
; 3 Absenden, 4 angekreuzt. Methode: 0 GET, 1 POST, 2 Gopher-Suche
; ================================================================
WB_FRM  = $C000
WB_FLD  = $C210
WB_ITYP = $FD60                 ; type/method eines Tags (Laenge + 14)
FRMMAX  = 4
FLDMAX  = 14
FLDW    = 96                    ; Breite eines Eingabefelds
F_NAME  = 2
F_VAL   = 34

        .pushseg
        .segment "BSS"
HNF:    .res 2                  ; Formulare
HNFLD:  .res 2                  ; Felder
HFORM:  .res 2                  ; aktuelles Formular ($FF = keins)
HCHK:   .res 2                  ; Attribut "checked"
FFOC:   .res 2                  ; Feld mit Schreibmarke ($FFFF = keins)
FSUB:   .res 2
FFRM:   .res 2
FI:     .res 2
FFIRST: .res 2
QPOST:  .res 2
WPOST:  .res 2                  ; naechste Anfrage ist POST (Inhalt in WB_HREF)
HITK:   .res 2                  ; Treffer: 0 Link, 1 Feld, 2 Knopf
FX0:    .res 2
FWN:    .res 2
FR0:    .res 2
FR1:    .res 2
FWW:    .res 2
        .popseg

; Feld A -> A = Offset des Datensatzes
fld_rec:
        and #$00FF
        sta FR0
        asl
        asl
        sta FR1                 ; *4
        asl
        asl
        asl
        clc
        adc FR1                 ; *36
        sta FR1
        lda FR0
        xba
        lsr
        lsr                     ; *64
        clc
        adc FR1                 ; *100
        clc
        adc #WB_FLD
        rts

; Breite von Feld/Knopf A (X = Steuercode $18/$19) -> A
fld_w:  cpx #$18
        bne @b
        lda #FLDW+2
        rts
@b:     jsr fld_rec
        clc
        adc #F_VAL
        tay
        BRD
        sta FWN
        stz FWW
@l:     lda FWN
        beq @e
        iny
        BRD
        phy
        jsr cw
        ply
        clc
        adc FWW
        sta FWW
        dec FWN
        bra @l
@e:     lda FWW
        clc
        adc #12
        rts

; ---------- beim Lesen des HTML ----------
f_reset:
        stz HNF
        stz HNFLD
        stz HSEL
        stz HOPT
        stz OPTN
        stz HTXA
        stz HTDEP
        stz HTBL
        stz IMGN
        lda #$FF
        sta HFORM
        lda #$FFFF
        sta FFOC
        lda #$FFFF
        sta FCUR
        rts

; Attributziele fuer form/input/button (aus at_target): C=1 wenn behandelt
at_form:
        lda HACT
        cmp #A_FORM
        beq @go
        cmp #A_INPUT
        beq @go
        cmp #A_BUTTON
        beq @go
        cmp #A_SELECT
        beq @go
        cmp #A_OPTION
        beq @go
        cmp #A_TEXTAREA
        beq @go
        clc
        rts
@go:    phy
        jsr at_form2
        ply
        sec
        rts
at_form2:
        ldx #fa_action
        jsr an_eq
        bcs @n1
        lda #WB_HREF
        ldx #127
        bra @set
@n1:    ldx #fa_method
        jsr an_eq
        bcc @ty
        ldx #fa_type
        jsr an_eq
        bcs @n2
@ty:    lda #WB_ITYP
        ldx #14
        bra @set
@n2:    ldx #fa_name
        jsr an_eq
        bcs @n3
        lda #WB_CTYP
        ldx #30
        bra @set
@n3:    ldx #fa_value
        jsr an_eq
        bcs @no
        lda #WB_ALT
        ldx #60
@set:   sta UTGT
        stx UMAX
@no:    rts
        .pushseg
        .segment "RODATA"
fa_action: .byte 6,"action"
fa_method: .byte 6,"method"
fa_type:   .byte 4,"type"
fa_name:   .byte 4,"name"
fa_value:  .byte 5,"value"
fa_chk:    .byte 7,"checked"
fa_sel:    .byte 8,"selected"
        .popseg

; ANAME gleich Text X (Hauptbank)? C=0 ja
an_eq:  stx FX0
        lda a:0,x
        eor ANAME
        and #$00FF
        bne @no
        lda a:0,x
        and #$00FF
        sta FI
        ldy #1
@l:     cpy FI
        beq @c
        bcs @y
@c:     tya
        clc
        adc FX0
        tax
        lda a:0,x
        eor ANAME,y
        and #$00FF
        bne @no
        iny
        bra @l
@y:     clc
        rts
@no:    sec
        rts

; Art aus WB_ITYP (erste zwei Buchstaben) -> A
f_itype:
        ldy #WB_ITYP
        BRD
        bne @h
        lda #1                  ; ohne type: Text
        rts
@h:     ldy #WB_ITYP+1
        lda [PSRC],y
        ora #$2020
        cmp #$6968              ; "hi"dden
        bne @1
        lda #2
        rts
@1:     cmp #$7573              ; "su"bmit
        beq @s
        cmp #$6D69              ; "im"age
        bne @2
@s:     lda #3
        rts
@2:     cmp #$6863              ; "ch"eckbox
        beq @c
        cmp #$6172              ; "ra"dio
        bne @3
@c:     lda #4
        rts
@3:     cmp #$6572              ; "re"set
        beq @x
        cmp #$7562              ; "bu"tton
        beq @x
        cmp #$6966              ; "fi"le
        beq @x
        lda #1
        rts
@x:     lda #0
        rts

ho_form:
        lda #1
        jsr block
        lda HNF
        cmp #FRMMAX
        bcc *+3
        rts
        sta HFORM
        ldx #132
        CALL gmul
        clc
        adc #WB_FRM
        sta FI
        tay                     ; Methode: "post" -> 1
        lda #0
        sta HT
        ldy #WB_ITYP
        BRD
        beq @m
        ldy #WB_ITYP+1
        BRD
        ora #$20
        cmp #'g'                ; method=gopher (Gopher-Suche), nicht "get"
        bne @mp
        ldy #WB_ITYP+2
        BRD
        ora #$20
        cmp #'o'
        bne @m
        lda #2
        sta HT
        bra @m
@mp:    cmp #'p'
        bne @m
        inc HT
@m:     ldy FI
        lda HT
        BWR
        ldy #WB_HREF            ; Ziel (Laenge + Text, hoechstens 127)
        BRD
        cmp #128
        bcc @l
        lda #0
@l:     sta UN
        ldx #0
@c:     txa
        clc
        adc #WB_HREF
        tay
        BRD
        pha
        txa
        clc
        adc FI
        inc a
        tay
        pla
        BWR
        inx
        cpx UN
        bcc @c
        beq @c
        inc HNF
        rts
hc_form:
        lda #1
        jsr block
        lda #$FF
        sta HFORM
        rts

; neues Feld der Art A mit Name/Wert aus WB_CTYP/WB_ALT. C=1 wenn kein Platz
f_new:  sta HT
        lda HFORM
        cmp #$FF
        beq @no
        lda HNFLD
        cmp #FLDMAX
        bcs @no
        jsr fld_rec
        sta FI
        tay
        lda HT
        BWR
        iny
        lda HFORM
        BWR
        lda FI
        clc
        adc #F_NAME
        tay
        ldx #WB_CTYP
        lda #31
        jsr f_cp
        lda FI
        clc
        adc #F_VAL
        tay
        ldx #WB_ALT
        lda #63
        jsr f_cp
        clc
        rts
@no:    sec
        rts
; Zeichenkette X -> Y (Puffer), hoechstens A Zeichen; Rohbytes ab $80 als Latin-1
f_cp:   sta UMAX
        stx UI
        sty UJ
        txy
        BRD
        cmp UMAX
        bcc @n
        lda UMAX
@n:     sta UN
        ldy UJ
        BWR
        ldx #0
@l:     cpx UN
        bcs @r
        inx
        txa
        clc
        adc UI
        tay
        BRD
        cmp #$80                ; Umlaute (Latin-1) -> interne Codes
        bcc @a
        phx
        sec
        sbc #$A0
        bcc @q
        tax
        lda #w_lat1
        jsr xtab
        plx
        bra @a
@q:     plx
        lda #'?'
@a:     pha
        txa
        clc
        adc UJ
        tay
        pla
        BWR
        bra @l
@r:     rts

; Standardbeschriftung, wenn ein Knopf keinen Wert hat
f_deflabel:
        lda FI
        clc
        adc #F_VAL
        tay
        BRD
        beq @go
        rts
@go:    WS ws_send
        sta PDST
        CBANK
        sta PDST+2
        lda [PDST]
        and #$00FF
        sta UN
        BWR
        ldx #0
@l:     cpx UN
        bcs @r
        inx
        txy
        lda [PDST],y
        pha
        txa
        clc
        adc FI
        clc
        adc #F_VAL
        tay
        pla
        BWR
        bra @l
@r:     rts

ho_input:
        jsr f_itype
        bne *+3
        rts
        cmp #4
        bne @n4
        ldx HCHK                ; nicht angekreuzt: nichts senden
        bne @ck
        rts
@ck:    ldy #WB_ALT             ; ohne Wert: "on"
        BRD
        bne @n4b
        lda #2
        BWR
        ldy #WB_ALT+1
        lda #'o'
        BWR
        iny
        lda #'n'
        BWR
@n4b:   lda #4
@n4:    pha
        jsr f_new
        pla
        bcc *+3
        rts
        cmp #1
        beq @txt
        cmp #3
        beq @btn
        inc HNFLD
        rts
@btn:   jsr f_deflabel
        lda #$19
        bra @emit
@txt:   lda #$18
@emit:  pha
        jsr flush
        pla
        jsr out_b
        lda HNFLD
        jsr out_b
        stz HTN
        stz HSTART
        inc HNFLD
        rts

; <button>: Absenden-Knopf (Inhalt wird uebersprungen)
ho_button:
        lda HSC
        bne @r
        jsr f_itype
        cmp #1                  ; ohne type oder "submit" (-> 1 bzw. 3)
        beq @ok
        cmp #3
        bne @r
@ok:    lda #3
        jsr f_new
        bcs @r
        jsr f_deflabel
        jsr flush
        lda #$19
        jsr out_b
        lda HNFLD
        jsr out_b
        stz HTN
        stz HSTART
        inc HNFLD
        lda #1                  ; Beschriftung im Tag ueberspringen
        sta HSKIP
        ldx #10
@c:     lda TNAME,x
        sta SKNAME,x
        dex
        dex
        bpl @c
@r:     rts

; ---------- Zeichnen ----------
; Feld/Knopf A (X = Code $18/$19) bei WLX, DLY
f_draw: sta FI
        cpx #$19
        bne *+5
        jmp @btn
        lda WLX                 ; Eingabefeld: weiss, vertieft
        sta RX
        lda DLY
        dec a
        sta RY
        lda #FLDW
        sta RW
        lda #11
        sta RH
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda #DKGRAY
        sta C1
        lda #GRAY
        sta C2
        CALL bevel
        lda WLX                 ; Text, rechts abgeschnitten
        clc
        adc #FLDW-2
        clc
        adc DXO
        sta CLIPR
        lda #BLACK
        CALL setcolor
        lda WLX
        inc a
        inc a
        sta SX
        lda DLY
        sta SY
        lda FI
        jsr f_valptr
        JSLF xf_wd_chars
        lda FI
        cmp FFOC
        bne @nc
        lda FI                  ; Schreibmarke an FCUR
        jsr fld_rec
        clc
        adc #F_VAL
        sta LEB
        tay
        BRD
        cmp FCUR
        bcs @fc
        sta FCUR
@fc:    lda FCUR
        jsr le_width
        clc
        adc WLX
        inc a
        inc a
        sta RX
        lda #BLACK
        CALL setcolor
        lda DLY
        sta RY
        lda #1
        sta RW
        lda #9
        sta RH
        CALL fillrect
@nc:    lda #320
        sta CLIPR
        lda FI                  ; Auswahl: Knopf mit Pfeil rechts
        jsr fld_rec
        tay
        BRD
        cmp #5
        beq *+5
        jmp @e
        lda WLX
        clc
        adc #FLDW-11
        sta RX
        lda DLY
        sta RY
        lda #10
        sta RW
        lda #9
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
        lda #BLACK              ; kleines Dreieck
        CALL setcolor
        lda WLX
        clc
        adc #FLDW-9
        sta RX
        lda DLY
        clc
        adc #3
        sta RY
        lda #6
        sta RW
        lda #1
        sta RH
        CALL fillrect
        inc RX
        inc RY
        lda #4
        sta RW
        CALL fillrect
        inc RX
        inc RY
        lda #2
        sta RW
        CALL fillrect
        bra @e
@btn:   lda FI                  ; Knopf
        ldx #$19
        jsr fld_w
        sec
        sbc #2
        sta RW
        lda WLX
        sta RX
        lda DLY
        dec a
        sta RY
        lda #11
        sta RH
        lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
        lda #BLACK
        CALL setcolor
        lda WLX
        clc
        adc #5
        sta SX
        lda DLY
        sta SY
        lda FI
        jsr f_valptr
        JSLF xf_wd_chars
@e:     lda #$FFFF
        sta DCOL
        rts
; PDST/WDN auf den Wert von Feld A (Auswahl: Text der gewaehlten Option)
f_valptr:
        pha
        jsr fld_rec
        tay
        BRD
        cmp #5
        bne @tx
        pla
        jsr fld_rec
        clc
        adc #F_CUR
        tay
        BRD
        jsr opt_addr
        clc
        adc #32
        tay
        jsr ob_rd               ; setzt PDST auf die Bildbank
        sta WDN
        iny
        sty PDST
        stz WDBOLD
        rts
@tx:    pla
        jsr fld_rec
        clc
        adc #F_VAL
        tay
        BRD
        sta WDN
        iny
        sty PDST
        lda NWB
        sta PDST+2
        stz WDBOLD
        rts

; ---------- Tasten in einem Feld ----------
f_key:  lda KEYC
        cmp #$1B
        bne @k1
        lda #$FFFF
        sta FFOC
        lda #$FFFF
        sta FCUR
        CALL redraw_cont
        rts
@k1:    cmp #$0D
        bne @k2
        lda FFOC
        jmp f_submit
@k2:    lda FFOC                ; Zeileneditor (lineed.inc)
        jsr fld_rec
        clc
        adc #F_VAL
        sta LEB
        lda #63
        sta LEM
        lda FCUR
        sta LEC
        jsr le_key
        lda LEC
        sta FCUR
        bcs @r
@rd:    CALL redraw_cont
@r:     rts

; ---------- Absenden: Feld/Knopf A ----------
f_submit:
        sta FSUB
        jsr fld_rec
        inc a
        tay
        BRD
        sta FFRM
        ldx #132
        CALL gmul
        clc
        adc #WB_FRM
        sta FI
        inc a                   ; Ziel -> WB_HREF -> WB_TURL
        tax
        ldy #WB_HREF
        jsr bs_copy
        jsr url_join
        bcc @j
        WS ws_nolink
        ldx #0
        jmp st_set
@j:     stz QPOST
        ldy FI
        BRD
        sta UK                  ; Methode
        cmp #2
        bne @np
        jmp f_gopher
@np:    cmp #1
        bne @get
        inc QPOST               ; POST: Inhalt nach WB_HREF
        ldy #WB_HREF
        lda #0
        BWR
        bra @q
@get:   ldy #WB_TURL            ; GET: alte Abfrage weg, '?' anhaengen
        BRD
        sta UN
        ldx #0
@sq:    inx
        cpx UN
        beq @sq1
        bcs @sq2
@sq1:   txa
        clc
        adc #WB_TURL
        tay
        BRD
        cmp #'?'
        bne @sq
        dex
        txa
        ldy #WB_TURL
        BWR
@sq2:   lda #'?'
        jsr tb_c
@q:     lda #1
        sta FFIRST
        stz UX                  ; Feld-Nr.
@f:     lda UX
        cmp HNFLD
        bcs @done
        jsr fld_rec
        sta UJ
        inc a
        tay
        BRD
        cmp FFRM
        bne @nx
        ldy UJ
        BRD
        beq @nx
        cmp #3
        bne @use
        lda UX                  ; nur der gedrueckte Knopf
        cmp FSUB
        bne @nx
@use:   lda UJ
        clc
        adc #F_NAME
        tay
        BRD
        beq @nx
        lda FFIRST
        bne @fi
        lda #'&'
        jsr q_c
@fi:    stz FFIRST
        lda UJ
        clc
        adc #F_NAME
        jsr q_enc
        lda #'='
        jsr q_c
        lda UJ
        clc
        adc #F_VAL
        jsr q_enc
@nx:    inc UX
        bra @f
@done:  lda QPOST
        sta WPOST
        lda #$FFFF
        sta FFOC
        lda #$FFFF
        sta FCUR
        stz WBACK
        jmp web_go

; Gopher-Suche: Ziel + TAB + Text des ersten Felds
f_gopher:
        lda #'%'
        jsr tb_c
        lda #'0'
        jsr tb_c
        lda #'9'
        jsr tb_c
        stz UX
@f:     lda UX
        cmp HNFLD
        bcs @g
        jsr fld_rec
        sta UJ
        tay
        BRD
        cmp #1
        bne @n
        lda UJ
        clc
        adc #F_VAL
        jsr q_enc
        bra @g
@n:     inc UX
        bra @f
@g:     lda #$FFFF
        sta FFOC
        lda #$FFFF
        sta FCUR
        stz WPOST
        stz WBACK
        jmp web_go

; Zeichen A an die Abfrage (WB_TURL bzw. POST-Inhalt in WB_HREF)
q_c:    ldx QPOST
        bne @p
        jmp tb_c
@p:     sta UT_C
        phy
        ldy #WB_HREF
        BRD
        cmp #190
        bcs @r
        inc a
        BWR
        clc
        adc #WB_HREF
        tay
        lda UT_C
        BWR
@r:     ply
        rts

; Zeichenkette bei A (Puffer) URL-kodiert anhaengen (Umlaute als UTF-8)
q_enc:  sta UI
        tay
        BRD
        sta UN
@l:     lda UN
        beq @r
        inc UI
        ldy UI
        BRD
        cmp #' '
        bne @n1
        lda #'+'
        jsr q_c
        bra @nx
@n1:    cmp #$80
        bcs @u
        sta HT
        jsr url_safe
        bcc @hx
        lda HT
        jsr q_c
        bra @nx
@hx:    lda HT
        jsr q_hex
        bra @nx
@u:     cmp #$87
        bcs @qq
        sec
        sbc #$80
        pha
        lda #$C3
        jsr q_hex
        plx
        lda f_utf,x
        and #$00FF
        jsr q_hex
        bra @nx
@qq:    lda #'?'
        jsr q_c
@nx:    dec UN
        bra @l
@r:     rts
        .pushseg
        .segment "RODATA"
f_utf:  .byte $A4,$B6,$BC,$84,$96,$9C,$9F
        .popseg
; %XX
q_hex:  sta HT
        lda #'%'
        jsr q_c
        lda HT
        lsr
        lsr
        lsr
        lsr
        and #$000F
        jsr u_hexd
        jsr q_c
        lda HT
        and #$000F
        jsr u_hexd
        jmp q_c

; ================================================================
; Auswahllisten (<select>/<option>) und <textarea>
; Feldart 5 = Auswahl. Datensatz: +34 Wert (hier hoechstens 30), +96 erste
; Option, +97 Anzahl, +98 gewaehlte Option. Optionen liegen in der Bildbank
; (IMBK) ab OPT0: 32 x 64 Bytes (+0 Feld, +1 Wert 1+30, +32 Text 1+30).
; Ein Klick schaltet zur naechsten Option weiter.
; ================================================================
OPT0    = $B800
OPTMAX  = 32
F_FIRST = 96
F_CNT   = 97
F_CUR   = 98

        .pushseg
        .segment "BSS"
HSEL:   .res 2                  ; 1 in <select> (mit Feld), 2 ohne Feld
HSELF:  .res 2                  ; Feld der Auswahl
HOPT:   .res 2                  ; Optionstext wird gesammelt
HOSEL:  .res 2                  ; Option hat "selected"
OPTN:   .res 2                  ; Optionen insgesamt
OPA:    .res 2                  ; Adresse der aktuellen Option
HTXA:   .res 2                  ; in <textarea>: Text wird Feldwert
HTXF:   .res 2
OPK:    .res 2
        .popseg

; Option A -> A = Adresse in der Bildbank
opt_addr:
        and #$00FF
        xba
        lsr
        lsr                     ; *64
        clc
        adc #OPT0
        rts
; Byte A in die Bildbank bei Y schreiben / lesen
ob_wr:  pha
        lda IMBK
        sta PDST+2
        stz PDST
        pla
        sep #$20
        sta [PDST],y
        rep #$20
        rts
ob_rd:  lda IMBK
        sta PDST+2
        stz PDST
        lda [PDST],y
        and #$00FF
        rts

ho_select:
        lda HSC
        beq *+3
        rts
        lda #2
        sta HSEL
        stz HOPT
        jsr gif_mem             ; Platz fuer die Optionen
        bcc *+3
        rts
        lda #5
        jsr f_new
        bcc *+3
        rts
        lda FI                  ; noch keine Option
        clc
        adc #F_FIRST
        tay
        lda OPTN
        BWR
        iny
        lda #0
        BWR
        iny
        lda OPTN
        BWR
        lda FI
        clc
        adc #F_VAL
        tay
        lda #0
        BWR
        lda HNFLD
        sta HSELF
        lda #1
        sta HSEL
        jsr flush
        lda #$18
        jsr out_b
        lda HNFLD
        jsr out_b
        stz HTN
        stz HSTART
        inc HNFLD
        rts
hc_select:
        jsr opt_end
        stz HSEL
        rts

ho_option:
        jsr opt_end
        lda HSEL
        cmp #1
        bne @r
        lda OPTN
        cmp #OPTMAX
        bcs @r
        jsr opt_addr
        sta OPA
        tay
        lda HSELF
        jsr ob_wr
        lda HCHK                ; "selected"
        sta HOSEL
        ldy #WB_ALT             ; Wert (hoechstens 30)
        BRD
        cmp #31
        bcc @v
        lda #30
@v:     sta OPK
        ldy OPA
        iny
        jsr ob_wr
        ldx #0
@c:     cpx OPK
        bcs @t
        txa
        clc
        adc #WB_ALT+1
        tay
        BRD
        pha
        txa
        clc
        adc OPA
        clc
        adc #2
        tay
        pla
        jsr ob_wr
        inx
        bra @c
@t:     lda OPA                 ; Text noch leer
        clc
        adc #32
        tay
        lda #0
        jsr ob_wr
        lda #1
        sta HOPT
@r:     rts
hc_option:
        jmp opt_end

; Zeichen A an den Optionstext
opt_add:
        pha
        lda OPA
        clc
        adc #32
        tay
        jsr ob_rd
        cmp #30
        bcs @f
        inc a
        jsr ob_wr               ; neue Laenge
        clc
        adc OPA
        clc
        adc #32
        tay
        pla
        jmp ob_wr
@f:     pla
        rts
; Leerraum: hoechstens ein Leerzeichen, nicht am Anfang
opt_sp: lda OPA
        clc
        adc #32
        tay
        jsr ob_rd
        beq @r
        clc
        adc OPA
        clc
        adc #32
        tay
        jsr ob_rd
        cmp #' '
        beq @r
        lda #' '
        jmp opt_add
@r:     rts

; Option abschliessen: leerer Wert = Text; erste oder "selected" wird gewaehlt
opt_end:
        lda HOPT
        bne @go
        rts
@go:    stz HOPT
        lda OPA                 ; Leerzeichen am Ende weg
        clc
        adc #32
        tay
        jsr ob_rd
        beq @tv
        sta OPK
        clc
        adc OPA
        clc
        adc #32
        tay
        jsr ob_rd
        cmp #' '
        bne @tv
        lda OPA
        clc
        adc #32
        tay
        lda OPK
        dec a
        jsr ob_wr
@tv:    ldy OPA                 ; Wert leer? dann den Text nehmen
        iny
        jsr ob_rd
        bne @cnt
        lda OPA
        clc
        adc #32
        tay
        jsr ob_rd
        sta OPK
        ldx #0
@cp:    cpx OPK
        beq @cp1
        bcs @cnt
@cp1:   txa
        clc
        adc OPA
        clc
        adc #32
        tay
        jsr ob_rd
        pha
        txa
        clc
        adc OPA
        inc a
        tay
        pla
        jsr ob_wr
        inx
        bra @cp
@cnt:   lda HSELF               ; Anzahl erhoehen
        jsr fld_rec
        sta FI
        clc
        adc #F_CNT
        tay
        BRD
        inc a
        BWR
        cmp #1                  ; erste Option oder "selected": waehlen
        beq @sel
        lda HOSEL
        beq @nx
@sel:   lda FI
        clc
        adc #F_CUR
        tay
        lda OPTN
        BWR
        lda FI
        jsr sel_val
@nx:    inc OPTN
        rts

; Feld (Datensatz A): Wert der gewaehlten Option uebernehmen
sel_val:
        sta OPK
        clc
        adc #F_CUR
        tay
        BRD
        jsr opt_addr
        sta OPA
        inc a                   ; Wert: Laenge + Text
        tay
        jsr ob_rd
        sta FI
        lda OPK
        clc
        adc #F_VAL
        tay
        lda FI
        BWR
        ldx #0
@c:     cpx FI
        bcs @r
        txa
        clc
        adc OPA
        clc
        adc #2
        tay
        jsr ob_rd
        pha
        txa
        clc
        adc OPK
        clc
        adc #F_VAL+1
        tay
        pla
        BWR
        inx
        bra @c
@r:     rts

; Klick auf eine Auswahl (Feld A): naechste Option
sel_next:
        jsr fld_rec
        sta OPK
        clc
        adc #F_CNT
        tay
        BRD
        beq @r
        sta FI                  ; Anzahl
        lda OPK
        clc
        adc #F_FIRST
        tay
        BRD
        sta OPA                 ; erste
        lda OPK
        clc
        adc #F_CUR
        tay
        BRD
        inc a
        sec
        sbc OPA
        cmp FI
        bcc @k
        lda #0
@k:     clc
        adc OPA
        BWR
        lda OPK
        jsr sel_val
        CALL redraw_cont
@r:     rts

; <textarea name=...>: einzeiliges Textfeld, Inhalt wird der Wert
ho_textarea:
        lda HSC
        beq *+3
        rts
        lda #1
        jsr f_new
        bcs @r
        lda FI
        clc
        adc #F_VAL
        sta HTXF
        tay
        lda #0
        BWR
        lda #1
        sta HTXA
        jsr flush
        lda #$18
        jsr out_b
        lda HNFLD
        jsr out_b
        stz HTN
        stz HSTART
        inc HNFLD
@r:     rts
hc_textarea:
        stz HTXA
        rts
; Zeichen A an den Textarea-Wert (Zeilenwechsel als Leerzeichen)
txa_add:
        pha
        ldy HTXF
        BRD
        cmp #63
        bcs @f
        inc a
        BWR
        clc
        adc HTXF
        tay
        pla
        BWR
        rts
@f:     pla
        rts
