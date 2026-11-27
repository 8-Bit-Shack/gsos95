; ================================================================
; GS/OS95: Internet-Browser (Fensterart 21)
; HTTP/1.0 ueber net.s, HTML -> Text mit Steuerzeichen -> Zeilen -> Bildschirm
; Text im Puffer (Bank NWB): $20..$7E ASCII, $80..$86 Umlaute, $87 Punkt,
;   $0A Zeilenende, $01 n href.. Link an, $02 Link aus, $10/$11 fett aus/an,
;   $14 m linker Rand m*6, $15 Abstand (Tabelle), $16 Trennlinie
; Zeile (6 Bytes): Offset, Merkmale (Bit 0 fett, Bit 1 Link), Rand, Link-Offset
; ================================================================
        .segment "XCODE2"
FARMODE .set 2

; Bank dieser Codebank (Texte aus webstr.inc) nach STRBK (beim Programmstart)
xf_strbk:
        sep #$20
        phk
        pla
        rep #$20
        and #$00FF
        sta STRBK
        rtl

xf_bbs_open:   lda #1               ; Mailbox-Telefonbuch
        sta WOBBS
        jsr web_open
        stz WOBBS
        rtl
xf_help_open:  lda #2               ; Hilfe: about:keys
        bra xf_wob
xf_arc_open:   lda #3               ; Software-Archiv: about:archiv
        bra xf_wob
xf_rss_open:   lda #4               ; Nachrichten: about:rss
        bra xf_wob
xf_dos_open:   lda #5               ; MS-DOS-Eingabeaufforderung: about:msdos
        bra xf_wob
xf_wx_open:    lda #6               ; Wetter: about:wetter
xf_wob: sta WOBBS
        jsr web_open
        stz WOBBS
        rtl
xf_vt_put:     jsr vt_char          ; fuer xfer.inc (XCODE3)
        rtl
xf_vt_cur:     cmp #0
        beq @off
        jsr vt_curon
        rtl
@off:   jsr vt_curoff
        rtl
xf_dl_open:    jsr dl_open
        rtl
xf_web_open:   jsr web_open
        rtl
xf_tn_poll:    jsr tn_poll
        rtl
xf_web_draw:   jsr web_draw
        rtl
xf_web_click:  jsr web_click
        rtl
xf_web_key:    jsr web_key
        rtl
xf_web_geom:   jsr web_geom
        rtl
xf_web_title:  jsr web_title
        rtl
xf_web_prog:   jsr web_prog
        rtl

HISMAX  = 3
MAXLN   = (WB_LINE-WB_LIN)/6
LH      = 10                    ; Zeilenhoehe
LINKC   = 4                     ; blau
MARGW   = 6                     ; Randeinheit in Punkten

; Byte aus dem Puffer an Offset (Y) -> A (16 Bit, oberes Byte 0)
.macro BRD
        lda [PSRC],y
        and #$00FF
.endmacro
.macro CBANK
        sep #$20
        phk
        pla
        rep #$20
        and #$00FF
.endmacro
.macro BWR
        sep #$20
        sta [PSRC],y
        rep #$20
.endmacro

        .pushseg
        .segment "BSS"
WBINIT:  .res 2                 ; Breitentabelle fertig
GWT:     .res 136               ; Breite je Zeichen $00..$87
NLINES:  .res 2
TBIX:     .res 2
WOBBS:   .res 2
TBIR:     .res 2
TBIM:     .res 2
TXTEND:  .res 2
WRAPW:   .res 2                 ; Breite des letzten Umbruchs
ADRFOC:  .res 2                 ; Adresszeile hat den Fokus
ADRSEL:  .res 2                 ; ganze Adresse markiert
STMSG:   .res 2                 ; Statuszeile: Text (0 = leer)
STXK:     .res 2                 ; 1 = Zahl STNUM anhaengen, 2 = Rechnername
STNUM:   .res 2
HISP:   .res 6
WRPAGE: .res 2
HISN:    .res 2
WREDIR:  .res 2
WBACK:   .res 2                 ; 1 = Laden aus dem Verlauf (nicht merken)
WSAVE:   .res 2                 ; 1 = Antwort nur speichern (Strg-S)
HCODE:   .res 2                 ; HTTP-Status
HEND:    .res 2                 ; Anfang des Inhalts
BEND:    .res 2                 ; Ende des Inhalts
HCHUNK:  .res 2
HPLAIN:  .res 2
UPDCHK:  .res 2                 ; 1: Antwort ist die Versionsdatei (about:update)
UVL:     .res 12                ; Zeilen 1..3: Anfang, Ende
UVN:     .res 6                 ; Build-Nummer der Datei
UVT:     .res 2
UVS:     .res 2
HLOC:    .res 2
HTYPOK:  .res 2
PFHE:    .res 2                 ; big_page: Ende des Stuecks (vor dem Filter)
RAWEND:  .res 2
UI:      .res 2
UJ:      .res 2
UK:      .res 2
UN:      .res 2
USCH:    .res 2                 ; 0 http, 1 https, 2 about, 3 gopher, 4 telnet, 5 irc
; HTML -> Text
HR:      .res 2
HE:      .res 2
HO:      .res 2
HWS:     .res 2
HNL:     .res 2
HTN:     .res 2
HSTART:  .res 2
HPRE:    .res 2
HSKIP:   .res 2
HBOLD:   .res 2
HLINK:   .res 2
HLTXT:   .res 2
HDEP:    .res 2
HQ:      .res 2
HOLN:    .res 8
HOLT:    .res 8
HTD:     .res 2
HTITLE:  .res 2
HFULL:   .res 2
HMORE:   .res 2
TBM:     .res 2
HC:      .res 2
HCL:     .res 2                 ; schliessendes Tag
HACT:    .res 2
TNAME:   .res 12
ANAME:   .res 10
SKNAME:  .res 12
HV:      .res 4
HT:      .res 2
; Zeilen
LPOS:    .res 2
WLX:      .res 2
LMG:     .res 2
LFL:     .res 2
LLK:     .res 2
LCH:     .res 2
BRP:     .res 2
BRF:     .res 2
BRM:     .res 2
BRLK:     .res 2
LEND:    .res 2
LREC:    .res 2
LW:      .res 2
; Zeichnen
DLN:     .res 2
DLY:     .res 2
DCOL:    .res 2
ULX:     .res 2
HITX:    .res 2
HITL:    .res 2
LXNS:    .res 2
FXC:     .res 2
FXI:     .res 2
        .popseg

; ---------- Breitentabelle ----------
web_init:
        lda WBINIT
        beq @go
        rts
@go:    inc WBINIT
        ldx #0
@l:     phx
        txa
        cmp #$20
        bcc @z
        cmp #$7F
        bcc @g
        cmp #$80
        bcc @z
@g:     JSLF xf_glw             ; Breite des Zeichens A
        bra @s
@z:     lda #0
@s:     plx
        sep #$20
        sta GWT,x
        rep #$20
        inx
        cpx #$88
        bcc @l
        rts

; Breite (mit Abstand) von Zeichen A -> A
cw:     cmp #$88
        bcs @q
        tax
        lda GWT,x
        and #$00FF
        inc a
        rts
@q:     lda #6
        rts

; ================= Fenster oeffnen =================
; vorhandenes Browserfenster -> A = Index, C=1; sonst C=0
web_find:
        stz UI
@l:     lda UI
        cmp NWIN
        bcs @no
        CALL recaddr
        sta PB
        lda (PB)
        cmp #21
        beq @y
        inc UI
        bra @l
@y:     lda UI
        sec
        rts
@no:    clc
        rts

web_open:
        jsr web_find
        bcc @new
        pha                     ; schon offen: nach vorn
        CALL win_unmin
        pla
        CALL win_front
        CALL redraw_all
        lda WOBBS               ; Telefonbuch im offenen Fenster
        beq @r0
        jsr net_ptr0
        jsr bbs_turl
        lda #1
        sta WBACK
        jsr web_load
        CALL redraw_all
        jmp img_fetch
