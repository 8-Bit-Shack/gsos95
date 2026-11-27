; ================================================================
; GS/OS95 v2.14: Explorer-Ergaenzungen (Teil von net.s, Codebank XCODE3)
;   up_sel     nach "Aufwaerts" den Ordner markieren, aus dem man kam
;   bin_empty  Papierkorb leeren mit Fortschritt und Abbrechen
;   sys_chk / sys_sub  Ablegen auf den Systemordner: einsortieren
;   snd_file   Klangdatei ($D8) abspielen: AIFF 8 Bit mono oder Rohdaten
;   keys_ptr   Hilfeseite Tastenkuerzel (about:keys) fuer den Browser
; ================================================================
xf_up_sel:      jsr up_sel
        rtl
xf_bin_empty:   jsr bin_empty
        rtl
xf_sys_chk:     jsr sys_chk
        rtl
xf_sys_sub:     jsr sys_sub
        rtl
xf_wc_sys:      jsr wc_sys
        rtl
xf_snd_file:    jsr snd_file
        rtl
xf_spl_pic:     jsr spl_pic
        rtl
xf_spl_save:    jsr spl_save
        rtl
xf_keys_ptr:    lda #w_keys
        ldx LANG
        beq :+
        lda #w_keys_en
:       sta PDST
        jsr x3bank
        sta PDST+2
        rtl

        .pushseg
        .segment "BSS"
UPN:    .res 2
UPI:    .res 2
UPC:    .res 2
SYI:    .res 2
SYX:    .res 2
SFH:    .res 4                  ; Griff der Klangdaten
SFL:    .res 4                  ; Laenge der Klangdaten
SFR:    .res 2                  ; Abtastrate
SRF:    .res 2                  ; 1: Ressourcenzweig
SFX:    .res 8                  ; Startbild: Griff, Adresse
SPSV:   .res 2                  ; 1: Bild in SFX gesichert
eof_pb: .res 8                  ; GetEOF: pcount, refNum, eof
        .popseg

; Bank dieses Codes -> A
x3bank: sep #$20
        phk
        pla
        rep #$20
        and #$00FF
        rts

; ---------- Aufwaerts: Herkunftsordner markieren ----------
; UPX = Stelle des ':' vor dem Namen in pathbuf, UPL = alte Laenge
up_sel:
        lda UPL
        sec
        sbc UPX
        dec a
        beq @r
        bmi @r
        cmp #32
        bcs @r
        sta UPN
        stz UPI
@l:     lda UPI
        cmp COUNT
        bcs @r
        CALL entep
        ldy #2
        lda [EP],y
        and #$00FF
        cmp UPN
        bne @nx
        ldx UPX
        ldy #3
@c:     lda pathbuf+3,x
        CALL upc8
        sta UPC
        lda [EP],y
        CALL upc8
        cmp UPC
        bne @nx
        inx
        iny
        tya
        sec
        sbc #3
        cmp UPN
        bcc @c
        lda UPI                 ; gefunden: markieren und sichtbar machen
        sta SEL2
        stz MSELN
        CALL win_geom
        lda UPI
        CALL ensure_vis
@r:     rts
@nx:    inc UPI
        bra @l

; ---------- Papierkorb leeren ----------
bin_empty:
        jsr bin_cnt
        ldx #2                  ; Fortschritt "Loesche:"
        CALL prog_begin
@l:     CALL prog_poll
        bcs @ab
        jsr bin_first
        bcs @done
        ldx #workpath
        ldy #srcpath
        CALL pcopy
        CALL prog_file
        CALL destroy_tree
        bcs @ab
        CALL prog_step
        bra @l
@done:  ldx #binpath            ; leer: Ordner selbst und Liste weg
        ldy #workpath
        CALL pcopy
        CALL destroy_tree
        CALL bin_clear
@ab:    CALL prog_end
        rts

; Zahl der Eintraege in *:RECYCLED -> A
bin_cnt:
        lda #binpath
        sta wopen_pb+4
        GSOS $2010,wopen_pb
        bcs @z
        lda wopen_pb+2
        sta wdirent_pb+2
        sta wclose_pb+2
        stz wdirent_pb+6
        stz wdirent_pb+8
        GSOS $201C,wdirent_pb
        php
        GSOS $2014,wclose_pb
        plp
        bcs @z
        lda wdirent_pb+14
        rts
