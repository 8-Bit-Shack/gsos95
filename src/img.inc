; ================================================================
; GS/OS95 Browser: GIF-Bilder in der Seite (Teil von web.s)
; h2t traegt jedes <img src="...gif"> ein (bis IMGMAX, Adresse in der
; Bildbank ab IMGURL) und schreibt $1A n in den Text. Nach dem Anzeigen
; der Seite holt img_fetch die Bilder nacheinander (Esc bricht ab), entpackt
; sie in die Bildbank (verkleinert: hoechstens IMGLH hoch, IMGLA Punkte)
; und bricht die Seite neu um. lay_all macht aus $1A n so viele Zeilen,
; wie das Bild hoch ist (Kennzeichen 4, Rand = Streifen, Link = Bild).
; ================================================================
IMGMAX  = 8
IMGURL  = $B000                 ; 8 x 256 Bytes
IMGLH   = 120
IMGLA   = 16000

        .pushseg
        .segment "BSS"
IMGN:   .res 2                  ; eingetragene Bilder
IMGNX:  .res 2                  ; naechster freier Platz in der Bildbank
IMGO:   .res IMGMAX*2           ; Lage, Breite, Hoehe (0 = nicht da)
IMGW:   .res IMGMAX*2
IMGH:   .res IMGMAX*2
IMGLK:  .res IMGMAX*2           ; Bild steht in einem Link: dessen Stelle im Text
IMGI:   .res 2
IMGK:   .res 2
IMGAB:  .res 2
IMLS:   .res 2
IMLC:   .res 2
IMLK:   .res 2
IMDH:   .res 2
IMDO:   .res 2
        .popseg

; beim Lesen: <img> mit GIF eintragen und $1A n ausgeben (C=0 wenn eingetragen)
img_reg:
        lda IMGN
        cmp #IMGMAX
        bcs @no
        jsr gif_mem
        bcs @no
        lda IMGN                ; Adresse (WB_HREF) in die Bildbank
        xba
        clc
        adc #IMGURL
        sta IMGK
        ldy #WB_HREF
        BRD
        sta IMGI
        inc IMGI
        ldx #0
@c:     cpx IMGI
        bcs @d
        txa
        clc
        adc #WB_HREF
        tay
        BRD
        pha
        txa
        clc
        adc IMGK
        tay
        pla
        jsr ob_wr
        inx
        bra @c
@d:     lda IMGN
        asl
        tax
        stz IMGH,x
        stz IMGLK,x
        jsr flush
        lda #$1A
        jsr out_b
        lda IMGN
        jsr out_b
        stz HTN
        stz HSTART
        inc IMGN
        clc
        rts
@no:    sec
        rts

; ---------- nach dem Anzeigen: Bilder holen ----------
img_fetch:
        lda IMGN
        bne @go
        rts
@go:    stz IMGNX
        stz IMGI
        stz IMGAB
@l:     lda IMGI
        cmp IMGN
        bcs @e
        lda IMGAB
        bne @e
        inc a                   ; Statuszeile: Bild n
        lda IMGI
        inc a
        sta STNUM
        WS ws_imgld
        ldx #1
        jsr st_set
        jsr img_one
        bcs @nx
        jsr net_ptr0            ; neu umbrechen und zeigen
        jsr lay_all
        lda WIN
        cmp #21
        bne @nx
        CALL redraw_cont
@nx:    inc IMGI
        bra @l
@e:     WS ws_ready
        ldx #0
        jsr st_set
        lda WIN
        cmp #21
        bne @r
        CALL redraw_win
@r:     rts

; Bild IMGI holen. C=1 wenn nicht (oder abgebrochen: IMGAB)
img_one:
        jsr net_ptr0
        ldx #0
        lda IMGI                ; Adresse -> WB_HREF -> url_join -> WB_TURL
        xba
        clc
        adc #IMGURL
        sta IMGK
        tay
        jsr ob_rd
        inc a
        sta UN
@c:     cpx UN
        bcs @j
        txa
        clc
        adc IMGK
        tay
        jsr ob_rd
        pha
        txa
        clc
        adc #WB_HREF
        tay
        pla
        BWR
        inx
        bra @c
@j:     jsr url_join
        bcc *+3
        rts
        jsr url_parse
        bcc *+3
        rts
        stz NTLS
        stz WPROXY
        stz WPOST
        lda USCH
        beq @h
        cmp #1
        beq *+5
        jmp @no
        lda NCAPS               ; https nur mit TLS auf der Karte
        and #CAP_TLS
        bne *+5
        jmp @no
        lda #1
        sta NTLS
@h:     lda NCAPS
        and #CAP_NAME
        bne @o
        JSLF xf_net_resolve
        bcc *+5
        jmp @no
@o:     lda WPORT
        sta DPORT
        JSLF xf_tcp_open
        bcc *+5
        jmp @nerr
        jsr req_build
        bcc *+5
        jmp @cl
        JSLF xf_tcp_write
        bcc *+5
        jmp @cl
        stz WPTR
        lda #$8000
        sta NLIM
        stz NSTRM
        lda #1
        sta NQUIET
        JSLF xf_tcp_readall
        stz NQUIET
        php
        JSLF xf_tcp_close
        plp
        bcc *+5
        jmp @nerr
        jsr net_ptr0
        lda WPTR
        sta RAWEND
        beq @no
        jsr hdr_parse
        lda HCODE
        cmp #200
        bne @no
        lda HCHUNK
        beq @nc
        jsr dechunk
