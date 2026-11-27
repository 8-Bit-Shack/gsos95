; ================================================================
; GS/OS95: Ziehen und Ablegen von Dateien und Ordnern mit der Maus
; gleiches Laufwerk = verschieben, anderes Laufwerk = kopieren,
; Papierkorb = verschieben
; ================================================================
DGW = 18
DGH = 16

; A = Eintrag, der gerade angeklickt wurde (im obersten Explorer)
drag_entry:
        sta DGIDX
        lda MODE
        bne @ok
        rts                     ; Laufwerke werden nicht gezogen
@ok:    lda EVREC+12
        sta DSX
        lda EVREC+10
        sta DSY
@w:     jsr dg_event
        lda EVREC
        cmp #2
        bne @w1
        rts                     ; nur geklickt
@w1:    lda EVREC+12
        sec
        sbc DSX
        bpl @x
        eor #$FFFF
        inc a
@x:     cmp #4
        bcs @start
        lda EVREC+10
        sec
        sbc DSY
        bpl @y
        eor #$FFFF
        inc a
@y:     cmp #4
        bcc @w
@start: ; Quelle(n) merken: alle markierten Eintraege
        JSLF xf_sel_list
        lda SLN
        bne @sl
        lda DGIDX
        sta sellist
        lda #1
        sta SLN
@sl:    ldy #dgents
        JSLF xf_sel_copy
        lda SLN
        sta DGN
        ldx #pathbuf
        ldy #dgdir
        jsr pcopy
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        ldy #R_ID
        lda (PB),y
        sta DGSRC
        jsr cur_hide
        jsr org0
        jsr dg_pos
        jsr xor_box
@loop:  jsr dg_event
        lda EVREC
        cmp #2
        beq @done
        lda DNX
        sta DOX
        lda DNY
        sta DOY
        jsr dg_pos
        lda DNX
        cmp DOX
        bne @mv
        lda DNY
        cmp DOY
        beq @loop
@mv:    lda DNX
        pha
        lda DNY
        pha
        lda DOX
        sta DNX
        lda DOY
        sta DNY
        jsr xor_box             ; alten Rahmen weg
        pla
        sta DNY
        pla
        sta DNX
        jsr xor_box
        bra @loop
@done:  jsr xor_box
        lda EVREC+12
        sta MX
        lda EVREC+10
        sta MY
        jsr orgr
        jmp drop_at

dg_event:
        pea $0000
        pea $FFFF
        pea $0000
        lda EVPTR
        pha
        TOOL $0A06              ; GetNextEvent
        pla
        rts

; Rahmenposition aus Mausposition
dg_pos: lda EVREC+12
        sec
        sbc #DGW/2
        bpl @x
        lda #0
@x:     cmp #320-DGW
        bcc @x2
        lda #320-DGW
@x2:    sta DNX
        lda EVREC+10
        sec
        sbc #DGH/2
        bpl @y
        lda #0
@y:     cmp #200-DGH
        bcc @y2
        lda #200-DGH
@y2:    sta DNY
        rts

; XOR-Rahmen DGW x DGH bei DNX,DNY
xor_box:
        stz TMPK
@h:     lda DNX
        clc
        adc TMPK
        tax
        ldy DNY
        jsr xorpix
        lda DNX
        clc
        adc TMPK
        tax
        lda DNY
        clc
        adc #DGH-1
        tay
        jsr xorpix
        inc TMPK
        inc TMPK
        lda TMPK
        cmp #DGW
        bcc @h
        lda #2
        sta TMPK
@v:     ldx DNX
        lda DNY
        clc
        adc TMPK
        tay
        jsr xorpix
        lda DNX
        clc
        adc #DGW-1
        tax
        lda DNY
        clc
        adc TMPK
        tay
        jsr xorpix
        inc TMPK
        inc TMPK
        lda TMPK
        cmp #DGH-1
        bcc @v
        rts

; ---------- Ablegen bei MX,MY ----------
drop_at:
        stz SYSSORT
        ; welches Fenster liegt oben unter der Maus?
        lda NWIN
        sta TMPK
@lw:    dec TMPK
        bpl @lw2
        jmp @desk
@lw2:   lda TMPK
        jsr win_hit
        bcc @lw
        lda TMPK
        jsr recaddr
        sta PB
        ldy #R_KIND
        lda (PB),y
        cmp #1
        beq @exp
        jmp redraw_all          ; anderes Fenster: nichts tun
@exp:   lda TMPK
        jsr win_front
        jsr cell_at
        sta DGHIT
        lda MODE
        bne @dir
        ; Arbeitsplatz: nur auf ein Laufwerk
        lda DGHIT
        bmi @none
        jsr entep
        lda [EP]
        cmp #$0100
        bne @none
        stz dstdir
        ldy #dstdir
        jsr pappend
        jmp @go
