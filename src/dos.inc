; ================================================================
; GS/OS95: MS-DOS-Eingabeaufforderung (zum Spass), XCODE4 (x4.s)
; Laeuft im Vollbild-Terminal (vt.inc, DOSON): Ausgabe ueber x4_put,
; Tasten kommen aus vt_event. Laufwerk C: ist das Startvolume.
; Nur lesend: DIR, CD, TYPE, DATE, TIME, MEM, VER, COLOR, CLS, ECHO.
; DEL, COPY, FORMAT ... aendern nichts.
; about:msdos startet sie im Browserfenster, Start > Programme > Werkzeuge
; > MS-DOS ebenso (Fenster schliesst sich bei EXIT).
; ================================================================
        .pushseg
        .segment "BSS"
DOSON:  .res 2                  ; Eingabeaufforderung hat das Terminal
DOSCL:  .res 2                  ; bei EXIT Fenster schliessen
DSQUIT_:  .res 2
DSNOBL_:  .res 2                  ; keine Leerzeile vor der Eingabe
DSLN_:    .res 2
DSLLN_:   .res 2
DSMODE_:  .res 2                  ; 1 TYPE, 2 DIR laeuft (seitenweise)
DSLC_:    .res 2                  ; Zeilen seit der letzten Pause
DSCOL_:   .res 2
DSKEY_:   .res 2
DST_:     .res 2
DST3_:    .res 2
DSI_:     .res 2                  ; Befehlswort von
DSE_:     .res 2                  ; bis
DSA_:     .res 2                  ; Argument
DSCI_:    .res 2
DSCL_:    .res 2
DSN_:     .res 4
DSSUM_:   .res 4
DSCNT_:   .res 2
DSDIRS_:  .res 2
DSREF_:   .res 2
DSRPOS_:  .res 2
DSRLEN_:  .res 2
DSLCR_:   .res 2
DSNL_:    .res 2
DSNB_:    .res 12
DSLINE_:  .res 80
DSUP_:    .res 80
DSLAST_:  .res 80
DSPATH_:  .res 82                 ; GS/OS-Pfad (Laenge + Zeichen)
DSROOT_:  .res 2                  ; Laenge des Volume-Teils
DSARG_:   .res 82
DSPB_:    .res 44
DSRES_:   .res 36
DSBUF_:   .res 256
        .popseg

; ---------- Einspruenge ----------
xf_dos_start:
        stz DSQUIT_
        stz DSLN_
        stz DSLLN_
        stz DSMODE_
        stz DSNOBL_
        jsr d_boot
        lda #12                 ; Bildschirm loeschen
        jsr x4_put
        lda #DS_BANNER
        jsr d_msg
        jsr d_prompt
        lda #1
        jsr x4_cur
        rtl

; Taste A
xf_dos_key:
        sta DSKEY_
        lda #0
        jsr x4_cur
        lda DSMODE_
        beq @in
        jsr d_morekey
        bra @x
@in:    lda DSKEY_
        cmp #$0D
        bne @k1
        jsr d_enter
        bra @x
@k1:    cmp #$08
        beq @bs
        cmp #$7F
        beq @bs
        cmp #$1B
        beq @esc
        cmp #$0B                ; Pfeil hoch: letzter Befehl
        beq @up
        cmp #$20
        bcc @x
        cmp #$7F
        bcs @x
        ldx DSLN_
        cpx #72
        bcs @x
        sep #$20
        sta DSLINE_,x
        rep #$20
        inc DSLN_
        jsr x4_put
        bra @x
@bs:    jsr d_bs
        bra @x
@esc:   jsr d_kill
        bra @x
@up:    jsr d_kill
@ul:    ldx DSLN_
        cpx DSLLN_
        bcs @x
        lda DSLAST_,x
        and #$00FF
        sep #$20
        sta DSLINE_,x
        rep #$20
        inc DSLN_
        jsr x4_put
        bra @ul
@x:     lda DSQUIT_
        bne @q
        lda #1
        jsr x4_cur
