        .segment "XCODE2"
FARMODE .set 2
xf_wc_ctrl:
        jsr wc_ctrl
        rtl
xf_wc_sdlg:
        jsr wc_sdlg
        rtl
xf_ctrl_click:
        jsr ctrl_click
        rtl
xf_ctrl_wkey:
        jsr ctrl_wkey
        rtl
xf_sdlg_click:
        jsr sdlg_click
        rtl
xf_sdlg_init:
        jsr sdlg_init
        rtl
xf_sdlg_ok:
        jsr sdlg_ok
        rtl
xf_setup_dbl:
        jsr setup_dbl
        rtl
; GS/OS95: Systemsteuerung (Anzeige, Maus, Datum/Uhrzeit, Ton, Sprache)
; eigene Dialoge im Win95-Stil, Werte im Battery-RAM bzw. in GSOS95.PREFS

SROWY = WY+30
SROWH = 22
SCX   = WX+96
CPCW  = 68
CPCH  = 36
CPX0  = WX+12
CPY0  = WY+32
CPN   = 7                       ; Eintraege

; ---------- Fenster "Systemsteuerung" ----------
        .pushseg
        .segment "RODATA"
cp_icons:  .word i_comp, i_mouse, i_clock, i_snd, i_font, i_ctrl, i_net
cp_texts:  .word t_anzeige, t_maus, t_datum, t_ton, t_sprache, t_weitere, t_netz
cp_textse: .word t_anzeige_en, t_maus_en, t_datum_en, t_ton_en, t_sprache_en, t_weitere_en, t_netz_en

        .popseg
wc_ctrl:
        CALL explorer
        stz CPI
@l:     lda CPI
        cmp #CPN
        bcc @go
        TEXT WX+8,WY+WH-14,t_obj7,BLACK
        rts
@go:    jsr cp_cell             ; IX/IY = Zellenecke
        lda IX
        clc
        adc #22
        sta SX
        lda IY
        sta SY
        lda CPI
        asl
        tax
        lda cp_icons,x
        sta SPR
        CALL blit4
        ; Beschriftung, markiert wenn CPSEL
        lda CPI
        asl
        tax
        lda cp_texts,x
        ldy LANG
        beq @de
        lda cp_textse,x
@de:    sta SPR
        CALL sprhdr
        and #$00FF
        sta BW
        lsr
        sta TMP
        lda IX
        clc
        adc #30
        sec
        sbc TMP
        sta SX
        lda IY
        clc
        adc #19
        sta SY
        lda CPI
        cmp CPSEL
        bne @ns
        lda SX
        dec a
        sta RX
        lda SY
        dec a
        sta RY
        lda BW
        clc
        adc #2
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
        CALL blit1
        inc CPI
        jmp @l

; Zelle CPI -> IX, IY
cp_cell:
        lda CPI
        ldx #0
@d:     cmp #3
        bcc @dd
        sbc #3
        inx
        bra @d
@dd:    sta TMP
        asl
        asl
        sta TMP2                ; *4
        lda TMP
        asl
        asl
        asl
        asl
        asl
        asl                     ; *64
        clc
        adc TMP2                ; *68
        clc
        adc #CPX0
        sta IX
        txa
        asl
        asl                     ; *4
        sta TMP
        asl
        asl
        asl                     ; *32
        clc
        adc TMP                 ; *36
        clc
        adc #CPY0
        sta IY
        rts

; Klick im Fenster "Systemsteuerung"
ctrl_click:
        HIT CPX0,CPY0,3*CPCW,3*CPCH
        bcs @in
        lda CPSEL
        bmi @r
        lda #$FFFF
        sta CPSEL
        CALL redraw_win
        rts
@r:     rts
@in:    CALL lmx
        sec
        sbc #CPX0
        ldx #0
@cx:    cmp #CPCW
        bcc @cxd
        sbc #CPCW
        inx
        bra @cx
@cxd:   stx TMP
        CALL lmy
        sec
        sbc #CPY0
        ldx #0
@cy:    cmp #CPCH
        bcc @cyd
        sbc #CPCH
        inx
        bra @cy
@cyd:   txa
        sta TMP2
        asl
        clc
        adc TMP2                ; Zeile*3
        clc
        adc TMP
        cmp #CPN
        bcc @cin
        rts
@cin:   pha
        JSLF xf_dblclick
        pla
        bcc @sg
        jmp cp_open
@sg:    cmp CPSEL
        beq @r2
        sta CPSEL
        CALL redraw_win
        rts
@r2:    rts