@none:  jmp redraw_all
@dir:   ldx #pathbuf
        ldy #dstdir
        jsr pcopy
        lda DGHIT
        bpl @hh
        jmp @go
@hh:    jsr entep
        lda [EP]
        cmp #$000F
        bne @go0
        ; auf einen Ordner (nicht auf sich selbst)
        lda NWIN
        dec a
        jsr recaddr
        sta PB
        ldy #R_ID
        lda (PB),y
        cmp DGSRC
        bne @sub
        lda DGHIT
        cmp DGIDX
        bne @sub
        jmp @go
@sub:   ldy #dstdir
        jsr pappend
@go0:
@go:    lda ISBIN
        sta DGBIN
        JSLF xf_sys_chk         ; auf den Systemordner: einsortieren?
        bcs *+5
        jmp drop_xfer
        lda #6
        jmp act_confirm
@desk:  ; Papierkorb-Symbol auf dem Schreibtisch?
        jsr org0
        lda #4
        sta RX
        lda #42
        sta RY
        lda #48
        sta RW
        lda #30
        sta RH
        jsr inrect
        php
        jsr orgr
        plp
        bcs @bin
        jmp redraw_all
@bin:   ; *:RECYCLED anlegen (Fehler egal)
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
        ldy #dstdir
        jsr pcopy
        lda #1
        sta DGBIN
        ; fall through

; dgdir:Name -> dstdir:Name
drop_xfer:
        ; gleicher Ordner? dann nichts tun
        ldx #dgdir
        ldy #dstdir
        jsr pequal
        bcc @diff
        jmp redraw_all
@diff:  lda DGN
        ldx DGBIN
        JSLF xf_prog_begin
        stz PSTI
@l:     lda PSTI
        cmp DGN
        bcs @ok
        asl
        asl
        asl
        asl
        sta TMP
        asl
        clc
        adc TMP
        clc
        adc #dgents
        sta EP
        jsr epmy
        ldx #dgdir
        ldy #srcpath
        jsr pcopy
        ldy #srcpath
        jsr pappend
        ldx #dstdir
        ldy #dstpath
        jsr pcopy
        ldy #dstpath
        jsr pappend
        JSLF xf_sys_sub         ; ggf. in den Unterordner von System
        lda [EP]
        sta XFTYPE
        lda DGBIN
        bne @cut
        ldx #dgdir
        ldy #dstdir
        jsr samevol
        lda #0
        rol a
        bra @st
@cut:   lda #1
@st:    sta XCUT
        jsr xfer
        bcs @err
        JSLF xf_prog_step
        inc PSTI
        bra @l
@err:   pha
        JSLF xf_prog_end
        jsr reload_all
        pla
        cmp #XABORTC
        beq @rd
        jmp fail
@ok:    JSLF xf_prog_end
        jsr reload_all
@rd:    jmp redraw_all

; Pfade X und Y gleich (ohne Gross/Klein)? C=1 wenn ja
pequal: stx PB
        sty PB2
        lda (PB)
        cmp (PB2)
        bne @no
        tax
        ldy #2
@l:     cpx #0
        beq @yes
        lda (PB),y
        jsr upc8
        sta TMP
        lda (PB2),y
        jsr upc8
        cmp TMP
        bne @no
        iny
        dex
        bra @l
@yes:   sec
        rts
@no:    clc
        rts
upc8:   and #$00FF
        cmp #'a'
        bcc @r
        cmp #'z'+1
        bcs @r
        and #$00DF
@r:     rts

; Eintrag unter MX,MY im obersten Explorer -> A (oder $FFFF)
cell_at:
        jsr win_geom
        lda VIEWM
        beq @grid
        jsr list_slot
        bcs @lsl
        lda #$FFFF
        rts
@lsl:   jmp @slot
@grid:  jsr grid_slot
        bcs @slot
        lda #$FFFF
        rts
@slot:  lda SLOT
        cmp PPG
        bcc @sv
        lda #$FFFF
        rts
@sv:    jsr pg_first
        clc
        adc SLOT
        cmp COUNT
        bcc @r
        lda #$FFFF
@r:     rts

; alle Explorer-Fenster neu einlesen
reload_all:
        jsr win_save
        stz DGK
@l:     lda DGK
        cmp NWIN
        bcs @e
        jsr win_load
        lda WIN
        cmp #1
        bne @n
        stz VOLOK
        lda #$FFFF
        sta SEL2
        stz PAGE
        jsr reload_view
        lda DGK
        jsr win_savek
@n:     inc DGK
        bra @l
@e:     lda NWIN
        beq @r
        dec a
        jmp win_load
@r:     rts
