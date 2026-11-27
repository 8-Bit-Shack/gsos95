        .segment "XCODE2"
FARMODE .set 2
xf_do_dcopy:
        jsr do_dcopy
        rtl
xf_dpoll:
        jsr dpoll
        rtl
xf_edit_hex:
        jsr edit_hex
        rtl
xf_launch_bas:
        jsr launch_bas
        rtl
xf_props_type_click:
        jsr props_type_click
        rtl
xf_props_type_draw:
        jsr props_type_draw
        rtl
xf_type_set:
        jsr type_set
        rtl
xf_vol_clip:
        jsr vol_clip
        rtl
xf_vol_paste:
        jsr vol_paste
        rtl
xf_wc_dcopy:
        jsr wc_dcopy
        rtl
; ================================================================
; GS/OS95: BASIC-Programme starten, Datentraeger kopieren,
; eingelegte Disketten erkennen, Dateityp aendern
; ================================================================

; ---------- BASIC-Programm ($FC) starten ----------
; Kopie von *:BASIC.SYSTEM mit eingetragenem Startpfad (ProDOS-8-Konvention:
; JMP, $EE, $EE, Pufferlaenge, Pfad ab $2006) wird als ProDOS-8-Programm gestartet.
launch_bas:
        CALL entry_path          ; srcpath = :VOL:ORDNER:NAME
        lda srcpath
        cmp #64
        bcc @len
        lda #17                 ; Pfad zu lang fuer ProDOS 8
        CALL show_msg
        rts
@len:   ; ProDOS-8-Pfad /VOL/ORDNER/NAME (Grossbuchstaben) nach basbuf
        sep #$20
        sta basbuf
        ldx #0
@c:     cpx srcpath
        bcs @ce
        lda srcpath+2,x
        cmp #':'
        bne @n1
        lda #'/'
@n1:    cmp #'a'
        bcc @n2
        cmp #'z'+1
        bcs @n2
        and #$DF
@n2:    sta basbuf+1,x
        inx
        bra @c
@ce:    rep #$20
        ; Programmordner als Prefix 0/8 (wie beim Starten von Programmen)
        ldx #pathbuf
        ldy #prefbuf
        CALL pcopy
        ldx #0
        sep #$20
@pc:    cpx prefbuf
        bcs @pe
        lda prefbuf+2,x
        cmp #':'
        bne @pn
        lda #'/'
        sta prefbuf+2,x
@pn:    inx
        bra @pc
@pe:    lda #'/'
        sta prefbuf+2,x
        inx
        stx prefbuf
        rep #$20
        ; Kopie anlegen: alte loeschen, *:BASIC.SYSTEM -> *:System:BASIC95.SYSTEM
        ldx #bas_dst
        ldy #dstpath
        CALL pcopy
        lda #$2002
        ldx #destroy_pb
        ldy #dstpath
        CALL gsosp
        ldx #bas_src
        ldy #srcpath
        CALL pcopy
        CALL copy_file
        bcc @cpok
        lda #16                 ; BASIC.SYSTEM fehlt
        CALL show_msg
        rts
@cpok:  ; Kopf pruefen und Startpfad eintragen
        lda #dstpath
        sta o4_pb+4
        lda #3
        sta o4_pb+8             ; Lesen + Schreiben
        stz o4_pb+10
        GSOS $2010,o4_pb
        bcc @op
        CALL fail
        rts
@op:    lda o4_pb+2
        sta io_pb+2
        sta cl_pb+2
        sta mk_pb+2
        lda #6
        sta io_pb+8
        GSOS $2012,io_pb        ; 6 Bytes lesen
        bcc *+5
        jmp @bad
        ldy #3
        lda [CBP],y
        cmp #$EEEE
        beq *+5
        jmp @bad
        lda basbuf
        and #$00FF
        sta TMP
        ldy #5
        lda [CBP],y
        and #$00FF
        cmp TMP
        beq @bad                ; Puffer zu klein
        bcc @bad
        lda #6
        sta mk_pb+6
        GSOS $2016,mk_pb        ; SetMark 6 (Pfad ab $2006)
        bcs @bad
        ldy #0
        sep #$20