@r0:    rts
@new:   JSLF xf_net_mem
        bcc @m
        lda #23                 ; kein Speicher
        CALL show_msg_nr
        CALL redraw_all
        rts
@m:     jsr web_init
        lda #21
        CALL win_open
        bcc @o
        lda #8                  ; zu viele Fenster
        CALL show_msg_nr
        CALL redraw_all
        rts
@o:     lda NWIN                ; Lage und Groesse
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda #8
        sta (PB),y
        ldy #R_Y
        lda #6
        sta (PB),y
        ldy #R_W
        lda #304
        sta (PB),y
        ldy #R_H
        lda #176
        sta (PB),y
        lda NWIN
        dec a
        CALL win_load
        stz NLINES
        stz PAGE
        stz WRAPW
        stz HISN
        jsr net_ptr0
        lda #1                  ; Startseite
        sta ADRFOC
        sta ADRSEL
        lda #$FFFF
        sta ADRCUR
        jsr home_turl
        jsr bbs_turl            ; (Telefonbuch: about:bbs)
        lda #1
        sta WBACK
        CALL redraw_all
        jsr web_load
        lda #1
        sta ADRFOC
        sta ADRSEL
        lda #$FFFF
        sta ADRCUR
        CALL redraw_top         ; Fenster und Taskleiste (Titel)
        jmp img_fetch
        .pushseg
        .segment "RODATA"
w_about: .byte 11,"about:start"
        .popseg

; WOBBS gesetzt: WB_TURL = about:bbs (1), about:keys (2), about:archiv (3), about:rss (4)
bbs_turl:
        lda WOBBS
        beq @r
        dec a
        asl
        tax
        lda w_wob,x
        sta UI
        tax
        lda a:0,x
        and #$00FF
        inc a
        sta UJ
        ldy #WB_TURL
@c:     ldx UI
        lda a:0,x
        BWR
        inc UI
        iny
        dec UJ
        bne @c
@r:     rts
        .pushseg
        .segment "RODATA"
w_kyu:  .byte 10,"about:keys"
w_aru:  .byte 12,"about:archiv"
w_rsu:  .byte 9,"about:rss"
w_wxu:  .byte 12,"about:wetter"
w_lzu:  .byte 8,"about:lz"
w_dsu:  .byte 11,"about:msdos"
w_wob:  .addr w_bbu, w_kyu, w_aru, w_rsu, w_dsu, w_wxu
        .popseg

net_ptr0:
        lda NWB
        sta PSRC+2
        stz PSRC
        rts

; ================= Geometrie (statt win_geom) =================
; NRV Zeilen sichtbar, TROWS = NLINES, MAXPG, SBON, PAGE begrenzen
web_geom:
        lda #1
        sta NCV
        lda CH
        sec
        sbc #52
        ldx #LH
        CALL gdiv
        bne @r
        inc a
@r:     sta NRV
        sta PPG
        lda CW                  ; neue Breite: neu umbrechen
        sec
        sbc #30
        cmp WRAPW
        beq @w
        sta WRAPW
        lda NWB
        beq @w
        jsr lay_all
@w:     lda NLINES
        sta TROWS
        sec
        sbc NRV
        bcs @mp
        lda #0
@mp:    sta MAXPG
        stz SBON
        beq @cl
        lda #1
        sta SBON
@cl:    lda PAGE
        cmp MAXPG
        bcc @ok
        beq @ok
        lda MAXPG
        sta PAGE
@ok:    rts

; ================= Zeichnen =================
web_draw:
        lda NWB
        bne @go
        rts
@go:    jsr net_ptr0
        ; Zurueck-Knopf
        lda #WX+4
        sta RX
        lda #WY+16
        sta RY
        lda #13
        sta RW
        lda #12
        sta RH
        jsr w_btn
        lda HISN
        bne @bk
        lda #DKGRAY
        bra @bc
@bk:    lda #BLACK
@bc:    CALL setcolor
        ldx #0                  ; Dreieck nach links
@tr:    phx
        txa
        clc
        adc #WX+7
        sta RX
        txa
        eor #$FFFF
        sec
        adc #WY+21
        sta RY
        txa
        asl
        inc a
        sta RH
        lda #1
        sta RW
        CALL fillrect
        plx
        inx
        cpx #4
        bcc @tr
        lda #WX+11
        sta RX
        lda #WY+21
        sta RY
        lda #4
        sta RW
        lda #1
        sta RH
        CALL fillrect
        ; Start-Knopf (Haus)
        lda #WX+18
        sta RX
        lda #WY+16
        sta RY
        lda #13
        sta RW
        lda #12
        sta RH
        jsr w_btn
        lda #BLACK
        CALL setcolor
        ldx #0
@hs:    phx
        txa
        clc
        adc #WY+18
        sta RY
        txa
        eor #$FFFF
        sec
        adc #WX+24
        sta RX
        txa
        asl
        inc a
        sta RW
        lda #1
        sta RH
        CALL fillrect
        plx
        inx
        cpx #4
        bcc @hs
        FILL WX+21,WY+22,7,4,BLACK
        FILL WX+24,WY+24,1,2,GRAY
        ; Adresszeile
        jsr adr_rect
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        jsr adr_draw
        jsr tb_draw
        ; Inhalt: Rahmen, weisse Flaeche, Statuszeile
        CALL exp_frame
        jsr st_draw
        jsr txt_draw
        jsr gif_draw
        CALL draw_sbar
        rts

; Knopf RX,RY,RW,RH
w_btn:  lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #GRAY
        sta C1
        lda #DKGRAY
        sta C2
        CALL bevel
        rts

; Rechteck der Adresszeile -> RX..RH
adr_rect:
        lda #WX+33
        sta RX
        lda #WY+16
        sta RY
        lda CW
        sec
        sbc #38+TBARW
        sta RW
        lda #12
        sta RH
        rts

; Text der Adresszeile (Ende sichtbar, wenn zu lang)
adr_draw:
        lda CW
        sec
        sbc #44+TBARW
        sta LW                  ; Platz (bis zu den Knoepfen rechts)
        ldy #WB_URL
        BRD
        sta UN
        lda ADRCUR              ; Schreibmarke (hoechstens am Ende)
        cmp UN
        bcc @cu
        lda UN
        sta ADRCUR
@cu:    lda ADRCUR
        ldx ADRFOC
        bne @cf
        lda UN                  ; ohne Fokus: Ende zeigen
@cf:    clc
        adc #WB_URL
        sta UJ                  ; letztes Zeichen vor der Schreibmarke
        stz UK                  ; Breite
        tay
@b:     cpy #WB_URL
        beq @s
        BRD
        phy
        jsr cw
        ply
        clc
        adc UK
        cmp LW
        bcs @s
        sta UK
        dey
        bra @b
@s:     iny                     ; erstes sichtbares Zeichen
        sty UI
        lda ADRSEL
        beq @ns
        lda ADRFOC
        beq @ns
        lda UN
        beq @ns
        lda #WX+35              ; markiert
        sta RX
        lda #WY+17
        sta RY
        lda UK
        inc a
        sta RW
        lda #10
        sta RH
        lda #NAVY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        bra @col
@ns:    lda #BLACK
@col:   CALL setcolor
        lda #WX+36
        sta SX
        lda #WY+18
        sta SY
        lda UI
        sta PDST
        lda NWB
        sta PDST+2
        lda UK                  ; Breite bis zur Schreibmarke
        sta UC
        lda UJ                  ; sichtbare Zeichen: ab UI, solange Platz ist
        sta UA
@fw:    lda UA
        sec
        sbc #WB_URL
        cmp UN
        bcs @fe
        lda UA
        inc a
        tay
        BRD
        jsr cw
        clc
        adc UK
        cmp LW
        bcs @fe
        sta UK
        inc UA
        bra @fw
