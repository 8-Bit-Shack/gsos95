; ================================================================
; GS/OS95 Browser: Vollbild-Terminal fuer Telnet (ANSI/VT100, 80 x 25)
; Teil von web.s (XCODE2). Der Bildschirm laeuft dabei im 640er-Modus
; (alle SCBs $8F, Palette 15 mit 4 Grautoenen), jedes Zeichen 8 x 8 Punkte
; aus dem PC-Zeichensatz (CP437, vtfont.inc).
; Speicher in der Bildbank IMBK:
;   $0000-$0F9F  Zellen: Wort = Zeichen | Attribut << 8
;                Attribut: Bit 0-2 Vordergrund, 3-5 Hintergrund (ANSI-Farbe),
;                Bit 6 hell, Bit 7 unterstrichen
;   $1000-$17FF  Schrift (256 x 8 Bytes, Bit 7 = links)
;   $1800-$19FF  Ausdehnung: Schriftbyte -> Maske im 640er-Format
; Farben: ANSI-Farbe -> Helligkeitsstufe (0 schwarz, 1 dunkel, 2 hell,
; 3 weiss), "hell" (fett) eine Stufe hoeher.
; Tasten gehen direkt an die Gegenstelle (Pfeile als ESC [ A..D),
; Apfel-Esc, Apfel-Q oder Strg-] beendet die Verbindung, Apfel-F wechselt
; zwischen Farbe (320er-Modus, 16 Farben, 4 Punkte breite Schrift) und
; scharf in Graustufen (640er-Modus).
; ================================================================
VT_FONT = $1000
VT_EXP  = $1800
VT_TMP  = $1A00                 ; +0 Maske XOR, +2 Hintergrund, +4 Glyphenende (Bank IMBK)

        .pushseg
        .segment "BSS"
VTON:   .res 2                  ; Terminal hat den Bildschirm
VTWAIT: .res 2                  ; Verbindung zu: auf Taste warten
VTX:    .res 2
VTY:    .res 2
VTWR:   .res 2                  ; Umbruch beim naechsten Zeichen
VTA:    .res 2                  ; Attribut
VTINV:  .res 2                  ; invers
VTST:   .res 2                  ; 0 Text, 1 nach ESC, 2 CSI, 3 ein Zeichen auslassen
VTNP:   .res 2                  ; Parameter-Nr.
VTPAR:  .res 16                 ; 8 Parameter
VTPRIV: .res 2                  ; "?" in CSI
VTSX:   .res 2
VTSY:   .res 2
VTSA:   .res 2
VTTOP:  .res 2                  ; Rollbereich
VTBOT:  .res 2
VTCV:   .res 2                  ; Schreibmarke sichtbar
VTCD:   .res 2                  ; Schreibmarke gezeichnet bei VTCX/VTCY
VTCX:   .res 2
VTCY:   .res 2
VTDX:   .res 2                  ; zu zeichnende Zelle
VTDY:   .res 2
VTCF:   .res 2                  ; als Schreibmarke (invers) zeichnen
VTG:    .res 2
VTGE:   .res 2
VTFL:   .res 2
VTBL:   .res 2
VTFGW:  .res 2
VTBGW:  .res 2
VTXW:   .res 2
VTSO:   .res 2
VTUL:   .res 2
VTT:    .res 2
VTT2:   .res 2
VTCH:   .res 2
VTE0:   .res 2
VTE1:   .res 2
VTEW:   .res 2                  ; Loeschzelle
VTEP:   .res 2                  ; Loeschmuster Bildschirm
VTN:    .res 2
VTMP:   .res 4                  ; Meldung: Zeiger
VTRC:   .res 2                  ; Wiederholungen (Rollen)
VTOT:   .res 2                  ; gemerkter Rollbereich oben
VTCM:   .res 2                  ; 1 = Farbmodus (320er, 4x8-Schrift, 16 Farben)
VTT3:   .res 2
VTMN:   .res 2
        .popseg

        .pushseg
        .segment "RODATA"
vt_lvf: .byte 0,1,2,2,1,1,2,2   ; schwarz rot gruen gelb blau magenta cyan weiss
vt_patw: .word $0000,$5555,$AAAA,$FFFF
vt_pal: .word $0000,$0666,$0BBB,$0FFF
vt_pal16: .word $0000,$0A00,$00A0,$0A50,$000A,$0A0A,$00AA,$0AAA   ; ANSI/CGA
        .word $0555,$0F55,$05F5,$0FF5,$055F,$0F5F,$05FF,$0FFF
vt_c16: .word $0000,$1111,$2222,$3333,$4444,$5555,$6666,$7777
        .word $8888,$9999,$AAAA,$BBBB,$CCCC,$DDDD,$EEEE,$FFFF
        .popseg

xf_vt_event:    jsr vt_event
        rtl

vt_ptr: lda IMBK
        sta PDST+2
        stz PDST
        rts

; ---------- Terminal einschalten (aus tn_start). C=1: kein Speicher ----------
vt_enter:
        jsr gif_mem
        bcc @ok
        rts
@ok:    CALL cur_hide
        lda #1
        sta VTON
        stz VTWAIT
        stz GIFON
        stz IMGN
        lda NPREF+2             ; Bit 7 gesetzt: scharf in Graustufen
        and #$0080
        eor #$0080
        beq @m
        lda #1