; Taste im Fenster "Systemsteuerung": Pfeile, Return
ctrl_wkey:
        lda KEYC
        cmp #$15
        bne @k1
        lda CPSEL
        inc a
        cmp #CPN
        bcc @set
        lda #0
        bra @set
@k1:    cmp #$08
        bne @k2
        lda CPSEL
        dec a
        bpl @set
        lda #CPN-1
        bra @set
@k2:    cmp #$0A
        bne @k3
        lda CPSEL
        bmi @z
        clc
        adc #3
        cmp #CPN
        bcc @set
@col:   cmp #3                  ; unten hinaus: oben in derselben Spalte
        bcc @set
        sbc #3
        bra @col
@z:     lda #0
        bra @set
@k3:    cmp #$0B
        bne @k4
        lda CPSEL
        bmi @z
        sec
        sbc #3
        bpl @set
        clc
        adc #9                  ; oben hinaus: unterste Zeile derselben Spalte
        cmp #CPN
        bcc @set
        sbc #3
        bra @set
@k4:    cmp #$0D
        bne @k5
        lda CPSEL
        bmi @r
        jmp cp_open
@k5:    cmp #$1B
        bne @k6
        CALL close_win
        rts
@k6:    and #$00DF
        cmp #'L'
        bne @r
        CALL toggle_lang
        rts
@set:   sta CPSEL
        CALL redraw_win
        rts
@r:     rts

; Eintrag A der Systemsteuerung oeffnen
        .pushseg
        .segment "RODATA"
cp_kind: .word 16, 17, 15, 18
        .popseg
cp_open:
        sta CPSEL
        cmp #6                  ; Netzwerk
        bne @nn
        JSLF xf_ndlg_init
        lda #22
        CALL win_open
        CALL redraw_all
        rts
@nn:    cmp #4
        bcc @dlg
        bne @more
        CALL toggle_lang
        rts
@more:  jmp open_cpnda
@dlg:   asl
        tax
        lda cp_kind,x
        pha
        jsr sdlg_init
        pla
        CALL win_open
        CALL redraw_all
        rts

; Apples Kontrollfeld-NDA (fuer CDevs von Drittanbietern), sonst Zubehoerliste
open_cpnda:
        CALL desk_init
        bcc @ok
        lda #4
        CALL show_msg
        rts
@ok:    stz NDAI
@l:     lda NDAI
        cmp NDAN
        bcs @none
        asl
        asl
        asl
        asl
        asl
        tax
        lda ndanames+1,x        ; "Co" / "Ko"
        and #$DFDF
        cmp #$4F43              ; 'CO'
        beq @hit
        cmp #$4F4B              ; 'KO'
        beq @hit
        inc NDAI
        bra @l
@hit:   lda NDAI
        inc a
        CALL nda_open
        rts
@none:  CALL open_access
        rts

; ================= Einstellungsdialoge (Art 15..18) =================
; Zeile: Typ, Text, Text_en, P1, P2
ST_NUM = 1                      ; Zahl mit -/+ (P1 = min, P2 = max)
ST_OPT = 2                      ; P1 Kaestchen
ST_SW  = 3                      ; 16 Farbfelder (P1: 0 = Palette, 1 = Randfarben)
ST_TOG = 4                      ; Ankreuzfeld

        .pushseg
        .segment "RODATA"
sdefs:  .word sd_time, sd_disp, sd_mouse, sd_sound
sd_time:  .word 5
          .word ST_NUM, s_tag, s_tag_en, 1, 31
          .word ST_NUM, s_mon, s_mon_en, 1, 12
          .word ST_NUM, s_jahr, s_jahr_en, 1987, 2039
          .word ST_NUM, s_std, s_std_en, 0, 23
          .word ST_NUM, s_min, s_min_en, 0, 59
sd_disp:  .word 5
          .word ST_SW, s_desk, s_desk_en, 0, 0
          .word ST_SW, s_rand, s_rand_en, 1, 0
          .word ST_NUM, s_saver, s_saver_en, 0, 15
          .word ST_NUM, s_muster, s_muster_en, 0, 4
          .word ST_TOG, s_dvon, s_dvon_en, 0, 0
sd_mouse: .word 2
          .word ST_OPT, s_dbl, s_dbl_en, 5, 0
          .word ST_TOG, s_fast, s_fast_en, 0, 0
sd_sound: .word 3
          .word ST_NUM, s_vol, s_vol_en, 0, 15
          .word ST_NUM, s_pitch, s_pitch_en, 0, 15
          .word ST_TOG, s_chime, s_chime_en, 0, 0