@fe:    lda UA
        sec
        sbc UI
        inc a
        sta WDN
        stz WDBOLD
        lda UN
        beq @e
        lda UA
        cmp UI
        bcc @e
        JSLF xf_wd_chars
@e:     lda ADRFOC              ; Schreibmarke
        beq @r
        lda ADRSEL
        bne @r
        lda #WX+36
        clc
        adc UC
        sta RX
        lda #WY+18
        sta RY
        lda #1
        sta RW
        lda #9
        sta RH
        lda #BLACK
        CALL setcolor
        CALL fillrect
@r:     rts

; Statuszeile
st_draw:
        lda STMSG
        bne @go
        rts
@go:    lda #WX+8
        sta SX
        lda CH
        clc
        adc #WY-14
        sta SY
        lda #BLACK
        CALL setcolor
        lda CW                  ; nicht ueber den Rahmen
        clc
        adc #WX-8
        clc
        adc DXO
        sta CLIPR
        lda STMSG
        JSLF xf_ui_str
        lda STXK
        cmp #1
        bne @h
        lda STNUM
        CALL drawnum
        WS ws_bytes
        JSLF xf_ui_str
        bra @e
@h:     cmp #2
        bne @e
        lda #WB_HOST+1
        sta PDST
        lda NWB
        sta PDST+2
        ldy #WB_HOST
        BRD
        sta WDN
        stz WDBOLD
        JSLF xf_wd_chars
        lda #'.'
        CALL putch
        lda #'.'
        CALL putch
        lda #'.'
        CALL putch
@e:     lda #320
        sta CLIPR
        rts

; Statuszeile setzen (A = Text, X = Art) und sofort zeigen
st_set: sta STMSG
        stx STXK
st_show:
        lda NWIN
        bne @w
        rts
@w:     dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        clc
        adc #4
        sta KL
        ldy #R_W
        adc (PB),y
        sec
        sbc #8
        sta KR
        ldy #R_Y
        lda (PB),y
        ldy #R_H
        clc
        adc (PB),y
        sec
        sbc #16
        sta KT
        clc
        adc #12
        sta KB
        CALL redraw_rect
        rts

; Fortschritt beim Laden (aus net.s)
web_prog:
        lda DLON
        beq @w
        lda DLTOT+1             ; KB bisher
        lsr
        lsr
        sta STNUM
        lda WPTR
        xba
        and #$00FF
        lsr
        lsr
        clc
        adc STNUM
        sta STNUM
        WS ws_dl
        ldx #1
        jmp st_set
@w:     lda WPTR
        sta STNUM
        WS ws_load
        ldx #1
        jmp st_set

; ---------- Textzeilen ----------
txt_draw:
        lda NLINES
        bne @go
        rts
@go:    lda PAGE
        sta DLN
        lda #WY+31
        sta DLY
        stz UI
@l:     lda UI
        cmp NRV
        bcs @r
        lda DLN
        cmp NLINES
        bcs @r
        jsr ln_draw
        inc DLN
        inc UI
        lda DLY
        clc
        adc #LH
        sta DLY
        bra @l
@r:     rts

; Zeile DLN -> LPOS, LEND, LFL, LMG, LLK
ln_get: lda DLN
        asl
        sta LREC
        asl
        clc
        adc LREC
        clc
        adc #WB_LIN
        sta LREC
        tay
        lda [PSRC],y
        sta LPOS
        iny
        iny
        BRD
        sta LFL
        iny
        BRD
        sta LMG
        iny
        lda [PSRC],y
        sta LLK
        lda DLN
        inc a
        cmp NLINES
        bcs @e
        tya
        clc
        adc #2
        tay
        lda [PSRC],y
        sta LEND
        rts
@e:     lda TXTEND
        sta LEND
        rts

; Zeile DLN bei DLY zeichnen
ln_draw:
        jsr ln_get
        lda LFL                 ; Bildzeile
        and #4
        beq @tl
        jmp img_line
@tl:    lda LMG
        jsr mg_x
        sta WLX
        lda #$FFFF
        sta DCOL
        stz LCH
        stz LXNS
        ldy LPOS
        BRD
        cmp #$16                ; Trennlinie
        bne @t
        lda #WX+8
        sta RX
        lda DLY
        clc
        adc #4
        sta RY
        lda WRAPW
        sta RW
        lda #1
        sta RH
        lda #DKGRAY
        CALL setcolor
        CALL fillrect
        inc RY
        lda #WHITE
        CALL setcolor
        CALL fillrect
        rts
@t:     lda LFL
        and #2
        beq @nl
        lda WLX
        sta ULX
@nl:    lda LPOS
@c:     cmp LEND
        bcc *+5
        jmp @end
        tay
        BRD
        cmp #$20
        bcc *+5
        jmp @vis
        cmp #$01
        bne @n1
        sty LLK
        lda LFL
        ora #2
        sta LFL
        lda WLX
        sta ULX
        iny
        BRD
        clc
        adc #2
        adc LPOS
        sta LPOS
        jmp @nx0
@n1:    cmp #$02
        bne @n2
        jsr ln_ul
        lda LFL
        and #$FFFD
        sta LFL
        jmp @nx
@n2:    cmp #$10
        bne @n3
        stz WDBOLD
        lda LFL
        and #$FFFE
        sta LFL
        jmp @nx
@n3:    cmp #$11
        bne @n4
        lda LFL
        ora #1
        sta LFL
        jmp @nx
@n4:    cmp #$14
        bne @n5
        iny
        BRD
        sta LMG
        inc LPOS
        lda LCH                 ; noch nichts in der Zeile? dann gilt der Rand sofort
        beq *+5
        jmp @nx
        lda LMG
        jsr mg_x
        sta WLX
        jmp @nx
@n5:    cmp #$17                ; Tabellenspalte: $17 Spalte Breite
        bne @n5b
        iny
        iny
        BRD
        inc LPOS
        inc LPOS
        bra @n5c
@n5b:   cmp #$15
        bne @n6
        lda #8
@n5c:   clc
        adc WLX
        sta WLX
        jmp @nx
@n6:    cmp #$1A                ; Bild (nicht geladen): ueberspringen
        beq @n6s
        cmp #$1C                ; Tabelle t Anfang/Ende: Nummer ueberspringen
        beq @n6s
        cmp #$1D
        bne @n6f
@n6s:   inc LPOS
        jmp @nx
@n6f:   cmp #$18                ; Eingabefeld / Knopf
        beq @fd
        cmp #$19
        beq *+5
        jmp @nx
@fd:    sta FXC
        iny
        BRD
        sta FXI
        ldx FXC
        jsr f_draw
        lda FXI
        ldx FXC
        jsr fld_w
        clc
        adc WLX
        sta WLX
        sta LXNS
        inc LCH
        inc LPOS
        bra @nx
@vis:   pha
        lda LFL                 ; Farbe
        and #2
        beq @blk
        lda #LINKC
        bra @sc
@blk:   lda #BLACK
@sc:    cmp DCOL
        beq @sk
        sta DCOL
        CALL setcolor
@sk:    pla
        sta HT
        inc LCH
        cmp #' '
        beq @sp
        JSLF xf_glp             ; Zeiger auf das Zeichen
        sta SPR
        lda WLX
        sta SX
        lda DLY
        sta SY
        CALL blit1
        lda LFL
        and #1
        beq @sp
        inc SX
        CALL blit1
@sp:    lda HT
        jsr cw
        clc
        adc WLX
        sta WLX
        lda LFL
        and #1
        beq @ns
        inc WLX
@ns:    lda HT                  ; Ende des letzten sichtbaren Zeichens
        cmp #' '
        beq @nx
        lda WLX
        sta LXNS
@nx:    inc LPOS
@nx0:   lda LPOS
        jmp @c
@end:   lda LFL
        and #2
        beq @r
        lda LXNS                ; Leerzeichen am Zeilenende nicht unterstreichen
        sta WLX
        jsr ln_ul