@z:     lda #0
        rts

; erster Eintrag des Papierkorbs -> workpath. C=1 wenn leer
bin_first:
        ldx #binpath
        ldy #workpath
        CALL pcopy
        lda #workpath
        sta wopen_pb+4
        GSOS $2010,wopen_pb
        bcs @no
        lda wopen_pb+2
        sta wdirent_pb+2
        sta wclose_pb+2
        lda #1
        sta wdirent_pb+6
        sta wdirent_pb+8
        GSOS $201C,wdirent_pb
        php
        GSOS $2014,wclose_pb
        plp
        bcs @no
        lda #wname+1            ; Pseudo-Eintrag: Laenge bei +2, Name ab +3
        sta EP
        lda MYBANK
        sta EP+2
        lda wname+2
        sep #$20
        sta wname+3
        rep #$20
        ldy #workpath
        CALL pappend
        clc
        rts
@no:    sec
        rts

; ---------- Ablegen auf den Systemordner ----------
; C=1: Ziel dstdir ist ein Ordner "System" und mindestens ein Objekt gehoert
; in einen seiner Unterordner (nicht beim Papierkorb)
sys_chk:
        lda DGBIN
        bne @no
        jsr is_sysdir
        bcc @no
        stz SYI
@l:     lda SYI
        cmp DGN
        bcs @no
        jsr sy_ep
        lda [EP]
        jsr sys_idx
        bcs @yes
        inc SYI
        bra @l
@yes:   sec
        rts
@no:    clc
        rts

; EP = dgents + SYI*48
sy_ep:  lda SYI
        asl
        asl
        asl
        asl
        sta UPC
        asl
        clc
        adc UPC
        clc
        adc #dgents
        sta EP
        lda MYBANK
        sta EP+2
        rts

; endet dstdir auf ":SYSTEM"? C=1
is_sysdir:
        ldy dstdir
        cpy #8
        bcc @no
        ldx #0
@c:     lda dstdir+1,y
        CALL upc8
        sta UPC
        phb                     ; Tabelle in dieser Codebank
        phk
        plb
        lda sysrev,x
        plb
        and #$00FF
        cmp UPC
        bne @no
        dey
        inx
        cpx #7
        bcc @c
        sec
        rts
@no:    clc
        rts
sysrev: .byte "METSYS:"

; Dateityp A -> X = Stelle in systyp/sysdn, C=1 wenn einsortierbar
sys_idx:
        sta UPC
        ldx #0
@l:     phb
        phk
        plb
        lda systyp,x
        plb
        cmp #0
        beq @no
        cmp UPC
        beq @y
        inx
        inx
        bra @l
@y:     sec
        rts
@no:    clc
        rts
systyp: .word $B6,$B7,$B8,$B9,$BA,$BB,$BD,$C7,$C8,0
sysdn:  .word sd_set,sd_set,sd_da,sd_da,sd_tl,sd_dv,sd_fs,sd_cd,sd_fn
; Pseudo-Eintraege fuer pappend: Typ, Laengenbyte, Name
sd_set: .word 0
        .byte 12,"System.Setup"
sd_da:  .word 0
        .byte 9,"Desk.Accs"
sd_tl:  .word 0
        .byte 5,"Tools"
sd_dv:  .word 0
        .byte 7,"Drivers"
sd_fs:  .word 0
        .byte 4,"FSTs"
sd_cd:  .word 0
        .byte 5,"CDevs"
sd_fn:  .word 0
        .byte 5,"Fonts"

; in drop_xfer: dstpath = dstdir:Unterordner:Name (EP = Eintrag), wenn SYSSORT
sys_sub:
        lda SYSSORT
        beq @r
        lda [EP]
        jsr sys_idx
        bcc @r
        stx SYX
        lda EP
        pha
        lda EP+2
        pha
        ldx #dstdir
        ldy #dstpath
        CALL pcopy
        ldx SYX
        phb
        phk
        plb
        lda sysdn,x
        plb
        sta EP
        jsr x3bank
        sta EP+2
        ldy #dstpath
        CALL pappend
        lda #$000F              ; Unterordner anlegen (Fehler egal)
        sta create_pb+8
        stz create_pb+10
        stz create_pb+12
        lda #$000D
        sta create_pb+14
        lda #$2001
        ldx #create_pb
        ldy #dstpath
        CALL gsosp
        pla
        sta EP+2
        pla
        sta EP
        ldy #dstpath
        CALL pappend