@m:     sta VTCM
        jsr vt_mode
        jsr vt_reset
        lda DOSON               ; Eingabeaufforderung: eigene Begruessung
        bne @nh
        WS ws_vthelp
        jsr vt_msg
@nh:    jsr vt_curon
        clc
        rts


; ---------- Schrift, Ausdehnung, Palette und SCBs fuer den Modus VTCM ----------
vt_mode:
        lda VTCM
        beq vt_mono
        jmp vt_color
vt_mono:
        ; Schrift aus dieser Codebank nach IMBK $1000
        CBANK
        sta VTMP+2
        lda #vt_font
        sta VTMP
        lda IMBK
        sta PDST+2
        lda #VT_FONT
        sta PDST
        lda VTMP                ; [VTMP] geht nicht (kein DP): ueber PSRC
        sta PSRC
        lda VTMP+2
        sta PSRC+2
        ldy #0
@fc:    lda [PSRC],y
        sta [PDST],y
        iny
        iny
        cpy #2048
        bcc @fc
        jsr net_ptr0
        jsr vt_ptr
        ; Ausdehnungstabelle
        ldx #0
@e:     stx VTT
        stz VTT2
        lda #8
        sta VTN
@b:     asl VTT2
        asl VTT2
        lda VTT
        asl
        sta VTT
        bit #$0100
        beq @z
        lda VTT2
        ora #3
        sta VTT2
@z:     dec VTN
        bne @b
        txa
        asl
        clc
        adc #VT_EXP
        tay
        lda VTT2
        xba
        sta [PDST],y
        inx
        cpx #256
        bcc @e
        ; Palette 15: die 4 Stufen in allen 4 Gruppen
        ldx #0
@p:     txa
        and #6
        tay
        lda vt_pal,y
        sta f:$E19E00+15*32,x
        inx
        inx
        cpx #32
        bcc @p
        ; SCBs: 640er-Modus, Palette 15
        lda #$8F8F
vt_scb: ldx #0
@s:     sta f:$E19D00,x
        inx
        inx
        cpx #200
        bcc @s
        rts

; Farbmodus: 4x8-Schrift entpacken, Ausdehnung 320er, 16 Farben, SCB $0F
vt_color:
        jsr vt_ptr
        CBANK
        sta PSRC+2
        lda #vt_font4
        sta PSRC
        stz VTT
@u:     ldy VTT
        lda [PSRC],y
        and #$00FF
        sta VTT2
        asl
        asl
        asl
        asl
        and #$00F0
        xba
        sta VTT3                ; ungerade Zeile ins obere Byte
        lda VTT2
        and #$00F0
        ora VTT3
        pha
        lda VTT
        asl
        clc
        adc #VT_FONT
        tay
        pla
        sta [PDST],y
        inc VTT
        lda VTT
        cmp #1024
        bcc @u
        jsr net_ptr0
        jsr vt_ptr
        ldx #0                  ; Ausdehnung: Bit 7..4 -> je ein Punkt $F
@e:     stz VTT2
        txa
        bit #$80
        beq @e1
        lda VTT2
        ora #$00F0
        sta VTT2
        txa
@e1:    bit #$40
        beq @e2
        lda VTT2
        ora #$000F
        sta VTT2
        txa
@e2:    bit #$20
        beq @e3
        lda VTT2
        ora #$F000
        sta VTT2
        txa
@e3:    bit #$10
        beq @e4
        lda VTT2
        ora #$0F00
        sta VTT2
@e4:    txa
        asl
        clc
        adc #VT_EXP
        tay
        lda VTT2
        sta [PDST],y
        inx
        cpx #256
        bcc @e
        ldx #0                  ; Palette 15: 16 ANSI-Farben
@p:     lda vt_pal16,x
        sta f:$E19E00+15*32,x
        inx
        inx
        cpx #32
        bcc @p
        lda #$0F0F              ; SCBs: 320er-Modus, Palette 15
        jmp vt_scb

; Apfel-F: Modus wechseln, merken, alles neu zeichnen
vt_toggle:
        jsr vt_curoff
        lda VTCM
        eor #1
        sta VTCM
        sep #$20
        lda NPREF+2
        and #$7F
        ldx VTCM
        bne @c
        ora #$80
@c:     sta NPREF+2
        rep #$20
        JSLF xf_net_save
        jsr vt_mode
        jsr vt_redraw
        jmp vt_curon
vt_redraw:
        jsr vt_ptr
        stz VTCF
        stz VTDY
@y:     stz VTDX
@x:     jsr vt_drw
        inc VTDX
        lda VTDX
        cmp #80
        bcc @x
        inc VTDY
        lda VTDY
        cmp #25
        bcc @y
        rts

; alles zuruecksetzen und loeschen
vt_reset:
        lda #7
        sta VTA
        stz VTINV
        stz VTX
        stz VTY
        stz VTWR
        stz VTST
        stz VTTOP
        lda #24
        sta VTBOT
        lda #1
        sta VTCV
        stz VTCD
        stz VTSX
        stz VTSY
        lda #7
        sta VTSA
        jmp vt_clrall

; ---------- Terminal verlassen ----------
vt_quit:
        lda TNON
        beq vt_leave
        stz TNON
        JSLF xf_tcp_close