@r:     rts

; Unterstreichung von ULX bis WLX
ln_ul:  lda WLX
        sec
        sbc ULX
        beq @r
        bcc @r
        dec a
        beq @r
        sta RW
        lda ULX
        sta RX
        lda DLY
        clc
        adc #8
        sta RY
        lda #1
        sta RH
        lda #LINKC
        CALL setcolor
        CALL fillrect
        lda #$FFFF
        sta DCOL
@r:     lda WLX
        sta ULX
        rts

; Rand A (Einheiten) -> x
mg_x:   sta HT
        asl
        clc
        adc HT
        asl                     ; *6
        clc
        adc #WX+8
        rts

; ================= Fenstertitel =================
web_title:
        lda NWB
        beq @def
        jsr net_ptr0
        ldy #WB_TITLE
        BRD
        beq @def
        sta WDN
        lda #WB_TITLE+1
        sta PDST
        lda NWB
        sta PDST+2
        lda #1
        sta WDBOLD
        JSLF xf_wd_chars
        stz WDBOLD
        rts
@def:   WS ws_inet
        JSLF xf_ui_str
        rts

; ================= Zeilenumbruch =================
lay_all:
        jsr net_ptr0
        jsr tab_meas
        lda #$FF
        sta TCUR
        stz NLINES
        lda #WB_TXT
        sta LPOS
        stz LFL
        stz LMG
        stz LLK
        jsr lay_new
@l:     lda LPOS
        cmp TXTEND
        bcc @go
        rts
@go:    tay
        BRD
        cmp #$20
        bcc *+5
        jmp @vis
        cmp #$0A
        bne @c1
        inc LPOS
        jsr lay_new
        bcs @full
        bra @l
@full:  rts
@c1:    cmp #$01
        bne @c2
        sty LLK
        lda LFL
        ora #2
        sta LFL
        iny
        BRD
        clc
        adc #2
        adc LPOS
        sta LPOS
        bra @l
@c2:    cmp #$02
        bne @c3
        lda LFL
        and #$FFFD
        sta LFL
        inc LPOS
        bra @l
@c3:    cmp #$10
        bne @c4
        lda LFL
        and #$FFFE
        sta LFL
        inc LPOS
        bra @l
@c4:    cmp #$11
        bne @c5
        lda LFL
        ora #1
        sta LFL
        inc LPOS
        jmp @l
@c5:    cmp #$14
        bne @c6
        iny
        BRD
        sta LMG
        lda LCH
        bne @c5b
        lda LMG
        jsr mg_w
        sta WLX
@c5b:   inc LPOS
        inc LPOS
        jmp @l
@c6:    cmp #$16
        bne @c7
        lda LCH
        beq @c6b
        jsr lay_new
        bcc *+5
        jmp @full
@c6b:   inc LPOS
        jsr lay_new
        bcc *+5
        jmp @full
        jmp @l
@c7:    cmp #$15
        bne @c7t
        lda #8
        jmp @w
@c7t:   cmp #$17                ; Tabellenspalte: $17 Spalte Breite
        bne @c7s
        iny
        BRD
        jsr col_w               ; Breite berechnen und eintragen
        sta HT
        ldy LPOS
        iny
        iny
        BWR
        lda HT
        clc
        adc WLX
        cmp WRAPW
        bcc @tf
        beq @tf
        lda LCH
        beq @tf
        jmp @wrp
@tf:    sta WLX
        inc LCH
        inc LPOS
        inc LPOS
        inc LPOS
        lda LPOS                ; hier darf umbrochen werden
        sta BRP
        lda LFL
        sta BRF
        lda LMG
        sta BRM
        lda LLK
        sta BRLK
        jmp @l
@c7s:   cmp #$1C                ; Tabelle t beginnt / endet
        bne @c7e
        iny
        BRD
        sta TCUR
        inc LPOS
        jmp @skip
@c7e:   cmp #$1D
        bne @c8
        iny
        BRD
        cmp TCUR
        bne @c7x
        lda #$FF
        sta TCUR
@c7x:   inc LPOS
        jmp @skip
@c8:    cmp #$1A                ; Bild in der Seite
        bne @c8f
        jsr img_lay
        bcc *+3
        rts
        jmp @l
@c8f:   cmp #$18                ; Eingabefeld / Knopf: feste Breite, 2 Bytes
        beq @fld
        cmp #$19
        bne @skip
@fld:   tax
        iny
        BRD
        jsr fld_w
        sta HT
        lda HT
        clc
        adc WLX
        cmp WRAPW
        bcc @ff
        beq @ff
        lda LCH
        beq @ff
        jsr lay_new
        bcc *+3
        rts
        jmp @l
@ff:    lda WLX
        clc
        adc HT
        sta WLX
        inc LCH
        inc LPOS
        inc LPOS
        jmp @l
@skip:  inc LPOS
        jmp @l
@vis:   jsr cw
        sta HT
        lda LFL
        and #1
        beq @nb
        inc HT
@nb:    lda HT
@w:     sta HT
        clc
        adc WLX
        cmp WRAPW
        bcc @fit
        beq @fit
        lda LCH
        beq @fit
@wrp:   lda BRP                 ; umbrechen
        cmp #$FFFF
        beq @force
        sta LPOS
        lda BRF
        sta LFL
        lda BRM
        sta LMG
        lda BRLK
        sta LLK
@force: jsr lay_new
        bcc *+3
        rts
        jmp @l
@fit:   lda WLX
        clc
        adc HT
        sta WLX
        inc LCH
        ldy LPOS
        inc LPOS
        BRD
        cmp #' '
        beq @br
        cmp #$15
        beq @br
        cmp #$17
        bne @nbr
@br:    lda LPOS
        sta BRP
        lda LFL
        sta BRF
        lda LMG
        sta BRM
        lda LLK
        sta BRLK
@nbr:   jmp @l

        .include "table.inc"
        .include "img.inc"
; Breite bis zum naechsten Tabstopp (alle TABW Punkte ab dem Rand, mind. 6)
TABW    = 40
tab_w:  lda LMG
        jsr mg_w
        sta TBM
        lda WLX
        sec
        sbc TBM
        clc
        adc #6
        ldx #TABW
        CALL gdiv
        inc a
        ldx #TABW
        CALL gmul
        clc
        adc TBM
        sec
        sbc WLX
        rts
; Rand A -> Breite in Punkten
mg_w:   sta HT
        asl
        clc
        adc HT
        asl
        rts

; neue Zeile ab LPOS mit aktuellem Zustand. C=1 wenn Tabelle voll
lay_new:
        lda NLINES
        cmp #MAXLN
        bcc @ok
        sec
        rts
@ok:    asl
        sta LREC
        asl
        clc
        adc LREC
        clc
        adc #WB_LIN
        tay
        lda LPOS
        sta [PSRC],y
        iny
        iny
        lda LFL
        BWR
        iny
        lda LMG
        BWR
        iny
        lda LLK
        sta [PSRC],y
        inc NLINES
        lda LMG
        jsr mg_w
        sta WLX
        stz LCH
        lda #$FFFF
        sta BRP
        clc
        rts

; ================= Klicks =================
web_click:
        jsr net_ptr0
        HIT WX+4,WY+16,13,12    ; Zurueck
        bcc @n1
        jmp web_back
@n1:    HIT WX+18,WY+16,13,12   ; Startseite
        bcc @n2
        jmp web_home
@n2:    jsr tb_hit
        bcc @n2a
        jmp tb_do
@n2a:   jsr adr_rect
        CALL inrect
        bcc @n3
        lda ADRFOC              ; erster Klick: alles markieren, zweiter: Schreibmarke
        beq @sel
        lda ADRSEL
        beq @sel
        stz ADRSEL
        bra @rw
@sel:   lda #1
        sta ADRFOC
        sta ADRSEL
        lda #$FFFF
        sta ADRCUR
