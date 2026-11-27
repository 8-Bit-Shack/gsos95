        .segment "XCODE2"
FARMODE .set 2
xf_act_eject:
        jsr act_eject
        rtl
xf_dblclick:
        jsr dblclick
        rtl
xf_open_doc:
        jsr open_doc
        rtl
xf_set_palettes:
        jsr set_palettes
        rtl
xf_view_key:
        jsr view_key
        rtl
xf_wc_view:
        jsr wc_view
        rtl
; ================================================================
; GS/OS95: Dokumente oeffnen (Text-Editor, SHR-Bilder), Doppelklick,
; Laufwerk auswerfen
; ================================================================
VLINES  = 12

; Doppelklick? A = Element. C=1 wenn innerhalb 30 Ticks auf dasselbe Element
dblclick:
        cmp LCLKI
        sta LCLKI
        bne @no
        lda EVREC+6
        sec
        sbc LCLKT
        cmp DBLT
        bcs @no
        lda EVREC+6
        sta LCLKT
        lda #$FFFF
        sta LCLKI               ; drittes Klicken zaehlt neu
        sec
        rts
@no:    lda EVREC+6
        sta LCLKT
        clc
        rts

; ---------- Datei nach Typ oeffnen (EP = Eintrag, Pfad im Explorer) ----------
; C=1 wenn behandelt
open_doc:
        lda [EP]
        cmp #$0004              ; TXT
        beq @txt
        cmp #$00B0              ; SRC
        beq @txt
        cmp #$00C1              ; Super-Hi-Res-Bild
        beq @pic
        cmp #$00C0              ; gepacktes Bild
        beq @pic
        cmp #$00D8              ; Klangdatei: abspielen
        bne @nsd
        JSLF xf_snd_file
        sec
        rts
@nsd:   clc
        rts
@txt:   CALL entry_path         ; Text: im Editor oeffnen
        JSLF xf_ed_file
        sec
        rts
@pic:   CALL entry_path
        jsr show_picture
        sec
        rts

; ---------- Text laden: pathbuf -> Kopierpuffer, VLEN, COUNT (Zeilen) ----------
view_load:
        stz VLEN
        GSOS $2010,open_pb
        bcs @r
        lda open_pb+2
        sta io_pb+2
        sta cl_pb+2
        lda #COPYBUF-2
        sta io_pb+8
        GSOS $2012,io_pb
        lda io_pb+12
        bcc @ok
        lda #0
@ok:    sta VLEN
        GSOS $2014,cl_pb
@r:     ; Zeilen zaehlen
        lda #1
        sta COUNT
        ldy #0
@c:     cpy VLEN
        bcs @e
        lda [CBP],y
        and #$00FF
        cmp #$0D
        bne @n
        inc COUNT
@n:     iny
        bra @c
@e:     rts

; Fensterinhalt Text-Editor
wc_view:
        TEXT WX+7,WY+18,t_menubar,BLACK
        SUNKEN WX+4,WY+28,WW-8,WH-32
        FILL WX+6,WY+30,WW-12,WH-36,WHITE
        jsr view_load
        lda PAGE
        cmp COUNT
        bcc @p
        lda COUNT
        dec a
        sta PAGE
@p:     ; Anfang der Zeile PAGE suchen
        ldy #0
        ldx PAGE
@s:     cpx #0
        beq @found
@s2:    cpy VLEN
        bcs @found
        lda [CBP],y
        iny
        and #$00FF
        cmp #$0D
        bne @s2
        dex
        bra @s
@found: sty VOFF
        lda #BLACK
        CALL setcolor
        lda #WX+WW-8
        clc
        adc DXO
        sta CLIPR
        stz VLN
@line:  lda VLN
        asl
        asl
        asl
        sta TMP
        lda VLN
        asl
        clc
        adc TMP                 ; *10
        clc
        adc #WY+32
        sta SY
        lda #WX+9
        sta SX
@ch:    ldy VOFF
        cpy VLEN
        bcs @eot
        lda [CBP],y
        iny
        sty VOFF
        and #$007F
        cmp #$0D
        beq @nl
        cmp #$09
        bne @nt
        lda #' '
@nt:    cmp #' '
        bcc @ch
        ldx SX
        cpx #WX+WW-10
        bcs @ch                 ; Rest der Zeile abschneiden
        CALL putch
        bra @ch