@q:     rtl

d_bs:   lda DSLN_
        beq @r
        dec DSLN_
        lda #8
        jsr x4_put
        lda #' '
        jsr x4_put
        lda #8
        jmp x4_put
@r:     rts
d_kill: lda DSLN_
        beq @r
        jsr d_bs
        bra d_kill
@r:     rts

; ---------- GS/OS: A = Aufruf, X = Parameterblock (Hauptbank) ----------
d_gs:   sta DST_
        lda MYBANK
        pha
        phx
        lda DST_
        pha
        jsl $E100B0
        rts

; Startvolume nach DSPATH_ (":NAME"), sonst "*"
d_boot: lda #1
        sta DSPB_
        lda #DSRES_
        sta DSPB_+2
        lda MYBANK
        sta DSPB_+4
        lda #36
        sta DSRES_
        lda #$2028              ; GetBootVol
        ldx #DSPB_
        jsr d_gs
        bcs @star
        ldx #0
        ldy #0
        sep #$20
        lda DSRES_+4
        cmp #':'
        beq @l
        lda #':'
        sta DSPATH_+2
        inx
@l:     cpy DSRES_+2
        bcs @e
        lda DSRES_+4,y
        sta DSPATH_+2,x
        iny
        inx
        cpx #64
        bcc @l
@e:     dex                     ; ':' am Ende weg
        lda DSPATH_+2,x
        cmp #':'
        beq @e2
        inx
@e2:    rep #$20
        cpx #2
        bcc @star
        bra @set
@star:  lda #'*'
        sta DSPATH_+2
        ldx #1
@set:   stx DSPATH_
        stx DSROOT_
        rts

; ---------- Ausgabe ----------
d_msg:  pha                     ; Text Nr. A (Deutsch/Englisch)
        asl
        tax
        TBL ds_de
        ldy LANG
        beq @d
        TBL ds_en
@d:     sta LP2
        pla
        cmp #DS_X2              ; lange Texte liegen bei den Browsertexten
        bcs @x2
        phk
        phk
        pla
        and #$00FF
        bra @b
@x2:    lda STRBK
@b:     sta LP2+2
        lda [LP2]
        and #$00FF
        sta DST_
        ldy #1
@l:     lda DST_
        beq @r
        lda [LP2],y
        and #$00FF
        phy
        jsr x4_put
        ply
        iny
        dec DST_
        bra @l
@r:     rts

d_nl:   inc DSLC_
        stz DSCOL_
        jmp x4_nl

d_upc:  cmp #'a'
        bcc @r
        cmp #'z'+1
        bcs @r
        and #$00DF
@r:     rts

; A Leerzeichen
d_spc:  tax
        beq @r
        bmi @r
@l:     phx
        lda #' '
        jsr x4_put
        plx
        dex
        bne @l
@r:     rts

; A (0..99) zweistellig
d_2d:   ldx #0
@l:     cmp #10
        bcc @o
        sbc #10
        inx
        bra @l
@o:     pha
        txa
        ora #'0'
        jsr x4_put
        pla
        ora #'0'
        jmp x4_put

; DSN_ (32 Bit) mit Tausenderpunkten, rechtsbuendig in A Stellen
d_num:  sta DST_
        ldx #0
@dv:    lda #0                  ; DSN_ / 10, Rest in A
        ldy #32
@b:     asl DSN_
        rol DSN_+2
        rol a
        cmp #10
        bcc @n
        sbc #10
        inc DSN_
@n:     dey
        bne @b
        ora #'0'
        sta DSNB_,x
        inx
        lda DSN_
        ora DSN_+2
        bne @dv
        stx DSNL_
        txa                     ; Breite = Ziffern + (Ziffern-1)/3
        dec a
        ldy #0
@d3:    cmp #3
        bcc @d3e
        sbc #3
        iny
        bra @d3
@d3e:   sta DST3_
        tya
        clc
        adc DSNL_
        eor #$FFFF
        sec
        adc DST_
        jsr d_spc
        ldx DSNL_