@rw:    CALL redraw_win
        rts
@n3:    CALL sbar_click
        bcc @n4
        rts
@n4:    lda ADRFOC              ; Klick in die Seite nimmt den Fokus
        beq @lk
        stz ADRFOC
        stz ADRSEL
        CALL redraw_win
@lk:    jsr link_hit
        bcs @go
        lda FFOC                ; daneben: Feld verliert die Schreibmarke
        bmi @r0
        lda #$FFFF
        sta FFOC
        CALL redraw_cont
@r0:    rts
@go:    ldx HITK
        beq @lnk
        cpx #3                  ; Bild: Link darum herum oder Bild gross
        bne @g2
        pha
        jsr img_lnk
        bcc @io
        plx
        bra @lnk
@io:    pla
        jmp img_open
@g2:    cpx #2
        bne @fo
        jmp f_submit            ; Knopf
@fo:    pha                     ; Auswahl: weiterschalten
        jsr fld_rec
        tay
        BRD
        cmp #5
        bne @fo2
        pla
        jmp sel_next
@fo2:   pla
        sta FFOC                ; Feld: Schreibmarke hinein
        lda #$FFFF
        sta FCUR
        stz ADRFOC
        stz ADRSEL
        CALL redraw_win
        rts
@lnk:   sta UI                  ; Link-Ziel nach WB_HREF
        tay
        iny
        BRD
        sta UN
        ldx #0
@c:     cpx UN
        bcs @cd
        iny
        BRD
        phy
        pha
        txa
        clc
        adc #WB_HREF+1
        tay
        pla
        BWR
        ply
        inx
        bra @c
@cd:    ldy #WB_HREF
        lda UN
        BWR
        jsr url_join
        bcc @ok
        WS ws_nolink
        ldx #0
        jmp st_set
@ok:    stz WBACK
        jmp web_go

; Link unter der Maus? C=1, A = Offset des $01
link_hit:
        lda NLINES
        bne @g
        clc
        rts
@g:     CALL lmy
        sec
        sbc #WY+31
        bcs *+5
        jmp @no
        ldx #LH
        CALL gdiv
        cmp NRV
        bcc *+5
        jmp @no
        clc
        adc PAGE
        cmp NLINES
        bcc *+5
        jmp @no
        sta DLN
        CALL lmx
        sta HITX
        jsr ln_get
        lda LFL                 ; Bildzeile: Klick zeigt das Bild gross
        and #4
        beq @tl
        lda #3
        sta HITK
        lda LLK
        sec
        rts
@tl:    stz LCH
        lda LMG
        jsr mg_x
        sta WLX
        lda LPOS
@c:     cmp LEND
        bcc *+5
        jmp @no
        tay
        BRD
        cmp #$20
        bcc *+5
        jmp @vis
        cmp #$01
        bne @n1
        sty LLK
        lda LFL
        ora #2
        sta LFL
        iny
        BRD
        clc
        adc #2
        adc LPOS
        sta LPOS
        bra @c
@n1:    cmp #$02
        bne @n2
        lda LFL
        and #$FFFD
        sta LFL
        jmp @nx
@n2:    cmp #$10
        bne @n3
        lda LFL
        and #$FFFE
        sta LFL
        jmp @nx
@n3:    cmp #$11
        bne @n4
        lda LFL
        ora #1
        sta LFL
        jmp @nx
@n4:    cmp #$14
        bne @n5
        inc LPOS
        lda LCH
        beq *+5
        jmp @nx
        iny
        BRD
        jsr mg_x
        sta WLX
        jmp @nx
@n5:    cmp #$15
        bne @n5t
        lda #8
        jmp @w
@n5t:   cmp #$17
        bne @n6
        iny
        iny
        BRD
        inc LPOS
        inc LPOS
        jmp @w
@n6:    cmp #$1A
        beq @n6s
        cmp #$1C
        beq @n6s
        cmp #$1D
        bne @n6f
@n6s:   inc LPOS
        jmp @nx
@n6f:   cmp #$18                ; Feld / Knopf getroffen?
        beq @fd
        cmp #$19
        beq *+5
        jmp @nx
@fd:    sta FXC
        iny
        BRD
        sta FXI
        inc LPOS
        ldx FXC
        jsr fld_w
        clc
        adc WLX
        sta HT
        lda HITX
        cmp WLX
        bcc @fn
        cmp HT
        bcs @fn
        lda #1
        ldx FXC
        cpx #$18
        beq @fk
        inc a
@fk:    sta HITK
        lda FXI
        sec
        rts
@fn:    lda HT
        sta WLX
        inc LCH
        bra @nx
@vis:   inc LCH
        jsr cw
        sta HT
        lda LFL
        and #1
        beq @nb
        inc HT
@nb:    lda HT
@w:     clc
        adc WLX
        sta HT                  ; rechter Rand des Zeichens
        lda HITX
        cmp WLX
        bcc @nx2
        cmp HT
        bcs @nx2
        lda LFL                 ; getroffen
        and #2
        beq @no
        stz HITK
        lda LLK
        sec
        rts
@nx2:   lda HT
        sta WLX
@nx:    inc LPOS
        lda LPOS
        jmp @c
@no:    clc
        rts

; ================= Tasten =================
web_key:
        jsr net_ptr0
        lda FFOC                ; Eingabefeld hat die Schreibmarke
        bmi @nf
        jmp f_key
@nf:    lda KEYC
        cmp #$1B
        bne @k1
        CALL close_win
        rts
@k1:    cmp #$12                ; Strg-R: neu laden
        bne @kd
        jmp web_reload
@kd:    cmp #$13                ; Strg-S: Seite als Datei speichern
        bne @kd2
        jmp web_save
@kd2:   cmp #$04                ; Strg-D: Favorit merken
        bne @kb
        jmp fav_add
@kb:    cmp #$02                ; Strg-B: Favoriten
        bne @kp
        jmp web_favs
@kp:    cmp #$10                ; Strg-P: Bild der Seite in Paint
        bne @k2
        JSLF xf_pt_img
        rts
@k2:    ldx ADRFOC
        bne @adr
        ; Seite blaettern
        cmp #$0A
        bne @k3
        lda #1
        bra @dn
@k3:    cmp #$0B
        bne @k4
        lda #1
        bra @up
@k4:    cmp #' '
        bne @k5
        lda NRV
        dec a
        bra @dn
@k5:    cmp #$08                ; Pfeil links: zurueck
        bne @k6
        jmp web_back
@k6:    cmp #$21                ; Buchstabe: in die Adresszeile
        bcc @r
        cmp #$7F
        bcs @r
        lda #1
        sta ADRFOC
        sta ADRSEL
        lda #$FFFF
        sta ADRCUR
        lda KEYC
        bra @adr
@r:     rts
@dn:    clc
        adc PAGE
        cmp MAXPG
        bcc @set
        lda MAXPG
        bra @set
@up:    sta HT
        lda PAGE
        sec
        sbc HT
        bpl @set
        lda #0
@set:   cmp PAGE
        beq @r
        sta PAGE
        CALL redraw_cont
        rts
@adr:   ; Adresszeile bearbeiten
        cmp #$0D
        bne @a1
        ldx TNON                ; Telnet/IRC: Zeile senden (ausser einer Adresse)
        beq @a0
        jsr tn_isurl
        bcc @a0
        jmp tn_line
@a0:
        stz ADRFOC
        stz ADRSEL
        ldx #WB_URL             ; Adresse -> Ziel
        ldy #WB_TURL
        jsr bs_copy
        stz WBACK
        jmp web_go
@a1:    lda #WB_URL             ; Zeileneditor (lineed.inc)
        sta LEB
        lda #250
        sta LEM
        lda ADRSEL              ; alles markiert?
        beq @ed
        stz ADRSEL
        lda KEYC
        cmp #$7F                ; Entf: alles weg
        beq @clr
        cmp #' '
        bcc @mv
        cmp #$7F
        bcs @mv