@nl:    inc VLN
        lda VLN
        cmp #VLINES
        bcc @line
@eot:   lda #320
        sta CLIPR
        rts

view_key:
        lda KEYC
        cmp #$1B
        bne @k1
        CALL win_close
        rts
@k1:    ldx #0
@l:     lda vkeys,x
        beq @r
        cmp KEYC
        beq @f
        inx
        inx
        inx
        inx
        bra @l
@f:     lda PAGE
        clc
        adc vkeys+2,x
        bpl @p
        lda #0
@p:     cmp COUNT
        bcc @s
        lda COUNT
        dec a
@s:     cmp PAGE
        beq @r
        sta PAGE
        CALL draw_window
        rts
@r:     rts
        .pushseg
        .segment "RODATA"
vkeys:  .word $0A,1, $0B,$FFFF, $15,VLINES-1, $08,$FFF4, 0

; ---------- SHR-Bild anzeigen (srcpath) ----------
        .popseg
show_picture:
        jsr pic_load
        bcc @ok
        cmp #0
        beq @fmt
        CALL fail
        rts
@fmt:   lda #9                  ; Bildformat nicht unterstuetzt
        CALL show_msg
        rts
@ok:    lda PICP                ; Bild in den Bildschirm kopieren
        sta PSRC
        lda PICP+2
        sta PSRC+2
        ldy #0
@c:     lda [PSRC],y
        tyx
        sta f:$E12000,x
        iny
        iny
        bpl @c
        jsr pic_free
pic_wait:
        ; auf Taste oder Klick warten
@w:     pea $0000
        pea $FFFF
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06
        pla
        lda EVREC
        cmp #1
        beq @done
        cmp #3
        bne @w
        lda EVREC+2             ; B: Bild in Paint bearbeiten
        and #$00DF
        cmp #'B'
        bne @done
        JSLF xf_pt_screen
        rts
@done:  jsr set_palettes
        CALL desktop_scb
        CALL redraw_all
        rts

; ---------- Bild laden: srcpath -> 32-KB-Bild im SHR-Format (PICP) ----------
; $C1 (unkomprimiert), $C0/$0001 (PackBytes), $C0/$0002 (Apple Preferred Format)
; C=0: PICP gueltig (danach pic_free). C=1: A = GS/OS-Fehler, A = 0: Format
xf_pic_load:
        jsr pic_load
        rtl
xf_pic_free:
        jsr pic_free
        rtl
pic_load:
        lda #$2006
        ldx #gfi_pb
        ldy #srcpath
        CALL gsosp
        bcc @g
        rts
@g:     lda gfi_pb+38           ; hoechstens 64 KB
        beq @sz
@fmt:   lda #0
        sec
        rts
@sz:    lda gfi_pb+36
        sta PSEND
        lda gfi_pb+8
        cmp #$00C1
        bne @c0
        stz PKIND
        bra @ok
@c0:    cmp #$00C0
        bne @fmt
        lda gfi_pb+10
        cmp #$0001
        beq @k
        cmp #$0002
        bne @fmt
@k:     sta PKIND
@ok:    lda #0                  ; 32-KB-Puffer fuer das Bild
        ldx #$8000
        jsr pic_new
        bcc @a1
        rts
@a1:    sta PICH
        stx PICH+2
        lda PSRC
        sta PICP
        lda PSRC+2
        sta PICP+2
        lda PSEND               ; Datei in einen eigenen Block lesen
        bne @nz
        jmp @fmt0
@nz:    tax
        lda #0
        jsr pic_new
        bcc @a2
        pha
        jsr pic_free
        pla
        sec
        rts
@a2:    sta PFH
        stx PFH+2
        lda PSRC
        sta PFP
        lda PSRC+2
        sta PFP+2
        lda #srcpath
        sta o4_pb+4
        lda #1
        sta o4_pb+8
        stz o4_pb+10
        GSOS $2010,o4_pb
        bcc *+5
        jmp @err
        lda o4_pb+2
        sta io_pb+2
        sta cl_pb+2
        lda io_pb+4
        pha
        lda io_pb+6
        pha
        lda PFP
        sta io_pb+4
        lda PFP+2
        sta io_pb+6
        lda PSEND
        sta io_pb+8
        GSOS $2012,io_pb
        php
        pha
        GSOS $2014,cl_pb
        pla
        plp
        plx
        stx io_pb+6
        plx
        stx io_pb+4
        bcs @err
        jsr pic_clear           ; schwarzes Bild, Palette 0
        lda PFP                 ; Quelle: Datei, Ziel: Bild
        sta PSRC
        lda PFP+2
        sta PSRC+2
        lda PICP
        sta PDST
        lda PICP+2
        sta PDST+2
        stz PSI
        stz PDI
        lda PKIND
        beq @raw
        cmp #1
        beq @pk
        jsr apf_load
        bcs @fmt1
        bra @done