@r:     rts

; Rueckfrage (Fensterart 12, CONFACT 6)
wc_sys: ICON WX+14,WY+30,i_folder
        TEXT WX+40,WY+34,t_csys1,BLACK
        TEXT WX+40,WY+46,t_csys2,BLACK
        CALL confirm_btns
        rts

; ---------- Klangdatei abspielen (IDX = Eintrag im Explorer) ----------
snd_file:
        CALL entry_path         ; -> srcpath, EP
        JSLF xf_snd_up
        bcc *+5
        jmp @r
        jsr snd_stop
        stz SRF
        lda #srcpath            ; erst Datenzweig oeffnen
        sta o4_pb+4
        lda #1
        sta o4_pb+8
        stz o4_pb+10
@op:    GSOS $2010,o4_pb
        bcc *+5
        jmp @r
        lda o4_pb+2
        sta io_pb+2
        sta cl_pb+2
        sta eof_pb+2
        lda #2
        sta eof_pb
        GSOS $2019,eof_pb       ; GetEOF
        lda eof_pb+4
        sta SFL
        lda eof_pb+6
        sta SFL+2
        ora SFL
        bne @sz
        GSOS $2014,cl_pb        ; Datenzweig leer: Ressourcenzweig (rSoundSample)
        lda SRF
        beq *+5
        jmp @r
        inc SRF
        lda #1
        sta o4_pb+10
        bra @op
@sz:    lda SFL+2
        cmp #8                  ; hoechstens 512 KB
        bcc @al
@cl:    GSOS $2014,cl_pb
        jmp @r
        ; Speicher: Laenge auf Seiten aufgerundet + 1 Seite
@al:    pha
        pha
        lda SFL
        and #$FF00
        clc
        adc #$0200
        sta UPC
        lda SFL+2
        adc #0
        pha
        lda UPC
        pha
        lda USERID
        pha
        pea $C000               ; fest, gesperrt
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta SFH
        pla
        sta SFH+2
        bcc @ok
        stz SFH
        stz SFH+2
        bra @cl
@ok:    lda SFH                 ; PDST = Anfang
        sta LP2
        lda SFH+2
        sta LP2+2
        ldy #2
        lda [LP2],y
        sta PDST+2
        lda [LP2]
        sta PDST
        lda io_pb+4
        pha
        lda io_pb+6
        pha
        lda PDST
        sta io_pb+4
        lda PDST+2
        sta io_pb+6
        lda SFL
        sta io_pb+8
        lda SFL+2
        sta io_pb+10
        GSOS $2012,io_pb
        php
        GSOS $2014,cl_pb
        plp
        plx
        stx io_pb+6
        plx
        stx io_pb+4
        stz io_pb+10
        bcc *+5
        jmp @fr
        lda #11025              ; Rohdaten: 11 kHz, ohne Vorzeichen
        sta SFR
        lda SRF
        beq @af
        jsr rsrc_parse          ; Ressourcenzweig: rSoundSample suchen
        bra @pc
@af:    jsr aiff_parse          ; AIFF: PDST/SFL auf die Daten, Rate, C=1 unbrauchbar
@pc:    bcs @fr
        jsr snd_prep            ; Nullbytes weg, Ende mit Stille auffuellen
        lda PDST
        sta sndpb
        lda PDST+2
        sta sndpb+2
        lda SFL                 ; Seiten
        clc
        adc #255
        xba
        and #$00FF
        sta UPC
        lda SFL+2
        adc #0
        xba
        and #$FF00
        ora UPC
        sta sndpb+4
        lda SFR                 ; Frequenz: Rate*32/1645 ~ Rate/51
        ldx #51
        CALL gdiv
        sta sndpb+6
        lda #$8000
        sta sndpb+8
        lda #3
        sta sndpb+10
        stz sndpb+12
        stz sndpb+14
        lda #255
        sta sndpb+16
        pea $0101
        lda MYBANK
        pha
        pea sndpb
        TOOL $0E08              ; FFStartSound