@pr:    dex
        lda DSNB_,x
        and #$00FF
        phx
        jsr x4_put
        plx
        txa
        beq @r
        lda DST3_
        beq @sep
        dec DST3_
        bra @pr
@sep:   lda #2
        sta DST3_
        lda #'.'
        ldy LANG
        beq @p
        lda #','
@p:     phx
        jsr x4_put
        plx
        bra @pr
@r:     rts

; Eingabe: "C:\PFAD>"
d_prompt:
        lda #'C'
        jsr x4_put
        lda #':'
        jsr x4_put
        jsr d_cwd
        lda #'>'
        jmp x4_put
d_cwd:  lda #'\'
        jsr x4_put
        ldx DSROOT_
        inx
@l:     cpx DSPATH_
        bcs @r
        lda DSPATH_+2,x
        and #$00FF
        cmp #':'
        bne @p
        lda #'\'
@p:     jsr d_upc
        phx
        jsr x4_put
        plx
        inx
        bra @l
@r:     rts

; ---------- Return: Zeile ausfuehren ----------
d_enter:
        jsr x4_nl
        stz DSLC_
        stz DSCOL_
@tr:    ldx DSLN_                 ; Leerzeichen am Ende weg
        beq @cp
        lda DSLINE_-2,x
        and #$FF00
        cmp #$2000
        bne @cp
        dec DSLN_
        bra @tr
@cp:    ldx #0
@c:     cpx DSLN_
        bcs @cd
        lda DSLINE_,x
        and #$00FF
        jsr d_upc
        sep #$20
        sta DSUP_,x
        lda DSLINE_,x
        sta DSLAST_,x
        rep #$20
        inx
        bra @c
@cd:    lda DSLN_
        beq @ex
        sta DSLLN_
@ex:    jsr d_exec
        stz DSLN_
        lda DSQUIT_
        ora DSMODE_
        bne @r
        lda DSNOBL_
        bne @nb
        jsr x4_nl
@nb:    stz DSNOBL_
        jmp d_prompt
@r:     rts

d_exec: ldx #0
@sp:    cpx DSLN_
        bcc @s1
        inc DSNOBL_               ; leere Zeile
        rts
@s1:    lda DSUP_,x
        and #$00FF
        cmp #' '
        bne @w
        inx
        bra @sp
@w:     stx DSI_
@we:    cpx DSLN_
        bcs @wd
        lda DSUP_,x
        and #$00FF
        cmp #' '
        beq @wd
        cmp #'.'
        beq @wd
        cmp #'\'
        beq @wd
        inx
        bra @we
@wd:    stx DSE_
@as:    cpx DSLN_
        bcs @ad
        lda DSUP_,x
        and #$00FF
        cmp #' '
        bne @ad
        inx
        bra @as
@ad:    stx DSA_
        lda DSE_                  ; Laufwerk "X:"
        sec
        sbc DSI_
        sta DSCL_
        cmp #2
        bne @look
        ldx DSI_
        lda DSUP_+1,x
        and #$00FF
        cmp #':'
        bne @look
        lda DSUP_,x
        and #$00FF
        cmp #'C'
        bne @drv
        inc DSNOBL_
        rts
@drv:   pha
        lda #DS_DRV
        jsr d_msg
        pla
        jsr x4_put
        jmp x4_nl
@look:  lda #dos_cmds
        jsr x4_kb
        stz DSCI_
@cl:    lda [LP2]
        and #$00FF
        bne @c0
        lda #DS_BAD
        jmp d_msg
@c0:    sta DST_
        cmp DSCL_
        bne @nx
        ldy #1
        ldx DSI_
@cc:    lda [LP2],y
        eor DSUP_,x
        and #$00FF
        bne @nx
        inx
        iny
        dec DST_
        bne @cc
        lda DSCI_
        asl
        tax
        jmp (dos_jt,x)
@nx:    lda [LP2]
        and #$00FF
        sec
        adc LP2
        sta LP2
        inc DSCI_
        bra @cl