@pk:    lda #32768
        sta PDEND
        jsr unpack
        bra @done
@raw:   ldy #0                  ; unkomprimiert: einfach kopieren
@r:     cpy PSEND
        bcs @done
        lda [PSRC],y
        sta [PDST],y
        iny
        iny
        bpl @r
@done:  jsr pf_free
        clc
        rts
@fmt1:  jsr pf_free
@fmt0:  jsr pic_free
        lda #0
        sec
        rts
@err:   pha
        jsr pf_free
        jsr pic_free
        pla
        sec
        rts

; Block X:A Bytes (X = unteres Wort, A = oberes) anlegen -> Handle A/X, Zeiger PSRC
pic_new:
        sta PNH
        pha                     ; Ergebnis
        pha
        lda PNH
        pha                     ; Groesse (lang)
        phx
        lda USERID
        pha
        pea $C010               ; fest, gesperrt
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        bcc @ok
        plx
        plx
        sec
        rts
@ok:    pla
        sta PNH
        plx
        stx PNH+2
        sta LP2                 ; Handle -> Zeiger
        stx LP2+2
        ldy #2
        lda [LP2],y
        sta PSRC+2
        lda [LP2]
        sta PSRC
        lda PNH
        ldx PNH+2
        clc
        rts

pic_free:
        lda PICH
        ora PICH+2
        beq @r
        lda PICH+2
        pha
        lda PICH
        pha
        TOOL $1002              ; DisposeHandle
        stz PICH
        stz PICH+2
@r:     rts
pf_free:
        lda PFH
        ora PFH+2
        beq @r
        lda PFH+2
        pha
        lda PFH
        pha
        TOOL $1002
        stz PFH
        stz PFH+2
@r:     rts

; Bild loeschen: alle Bytes 0
pic_clear:
        lda PICP
        sta PDST
        lda PICP+2
        sta PDST+2
        lda #0
        ldy #0
@l:     sta [PDST],y
        iny
        iny
        bpl @l
        rts

; PackBytes entpacken: Quelle PSRC+PSI (bis PSEND), Ziel PDST+PDI (bis PDEND)
unpack:
@l:     lda PDI
        cmp PDEND
        bcc *+3
        rts
        lda PSI
        cmp PSEND
        bcc *+3
        rts
        jsr ub_get              ; Kennbyte
        pha
        and #$003F
        inc a
        sta PCNT
        pla
        and #$00C0
        beq @lit
        cmp #$0040
        beq @rep1
        cmp #$0080
        beq @rep4
        asl PCNT                ; 11: ein Byte (Anzahl*4)-mal
        asl PCNT
@rep1:  jsr ub_get
        sta PB0
@r1:    lda PB0
        jsr ub_put
        dec PCNT
        bne @r1
        bra @l
@lit:   jsr ub_get
        jsr ub_put
        dec PCNT
        bne @lit
        bra @l
@rep4:  jsr ub_get              ; Muster aus 4 Bytes
        sta PB0
        jsr ub_get
        sta PB0+1
        jsr ub_get
        sta PB0+2
        jsr ub_get
        sta PB0+3
@r4:    ldx #0
@r4b:   lda PB0,x
        phx
        jsr ub_put
        plx
        inx
        cpx #4
        bcc @r4b
        dec PCNT
        bne @r4
        jmp @l
; naechstes Quellbyte -> A (0, wenn zu Ende)
ub_get: ldy PSI
        cpy PSEND
        bcs @z
        inc PSI
        lda [PSRC],y
        and #$00FF
        rts
@z:     lda #0
        rts
; Byte A ins Ziel (nur bis PDEND)
ub_put: ldy PDI
        cpy PDEND
        bcs @r
        sep #$20
        sta [PDST],y
        rep #$20
        inc PDI