; Rand-/Textfarben (Apple-II-Farben) -> naechster Paletteneintrag
bordmap: .byte 0,7,13,3,5,1,4,11,2,6,14,8,10,9,12,15

; Zeiger auf die Zeilentabelle des Dialogs WIN -> SDP, Zeilenzahl -> SDN
        .popseg
sdlg_tab:
        lda WIN
sdlg_tabk:
        sec
        sbc #15
        asl
        tax
        lda sdefs,x
        sta SDP
        tax
        lda a:0,x
        sta SDN
        rts

; Zeile SDR: X = Zeiger auf Zeilenbeschreibung, IY = y
sdlg_row:
        lda SDR
        asl
        sta TMP
        asl
        asl
        clc
        adc TMP                 ; *10
        clc
        adc SDP
        inc a
        inc a
        tax
        stx SDX
        lda SDR
        asl
        sta TMP
        asl
        asl
        asl                     ; *16
        clc
        adc TMP                 ; *18
        clc
        adc SDR
        adc SDR
        adc SDR
        adc SDR                 ; *22
        clc
        adc #SROWY
        sta IY
        rts

wc_sdlg:
        jsr sdlg_tab
        stz SDR
@row:   lda SDR
        cmp SDN
        bcs @btns
        jmp @go
@btns:  ; OK / Abbrechen
        BUTTON WX+WW-136,WY+WH-24,60,15
        TEXTC WX+WW-106,WY+WH-20,t_ok,BLACK
        BUTTON WX+WW-70,WY+WH-24,62,15
        TEXTC WX+WW-39,WY+WH-20,t_abbr,BLACK
        rts
@go:    jsr sdlg_row
        ; Beschriftung
        ldx SDX
        lda a:2,x
        ldy LANG
        beq @de
        lda a:4,x
@de:    sta SPR
        lda #WX+10
        sta SX
        lda IY
        clc
        adc #2
        sta SY
        lda #BLACK
        CALL setcolor
        CALL blit1
        ldx SDX
        lda a:0,x
        cmp #ST_NUM
        bne @n1
        jsr sd_drawnum
        bra @nx
@n1:    cmp #ST_OPT
        bne @n2
        jsr sd_drawopt
        bra @nx
@n2:    cmp #ST_SW
        bne @n3
        jsr sd_drawsw
        bra @nx
@n3:    jsr sd_drawtog
@nx:    inc SDR
        jmp @row

; Wert der Zeile SDR -> A
sd_val: lda SDR
        asl
        tax
        lda sval,x
        rts

sd_drawnum:
        ; weisses Feld mit der Zahl
        lda #SCX
        sta RX
        lda IY
        sta RY
        lda #34
        sta RW
        lda #13
        sta RH
        jsr sunk_r
        lda #SCX+3
        sta SX
        lda IY
        clc
        adc #3
        sta SY
        lda #BLACK
        CALL setcolor
        jsr sd_val
        CALL drawnum
        ; Knoepfe - und +
        lda #SCX+40
        jsr sd_btn
        lda #'-'
        CALL putch
        lda #SCX+58
        jsr sd_btn
        lda #'+'
        CALL putch
        rts
; Knopf 15x13 bei A, IY; danach SX/SY fuer das Zeichen
sd_btn: sta RX
        pha
        lda IY
        sta RY
        lda #15
        sta RW
        lda #13
        sta RH
        jsr btn_r
        pla
        clc
        adc #5
        sta SX
        lda IY
        clc
        adc #3
        sta SY
        lda #BLACK
        CALL setcolor
        rts

sd_drawopt:
        ldx SDX
        lda a:6,x
        sta SDK                 ; Anzahl
        stz SDI
@l:     lda SDI
        cmp SDK
        bcs @lab
        asl
        asl
        asl
        asl
        clc
        adc #SCX
        sta RX
        lda IY
        inc a
        sta RY
        lda #13
        sta RW
        lda #11
        sta RH
        jsr sunk_r
        jsr sd_val
        cmp SDI
        bne @nx
        lda RX
        clc
        adc #3
        sta RX
        lda RY
        clc
        adc #3
        sta RY
        lda #7
        sta RW
        lda #5
        sta RH
        lda #NAVY
        CALL setcolor
        CALL fillrect
@nx:    inc SDI
        bra @l