@r:     rts
@fr:    jmp snd_free
@r0:    stz SFH
        stz SFH+2
        rts

; laufenden Klang anhalten, alten Speicher freigeben
snd_stop:
        pea $0002               ; Generator 1
        TOOL $0F08              ; FFStopSound
snd_free:
        lda SFH
        ora SFH+2
        beq @r
        lda SFH+2
        pha
        lda SFH
        pha
        TOOL $1002              ; DisposeHandle
        stz SFH
        stz SFH+2
@r:     rts

; 32-Bit-Wert (gross-endig) bei [LP2],Y -> UPI (hi), A (lo)
be32:   lda [LP2],y
        xba
        sta UPI
        iny
        iny
        lda [LP2],y
        xba
        rts

; LP2 += A (16 Bit, mit Uebertrag in die Bank)
lp_add: clc
        adc LP2
        sta LP2
        bcc @r
        inc LP2+2
@r:     rts

; AIFF? Sonst Rohdaten lassen. C=1: Format nicht abspielbar
aiff_parse:
        lda SFL+2
        bne @big
        lda SFL
        cmp #64
        bcs @big
        clc                     ; winzig: als Rohdaten
        rts
@big:   lda [PDST]
        cmp #$4F46              ; "FO"
        beq *+5
        jmp @raw
        ldy #2
        lda [PDST],y
        cmp #$4D52              ; "RM"
        beq *+5
        jmp @raw
        ldy #8
        lda [PDST],y
        cmp #$4941              ; "AI"
        beq *+5
        jmp @raw
        lda PDST                ; LP2 = erster Block (Offset 12)
        sta LP2
        lda PDST+2
        sta LP2+2
        lda #12
        jsr lp_add
        stz SYI                 ; hoechstens 16 Bloecke
@ck:    inc SYI
        lda SYI
        cmp #17
        bcc *+5
        jmp @bad
        lda [LP2]
        sta SYX                 ; Kennung (erste 2 Zeichen)
        ldy #4
        jsr be32
        sta UPN                 ; Blocklaenge lo, UPI = hi
        lda SYX
        cmp #$4F43              ; "CO"MM
        beq @comm
        cmp #$5353              ; "SS"ND
        beq @ssnd
@nxt:   lda UPN                 ; naechster Block: 8 + Laenge (gerade)
        clc
        adc #9
        and #$FFFE
        jsr lp_add
        lda UPI
        clc
        adc LP2+2
        sta LP2+2
        bra @ck
@comm:  ldy #8                  ; Kanaele = 1, Bits = 8
        lda [LP2],y
        xba
        cmp #1
        beq *+5
        jmp @bad
        ldy #14
        lda [LP2],y
        xba
        cmp #8
        beq *+5
        jmp @bad
        ldy #16                 ; Exponent, Mantisse (80 Bit)
        lda [LP2],y
        xba
        and #$7FFF
        sta UPC
        lda #16398
        sec
        sbc UPC
        bmi @nxt
        cmp #16
        bcs @nxt
        tax
        ldy #18
        lda [LP2],y
        xba
@sh:    dex
        bmi @sr
        lsr
        bra @sh
@sr:    sta SFR
        bra @nxt
@ssnd:  lda UPN                 ; Datenlaenge = Blocklaenge - 8 - Offset
        sec
        sbc #8
        sta SFL
        lda UPI
        sbc #0
        sta SFL+2
        ldy #8                  ; Offset innerhalb der Daten
        jsr be32
        sta UPN
        lda SFL
        sec
        sbc UPN
        sta SFL
        lda SFL+2
        sbc UPI
        sta SFL+2
        bmi @bad
        lda UPN
        clc
        adc #16
        sta UPC
        lda UPC
        jsr lp_add
        lda LP2
        sta PDST
        lda LP2+2
        sta PDST+2
        lda #$80                ; AIFF: mit Vorzeichen -> umdrehen
        sta UPC
        clc
        rts