@r:     rts

; Apple Preferred Format: Block "MAIN" suchen und auspacken. C=1: nicht gefunden
apf_load:
        stz PSI
@blk:   lda PSI                 ; Blockkopf: Laenge (lang), Name (Pascal)
        clc
        adc #9
        bcs @no
        cmp PSEND
        bcs @no
        ldy PSI
        iny
        iny
        lda [PSRC],y
        bne @no                 ; Block ueber 64 KB
        dey
        dey
        lda [PSRC],y
        sta PBLEN
        cmp #9
        bcc @no
        iny
        iny
        iny
        iny
        lda [PSRC],y
        and #$00FF
        cmp #4
        bne @nx
        iny
        lda [PSRC],y
        cmp #'M'+'A'*256
        bne @nx
        iny
        iny
        lda [PSRC],y
        cmp #'I'+'N'*256
        beq @main
@nx:    lda PSI
        clc
        adc PBLEN
        bcs @no
        sta PSI
        bra @blk
@no:    sec
        rts
@main:  lda PSI                 ; Daten nach Laenge (4) + Name (5)
        clc
        adc #9
        tay
        lda [PSRC],y            ; MasterMode
        sta PMODE
        iny
        iny
        lda [PSRC],y            ; Punkte pro Zeile
        sta PPPL
        iny
        iny
        lda [PSRC],y            ; Anzahl Farbtabellen
        sta PNCT
        iny
        iny
        ; Farbtabellen (hoechstens 16) nach Bild+32256
        stz PCNT
        lda #32256
        sta PDI
@ct:    lda PCNT
        cmp PNCT
        bcs @ctd
        ldx #16
@cw:    lda PCNT
        cmp #16
        bcs @cs
        lda [PSRC],y
        phy
        ldy PDI
        sta [PDST],y
        ply
        inc PDI
        inc PDI
@cs:    iny
        iny
        dex
        bne @cw
        inc PCNT
        bra @ct
@ctd:   lda [PSRC],y            ; Anzahl Zeilen
        sta PNSL
        iny
        iny
        sty PDIR                ; Zeilenverzeichnis
        lda PNSL
        asl
        asl
        clc
        adc PDIR
        sta PSI                 ; gepackte Zeilen
        ; Bytes pro Zeile
        lda PPPL
        inc a
        lsr
        pha
        lda PMODE               ; 640er Modus: 4 Punkte je Byte
        and #$0080
        beq @w3
        pla
        inc a
        lsr
        bra @w320
@w3:    pla
@w320:  cmp #161
        bcc *+5
        lda #160
        sta PBPL
        stz PLINE                ; Zeile
@ln:    lda PLINE
        cmp PNSL
        bcs @end
        cmp #200
        bcs @end
        asl
        asl
        clc
        adc PDIR
        tay
        lda [PSRC],y            ; gepackte Laenge
        sta PBLEN
        iny
        iny
        lda [PSRC],y            ; Modus: unteres Byte = SCB
        pha
        lda PLINE
        clc
        adc #32000
        tay
        pla
        sep #$20
        sta [PDST],y
        rep #$20
        lda PSEND               ; Ende der Zeile im Quelltext
        pha
        lda PSI
        clc
        adc PBLEN
        bcs @ov
        cmp PSEND
        bcs @ov
        sta PSEND
@ov:    lda PLINE
        ldx #160
        CALL gmul
        sta PDI
        clc
        adc PBPL
        sta PDEND
        lda PSI
        clc
        adc PBLEN
        pha
        jsr unpack
        pla
        sta PSI
        pla
        sta PSEND
        inc PLINE
        bra @ln
@end:   clc
        rts

set_palettes:
        ldx #0
@pl:    lda palettes,x
        sta f:$E19E00,x
        inx
        inx
        cpx #512
        bcc @pl
        rts

; ---------- Laufwerk auswerfen ----------
act_eject:
        lda SEL2
        bpl @g
        rts
@g:     asl
        tax
        lda voldev,x
        sta dctl_pb+2
        GSOS $202E,dctl_pb      ; DControl: Auswerfen
        bcc @ok
        CALL fail
        rts
@ok:    stz VOLOK
        CALL reload_view
        CALL redraw_all
        rts

FARMODE .set 0
        .segment "CODE"