dos_cmds:
        .byte 2,"CD",5,"CHDIR",3,"CLS",5,"COLOR",4,"DATE",5,"DATUM",3,"DIR",4,"ECHO"
        .byte 4,"EXIT",4,"HELP",5,"HILFE",1,"?",3,"MEM",4,"TIME",4,"ZEIT",4,"TYPE",3,"VER"
        .byte 3,"WIN",6,"FORMAT",3,"DEL",5,"ERASE",4,"COPY",2,"MD",5,"MKDIR",2,"RD",5,"RMDIR"
        .byte 3,"REN",4,"MOVE",4,"PING",8,"NSLOOKUP",8,"IPCONFIG",8,"WINIPCFG",6,"FINGER",0
dos_jt: .addr d_cd, d_cd, d_cls, d_color, d_date, d_date, d_dir, d_echo
        .addr d_exit, d_help, d_help, d_help, d_mem, d_time, d_time, d_type, d_ver
        .addr d_win, d_fmt, d_deny, d_deny, d_deny, d_deny, d_deny, d_deny, d_deny
        .addr d_deny, d_deny, d_ping, d_nslook, d_ipcfg, d_ipcfg, d_finger

; ---------- einfache Befehle ----------
d_ver:  lda #DS_VER
        jmp d_msg
d_win:  lda #DS_WIN
        jmp d_msg
d_fmt:  lda #DS_FMT
        jmp d_msg
d_deny: lda #DS_DENY
        jmp d_msg
d_help: lda #DS_HELP1
        jsr d_msg
        lda #DS_HELP2
        jsr d_msg
        lda #DS_HELP3
        jmp d_msg
d_exit: inc DSQUIT_
        rts
d_cls:  lda #12
        jsr x4_put
        inc DSNOBL_
        rts
d_echo: ldx DSA_
@l:     cpx DSLN_
        bcs @e
        lda DSLINE_,x
        and #$00FF
        phx
        jsr x4_put
        plx
        inx
        bra @l
@e:     jmp x4_nl

; COLOR hg vg (hexadezimal wie bei DOS)
d_color:
        stz DST_                  ; Hintergrund
        lda #7
        sta DST3_                 ; Vordergrund
        ldx DSA_
        cpx DSLN_
        bcs @set
        jsr @hex
        bcc *+5
        jmp @r
        sta DST3_
        inx
        cpx DSLN_
        bcs @set
        jsr @hex
        bcc *+5
        jmp @r
        ldy DST3_
        sty DST_
        sta DST3_
@set:   lda DST_
        and #7
        cmp DST3_                 ; gleiche Farben: nichts tun
        beq @r
        lda #$1B
        jsr x4_put
        lda #'['
        jsr x4_put
        lda #'0'
        jsr x4_put
        lda #';'
        jsr x4_put
        lda #'3'
        jsr x4_put
        lda DST3_
        and #7
        tax
        TBL dos_ansi
        and #$00FF
        jsr x4_put
        lda #';'
        jsr x4_put
        lda #'4'
        jsr x4_put
        lda DST_
        and #7
        tax
        TBL dos_ansi
        and #$00FF
        jsr x4_put
        lda DST3_
        and #8
        beq @m
        lda #';'
        jsr x4_put
        lda #'1'
        jsr x4_put
@m:     lda #'m'
        jsr x4_put
        jmp d_cls
@r:     rts
@hex:   lda DSUP_,x
        and #$00FF
        sec
        sbc #'0'
        cmp #10
        bcc @h
        sbc #7
        cmp #10
        bcc @bad
        cmp #16
        bcs @bad
@h:     clc
        rts
@bad:   sec
        rts
dos_ansi: .byte "04261537"

; ---------- Datum und Uhrzeit (ReadTimeHex) ----------
d_clock:
        pha
        pha
        pha
        pha
        ldx #$0D03
        jsl $E10000
        pla
        sta DSPB_                 ; Sekunde, Minute
        pla
        sta DSPB_+2               ; Stunde, Jahr
        pla
        sta DSPB_+4               ; Tag, Monat
        pla
        sta DSPB_+6               ; -, Wochentag
        rts