@clr:   ldy #WB_URL             ; Zeichen ersetzt die Markierung
        lda #0
        BWR
        stz ADRCUR
        lda KEYC
        cmp #$7F
        beq @rd
        bra @ed
@mv:    lda #$FFFF              ; Pfeile: Markierung weg, Schreibmarke ans Ende
        sta ADRCUR
@ed:    lda ADRCUR
        sta LEC
        jsr le_key
        lda LEC
        sta ADRCUR
        bcs @r2
@rd:    jsr adr_redraw
@r2:    rts

; nur die Adresszeile neu zeichnen
adr_redraw:
        lda NWIN
        dec a
        CALL recaddr
        sta PB
        ldy #R_X
        lda (PB),y
        clc
        adc #33
        sta KL
        ldy #R_W
        adc (PB),y
        sec
        sbc #38
        sta KR
        ldy #R_Y
        lda (PB),y
        clc
        adc #16
        sta KT
        clc
        adc #12
        sta KB
        CALL redraw_rect
        rts

; ================= Navigation =================
web_home:
        jsr home_turl
        stz WBACK
        jmp web_go

web_save:
        jsr net_ptr0
        ldx #WB_BASE
        ldy #WB_TURL
        jsr bs_copy
        lda #1
        sta WSAVE
        sta WBACK
        jmp web_go

web_favs:
        jsr net_ptr0
        ldx #0
        ldy #WB_TURL
@kc:    lda w_lzu,x
        BWR
        iny
        inx
        cpx #9
        bcc @kc
        stz WBACK
        jmp web_go

; ================= Symbolleiste rechts der Adresszeile =================
; 4 Knoepfe: neu laden, Favorit merken, Favoriten, speichern
TBARW = 4*14
; Knopf X (0..3) -> RX..RH
tb_rect:
        txa
        asl
        sta RX                  ; X*2
        asl
        asl
        asl                     ; X*16
        sec
        sbc RX                  ; X*14
        clc
        adc CW
        sec
        sbc #TBARW+3-WX
        sta RX
        lda #WY+16
        sta RY
        lda #13
        sta RW
        lda #12
        sta RH
        rts

tb_draw:
        ldx #0
@l:     phx
        jsr tb_rect
        jsr w_btn
        plx
        phx
        jsr tb_rect
        lda RX
        clc
        adc #3
        sta TBIX
        lda RY
        clc
        adc #2
        sta RY
        lda #BLACK
        CALL setcolor
        lda #1
        sta RW
        sta RH
        pla
        pha
        asl
        asl
        asl
        tay                     ; 8 Zeilen je Bild
        lda #8
        sta TBIR
@row:   lda tb_icons,y
        and #$00FF
        sta TBIM
        lda TBIX
        sta RX
@bit:   lda TBIM
        beq @nr
        asl
        bit #$0100
        php
        and #$00FF
        sta TBIM
        plp
        beq @nb
        phy
        CALL fillrect
        ply
@nb:    inc RX
        bra @bit
@nr:    inc RY
        iny
        dec TBIR
        bne @row
        plx
        inx
        cpx #4
        bcc @l
        rts

; C=1, X = Knopf, wenn der Klick einen Knopf trifft
tb_hit:
        ldx #0
@l:     phx
        jsr tb_rect
        CALL inrect
        plx
        bcs @r
        inx
        cpx #4
        bcc @l
        clc
@r:     rts

tb_do:  txa
        asl
        tax
        jmp (tb_jt,x)
tb_jt:  .word web_reload, fav_add, web_favs, web_save
        .pushseg
        .segment "RODATA"
tb_icons:
        .byte %00111010,%01000110,%10001110,%10000000,%10000001,%01000010,%00111100,0  ; neu laden
        .byte %00010000,%00010000,%11111110,%01111100,%00111000,%01101100,%11000110,0  ; Stern
        .byte %11111110,%10000010,%10111010,%10000010,%10111010,%10000010,%11111110,0  ; Liste
        .byte %11111100,%10100110,%10100010,%10000010,%10111010,%10111010,%11111110,0  ; Diskette
        .popseg

web_reload:
        jsr net_ptr0
        ldx #WB_BASE
        ldy #WB_TURL
        jsr bs_copy
        lda PAGE                ; Bildlage behalten
        sta WRPAGE
        lda #1
        sta WBACK
        jmp web_go

web_back:
        lda HISN
        bne @go
        rts
@go:    dec HISN
        jsr net_ptr0
        lda HISN
        jsr his_addr
        tax
        ldy #WB_TURL
        jsr bs_copy
        lda HISN
        asl
        tax
        lda HISP,x
        sta WRPAGE
        lda #1
        sta WBACK
        jmp web_go

; Verlaufseintrag A -> Offset
his_addr:
        xba
        lsr                     ; *128
        clc
        adc #WB_HIS
        rts

; Zeichenkette (Laengenbyte + Text) im Puffer von X nach Y kopieren
bs_copy:
        stx UI
        sty UJ
        txy
        BRD
        sta UN
        ldx #0
@l:     txa
        clc
        adc UI
        tay
        BRD
        pha
        txa
        clc
        adc UJ
        tay
        pla
        BWR
        inx
        cpx UN
        bcc @l
        beq @l
        rts

; aktuelle Seite in den Verlauf (aelteste faellt heraus)
his_push:
        JSLF xf_hist_add        ; auch in die Verlaufsliste (about:verlauf)
        ldy #WB_BASE
        BRD
        beq @r
        cmp #127
        bcs @r                  ; zu lang fuer den Verlauf
        lda HISN
        cmp #HISMAX
        bcc @put
        ldx #WB_HIS+128         ; nach vorn schieben
        ldy #WB_HIS
        jsr bs_copy
        ldx #WB_HIS+256
        ldy #WB_HIS+128
        jsr bs_copy
        lda HISP+2              ; Bildlage mitschieben
        sta HISP
        lda HISP+4
        sta HISP+2
        dec HISN
@put:   lda HISN
        jsr his_addr
        tay
        ldx #WB_BASE
        jsr bs_copy
        lda HISN
        asl
        tax
        lda PAGE
        sta HISP,x
        inc HISN
@r:     rts

; Ziel WB_TURL laden und anzeigen
web_go: jsr net_ptr0
        lda WBACK
        bne @nb
        jsr his_push
@nb:    jsr web_load
        stz WSAVE
        lda VTON                ; Vollbild-Terminal hat den Bildschirm
        beq @nvt
        rts
@nvt:   lda TNON                ; Telnet/IRC: Eingabe in der Adresszeile
        beq @nt
        stz WPOST
        jsr net_ptr0
        ldy #WB_URL
        lda #0
        BWR
        lda #1
        sta ADRFOC
        stz ADRSEL
        CALL redraw_top
        rts
@nt:    lda WBACK               ; zurueck: alte Bildlage wieder herstellen
        beq @np
        lda WRPAGE
        cmp NLINES
        bcs @np
        sta PAGE
@np:    stz WPOST
        stz ADRFOC
        stz ADRSEL
        CALL redraw_top         ; Fenster und Taskleiste (Titel)
        jmp img_fetch           ; dann die Bilder der Seite

; ================= Laden =================
web_load:
        stz UPDCHK
        stz NABORT
        stz WREDIR
        stz GIFON
        lda TNON                ; Telnet/IRC beenden
        beq @nt
        stz TNON
        JSLF xf_tcp_close
@nt:
web_again:                      ; (auch vom Telefonbuch: WB_TURL laden)
@again: jsr net_ptr0
        jsr url_parse
        bcc @p
        WS ws_e_url
        jmp err_page
@p:     ldx #WB_TURL            ; Adresszeile zeigt das Ziel
        ldy #WB_URL
        jsr bs_copy
        lda USCH
        cmp #2
        bne @n2
        jmp about_page
