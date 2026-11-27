; ================================================================
; GS/OS95: Textvorschau im Eigenschaften-Fenster (zweite Codebank)
; Liest die ersten Bytes einer Textdatei (Typ $04/$B0) nach pvbuf.
; ================================================================
        .segment "XCODE"
FARMODE .set 1
PVMAX = 80
PVX   = WX+40
PVY   = WY+105
xf_pv_load:
        jsr pv_load
        rtl
xf_pv_none:
        stz PVN
        stz PVIMG
        rtl
pv_load:
        stz PVN
        stz PVIMG
        lda MODE
        bne @f
        rts
@f:     lda SEL2
        CALL entep
        lda [EP]
        cmp #$00C1              ; Super-Hi-Res-Bild
        beq @img
        cmp #$00C0              ; gepackt: $0001 PackBytes, $0002 APF
        bne @nimg
        ldy #E_AUX
        lda [EP],y
        beq @nimg
        cmp #$0003
        bcs @nimg
@img:   jmp pv_img
@nimg:  lda [EP]
        cmp #$0004
        beq @txt
        cmp #$00B0
        beq @txt
        rts
@txt:   lda open_pb+4           ; Datei srcpath oeffnen (Parameterblock kurz umbiegen)
        pha
        lda #srcpath
        sta open_pb+4
        GSOS $2010,open_pb
        pla
        sta open_pb+4
        bcc @op
        rts
@op:    lda open_pb+2
        sta io_pb+2
        sta cl_pb+2
        lda io_pb+4             ; in den eigenen Puffer lesen, Zeiger danach zurueck
        pha
        lda io_pb+6
        pha
        lda io_pb+8
        pha
        lda #pvbuf
        sta io_pb+4
        lda MYBANK
        and #$00FF
        sta io_pb+6
        lda #PVMAX
        sta io_pb+8
        stz io_pb+10
        GSOS $2012,io_pb
        lda io_pb+12
        bcc @rd
        cmp #1                  ; Dateiende vor 80 Bytes ist kein Fehler
        bcs @rd
        lda #0
@rd:    cmp #PVMAX+1
        bcc @n
        lda #PVMAX
@n:     sta PVN
        pla
        sta io_pb+8
        pla
        sta io_pb+6
        pla
        sta io_pb+4
        GSOS $2014,cl_pb
        rts

xf_pv_draw:
        jsr pv_draw
        rtl
pv_draw:
        lda PVIMG
        beq @txt
        jmp pv_drawimg
@txt:   lda PVN
        bne @go
        rts
@go:    lda #PVX
        sta RX
        lda #PVY
        sta RY
        lda #WW-52
        sta RW
        lda #24
        sta RH
        lda #DKGRAY
        CALL setcolor
        CALL fillrect
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #WHITE
        CALL setcolor
        CALL fillrect
        lda #PVX+WW-56
        clc
        adc DXO
        sta CLIPR
        lda #BLACK
        CALL setcolor
        stz PVI                 ; Leseposition
        lda #PVY+3
        sta PVL
@line:  ldx PVI                 ; Zeile: bis CR, hoechstens 40 Zeichen; Steuerzeichen -> Leerzeichen
        stx PVS
        ldy #0
@c:     cpx PVN
        bcs @eol
        lda pvbuf,x
        and #$007F
        cmp #$0D
        beq @cr
        cmp #$20
        bcs @pr
        lda #$20
@pr:    sep #$20
        sta pvbuf,x
        rep #$20
        inx
        iny
        cpy #40
        bcc @c
        bra @eol
@cr:    inx
@eol:   stx PVI
        sty STRN
        lda #pvbuf
        clc
        adc PVS
        sta STRP
        lda #PVX+3
        sta SX
        lda PVL
        sta SY
        lda STRN
        beq @nd
        CALL drawstr
@nd:    lda PVL
        clc
        adc #9
        sta PVL
        cmp #PVY+3+18
        bcs @done
        lda PVI
        cmp PVN
        bcc @line
@done:  lda #320
        sta CLIPR
        rts

; ---------- Bildvorschau: 320x200-Bild auf 40x25 verkleinert ----------
PVTW = 40
PVTH = 25
; Datei srcpath lesen, jede 8. Zeile/Spalte, Farben auf unsere Palette abbilden
pv_img: JSLF xf_pic_load         ; ganzes Bild (auch gepackt) in den Speicher
        bcc @ld
        rts