@cb:    lda basbuf,y
        sta [CBP],y
        iny
        cpy #65
        bcc @cb
        rep #$20
        lda basbuf
        and #$00FF
        inc a
        sta io_pb+8
        GSOS $2013,io_pb        ; Write
        bcs @bad
        GSOS $2014,cl_pb
        ; Prefix setzen und starten
        lda #8
        sta prefix_pb+2
        GSOS $2009,prefix_pb
        stz prefix_pb+2
        GSOS $2009,prefix_pb
        ldx #dstpath
        ldy #launchbuf
        CALL pcopy
        CALL launch
        rts
@bad:   GSOS $2014,cl_pb
        lda #16
        CALL show_msg
        rts

        .pushseg
        .segment "RODATA"
bas_src: .word 14
         .byte "*:BASIC.SYSTEM"
bas_dst: .word 23
         .byte "*:System:BASIC95.SYSTEM"

; ---------- Datentraeger kopieren (Arbeitsplatz: Kopieren / Einfuegen) ----------
; Kopieren im Arbeitsplatz merkt sich das Quelllaufwerk
        .popseg
vol_clip:
        lda SEL2
        bmi @r
        CALL entep
        lda [EP]
        cmp #$0100
        bne @r
        lda SEL2
        asl
        tax
        lda voldev,x
        sta VCSRC
        ldy #2                  ; Name merken (fuer die Rueckfrage)
        ldx #0
        sep #$20
@n:     lda [EP],y
        sta vcname,x
        iny
        inx
        cpx #17
        bcc @n
        rep #$20
        lda #1
        sta VCOK
@r:     CALL redraw_win
        rts

; Einfuegen auf ein anderes Laufwerk: Rueckfrage, dann blockweise kopieren
vol_paste:
        lda VCOK
        beq @no
        lda SEL2
        bmi @no
        CALL entep
        lda [EP]
        cmp #$0100
        beq @ok
        cmp #$0101
        bne @no
@ok:    lda SEL2
        asl
        tax
        lda voldev,x
        cmp VCSRC
        beq @no
        sta VCDST
        ; nicht auf das Startlaufwerk
        lda [EP]
        cmp #$0101
        beq @nb
        lda SEL2
        sta IDX
        CALL entry_path
        ldx #srcpath
        ldy #bootvol
        CALL samevol
        bcc @nb
        lda #10
        CALL show_msg
        rts
@nb:    lda #5
        CALL act_confirm
        rts
@no:    lda #7
        CALL show_msg
        rts

; Rueckfrage-Text (CONFACT 5)
wc_dcopy:
        ICON WX+14,WY+30,i_floppy
        TEXT WX+40,WY+30,t_cdc1,BLACK
        ; "Quelle -> Ziel"
        lda vcname
        and #$00FF
        sta STRN
        lda #vcname+1
        sta STRP
        lda #WX+40
        sta SX
        lda #WY+44
        sta SY
        lda #NAVY
        CALL setcolor
        CALL drawstr
        TEXT WX+40,WY+58,t_cdc2,BLACK
        TEXT WX+40,WY+70,t_cdc3,BLACK
        CALL confirm_btns
        rts

; blockweise kopieren (aus confirm_yes)
do_dcopy:
        ; Groesse beider Laufwerke
        lda VCSRC
        jsr dev_blocks
        bcs @err
        sta DCN
        stx DCN+2
        lda VCDST
        jsr dev_blocks
        bcs @err
        cmp DCN
        bne @size
        cpx DCN+2
        beq @go
@size:  lda #18
        CALL show_msg
        rts
@err:   CALL fail
        rts
@go:    stz DCB
        stz DCB+2
        lda CBP
        sta dr_pb+4
        lda CBP+2
        sta dr_pb+6
@loop:  ; Anzahl = min(16, Rest)
        lda DCN
        sec
        sbc DCB
        sta TMP
        lda DCN+2
        sbc DCB+2
        bne @full
        lda TMP
        bne @nz
        jmp @done