@nc:    lda IMGNX               ; Platz und Grenzen
        sta F_BASE
        lda WRAPW
        sta F_LIMW
        lda #IMGLH
        sta F_LIMH
        lda #G_PIXE
        sec
        sbc IMGNX
        cmp #IMGLA
        bcc @la
        lda #IMGLA
@la:    cmp #64
        bcc @no
        sta F_LIMA
        jsr gif_dec
        bcs @no
        lda F_SW                ; Platzhalter (1x1 usw.) nicht zeigen
        cmp #4
        bcc @ok
        lda F_SH
        cmp #4
        bcc @ok
        lda IMGI
        asl
        tax
        lda IMGNX
        sta IMGO,x
        lda F_SW
        sta IMGW,x
        lda F_SH
        sta IMGH,x
        lda F_SW
        ldx F_SH
        CALL gmul
        clc
        adc IMGNX
        sta IMGNX
@ok:    clc
        rts
@cl:    JSLF xf_tcp_close
@no:    sec
        rts
@nerr:  lda NERR                ; Esc: alle weiteren Bilder auslassen
        cmp #NE_ABRT
        bne @no
        lda #1
        sta IMGAB
        sec
        rts

; ---------- Umbruch: $1A n an LPOS ----------
; C=0: erledigt (Zeilen angelegt), C=1: Tabelle voll
img_lay:
        iny
        BRD
        sta IMLK
        cmp IMGN
        bcs @skip
        asl
        tax
        lda IMGH,x
        beq @skip
        clc                     ; Streifen = Hoehe / LH (aufgerundet)
        adc #LH-1
        ldx #LH
        CALL gdiv
        sta IMLC
        lda IMLK                ; steht das Bild in einem Link?
        asl
        tax
        stz IMGLK,x
        lda LFL
        and #2
        beq @nl
        lda LLK
        sta IMGLK,x
@nl:    lda LCH                 ; angefangene Zeile beenden
        beq @first
        jsr lay_new
        bcs @r
@first: stz IMLS
@s:     jsr img_mark            ; Zeile NLINES-1 als Streifen IMLS
        inc IMLS
        lda IMLS
        cmp IMLC
        bcs @d
        jsr lay_new
        bcs @r
        bra @s
@d:     inc LPOS
        inc LPOS
        jmp lay_new             ; Text danach in neuer Zeile
@skip:  inc LPOS
        inc LPOS
        clc
@r:     rts
img_mark:
        lda NLINES
        dec a
        asl
        sta LREC
        asl
        clc
        adc LREC
        clc
        adc #WB_LIN+2
        tay
        lda #4                  ; Kennzeichen: Bildzeile
        BWR
        iny
        lda IMLS             ; Streifen
        BWR
        iny
        lda IMLK                ; Bild
        sta [PSRC],y
        rts

; ---------- Zeichnen einer Bildzeile (aus ln_draw) ----------
img_line:
        lda LLK
        cmp IMGN
        bcs @r
        asl
        tax
        lda IMGH,x
        beq @r
        sta IMDH
        lda IMGO,x
        sta IMDO
        lda IMGW,x
        sta F_SW
        lda IMBK
        sta PDST+2
        stz PDST
        lda LMG                 ; erste Bildzeile dieses Streifens
        ldx #LH
        CALL gmul
        sta F_GY
        stz F_GBI
@row:   lda F_GBI
        cmp #LH
        bcs @r
        lda F_GY
        cmp IMDH
        bcs @r
        lda DLY
        dec a
        clc
        adc DYO
        clc
        adc F_GBI
        cmp KT
        bcc @nr
        cmp KB
        bcs @nr
        pha
        lda F_GY
        ldx F_SW
        CALL gmul
        clc
        adc IMDO
        sta F_GROWB
        lda F_SW
        sta F_GNC
        lda #WX+8
        clc
        adc DXO
        sta F_GDX
        pla
        jsr g_row
@nr:    inc F_GBI
        inc F_GY
        bra @row
@r:     rts

; Klick auf ein Bild (A = Nummer): Bild allein anzeigen
img_open:
        sta IMGI
        jsr net_ptr0
        ldx #0
        lda IMGI
        xba
        clc
        adc #IMGURL
        sta IMGK
        tay
        jsr ob_rd
        inc a
        sta UN
@c:     cpx UN
        bcs @j
        txa
        clc
        adc IMGK
        tay
        jsr ob_rd
        pha
        txa
        clc
        adc #WB_HREF
        tay
        pla
        BWR
        inx
        bra @c
@j:     jsr url_join
        bcs @r
        stz WBACK
        jmp web_go
@r:     rts

; Bild A in einem Link? C=1 und A = Stelle des Links im Text
img_lnk:
        cmp IMGN
        bcs @no
        asl
        tax
        lda IMGLK,x
        beq @no
        sec
        rts
@no:    clc
        rts