d_date: jsr d_clock
        lda #DS_DATE
        jsr d_msg
        lda #DS_WDAY            ; Wochentag: zwei Zeichen aus dem Text
        asl
        tax
        TBL ds_de
        ldy LANG
        beq @w
        TBL ds_en
@w:     jsr x4_kb
        lda DSPB_+7
        and #$00FF
        dec a
        and #7
        asl
        tay
        iny
        lda [LP2],y
        pha
        and #$00FF
        jsr x4_put
        pla
        xba
        and #$00FF
        jsr x4_put
        lda #' '
        jsr x4_put
        lda DSPB_+4
        jsr d_dmy
        jmp x4_nl
; TT.MM.JJJJ aus DSPB_+4 (Tag), +5 (Monat), +3 (Jahr seit 1900)
d_dmy:  lda DSPB_+4
        and #$00FF
        inc a
        jsr d_2d
        lda #'.'
        jsr x4_put
        lda DSPB_+5
        and #$00FF
        inc a
        jsr d_2d
        lda #'.'
        jsr x4_put
        lda DSPB_+3
        and #$00FF
        ldx #'9'
        cmp #100
        bcc @c
        sbc #100
        ldx #'0'
@c:     pha
        lda #'1'
        cpx #'9'
        beq @c1
        lda #'2'
@c1:    phx
        jsr x4_put
        pla
        jsr x4_put
        pla
        jmp d_2d
d_time: jsr d_clock
        lda #DS_TIME
        jsr d_msg
        lda DSPB_+2
        jsr @hm
        lda #':'
        jsr x4_put
        lda DSPB_+1
        jsr @hm
        lda #':'
        jsr x4_put
        lda DSPB_
        jsr @hm
        lda #','
        jsr x4_put
        lda #0
        jsr d_2d
        jmp x4_nl
@hm:    and #$00FF
        jmp d_2d

; ---------- MEM ----------
d_mem:  stz DSN_
        lda #$000A              ; 655360
        sta DSN_+2
        lda #12
        jsr d_num
        lda #DS_MEM1
        jsr d_msg
        pha
        pha
        ldx #$1B02              ; FreeMem
        jsl $E10000
        pla
        sta DSN_
        pla
        sta DSN_+2
        lda #12
        jsr d_num
        lda #DS_MEM2
        jmp d_msg

; ---------- Dateien ----------
; A = Pfad -> DSREF_. C=1 bei Fehler
d_open: sta DSPB_+4
        lda MYBANK
        sta DSPB_+6
        lda #2
        sta DSPB_
        lda #$2010
        ldx #DSPB_
        jsr d_gs
        bcs @r
        lda DSPB_+2
        sta DSREF_
@r:     rts
d_close:
        lda #1
        sta DSPB_
        lda DSREF_
        sta DSPB_+2
        lda #$2014
        ldx #DSPB_
        jmp d_gs
; Dateityp von DSARG_ nach A. C=1: gibt es nicht
d_ftype:
        lda #3
        sta DSPB_
        lda #DSARG_
        sta DSPB_+2
        lda MYBANK
        sta DSPB_+4
        lda #$2006              ; GetFileInfo
        ldx #DSPB_
        jsr d_gs
        bcs @r
        lda DSPB_+8
@r:     rts