@nz:    cmp #16
        bcc @part
@full:  lda #16
        sta TMP
@part:  lda TMP
        xba
        asl                     ; *512
        sta dr_pb+8
        stz dr_pb+10
        lda DCB
        sta dr_pb+12
        lda DCB+2
        sta dr_pb+14
        lda #512
        sta dr_pb+16
        lda VCSRC
        sta dr_pb+2
        GSOS $202F,dr_pb        ; DRead
        bcs @err
        lda VCDST
        sta dr_pb+2
        GSOS $2030,dr_pb        ; DWrite
        bcs @err
        lda DCB
        clc
        adc TMP
        sta DCB
        bcc @pr
        inc DCB+2
@pr:    jsr dc_progress
        jmp @loop
@done:  stz VCOK
        stz VOLOK
        CALL reload_view
        CALL redraw_all
        rts

; Fortschritt in der Statuszeile: "nn %"
dc_progress:
        lda DCB
        and #$00FF
        cmp #16
        bcs @r                  ; nur ab und zu
        CALL cur_hide
        CALL anc_b
        FILL WX+6,WY+WH-14,110,9,GRAY
        ; Prozent = DCB*100/DCN (grob ueber 16-Bit-Werte >> 8)
        lda DCB+1
        and #$00FF
        ldx #100
        jsr mul16
        pha
        lda DCN+1
        and #$00FF
        tax
        pla
        jsr div16
        sta NUMLO
        stz NUMHI
        lda #WX+8
        sta SX
        lda #WY+WH-14
        sta SY
        lda #BLACK
        CALL setcolor
        CALL putdec32
        lda #' '
        CALL putch
        lda #'%'
        CALL putch
        CALL anc_end
        CALL cur_show
@r:     rts

; A * X -> A (16 Bit)
mul16:  sta TMP
        stx MULX
        lda #0
        ldx #16
@l:     asl a
        asl TMP
        bcc @n
        clc
        adc MULX
@n:     dex
        bne @l
        rts
; A / X -> A (16 Bit, X > 0)
div16:  stx DVD
        cpx #0
        bne @g
        lda #0
        rts
@g:     sta DVN
        stz DVR
        ldx #16
@l:     asl DVN
        rol DVR
        lda DVR
        cmp DVD
        bcc @n
        sbc DVD
        sta DVR
        inc DVN
@n:     dex
        bne @l
        lda DVN
        rts

; Gesamtbloecke des Geraets A -> A (lo), X (hi). C=1 bei Fehler
dev_blocks:
        sta di8_pb+2
        lda #devname_buf
        sta di8_pb+4
        lda MYBANK
        sta di8_pb+6
        GSOS $202C,di8_pb       ; DInfo
        bcs @r
        lda di8_pb+10
        ldx di8_pb+12
        clc
@r:     rts