vt_leave:
        JSLF xf_xm_kill
        stz VTON
        stz VTWAIT
        JSLF xf_set_palettes
        CALL desktop_scb
        lda DOSON
        beq @tn
        stz DOSON               ; Eingabeaufforderung beendet
        lda DOSCL
        beq @rd
        stz DOSCL
        CALL cur_show
        CALL close_win
        CALL redraw_all
        rts
@tn:    jsr net_ptr0
        WS ws_tnoff
        jsr tn_msg
        jsr tn_view
@rd:    CALL redraw_all
        CALL cur_show
        rts

; ---------- Ereignis aus der Hauptschleife ----------
vt_event:
        lda EVREC
        cmp #3
        beq @k
        cmp #5
        beq @k
        lda TNON
        bne *+5
        jmp @r
        jmp tn_poll
@k:     lda VTWAIT
        beq @k2
        jmp vt_leave
@k2:    lda EVREC+2             ; Strg-] (wie bei Telnet ueblich) beendet
        and #$00FF
        cmp #$1D
        bne @k2a
        jmp vt_quit
@k2a:   lda EVREC+14
        and #$0900              ; offener oder geschlossener Apfel
        beq @k3
        lda EVREC+2
        and #$007F
        cmp #$1B
        beq @q
        ora #$20
        cmp #'f'
        bne @nf
        jmp vt_toggle
@nf:    ldx DOSON               ; Eingabeaufforderung: keine Uebertragung
        bne @nu
        cmp #'d'                ; Apfel-D: Datei empfangen
        bne @nd
        JSLF xf_xm_recv
        rts
@nd:    cmp #'u'                ; Apfel-U: Datei senden
        bne @nu
        JSLF xf_xm_send
        rts
@nu:    cmp #'q'
        bne @r
@q:     jmp vt_quit
@k3:    lda EVREC+2
        and #$00FF
        ldx SSHPW               ; SSH: Benutzer/Passwort eingeben
        beq @k3d
        JSLF xf_ssh_key
        bcc @r
        jmp vt_quit
@k3d:   ldx DOSON
        beq @k3t
        JSLF xf_dos_key
        lda DSQUIT_
        beq @r
        jmp vt_quit
@k3t:   ldx XMON                ; Eingabe / laufende Uebertragung
        beq @k4
        JSLF xf_xm_key
        rts
@k4:    jmp vt_send
@r:     rts

