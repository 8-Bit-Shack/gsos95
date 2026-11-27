        .segment "XCODE3"
FARMODE .set 3
xf_prog_begin:
        jsr prog_begin
        rtl
xf_prog_end:
        jsr prog_end
        rtl
xf_prog_file:
        jsr prog_file
        rtl
xf_prog_poll:
        jsr prog_poll
        rtl
xf_prog_step:
        jsr prog_step
        rtl
xf_wc_prog:
        jsr wc_prog
        rtl
; ================================================================
; GS/OS95: Fortschrittsanzeige beim Kopieren/Verschieben mit Abbrechen
; ================================================================
PRBX = WX+16
PRBY = WY+70
PRBW = WW-32
XABORTC = $FFFE                 ; Fehlercode "vom Benutzer abgebrochen"

; A = Anzahl Objekte, X = 0 kopieren / 1 verschieben
prog_begin:
        sta PRGN
        stx PRGM
        stz PRGI
        stz XABORT
        stz PRGNL
        stz PRGON
        lda #19
        CALL win_open
        bcs @r
        lda #1
        sta PRGON
        CALL redraw_win
@r:     rts

prog_end:
        lda PRGON
        beq @r
        stz PRGON
        CALL win_pop
@r:     rts

wc_prog:
        ICON WX+14,WY+28,i_folder
        lda PRGM
        bne @mv
        TEXT WX+40,WY+30,t_pcopy,BLACK
        bra @t
@mv:    cmp #2
        beq @dl
        TEXT WX+40,WY+30,t_pmove,BLACK
        bra @t
@dl:    TEXT WX+40,WY+30,t_pdel,BLACK
@t:     jsr prog_name
        SUNKEN PRBX,PRBY,PRBW,12
        jsr prog_bar
        BUTTON WX+WW-72,WY+WH-26,62,15
        TEXTC WX+WW-41,WY+WH-22,t_abbr,BLACK
        rts

prog_name:
        FILL WX+40,WY+44,WW-50,10,GRAY
        lda PRGNL
        beq @r
        lda #prgname
        sta STRP
        lda PRGNL
        sta STRN
        lda #WX+40
        sta SX
        lda #WY+44
        sta SY
        lda #BLACK
        CALL setcolor
        CALL drawstr
@r:     rts

prog_bar:
        FILL PRBX+2,PRBY+2,PRBW-4,8,WHITE
        FILL WX+16,WY+86,120,9,GRAY
        lda PRGN
        beq @r
        lda PRGI
        ldx #PRBW-4
        CALL gmul
        ldx PRGN
        CALL gdiv
        beq @tx
        sta RW
        lda #PRBX+2
        sta RX
        lda #PRBY+2
        sta RY
        lda #8
        sta RH
        lda #NAVY
        CALL setcolor
        CALL fillrect
@tx:    lda #WX+16
        sta SX
        lda #WY+86
        sta SY
        lda #BLACK
        CALL setcolor
        lda PRGI
        CALL drawnum
        LSPR t_pof
        CALL blit1
        CALL sprhdr              ; Breite von " von " weiterruecken
        and #$00FF
        clc
        adc SX
        sta SX
        lda PRGN
        CALL drawnum
@r:     rts

; Dialog sichtbar? C=1
prog_vis:
        lda PRGON
        beq @n
        lda WIN
        cmp #19
        bne @n
        sec
        rts
@n:     clc
        rts

; Name der Datei srcpath anzeigen
prog_file:
        jsr prog_vis
        bcs @go
        rts
@go:    lda srcpath
        tax
@f:     dex
        bmi @a
        lda srcpath+2,x
        and #$00FF
        cmp #':'
        bne @f
@a:     inx                     ; X = erstes Zeichen des Namens
        ldy #0
@c:     cpx srcpath
        bcs @e
        lda srcpath+2,x
        sep #$20
        sta prgname,y
        rep #$20
        inx
        iny
        cpy #30
        bcc @c
@e:     sty PRGNL
        CALL cur_hide
        jsr prog_name
        CALL cur_show
        rts

; ein Objekt fertig
prog_step:
        inc PRGI
        jsr prog_vis
        bcs @go
        rts
@go:    CALL cur_hide
        jsr prog_bar
        CALL cur_show
        rts

; Abbrechen gedrueckt (Escape, Apfel-Punkt oder Knopf)? C=1
prog_poll:
        lda PRGON
        bne @go
        clc
        rts
@go:    lda XABORT
        bne @ab
        pha
        pea $002A               ; Maus runter, Taste, Wiederholung
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        beq @no
        lda EVREC
        cmp #1
        beq @ms
        lda EVREC+2
        and #$00FF
        cmp #$1B
        beq @set
        cmp #'.'
        bne @no
        lda EVREC+14
        and #$0100              ; Apfel-Taste
        bne @set
@no:    clc
        rts
@ms:    lda EVREC+12
        sta MX
        lda EVREC+10
        sta MY
        jsr prog_vis
        bcc @no
        HIT WX+WW-72,WY+WH-26,62,15
        bcc @no
@set:   lda #1
        sta XABORT
@ab:    sec
        rts

FARMODE .set 0
        .segment "CODE"
