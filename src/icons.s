        .segment "XCODE2"
FARMODE .set 2
xf_ent_icon:
        jsr ent_icon
        rtl
xf_ficon_draw:
        jsr ficon_draw
        rtl
xf_icons_load:
        jsr icons_load
        rtl
xf_open_assoc:
        jsr open_assoc
        rtl
; ================================================================
; GS/OS95: Finder-Symboldateien ($CA) aus *:Icons
; - Programm- und Dokumentsymbole wie im Apple-Finder
; - Dokumente mit dem zugehoerigen Programm oeffnen (Message Center #1)
; Format: Apple II File Type Note $CA
; ================================================================
ICMAX   = 400
ICFMAX  = 12

; ---------- alle Symboldateien laden (einmal beim Start) ----------
icons_load:
        stz ICN
        lda #icdir
        sta ico_pb+4
        GSOS $2010,ico_pb       ; Open "*:Icons"
        bcc @o
        rts
@o:     lda ico_pb+2
        sta icd_pb+2
        sta icc_pb+2
        stz ICFN
@ent:   lda #1
        sta icd_pb+6
        sta icd_pb+8
        GSOS $201C,icd_pb       ; GetDirEntry
        bcs @done
        lda icd_pb+16
        and #$00FF
        cmp #$00CA
        bne @ent
        lda icd_pb+20           ; nur Dateien < 128 KB
        cmp #2
        bcs @ent
        jsr icon_file
        lda ICFN
        cmp #ICFMAX
        bcc @ent
@done:  GSOS $2014,icc_pb
        rts

; eine Symboldatei: Pfad "*:Icons:" + Name, laden, Eintraege eintragen
icon_file:
        ; Pfad zusammensetzen
        ldx #0
@p:     lda icdir+2,x
        sta icpath+2,x
        inx
        cpx icdir
        bcc @p
        sep #$20
        lda #':'
        sta icpath+2,x
        rep #$20
        inx
        ldy #0
@n:     cpy icname+2
        bcs @pe
        lda icname+4,y
        sep #$20
        sta icpath+2,x
        rep #$20
        inx
        iny
        bra @n
@pe:    stx icpath
        ; Speicher
        pha
        pha
        lda icd_pb+20
        pha
        lda icd_pb+18
        pha
        lda USERID
        pha
        pea $C000
        pea $0000
        pea $0000
        TOOL $0902              ; NewHandle
        pla
        sta LP
        pla
        sta LP+2
        bcc @h
        rts
@h:     ldy #2
        lda [LP],y
        tax
        lda [LP]
        sta LP
        stx LP+2
        lda #icpath
        sta icf_pb+4
        GSOS $2010,icf_pb
        bcc @op
        rts
@op:    lda icf_pb+2
        sta icr_pb+2
        sta icx_pb+2
        lda LP
        sta icr_pb+4
        lda LP+2
        sta icr_pb+6
        lda icd_pb+18
        sta icr_pb+8
        lda icd_pb+20
        sta icr_pb+10
        GSOS $2012,icr_pb       ; Read
        php
        GSOS $2014,icx_pb
        plp
        bcc @rd
        rts
@rd:    inc ICFN
        ; Kennung $0001 an +4
        ldy #4
        lda [LP],y
        cmp #1
        beq @ok
        rts
@ok:    ; Restlaenge = Dateigroesse - 26
        lda icr_pb+12
        sec
        sbc #26
        sta ICREM
        lda icr_pb+14
        sbc #0
        sta ICREM+2
        lda LP
        clc
        adc #26
        sta LP
        bcc @lp
        inc LP+2
@lp:    lda ICREM+2             ; mind. 2 Bytes uebrig?
        bne @more
        lda ICREM
        cmp #88
        bcs @more
        rts
@more:  lda [LP]                ; iDataLen
        beq @r
        sta ICL
        ; Laenge pruefen
        lda ICREM+2
        bne @fit
        lda ICL
        cmp ICREM
        beq @fit
        bcs @r
@fit:   lda ICN
        cmp #ICMAX
        bcs @r
        asl
        asl
        tax
        lda LP
        sta icrec,x
        lda LP+2
        sta icrec+2,x
        inc ICN
        lda ICREM
        sec
        sbc ICL
        sta ICREM
        lda ICREM+2
        sbc #0
        sta ICREM+2
        lda LP
        clc
        adc ICL
        sta LP
        bcc @lp
        inc LP+2
        bra @lp
@r:     rts

; ---------- passendes Symbol fuer Eintrag EP: A = Nr.+1 oder $FFFF ----------
icon_find:
        lda #$FFFF
        sta ICBEST
        stz ICBS
        lda [EP]
        cmp #$000F
        beq @none
        cmp #$0100
        bcs @none
        sta ICT
        ldy #E_AUX
        lda [EP],y
        sta ICA
        ; Name in Grossbuchstaben
        ldy #2
        lda [EP],y
        and #$00FF
        sta ICNL
        ldx #0
        ldy #3
@nc:    cpx ICNL
        bcs @nd
        lda [EP],y
        CALL upc8
        sep #$20
        sta icnm,x
        rep #$20
        inx
        iny
        bra @nc
@nd:    stz ICI
@rl:    lda ICI
        cmp ICN
        bcc @rg
        lda ICBEST
        bmi @none
        inc a
        rts
@none:  lda #$FFFF
        rts
@rg:    asl
        asl
        tax
        lda icrec,x
        sta LP
        lda icrec+2,x
        sta LP+2
        stz ICS
        ldy #82                 ; iDataType
        lda [LP],y
        beq @t0
        cmp ICT
        bne @skip
        inc ICS
@t0:    ldy #84                 ; iDataAux
        lda [LP],y
        beq @a0
        cmp ICA
        bne @skip
        inc ICS
        inc ICS
@a0:    jsr icon_namematch
        bcc @skip
        lda ICS
        beq @skip               ; passt auf alles: lieber eigenes Symbol
        cmp ICBS
        beq @skip
        bcc @skip
        sta ICBS
        lda ICI
        sta ICBEST
@skip:  inc ICI
        bra @rl

; Namensmuster an [LP]+66 gegen icnm/ICNL, '*' als Platzhalter. C=1 passt.
icon_namematch:
        ldy #66
        lda [LP],y
        and #$00FF
        cmp #16
        bcc @l
        lda #15
@l:     sta ICPL
        ; Muster nach icpat (Grossbuchstaben)
        ldx #0
        ldy #67
@c:     cpx ICPL
        bcs @cd
        lda [LP],y
        CALL upc8
        sep #$20
        sta icpat,x
        rep #$20
        inx
        iny
        bra @c
@cd:    lda ICPL
        bne @nz
        sec                     ; leer = jeder Name
        rts
@nz:    ; Stern suchen
        ldx #0
@s:     cpx ICPL
        bcs @nostar
        lda icpat,x
        and #$00FF
        cmp #'*'
        beq @star
        inx
        bra @s
@nostar:
        lda ICPL
        cmp ICNL
        bne @no
        ldx #0
@e:     cpx ICPL
        bcs @yes8
        lda icpat,x
        eor icnm,x
        and #$00FF
        bne @no
        inx
        bra @e
@yes8:  lda ICS
        clc
        adc #8                  ; genauer Name
        sta ICS
        sec
        rts
@no:    clc
        rts
@star:  stx ICPRE               ; Zeichen vor dem Stern
        lda ICPL
        sec
        sbc ICPRE
        dec a
        sta ICSUF               ; Zeichen nach dem Stern
        clc
        adc ICPRE
        cmp ICNL
        beq @lok
        bcs @no
@lok:   ldx #0
@pre:   cpx ICPRE
        bcs @suf
        lda icpat,x
        eor icnm,x
        and #$00FF
        bne @no
        inx
        bra @pre
@suf:   ; Ende vergleichen
        lda ICNL
        sec
        sbc ICSUF
        tay                     ; Startpos. im Namen
        lda ICPRE
        inc a
        tax                     ; Startpos. im Muster
@sl:    cpx ICPL
        bcs @sy
        lda icpat,x
        eor icnm,y
        and #$00FF
        bne @no
        inx
        iny
        bra @sl
@sy:    lda ICPRE
        ora ICSUF
        beq @any
        lda ICS
        clc
        adc #4                  ; Muster wie *.ASM
        sta ICS
@any:   sec
        rts

; Symbol fuer Eintrag EP (Cache in E_ICN): A = Nr.+1 oder $FFFF
ent_icon:
        lda ICN
        bne @g
        lda #$FFFF
        rts
@g:     ldy #E_ICN
        lda [EP],y
        bne @r
        jsr icon_find
        ldy #E_ICN
        sta [EP],y
@r:     rts

; ---------- Finder-Symbol A (Nr.+1) bei Mitte CX, oben IY zeichnen ----------
ficon_draw:
        dec a
        asl
        asl
        tax
        lda icrec,x
        sta LP
        lda icrec+2,x
        sta LP+2
        ldy #86+2
        lda [LP],y
        sta FISZ
        ldy #86+4
        lda [LP],y
        sta FIH
        ldy #86+6
        lda [LP],y
        sta FIW
        lda #86+8
        sta FIOFF
        ; zu gross? dann kleines Symbol
        lda FIH
        cmp #22
        bcs @small
        lda FIW
        cmp #41
        bcc @big
@small: lda FISZ
        asl
        clc
        adc #86+8
        pha
        tay
        iny
        iny
        lda [LP],y
        sta FISZ
        iny
        iny
        lda [LP],y
        sta FIH
        iny
        iny
        lda [LP],y
        sta FIW
        pla
        clc
        adc #8
        sta FIOFF
@big:   lda FIH
        cmp #22
        bcc @hk
        lda #21
        sta FIH
@hk:    ; Zeiger Bild (LP) und Maske (LP2)
        lda LP
        clc
        adc FIOFF
        sta LP
        bcc @b1
        inc LP+2
@b1:    lda LP
        clc
        adc FISZ
        sta LP2
        lda LP+2
        adc #0
        sta LP2+2
        lda FIW
        inc a
        lsr
        sta FIBPR
        ; Position: horizontal mittig, unten buendig ueber dem Namen (Zeile 17)
        lda FIW
        lsr
        sta FIX0
        lda CX
        sec
        sbc FIX0
        sta FIX0
        lda IY
        clc
        adc #17
        sec
        sbc FIH
        sta FIY0
        stz FIR
@row:   lda FIR
        cmp FIH
        bcs @end
        stz FIC
@col:   lda FIC
        cmp FIW
        bcs @nr
        ; Byte-Offset = r*bpr + c/2
        lda FIR
        ldx FIBPR
        jsr imul8
        sta FIOB
        lda FIC
        lsr
        clc
        adc FIOB
        tay
        sep #$20
        lda [LP2],y
        sta FIM
        lda [LP],y
        sta FIP
        rep #$20
        lda FIC
        and #1
        bne @lo
        lda FIM
        and #$00F0
        beq @nx
        lda FIP
        lsr
        lsr
        lsr
        lsr
        bra @set
@lo:    lda FIM
        and #$000F
        beq @nx
        lda FIP
@set:   and #$000F
        CALL setcolor
        lda FIX0
        clc
        adc FIC
        tax
        lda FIY0
        clc
        adc FIR
        tay
        CALL setpixel
@nx:    inc FIC
        bra @col
@nr:    inc FIR
        bra @row
@end:   rts

; A * X (beide klein) -> A
imul8:  stx FIMX
        tax
        lda #0
@m:     cpx #0
        beq @r
        clc
        adc FIMX
        dex
        bra @m
@r:     rts

; ---------- Dokument mit seinem Programm oeffnen ----------
; EP = Eintrag im aktuellen Ordner. C=1: erledigt (Programm gestartet/Meldung)
open_assoc:
        jsr ent_icon
        cmp #$FFFF
        bne @h
@no:    clc
        rts
@h:     dec a
        asl
        asl
        tax
        lda icrec,x
        sta LP
        lda icrec+2,x
        sta LP+2
        ldy #2
        lda [LP],y              ; Laenge iDataBoss
        and #$00FF
        beq @no
        cmp #64
        bcc @bl
        lda #63
@bl:    sta boss
        ; iDataBoss nach boss (GS/OS-String), '/' -> ':'
        tax
        ldy #3
        stz ICI
@bc:    lda ICI
        cmp boss
        bcs @bd
        lda [LP],y
        and #$00FF
        cmp #'/'
        bne @bs
        lda #':'
@bs:    ldx ICI
        sep #$20
        sta boss+2,x
        rep #$20
        inc ICI
        iny
        bra @bc
@bd:    jsr app_find
        bcs @found
        lda #14                 ; Programm nicht gefunden
        CALL show_msg_nr
        CALL redraw_all
        sec
        rts
@found: ; Dokumentpfad (pathbuf:Name) als Nachricht #1 hinterlegen
        lda pathbuf
        pha
        CALL path_append
        jsr msg_open
        pla
        sta pathbuf
        ; Prefix 8/0 = Programmordner (ProDOS-Schreibweise /VOL/DIR/)
        ldx launchbuf
@ls:    dex
        bmi @nopf
        lda launchbuf+2,x
        and #$00FF
        cmp #':'
        bne @ls
        stx ICI                 ; Laenge des Ordnerteils
        ldx #0
        sep #$20
@pc:    cpx ICI
        bcs @pe
        lda launchbuf+2,x
        cmp #':'
        bne @pn
        lda #'/'
@pn:    sta prefbuf+2,x
        inx
        bra @pc
@pe:    lda #'/'
        sta prefbuf+2,x
        inx
        stx prefbuf
        rep #$20
        lda #8
        sta prefix_pb+2
        GSOS $2009,prefix_pb
        stz prefix_pb+2
        GSOS $2009,prefix_pb
@nopf:  rep #$20
        CALL launch
        rts

; Nachricht #1 (Datei oeffnen) mit Pfad pathbuf an das Message Center
msg_open:
        ; Laenge + 10
        lda pathbuf
        clc
        adc #10
        sta ICL
        pha
        pha
        pea $0000
        lda ICL
        pha
        lda USERID
        pha
        pea $C000
        pea $0000
        pea $0000
        TOOL $0902
        pla
        sta LP
        pla
        sta LP+2
        bcc @h
        rts
@h:     lda LP
        sta MSGH
        lda LP+2
        sta MSGH+2
        ldy #2
        lda [LP],y
        tax
        lda [LP]
        sta LP
        stx LP+2
        lda #0
        sta [LP]                ; naechste Nachricht
        ldy #2
        sta [LP],y
        ldy #4
        lda #1                  ; Typ 1
        sta [LP],y
        ldy #6
        lda MSGPRT              ; 0 = oeffnen, 1 = drucken
        sta [LP],y
        sep #$20
        ldy #8
        lda pathbuf
        sta [LP],y              ; Pascal-String, '/' als Trenner
        ldx #0
@c:     cpx pathbuf
        bcs @e
        iny
        lda pathbuf+2,x
        cmp #':'
        bne @s
        lda #'/'
@s:     sta [LP],y
        inx
        bra @c
@e:     iny
        lda #0
        sta [LP],y              ; Ende der Liste
        rep #$20
        pea 1                   ; hinzufuegen
        pea 1                   ; Typ 1
        lda MSGH+2
        pha
        lda MSGH
        pha
        TOOL $1501              ; MessageCenter
        lda MSGH+2
        pha
        lda MSGH
        pha
        TOOL $1002              ; DisposeHandle (MessageCenter hat kopiert)
        rts

; ---------- Programm zu boss suchen -> launchbuf, C=1 gefunden ----------
app_find:
        ; 1. wie angegeben ('*' oder ':' am Anfang) bzw. ':' davor
        stz launchbuf
        lda boss+2
        and #$00FF
        cmp #'*'
        beq @abs
        cmp #':'
        beq @abs
        lda #':'
        jsr lb_ch
@abs:   ldx #boss
        jsr lb_str
        jsr lb_exists
        bcs @r
        ; letzter Pfadteil -> icnm/ICNL (Programmname)
        jsr boss_last
        ; 2. im Ordner des Dokuments
        ldx #0
@cp:    lda pathbuf,x
        sta launchbuf,x
        inx
        inx
        cpx #260
        bcc @cp
        lda #':'
        jsr lb_ch
        jsr lb_last
        jsr lb_exists
        bcs @r
        ; 3. im Wurzelverzeichnis des Startlaufwerks
        stz launchbuf
        lda #'*'
        jsr lb_ch
        lda #':'
        jsr lb_ch
        jsr lb_last
        jsr lb_exists
        bcs @r
        ; 4. alle Laufwerke: Wurzel und erste Ordnerebene
        jmp app_search
@r:     rts

; Programmname (letzter Teil von boss) nach appnm/APNL
boss_last:
        ldx boss
@f:     dex
        bmi @s
        lda boss+2,x
        and #$00FF
        cmp #':'
        beq @s
        cmp #'*'
        beq @s
        bra @f
@s:     inx                     ; Start des Namens
        ldy #0
@c:     cpx boss
        bcs @e
        lda boss+2,x
        sep #$20
        sta appnm,y
        rep #$20
        inx
        iny
        bra @c
@e:     sty APNL
        rts

; launchbuf += Zeichen A
lb_ch:  ldx launchbuf
        sep #$20
        sta launchbuf+2,x
        rep #$20
        inc launchbuf
        rts
; launchbuf += GS/OS-String bei X
lb_str: stx TMP
        lda (TMP)
        sta ICL
        ldy #2
@c:     lda ICL
        beq @r
        lda (TMP),y
        jsr lb_ch
        iny
        dec ICL
        bra @c
@r:     rts
; launchbuf += Programmname
lb_last:
        ldx #0
@c:     cpx APNL
        bcs @r
        phx
        lda appnm,x
        jsr lb_ch
        plx
        inx
        bra @c
@r:     rts
; existiert launchbuf als Programm/Datei? C=1 ja
lb_exists:
        lda #$2006
        ldx #gfi_pb
        ldy #launchbuf
        CALL gsosp
        bcs @no
        lda gfi_pb+8            ; Ordner zaehlen nicht
        cmp #$000F
        beq @no
        sec
        rts
@no:    clc
        rts

; Suche auf allen Laufwerken (Wurzel + eine Ordnerebene)
app_search:
        lda #1
        sta ICDEV
@dev:   lda ICDEV
        sta dinfo_pb+2
        GSOS $202C,dinfo_pb
        bcc @dv
        cmp #$0011              ; keine weiteren Geraete
        beq @nf
        bra @nd
@dv:    lda #devname_buf+2
        sta volume_pb+2
        GSOS $2008,volume_pb
        bcs @nd
        ; Wurzel: ":VOL:" + Name
        jsr lb_vol
        jsr lb_last
        jsr lb_exists
        bcs @found
        ; erste Ordnerebene
        jsr lb_vol
        dec launchbuf           ; ':' am Ende weg -> ":VOL"
        jsr search_level
        bcs @found
@nd:    inc ICDEV
        lda ICDEV
        cmp #40
        bcc @dev
@nf:    clc
        rts
@found: sec
        rts

; launchbuf = Volumename (":VOL") + ':'
lb_vol: stz launchbuf
        ldx #0
@c:     cpx volname_buf+2
        bcs @e
        lda volname_buf+4,x
        phx
        jsr lb_ch
        plx
        inx
        bra @c
@e:     lda #':'
        jmp lb_ch

; Ordner in launchbuf durchsuchen: fuer jeden Unterordner F pruefe Pfad:F:Name
search_level:
        lda launchbuf
        sta ICPATHL
        lda #launchbuf
        sta ico_pb+4
        GSOS $2010,ico_pb
        bcc @o
        clc
        rts
@o:     lda ico_pb+2
        sta icd_pb+2
        sta icc_pb+2
@ent:   lda #1
        sta icd_pb+6
        sta icd_pb+8
        lda ICPATHL
        sta launchbuf
        GSOS $201C,icd_pb
        bcs @nf
        lda icd_pb+16
        and #$00FF
        cmp #$000F
        bne @ent
        lda #':'
        jsr lb_ch
        ldx #0
@n:     cpx icname+2
        bcs @ne
        lda icname+4,x
        phx
        jsr lb_ch
        plx
        inx
        bra @n
@ne:    lda #':'
        jsr lb_ch
        jsr lb_last
        jsr lb_exists
        bcc @ent
        GSOS $2014,icc_pb
        sec
        rts
@nf:    GSOS $2014,icc_pb
        clc
        rts

FARMODE .set 0
        .segment "CODE"