; Taste A senden (Pfeile als ESC [ x)
vt_send:
        sta VTCH
        jsr net_ptr0
        lda #WB_REQ
        sta UWR
        lda VTCH
        ldx #'D'
        cmp #$08
        beq @arr
        ldx #'C'
        cmp #$15
        beq @arr
        ldx #'A'
        cmp #$0B
        beq @arr
        ldx #'B'
        cmp #$0A
        beq @arr
        cmp #$7F                ; Delete -> Rueckschritt
        bne @n1
        lda #$08
        sta VTCH
        bra @one
@n1:    cmp #$0D                ; Return -> CR NUL (SSH: nur CR)
        bne @one
        ldx SSHON
        bne @one
        jsr rq_c
        lda #0
        jsr rq_c
        bra @go
@arr:   phx
        lda #$1B
        jsr rq_c
        lda #'['
        jsr rq_c
        pla
        jsr rq_c
        bra @go
@one:   lda VTCH
        jsr rq_c
@go:    jmp tn_out              ; kein lokales Echo: Mailboxen zeigen selbst an

; Meldung A (Text dieser Codebank) + CR LF
vt_msg: sta VTMP
        lda VTX                 ; in einer neuen Zeile beginnen
        ora VTWR
        beq @nl
        lda #13
        jsr vt_char
        lda #10
        jsr vt_char
@nl:    CBANK
        sta VTMP+2
        lda VTMP
        sta PSRC
        lda VTMP+2
        sta PSRC+2
        lda [PSRC]
        and #$00FF
        sta VTMN
        ldy #1
@l:     lda VTMN
        beq @e
        lda VTMP
        sta PSRC
        lda VTMP+2
        sta PSRC+2
        lda [PSRC],y
        and #$00FF
        phy
        jsr vt_char
        ply
        iny
        dec VTMN
        bra @l
@e:     lda #13
        jsr vt_char
        lda #10
        jsr vt_char
        jmp net_ptr0

; ---------- Schreibmarke ----------
vt_curoff:
        lda VTCD
        beq @r
        stz VTCD
        jsr vt_ptr
        lda VTCX
        sta VTDX
        lda VTCY
        sta VTDY
        stz VTCF
        jsr vt_drw
@r:     rts
vt_curon:
        lda VTCV
        beq @r
        lda VTWAIT
        bne @r
        jsr vt_ptr
        lda VTX
        cmp #80
        bcc @x
        lda #79
@x:     sta VTCX
        sta VTDX
        lda VTY
        sta VTCY
        sta VTDY
        lda #1
        sta VTCF
        sta VTCD
        jsr vt_drw
        stz VTCF
@r:     rts

; ---------- ein empfangenes Byte ----------
vt_char:
        and #$00FF
        sta VTCH
        jsr vt_ptr
        lda VTST
        bne @st
        lda VTCH
        cmp #$20
        bcs @pr
        jmp vt_ctl
@pr:    cmp #$7F
        beq @r
        jmp vt_print
@r:     rts
@st:    cmp #1
        bne @s2
        jmp vt_esc
@s2:    cmp #2
        bne @s3
        jmp vt_csi
@s3:    stz VTST
        rts

; Steuerzeichen (A = VTCH)
vt_ctl: cmp #$1B
        bne @n1
        lda #1
        sta VTST
        rts
@n1:    cmp #13
        bne @n2
        stz VTX
        stz VTWR
        rts
@n2:    cmp #10
        beq @lf
        cmp #11
        beq @lf
        cmp #12                 ; Seitenvorschub: Bildschirm loeschen
        bne @n3
        jsr vt_clrall
        stz VTX
        stz VTY
        stz VTWR
        rts
@lf:    jmp vt_lf
@n3:    cmp #8
        bne @n4
        stz VTWR
        lda VTX
        beq @r
        dec VTX
@r:     rts
@n4:    cmp #9
        bne @r
        lda VTX
        ora #7
        inc a
        cmp #80
        bcc @t
        lda #79
@t:     sta VTX
        rts

; Zeile weiter (am unteren Rand rollen)
vt_lf:  stz VTWR
        lda VTY
        cmp VTBOT
        bne @n
        jmp vt_scup
@n:     cmp #24
        bcs @r
        inc VTY
@r:     rts
; Zeile zurueck (am oberen Rand rollen)
vt_ri:  stz VTWR
        lda VTY
        cmp VTTOP
        bne @n
        jmp vt_scdn
@n:     lda VTY
        beq @r
        dec VTY
@r:     rts

; druckbares Zeichen VTCH
vt_print:
        lda VTWR
        beq @nw
        stz VTWR
        stz VTX
        jsr vt_lf
@nw:    jsr vt_cattr
        xba
        ora VTCH
        pha
        lda VTX
        sta VTDX
        lda VTY
        sta VTDY
        jsr vt_ci
        pla
        sta [PDST],y
        stz VTCF
        jsr vt_drw
        lda VTX
        cmp #79
        bcc @adv
        lda #1
        sta VTWR
        rts
@adv:   inc VTX
        rts

; Attribut zum Schreiben (invers: Vorder-/Hintergrund getauscht)
vt_cattr:
        lda VTA
        ldx VTINV
        beq @r
        and #7
        asl
        asl
        asl
        sta VTT
        lda VTA
        lsr
        lsr
        lsr
        and #7
        ora VTT
        sta VTT
        lda VTA
        and #$C0
        ora VTT
@r:     and #$00FF
        rts

; Y = Zellenadresse von VTDX/VTDY
vt_ci:  lda VTDY
        asl
        asl
        asl
        asl
        asl
        sta VTT2                ; *32
        asl
        asl                     ; *128
        clc
        adc VTT2
        adc VTDX
        adc VTDX
        tay
        rts

; ---------- Zelle VTDX/VTDY zeichnen (VTCF: invers) ----------
vt_drw: jsr vt_ci
        lda [PDST],y
        pha
        and #$00FF
        asl
        asl
        asl
        clc
        adc #VT_FONT
        sta VTG
        clc
        adc #8
        sta VTGE
        pla
        xba
        and #$00FF
        sta VTT                 ; Attribut
        lda VTCM
        beq @mono
        jsr vt_colw
        jmp @nc
@mono:  lda VTT
        and #7
        tax
        lda vt_lvf,x
        and #$00FF
        sta VTFL
        lda VTT
        bit #$40                ; hell
        beq @nb
        lda VTFL
        cmp #3
        bcs @nb
        inc VTFL
@nb:    lda VTT
        lsr
        lsr
        lsr
        and #7
        tax
        lda vt_lvf,x
        and #$00FF
        sta VTBL
        cmp VTFL                ; gleiche Stufe bei verschiedener Farbe: lesbar machen
        bne @ok
        lda VTT
        and #7
        sta VTT2
        lda VTT
        lsr
        lsr
        lsr
        and #7
        cmp VTT2
        beq @ok
        lda #3
        ldx VTBL
        cpx #2
        bcc @s
        lda #0
@s:     sta VTFL
@ok:    lda VTT
        and #$80
        sta VTUL
        lda VTFL
        asl
        tax
        lda vt_patw,x
        sta VTFGW
        lda VTBL
        asl
        tax
        lda vt_patw,x
        sta VTBGW
        lda VTCF                ; Schreibmarke: invers
        beq @nc
        lda VTFGW
        ldx VTBGW
        sta VTBGW
        stx VTFGW
@nc:    lda VTFGW
        eor VTBGW
        sta VTXW
        ldy #VT_TMP             ; fuer die Schleife in die Bank IMBK
        sta [PDST],y
        ldy #VT_TMP+2
        lda VTBGW
        sta [PDST],y
        ldy #VT_TMP+4
        lda VTGE
        sta [PDST],y
        lda VTDY                ; Bildschirm: Zeile*1280 + Spalte*2
        xba
        sta VTT2
        asl
        asl
        clc
        adc VTT2
        adc VTDX
        adc VTDX
        sta VTSO
        tax
        ldy VTG
        phb
        sep #$20
        lda IMBK
        pha
        plb
        rep #$20
@r:     lda VT_FONT-VT_FONT,y   ; Schrift (Bank IMBK)
        and #$00FF
        asl
        phx
        tax
        lda VT_EXP,x
        and VT_TMP
        eor VT_TMP+2
        plx
        sta f:$E12000,x
        txa
        clc
        adc #160
        tax
        iny
        cpy VT_TMP+4
        bcc @r
        plb
        lda VTUL                ; unterstrichen: letzte Punktzeile voll
        beq @x
        lda VTSO
        clc
        adc #7*160
        tax
        lda VTXW
        eor VTBGW
        sta f:$E12000,x
@x:     rts

; Farbmodus: Muster aus Attribut VTT (hell = Farbe + 8)
vt_colw:
        lda VTT
        and #$40
        beq @n
        lda #8
@n:     sta VTT2
        lda VTT
        and #7
        ora VTT2
        asl
        tax
        lda vt_c16,x
        sta VTFGW
        lda VTT
        lsr
        lsr
        lsr
        and #7
        asl
        tax
        lda vt_c16,x
        sta VTBGW
        lda VTT
        and #$80
        sta VTUL
        rts

; ---------- Loeschen ----------
; Loeschzelle/-muster aus dem aktuellen Hintergrund
vt_eset:
        lda VTA
        and #$38
        ora #$07
        xba
        ora #$20
        sta VTEW
        lda VTA
        lsr
        lsr
        lsr
        and #7
        ldx VTCM
        beq @g
        asl
        tax
        lda vt_c16,x
        sta VTEP
        rts
@g:     tax
        lda vt_lvf,x
        and #$00FF
        asl
        tax
        lda vt_patw,x
        sta VTEP
        rts
; Zeile VTDY, Spalten VTE0..VTE1
vt_erase:
        lda VTE1
        cmp VTE0
        bcs @go
        rts
@go:    jsr vt_eset
        jsr vt_ptr
        lda VTE0
        sta VTDX
        jsr vt_ci
        lda VTE1
        sec
        sbc VTE0
        inc a
        sta VTN                 ; Zellen
@c:     lda VTEW
        sta [PDST],y
        iny
        iny
        dec VTN
        bne @c
        lda VTDY                ; Bildschirm: 8 Punktzeilen
        xba
        sta VTT2
        asl
        asl
        clc
        adc VTT2
        adc VTE0
        adc VTE0
        sta VTSO
        lda VTE1
        sec
        sbc VTE0
        inc a
        asl
        sta VTT                 ; Bytes je Punktzeile
        lda #8
        sta VTN
@pl:    ldx VTSO
        ldy VTT
        lda VTEP
@pw:    sta f:$E12000,x
        inx
        inx
        dey
        dey
        bne @pw
        lda VTSO
        clc
        adc #160
        sta VTSO
        dec VTN
        bne @pl
        rts
; ganze Zeilen VTE0..VTE1 (Zeilennummern) loeschen
vt_erows:
        lda VTE0
        sta VTT
        lda VTE1
        sta VTMN
@l:     lda VTT
        cmp VTMN
        beq @d
        bcs @r
@d:     sta VTDY
        lda VTT
        pha
        stz VTE0
        lda #79
        sta VTE1
        jsr vt_erase
        pla
        sta VTT
        inc VTT
        bra @l
@r:     rts
vt_clrall:
        stz VTE0
        lda #24
        sta VTE1
        jmp vt_erows

; ---------- Rollen im Bereich VTTOP..VTBOT ----------
vt_scup:
        jsr vt_ptr
        lda VTBOT
        sec
        sbc VTTOP
        beq @last
        sta VTN                 ; Zeilen zu verschieben
        ; Zellen: (bot-top)*160 Bytes ab top*160 um 160 nach vorn
        lda VTTOP
        sta VTDY
        stz VTDX
        jsr vt_ci
        lda VTN
        asl
        asl
        asl
        asl
        asl
        sta VTT2
        asl
        asl
        clc
        adc VTT2                ; *160
        tax
        phb
        sep #$20
        lda IMBK
        pha
        plb
        rep #$20
@c:     lda $00A0,y
        sta $0000,y
        iny
        iny
        dex
        dex
        bne @c
        plb
        ; Bildschirm: MVN
        lda VTTOP
        xba
        sta VTT2
        asl
        asl
        clc
        adc VTT2                ; top*1280
        adc #$2000
        tay                     ; Ziel
        clc
        adc #1280
        tax                     ; Quelle
        lda VTN
        xba
        sta VTT2
        asl
        asl
        clc
        adc VTT2                ; Zeilen*1280
        dec a
        phb
        .byte $54,$E1,$E1       ; MVN $E1,$E1
        plb
@last:  lda VTBOT
        sta VTDY
        stz VTE0
        lda #79
        sta VTE1
        jmp vt_erase

vt_scdn:
        jsr vt_ptr
        lda VTBOT
        sec
        sbc VTTOP
        beq @first
        sta VTN
        ; Zellen rueckwaerts: Zeilen top..bot-1 nach top+1..bot
        lda VTBOT
        sta VTDY
        stz VTDX
        jsr vt_ci               ; Anfang der Zeile bot
        dey
        dey                     ; letztes Wort der Zeile bot-1
        lda VTN
        asl
        asl
        asl
        asl
        asl
        sta VTT2
        asl
        asl
        clc
        adc VTT2
        tax
        phb
        sep #$20
        lda IMBK
        pha
        plb
        rep #$20
@c:     lda $0000,y
        sta $00A0,y
        dey
        dey
        dex
        dex
        bne @c
        plb
        ; Bildschirm: MVP von hinten
        lda VTBOT
        xba
        sta VTT2
        asl
        asl
        clc
        adc VTT2                ; bot*1280
        adc #$2000-1
        tax                     ; Quelle: Ende Zeile bot-1
        clc
        adc #1280
        tay
        lda VTN
        xba
        sta VTT2
        asl
        asl
        clc
        adc VTT2
        dec a
        phb
        .byte $44,$E1,$E1       ; MVP $E1,$E1
        plb
@first: lda VTTOP
        sta VTDY
        stz VTE0
        lda #79
        sta VTE1
        jmp vt_erase

; ---------- ESC x ----------
vt_esc: stz VTST
        lda VTCH
        cmp #'['
        bne @n1
        lda #2
        sta VTST
        stz VTNP
        stz VTPRIV
        ldx #14
@z:     stz VTPAR,x
        dex
        dex
        bpl @z
        rts
@n1:    cmp #'7'
        bne @n2
        jmp vt_save
@n2:    cmp #'8'
        bne @n3
        jmp vt_rest
@n3:    cmp #'D'
        bne @n4
        jmp vt_lf
@n4:    cmp #'M'
        bne @n5
        jmp vt_ri
@n5:    cmp #'E'
        bne @n6
        stz VTX
        jmp vt_lf
@n6:    cmp #'c'
        bne @n7
        jmp vt_reset
@n7:    cmp #'('
        beq @sk
        cmp #')'
        bne @r
@sk:    lda #3
        sta VTST
@r:     rts

vt_save:
        lda VTX
        sta VTSX
        lda VTY
        sta VTSY
        lda VTA
        sta VTSA
        rts
vt_rest:
        lda VTSX
        sta VTX
        lda VTSY
        sta VTY
        lda VTSA
        sta VTA
        stz VTWR
        rts

; ---------- CSI ----------
vt_csi: lda VTCH
        cmp #'0'
        bcc @n
        cmp #'9'+1
        bcs @n
        sec                     ; Ziffer: Parameter = Parameter*10 + Ziffer
        sbc #'0'
        pha
        lda VTNP
        asl
        tax
        lda VTPAR,x
        asl
        sta VTT
        asl
        asl
        clc
        adc VTT
        sta VTT
        pla
        clc
        adc VTT
        sta VTPAR,x
        rts
@n:     cmp #';'
        bne @n1
        lda VTNP
        cmp #7
        bcs @r
        inc VTNP
@r:     rts
@n1:    cmp #'?'
        beq @pv
        cmp #'>'
        beq @pv
        cmp #'='
        bne @n2
@pv:    lda #1
        sta VTPRIV
        rts
@n2:    cmp #$1B                ; neue Folge
        bne @n3
        lda #1
        sta VTST
        rts
@n3:    cmp #$20
        bcs @n4
        jmp vt_ctl              ; Steuerzeichen mittendrin
@n4:    cmp #$40
        bcc @r                  ; Zwischenzeichen
        stz VTST
        inc VTNP                ; Anzahl Parameter
        stz VTWR
        lda VTCH
        ; Endzeichen auswerten
        cmp #'m'
        bne @f1
        jmp vt_sgr
@f1:    cmp #'H'
        beq @cup
        cmp #'f'
        bne @f2
@cup:   jsr vt_p1
        dec a
        jsr vt_sety
        lda VTPAR+2
        bne @c2
        inc a
@c2:    dec a
        jmp vt_setx
@f2:    cmp #'A'
        bne @f3
        jsr vt_p1
        sta VTT
        lda VTY
        sec
        sbc VTT
        bcs @a1
        lda #0
@a1:    cmp VTTOP               ; nicht ueber den Rollbereich, wenn darin
        bcs @a2
        ldx VTY
        cpx VTTOP
        bcc @a2
        lda VTTOP
@a2:    sta VTY
        rts
@f3:    cmp #'B'
        bne @f4
        jsr vt_p1
        clc
        adc VTY
        cmp VTBOT
        bcc @b1
        ldx VTY
        cpx VTBOT
        bcs @b0
        lda VTBOT
        bra @b1
@b0:    cmp #25
        bcc @b1
        lda #24
@b1:    sta VTY
        rts
@f4:    cmp #'C'
        bne @f5
        jsr vt_p1
        clc
        adc VTX
        jmp vt_setx
@f5:    cmp #'D'
        bne @f6
        jsr vt_p1
        sta VTT
        lda VTX
        sec
        sbc VTT
        bcs @d1
        lda #0
@d1:    sta VTX
        rts
@f6:    cmp #'G'
        beq @g
        cmp #'`'
        bne @f7
@g:     jsr vt_p1
        dec a
        jmp vt_setx
@f7:    cmp #'d'
        bne @f8
        jsr vt_p1
        dec a
        jmp vt_sety
@f8:    cmp #'J'
        bne @f9
        jmp vt_ed
@f9:    cmp #'K'
        bne @f10
        jmp vt_el
@f10:   cmp #'X'
        bne @f11
        jsr vt_p1
        clc
        adc VTX
        dec a
        cmp #80
        bcc @x1
        lda #79
@x1:    sta VTE1
        lda VTX
        sta VTE0
        lda VTY
        sta VTDY
        jmp vt_erase
@f11:   cmp #'P'
        bne @f12
        jmp vt_dch
@f12:   cmp #'@'
        bne @f13
        jmp vt_ich
@f13:   cmp #'L'
        bne @f14
        jmp vt_il
@f14:   cmp #'M'
        bne @f15
        jmp vt_dl
@f15:   cmp #'r'
        bne @f16
        lda VTPRIV
        bne @rr
        jsr vt_p1
        dec a
        sta VTT
        lda VTPAR+2
        bne @r2
        lda #25
@r2:    dec a
        cmp #25
        bcc @r3
        lda #24
@r3:    cmp VTT
        beq @rr
        bcc @rr
        sta VTBOT
        lda VTT
        sta VTTOP
        stz VTX
        stz VTY
@rr:    rts
@f16:   cmp #'s'
        bne @f17
        jmp vt_save
@f17:   cmp #'u'
        bne @f18
        jmp vt_rest
@f18:   cmp #'n'
        bne @f19
        jmp vt_dsr
@f19:   cmp #'c'
        bne @f20
        jmp vt_da
@f20:   cmp #'h'
        beq @hl
        cmp #'l'
        bne @f21
@hl:    lda VTPRIV              ; ?25 h/l: Schreibmarke an/aus
        beq @rr
        lda VTPAR
        cmp #25
        bne @rr
        stz VTCV
        lda VTCH
        cmp #'h'
        bne @rr
        inc VTCV
        rts
@f21:   cmp #'S'
        bne @f22
        jsr vt_rcnt
@su:    jsr vt_scup
        dec VTRC
        bne @su
        rts
@f22:   cmp #'T'
        bne @rr
        jsr vt_rcnt
@sd:    jsr vt_scdn
        dec VTRC
        bne @sd
        rts

; VTRC = erster Parameter, hoechstens Hoehe des Rollbereichs
vt_rcnt:
        lda VTBOT
        sec
        sbc VTTOP
        inc a
        sta VTT
        jsr vt_p1
        cmp VTT
        bcc @c
        lda VTT
@c:     sta VTRC
        rts

; erster Parameter, 0 -> 1 (hoechstens 80)
vt_p1:  lda VTPAR
        bne @n
        inc a
@n:     cmp #81
        bcc @r
        lda #80
@r:     rts
vt_setx:
        cmp #80
        bcc @s
        lda #79
@s:     sta VTX
        rts
vt_sety:
        cmp #25
        bcc @s
        lda #24
@s:     sta VTY
        rts

; ESC [ n J
vt_ed:  lda VTPAR
        cmp #1
        beq @b
        cmp #2
        bcs @all
        lda VTY                 ; ab der Schreibmarke
        sta VTDY
        lda VTX
        sta VTE0
        lda #79
        sta VTE1
        jsr vt_erase
        lda VTY
        cmp #24
        bcs @r
        inc a
        sta VTE0
        lda #24
        sta VTE1
        jmp vt_erows
@b:     lda VTY                 ; bis zur Schreibmarke
        beq @b1
        dec a
        sta VTE1
        stz VTE0
        jsr vt_erows
@b1:    lda VTY
        sta VTDY
        stz VTE0
        lda VTX
        sta VTE1
        jmp vt_erase
@all:   jsr vt_clrall           ; wie ANSI.SYS: auch nach oben links
        stz VTX
        stz VTY
@r:     rts
; ESC [ n K
vt_el:  lda VTY
        sta VTDY
        lda VTPAR
        cmp #1
        beq @b
        bcs @all
        lda VTX
        sta VTE0
        lda #79
        sta VTE1
        jmp vt_erase
@b:     stz VTE0
        lda VTX
        sta VTE1
        jmp vt_erase
@all:   stz VTE0
        lda #79
        sta VTE1
        jmp vt_erase

; Zeichen loeschen (P) / einfuegen (@): Zeile ab VTX verschieben, neu zeichnen
vt_dch: jsr vt_p1
        sta VTN
        lda VTY
        sta VTDY
        lda VTX
        sta VTT                 ; Ziel
@l:     lda VTT
        clc
        adc VTN
        cmp #80
        bcs @f
        sta VTDX
        jsr vt_ci
        lda [PDST],y
        pha
        lda VTT
        sta VTDX
        jsr vt_ci
        pla
        sta [PDST],y
        inc VTT
        bra @l
@f:     jsr vt_eset             ; Rest leer
@f2:    lda VTT
        cmp #80
        bcs @d
        sta VTDX
        jsr vt_ci
        lda VTEW
        sta [PDST],y
        inc VTT
        bra @f2
@d:     jmp vt_rdrow
vt_ich: jsr vt_p1
        sta VTN
        lda VTY
        sta VTDY
        lda #79
        sta VTT                 ; Ziel, von rechts
@l:     lda VTT
        sec
        sbc VTN
        bcc @f
        cmp VTX
        bcc @f
        sta VTDX
        jsr vt_ci
        lda [PDST],y
        pha
        lda VTT
        sta VTDX
        jsr vt_ci
        pla
        sta [PDST],y
        dec VTT
        bra @l
@f:     jsr vt_eset
@f2:    lda VTT
        bmi @d
        cmp VTX
        bcc @d
        sta VTDX
        jsr vt_ci
        lda VTEW
        sta [PDST],y
        dec VTT
        bra @f2
@d:     ; fallthrough
vt_rdrow:                       ; Zeile VTDY ab VTX neu zeichnen
        lda VTX
        sta VTDX
@l:     stz VTCF
        jsr vt_drw
        inc VTDX
        lda VTDX
        cmp #80
        bcc @l
        rts

; Zeilen einfuegen (L) / loeschen (M) ab der Schreibmarke
vt_il:  jsr vt_lnset
        bcs @r
@l:     jsr vt_scdn
        dec VTRC
        bne @l
        jmp vt_lnend
@r:     rts
vt_dl:  jsr vt_lnset
        bcs @r
@l:     jsr vt_scup
        dec VTRC
        bne @l
        jmp vt_lnend
@r:     rts
vt_lnset:
        lda VTY
        cmp VTTOP
        bcc @no
        cmp VTBOT
        beq @ok
        bcs @no
@ok:    lda VTBOT               ; hoechstens bis zum Bereichsende
        sec
        sbc VTY
        inc a
        sta VTT
        jsr vt_p1
        cmp VTT
        bcc @c
        lda VTT
@c:     sta VTRC
        lda VTTOP
        sta VTOT
        lda VTY
        sta VTTOP
        stz VTX
        clc
        rts
@no:    sec
        rts
vt_lnend:
        lda VTOT
        sta VTTOP
        rts

; ESC [ ... m
vt_sgr: ldx #0
@l:     txa
        lsr
        cmp VTNP
        bcc @g
        rts
@g:     lda VTPAR,x
        phx
        jsr vt_sg1
        plx
        inx
        inx
        bra @l
vt_sg1: cmp #0
        bne @n1
        lda #7
        sta VTA
        stz VTINV
        rts
@n1:    cmp #1
        bne @n2
        lda VTA
        ora #$40
        sta VTA
        rts
@n2:    cmp #22
        beq @nb
        cmp #2
        bne @n3
@nb:    lda VTA
        and #$FFBF
        sta VTA
        rts
@n3:    cmp #4
        bne @n4
        lda VTA
        ora #$80
        sta VTA
        rts
@n4:    cmp #24
        bne @n5
        lda VTA
        and #$FF7F
        sta VTA
        rts
@n5:    cmp #7
        bne @n6
        lda #1
        sta VTINV
        rts
@n6:    cmp #27
        bne @n7
        stz VTINV
        rts
@n7:    cmp #30
        bcc @r
        cmp #38
        bcs @n8
        sbc #30-1               ; C=0: -30
@fg:    sta VTT
        lda VTA
        and #$FFF8
        ora VTT
        sta VTA
        rts
@n8:    cmp #39
        bne @n9
        lda #7
        bra @fg
@n9:    cmp #40
        bcc @r
        cmp #48
        bcs @n10
        sbc #40-1
@bg:    asl
        asl
        asl
        sta VTT
        lda VTA
        and #$FFC7
        ora VTT
        sta VTA
        rts
@n10:   cmp #49
        bne @n11
        lda #0
        bra @bg
@n11:   cmp #90                 ; helle Farben
        bcc @r
        cmp #98
        bcs @n12
        sbc #90-1
        pha
        lda VTA
        ora #$40
        sta VTA
        pla
        bra @fg
@n12:   cmp #100
        bcc @r
        cmp #108
        bcs @r
        sbc #100-1
        bra @bg
@r:     rts

; ESC [ 6 n -> ESC [ zeile ; spalte R ;  ESC [ 5 n -> ESC [ 0 n
vt_dsr: lda VTPAR
        cmp #6
        beq @pos
        cmp #5
        bne @r
        jsr tn_qbeg
        jsr vt_csiq
        lda #'0'
        jsr rq_c
        lda #'n'
        jsr rq_c
        jmp tn_qend
@pos:   jsr tn_qbeg
        jsr vt_csiq
        lda VTY
        inc a
        jsr vt_dec
        lda #';'
        jsr rq_c
        lda VTX
        cmp #80
        bcc @x
        lda #79
@x:     inc a
        jsr vt_dec
        lda #'R'
        jsr rq_c
        jmp tn_qend
@r:     rts
; ESC [ c -> ESC [ ? 1 ; 0 c  (VT100)
vt_da:  lda VTPRIV
        bne @r
        jsr tn_qbeg
        jsr vt_csiq
        ldx #0
@l:     lda vt_dast,x
        and #$00FF
        beq @e
        phx
        jsr rq_c
        plx
        inx
        bra @l
@e:     jmp tn_qend
@r:     rts
vt_csiq:
        jsr net_ptr0
        lda #$1B
        jsr rq_c
        lda #'['
        jmp rq_c
; A (1..99) dezimal in die Antwort
vt_dec: ldx #0
@t:     cmp #10
        bcc @o
        sbc #10
        inx
        bra @t
@o:     pha
        txa
        beq @u
        clc
        adc #'0'
        jsr rq_c
@u:     pla
        clc
        adc #'0'
        jmp rq_c

        .pushseg
        .segment "RODATA"
vt_dast: .byte "?1;0c",0
        .popseg

        .include "vtfont.inc"
        .include "vtfont4.inc"