; ---------- eingelegte Disketten erkennen ----------
; alle 2 s: Wechseldatentraeger (ausser 5,25") per DStatus pruefen
dpoll:
        JSLF xf_cd_tick         ; CD-Wiedergabe: Anzeige im Sekundentakt
        JSLF xf_sol_tick        ; Solitaer: Zeit
        lda NDAWAS
        beq *+5
        jmp @r
        lda MENU
        ora DROP
        beq *+5
        jmp @r
        lda WIN
        cmp #21                 ; Browser und CD-Player sind keine Dialoge
        beq @dpgo
        cmp #23
        bcs @dpgo
        cmp #7
        bcc @dpgo
        jmp @r                  ; Dialog offen
@dpgo:  stz DPCHG
        stz DPNEW
        lda #1
        sta DPDEV
@dev:   lda DPDEV
        sta di8_pb+2
        lda #devname_buf
        sta di8_pb+4
        lda MYBANK
        sta di8_pb+6
        GSOS $202C,di8_pb       ; DInfo
        bcs @end
        lda di8_pb+8
        and #$0004              ; Wechselmedium?
        beq @next
        lda di8_pb+20
        beq @next               ; 5,25"-Laufwerk: nicht abfragen
        lda DPDEV
        sta ds_pb+2
        stz ds_pb+4
        lda #dsbuf
        sta ds_pb+6
        lda MYBANK
        sta ds_pb+8
        lda #2
        sta ds_pb+10
        stz ds_pb+12
        GSOS $202D,ds_pb        ; DStatus
        bcs @next
        lda di8_pb+20           ; CD-ROM: Laufwerk selbst fragen (TEST UNIT READY),
        cmp #$0007              ; der Treiber merkt das Auswerfen nicht immer
        bne @ncd
        lda DPDEV
        JSLF xf_cd_tur
        sta dsbuf
@ncd:   lda DPDEV
        asl
        tax
        lda dsbuf
        and #$0010              ; Datentraeger eingelegt
        cmp dpst,x
        beq @next
        sta dpst,x
        ldy DPINI
        beq @next               ; erster Durchlauf: nur merken
        inc DPCHG
        cmp #0
        beq @next
        lda DPDEV
        sta DPNEW
@next:  inc DPDEV
        lda DPDEV
        cmp #DPMAX
        bcs *+5
        jmp @dev
@end:   lda #1
        sta DPINI
        lda DPCHG
        bne @chg
@r:     rts
@chg:   stz VOLOK
        lda DPNEW
        beq @show
        ; neuer Datentraeger: lesbar?
        sta di8_pb+2
        GSOS $202C,di8_pb
        GSOS $2008,volume_pb
        bcc @show
        lda di8_pb+20           ; CD-ROM ohne lesbaren Datentraeger: Audio-CD abspielen
        cmp #$0007
        bne @fmt
        lda DPNEW
        JSLF xf_cd_auto
        rts
@fmt:   ; unlesbar: Arbeitsplatz zeigen und Formatieren anbieten
        jsr dp_explorer
        stz SEL2
@f:     lda SEL2
        cmp COUNT
        bcs @show2
        asl
        tax
        lda voldev,x
        cmp DPNEW
        beq @found
        inc SEL2
        bra @f
@found: CALL redraw_all
        CALL act_erase
        rts
@show:  ; Arbeitsplatz offen? dann neu einlesen
        lda WIN
        cmp #1
        bne @rr
        lda MODE
        bne @rr
        CALL load_volumes
        CALL redraw_win
        rts
@show2: lda #$FFFF
        sta SEL2
        CALL redraw_all
        rts
@rr:    rts

; Arbeitsplatz-Fenster nach vorn bzw. oeffnen
dp_explorer:
        lda WIN
        cmp #1
        bne @new
        lda MODE
        bne @new
        CALL load_volumes
        rts
@new:   CALL new_explorer
        bcc @ok
        rts
@ok:    stz ISBIN
        stz MODE
        CALL load_volumes
        rts

; ---------- Dateityp aendern (Eigenschaften) ----------
TYPX = WX+WW-62
TYPY = WY+44
props_type_draw:
        lda MODE
        bne *+5
        jmp @r
        lda SEL2
        CALL entep
        lda [EP]
        cmp #$000F
        bne *+5
        jmp @r
        BUTTON TYPX,TYPY,54,13
        TEXTC TYPX+27,TYPY+3,t_chg,BLACK
@r:     rts

; C=1 wenn behandelt
props_type_click:
        lda MODE
        beq @no
        lda SEL2
        CALL entep
        lda [EP]
        cmp #$000F
        beq @no
        HIT TYPX,TYPY,54,13
        bcs @go
@no:    clc
        rts
@go:    ; Vorschlag "TT/AAAA"
        lda #editbuf+2
        sta TMP2
        lda gfi_pb+8
        jsr hex2buf
        lda #'/'
        jsr chbuf
        lda gfi_pb+11
        jsr hex2buf
        lda gfi_pb+10
        jsr hex2buf
        lda #7
        sta editbuf
        lda #1
        sta editsel
        lda #4
        sta TXTMODE
        lda #11
        CALL win_open
        CALL redraw_all
        sec
        rts
; Byte A als 2 Hexziffern an (TMP2)
hex2buf:
        and #$00FF
        pha
        lsr
        lsr
        lsr
        lsr
        jsr hexd
        jsr chbuf
        pla
        and #$000F
        jsr hexd
chbuf:  sep #$20
        sta (TMP2)
        rep #$20
        inc TMP2
        rts
hexd:   cmp #10
        bcc @d
        adc #6
@d:     adc #'0'
        rts

; aus edit_ok (TXTMODE 4): editbuf "TT" oder "TT/AAAA" (hex)
type_set:
        stz TYVAL
        stz TYAUX
        ldx #0
        ldy #0                  ; 0 = Typ, 1 = Zusatztyp
@l:     cpx editbuf
        bcs @e
        lda editbuf+2,x
        and #$00FF
        inx
        cmp #'$'
        beq @l
        cmp #'/'
        bne @dg
        ldy #1
        bra @l
@dg:    cmp #'a'
        bcc @u
        and #$00DF
@u:     sec
        sbc #'0'
        cmp #10
        bcc @v
        sbc #7
        cmp #16
        bcc *+5
        jmp @bad
@v:     sta TMP
        cpy #0
        bne @aux
        lda TYVAL
        asl
        asl
        asl
        asl
        ora TMP
        sta TYVAL
        bra @l
@aux:   lda TYAUX
        asl
        asl
        asl
        asl
        ora TMP
        sta TYAUX
        bra @l
@e:     lda TYVAL
        cmp #$0100
        bcs @bad
        cpy #0
        bne @hasaux
        lda gfi_pb+10           ; Zusatztyp behalten
        sta TYAUX
@hasaux:
        CALL win_pop             ; Eingabe
        lda TYVAL
        sta gfi_pb+8
        lda TYAUX
        sta gfi_pb+10
        stz gfi_pb+12
        lda #4                  ; SetFileInfo: Pfad, Zugriff, Typ, Zusatztyp
        sta gfi_pb
        lda #$2005
        ldx #gfi_pb
        ldy #srcpath
        CALL gsosp
        php
        pha
        lda #12
        sta gfi_pb
        pla
        plp
        bcc @ok
        CALL win_pop
        CALL fail
        rts
@ok:    lda #$2006
        ldx #gfi_pb
        ldy #srcpath
        CALL gsosp
        CALL reload_view
        CALL redraw_all
        rts
@bad:   lda #19
        CALL show_msg
        rts

; ---------- Parameterbloecke ----------
        .pushseg
        .segment "RODATA"
mk_pb:  .word 3, 0, 0
        .word 0, 0              ; Verschiebung
di8_pb: .word 8, 0
        .word 0, 0              ; devName
        .word 0                 ; characteristics
        .word 0, 0              ; totalBlocks
        .word 0, 0, 0           ; slot, unit, version
        .word 0                 ; deviceID
ds_pb:  .word 5, 0, 0
        .word 0, 0              ; statusList
        .word 0, 0              ; requestCount
        .word 0, 0              ; transferCount
dr_pb:  .word 6, 0
        .word 0, 0              ; buffer
        .word 0, 0              ; requestCount
        .word 0, 0              ; startingBlock
        .word 512               ; blockSize
        .word 0, 0              ; transferCount

; Eingabe im Dateityp-Feld: 0-9 A-F / $
        .popseg
edit_hex:
        lda editsel
        beq @a
        stz editbuf
        stz editsel
@a:     lda editbuf
        cmp #12
        bcs @r
        lda KEYC
        cmp #'/'
        beq @add
        cmp #'$'
        beq @add
        cmp #'0'
        bcc @r
        cmp #'9'+1
        bcc @add
        and #$00DF
        cmp #'A'
        bcc @r
        cmp #'F'+1
        bcs @r
@add:   ldx editbuf
        sep #$20
        sta editbuf+2,x
        rep #$20
        inc editbuf
        CALL edit_redraw
        rts
@r:     rts

FARMODE .set 0
        .segment "CODE"