@lab:   ; "langsam ... schnell" unter den Kaestchen
        lda #SCX-2
        sta SX
        lda IY
        clc
        adc #13
        sta SY
        LSPR s_slow
        lda #DKGRAY
        CALL setcolor
        CALL blit1
        lda SDK
        asl
        asl
        asl
        asl
        clc
        adc #SCX+12
        sta TMP
        LSPR s_quick
        CALL sprhdr
        and #$00FF
        sta TMP2
        lda TMP
        sec
        sbc TMP2
        sta SX
        CALL blit1
        rts

sd_drawsw:
        ldx SDX
        lda a:6,x
        sta SDK                 ; 0 Palette, 1 Randfarben
        stz SDI
@l:     lda SDI
        cmp #16
        bcs @r
        asl
        asl
        asl
        clc
        adc #SCX-12
        sta RX
        lda IY
        sta RY
        lda #8
        sta RW
        lda #13
        sta RH
        lda SDI
        ldx SDK
        beq @p
        tax
        lda bordmap,x
        and #$00FF
@p:     CALL setcolor
        CALL fillrect
        jsr sd_val
        cmp SDI
        bne @nx
        ; Auswahlrahmen
        lda #BLACK
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
@nx:    inc SDI
        bra @l
@r:     rts

sd_drawtog:
        lda #SCX
        sta RX
        lda IY
        inc a
        sta RY
        lda #11
        sta RW
        sta RH
        jsr sunk_r
        jsr sd_val
        beq @r
        lda #SCX+3
        sta SX
        lda IY
        inc a
        sta SY
        lda #t_x
        sta SPR
        lda #BLACK
        CALL setcolor
        CALL blit1
        rts
@r:     rts

; Knopf / vertieftes weisses Feld bei RX,RY,RW,RH
btn_r:  lda #GRAY
        CALL setcolor
        CALL fillrect
        lda #WHITE
        sta C1
        lda #BLACK
        sta C2
        CALL bevel
        jsr shrink1
        lda #GRAY
        sta C1
        lda #DKGRAY
        sta C2
        CALL bevel
        jmp grow1
sunk_r: lda #WHITE
        CALL setcolor
        CALL fillrect
        lda #DKGRAY
        sta C1
        lda #WHITE
        sta C2
        CALL bevel
        jsr shrink1
        lda #BLACK
        sta C1
        lda #GRAY
        sta C2
        CALL bevel
grow1:  dec RX
        dec RY
        inc RW
        inc RW
        inc RH
        inc RH
        rts
shrink1:
        inc RX
        inc RY
        dec RW
        dec RW
        dec RH
        dec RH
        rts

; ---------- Klicks im Einstellungsdialog ----------
sdlg_click:
        HIT WX+WW-136,WY+WH-24,60,15
        bcc @n0
        jmp sdlg_ok
@n0:    HIT WX+WW-70,WY+WH-24,62,15
        bcc @n1
        CALL close_win
        rts
@n1:    jsr sdlg_tab
        stz SDR
@row:   lda SDR
        cmp SDN
        bcc @go
        rts
@go:    jsr sdlg_row
        CALL lmy
        sec
        sbc IY
        cmp #SROWH-4
        bcc @hit
        inc SDR
        bra @row
@hit:   lda SDR
        asl
        tax
        stx SDI                 ; Index in sval
        ldy SDX
        lda 0,y
        cmp #ST_NUM
        bne @t2
        ; -: SCX+40..54, +: SCX+58..72
        CALL lmx
        sec
        sbc #SCX+40
        bcs *+5
        jmp @r
        cmp #15
        bcc @minus
        sbc #18
        bcc @r
        cmp #15
        bcs @r
        ldx SDI
        lda sval,x
        ldy SDX
        cmp 8,y
        bcs @r
        inc a
        bra @set
@minus: ldx SDI
        lda sval,x
        ldy SDX
        cmp 6,y
        beq @r
        bcc @r
        dec a
        bra @set
@t2:    cmp #ST_OPT
        bne @t3
        CALL lmx
        sec
        sbc #SCX
        bcc @r
        lsr
        lsr
        lsr
        lsr
        ldy SDX
        cmp 6,y
        bcs @r
        bra @set
@t3:    cmp #ST_SW
        bne @t4
        CALL lmx
        sec
        sbc #SCX-12
        bcc @r
        lsr
        lsr
        lsr
        cmp #16
        bcs @r
        bra @set
@t4:    CALL lmx                 ; Ankreuzfeld (auch Klick auf den Text)
        ldx SDI
        lda sval,x
        eor #1
@set:   ldx SDI
        sta sval,x
        CALL redraw_win
        rts
@r:     rts