@n2:    WS ws_start
        ldx #0
        jsr st_set
        JSLF xf_net_start
        bcc @n3
        jmp net_err
@n3:    JSLF xf_ntp_once        ; einmal je Sitzung die Uhr stellen
        bcc @n3b
        jmp @again              ; Adresse neu zerlegen (WB_HOST benutzt)
@n3b:   stz NTLS
        stz WPROXY
        lda USCH                ; ssh://: Anmeldung im Terminal, dann FujiNet
        cmp #6
        bne @n3c
        jmp ssh_go
@n3c:   lda USCH                ; ftp://: nur Uthernet II und FujiNet
        cmp #7
        bne @n3d
        JSLF xf_ftp_pre
        bcc @n3d
        jmp ftp_pg
@n3d:
        lda USCH                ; https: Karte mit TLS (CoGS, FujiNet)
        cmp #1                  ; oder ueber einen Proxy (about:proxy=...)
        bne @ns
        lda NCAPS
        and #CAP_TLS
        bne @tls
        jsr px_setup
        bcc @px
        WS ws_e_tls
        jmp err_page
@px:    WS ws_look
        ldx #2
        jsr st_set
        JSLF xf_net_resolve
        bcc @px2
        jsr px_restore
        jmp net_err
@px2:   WS ws_conn
        ldx #2
        jsr st_set
        lda PXPORT
        sta DPORT
        JSLF xf_tcp_open
        php
        jsr px_restore
        plp
        bcc @co
        jmp net_err
@tls:   lda #1
        sta NTLS
@ns:    lda NCAPS               ; CoGS, FujiNet, Modem: Karte nimmt den Namen
        and #CAP_NAME
        bne @rs
        WS ws_look
        ldx #2
        jsr st_set
        JSLF xf_net_resolve
        bcc @rs
        jmp net_err
@rs:    WS ws_conn
        ldx #2
        jsr st_set
        lda WPORT
        sta DPORT
        JSLF xf_tcp_open
        bcc @co
        jmp net_err
@co:    lda USCH                ; Telnet/IRC: keine Anfrage
        cmp #7
        bne @co0
        jmp ftp_go
@co0:   cmp #4
        bcc @co1
        jmp tn_start
@co1:   jsr req_build
        stz WPOST               ; Weiterleitungen danach mit GET
        bcc @rq
        JSLF xf_tcp_close
        WS ws_e_url
        jmp err_page
@rq:    JSLF xf_tcp_write
        bcc @wr
        JSLF xf_tcp_close
        jmp net_err
@wr:    stz WPTR
        lda #$8000
        sta NLIM
        jsr web_prog
        lda #1                  ; volle Puffer: anhalten (fuer Downloads)
        sta NSTRM
        JSLF xf_tcp_readall
        stz NSTRM
        bcc @rd
        JSLF xf_tcp_close
        jmp net_err
@rd:    lda NFULL               ; alles da: Verbindung gleich schliessen
        bne @rd1
        JSLF xf_tcp_close
@rd1:   lda USCH                ; Gopher: kein Antwortkopf
        cmp #3
        bne @rd2
        jmp gop_page
@rd2:   lda WPTR
        sta RAWEND
        bne @hd
        WS ws_e_empty
        jmp err_page
@hd:    jsr net_ptr0
        jsr hdr_parse
        lda WSAVE               ; Speichern: wie ein Download offen lassen
        bne @hd2
        lda NFULL               ; noch offen: fuer Download oder lange Textseite
        beq @hd2
        lda HCODE
        cmp #200
        bcc @hcl
        cmp #300
        bcc @hk
        lda HTYPOK              ; Fehlerseite des Servers (4xx/5xx) als Text
        beq @hcl
        lda HCODE
        cmp #400
        bcs @hk
        bra @hcl
@hk:    lda HTYPOK              ; Textseite: nur ohne "chunked" stueckweise
        beq @hd2
        lda HCHUNK
        beq @hd2
@hcl:   JSLF xf_tcp_close
        stz NFULL
@hd2:
        lda HLOC                ; Weiterleitung?
        beq @nr
        lda HCODE
        cmp #300
        bcc @nr
        cmp #400
        bcs @nr
        inc WREDIR
        lda WREDIR
        cmp #6
        bcc @rj
        WS ws_e_redir
        jmp err_page
@rj:    ldx #WB_TURL            ; relativ zur bisherigen Adresse
        ldy #WB_BASE
        jsr bs_copy
        jsr url_join
        bcc @rj2
        WS ws_e_url
        jmp err_page
@rj2:   jmp @again
@nr:    lda HCODE
        cmp #400
        bcc @ok
        lda BEND                ; Fehlerseite des Servers zeigen, wenn es eine gibt
        sec
        sbc HEND
        cmp #40
        bcs @ok
        WS ws_e_http
        jmp err_code
@ok:    lda WSAVE               ; Strg-S: Seite unveraendert speichern
        beq @ok1
        stz WSAVE
        lda HCODE
        cmp #300
        bcs @ok1
        lda HCHUNK
        beq @sv
        jsr dechunk
@sv:    jmp dl_run
@ok1:   lda HTYPOK
        bne @ty
        lda HCODE               ; keine Textseite: herunterladen
        cmp #300
        bcs @et
        lda NFULL               ; GIF (ganz im Puffer): anzeigen
        bne @dl
        jsr is_gif
        bcs @dl
        lda HCHUNK
        beq @gf
        jsr dechunk
@gf:    jmp gif_show
@dl:    jmp dl_run
@et:    WS ws_e_type
        jmp err_type
@ty:    lda NFULL               ; laenger als der Puffer: stueckweise
        beq @ty1
        jmp big_page
@ty1:   lda HCHUNK
        beq @nc
        jsr dechunk
@nc:    lda UPDCHK              ; about:update: Versionsdatei auswerten
        beq @nc0
        jsr upd_page
@nc0:   WS ws_fmt
        ldx #0
        jsr st_set
        jsr net_ptr0
        lda HEND
        sta HR
        lda BEND
        sta HE
        JSLF xf_pf_init         ; Software-Archiv / Feed: umbauen
        JSLF xf_pf_run
        jsr net_ptr0
        jsr h2t
        jmp page_done

; ftp://: Steuerverbindung steht (FujiNet: Daten kommen gleich)
ftp_go: JSLF xf_ftp_run
        bcc @rd
        cmp #$FFFF
        beq ftp_pg
        JSLF xf_tcp_close
        jmp net_err
@rd:    stz WPTR
        lda #$8000
        ldx FTDIR
        beq @f
        lda #$3F00
@f:     sta NLIM
        jsr web_prog
        lda FTDIR
        eor #1
        sta NSTRM               ; Datei: bei vollem Puffer anhalten
        JSLF xf_tcp_readall
        stz NSTRM
        bcc @ok
        JSLF xf_tcp_close
        jmp net_err
@ok:    lda FTDIR
        bne @ls
        lda NFULL
        bne @dl
        JSLF xf_tcp_close
@dl:    stz HEND
        jmp dl_run
@ls:    JSLF xf_tcp_close
        JSLF xf_ftp_list
ftp_pg: jsr net_ptr0
        jsr h2t
        jmp page_done

; Seite groesser als 32 KB: umwandeln, Rest nach vorn, weiterlesen
big_page:
        WS ws_fmt
        ldx #0
        jsr st_set
        jsr net_ptr0
        jsr h2t_init
        JSLF xf_pf_init
        jsr net_ptr0
        lda HEND
        sta HR
@chunk: lda WPTR                ; Schnitt vor dem letzten '<' (ganze Tags)
        sta HE
        lda NFULL
        bne *+5
        jmp @last
        ldy WPTR
        ldx #2048
@s:     dey
        cpy HR
        bcc @cut
        BRD
        cmp #'<'
        beq @lt
        dex
        bne @s
        bra @cut
@lt:    cpy HR                  ; kein Fortschritt (riesiges Tag): alles nehmen
        beq @cut
        sty HE