@ld:    lda PICP
        sta PSRC
        lda PICP+2
        sta PSRC+2
        lda #pvthumb            ; Sprite in der Grafikbank: Breite, Hoehe, Pixel
        sta LP2
        lda ABANK
        sta LP2+2
        lda #PVTW+PVTH*256
        sta [LP2]
        stz PVR
@row:   ldy #0
@px:    phy                     ; Pixel (x*8, r*8) = oberes Halbbyte von Byte r*1280+x*4
        lda PVR
        ldx #1280
        CALL gmul
        sta PVB
        lda 1,s
        asl
        asl
        clc
        adc PVB
        tay
        lda [PSRC],y
        and #$00F0
        lsr
        lsr
        lsr
        lsr
        pha
        lda PVR                 ; Ziel: 2 + Zeile*40 + x
        ldx #PVTW
        CALL gmul
        clc
        adc 3,s
        inc a
        inc a
        tay
        pla
        sep #$20
        sta [LP2],y
        rep #$20
        ply
        iny
        cpy #PVTW
        bcc @px
        inc PVR
        lda PVR
        cmp #PVTH
        bcc @row
        ; Farben umsetzen: je Zeile Palette aus dem SCB der Bildzeile r*8
        lda #$FFFF
        sta PVPAL
        stz PVR
@mr:    lda PVR
        asl
        asl
        asl
        clc
        adc #32000
        tay
        lda [PSRC],y
        and #$000F
        cmp PVPAL
        beq @mok
        sta PVPAL
        jsr pv_map              ; pvmap[0..15] fuer diese Palette
@mok:   lda PVR
        ldx #PVTW
        CALL gmul
        inc a
        inc a
        tay
        ldx #PVTW
@mp:    lda [LP2],y
        and #$000F
        phx
        tax
        lda pvmap,x
        plx
        sep #$20
        sta [LP2],y
        rep #$20
        iny
        dex
        bne @mp
        inc PVR
        lda PVR
        cmp #PVTH
        bcc @mr
        lda #1
        sta PVIMG
        JSLF xf_pic_free
        rts

; Palette PVPAL des Bildes (ab Byte 256 im Puffer) -> naechste unserer 16 Farben
pv_map: stz PVC
@c:     lda PVPAL               ; Farbe PVC des Bildes: $0RGB
        asl
        asl
        asl
        asl
        clc
        adc PVC
        asl
        clc
        adc #32256
        tay
        lda [PSRC],y
        sta PVRGB
        lda #$7FFF
        sta PVBD
        ldx #0
@k:     lda palettes,x          ; unsere Farbe x/2
        jsr pv_dist
        cmp PVBD
        bcs @kn
        sta PVBD
        txa
        lsr
        sta PVBK
@kn:    inx
        inx
        cpx #32
        bcc @k
        lda PVC
        tax
        lda PVBK
        sep #$20
        sta pvmap,x
        rep #$20
        inc PVC
        lda PVC
        cmp #16
        bcc @c
        rts
; Abstand der Farbe A zu PVRGB (Summe der Betraege von R, G, B)
pv_dist:
        sta PVQ
        stz PVS2
        lda #$0F00
        jsr @one
        lda #$00F0
        jsr @one
        lda #$000F
        jsr @one
        lda PVS2
        rts
@one:   sta PVMK
        lda PVQ
        and PVMK
        sta PVT1
        lda PVRGB
        and PVMK
        sec
        sbc PVT1
        bpl @p
        eor #$FFFF
        inc a
@p:     cmp #$0100              ; auf 0..15 bringen
        bcc @s1
        xba
        and #$00FF
@s1:    cmp #$0010
        bcc @s2
        lsr
        lsr
        lsr
        lsr
@s2:    clc
        adc PVS2
        sta PVS2
        rts

pv_drawimg:
        lda #PVX                ; schwarzer Rahmen, Bild mittig
        sta RX
        lda #PVY
        sta RY
        lda #WW-52
        sta RW
        lda #PVTH+2
        sta RH
        lda #BLACK
        CALL setcolor
        CALL fillrect
        lda #PVX+(WW-52)/2-PVTW/2
        sta SX
        lda #PVY+1
        sta SY
        lda #pvthumb
        sta SPR
        CALL blit4
        rts

pvthumb: .res 2+PVTW*PVTH
FARMODE .set 0
        .segment "CODE"