; ---------- Werte lesen / anwenden ----------
; A = Art des Dialogs
sdlg_init:
        cmp #15
        bne @i16
        ; Uhrzeit lesen
        pha
        pha
        pha
        pha
        TOOL $0D03              ; ReadTimeHex
        pla                     ; Minute|Sekunde
        xba
        and #$00FF
        sta sval+8
        pla                     ; Jahr|Stunde
        pha
        and #$00FF
        sta sval+6
        pla
        xba
        and #$00FF
        clc
        adc #1900
        sta sval+4
        pla                     ; Monat|Tag (ab 0)
        pha
        and #$00FF
        inc a
        sta sval
        pla
        xba
        and #$00FF
        inc a
        sta sval+2
        pla
        rts
@i16:   cmp #16
        bne @i17
        lda DESKC
        sta sval
        lda #$1C
        jsr rdbp
        and #$000F
        sta sval+2
        lda SSMIN
        sta sval+4
        lda DESKPAT
        sta sval+6
        lda DVON
        sta sval+8
        rts
@i17:   cmp #17
        bne @i18
        lda #$2E
        jsr rdbp
        cmp #5
        bcc @d
        lda #2
@d:     sta sval
        lda #$33
        jsr rdbp
        and #1
        sta sval+2
        rts
@i18:   lda #$1E
        jsr rdbp
        and #$000F
        sta sval
        lda #$1F
        jsr rdbp
        and #$000F
        sta sval+2
        lda SNDOFF
        eor #1
        sta sval+4
        rts

; Battery-RAM-Parameter A lesen -> A
rdbp:   pha
        pha                     ; Ergebnis
        lda 3,s
        pha
        TOOL $0C03              ; ReadBParam
        pla
        sta 1,s
        pla
        rts
; Wert A in Battery-RAM-Parameter X schreiben
wrbp:   pha
        phx
        TOOL $0B03              ; WriteBParam
        rts

sdlg_ok:
        lda WIN
        cmp #15
        bne @o16
        ; WriteTimeHex(MonatTag, JahrStunde, MinuteSekunde)
        lda sval+2
        dec a
        xba
        sta TMP
        lda sval
        dec a
        ora TMP
        pha
        lda sval+4
        sec
        sbc #1900
        xba
        ora sval+6
        pha
        lda sval+8
        xba
        pha
        TOOL $0E03              ; WriteTimeHex
        CALL read_clock
        jmp @done
@o16:   cmp #16
        bne @o17
        lda sval
        sta DESKC
        lda sval+4
        sta SSMIN
        lda sval+6
        sta DESKPAT
        lda sval+8
        sta DVON
        CALL save_prefs
        JSLF xf_dv_scan
        lda sval+2
        ldx #$1C
        jsr wrbp
        lda sval+2
        jsr set_border
        bra @done
@o17:   cmp #17
        bne @o18
        lda sval
        ldx #$2E
        jsr wrbp
        lda sval+2
        ldx #$33
        jsr wrbp
        jsr setup_dbl
        bra @done
@o18:   lda sval
        ldx #$1E
        jsr wrbp
        lda sval+2
        ldx #$1F
        jsr wrbp
        lda sval+4
        eor #1
        sta SNDOFF
        CALL save_prefs
        lda sval
        jsr set_volume
        TOOL $2C03              ; SysBeep zur Kontrolle
@done:  CALL win_pop
        CALL redraw_all
        rts

; Randfarbe A sofort setzen ($C034, untere 4 Bit)
set_border:
        and #$000F
        sta TMP
        sep #$20
        lda f:$E0C034
        and #$F0
        ora TMP
        sta f:$E0C034
        rep #$20
        rts

; Lautstaerke A (0..15) im Sound-GLU setzen ($C03C, untere 4 Bit)
set_volume:
        and #$000F
        sta TMP
        sep #$20
        ldx #0
@w:     lda f:$E0C03C
        bpl @ok
        dex
        bne @w
@ok:    and #$F0
        ora TMP
        sta f:$E0C03C
        rep #$20
        rts

; Doppelklickzeit aus dem Battery-RAM ($2E: 0 = lang ... 4 = kurz)
        .pushseg
        .segment "RODATA"
dbltab: .word 60, 45, 32, 22, 14
        .popseg
setup_dbl:
        lda #$2E
        jsr rdbp
        cmp #5
        bcc @ok
        lda #2
@ok:    asl
        tax
        lda dbltab,x
        sta DBLT
        rts

FARMODE .set 0
        .segment "CODE"