@cut:   lda #1
        sta HMORE
        lda HE
        sta PFHE
        JSLF xf_pf_run
        jsr net_ptr0
        jsr h2t_lp
        lda PFHE
        sta HE
        lda HFULL               ; Textbereich voll: Rest verwerfen
        bne @stop
        ldx #0                  ; Rest HE..WPTR nach vorn
        ldy HE
@mv:    cpy WPTR
        bcs @mvd
        BRD
        phy
        txy
        BWR
        ply
        inx
        iny
        bra @mv
@mvd:   stx WPTR
        stz HR
        lda #$8000
        sta NLIM
        lda #1
        sta NSTRM
        JSLF xf_tcp_readall
        stz NSTRM
        bcs @err
        jsr net_ptr0
        brl @chunk
@err:   lda #0                  ; Fehler mitten in der Seite: zeigen, was da ist
        sta NFULL
        stz HMORE
        jsr h2t_fin
        JSLF xf_tcp_close
        jmp page_done
@stop:  JSLF xf_tcp_close      ; "gekuerzt" steht schon da
        stz NFULL
        stz HMORE
        jmp page_done
@last:  stz HMORE
        JSLF xf_pf_run
        jsr net_ptr0
        jsr h2t_lp
        jmp page_done

; Netzwerkfehler NERR -> Fehlerseite
net_err:
        lda NERR
        cmp #NE_CARD
        bne @1
        WS ws_e_card
        jmp err_page
@1:     cmp #NE_DNS
        bne @2
        WS ws_e_dns
        jmp err_host
@2:     cmp #NE_CONN
        bne @3
        WS ws_e_conn
        jmp err_page
@3:     cmp #NE_ABRT
        bne @4
        WS ws_e_abrt
        jmp err_page
@4:     cmp #NE_MEM
        bne @5
        WS ws_e_mem
        jmp err_page
@5:     cmp #NE_LINK
        bne @6
        WS ws_e_link
        jmp err_page
@6:     cmp #NE_TLS
        bne @7
        WS ws_e_tlsf
        jmp err_page
@7:     cmp #NE_CERT
        bne @9
        WS ws_e_cert
        jmp err_host
@9:     cmp #NE_MARI
        bne @10
        WS ws_e_mari
        jmp err_page
@10:    cmp #NE_MDM
        bne @8
        WS ws_e_mdm
        jmp err_page
@8:     WS ws_e_time
        jmp err_page

; Seite fertig: Zeilen, Basis-Adresse, Anzeige
page_done:
        jsr net_ptr0
        ldx #WB_TURL
        ldy #WB_BASE
        jsr bs_copy
        lda CW
        sec
        sbc #30
        sta WRAPW
        jsr lay_all
        stz PAGE
        stz HPRE
        WS ws_ready
        sta STMSG
        stz STXK
        rts

; ---------- Fehlerseiten (als HTML erzeugt) ----------
; A = Meldung; err_host haengt den Rechnernamen an, err_code den HTTP-Status,
; err_type den Inhaltstyp
err_page:
        stz UK
        bra err_go
err_host:
        ldx #1
        stx UK
        bra err_go
err_code:
        ldx #2
        stx UK
        bra err_go
err_type:
        ldx #3
        stx UK
err_go: sta UN
        jsr net_ptr0
        stz HO
        lda #w_eh1
        jsr raw_m
        WS ws_err
        jsr raw_x
        lda #w_eh2
        jsr raw_m
        WS ws_err
        jsr raw_x
        lda #w_eh3
        jsr raw_m
        lda UN
        jsr raw_x
        lda UK
        cmp #1
        bne @c2
        ldx #WB_HOST
        jsr raw_b
        bra @e
@c2:    cmp #2
        bne @c3
        lda HCODE
        jsr raw_num
        bra @e
@c3:    cmp #3
        bne @e
        ldx #WB_CTYP
        jsr raw_b
@e:     lda #w_eh4
        jsr raw_m
        ldx #WB_TURL
        jsr raw_b
        stz HPLAIN
        lda HO
        sta HE
        stz HR
        jsr h2t
        jsr page_done
        WS ws_err
        sta STMSG
        rts

        .pushseg
        .segment "RODATA"
w_eh1:  .byte 7,"<title>"
w_eh2:  .byte 12,"</title><h2>"
w_eh3:  .byte 8,"</h2><p>"
w_eh4:  .byte 10,"</p><p><i>"
        .popseg

; Zeichenkette der Hauptbank (Laengenbyte) an A in den Rohpuffer (HO)
raw_m:  sta UI
        tax
        lda a:0,x
        and #$00FF
        sta UJ
@l:     lda UJ
        beq @r
        inc UI
        ldx UI
        lda a:0,x
        jsr raw_c
        dec UJ
        bra @l
@r:     rts
; dasselbe fuer Texte dieser Codebank (Umlaute werden zu Latin-1)
raw_x:  sta PDST
        CBANK
        sta PDST+2
        lda [PDST]
        and #$00FF
        sta UJ
        ldy #1
@l:     lda UJ
        beq @r
        lda [PDST],y
        and #$00FF
        cmp #$80
        bcc @a
        cmp #$87
        bcs @a
        sec
        sbc #$80
        tax
        lda w_l1,x
        and #$00FF
@a:     phy
        jsr raw_c
        ply
        iny
        dec UJ
        bra @l
@r:     rts
        .pushseg
        .segment "RODATA"
w_l1:   .byte $E4,$F6,$FC,$C4,$D6,$DC,$DF
        .popseg
; Zeichenkette im Puffer bei X in den Rohpuffer
raw_b:  stx UI
        txy
        BRD
        sta UJ
@l:     lda UJ
        beq @r
        inc UI
        ldy UI
        BRD
        cmp #'<'                ; nichts als HTML deuten
        bne @c
        lda #'('
@c:     jsr raw_c
        dec UJ
        bra @l
@r:     rts
; Zeichen A an HO
raw_c:  ldy HO
        cpy #WB_TXT-16
        bcs @r
        BWR
        inc HO
@r:     rts
; Zahl A dezimal
raw_num:
        sta UI
        ldx #100
        jsr @d
        ldx #10
        jsr @d
        lda UI
        clc
        adc #'0'
        jmp raw_c
@d:     stx UJ
        ldy #0
        lda UI
@l:     cmp UJ
        bcc @o
        sbc UJ
        iny
        bra @l
@o:     sta UI
        tya
        clc
        adc #'0'
        jmp raw_c

; Startseite (in dieser Codebank) als HTML laden
about_page:
        jsr net_ptr0
        jsr about_cmd           ; about:fav usw.
        bcs @st
        lda BBSGO               ; Telefonbuch: Verbindung aufbauen
        beq @ne
        stz BBSGO
        jmp web_again
@ne:    jmp @e
@st:    jsr net_ptr0
        stz HO
        lda #w_start
        ldx LANG
        beq @de
        lda #w_start_en
@de:    sta PDST
        CBANK
        sta PDST+2
        stz HPLAIN
        ldy #0
@l:     lda [PDST],y
        and #$00FF
        beq @e
        phy
        jsr raw_c
        ply
        iny
        bra @l
@e:     lda HO
        sta HE
        stz HR
        jsr h2t
        jmp page_done

        .include "webdata.inc"
        .include "webstr.inc"         ; Texte (auch fuer net.s: Bank in STRBK)
        .include "dosstr2.inc"        ; lange Texte der Eingabeaufforderung (dos.inc)
        .include "url.inc"
        .include "cookie.inc"
        .include "lineed.inc"
        .include "html.inc"
        .include "forms.inc"
        .include "fav.inc"
        .include "bbs.inc"
        .include "dl.inc"
        .include "gopher.inc"
        .include "gif.inc"
        .include "term.inc"
        .include "vt.inc"
        .include "proxy.inc"

FARMODE .set 0
        .segment "CODE"