@raw:   stz UPC
        clc
        rts
@bad:   sec
        rts

; Ressourcenzweig bei PDST: erste Ressource vom Typ rSoundSample ($8024) suchen.
; Kopf: Format, Seiten, Tonhoehe, Stereo, Rate (je 2 Bytes), dann Daten ohne Vorzeichen
rsrc_parse:
        ldy #4                  ; rFileToMap
        lda [PDST],y
        sta UPN
        lda PDST                ; LP2 = Map
        sta LP2
        lda PDST+2
        sta LP2+2
        iny
        iny
        lda [PDST],y
        clc
        adc LP2+2
        sta LP2+2
        lda UPN
        jsr lp_add
        ldy #14                 ; mapToIndex
        lda [LP2],y
        sta UPN
        ldy #24                 ; mapIndexUsed (lo)
        lda [LP2],y
        beq @bad
        cmp #200
        bcs @bad
        sta SYI
        lda UPN
        jsr lp_add              ; LP2 = Index (20 Bytes je Eintrag)
@l:     lda [LP2]
        cmp #$8024
        beq @f
        lda #20
        jsr lp_add
        dec SYI
        bne @l
@bad:   sec
        rts
@f:     ldy #6                  ; resOffset
        lda [LP2],y
        sta UPN
        iny
        iny
        lda [LP2],y
        sta UPI
        ldy #12                 ; resSize
        lda [LP2],y
        sta SFL
        iny
        iny
        lda [LP2],y
        sta SFL+2
        lda PDST                ; LP2 = Ressource
        sta LP2
        lda PDST+2
        clc
        adc UPI
        sta LP2+2
        lda UPN
        jsr lp_add
        ldy #8                  ; Abtastrate
        lda [LP2],y
        beq @nr
        sta SFR
@nr:    lda #10                 ; Daten hinter dem Kopf
        jsr lp_add
        lda LP2
        sta PDST
        lda LP2+2
        sta PDST+2
        lda SFL
        sec
        sbc #10
        sta SFL
        lda SFL+2
        sbc #0
        sta SFL+2
        bmi @bad
        stz UPC                 ; ohne Vorzeichen
        clc
        rts

; Daten bei PDST (SFL Bytes): Bit 7 nach UPC umdrehen, 0 -> 1,
; danach 256 Bytes Stille ($80)
snd_prep:
        lda PDST
        sta LP2
        lda PDST+2
        sta LP2+2
        lda SFL
        sta SYI
        lda SFL+2
        sta SYX
        sep #$20
@l:     lda SYI
        ora SYI+1
        ora SYX
        beq @e
        lda [LP2]
        eor UPC
        bne @nz
        inc a
@nz:    sta [LP2]
        rep #$20
        inc LP2
        bne @b
        inc LP2+2
@b:     lda SYI
        bne @d
        dec SYX
@d:     dec SYI
        sep #$20
        bra @l
@e:     ldy #0
        lda #$80
@f:     sta [LP2],y
        iny
        cpy #256
        bcc @f
        rep #$20
        rts

; ---------- Startbild aus *:System:GSOS95.SPL (wie START.GS.OS) ----------
; liest 32 KB (Bild, SCBs, Paletten) direkt nach $E1/2000. C=1: keine Datei
spl_pic:
        lda SPSV                ; Bild schon vor QDStartUp gesichert?
        beq @ld
        stz SPSV
        jmp @cp