; Argument -> DSARG_ (GS/OS-Pfad, '\' wird ':'). C=1: kein Argument
d_mkarg:
        lda DSA_
        cmp DSLN_
        bcc @a
        rts
@a:     tax
        lda DSLINE_,x
        and #$00FF
        cmp #'\'
        bne @rel
        inc DSA_
        lda DSROOT_
        bra @base
@rel:   lda DSPATH_
@base:  sta DST_
        ldx #0
        sep #$20
@c:     cpx DST_
        bcs @c2
        lda DSPATH_+2,x
        sta DSARG_+2,x
        inx
        bra @c
@c2:    ldy DSA_
        cpy DSLN_
        bcs @e
        lda #':'
        sta DSARG_+2,x
        inx
@c3:    cpy DSLN_
        bcs @e
        lda DSLINE_,y
        cmp #'\'
        bne @s
        lda #':'
@s:     sta DSARG_+2,x
        inx
        iny
        cpx #78
        bcc @c3
@e:     dex
        lda DSARG_+2,x
        cmp #':'
        beq @e2
        inx
@e2:    rep #$20
        stx DSARG_
        clc
        rts

; CD, CD .., CD \, CD NAME
d_cd:   ldx DSA_
        cpx DSLN_
        bcc @arg
        lda #'C'
        jsr x4_put
        lda #':'
        jsr x4_put
        jsr d_cwd
        jmp x4_nl
@arg:   lda DSUP_,x
        cmp #$2E2E              ; ".."
        bne @to
        ldx DSPATH_
@up:    cpx DSROOT_
        bcc @r
        beq @r
        dex
        lda DSPATH_+2,x
        and #$00FF
        cmp #':'
        bne @up
        cpx DSROOT_
        bcs @u2
        ldx DSROOT_
@u2:    stx DSPATH_
@r:     rts
@to:    jsr d_mkarg
        jsr d_ftype
        bcs @bad
        cmp #$000F
        bne @bad
        ldx #0
@cp:    lda DSARG_,x
        sta DSPATH_,x
        inx
        inx
        cpx #82
        bcc @cp
        rts
@bad:   lda #DS_BADDIR
        jmp d_msg

; TYPE datei
d_type: jsr d_mkarg
        bcc @a
        lda #DS_NEEDP
        jmp d_msg
@a:     jsr d_ftype
        bcs @nf
        cmp #$000F
        beq @dn
        lda #DSARG_
        jsr d_open
        bcs @nf
        stz DSRLEN_
        stz DSRPOS_
        stz DSLCR_
        lda #1
        sta DSMODE_
        jmp d_cont
@nf:    lda #DS_NOFILE
        jmp d_msg
@dn:    jmp d_deny

; DIR
d_dir:  lda #DS_VOL
        jsr d_msg
        ldx #1
@v:     cpx DSROOT_
        bcs @ve
        lda DSPATH_+2,x
        and #$00FF
        jsr d_upc
        phx
        jsr x4_put
        plx
        inx
        bra @v
@ve:    jsr d_nl
        lda #DS_DIROF
        jsr d_msg
        jsr d_cwd
        jsr d_nl
        jsr d_nl
        lda #DSPATH_
        jsr d_open
        bcc @ok
        lda #DS_BADDIR
        jmp d_msg
@ok:    stz DSCNT_
        stz DSDIRS_
        stz DSSUM_
        stz DSSUM_+2
        lda #2
        sta DSMODE_
        ; weiter in d_cont

; seitenweise weiter (TYPE / DIR)
d_cont: lda DSLC_
        cmp #22
        bcc @go
        lda #DS_MORE
        jmp d_msg
@go:    lda DSMODE_
        cmp #2
        beq @d
        jsr d_tstep
        bra @c
@d:     jsr d_dstep
@c:     bcc d_cont
d_finish:
        jsr d_close
        lda DSMODE_
        cmp #2
        bne @nf
        lda DSCNT_
        sta DSN_
        stz DSN_+2
        lda #9
        jsr d_num
        lda #DS_FILES
        jsr d_msg
        lda DSSUM_
        sta DSN_
        lda DSSUM_+2
        sta DSN_+2
        lda #15
        jsr d_num
        lda #DS_BYTES
        jsr d_msg
        jsr x4_nl
        lda DSDIRS_
        sta DSN_
        stz DSN_+2
        lda #9
        jsr d_num
        lda #DS_DIRS
        jsr d_msg
        jsr x4_nl
@nf:    stz DSMODE_
        rts

; Taste waehrend "-- Fortsetzung --"
d_morekey:
        lda #13
        jsr x4_put
        lda #$1B                ; Zeile loeschen (ESC [ K)
        jsr x4_put
        lda #'['
        jsr x4_put
        lda #'K'
        jsr x4_put
        lda DSKEY_
        cmp #$1B
        beq @ab
        cmp #3
        beq @ab
        stz DSLC_
        jsr d_cont
        bra @p
@ab:    lda #'^'
        jsr x4_put
        lda #'C'
        jsr x4_put
        jsr x4_nl
        lda #1                  ; ohne Summenzeilen
        sta DSMODE_
        jsr d_finish
@p:     lda DSMODE_
        bne @r
        jsr x4_nl
        jmp d_prompt
@r:     rts

; TYPE: eine Zeile. C=1 am Dateiende
d_tstep:
@l:     lda DSRPOS_
        cmp DSRLEN_
        bcc @have
        lda #4
        sta DSPB_
        lda DSREF_
        sta DSPB_+2
        lda #DSBUF_
        sta DSPB_+4
        lda MYBANK
        sta DSPB_+6
        lda #256
        sta DSPB_+8
        stz DSPB_+10
        lda #$2012              ; Read
        ldx #DSPB_
        jsr d_gs
        bcs @eof
        lda DSPB_+12
        beq @eof
        sta DSRLEN_
        stz DSRPOS_
@have:  ldx DSRPOS_
        inc DSRPOS_
        lda DSBUF_,x
        and #$00FF
        cmp #$0A
        bne @n1
        ldx DSLCR_
        stz DSLCR_
        cpx #0
        bne @l
        bra @nl
@n1:    stz DSLCR_
        cmp #$0D
        bne @n2
        inc DSLCR_
@nl:    jsr d_nl
        clc
        rts
@n2:    cmp #9
        beq @pr
        cmp #$20
        bcc @l
@pr:    jsr x4_put
        inc DSCOL_
        lda DSCOL_
        cmp #80
        bcs *+5
        jmp @l
        stz DSCOL_
        inc DSLC_
        clc
        rts
@eof:   lda DSCOL_
        beq @e
        jsr x4_nl
@e:     sec
        rts

; DIR: ein Eintrag. C=1 am Ende
d_dstep:
        lda #11
        sta DSPB_
        lda DSREF_
        sta DSPB_+2
        stz DSPB_+4
        lda #1
        sta DSPB_+6
        sta DSPB_+8
        lda #DSRES_
        sta DSPB_+10
        lda MYBANK
        sta DSPB_+12
        lda #36
        sta DSRES_
        lda #$201C              ; GetDirEntry
        ldx #DSPB_
        jsr d_gs
        bcc @ok
        sec
        rts
@ok:    ldx #0
@n:     cpx DSRES_+2
        bcs @ne
        lda DSRES_+4,x
        and #$00FF
        jsr d_upc
        phx
        jsr x4_put
        plx
        inx
        bra @n
@ne:    lda #16
        sec
        sbc DSRES_+2
        jsr d_spc
        lda DSPB_+16
        and #$00FF
        cmp #$0F
        bne @f
        inc DSDIRS_
        lda #DS_DIRTAG
        jsr d_msg
        bra @dt
@f:     inc DSCNT_
        lda DSPB_+18
        sta DSN_
        clc
        adc DSSUM_
        sta DSSUM_
        lda DSPB_+20
        sta DSN_+2
        adc DSSUM_+2
        sta DSSUM_+2
        lda #13
        jsr d_num
@dt:    lda #2
        jsr d_spc
        lda DSPB_+38              ; Aenderungsdatum (wie ReadTimeHex) ab +34
        sta DSPB_+4
        lda DSPB_+36
        sta DSPB_+2
        jsr d_dmy
        lda #2
        jsr d_spc
        lda DSPB_+36
        and #$00FF
        jsr d_2d
        lda #':'
        jsr x4_put
        lda DSPB_+35
        and #$00FF
        jsr d_2d
        jsr d_nl
        clc
        rts

        .include "dosnet.inc"
        .include "dosstr.inc"