@ld:    pha                     ; Speicherblock 32 KB (erst ganz lesen, dann in
        pha                     ; ruhiger Reihenfolge auf den Schirm: kein Flackern)
        pea $0000
        pea $8000
        lda USERID
        pha
        pea $C000
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta SFX
        pla
        sta SFX+2
        bcc *+5
        jmp @r
        lda SFX
        sta LP2
        lda SFX+2
        sta LP2+2
        ldy #2
        lda [LP2],y
        sta SFX+6
        lda [LP2]
        sta SFX+4
        lda #splpath
        sta o4_pb+4
        jsr x3bank
        sta o4_pb+6
        lda #1
        sta o4_pb+8
        stz o4_pb+10
        GSOS $2010,o4_pb
        php
        lda MYBANK
        sta o4_pb+6
        plp
        bcc *+5
        jmp @fr
        lda o4_pb+2
        sta io_pb+2
        sta cl_pb+2
        lda io_pb+4
        pha
        lda io_pb+6
        pha
        lda SFX+4
        sta io_pb+4
        lda SFX+6
        sta io_pb+6
        lda #$8000
        sta io_pb+8
        stz io_pb+10
        GSOS $2012,io_pb
        php
        GSOS $2014,cl_pb
        plp
        plx
        stx io_pb+6
        plx
        stx io_pb+4
        bcc *+5
        jmp @fr
        lda io_pb+12            ; ganz gelesen?
        cmp #$8000
        beq *+5
        jmp @fr
@cp:    lda SFX+4               ; Paletten zuerst
        clc
        adc #32256
        sta LP2
        lda SFX+6
        adc #0
        sta LP2+2
        lda #$9E00
        sta PDST
        lda #$00E1
        sta PDST+2
        ldy #510
@pl:    lda [LP2],y
        sta [PDST],y
        dey
        dey
        bpl @pl
        stz UPI                 ; dann je Zeile Punkte, danach ihr SCB
@rw:    lda UPI
        xba
        lsr
        sta UPC
        lsr
        lsr
        clc
        adc UPC
        sta UPC                 ; Zeile * 160
        clc
        adc SFX+4
        sta LP2
        lda SFX+6
        adc #0
        sta LP2+2
        lda UPC
        clc
        adc #$2000
        sta PDST
        ldy #158
@px:    lda [LP2],y
        sta [PDST],y
        dey
        dey
        bpl @px
        lda SFX+4
        clc
        adc #32000
        sta LP2
        lda SFX+6
        adc #0
        sta LP2+2
        ldy UPI
        sep #$20
        lda [LP2],y
        tyx
        sta f:$E19D00,x
        rep #$20
        inc UPI
        lda UPI
        cmp #200
        bcc @rw
        jsr @fr
        clc
        rts
@fr:    lda SFX+2
        pha
        lda SFX
        pha
        TOOL $1002              ; DisposeHandle
        sec
@r:     rts
; Vor QDStartUp (das Farben und SCBs zuruecksetzt und den Schirm loescht):
; Bootbild samt SCBs/Paletten in einen Block sichern, dann alle Farben schwarz -
; so gibt es keine verwuerfelten Farben, und spl_pic stellt es sofort wieder her
; (ohne die Datei neu zu lesen).
spl_save:
        stz SPSV
        lda BOOTP
        bne *+3
        rts
        pha
        pha
        pea $0000
        pea $8000
        lda USERID
        pha
        pea $C000
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta SFX
        pla
        sta SFX+2
        bcc *+3
        rts
        lda SFX
        sta LP2
        lda SFX+2
        sta LP2+2
        ldy #2
        lda [LP2],y
        sta SFX+6
        lda [LP2]
        sta SFX+4
        sta LP2
        lda SFX+6
        sta LP2+2
        lda #$2000
        sta PDST
        lda #$00E1
        sta PDST+2
        ldy #0
@cp:    lda [PDST],y
        sta [LP2],y
        iny
        iny
        cpy #$8000
        bcc @cp
        jsr waitvb3
        ldx #510                ; alle Farben schwarz
        lda #0
@bk:    sta f:$E19E00,x
        dex
        dex
        bpl @bk
        ldy #32200-2            ; Punkte und SCBs loeschen: QDStartUp setzt gleich die
@cl:    sta [PDST],y            ; Standardfarben - dann ist schon alles Farbe 0 (schwarz)
        dey
        dey
        bpl @cl
        inc SPSV
        rts
; auf den Bildruecklauf warten (wie waitvbl in app.s)
waitvb3:
        sep #$20
@w1:    lda f:$E0C019
        bmi @w1
@w2:    lda f:$E0C019
        bpl @w2
        rep #$20
        rts
splpath: .byte 19,0,"*:System:GSOS95.SPL"

; ---------- Hilfeseite Tastenkuerzel ----------
        .include "keyhtml.inc"
